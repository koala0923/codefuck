#ifndef  PROTOL_CODEC_H
#define  PROTOL_CODEC_H

#include <muduo/base/Logging.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/Endian.h>
#include <muduo/net/TcpConnection.h>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>



#include "WorkThreadPool.h"

class ProtolCodec : boost::noncopyable
{
 public:
  enum ErrorCode
    {
      kNoError = 0,
      kInvalidLength,
      kInvalidProgess,
      kUnknownMessageType,
      kParseError,
    };
//  typedef boost::shared_ptr<Message> MessagePtr;

  typedef boost::function<void (const muduo::net::TcpConnectionPtr&,
				MessagePtr& message,
                                  muduo::Timestamp)> YFQAppMessageCallback;

  explicit ProtolCodec(const YFQAppMessageCallback& cb)
    : messageCallback_(cb)
  {

  }

  void onMessage(const muduo::net::TcpConnectionPtr& conn,
                 muduo::net::Buffer* buf,
                 muduo::Timestamp receiveTime);


private:
  PduHeader_t readHead(const char* buf);
  int16_t asInt16(const char* buf);
  int32_t asInt32(const char* buf);
  MessagePtr parse(const muduo::net::TcpConnectionPtr& conn,const char* buf , int len, ErrorCode* error);
 private:

  YFQAppMessageCallback messageCallback_;
   const static int kHeaderLen = 16;   //包头长度
   const static int kMinMessageLen =  2; //默认最小包体长度
   const static int kMaxMessageLen = 64*1024*1024; //最大包长
   const static int kCommidPos = 10;        //命令id 在包中的位置
};




















#endif
