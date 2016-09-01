#include "ProtolCodec.h"
#include <string>
#include <muduo/net/Endian.h>

#include<boost/implicit_cast.hpp>

void
ProtolCodec::onMessage (const muduo::net::TcpConnectionPtr& conn,
			muduo::net::Buffer* buf, muduo::Timestamp receiveTime)
{

  LOG_INFO << "Rececie message!!! buf len = " << buf->readableBytes ();
  while (buf->readableBytes () >= kHeaderLen)
    {
      const int32_t len = buf->peekInt32 ();
      LOG_INFO << "package len " << len;

      if (len > kMaxMessageLen || len <= kHeaderLen + kMinMessageLen)
	{
	  if (conn && conn->connected ())
	    {
	      conn->shutdown ();
	    }
	  break;
	}
      else if (buf->readableBytes () >= boost::implicit_cast<size_t> (len))
	{
	  ErrorCode errorCode = kNoError;
	  MessagePtr message = parse (conn, buf->peek (), len, &errorCode);

	  if (errorCode == kNoError && message)
	    {
	      messageCallback_ (conn, message, receiveTime);
	      buf->retrieve (len);
	    }
	  else
	    {
	      if (conn && conn->connected ())
		{
		  conn->shutdown ();
		}
	      break;
	    }
	}
      else
	{
	  break;
	}
    }
}

int16_t
ProtolCodec::asInt16 (const char* buf)
{
  int16_t be16 = 0;
  ::memcpy (&be16, buf, sizeof(be16));
  return muduo::net::sockets::networkToHost16 (be16);
}

int32_t
ProtolCodec::asInt32 (const char* buf)
{
  int32_t be32 = 0;
  ::memcpy (&be32, buf, sizeof(be32));
  return muduo::net::sockets::networkToHost32 (be32);
}

PduHeader_t
ProtolCodec::readHead (const char *buf)
{
  PduHeader_t head;
  head.length = asInt32 (buf);
  head.version = asInt16 (buf + sizeof(int32_t));
  head.flag = asInt16 (buf + sizeof(int32_t) + sizeof(int16_t));
  head.service_id = asInt16 (buf + sizeof(int32_t) + sizeof(int16_t) * 2 );
  head.command_id = asInt16 (buf + sizeof(int32_t) + sizeof(int16_t) * 3);
  head.seq_num = asInt16 (buf + sizeof(int32_t) + sizeof(int16_t) * 4);
  head.reversed = asInt16 (buf + sizeof(int32_t) + sizeof(int16_t) * 5);
  return head;
}

MessagePtr
ProtolCodec::parse (const muduo::net::TcpConnectionPtr& conn, const char* buf, int len, ErrorCode* error)
{
  MessagePtr message;
  PduHeader_t head = readHead (buf);

  LOG_INFO << "length = " << head.length<<
      " version = " << head.version<<
      " flag = " << head.flag<<
      " service_id = " << head.service_id<<
      " command_id = " << head.command_id<<
      " seq_num = " << head.seq_num<<
      " reversed = " << head.reversed;


  *error = kNoError;
  if (head.command_id != YFQAppService::CID_LOGIN_REQ_MSG)
    {
      /*不是登陆包,看看这个连接有没有登陆*/
      muduo::net::TcpConnection *addr = conn.get(); /*获取TcpConnectionPtr的原始地址*/
	  assert(addr != NULL);
	  CUserPtr  user;
      if (!(user = CUserManager::Instance ()->GetUser (addr)))
	{
	  *error = kInvalidProgess;               /*非法的协议交互流程*/
	  LOG_INFO << "Not Login ,But Receive Message commid = " << head.command_id;
	  return message;
	}
    }

  switch (head.command_id)
    {
    case YFQAppService::CID_LOGIN_REQ_MSG: //登陆包
      {
	message.reset (new LoginMessage ());
	message->getHead (head);
	if (message->readBody (buf + kHeaderLen, len - kHeaderLen) < 0)
	  {
	    *error = kParseError;
	  }

      }
      break;
    case YFQAppService::CID_HEARTBEAT_REQ_MSG: //心跳包
      {
	message.reset (new HeartBeatMessage ());
	message->getHead (head);
	if (message->readBody (buf + kHeaderLen, len - kHeaderLen) < 0)
	  {
	    *error = kParseError;
	  }
      }
      break;
    case YFQAppService::CID_LOGINOUT_REQ_MSG: //登出包
      {
	message.reset (new LoginOutMessage ());
	message->getHead (head);
	if (message->readBody (buf + kHeaderLen, len - kHeaderLen) < 0)
	  {
	    *error = kParseError;
	  }
      }
      break;
    case YFQAppService::CID_GPS_REQ_MSG: //App上传gps数据包
      {
	message.reset (new GpsMessage ());
	message->getHead (head);
	if (message->readBody (buf + kHeaderLen, len - kHeaderLen) < 0)
	  {
	    *error = kParseError;
	  }
      }
      break;
    default:  //其他不认识
      {
	*error = kUnknownMessageType;
	LOG_INFO << "Receive unknown Message commid = " << head.command_id;
      }
    }

  return message;
}


