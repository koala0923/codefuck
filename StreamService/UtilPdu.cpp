#include "UtilPdu.h"

unsigned long long int
get_tick_count ()
{
  struct timeval tval;
  unsigned long long int ret_tick;

  gettimeofday (&tval, NULL);

  ret_tick = tval.tv_sec * 1000L + tval.tv_usec / 1000L;
  return ret_tick;

}

CByteStream::CByteStream (unsigned char* buf, unsigned int len)
{
  m_pBuf = buf;
  m_len = len;
  m_pos = 0;
}

void
CByteStream::operator >> (char& data)
{
  _ReadByte (&data, 1);
}

void
CByteStream::operator >> (unsigned char& data)
{
  _ReadByte (&data, 1);
}

void
CByteStream::operator >> (short &data)
{
  unsigned char buf[2];

  _ReadByte (buf, 2);

  data = (buf[0] << 8) | buf[1];
}

void
CByteStream::operator >> (unsigned short &data)
{
  unsigned char buf[2];

  _ReadByte (buf, 2);

  data = (buf[0] << 8) | buf[1];
}

void
CByteStream::operator >> (int& data)
{
  unsigned char buf[4];

  _ReadByte (buf, 4);

  data = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

void
CByteStream::operator >> (unsigned int& data)
{
  unsigned char buf[4];

  _ReadByte (buf, 4);

  data = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

void
CByteStream::_ReadByte (void* buf, unsigned int len)
{
  assert(m_len >= m_pos + len);

  memcpy (buf, m_pBuf + m_pos, len);

  m_pos += len;
}

short
CByteStream::ReadInt16 (unsigned char *buf)
{
  short data = (buf[0] << 8) | buf[1];
  return data;
}

unsigned short
CByteStream::ReadUint16 (unsigned char *buf)
{
  unsigned short data = (buf[0] << 8) | buf[1];
  return data;
}

int
CByteStream::ReadInt32 (unsigned char *buf)
{
  int data = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
  return data;
}

unsigned int
CByteStream::ReadUint32 (unsigned char *buf)
{
  unsigned int data = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
  return data;
}

void
CByteStream::WriteInt16 (unsigned char *buf, short data)
{
  buf[0] = static_cast<unsigned char> (data >> 8);
  buf[1] = static_cast<unsigned char> (data & 0xFF);
}

void
CByteStream::WriteUint16 (unsigned char *buf, unsigned short data)
{
  buf[0] = static_cast<unsigned char> (data >> 8);
  buf[1] = static_cast<unsigned char> (data & 0xFF);
}

void
CByteStream::WriteInt32 (unsigned char *buf, int data)
{
  buf[0] = static_cast<unsigned char> (data >> 24);
  buf[1] = static_cast<unsigned char> ((data >> 16) & 0xFF);
  buf[2] = static_cast<unsigned char> ((data >> 8) & 0xFF);
  buf[3] = static_cast<unsigned char> (data & 0xFF);
}

void
CByteStream::WriteUint32 (unsigned char *buf, unsigned int data)
{
  buf[0] = static_cast<unsigned char> (data >> 24);
  buf[1] = static_cast<unsigned char> ((data >> 16) & 0xFF);
  buf[2] = static_cast<unsigned char> ((data >> 8) & 0xFF);
  buf[3] = static_cast<unsigned char> (data & 0xFF);
}
