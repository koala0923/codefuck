#include "OracleConnection.h"
#include <muduo/base/Logging.h>



OracleConnection * OracleConnection::pool_ = NULL;

OracleConnection::OracleConnection ()
{

}

OracleConnection::~OracleConnection ()
{
  //释放连接
  env_->terminateConnectionPool (connPool_);
  //env->terminateConnection(conn);

  oracle::occi::Environment::terminateEnvironment (env_);
}

OracleConnection*
OracleConnection::Instance ()
{
  if (pool_ == NULL)
    {
      pool_ = new OracleConnection ();
    }
  return pool_;
}

int
OracleConnection::Init (std::string host, std::string sid, std::string username,
			std::string password, unsigned int maxsize,
			unsigned int minsize)
{
  host_ = host;
  sid_ = sid;
  username_ = username;
  password_ = password;
  maxsize_ = maxsize;
  minsize_ = minsize;
  url_ = host + "/" + sid;

	      //oracle::occi::Environment::createEnvironment ("ZHS16GBK", "UTF8");
  env_ = oracle::occi::Environment::createEnvironment (oracle::occi::Environment::OBJECT);

  try
    {
      connPool_ = env_->createConnectionPool (username_, password_, url_,
					      minsize_, maxsize_, 2);

    }
  catch (oracle::occi::SQLException &sqlExcp)
    {

      LOG_ERROR<< "# ERR: " << sqlExcp.what() << " (Oracle error code: " << sqlExcp.getErrorCode () << ", SQLState: " << sqlExcp.getMessage () << " )";
      return -1;
    }
  return 0;
}

oracle::occi::Connection*
OracleConnection::GetConnection ()
{

  try
    {
      //从连接池获取连接
      oracle::occi::Connection *conn;
      conn = connPool_->createConnection (username_, password_);
      return conn;
    }
  catch (oracle::occi::SQLException &sqlExcp)
    {

      LOG_ERROR<< "# ERR: " << sqlExcp.what() << " (Oracle error code: " << sqlExcp.getErrorCode () << ", SQLState: " << sqlExcp.getMessage () << " )";
      return NULL;
    }

}


//回收数据库连接
void OracleConnection::ReleaseConnection(oracle::occi::Connection * conn) {
  try
      {
      connPool_->terminateConnection (conn);
      }
    catch (oracle::occi::SQLException &sqlExcp)
      {

        LOG_ERROR<< "# ERR: " << sqlExcp.what() << " (Oracle error code: " << sqlExcp.getErrorCode () << ", SQLState: " << sqlExcp.getMessage () << " )";

      }
}
