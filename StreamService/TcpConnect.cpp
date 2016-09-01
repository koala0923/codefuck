#include"TcpConnect.h"

#include "EventLoop.h"

#include "event2/buffer.h"

#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <strstream>

#include "StreamService.pb.h"
#include "StrUser.h"
#include "log4z.h"

static ConnMap_t g_conn_map;
#define CLIENT_TIMEOUT				60000 //120000 // 30000 //超时时间为一分钟

TcpConnect::TcpConnect (const int fd) :
    m_fd (fd)
{
  m_Port = 0;
  m_pBuffEvent = NULL;
  m_last_send_tick = 0;
  m_last_recv_tick = 0;
}

TcpConnect::~TcpConnect ()
{
  //printf ("Destruct TcpConnect fd = %d\n", m_fd);
  LOGI("Destruct TcpConnect fd = " << m_fd);
}

void
TcpConnect::Init ()
{
  g_conn_map.insert (std::make_pair<int, TcpConnect*> (m_fd, this));

  event_base *base = EventLoop::Instance ()->GetEventBase ();
  m_pBuffEvent = bufferevent_socket_new (base, m_fd, BEV_OPT_CLOSE_ON_FREE);
  if (m_pBuffEvent == NULL)
    {
      LOGE("Init BuffEvent errno");
      return;
    }

  bufferevent_setcb (m_pBuffEvent, TcpConnect::socket_read_cb, NULL,
		     TcpConnect::socket_event_cb, this);

  bufferevent_enable (m_pBuffEvent, EV_READ | EV_PERSIST);

  m_last_send_tick = m_last_recv_tick = get_tick_count ();
}

void
TcpConnect::SetAddr (sockaddr const* sock)
{

  struct sockaddr_in *client_addr = (struct sockaddr_in *) sock;
  if (client_addr->sin_addr.s_addr != INADDR_NONE)
    {
      m_ClientIp = inet_ntoa (client_addr->sin_addr);
      m_Port = client_addr->sin_port;
    }

}

std::string
TcpConnect::GetAddr ()
{
  std::strstream ss;
  std::string address;
  std::string port;
  ss << m_Port;
  ss >> port;
  // printf("%d fuck port = %s\n", m_Port,port.c_str());
  address = m_ClientIp + ":" + port;
  return address;
}

void
TcpConnect::socket_read_cb (bufferevent *bev, void *arg)
{
  CSsPdu* pPdu = NULL;
  TcpConnect *connect = (TcpConnect *) arg;
  unsigned int be_pdu_len = 0;
  //printf ("socket_read_cb() receiveMsg\n");
  LOGD("socket_read_cb() receiveMsg");
  while ((pPdu = CSsPdu::ReadPdu (bev, be_pdu_len)))
    {

      connect->HandlePdu (pPdu);
      connect->SetLastRecTick (get_tick_count ());
      delete pPdu;
      pPdu = NULL;

    }

  if (be_pdu_len > 1024)
    {
      connect->Close ();
    }
}

//void
//TcpConnect::socket_read_cb (bufferevent *bev, void *arg)
//{
//  char msg[4096];
//  TcpConnect *connect = (TcpConnect *) arg;
//
//  struct evbuffer*inbuffer = bufferevent_get_input (bev);
//  printf ("input buffer size = %d\n", (int) evbuffer_get_length (inbuffer));
//
//  if (evbuffer_get_length (inbuffer) >= 99)
//    {
//      int fd = connect->Getfd ();
//      close (fd);
//      bufferevent_free (bev);
//      return;
//    }
//
//  size_t len = bufferevent_read (bev, msg, sizeof(msg) - 1);
//
//  connect->SetLastRecTick (get_tick_count ());
//  msg[len] = '\0';
//  printf ("server read the data: %s from %s\n", msg,
//	  connect->GetAddr ().c_str ());
//
//  char reply[] = "I has read your data";
//  bufferevent_write (bev, reply, strlen (reply));
//}

