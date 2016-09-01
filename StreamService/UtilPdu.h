#ifndef UTILPDU_H_
#define UTILPDU_H_

#include <assert.h>
#include<string.h>
#include <time.h>
#include <sys/time.h>

unsigned long long int
get_tick_count ();

class CByteStream
{
public:
  CByteStream (unsigned char *buf, unsigned int len);
  ~CByteStream ()
  {
  }
  ;
  unsigned int
  GetPos ()
  {
    return m_pos;
  }
  ;
  unsigned int
  GetLen ()
  {
    return m_len;
  }
  ;
  void
  Skip (unsigned int len)
  {
    m_pos += len;
    assert(m_pos < m_len);
  }

  void
  operator >> (char& data);
  void
  operator >> (unsigned char& data);
  void
  operator >> (short& data);
  void
  operator >> (unsigned short& data);
  void
  operator >> (int& data);
  void
  operator >> (unsigned int& data);

  static short
  ReadInt16 (unsigned char* buf);
  static unsigned short
  ReadUint16 (unsigned char* buf);
  static int
  ReadInt32 (unsigned char* buf);
  static unsigned int
  ReadUint32 (unsigned char* buf);
  static void
  WriteInt16 (unsigned char* buf, short data);
  static void
  WriteUint16 (unsigned char* buf, unsigned short data);
  static void
  WriteInt32 (unsigned char* buf, int data);
  static void
  WriteUint32 (unsigned char* buf, unsigned int data);

private:
  void
  _WriteByte (void* buf, unsigned int len);
  void
  _ReadByte (void* buf, unsigned int len);
private:
  unsigned char* m_pBuf;
  unsigned int m_len;
  unsigned int m_pos;
};
#endif /* UTILPDU_H_ */
