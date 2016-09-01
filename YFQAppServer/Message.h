#ifndef MESSAGE_H
#define MESSAGE_H
#include <muduo/base/Logging.h>
#include <muduo/net/Endian.h>
#include <muduo/base/Mutex.h>
//#include <muduo/net/TcpConnection.h>
#include <map>
//#include <boost/shared_ptr.hpp>

#include "YFQAppService.pb.h"
#include "YfqUser.h"

typedef struct
{
  unsigned int length;		  	// 整个包的长度
  unsigned short int version;	   	// 版本号
  unsigned short int flag;		  	// not used
  unsigned short int service_id;	  	// 服务器id
  unsigned short int command_id;	  	// 命令id
  unsigned short int seq_num;     	// 包序号
  unsigned short int reversed;    	// 保留

} PduHeader_t;

//typedef boost::weak_ptr<muduo::net::TcpConnection> WeakTcpConnectionPtr;

class Message
{
public:
  Message ()
  {

  }
  virtual
  ~Message (void)
  {

  }
  void
  getHead (PduHeader_t head)
  {
    m_pdu_header = head;
  }
  void setContext(WeakTcpConnectionPtr conn, muduo::Timestamp time)
  {
    m_weak_conn = conn,
	m_time =  time;
  }

  int send(YFQAppService::MessageCmdID ,const google::protobuf::MessageLite* msg);
  virtual int
  readBody (const char* buf, int len);

  virtual int
  process (void)=0;

  int addUser(const std::string& username);
  int removeUser(const std::string& username);
private:
  PduHeader_t m_pdu_header;
  WeakTcpConnectionPtr m_weak_conn;
  muduo::Timestamp m_time;
  const static int kHeaderLen = 16;   //包头长度
};

typedef boost::shared_ptr<Message> MessagePtr;

class LoginMessage : public Message
{
public:
  LoginMessage ()
  {
    m_user_name = "";
    m_password = "";
    m_reversed = "";
  }
  ~LoginMessage ()
  {

  }
  int
  readBody (const char* buf, int len);
  int
  process ();
  bool _UserAuthentication();
private:
  std::string m_user_name;   //用户名
  std::string m_password;   //密码
  std::string m_reversed;   //保留字段
};

class HeartBeatMessage : public Message
{
public:
  HeartBeatMessage ()
  {
    m_user_name = "";
    m_msg_id = 0;
    m_reversed = "";
  }

  ~HeartBeatMessage ()
  {

  }
  int
  readBody (const char* buf, int len);
  int
  process ();
  bool _UserAuthentication();
private:
  std::string m_user_name;  //用户名
  uint32_t m_msg_id;		//消息id
  std::string m_reversed;   //保留字段

};

class LoginOutMessage : public Message
{
public:
  LoginOutMessage ()
  {
    m_user_name = "";

    m_reversed = "";
  }

  ~LoginOutMessage ()
  {

  }

  int
  readBody (const char* buf, int len);
  int
  process ();
private:
  std::string m_user_name;
  std::string m_reversed;
};

class GpsMessage : public Message
{
public:
  GpsMessage ()
  {
    m_user_name = "";
    m_longitude = 0.0;
    m_latitude = 0.0;
    m_address = "中国.深圳.";
    m_date_time = "1970-01-01 00:00:00";
    m_user_id = "";
    m_reversed = "";
  }
  ~GpsMessage ()
  {

  }
  int
  readBody (const char* buf, int len);
  int
  process ();
 bool GpsOperateDB();
private:
  std::string m_user_name;        //用户名
  double m_longitude;       		//经度,double类型
  double m_latitude;        		//纬度,double类型
  std::string m_address;         	//老人所在地址
  std::string m_date_time;        //时间，格式：YYYY-MM-DD HH:mm:ss
  std::string m_user_id;         //老人用户ID
  std::string m_reversed;  	    //保留字段
};

#endif
