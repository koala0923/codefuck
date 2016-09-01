#include "Message.h"
#include <string.h>
#include <iostream>
#include <stdio.h>
#include<sstream>
#include <muduo/base/Timestamp.h>
#include "ConnectionPool.h"
#include "OracleConnection.h"

#define LINUXOCCI //避免函数重定义错误
#include <occi.h>

void
Message::readHead (void)
{

  ::memcpy (&serialNO_, data_ + kSerialNoPos, sizeof serialNO_);
  serialNO_ = muduo::net::sockets::networkToHost32 (serialNO_);
  char address[6] =
    { };
  ::memcpy (address, data_ + kDeviceAddressPos, 6);
  deviceAddress_ = BCDtoDec ((const unsigned char*) address, 6);
  return;
}

unsigned char
Message::getCommid (const char* pdata, const int len)
{

  unsigned char commid = 0;
  ::memcpy (&commid, pdata + kHeaderLen, sizeof commid);
  return commid;
}

unsigned long
Message::power (int base, int times)
{
  int i;
  unsigned long rslt = 1;

  for (i = 0; i < times; i++)
    rslt *= base;

  return rslt;
}

unsigned long
Message::BCDtoDec (const unsigned char *bcd, int length)
{
  int i, tmp;
  unsigned long dec = 0;

  for (i = 0; i < length; i++)
    {
      tmp = ((bcd[i] >> 4) & 0x0F) * 10 + (bcd[i] & 0x0F);
      dec += tmp * power (100, length - 1 - i);
    }

  return dec;
}

std::string
Message::nowTime ()
{
  muduo::Timestamp now_time = muduo::Timestamp::now ();
  long int microSecondsSinceEpoch = now_time.microSecondsSinceEpoch ();
  char buf[32] =
    { 0 };
  const int kMicroSecondsPerSecond = 1000 * 1000;
  time_t seconds = static_cast<time_t> (microSecondsSinceEpoch
      / kMicroSecondsPerSecond);
  struct tm tm_time;
  gmtime_r (&seconds, &tm_time);

  snprintf (buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d",
	    tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
	    tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);

  return buf;
}

//mysql接口
//int
//HeartBeatMessage::process (void)
//{
//
//  LOG_INFO << "Device Address: " << deviceAddress_ << " Socket statue: "
//      << socket_statue_ << " 总电量 sum electric: " << electric_sum_
//      << " 当前电压 voltage: " << voltage_ << " 当前电流 electricity: " << electricity_
//      << " 当前功率 power: " << power_;
//
//  std::stringstream ss;
//  std::string sql;
//  ss
//      << "insert into YFQ_DATABASE.TBL_SmartSocket( deviceAddress, socketStatue, electricSum, voltage, electricity, power, date) VALUES ("
//      << deviceAddress_ << ", " << socket_statue_ << ", " << electric_sum_
//      << ", " << voltage_ << ", " << electricity_ << "," << power_ << ", "
//      << "'" << nowTime () << "' );";
//  sql = ss.str ();
////  LOG_INFO << "sql :  " << sql;
//
//  sql::Connection *con;
//  sql::Statement *state;
//
//  // 从连接池中获取mysql连接
//  con = ConnectionPool::Instance ()->GetConnection ();
//  if (con == NULL)
//    {
//      LOG_ERROR<< "# ERR:数据库连接池没有可用连接 .";
//      return -1;
//    }
//
//  state = con->createStatement ();
//  assert(state != NULL);
////  state->execute ("use YFQ_DATABASE");
//
//// 插入数据
//
//  try
//    {
//      state->executeUpdate (sql);
//    }
//  catch (sql::SQLException &e)
//    {
//      LOG_ERROR<< "执行sql语句出错 # ERR: " << e.what() << " (MySQL error code: " << e.getErrorCode() << ", SQLState: " << e.getSQLState() << " )";
//      delete state;
//      ConnectionPool::Instance ()->ReleaseConnection (con);
//      return -1;
//    }
//
//  delete state;
//  ConnectionPool::Instance ()->ReleaseConnection (con);
//
//  return UpdateElectricalAppliance ();
//
//}

