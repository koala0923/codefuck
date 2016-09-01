#include "Message.h"
#include <string.h>
#include <iostream>
#include <stdio.h>
#include<sstream>
#include <muduo/base/Timestamp.h>
#include <muduo/net/protorpc/google-inl.h>

int
Message::readBody (const char *buf, int len)
{
  return 0;
}

int
Message::send (YFQAppService::MessageCmdID commid,
	       const google::protobuf::MessageLite* msg)
{
  muduo::net::Buffer buf;
  assert(buf.readableBytes () == 0);
  int byte_size = msg->ByteSize ();
  buf.ensureWritableBytes (kHeaderLen + byte_size);

  buf.appendInt32 (kHeaderLen + byte_size);
  buf.appendInt16 (m_pdu_header.version);
  buf.appendInt16 (m_pdu_header.flag);
  buf.appendInt16 (m_pdu_header.service_id);
  buf.appendInt16 (commid);
  buf.appendInt16 (m_pdu_header.seq_num);
  buf.appendInt16 (m_pdu_header.reversed);

//    // code copied from MessageLite::SerializeToArray() and MessageLite::SerializePartialToArray().
  GOOGLE_DCHECK(msg->IsInitialized())
					 << InitializationErrorMessage (
					     "serialize", *msg);

  uint8_t* start = reinterpret_cast<uint8_t*> (buf.beginWrite ());
  uint8_t* end = msg->SerializeWithCachedSizesToArray (start);
  if (end - start != byte_size)
    {
      ByteSizeConsistencyError (byte_size, msg->ByteSize (),
				static_cast<int> (end - start));
    }
  buf.hasWritten (byte_size);


  LOG_INFO << "readableBytes = " << buf.readableBytes () << " kHeaderLen = " << kHeaderLen
        << " byte_size = " << byte_size;
  assert(buf.readableBytes () == kHeaderLen + byte_size);

  muduo::net::TcpConnectionPtr conn = m_weak_conn.lock ();
  if (conn)
    {
      conn->send (&buf);
    }
  else
    {
      return -1;
    }

  return 0;
}


int Message::addUser(const std::string& username)
{

  muduo::net::TcpConnectionPtr conn = m_weak_conn.lock ();
    if (conn)
      {
	  CUserPtr ptr_user( new CUser(username));
	  ptr_user->SetConnect (m_weak_conn);
	  muduo::net::TcpConnection *addr = conn.get(); /*获取TcpConnection 的原始地址*/
	  assert(addr != NULL);
	  CUserManager::Instance ()->AddUser (addr, ptr_user);
	  CUserManager::Instance ()->AddUser (username, ptr_user);
      }
    else
      {
        return -1;
      }

  return 0;
}

int Message::removeUser(const std::string& username)
{
  muduo::net::TcpConnectionPtr conn = m_weak_conn.lock ();
      if (conn)
        {
  	   conn->shutdown(); /*在YFQAppServer 状态回调中删除列表中的用户*/
        }
      else
        {
          return -1;
        }
      return 0;
}
int
LoginMessage::process (void)
{

  YFQAppService::LoginResultType result = YFQAppService::LOGIN_RESULT_NONE;
  std::string resultString;
  if (_UserAuthentication ())
    {
      result = YFQAppService::LOGIN_RESULT_OK;
      resultString = "登陆成功";

      addUser( m_user_name);
    }
  else
    {
      result = YFQAppService::LOGIN_RESULT_FAILED;
      resultString = "登陆失败";
    }

  YFQAppService::LoginRes Resmsg;
  Resmsg.set_result_code (result);
  Resmsg.set_result_string (resultString);
  Resmsg.set_reversed ("");
  send (YFQAppService::CID_LOGIN_RES_MSG, &Resmsg);
  return 0;
}

int
LoginMessage::readBody (const char* buf, int len)
{
  YFQAppService::LoginReq msg;
  if (!msg.ParseFromArray (buf, len))
    {
      LOG_ERROR<<"ParseFromArray error For LoginReq";
      return -1;
    }
  m_user_name = msg.user_name ();
  m_password = msg.password ();
  m_reversed = msg.reversed ();
  LOG_INFO << "user_name = " << m_user_name << " m_password = " << m_password
      << " reversed = " << m_reversed;
  return 0;
}

bool
LoginMessage::_UserAuthentication ()
{
  return true;
}

int
HeartBeatMessage::process (void)
{
  YFQAppService::HeartBeatRes Resmsg;
  Resmsg.set_user_name (m_user_name);
  Resmsg.set_msg_id (m_msg_id);
  Resmsg.set_reversed ("");
  send (YFQAppService::CID_HEARTBEAT_RES_MSG, &Resmsg);
  return 0;
}

int
HeartBeatMessage::readBody (const char *buf, int len)
{
  YFQAppService::HeartBeatReq msg;
  if (!msg.ParseFromArray (buf, len))
    {
      LOG_ERROR<<"ParseFromArray error For HeartBeatReq";
      return -1;
    }
  m_user_name = msg.user_name ();
  m_msg_id = msg.msg_id ();
  m_reversed = msg.reversed ();
  LOG_INFO << "user_name = " << m_user_name << " m_msg_id = " << m_msg_id
      << " reversed = " << m_reversed;
  return 0;
}

int
LoginOutMessage::process (void)
{
  /*断开连接，清楚已经连接用户的缓存*/

  return 0;
}

int
LoginOutMessage::readBody (const char *buf, int len)
{
  YFQAppService::LogoutReq msg;
  if (!msg.ParseFromArray (buf, len))
    {
      LOG_ERROR<<"ParseFromArray error For LogoutReq";
      return -1;
    }
  m_user_name = msg.user_name ();
  m_reversed = msg.reversed ();
  LOG_INFO << "user_name = " << m_user_name << " reversed = " << m_reversed;
  return 0;
}

int
GpsMessage::process (void)
{
  YFQAppService::GpsResultType result = YFQAppService::GPS_RESULT_NONE;
    std::string resultString;
    if (GpsOperateDB ())
      {
        result = YFQAppService::GPS_RESULT_OK;
        resultString = "成功上传数据";

      }
    else
      {
        result = YFQAppService::GPS_RESULT_FAILED;
        resultString = "插入数据失败";
      }
    YFQAppService::GpsRes Resmsg;
    Resmsg.set_result_code (result);
    Resmsg.set_result_string (resultString);
    Resmsg.set_reversed ("");
    send (YFQAppService::CID_GPS_RES_MSG, &Resmsg);
  return 0;
}

int
GpsMessage::readBody (const char *buf, int len)
{
  YFQAppService::GpsReq msg;
  if (!msg.ParseFromArray (buf, len))
    {
      LOG_ERROR<<"ParseFromArray error For GpsReq";
      return -1;
    }

  m_user_name = msg.user_name ();
  m_longitude = msg.longitude ();
  m_latitude = msg.latitude ();
  m_address = msg.address ();
  m_date_time = msg.date_time ();
  m_user_id = msg.user_id ();
  m_reversed = msg.reversed ();
  LOG_INFO << "user_name = " << m_user_name << " longitude = " << m_longitude
      << "latitude = " << m_latitude << " address = " << m_address
      << "date_time = " << m_date_time << " user_id = " << m_user_id
      << "reversed = " << m_reversed;
  return 0;
}

bool GpsMessage::GpsOperateDB()
{
  return true;
}