void
TcpConnect::HandlePdu (CSsPdu* pPdu)
{
//  m_last_recv_tick = get_tick_count ();

//printf ("HandlePdu() CommandId = %d\n", pPdu->GetCommandId ());
  LOGD("HandlePdu() CommandId = " << pPdu->GetCommandId ());

  if (pPdu->GetCommandId () != StreamService::CID_LOGIN_REQ_MSG)
    {
      /*不是登陆包,看看这个连接有没有登陆*/

//      printf ("权限验证\n");
      LOGD("权限验证")
      CUser* pUser = NULL;
      if ((pUser = CUserManager::Instance ()->GetUser (m_fd)) == NULL)
	{
	  //这将自动close套接字和free读写缓冲区
	  bufferevent_free (m_pBuffEvent);
	  g_conn_map.erase (m_fd);
	  delete this;
	  return;
	}

    }
  switch (pPdu->GetCommandId ())
    {

    case StreamService::CID_LOGIN_REQ_MSG:
      //printf ("Login Message>>>>>\n");
      LOGD("Login Message>>>>>")
      ;
      _HandleLoginRequest (pPdu);
      break;
    case StreamService::CID_HEARTBEAT_REQ_MSG:
      //printf ("HeartBeat Message>>>>>\n");
      LOGD("HeartBeat Message>>>>>")
      ;
      _HandleHeartBeat (pPdu);
      break;
    case StreamService::CID_CALL_REQ_MSG:
      //printf ("Call Message>>>>>\n");
      LOGD("Call Message>>>>>")
      ;
      _HandleCallRequest (pPdu);
      break;
    case StreamService::CID_CALL_RES_MSG:
      //printf ("Call Res Message>>>>>");
      LOGD("Call Res Message>>>>>")
      ;
      _HandleCallRespond (pPdu);
      break;
    case StreamService::CID_HANGUP_REQ_MSG:
      //printf ("HangUp Message>>>>>\n");
      LOGD("HangUp Message>>>>>")
      ;
      _HandleHangUpRequest (pPdu);
      break;
    default:
      // printf ("Unknown Message>>>>>\n");
      LOGD("Unknown Message>>>>>")
      ;
      Close ();
      break;
    }
}

void
TcpConnect::socket_event_cb (bufferevent *bev, short events, void *arg)
{
  TcpConnect *connect = (TcpConnect *) arg;
  if (events & BEV_EVENT_EOF)
    {

      // printf ("connection closed from %s\n", connect->GetAddr ().c_str ());
      LOGI("Connection closed from " << connect->GetAddr ().c_str ());

    }
  else if (events & BEV_EVENT_ERROR)
    {

      //    printf ("some other error\n");
      LOGE("some other error");
    }

  CUser* pUser = NULL;
  if ((pUser = CUserManager::Instance ()->GetUser (connect->Getfd ())) != NULL)
    {
      CUserManager::Instance ()->RemoveUser (connect->Getfd ());
      CUserManager::Instance ()->RemoveUser (pUser->GetUsername ());
      delete pUser;
    }
  //这将自动close套接字和free读写缓冲区
  bufferevent_free (bev);
  g_conn_map.erase (connect->Getfd ());
  delete connect;

//  for (ConnMap_t::iterator it = g_conn_map.begin(); it != g_conn_map.end(); it++) {
//  		printf("connected client ip %s , fd = %d\n",it->second->GetAddr().c_str(), it->first);
//  	}
}

int
TcpConnect::SendPdu (CSsPdu* pPdu)
{
  m_last_send_tick = get_tick_count ();
  return bufferevent_write (m_pBuffEvent, pPdu->GetBuffer (),
			    pPdu->GetLength ());
}

