#ifndef CONFIG_H_
#define CONFIG_H_

#include<stdio.h>
#include<iostream>
#include<map>
#include<string>

class Config
{
public:
  Config (const char* filename);
  ~Config ();
  char*
  GetConfigName (const char* name);
  int
  SetConfigValue (const char* name, const char* value);
private:
  void
  _ReadFile (const char* filename);
  int
  _WriteFile (const char* filename = NULL);
  void
  _ParseLine (char* line);
  char*
  _TrimSpace (char* name);

  bool m_read_ok;
  std::map<std::string, std::string> m_config_map;
  std::string m_config_file_name;
};

#endif

