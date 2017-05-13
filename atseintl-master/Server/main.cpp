//----------------------------------------------------------------------------
//
//  File:        main.cpp
//  Description: main function for TseServer
//  Created:     Dec 11, 2003
//  Authors:     Mark Kasprowicz
//
//  Description:
//
//  Return types:
//
//  Copyright Sabre 2003
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
#include "Allocator/TrxMalloc.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/ProgramOptionsParser.h"
#include "Common/TSEException.h"
#include "DBAccess/DiskCache.h"
#include "DBAccess/RemoteCache/RCStartStop.h"
#include "Server/AppConsoleController.h"
#include "Server/TseServer.h"
#include "Util/StackUtil.h"

#include <fstream>
#include <iostream>

#include <boost/filesystem.hpp>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <linux/limits.h>
#include <log4cxx/logmanager.h>
#include <sys/resource.h>
#include <unistd.h>

//------------------------------------------------------------------------------
// Uncomment this block to turn on glibc's malloc checking (mcheck) facility.
//------------------------------------------------------------------------------
#if 0
  // malloc debugging
#include <malloc.h>
#include <mcheck.h>

  extern "C"
  void tseMcheckAbort(enum mcheck_status status)
  {
      abort();
  }

  extern "C"
  void initMcheck()
  {
      // mcheck_pedantic turns on more frequent consistency checks
      // in the mcheck facility.
      //mcheck_pedantic(tseMcheckAbort);
      mcheck(tseMcheckAbort);
  }

  void (*__malloc_initialize_hook) (void) = initMcheck;
#endif
//------------------------------------------------------------------------------

extern int errno;

using namespace std;

//----------------------------------------------------------------------------
// handleTerminate()
//----------------------------------------------------------------------------
extern "C" void
handleTerminate()
{
  cerr << "Error: terminate called" << endl;
  abort();
}

//----------------------------------------------------------------------------
// handleUnexpectedException()
//----------------------------------------------------------------------------
extern "C" void
handleUnexpected()
{
  // @todo When we get a standard TSE Exception,
  //       try to rethrow that in case it can be handled
  cerr << "Error: Unexpected exception" << endl;
  abort();
}

