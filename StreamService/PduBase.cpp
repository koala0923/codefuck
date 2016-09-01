#include "PduBase.h"

#include <assert.h>
#include "log4z.h"

CSsPdu::CSsPdu ()
{
  m_buf = NULL;
  m_buf_length = 0;
  m_pdu_header.version = IM_PDU_VERSION;
  m_pdu_header.flag = 1;
  m_pdu_header.service_id = 1;
  m_pdu_header.command_id = 0;
  m_pdu_header.seq_num = 0;
  m_pdu_header.reversed = 0;
}

CSsPdu::~CSsPdu ()
{
  if (m_buf != NULL)
    delete[] m_buf;
  m_buf = NULL;
}

CSsPdu*
CSsPdu::ReadPdu (bufferevent *bev, unsigned int& be_pdu_len)
{
  struct evbuffer* inbuffer = bufferevent_get_input (bev);
  unsigned int inbufferlen = evbuffer_get_length (inbuffer);
  unsigned int pdu_len, cpy_len;
  CSsPdu* pPdu = NULL;
  cpy_len = evbuffer_copyout (inbuffer, (void *) &pdu_len,
			      sizeof(unsigned int));
  pdu_len = ntohl (pdu_len);
//  printf ("ReadPdu cpy_len = %d, pdu_len = %d, inbufferlen = %d\n", cpy_len, pdu_len, inbufferlen);
  LOGD(
      "ReadPdu cpy_len = "<< cpy_len << ", pdu_len = " << pdu_len << ", inbufferlen = " << inbufferlen);
   be_pdu_len = pdu_len;
  if ((cpy_len >= sizeof(unsigned int)) && pdu_len <= inbufferlen)
    {

      pPdu = new CSsPdu ();
      pPdu->Write (bev, pdu_len);
      pPdu->ReadPduHeader (pdu_len);

    }

  return pPdu;
}

void
CSsPdu::Write (bufferevent *bev, unsigned int len)
{
  m_buf = new unsigned char[len + 1];
  assert(m_buf != 0);

  size_t read_len = bufferevent_read (bev, m_buf, len);
  assert(read_len == len);
  m_buf_length = len;
//  printf ("assert copy len = %d, copy package len = %d\n", len, (int) read_len);
  LOGD(
      "assert copy len =  "<< len << ", copy package len =  " << (int) read_len);
}

int
CSsPdu::ReadPduHeader (unsigned int len)
{
  int ret = -1;
  if (len >= IM_PDU_HEADER_LEN && m_buf)
    {
      CByteStream is (m_buf, len);

      is >> m_pdu_header.length;
      is >> m_pdu_header.version;
      is >> m_pdu_header.flag;
      is >> m_pdu_header.service_id;
      is >> m_pdu_header.command_id;
      is >> m_pdu_header.seq_num;
      is >> m_pdu_header.reversed;

      ret = 0;
    }

  return ret;
}

void
CSsPdu::WriteHeader (void)
{
  assert(m_buf != NULL);
  CByteStream::WriteInt32 (m_buf, GetLength ());
  CByteStream::WriteUint16 (m_buf + 4, m_pdu_header.version);
  CByteStream::WriteUint16 (m_buf + 6, m_pdu_header.flag);
  CByteStream::WriteUint16 (m_buf + 8, m_pdu_header.service_id);
  CByteStream::WriteUint16 (m_buf + 10, m_pdu_header.command_id);
  CByteStream::WriteUint16 (m_buf + 12, m_pdu_header.seq_num);
  CByteStream::WriteUint16 (m_buf + 14, m_pdu_header.reversed);
}

void
CSsPdu::SetPBMsg (const google::protobuf::MessageLite* msg)
{
  unsigned int msg_size = msg->ByteSize ();
  m_buf = new unsigned char[IM_PDU_HEADER_LEN + msg_size];
  memset (m_buf, 0, IM_PDU_HEADER_LEN + msg_size);
  assert(m_buf != NULL);
  if (!msg->SerializeToArray (m_buf + IM_PDU_HEADER_LEN, msg_size))
    {
//      printf ("pb msg miss required felds.");
      LOGE("pb msg miss required felds.");
    }
  m_buf_length = IM_PDU_HEADER_LEN + msg_size;
  WriteHeader ();
}

/*
 * struct evbuffer* inbuffer = bufferevent_get_input (bev);
 unsigned int inbufferlen = evbuffer_get_length (inbuffer);
 unsigned int pdu_len , cpy_len;

 cpy_len = evbuffer_copyout(inbuffer, (void *)&pdu_len, sizeof(unsigned int));

 printf("fuck process fuck\n");

 printf("some len %d, %d, %d\n", cpy_len, pdu_len, inbufferlen);

 while((cpy_len >= sizeof(unsigned int)) && pdu_len <= inbufferlen){

 {
 printf("in while process \n");
 printf("%d, %d, %d\n", cpy_len, pdu_len, inbufferlen);
 }

 pdu_len 		= 0;
 cpy_len 		= 0;
 inbufferlen 	= evbuffer_get_length(inbuffer);
 cpy_len 	  	= evbuffer_copyout(inbuffer, (void *)&pdu_len, sizeof(unsigned int));
 }
 *
 *
 *
 * */
