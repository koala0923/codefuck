#ifndef MESSAGE_H
#define MESSAGE_H
#include <muduo/base/Logging.h>
#include <muduo/net/Endian.h>
#include <muduo/base/Mutex.h>
#include <map>

class Message
{
public:
  Message (const char *pdata, const unsigned int len) :
      data_ (pdata), len_ (len)
  {

  }
  virtual
  ~Message (void)
  {

  }
  void
  readHead ();
  static unsigned char
  getCommid (const char* pdata, const int len);
  void
  setCommid (const char commid)
  {
    commid_ = commid;
  }
  std::string
  nowTime ();
  virtual void
  readBody ()
  {
    LOG_INFO << "Message read body";
  }
  ;
  virtual int
  process (void)=0;

private:
  unsigned long
  BCDtoDec (const unsigned char *bcd, int length);
  unsigned long
  power (int base, int times);

public:
  const char *data_;                 //数据指针
  unsigned int len_;					//数据长度
  unsigned long deviceAddress_; 		//设备地址


  const static unsigned int kHeaderLen = 14;  			//包头固定长度14字节
  const static unsigned int kSerialNoPos = 1;                     //指令编号在buf中的位置
  const static unsigned int kDeviceAddressPos = 5;                //设备地址在buf中的位置

private:
  unsigned int serialNO_;    			//指令编号
  unsigned char commid_;     			//命令id
};

class HeartBeatMessage : public Message
{
public:
  HeartBeatMessage (const char *pdata, const unsigned int len) :
      Message (pdata, len)
  {

  }
  void
  readBody ();
  virtual
  ~HeartBeatMessage (void)
  {

  }
  int
  process (void);
  int UpdateElectricalAppliance();
private:
  unsigned short identifier_; 	    //标志符
  char socket_statue_;              //当前继电器状态
  double electric_sum_;             //当前总电量
  double voltage_;		            //当前电压
  double electricity_;              //当前电流
  double power_;                    //当前功率

};

typedef   std::map<long, double> SocketPowerMap_t;  //保存上一次的功率值，以判断状态

class SocketStatuManager
{
public:
  ~SocketStatuManager ();
  static SocketStatuManager*
  Instance ();
  void Setpowerthreshold(double power) { powerthreshold_ = power ;}
  void StatuJudge(const long device,const double power, int& statu, bool& change);
protected:
  SocketStatuManager ();
private:
  static SocketStatuManager* m_pSocketStManager;
  SocketPowerMap_t last_powers_;
  double powerthreshold_;        //判断插座上是否有用电器的功率阈值
  mutable muduo::MutexLock mutex_;
};

#endif
