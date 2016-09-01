#ifndef ORACLE_CONNECTION_H
#define ORACLE_CONNECTION_H

#include <iostream>
#include <string>
#include <iomanip>
#define LINUXOCCI //避免函数重定义错误
#include <occi.h>
#include <occiControl.h>




class OracleConnection
{
public:
  virtual
  ~ OracleConnection ();
  static OracleConnection*
  Instance ();
  int
  Init (std::string host, std::string sid,
	std::string username, std::string password, unsigned int maxsize, unsigned int minsize);
  oracle::occi::Connection* GetConnection();
  void ReleaseConnection(oracle::occi::Connection * conn);
protected:
  OracleConnection ();
private:
  static OracleConnection *pool_;
  std::string url_;                  //ip+port
  std::string host_;                 //ip地址
  std::string sid_;                  //sid
  std::string username_;			//用户名
  std::string password_;			//密码
  unsigned int maxsize_;            //最大连接数
  unsigned int minsize_;            //最小连接数

  oracle::occi::ConnectionPool* connPool_;        //连接从池
  oracle::occi::Environment *env_;
};

#endif
