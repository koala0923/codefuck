#include "ConnectionPool.h"

#include <muduo/base/Logging.h>

ConnectionPool * ConnectionPool::pool_ = NULL;

ConnectionPool::ConnectionPool ()
{
  maxsize_ = 0;
}

ConnectionPool::~ConnectionPool ()
{
  this->DestoryConnPool();
}

ConnectionPool*
ConnectionPool::Instance ()
{
  if (pool_ == NULL)
    {
      pool_ = new ConnectionPool ();
    }
  return pool_;
}

int
ConnectionPool::Init (std::string host, std::string port, std::string database,
		      std::string username, std::string password,
		      unsigned int maxsize)
{
  host_ = host;
  port_ = port;
  database_ = database;
  username_ = username;
  password_ = password;
  maxsize_ = maxsize;

  // url  tcp://127.0.0.1:3306/database_name
  url_ = "tcp://" + host_ + ":" + port_ +"/" + database;

  try
    {
      this->driver_ = sql::mysql::get_driver_instance ();
    }
  catch (sql::SQLException&e)
    {
//      std::cout << "# ERR: " << e.what();
//      std::cout << " (MySQL error code: " << e.getErrorCode();
//      std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
      LOG_ERROR<< "# ERR: " << e.what() << " (MySQL error code: " << e.getErrorCode() << ", SQLState: " << e.getSQLState() << " )";
      return -1;
    }
  catch (std::runtime_error&e)
    {
      // std::cout << "# ERR: " << e.what() << std::endl;
      LOG_ERROR<< "# ERR: " << e.what();
      return -1;
    }

  return this->InitConnection (maxsize_ / 2);

}

//创建连接,返回一个Connection
sql::Connection*
ConnectionPool::CreateConnection ()
{
  sql::Connection* conn;
  try
    {
      conn = driver_->connect (this->url_, this->username_, this->password_); //建立连接
      return conn;
    }
  catch (sql::SQLException&e)
    {
      //perror("创建连接出错");
      LOG_ERROR<< "创建连接出错 # ERR: " << e.what() << " (MySQL error code: " << e.getErrorCode() << ", SQLState: " << e.getSQLState() << " )";
      return NULL;
    }
  catch (std::runtime_error&e)
    {
      //perror("运行时出错");
      LOG_ERROR<< "运行时出错 # ERR: " << e.what();
      return NULL;
    }
}

int
ConnectionPool::InitConnection (int iInitialSize)
{

  sql::Connection*conn;
  pthread_mutex_lock (&lock_);

  for (int i = 0; i < iInitialSize; i++)
    {
      conn = this->CreateConnection ();
      if (conn)
	{
	  connList_.push_back (conn);
	  ++(this->cursize_);
	}
      else
	{

	  LOG_ERROR<< "创建CONNECTION出错";
	  pthread_mutex_unlock(&lock_);
	  return -1;
	}
    }
  pthread_mutex_unlock (&lock_);
  return 0;
}

sql::Connection*
ConnectionPool::GetConnection ()
{
  sql::Connection*con;
  pthread_mutex_lock (&lock_);
  if (connList_.size () > 0)
    {
      con = connList_.front ();  //得到第一个连接
      connList_.pop_front ();   //移除第一个连接
      if (con->isClosed ())
	{   //如果连接已经被关闭，删除后重新建立一个
	  delete con;
	  con = this->CreateConnection ();
	}
      //如果连接为空，则创建连接出错
      if (con == NULL)
	{
	  --cursize_;
	}
      pthread_mutex_unlock (&lock_);
      return con;
    }
  else
    {
      if (cursize_ < maxsize_)
	{ //还可以创建新的连接
	  con = this->CreateConnection ();
	  if (con)
	    {
	      ++cursize_;
	      pthread_mutex_unlock (&lock_);
	      return con;
	    }
	  else
	    {
	      pthread_mutex_unlock (&lock_);
	      return NULL;
	    }
	}
      else
	{ //建立的连接数已经达到maxSize
	  pthread_mutex_unlock (&lock_);
	  return NULL;
	}
    }
}

//回收数据库连接
void ConnectionPool::ReleaseConnection(sql::Connection * conn) {
    if (conn) {
        pthread_mutex_lock(&lock_);
        connList_.push_back(conn);
        pthread_mutex_unlock(&lock_);
    }
}



//销毁连接池,首先要先销毁连接池的中连接
void ConnectionPool::DestoryConnPool() {
    std::list<sql::Connection*>::iterator icon;
    pthread_mutex_lock(&lock_);
    for (icon = connList_.begin(); icon != connList_.end(); ++icon) {
        this->DestoryConnection(*icon); //销毁连接池中的连接
    }
    cursize_ = 0;
    connList_.clear(); //清空连接池中的连接
    pthread_mutex_unlock(&lock_);
}

//销毁一个连接
void ConnectionPool::DestoryConnection(sql::Connection* conn) {
    if (conn) {
        try {
            conn->close();
        } catch (sql::SQLException&e) {
//            perror(e.what());
            LOG_ERROR<< "创建连接出错 # ERR: " << e.what() << " (MySQL error code: " << e.getErrorCode() << ", SQLState: " << e.getSQLState() << " )";
        } catch (std::exception&e) {
//            perror(e.what());
            LOG_ERROR<< "运行时出错 # ERR: " << e.what();
        }
        delete conn;
    }
}

