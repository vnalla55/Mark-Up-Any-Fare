
//----------------------------------------------------------------------------
//
//  File:        AppConsoleController.cpp
//
//  Copyright Sabre 2005
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
#include "Server/AppConsoleController.h"

#include "Adapter/Adapter.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/TrxCounter.h"
#include "Common/TseSrvStats.h"
#include "DataModel/BaseTrx.h"
#include "Server/AppConsoleStats.h"
#include "Server/LoadableModule.h"
#include "Server/TseAppConsole.h"
#include "Server/TseServer.h"
#include "Util/FlatSet.h"
#include "Util/StackUtil.h"

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <cerrno>
#include <csignal>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <fcntl.h>
#include <pwd.h>
#include <sys/ipc.h>
#include <sys/resource.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <unistd.h>

using namespace std;

namespace tse
{
namespace
{
ConfigurableValue<bool>
handleSegv("APPLICATION_CONSOLE", "HANDLE_SEGV", true);
ConfigurableValue<bool>
allowCacheToolsCfg("APPLICATION_CONSOLE", "ALLOW_CACHETOOLS", false);
ConfigurableValue<bool>
segvStackTrace("APPLICATION_CONSOLE", "PRINT_SEGV_STACKTRACE", false);
ConfigurableValue<bool>
segvOutputRunningTrx("APPLICATION_CONSOLE", "SEGV_OUTPUT_RUNNING_TRX", false);
ConfigurableValue<uint32_t>
handleSegvMaxTime("APPLICATION_CONSOLE", "HANDLE_SEGV_MAX_TIME", 300);
ConfigurableValue<std::string>
serverTypeCfg("APPLICATION_CONSOLE", "SERVER_TYPE");
ConfigurableValue<int>
portCfg("APPLICATION_CONSOLE", "PORT", 0);
ConfigurableValue<std::string>
redeployFilenameCfg("APPLICATION_CONSOLE", "REDEPLOY_SCRIPT");
ConfigurableValue<std::string>
activateFilenameCfg("APPLICATION_CONSOLE", "ACTIVATE_FILE");
ConfigurableValue<bool>
statsUseSharedMem("APPLICATION_CONSOLE", "USE_SHARED_MEMORY", false);
ConfigurableValue<std::string>
iorFile("APPLICATION_CONSOLE", "IOR_FILE");

const uint32_t DEFAULT_HANDLE_SEGV_MAX_TIME = 300; // seconds

const int FAULT_SIGNALS[] = {SIGSEGV, SIGBUS, SIGILL, SIGFPE};

log4cxx::LoggerPtr _logger = 0;

Logger
loggerSegvTransactions("atseintl.SegvTransactions");

boost::mutex _mutex;
boost::condition_variable _condition;

bool _shutdown = false;

string _host;
string _process;
string _user;
string _build = BUILD_LABEL_STRING;

key_t _statsShmId = 0;
tse::AppConsoleStats* _acStats = nullptr;

sighandler_t prevINT = nullptr;
sighandler_t prevTERM = nullptr;
sighandler_t prevQUIT = nullptr;

tse::TseAppConsole _appConsole;

std::string _redeployFilename;
std::string _activateFilename;

bool _segvStackTrace = false;
bool _segvOutputRunningTrx = false;
bool _terminate = false;
bool _waiting = false;
uint32_t _handleSegvMaxTime = DEFAULT_HANDLE_SEGV_MAX_TIME;

boost::mutex _handleFaultMutex;

void
setFaultSignalsTo(sighandler_t handler)
{
  for (int signum : FAULT_SIGNALS)
  {
    signal(signum, handler);
  }
}

void
handleShutdown(int signum)
{
  AppConsoleController::halt(0, true);
}

void
handleALRM_insideFault(int signum)
{
  abort();
}

const char*
signalName(int signum)
{
  switch (signum)
  {
  case SIGSEGV:
    return "A SEGV";
  case SIGBUS:
    return "A BUS";
  case SIGILL:
    return "An ILL";
  case SIGFPE:
    return "A FPE";
  default:
    return "An unknown";
  }
}

void
handleFault(int signum)
{
  boost::lock_guard<boost::mutex> lock(_handleFaultMutex);

  if (_handleSegvMaxTime > 0)
  {
    // Kill the application after some time in case of a deadlock below.
    signal(SIGALRM, &handleALRM_insideFault);
    alarm(_handleSegvMaxTime);
  }

  ::fprintf(::stderr, "%s signal was caught\n", signalName(signum));
  const tse::DateTime localTime = tse::DateTime::localTime();
  ::fprintf(::stderr, "Current time [%s]\n", localTime.toSimpleString().c_str());
  std::map<std::string, LoadableModule<Adapter>>* adapters = Global::adapters();
  if (adapters != nullptr)
  {
    std::map<std::string, LoadableModule<Adapter>>::iterator i = adapters->begin();
    std::map<std::string, LoadableModule<Adapter>>::iterator j = adapters->end();

    for (; i != j; ++i)
    {
      Adapter* adapter = i->second.instance();
      if (adapter != nullptr)
      {
        ::fprintf(::stderr, "Shutting down adapter %s\n", i->first.c_str());
        adapter->shutdown();
      }
    }
  }

  if (_segvStackTrace)
    StackUtil::outputStackTrace();

  if (_segvOutputRunningTrx)
  {
    boost::lock_guard<boost::mutex> lock(TrxCounter::activeTrxMutex);

    for (BaseTrx* trx : TrxCounter::getActiveTrxs())
    {
      LOG4CXX_INFO(loggerSegvTransactions, trx->rawRequest());
    }
  }

  abort();
}
}

namespace AppConsoleController
{
bool
startup(const string& cfg, TseServer* srv)
{
  if (_logger == nullptr)
    _logger = log4cxx::Logger::getLogger("atseintl.Server.AppConsoleController");

  //_server = srv;

  // Figure out our host and proc variables, there are used in
  // the messaging later
  { // Didnt want this variable hanging around
    char hostname[1024];

    if (::gethostname(hostname, 1023) < 0) // lint !e505 !e506
    {
      //          LOG4CXX_ERROR(_logger, "Unable to retrieve hostname");
      return false;
    }

    _host = hostname; // lint !e603
  }

  {
    char pid[1024];
    pid_t proc = ::getpid();
    snprintf(pid, 1023, "%d", proc);

    _process = pid;
  }

  {
    struct passwd* pw = getpwuid(getuid());
    if (pw == nullptr)
    {
      //          LOG4CXX_ERROR(_logger, "Unable to retrieve username");
      return false;
    }

    _user = pw->pw_name;
  }

  prevINT = signal(SIGINT, handleShutdown);
  prevTERM = signal(SIGTERM, handleShutdown);
  prevQUIT = signal(SIGQUIT, handleShutdown);
  std::string allowCacheTools = "N";
  if (handleSegv.getValue())
    setFaultSignalsTo(&handleFault);

  // Get config value specifying whether or not to print a stacktrace when
  // receiving a SEGV.

  _segvStackTrace = segvStackTrace.getValue();

  _segvOutputRunningTrx = segvOutputRunningTrx.getValue();

  _handleSegvMaxTime = handleSegvMaxTime.getValue();

  TseSrvStats::SERVER_TYPE serverType = TseSrvStats::SERVERTYPE_UNKNOWN;
  std::string st = serverTypeCfg.getValue();

  if (!st.empty())
  {
    char c = st[0];
    switch (c)
    {
    case 'P':
    case 'p':
      serverType = TseSrvStats::SERVERTYPE_PRICING;
      break;

    case 'S':
    case 's':
      serverType = TseSrvStats::SERVERTYPE_SHOPPING;
      break;

    case 'F':
    case 'f':
      serverType = TseSrvStats::SERVERTYPE_FAREDISPLAY;
      break;

    case 'T':
    case 't':
      serverType = TseSrvStats::SERVERTYPE_TAX;
      break;

    case 'H':
    case 'h':
      serverType = TseSrvStats::SERVERTYPE_HISTORICAL;
      break;
    }
  }

  int rcvPort = portCfg.getValue();

  // If we didnt find the serverType in the cfg file, try to deduce it
  // from the port number
  if (serverType == TseSrvStats::SERVERTYPE_UNKNOWN)
  {
    switch (rcvPort)
    {
    case 5000:
      serverType = TseSrvStats::SERVERTYPE_PRICING;
      break;
    case 5001:
      serverType = TseSrvStats::SERVERTYPE_SHOPPING;
      break;

    case 5002:
      serverType = TseSrvStats::SERVERTYPE_FAREDISPLAY;
      break;

    case 5003:
      serverType = TseSrvStats::SERVERTYPE_TAX;
      break;

    case 5004:
      serverType = TseSrvStats::SERVERTYPE_PRICING;
      break;

    case 5006:
      serverType = TseSrvStats::SERVERTYPE_HISTORICAL;
      break;
    }
  }

  {
    std::ostringstream oss;
    oss << rcvPort;
    std::string port = oss.str();

    LOG4CXX_DEBUG(_logger,
                  "Setting serverInfo to instance '" << port << "' and serverType '" << serverType
                                                     << "'");
    TseSrvStats::setServerInfo(port, serverType);
  }

  bool _statsUseSharedMem = statsUseSharedMem.getValue();
  _redeployFilename = redeployFilenameCfg.getValue();
  _activateFilename = activateFilenameCfg.getValue();
  if (_statsUseSharedMem)
  {
    LOG4CXX_DEBUG(_logger, "AppConsole stats using shared memory");
    key_t _statsShmId = shmget(rcvPort, sizeof(AppConsoleStats), 0666 | IPC_CREAT);
    if (_statsShmId < 0)
      return false;

    _acStats = (AppConsoleStats*)shmat(_statsShmId, nullptr, 0);
    if (_acStats == (AppConsoleStats*)-1)
      return false;

    memset(_acStats, 0, sizeof(AppConsoleStats));
    TseSrvStats::_acStats = _acStats;
  }

  if (rcvPort > 0)
  {
    LOG4CXX_INFO(_logger, "AppConsole using port " << rcvPort);

    // Only make a stats if we are going to listen or start shared mem,
    // otherwise they are useless
    if (_acStats == nullptr)
    {
      LOG4CXX_DEBUG(_logger, "AppConsole stats using heap memory");

      _acStats = new AppConsoleStats;
      memset(_acStats, 0, sizeof(AppConsoleStats));
      TseSrvStats::_acStats = _acStats;
    }
    if (!_appConsole.start(rcvPort, srv, allowCacheToolsCfg.getValue()))
    {
      return false;
    }
  }
  else
  {
    LOG4CXX_DEBUG(_logger, "AppConsole socket not started port = '" << rcvPort << "'");
  }

  std::string ior = iorFile.getValue();
  if ((!ior.empty()) && (!_host.empty()))
  {
    std::ofstream os(ior.c_str());
    os << _host << ':' << rcvPort;
    os.close();
  }

  if (_acStats != nullptr)
  {
    struct timeval tv;

    if (gettimeofday(&tv, nullptr) == 0)
      _acStats->startTime = tv.tv_sec;
    else
      _acStats->startTime = 0;
  }

  return true;
}

void
shutdown()
{
  if (_shutdown == true)
    return;

  _shutdown = true;
  _appConsole.stop();

  if (_acStats != nullptr)
  {
    TseSrvStats::_acStats = nullptr;

    if (_statsShmId > 0)
    {
      // Its a shared memory segment
      shmdt(_acStats);
      shmctl(_statsShmId, IPC_RMID, nullptr);
    }
    else
    {
      delete _acStats;
    }

    _acStats = nullptr;
  }

  // Remove our handlers
  signal(SIGINT, prevINT); // lint !e530
  signal(SIGTERM, prevTERM); // lint !e530
  signal(SIGQUIT, prevQUIT); // lint !e530
}

int
run()
{
  boost::unique_lock<boost::mutex> g(_mutex);

  if (!_terminate)
  {
    _waiting = true;

    // Now just sleep until we get a signal or a shutdown.
    _condition.wait(g);

    _waiting = false;
  }

  return EXIT_SUCCESS;
}

bool
halt(const unsigned int delaySeconds, bool ignoreSEGV)
{
  if (delaySeconds > 0)
  {
    signal(SIGALRM, handleShutdown);
    alarm(delaySeconds);
  }
  else
  {
    if (ignoreSEGV)
    {
      if (_logger == nullptr)
        _logger = log4cxx::Logger::getLogger("atseintl.Server.AppConsoleController");

      signal(SIGSEGV, SIG_IGN);
    }

    boost::lock_guard<boost::mutex> g(_mutex);
    _condition.notify_one();
  }

  return true;
}

void
setTerminate()
{
  if (!_terminate)
  {
    _terminate = true;
    if (_waiting)
    {
      halt(0, true);
    }
  }
}

const std::string&
host()
{
  return _host;
}

const std::string&
process()
{
  return _process;
}

const std::string&
user()
{
  return _user;
}

const std::string&
build()
{
  return _build;
}

TseAppConsole&
appConsole()
{
  return _appConsole;
}

const std::string&
redeployFilename()
{
  return _redeployFilename;
}

const std::string&
activateFilename()
{
  return _activateFilename;
}
}
}