void
TcpConnect::_HandleLoginRequest (CSsPdu* pPdu)
{
  StreamService::LoginReq msg;
  CHECK_PB_PARSE_MSG(
      msg.ParseFromArray (pPdu->GetBodyData (), pPdu->GetBodyLength ()));

  std::string user_name = msg.user_name ();
  std::string password = msg.password ();
  std::string reversed = msg.reversed ();
//  printf ("HandleLoginReq, user_name = %s, password = %s , reversed = %s \n",
//	  user_name.c_str (), password.c_str (), reversed.c_str ());
  LOGD(
      "HandleLoginReq, user_name = "<< user_name.c_str () << ", password =  " << password.c_str () << ", reversed = " << reversed.c_str ());
  StreamService::LoginResultType result = StreamService::LOGIN_RESULT_NONE;
  std::string resultString;
  if (_UserAuthentication (user_name, password))
    {

      result = StreamService::LOGIN_RESULT_OK;
      resultString = "登陆成功";
      CUser* pUser = new CUser (user_name);
      pUser->SetConnect (this);
      CUserManager::Instance ()->AddUser (m_fd, pUser);
      CUserManager::Instance ()->AddUser (user_name, pUser);
    }
  else
    {
      result = StreamService::LOGIN_RESULT_FAILED;
      resultString = "登陆失败";
    }

  StreamService::LoginRes Resmsg;
  Resmsg.set_result_code (result);
  Resmsg.set_result_string (resultString);
  Resmsg.set_reversed ("");

  CSsPdu pdu;
  pdu.SetCommandId (StreamService::CID_LOGIN_RES_MSG);
  pdu.SetSeqNum (pPdu->GetSeqNum ());
  pdu.SetPBMsg (&Resmsg);
  SendPdu (&pdu);
}

void
TcpConnect::_HandleHeartBeat (CSsPdu* pPdu)
{
  StreamService::HeartBeatReq msg;
  CHECK_PB_PARSE_MSG(
      msg.ParseFromArray (pPdu->GetBodyData (), pPdu->GetBodyLength ()));

  std::string user_name = msg.user_name ();
  unsigned int msg_id = msg.msg_id ();
  std::string reversed = msg.reversed ();
//  printf ("HandleHeartBeatReq, user_name = %s, msgId = %d , reversed = %s \n",
//	  user_name.c_str (), msg_id, reversed.c_str ());
  LOGD(
      "HandleHeartBeatReq, user_name = "<< user_name.c_str () << ", msgId = " << msg_id << ", reversed = " << reversed.c_str ());
  StreamService::HeartBeatRes Resmsg;
  Resmsg.set_user_name (user_name);
  Resmsg.set_msg_id (msg_id);
  Resmsg.set_reversed ("");

  CSsPdu pdu;
  pdu.SetCommandId (StreamService::CID_HEARTBEAT_RES_MSG);
  pdu.SetSeqNum (pPdu->GetSeqNum ());
  pdu.SetPBMsg (&Resmsg);
  SendPdu (&pdu);
}

void
TcpConnect::_HandleCallRequest (CSsPdu* pPdu)
{
  StreamService::CallReq msg;
  CHECK_PB_PARSE_MSG(
      msg.ParseFromArray (pPdu->GetBodyData (), pPdu->GetBodyLength ()));

  std::string from_user_name = msg.from_user_name ();
  std::string to_session_name = msg.to_session_name ();
  StreamService::CallType call_type = msg.call_type ();
  std::string reversed = msg.reversed ();
//  printf (
//      "HandleCallReq, from_user_name = %s, to_session_name = %s, call_type = %d reversed = %s \n",
//      from_user_name.c_str (), to_session_name.c_str (), call_type,
//      reversed.c_str ());
  LOGD(
      "HandleCallReq, from_user_name = "<< from_user_name.c_str () << ", to_session_name = " << to_session_name.c_str () << ", " ", call_type = "<< call_type<<", reversed = " << reversed.c_str ());
  CUser * pUser = NULL;
  if ((pUser = CUserManager::Instance ()->GetUser (to_session_name)) != NULL)
    {
      /*对端在线, 转发*/
      LOGD("HandleCallReq forward Success....")
      pUser->GetConnect ()->SendPdu (pPdu);

    }
  else
    {
      /*对端不在线, 通知发起方*/
      LOGD("HandleCallReq forward Failed User not Online....");
      StreamService::CallRes Resmsg;
      Resmsg.set_from_user_name (from_user_name);
      Resmsg.set_to_session_name (to_session_name);
      Resmsg.set_call_type (call_type);
      Resmsg.set_call_result_type (StreamService::CALL_Result_USER_OFFLINE);
      Resmsg.set_reversed ("");

      CSsPdu pdu;
      pdu.SetCommandId (StreamService::CID_CALL_RES_MSG);
      pdu.SetSeqNum (pPdu->GetSeqNum ());
      pdu.SetPBMsg (&Resmsg);
      SendPdu (&pdu);
    }

}

