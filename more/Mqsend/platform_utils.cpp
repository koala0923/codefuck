/*
 * platform_utils.cpp
 *
 *  Created on: 2016年3月21日
 *      Author: koala
 */


#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

uint64_t now_microseconds(void)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (uint64_t) tv.tv_sec * 1000000 + (uint64_t) tv.tv_usec;
}

void microsleep(int usec)
{
  struct timespec req;
  req.tv_sec = 0;
  req.tv_nsec = 1000 * usec;
  nanosleep(&req, NULL);
}