int
main(int argc, char* argv[])
{
  const MallocContextDisabler context;

  //
  // Start time
  //
  time_t timeStart(time(nullptr));

  //
  // Name of sentinel file to be written when the server is ready to
  // take requests
  //
  string readyFile;

  //
  // Server port, to override the configured value
  //
  uint16_t ipport = 0;

  //
  // Application Console port, to override the configured value
  //
  uint16_t acport = 0;

  //
  // BigIP device
  //
  string bigip;

  //
  // BigIP pool name
  //
  string poolname;

  //
  // System type
  //
  string systype;

  //
  // Connection limit, to override the configured value
  //
  uint16_t connlimit = 0;

  //
  // Connection limit basis, to override the configured value
  //
  string connlimitBasis;

  //
  // Application Console group name, to override the configured value
  //
  string groupname;

  //
  // default values
  //
  bool isDaemon = false;
  string config = tse::TseServer::DEFAULT_CFG_FILE;
  vector<string> defines;

  // Get the readyfile from the environment (may be overridden later
  // by command line arguments)
  {
    char* envString = getenv("ATSE_READYFILE");
    if (envString != nullptr)
    {
      readyFile = envString;
    }
  }

  tse::ProgramOptionsParser parser(tse::TseServer::DEFAULT_CFG_FILE);
  parser.parseArguments(argc, argv);
  parser.readVariablesFromArguments(isDaemon,
                                    config,
                                    defines,
                                    readyFile,
                                    ipport,
                                    bigip,
                                    poolname,
                                    systype,
                                    acport,
                                    connlimit,
                                    connlimitBasis,
                                    groupname);

  if (!readyFile.empty())
  {
    if (access(readyFile.c_str(), F_OK) == 0)
    {
      unlink(readyFile.c_str());
    }
  }

  if (acport > 0)
    defines.emplace_back("APPLICATION_CONSOLE.PORT=" + std::to_string(acport));

  if (ipport > 0)
    defines.emplace_back("SERVER_SOCKET_ADP.PORT=" + std::to_string(ipport));

  if (!bigip.empty())
    defines.emplace_back("SERVER_SOCKET_ADP.BIGIP=" + bigip);

  if (!poolname.empty())
    defines.emplace_back("SERVER_SOCKET_ADP.POOLNAME=" + poolname);

  if (!systype.empty())
    defines.emplace_back("SERVER_SOCKET_ADP.SYSTYPE=" + systype);

  if (connlimit > 0)
    defines.emplace_back("SERVER_SOCKET_ADP.CONNLIMIT=" + std::to_string(connlimit));

  if (!connlimitBasis.empty())
    defines.push_back("SERVER_SOCKET_ADP.CONNLIMIT_BASIS=" + connlimitBasis);

  if (!groupname.empty())
    defines.emplace_back("APPLICATION_CONSOLE.GROUPNAME=" + groupname);

  if (isDaemon)
  {
    if (daemon(0, 0) == -1)
    {
      cerr << "Daemon failed - " << strerror(errno) << endl;
      exit(EXIT_FAILURE);
    }
  }

  // Setup handler functions
  set_terminate(handleTerminate);
  set_unexpected(handleUnexpected);

  try
  {
    tse::TseServer srv("TSE_SERVER");
    srv.configName() = config;

    if (!srv.initialize(argc, argv, defines))
    {
      cerr << "Unable to initialize server" << endl;
      return 1;
    }

    // Has to be called after server init since it sets up the logger
    log4cxx::LoggerPtr mainLogger = log4cxx::Logger::getLogger("atseintl.Server.main");

    LOG4CXX_INFO(mainLogger, "Server Created");

    LOG4CXX_INFO(mainLogger, "Starting Application Console");

    if (!tse::AppConsoleController::startup("APPLICATION_CONSOLE", &srv))
    {
      LOG4CXX_ERROR(mainLogger, "Command and Control would not be started, exiting!");
      return 2;
    }

    DISKCACHE.removeObsoleteTableVersions();
    DISKCACHE.waitForQueueToSubside();

    tse::RemoteCache::startRC();

    if (!readyFile.empty())
    {
      boost::filesystem::path p(readyFile);
      std::ofstream f(readyFile.c_str());
      f << "started";
      f.close();
      if (!boost::filesystem::exists(p))
      {
        LOG4CXX_ERROR(mainLogger, "Unable to create readyFile")
        return 5;
      }
    }

    time_t timeUp(time(nullptr));
    time_t startSecs(timeUp - timeStart);
    int st_minutes(startSecs / 60);
    int st_seconds(startSecs % 60);
    char st_display[6];
    sprintf(st_display, "%d:%2.2d", st_minutes, st_seconds);
    LOG4CXX_INFO(mainLogger, "TseServer Time to Start: " << st_display);

    LOG4CXX_INFO(mainLogger, "** TseServer is running **");

    tse::AppConsoleController::run();

    if (!readyFile.empty())
    {
      if (access(readyFile.c_str(), F_OK) == 0)
      {
        unlink(readyFile.c_str());
      }
    }

    LOG4CXX_INFO(mainLogger, "** TseServer is terminating **");

    tse::RemoteCache::stopRC();

    LOG4CXX_INFO(mainLogger, "Stopping Application Console");

    tse::AppConsoleController::shutdown();

    // TseServer shuts down in its destructor here...

    LOG4CXX_INFO(mainLogger, "Destructing Server");
  }
  catch (tse::TSEException& e)
  {
    cerr << "Error tse::Exception caught " << e.what();
    cerr << e.where() << endl;
    return 3;
  }
  catch (...) // everything
  {
    cerr << "Error: Unknown exception" << endl;
    return 4;
  }

  log4cxx::LogManager::shutdown();

  return EXIT_SUCCESS;
}
