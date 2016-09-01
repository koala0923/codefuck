#ifndef CONNECTION_POOL_H
#define CONNECTION_POOL_H

#include <iostream>
#include <string>
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/exception.h>
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/resultset.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/statement.h>
#include <pthread.h>
#include <list>



class ConnectionPool
{
public:
  virtual
  ~ ConnectionPool ();
  static ConnectionPool*
  Instance ();
  int
  Init (std::string host, std::string port, std::string database,
	std::string username, std::string password, unsigned int maxsize);
  int InitConnection(int iInitialSize);
  sql::Connection * CreateConnection();
  sql::Connection* GetConnection();
  void ReleaseConnection(sql::Connection * conn);
  void DestoryConnPool();
  void DestoryConnection(sql::Connection* conn);
protected:
  ConnectionPool ();
private:
  static ConnectionPool *pool_;
  std::string url_;                  //ip+port
  std::string host_;                 //ip地址
  std::string port_;				//端口
  std::string database_;			//数据库名
  std::string username_;			//用户名
  std::string password_;			//密码
  unsigned int maxsize_;            //最大连接数
  unsigned int cursize_;            //现在连接数
  std::list< sql::Connection* >  connList_;      //连接池的容器队列
  pthread_mutex_t lock_;           //线程锁
  sql::Driver *driver_;
};

#endif