//oracle 接口
int
HeartBeatMessage::process (void)
{

  LOG_INFO << "Device Address: " << deviceAddress_ << " Socket statue: "
      << socket_statue_ << " 总电量 sum electric: " << electric_sum_
      << " 当前电压 voltage: " << voltage_ << " 当前电流 electricity: " << electricity_
      << " 当前功率 power: " << power_;

  std::stringstream ss;
  std::string sql;
  ss
      << "insert into BPMS_SmartSocket(deviceAddress, cur_socketStatue, cur_electricSum, cur_voltage, cur_electricity, cur_power, upload_date) VALUES ("
      << deviceAddress_ << ", " << socket_statue_ << ", " << electric_sum_
      << ", " << voltage_ << ", " << electricity_ << "," << power_ << ", "
      << "to_date('" << nowTime () << "'," << "'yyyy-mm-dd hh24:mi:ss'))";
  sql = ss.str ();
  LOG_INFO << "sql :  " << sql;

  oracle::occi::Connection *con;
  oracle::occi::Statement *state;

  // 从连接池中获取mysql连接
  con = OracleConnection::Instance ()->GetConnection ();
  if (con == NULL)
    {
      LOG_ERROR<< "# ERR:数据库连接池没有可用连接 .";
      return -1;
    }

//  state->execute ("use YFQ_DATABASE");

// 插入数据

  try
    {
      state = con->createStatement ();
      if (NULL == state)
	{
	  printf ("createStatement error.\n");
	  return -1;
	}
      state->setAutoCommit (TRUE);
      // 设置执行的SQL语句
      state->setSQL (sql);
      // 执行SQL语句
      unsigned int nRet = state->executeUpdate ();
      if (nRet == 0)
	{
	  LOG_ERROR<<"executeUpdate insert error.";
	}

      con->flushCache ();
      con->terminateStatement (state);

      OracleConnection::Instance ()->ReleaseConnection (con);
    }
  catch (oracle::occi::SQLException &sqlExcp)
    {
      LOG_ERROR<< "# ERR: " << sqlExcp.what() << " (Oracle error code: " << sqlExcp.getErrorCode () << ", SQLState: " << sqlExcp.getMessage () << " )";

      return -1;
    }

  return UpdateElectricalAppliance ();

}

//mysql接口
//int
//HeartBeatMessage::UpdateElectricalAppliance ()
//{
//  int statue = 0;
//  bool change = false;
//  SocketStatuManager::Instance ()->StatuJudge (deviceAddress_, power_, statue,
//					       change);
//  if (change == true)
//    {
//      std::stringstream ss;
//      std::string sql;
//      ss
//	  << "insert into YFQ_DATABASE.TBL_SmartSocketEl( deviceAddress, elApplianceSt, date) VALUES ("
//	  << deviceAddress_ << ", " << statue << ", " << "'" << nowTime ()
//	  << "' );";
//      sql = ss.str ();
//      LOG_INFO << "sql :  " << sql;
//
//      sql::Connection *con;
//      sql::Statement *state;
//
//      // 从连接池中获取mysql连接
//      con = ConnectionPool::Instance ()->GetConnection ();
//      if (con == NULL)
//	{
//	  LOG_ERROR<< "# ERR:数据库连接池没有可用连接 .";
//	  return -1;
//	}
//
//      state = con->createStatement ();
//      assert(state != NULL);
//      //  state->execute ("use YFQ_DATABASE");
//
//      // 插入数据
//
//      try
//	{
//	  state->executeUpdate (sql);
//	}
//      catch (sql::SQLException &e)
//	{
//	  LOG_ERROR<< "执行sql语句出错 # ERR: " << e.what() << " (MySQL error code: " << e.getErrorCode() << ", SQLState: " << e.getSQLState() << " )";
//	  delete state;
//	  ConnectionPool::Instance ()->ReleaseConnection (con);
//	  return -1;
//	}
//
//      delete state;
//      ConnectionPool::Instance ()->ReleaseConnection (con);
//    }
//  return 0;
//}

