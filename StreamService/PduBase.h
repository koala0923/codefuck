#ifndef PDUBASE_H_
#define PDUBASE_H_

#include <stdio.h>
#include <bufferevent.h>
#include "event2/buffer.h"

#include "UtilPdu.h"
#include "StreamService.pb.h"
#include "log4z.h"

#define CHECK_PB_PARSE_MSG(ret) { \
    if (ret == false) \
    {\
        printf("parse pb msg failed.");\
		LOGE("parse pb msg failed.");\
        return;\
    }\
}

#define IM_PDU_HEADER_LEN		16
#define IM_PDU_VERSION			1

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

class CSsPdu
{
public:
  CSsPdu ();
  virtual
  ~CSsPdu ();
  void
  Write (bufferevent *bev, unsigned int len);
  int
  ReadPduHeader (unsigned int len);
  void
  WriteHeader ();
  unsigned short int
  GetVersion ()
  {
    return m_pdu_header.version;
  }
  unsigned short int
  GetFlag ()
  {
    return m_pdu_header.flag;
  }
  unsigned short int
  GetServiceId ()
  {
    return m_pdu_header.service_id;
  }
  unsigned short int
  GetCommandId ()
  {
    return m_pdu_header.command_id;
  }
  void
  SetCommandId (unsigned short int commandid)
  {
    m_pdu_header.command_id = commandid;
  }
  unsigned short int
  GetSeqNum ()
  {
    return m_pdu_header.seq_num;
  }
  void
  SetSeqNum (unsigned short int seqnum)
  {
    m_pdu_header.seq_num = seqnum;
  }
  unsigned short int
  GetReversed ()
  {
    return m_pdu_header.reversed;
  }

  unsigned int
  GetBodyLength ()
  {
    return m_buf_length - IM_PDU_HEADER_LEN;
  }
  unsigned char*
  GetBodyData ()
  {
    return m_buf + IM_PDU_HEADER_LEN;
  }
  unsigned int
  GetLength ()
  {
    return m_buf_length;
  }
  unsigned char*
  GetBuffer ()
  {
    return m_buf;
  }
  void
  SetPBMsg (const google::protobuf::MessageLite* msg);
public:

  static CSsPdu*
  ReadPdu (bufferevent *bev, unsigned int& be_pdu_len);
private:
  unsigned char* m_buf;
  unsigned int m_buf_length;
  PduHeader_t m_pdu_header;
};

#endif