void
TcpConnect::_HandleCallRespond (CSsPdu* pPdu)
{
  StreamService::CallRes msg;
  CHECK_PB_PARSE_MSG(
      msg.ParseFromArray (pPdu->GetBodyData (), pPdu->GetBodyLength ()));

  std::string from_user_name = msg.from_user_name ();
  std::string to_session_name = msg.to_session_name ();
  StreamService::CallType call_type = msg.call_type ();
  std::string reversed = msg.reversed ();
//  printf (
//      "HandleCallRes, from_user_name = %s, to_session_name = %s, call_type = %d reversed = %s \n",
//      from_user_name.c_str (), to_session_name.c_str (), call_type,
//      reversed.c_str ());
  LOGD(
      "HandleCallReq, from_user_name = "<< from_user_name.c_str () << ", to_session_name = " << to_session_name.c_str () << ", " ", call_type = "<< call_type<<", reversed = " << reversed.c_str ());
  CUser * pUser = NULL;
  if ((pUser = CUserManager::Instance ()->GetUser (to_session_name)) != NULL)
    {
      /*对端在线, 转发*/
      pUser->GetConnect ()->SendPdu (pPdu);

    }
  //不在线,不做处理.
}
void
TcpConnect::_HandleHangUpRequest (CSsPdu* pPdu)
{
  StreamService::HangUpReq msg;
  CHECK_PB_PARSE_MSG(
      msg.ParseFromArray (pPdu->GetBodyData (), pPdu->GetBodyLength ()));

  std::string from_user_name = msg.from_user_name ();
  std::string to_session_name = msg.to_session_name ();
  StreamService::CallType call_type = msg.call_type ();
  std::string reversed = msg.reversed ();
//  printf (
//      "HandleHangUpReq, from_user_name = %s, to_session_name = %s, call_type = %d reversed = %s \n",
//      from_user_name.c_str (), to_session_name.c_str (), call_type,
//      reversed.c_str ());
  LOGD(
      "HandleCallReq, from_user_name = "<< from_user_name.c_str () << ", to_session_name = " << to_session_name.c_str () << ", " ", call_type = "<< call_type<<", reversed = " << reversed.c_str ());
  CUser * pUser = NULL;
  if ((pUser = CUserManager::Instance ()->GetUser (to_session_name)) != NULL)
    {
      /*对端在线, 转发*/
      pUser->GetConnect ()->SendPdu (pPdu);

    }

}

bool
TcpConnect::_UserAuthentication (const std::string& username,
				 const std::string& password)
{
  //现在默认返回鉴权成功
  return true;
}

void
TcpConnect::timeout_cb (int fd, short event, void *params)
{
  TcpConnect* pConn = NULL;
  unsigned long long cur_time = get_tick_count ();
  ConnMap_t::iterator old;
  for (ConnMap_t::iterator it = g_conn_map.begin (); it != g_conn_map.end ();)
    {
      old = it;
      it++;
      pConn = (TcpConnect*) old->second;
      pConn->OnTimer (cur_time);
    }
  struct cb_arg *arg = (struct cb_arg*) params;
  struct event *ev = arg->ev;
  struct timeval tv = arg->tv;

  evtimer_add(ev, &tv);

}

void
TcpConnect::OnTimer (unsigned long long cur_time)
{
  //printf("%ld, %ld, %ld \n", cur_time, m_last_recv_tick, cur_time-m_last_recv_tick);
  if (cur_time > m_last_recv_tick + CLIENT_TIMEOUT)
    {
      //printf ("Time out\n");
      LOGD("Time out ");
      Close ();
    }
}

void
TcpConnect::Close (void)
{
  CUser* pUser = NULL;
  if ((pUser = CUserManager::Instance ()->GetUser (m_fd)) != NULL)
    {
      CUserManager::Instance ()->RemoveUser (m_fd);
      CUserManager::Instance ()->RemoveUser (pUser->GetUsername ());
      delete pUser;
    }
  //这将自动close套接字和free读写缓冲区
  bufferevent_free (m_pBuffEvent);
  g_conn_map.erase (m_fd);
  delete this;
}
