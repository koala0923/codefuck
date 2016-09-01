#include "ProtolCodec.h"
#include <string>
#include <muduo/net/Endian.h>



void
ProtolCodec::onMessage (const muduo::net::TcpConnectionPtr& conn,
			muduo::net::Buffer* buf, muduo::Timestamp receiveTime)
{

  size_t len = buf->readableBytes ();
  LOG_INFO << conn->name () << " receive " << len << " bytes at "
      << receiveTime.toString ();

  while (buf->readableBytes () >= kHeaderLen)
    {

      const   char* data = buf->peek ();
      unsigned char start_mark = 0;
      unsigned short int dataarea_len = 0;

      ::memcpy (&start_mark, data, sizeof start_mark);
      ::memcpy (&dataarea_len, data + kDataAreaPos, sizeof dataarea_len);  //数据区长度在第11位

      dataarea_len = muduo::net::sockets::networkToHost16 (dataarea_len);

      /*完整的包= 包头 + 包体 + 校验 + 结束标记*/
      if (start_mark != 0x5b || dataarea_len > 32767)
	{

	  LOG_ERROR<< "Invalid length " << dataarea_len << "Or Invalid start_mark" << start_mark;

	  if (conn && conn->connected())
	    {
	      conn->shutdown();
	    }
	  break;

	}
      else if (buf->readableBytes() >= kHeaderLen + dataarea_len + 2)
	{
	  unsigned short pack_len = (unsigned short)kHeaderLen + dataarea_len + 2;
	  unsigned char commid = Message::getCommid(data,pack_len);
	  switch(commid)
	    {
	      case 0x82 : //心跳包
		{
		 // LOG_INFO << "Receive HeartBeat Message commid = " << commid;
//		    Message * pMessage = new HeartBeatMessage(data, pack_len);
		    MessagePtr pMessage(new HeartBeatMessage(data, pack_len));
		    pMessage->readHead();
		    pMessage->readBody();
		    messageCallback_(conn, pMessage, receiveTime);
		    //delete pMessage;
		}
	      break;
	      default:  //其他不认识
		{
		  LOG_INFO << "Receive unknown Message commid = " << commid;
		}
	    }
	  buf->retrieve(pack_len);
	}
      else
	{
	  break;
	}

    }

}

void
ProtolCodec::send (muduo::net::TcpConnection* conn,
		   const muduo::StringPiece& message)
{
//  buf->retrieve(kHeaderLen);
//  	  muduo::string message(buf->peek(), len);
//  	  messageCallback_(conn, message, receiveTime);
//  	  buf->retrieve(len);
}
