#ifndef  PROTOL_CODEC_H
#define  PROTOL_CODEC_H

#include <muduo/base/Logging.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/Endian.h>
#include <muduo/net/TcpConnection.h>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include "Message.h"

class ProtolCodec : boost::noncopyable
{
 public:
  typedef boost::shared_ptr<Message> MessagePtr;
//  typedef boost::function<void (const muduo::net::TcpConnectionPtr&,
//                                Message* message,
//                                muduo::Timestamp)> SmartSocketMessageCallback;
  typedef boost::function<void (const muduo::net::TcpConnectionPtr&,
						MessagePtr& message,
                                  muduo::Timestamp)> SmartSocketMessageCallback;

  explicit ProtolCodec(const SmartSocketMessageCallback& cb)
    : messageCallback_(cb)
  {

  }

  void onMessage(const muduo::net::TcpConnectionPtr& conn,
                 muduo::net::Buffer* buf,
                 muduo::Timestamp receiveTime);
  // FIXME: TcpConnectionPtr
  void send(muduo::net::TcpConnection* conn,
            const muduo::StringPiece& message);


 private:
  SmartSocketMessageCallback messageCallback_;
  const static size_t 	kHeaderLen = 14;  						//包头固定长度14字节
  const static size_t   kDataAreaPos = 12;                      //数据区长度在buf中的位置
};




















#endif
