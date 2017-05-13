#ifndef CommandLine_H
#define CommandLine_H

#include <string>
#include <vector>

struct ArgProc
{
  bool required;
  char opt;
  std::string & data;
  const char * description;
};

class CommandLine
{
public:

  CommandLine(int argc_, char ** argv_, ArgProc * argProcInfo, size_t argProcInfoLen_ );

  void usage();

  std::vector<std::string> & getFreeArgs() { return unqualified; }

private:
  int argc;
  char ** argv;
  ArgProc * argProcInfo;
  size_t argProcInfoLen;
  std::vector<std::string> unqualified;
};

#endif
