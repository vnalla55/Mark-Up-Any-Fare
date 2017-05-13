//----------------------------------------------------------------------------
//
//  File:        main.cpp
//  Description: main function for cachetest
//  Created:     August, 2008
//  Authors:     John Watilo
//
//  Description:
//
//  Return types:
//
//  Copyright Sabre 2008
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#include "Common/Logger.h"
#include "Tools/ldc/CacheTestApp.h"

#include <cstdlib>
#include <iostream>

#include <unistd.h>

using namespace std;

void
syntax()
{
  cout << endl;
  cout << "Usage: cachetest [OPTION]... " << endl;
  cout << endl;
  cout << "-cache <name>  specify cache name" << endl;
  cout << "-host <host>   specify tseserver host, default 'localhost'" << endl;
  cout << "-port <port>   specify tseserver port, default '5001'" << endl;
  cout << "-cmd <command> execute command and exit" << endl;
  cout << "-h/-help       print usage and exit" << endl;
  cout << endl;
}

void
parseArgs(
    int argc, char* argv[], std::string& cachename, std::string& host, int& port, std::string& cmd)
{
  bool reading_cmd(false);
  cmd = "";
  std::string arg;
  for (int a = 0; a < argc; ++a)
  {
    arg = argv[a];

    if (arg == "-cache")
    {
      reading_cmd = false;
      ++a;
      if (a < argc)
      {
        cachename = argv[a];
        cout << "INFO: Cache name [" << cachename << "] set from command line." << endl;
      }
    }
    else if (arg == "-host")
    {
      reading_cmd = false;
      ++a;
      if (a < argc)
      {
        host = argv[a];
        cout << "INFO: Host for tseserver [" << host << "] set from command line." << endl;
      }
    }
    else if (arg == "-port")
    {
      reading_cmd = false;
      ++a;
      if (a < argc)
      {
        port = atoi(argv[a]);
        cout << "INFO: Port for tseserver [" << port << "] set from command line." << endl;
      }
    }
    else if (arg == "-help")
    {
      syntax();
      exit(EXIT_SUCCESS);
    }
    else if (arg == "-h")
    {
      syntax();
      exit(EXIT_SUCCESS);
    }
    else if (arg == "-cmd")
    {
      reading_cmd = true;
      ++a;
      if (a < argc)
      {
        cmd = argv[a];
      }
    }
    else if (a > 0)
    {
      if (reading_cmd)
      {
        cmd.append(" ");
        cmd.append(arg);
      }
      else
      {
        cout << "INFO: Unrecognized cachetest argument [" << arg << "] was ignored." << endl;
        syntax();
        exit(EXIT_FAILURE);
      }
    }
  }

  if (!cmd.empty())
  {
    cout << "INFO: Command for tseserver [" << cmd << "] set from command line." << endl;
  }
}

int
main(int argc, char* argv[])
{
  std::string workingDir;
  std::string host = "localhost";
  int port = 5001;
  std::string cachename;
  std::string cmd;

  // limit scope
  {
    char buff[PATH_MAX + 1];
    if (getcwd(buff, sizeof(buff)) == nullptr)
    {
      cerr << "Unable to get the current working directory" << endl;
      exit(EXIT_FAILURE);
    }
    workingDir = buff;
  }

  // handle command line arguments
  parseArgs(argc, argv, cachename, host, port, cmd);

  // initialize LOG4CXX
  log4cxx::LoggerPtr mainLogger = log4cxx::Logger::getLogger("atseintl.Tools.ldc.main");

  // create application object
  tse::CacheTestApp app(host, port, cachename, workingDir);

  // run the application
  int rc = app.run(cmd);

  // get out of here
  if (cmd.empty())
  {
    std::cout << std::endl << "Bye!" << std::endl;
  }
  exit(rc);
}