//oracle 接口
int
HeartBeatMessage::UpdateElectricalAppliance ()
{
  int statue = 0;
  bool change = false;
  SocketStatuManager::Instance ()->StatuJudge (deviceAddress_, power_, statue,
					       change);
  if (change == true)
    {
      std::stringstream ss;
      std::string sql;
      ss
	  << "insert into BPMS_SMARTSOCKETEL( deviceAddress, elApplianceSt, changeDate) VALUES ("
	  << deviceAddress_ << ", " << statue << ", " << "to_date('"
	  << nowTime () << "'," << "'yyyy-mm-dd hh24:mi:ss'))";
      sql = ss.str ();
      LOG_INFO << "sql :  " << sql;

      oracle::occi::Connection *con;
      oracle::occi::Statement *state;

      // 从连接池中获取mysql连接
      con = OracleConnection::Instance ()->GetConnection ();
      if (con == NULL)
	{
	  LOG_ERROR<< "# ERR:数据库连接池没有可用连接 .";
	  return -1;
	}

      //  state->execute ("use YFQ_DATABASE");

      // 插入数据

      try
	{
	  state = con->createStatement ();
	  if (NULL == state)
	    {
	      printf ("createStatement error.\n");
	      return -1;
	    }
	  state->setAutoCommit (TRUE);
	  // 设置执行的SQL语句
	  state->setSQL (sql);
	  // 执行SQL语句
	  unsigned int nRet = state->executeUpdate ();
	  if (nRet == 0)
	    {
	      LOG_ERROR<<"executeUpdate insert error.";
	    }

	  con->flushCache ();
	  con->terminateStatement (state);

	  OracleConnection::Instance ()->ReleaseConnection (con);
	}
      catch (oracle::occi::SQLException &sqlExcp)
	{

	  LOG_ERROR<< "# ERR: " << sqlExcp.what() << " (Oracle error code: " << sqlExcp.getErrorCode () << ", SQLState: " << sqlExcp.getMessage () << " )";

	  return -1;
	}

      return 0;

    }

  return -1;
}
void
HeartBeatMessage::readBody (void)
{
  //LOG_INFO << "HeartBeatMessage read body";

  ::memcpy (&identifier_, data_ + kHeaderLen + 1, sizeof identifier_);
  identifier_ = muduo::net::sockets::networkToHost16 (identifier_);

  unsigned int socket_statue_pos = kHeaderLen + 8;
  unsigned int electric_sum_pos = kHeaderLen + 10;
  unsigned int voltage_pos = kHeaderLen + 15;
  unsigned int electricity_pos = kHeaderLen + 19;
  unsigned int powepos = kHeaderLen + 23;

  char buf[64] =
    { };
  ::memcpy (&socket_statue_, data_ + socket_statue_pos, sizeof socket_statue_);

  sprintf (buf, "%x%x%x.%x", data_[electric_sum_pos],
	   data_[electric_sum_pos + 1], data_[electric_sum_pos + 2],
	   data_[electric_sum_pos + 3]);
  electric_sum_ = atof (buf);

  memset (buf, 0, 64);
  sprintf (buf, "%x%x.%x", data_[voltage_pos], data_[voltage_pos + 1],
	   data_[voltage_pos + 2]);
  voltage_ = atof (buf);

  memset (buf, 0, 64);
  sprintf (buf, "%x.%x%x", data_[electricity_pos], data_[electricity_pos + 1],
	   data_[electricity_pos + 2]);
  electricity_ = atof (buf);

  memset (buf, 0, 64);
  sprintf (buf, "%x%x.%x", data_[powepos], data_[powepos + 1],
	   data_[powepos + 2]);
  power_ = atof (buf);

}

SocketStatuManager *SocketStatuManager::m_pSocketStManager = NULL;

SocketStatuManager::SocketStatuManager ()
{

}

SocketStatuManager::~SocketStatuManager ()
{

}

SocketStatuManager *
SocketStatuManager::Instance (void)
{
  if (m_pSocketStManager == NULL)
    {
      m_pSocketStManager = new SocketStatuManager ();
    }
  return m_pSocketStManager;
}

void
SocketStatuManager::StatuJudge (const long device, const double power,
				int& statue, bool& change)
{
  muduo::MutexLockGuard lock (mutex_);
  SocketPowerMap_t::iterator it = last_powers_.find (device);
  if (it != last_powers_.end ())
    {
      double lastTime = it->second;
      LOG_INFO << "lastTime power = " << lastTime << " " << "now power = "
	  << power;
      if (lastTime <= 0.5 && power <= 0.5)
	{
	  //这一次和上一次都小于最小功率，状态不变
	  statue = 0;
	  change = false;
	}
      else if (lastTime <= 0.5 && power >= powerthreshold_)
	{
	  //上一次小于最小功率，这一次大于阈值，状态改变
	  statue = 1;
	  change = true;
	}
      else if ((lastTime >= 0.4 && lastTime < powerthreshold_)
	  && power < powerthreshold_)
	{
	  //上一大于最小功率并小于阈值，这一次小于阈值，状态不变
	  statue = 0;
	  change = false;
	}
      else if ((lastTime >= 0.4 && lastTime < powerthreshold_)
	  && power >= powerthreshold_)
	{
	  //上一大于最小功率并小于阈值，这一次大于阈值，状态改变
	  statue = 1;
	  change = true;
	}
      else if (lastTime >= powerthreshold_ && power < powerthreshold_)
	{
	  //上一次功率大于阈值，这一次功率小于阈值，状态改变
	  statue = 0;
	  change = true;
	}
      else if (lastTime >= powerthreshold_ && power >= powerthreshold_)
	{
	  //上一次功率大于阈值，这一次功率大于阈值，状态不变
	  statue = 1;
	  change = false;
	}
    }
  else
    {
      if (power < powerthreshold_)
	{
	  statue = 0;
	  change = true;
	}
      else if (power >= powerthreshold_)
	{
	  statue = 1;
	  change = true;
	}
    }
  last_powers_[device] = power;
}

