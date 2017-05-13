//----------------------------------------------------------------------------
//
//  File:        CacheTestApp.cpp
//  Description: Cache Test application class implementation
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

#include "Tools/ldc/CacheTestApp.h"

#include "Adapter/CacheNotifyControl.h"
#include "Common/Logger.h"
#include "IDOPMacros.h" // IDOP
#include "IDOPPrompter.h" // IDOP
#include "IDOPSocket.h" // IDOP
#include "IDOPXMLInitiator.h" // IDOP

#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <boost/tokenizer.hpp>
#include <fcntl.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>
#include <sys/resource.h>
#include <unistd.h>

#define STR_TOUPPER(a) std::transform(a.begin(), a.end(), a.begin(), (int (*)(int))std::toupper);
#define STR_TOLOWER(a) std::transform(a.begin(), a.end(), a.begin(), (int (*)(int))std::tolower);

#define SAY(cacheId, message)                                                                      \
  {                                                                                                \
    if (std::string(cacheId) != "")                                                                \
    {                                                                                              \
      cout << "[" << cacheId << "] ";                                                              \
    }                                                                                              \
    cout << message << endl;                                                                       \
  }

using namespace std;

namespace tse
{
static const char* CMD_CACHE_FLUSH = "FLSH";
static const char* CMD_STOP = "DOWN";
static const char* CMD_STATS = "RFSH";
static const char* CMD_MEMKEYS = "MKEY";
static const char* CMD_MEMKEYSANDVALUES = "MKVL";
static const char* CMD_DISKKEYS = "DKEY";
static const char* CMD_LDCTYPES = "LDCT";
static const char* CMD_LDC_COMPARE_VALUES = "LDCC";
static const char* CMD_INVALIDATE_KEY = "INVK";
static const char* CMD_INSERT_DUMMY_OBJECT = "INSD";
static const char* CMD_INJECT_CACHE_NOTIFY = "INJC";
static const char* CMD_OBJECT_EXISTS = "EXIS";
static const char* CMD_CACHE_PARM = "CPRM";
static const char* CMD_QUERY_OBJECT = "QOBJ";
static const char* CMD_GET_NATION_LOADLIST = "GNLL";
static const char* CMD_LDC_COMP_DB_VALUES = "LDCD";
static const char* CMD_LDC_STATS = "LATS";
static const char* CMD_BDB_PANIC = "BDBP";

typedef std::vector<std::string> STRINGVEC;
typedef std::map<std::string, STRINGVEC> CACHECTRLMAP;

struct CacheData
{
  CacheData() : _ldcEnabled(false), _count(0) {}

  std::string _name;
  std::string _type;
  bool _ldcEnabled;
  uint64_t _count;
};

typedef std::map<std::string, CacheData, std::less<std::string> > CACHEDATAMAP;
size_t
longestCacheName(0);
size_t
longestCacheType(0);

CACHEDATAMAP cdMap;

log4cxx::LoggerPtr
CacheTestApp::_logger(log4cxx::Logger::getLogger("atseintl.Tools.ldc.CacheTestApp"));

CACHECTRLMAP keyFields;
CACHECTRLMAP cacheIds;
CACHECTRLMAP historicalKeys;
CACHECTRLMAP historicalIds;

void
addMatchingNotifications(const std::string& cacheId,
                         const CACHECTRLMAP& ccm,
                         std::vector<std::string>& notifications)
{
  for (CACHECTRLMAP::const_iterator i = ccm.begin(); i != ccm.end(); ++i)
  {
    const std::string& notification((*i).first);
    const STRINGVEC& tables((*i).second);
    for (STRINGVEC::const_iterator j = tables.begin(); j != tables.end(); ++j)
    {
      const std::string& table((*j));
      if (table == cacheId)
      {
        notifications.push_back(notification);
      }
    }
  }
}

void
printCacheCtrlMap(const std::string& name, const CACHECTRLMAP& ccm)
{
  SAY("", "Cache Control Map [" << name << "]:");

  for (CACHECTRLMAP::const_iterator i = ccm.begin(); i != ccm.end(); ++i)
  {
    const std::string& key((*i).first);
    const STRINGVEC& vec((*i).second);

    SAY("", "  KEY = [" << key << "]");
    for (STRINGVEC::const_iterator j = vec.begin(); j != vec.end(); ++j)
    {
      const std::string& item((*j));
      SAY("", "    ITEM = [" << item << "]");
    }
  }
}

CacheTestApp::CacheTestApp(const std::string& host,
                           int port,
                           const std::string& cacheName,
                           const std::string& workingDir)
  : _host(host), _port(port), _cacheName(cacheName), _workingDir(workingDir), _showMenu(true)
{
  log4cxx::BasicConfigurator::configure();
  log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getError());

  std::string cacheNotifyXML("cacheNotify.xml");
  tse::CacheNotifyControl cacheControl(
      cacheNotifyXML, keyFields, cacheIds, historicalKeys, historicalIds);
  cacheControl.parse();
}

CacheTestApp::~CacheTestApp()
{
  log4cxx::BasicConfigurator::resetConfiguration();
}

int
CacheTestApp::run(const std::string& cmd)
{
  int rc = EXIT_SUCCESS;
  sleep(1);
  return mainMenu(cmd);
}

int
CacheTestApp::processCommand(const std::string& response,
                             bool& notExiting,
                             bool allowPrompt,
                             bool interactive)
{
  int rc = EXIT_SUCCESS;
  std::string cmd;
  STRINGVEC cmdArgs;

  boost::char_separator<char> sep(" ", "", boost::keep_empty_tokens);
  boost::tokenizer<boost::char_separator<char> > toks(response, sep);
  for (boost::tokenizer<boost::char_separator<char> >::const_iterator t = toks.begin();
       t != toks.end();
       ++t)
  {
    if (t == toks.begin())
    {
      cmd = (*t);
    }
    else
    {
      cmdArgs.push_back((*t));
    }
  }

  // COMMAND: allnations
  if (strcasecmp(cmd.c_str(), "allnations") == 0)
  {
    nationLoadList();
  }

  // COMMAND: cache || type
  else if ((strcasecmp(cmd.c_str(), "cache") == 0) || (strcasecmp(cmd.c_str(), "type") == 0))
  {
    if (allowPrompt)
    {
      std::string arg(cmdArgs.size() > 0 ? cmdArgs[0] : "");
      promptCacheName(arg, interactive);
    }
    else
    {
      _cacheName = (cmdArgs.size() > 0 ? cmdArgs[0] : "");
    }
  }

  // COMMAND: cache_exact
  else if (strcasecmp(cmd.c_str(), "cache_exact") == 0)
  {
    _cacheName = (cmdArgs.size() > 0 ? cmdArgs[0] : "");
  }

  // COMMAND: compdbvals
  else if (strcasecmp(cmd.c_str(), "compdbvals") == 0)
  {
    Scope scope(SCOPE_SINGLE);

    for (STRINGVEC::iterator it = cmdArgs.begin(); it != cmdArgs.end(); ++it)
    {
      if (strcasecmp((*it).c_str(), "all") == 0)
      {
        scope = SCOPE_ALL_LDC;
      }
      else if (strcasecmp((*it).c_str(), "w") == 0)
      {
        scope = SCOPE_WORLD;
      }
    }

    compareDbValues(scope);
  }

  // COMMAND: compkeys
  else if (strcasecmp(cmd.c_str(), "compkeys") == 0)
  {
    Scope scope(SCOPE_SINGLE);
    for (STRINGVEC::iterator it = cmdArgs.begin(); it != cmdArgs.end(); ++it)
    {
      if (strcasecmp((*it).c_str(), "all") == 0)
      {
        scope = SCOPE_ALL_LDC;
      }
    }
    compareKeys(scope);
  }

  // COMMAND: compvals
  else if (strcasecmp(cmd.c_str(), "compvals") == 0)
  {
    Scope scope(SCOPE_SINGLE);
    for (STRINGVEC::iterator it = cmdArgs.begin(); it != cmdArgs.end(); ++it)
    {
      if (strcasecmp((*it).c_str(), "all") == 0)
      {
        scope = SCOPE_ALL_LDC;
      }
    }
    compareValues(scope);
  }

  // COMMAND: diskkeys
  else if (strcasecmp(cmd.c_str(), "diskkeys") == 0)
  {
    Scope scope(SCOPE_SINGLE);
    bool inclDates(false);

    for (STRINGVEC::iterator it = cmdArgs.begin(); it != cmdArgs.end(); ++it)
    {
      if (strcasecmp((*it).c_str(), "all") == 0)
      {
        scope = SCOPE_ALL_LDC;
      }
      else if (strcasecmp((*it).c_str(), "d") == 0)
      {
        inclDates = true;
      }
    }
    diskkeysTseServer(NULL, scope, inclDates);
  }

  // COMMAND: dummy
  else if (strcasecmp(cmd.c_str(), "dummy") == 0)
  {
    Scope scope(SCOPE_SINGLE);
    for (STRINGVEC::iterator it = cmdArgs.begin(); it != cmdArgs.end(); ++it)
    {
      if (strcasecmp((*it).c_str(), "all") == 0)
      {
        scope = SCOPE_ALL_LDC;
      }
      else if (strcasecmp((*it).c_str(), "w") == 0)
      {
        scope = SCOPE_WORLD;
      }
    }
    insertDummyObject(scope);
  }

  // COMMAND: exists
  else if (strcasecmp(cmd.c_str(), "exists") == 0)
  {
    if (cmdArgs.size() > 0)
    {
      objectExists(*(cmdArgs.begin()));
    }
    else
    {
      objectExists("");
    }
  }

  // COMMAND: exit || quit
  else if ((strcasecmp(cmd.c_str(), "exit") == 0) || (strcasecmp(cmd.c_str(), "quit") == 0))
  {
    notExiting = false;
  }

  // COMMAND: flush
  else if (strcasecmp(cmd.c_str(), "flush") == 0)
  {
    Scope scope(SCOPE_SINGLE);
    for (STRINGVEC::iterator it = cmdArgs.begin(); it != cmdArgs.end(); ++it)
    {
      if (strcasecmp((*it).c_str(), "all") == 0)
      {
        scope = SCOPE_ALL_LDC;
      }
      else if (strcasecmp((*it).c_str(), "w") == 0)
      {
        scope = SCOPE_WORLD;
      }
    }
    flushCache(scope);
  }

  // COMMAND: getdb || getlddc || getmem
  else if ((strcasecmp(cmd.c_str(), "getdb") == 0) || (strcasecmp(cmd.c_str(), "getldc") == 0) ||
           (strcasecmp(cmd.c_str(), "getmem") == 0))
  {
    STR_TOLOWER(cmd);
    std::string arg1(cmdArgs.size() > 0 ? cmdArgs[0] : "");
    STR_TOLOWER(arg1);
    std::string arg2(cmdArgs.size() > 1 ? cmdArgs[1] : "");
    if ((arg1 != "xml") && (arg1 != "text") && (arg1 != "flat"))
    {
      SAY("", "SORRY! Invalid format <f> specified.  Must be 'text', 'xml', or 'flat'.");
    }
    else
    {
      getObject(cmd, arg1, arg2);
    }
  }

  // COMMAND: ldcstats
  else if (strcasecmp(cmd.c_str(), "ldcstats") == 0)
  {
    std::string arg(cmdArgs.size() > 0 ? cmdArgs[0] : "");
    statsLDC(arg);
  }

  // COMMAND: panic
  else if (strcasecmp(cmd.c_str(), "panic") == 0)
  {
    causeBDBPanic();
  }

  // COMMAND: parm
  else if (strcasecmp(cmd.c_str(), "parm") == 0)
  {
    Scope scope(SCOPE_SINGLE);
    for (STRINGVEC::iterator it = cmdArgs.begin(); it != cmdArgs.end(); ++it)
    {
      if (strcasecmp((*it).c_str(), "all") == 0)
      {
        scope = SCOPE_ALL_LDC;
      }
      else if (strcasecmp((*it).c_str(), "w") == 0)
      {
        scope = SCOPE_WORLD;
      }
    }
    cacheParm(scope);
  }

  // COMMAND: help
  else if (strcasecmp(cmd.c_str(), "help") == 0)
  {
    _showMenu = true;
  }

  // COMMAND: inject
  else if (strcasecmp(cmd.c_str(), "inject") == 0)
  {
    std::string arg1(cmdArgs.size() > 0 ? cmdArgs[0] : "");
    std::string arg2(cmdArgs.size() > 1 ? cmdArgs[1] : "");
    injectCacheNotify(arg1, arg2);
  }

  // COMMAND: inval
  else if (strcasecmp(cmd.c_str(), "inval") == 0)
  {
    if (cmdArgs.size() > 0)
    {
      invalidateKey(*(cmdArgs.begin()));
    }
    else
    {
      invalidateKey("");
    }
  }

  // COMMAND: memkeys
  else if (strcasecmp(cmd.c_str(), "memkeys") == 0)
  {
    Scope scope(SCOPE_SINGLE);
    bool inclValues(false);

    for (STRINGVEC::iterator it = cmdArgs.begin(); it != cmdArgs.end(); ++it)
    {
      if (strcasecmp((*it).c_str(), "all") == 0)
      {
        scope = SCOPE_ALL_LDC;
      }
      else if (strcasecmp((*it).c_str(), "w") == 0)
      {
        scope = SCOPE_WORLD;
      }
      else if (strcasecmp((*it).c_str(), "v") == 0)
      {
        inclValues = true;
      }
    }
    memkeysTseServer(NULL, scope, inclValues);
  }

  // COMMAND: menu
  else if (strcasecmp(cmd.c_str(), "menu") == 0)
  {
    _showMenu = true;
  }

  // COMMAND: run
  else if (strcasecmp(cmd.c_str(), "run") == 0)
  {
    if (cmdArgs.size() > 0)
    {
      runScript(*(cmdArgs.begin()), notExiting);
    }
    else
    {
      SAY("", "SORRY! No command file specified.");
    }
  }

  // COMMAND: server
  else if (strcasecmp(cmd.c_str(), "server") == 0)
  {
    if (allowPrompt)
    {
      promptHostPort();
    }
    else
    {
      _host = (cmdArgs.size() > 0 ? cmdArgs[0] : "");
      _port = (cmdArgs.size() > 1 ? atoi(cmdArgs[1].c_str()) : 0);
    }
  }

  // COMMAND: stats
  else if (strcasecmp(cmd.c_str(), "stats") == 0)
  {
    statsTseServer();
  }

  // COMMAND: stop
  else if (strcasecmp(cmd.c_str(), "stop") == 0)
  {
    stopTseServer();
  }

  // bogus input
  else if (notExiting)
  {
    SAY("", "SORRY! Unrecognized command [" << cmd << "].");
  }

  return rc;
}

int
CacheTestApp::mainMenu(const std::string& oneCmd)
{
  int rc = EXIT_SUCCESS;
  bool notExiting = true;
  std::string response;

  while (notExiting)
  {
    if (oneCmd.empty())
    {
      if (_showMenu)
      {
        SAY("", "");
        SAY("", "  /=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\\");
        SAY("", "  |                             CACHETEST 3000!                               |");
        SAY("", "  |                Do not be afraid.  It is only the future.                  |");
        SAY("", "  |---------------------------------------------------------------------------|");
        SAY("", "  |                         C O M M A N D   L I S T                           |");
        SAY("", "  |---------------------------------------------------------------------------|");
        SAY("", "  |  allnations           - Get NationDAO's internal sorted list              |");
        SAY("", "  |  cache [name]         - Set cache to use; if name is omitted or not       |");
        SAY("", "  |                         specific enough, a list will be presented         |");
        SAY("", "  |  cache_exact [name]   - Set cache to use to be the exact name specified   |");
        SAY("", "  |  compdbvals [all|w]   - Compare mem values to database values; use 'all'  |");
        SAY("", "  |                         to compare all LDC-enabled caches or 'w' to       |");
        SAY("", "  |                         compare all regardless of LDC                     |");
        SAY("", "  |  compkeys [all]       - Compare mem keys to disk keys; use 'all' to       |");
        SAY("", "  |                         compare all LDC-enabled caches                    |");
        SAY("", "  |  compvals [all]       - Compare mem values to disk values; use 'all' to   |");
        SAY("", "  |                         compare all LDC-enabled caches                    |");
        SAY("", "  |  diskkeys [d] [all]   - Get keys on disk; use the 'd' option to include   |");
        SAY("", "  |                         the LDC insert date for each key; use 'all' to    |");
        SAY("", "  |                         get keys for all LDC-enabled caches               |");
        SAY("", "  |  dummy [all|w]        - Insert dummy cache object; use 'all' to include   |");
        SAY("", "  |                         into all LDC-enabled caches or 'w' to include     |");
        SAY("", "  |                         all regardless of LDC                             |");
        SAY("", "  |  exist[s] <key>       - See if the specified key exists                   |");
        SAY("", "  |  exit                 - Exit this program                                 |");
        SAY("", "  |  flush [all|w]        - Flush the cache; 'all' to flush all LDC-enabled   |");
        SAY("", "  |                         caches or 'w' to flush all regardless of LDC      |");
        SAY("", "  |  getdb <f> <k>        - Get the object specified by key <k> from the      |");
        SAY("", "  |                         database and display in boost text (<f> = text),  |");
        SAY("", "  |                         XML (<f> = xml), or enhanced text (<f> = flat)    |");
        SAY("", "  |                         format.                                           |");
        SAY("", "  |  getldc <f> <k>       - Get the object specified by key <k> from the LDC  |");
        SAY("", "  |                         and display in boost text (<f> = text), XML (<f>  |");
        SAY("", "  |                         = xml), or enhanced text (<f> = flat) format      |");
        SAY("", "  |  getmem <f> <k>       - Get the object specified by key <k> from memory   |");
        SAY("", "  |                         and display in boost text (<f> = text), XML (<f>  |");
        SAY("", "  |                         = xml), or enhanced text (<f> = flat) format      |");
        SAY("", "  |  parm [all]           - List CacheParm members; use 'all' to include all  |");
        SAY("", "  |                         caches regardless of LDC                          |");
        SAY("", "  |  help                 - Show this menu                                    |");
        SAY("", "  |  inject <t> <s>       - Inject a cache notification into the server's     |");
        SAY("", "  |                         CacheNotifyAdapter, where <t> is the event type   |");
        SAY("", "  |                         (A,C,D,F) and <s> is notification key string      |");
        SAY("", "  |  inval <key>          - Invalidate the specified key                      |");
        SAY("", "  |  ldcstats [name]      - Get global LDC stats or, if 'name' specified,     |");
        SAY("", "  |                         stats for a specific cache                        |");
        SAY("", "  |  memkeys [v] [all|w]  - Get keys in memory; use 'v' to include the value  |");
        SAY("", "  |                         (enhanced text format); use 'all' to get keys for |");
        SAY("", "  |                         all LDC-enabled caches or 'w' to include all      |");
        SAY("", "  |                         caches regardless of LDC                          |");
        SAY("", "  |  menu                 - Show this menu                                    |");
        SAY("", "  |  panic                - Cause the server to have a BDB panic              |");
        SAY("", "  |  quit                 - Quit this program                                 |");
        SAY("", "  |  run <file>           - Run a series of commands from file                |");
        SAY("", "  |  server               - Set tseserver host/port                           |");
        SAY("", "  |  stats                - Get tseserver stats                               |");
        SAY("", "  |  stop                 - Stop the tseserver!                               |");
        SAY("", "  |  type [name]          - Set cache to use; if [name] is omitted            |");
        SAY("", "  |                         or not specific enough, a list will be            |");
        SAY("", "  |                         presented                                         |");
        SAY("", "  \\=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=/");
        _showMenu = false;
      }

      idop::Prompter::enableHistory(true);
      notExiting = idop::Prompter::getInput("Command> ", response, idop::Prompter::TEXT, true);
      idop::Prompter::enableHistory(false);
    }
    else
    {
      response = oneCmd;
    }

    rc = processCommand(response, notExiting, true, oneCmd.empty());

    if (!oneCmd.empty())
    {
      notExiting = false;
    }
  }

  return rc;
}

bool
CacheTestApp::compareDbValues(Scope scope)
{
  bool retval = true;

  if (scope != SCOPE_SINGLE)
  {
    STRINGVEC goodReturns;
    STRINGVEC badReturns;
    std::string saveCacheName = _cacheName;

    STRINGVEC population;
    if (getCacheData(population, scope))
    {
      for (STRINGVEC::iterator ss = population.begin(); ss != population.end(); ++ss)
      {
        // recursive call
        _cacheName = firstWord((*ss));
        if (compareDbValues(SCOPE_SINGLE))
        {
          goodReturns.push_back(_cacheName);
        }
        else
        {
          badReturns.push_back(_cacheName);
        }
      }
    }
    _cacheName = saveCacheName;
    SAY("", endl << "###");
    if (badReturns.size() == 0)
    {
      SAY("",
          " SUPER-CONGRATULATIONS!! Values for all " << goodReturns.size()
                                                     << " types are in sync.");
    }
    else
    {
      SAY("",
          " SORRY.  Out of " << (goodReturns.size() + badReturns.size()) << " types, "
                             << badReturns.size() << " had values out of sync:");
      for (STRINGVEC::const_iterator b = badReturns.begin(); b != badReturns.end(); ++b)
      {
        SAY("", "         " << (*b));
      }
    }
    SAY("", "###");
    return (badReturns.size() == 0);
  }

  SAY("", endl);
  SAY(_cacheName, "Comparing values...");

  long numCompared(-1);
  std::string response;
  retval = sendToTseServer(CMD_LDC_COMP_DB_VALUES, _cacheName, response);
  if (retval && (!response.empty()))
  {
    STRINGVEC responseLines;
    retval = separateResponse(response, &responseLines);
    for (STRINGVEC::iterator svit = responseLines.begin(); svit != responseLines.end(); ++svit)
    {
      const std::string& response_line((*svit));

      if (svit == responseLines.begin())
      {
        numCompared = strtol(response_line.c_str(), NULL, 10);
      }

      SAY(_cacheName, " " << response_line);
    }
    if (!retval)
    {
      SAY(_cacheName, "ERROR: Comparison failed!");
      retval = false;
    }
  }
  else
  {
    SAY(_cacheName, "ERROR: No response from tseserver!");
    retval = false;
  }

  if (retval)
  {
    SAY(_cacheName,
        "CONGRATULATIONS! " << numCompared << " values in memory and in the database are in sync.");
  }
  else
  {
    SAY(_cacheName,
        "SORRY.  One or more values in memory are out of sync with values in the database.");
  }

  SAY(_cacheName, "Finished value comparision.");
  return retval;
}

bool
CacheTestApp::compareValues(Scope scope)
{
  bool retval = true;

  if (scope != SCOPE_SINGLE)
  {
    STRINGVEC badReturns;
    STRINGVEC goodReturns;
    std::string saveCacheName = _cacheName;

    STRINGVEC population;
    if (getCacheData(population, scope))
    {
      for (STRINGVEC::iterator ss = population.begin(); ss != population.end(); ++ss)
      {
        // recursive call
        _cacheName = firstWord((*ss));
        if (compareValues(SCOPE_SINGLE))
        {
          goodReturns.push_back(_cacheName);
        }
        else
        {
          badReturns.push_back(_cacheName);
        }
      }
    }
    _cacheName = saveCacheName;
    SAY("", endl << "###");
    if (badReturns.size() == 0)
    {
      SAY("",
          " SUPER-CONGRATULATIONS!! Values for all " << goodReturns.size()
                                                     << " types are in sync.");
    }
    else
    {
      SAY("",
          " SORRY.  Out of " << (goodReturns.size() + badReturns.size()) << " types, "
                             << badReturns.size() << " had values out of sync:");
      for (STRINGVEC::const_iterator b = badReturns.begin(); b != badReturns.end(); ++b)
      {
        SAY("", "         " << (*b));
      }
    }
    SAY("", "###");
    return (badReturns.size() == 0);
  }

  SAY("", endl);
  SAY(_cacheName, "Comparing values...");

  long numCompared(-1);
  std::string response;
  retval = sendToTseServer(CMD_LDC_COMPARE_VALUES, _cacheName, response);
  if (retval && (!response.empty()))
  {
    STRINGVEC responseLines;
    retval = separateResponse(response, &responseLines);
    for (STRINGVEC::iterator svit = responseLines.begin(); svit != responseLines.end(); ++svit)
    {
      const std::string& response_line((*svit));

      if (svit == responseLines.begin())
      {
        numCompared = strtol(response_line.c_str(), NULL, 10);
      }

      SAY(_cacheName, " " << response_line);
    }
    if (!retval)
    {
      SAY(_cacheName, "ERROR: Comparison failed!");
      retval = false;
    }
  }
  else
  {
    SAY(_cacheName, "ERROR: No response from tseserver!");
    retval = false;
  }

  if (retval)
  {
    SAY(_cacheName,
        "CONGRATULATIONS! " << numCompared << " values in memory and on disk are in sync.");
  }
  else
  {
    SAY(_cacheName, "SORRY.  One or more values in memory are out of sync with values on disk.");
  }

  SAY(_cacheName, "Finished value comparision.");
  return retval;
}

bool
CacheTestApp::compareKeys(Scope scope)
{
  bool retval = true;

  if (scope != SCOPE_SINGLE)
  {
    STRINGVEC badReturns;
    STRINGVEC goodReturns;
    std::string saveCacheName = _cacheName;

    STRINGVEC population;
    if (getCacheData(population, scope))
    {
      for (STRINGVEC::iterator ss = population.begin(); ss != population.end(); ++ss)
      {
        // recursive call
        _cacheName = firstWord((*ss));
        if (compareKeys(SCOPE_SINGLE))
        {
          goodReturns.push_back(_cacheName);
        }
        else
        {
          badReturns.push_back(_cacheName);
        }
      }
    }
    _cacheName = saveCacheName;
    SAY("", endl << "###");
    if (badReturns.size() == 0)
    {
      SAY("",
          " SUPER-CONGRATULATIONS!! Keys for all " << goodReturns.size() << " types are in sync.");
    }
    else
    {
      SAY("",
          " SORRY.  Out of " << (goodReturns.size() + badReturns.size()) << " types, "
                             << badReturns.size() << " had keys out of sync:");
      for (STRINGVEC::const_iterator b = badReturns.begin(); b != badReturns.end(); ++b)
      {
        SAY("", "         " << (*b));
      }
    }
    SAY("", "###");
    return (badReturns.size() == 0);
  }

  SAY(_cacheName, "Comparing keys...");

  STRINGVEC memVec;
  STRINGVEC ldcVec;

  memkeysTseServer(&memVec);
  diskkeysTseServer(&ldcVec);

  if (memVec.size() == ldcVec.size())
  {
    SAY(_cacheName, "Key count is identical [" << memVec.size() << "].");
  }
  else
  {
    SAY(_cacheName,
        "Key count is inconsistent.  Memory count = [" << memVec.size() << "] while disk count = ["
                                                       << ldcVec.size() << "].");
    retval = false;
  }

  STRINGSET memSet(memVec.begin(), memVec.end());
  STRINGSET ldcSet(ldcVec.begin(), ldcVec.end());

  SAY(_cacheName, "Starting key comparision...");

  uint32_t notFoundOnDisk = 0;
  for (STRINGSET::const_iterator mit = memSet.begin(); mit != memSet.end(); ++mit)
  {
    const std::string& memKey = (*mit);
    if (ldcSet.find(memKey) == ldcSet.end())
    {
      if (++notFoundOnDisk == 1)
      {
        SAY(_cacheName, "   > Memory keys not found on disk:");
      }
      SAY(_cacheName, "      " << memKey);
    }
  }

  uint32_t notFoundInMem = 0;
  for (STRINGSET::const_iterator dit = ldcSet.begin(); dit != ldcSet.end(); ++dit)
  {
    const std::string& ldcKey = (*dit);
    if (memSet.find(ldcKey) == memSet.end())
    {
      if (++notFoundInMem == 1)
      {
        SAY(_cacheName, "   < Disk keys not found in memory:");
      }
      SAY(_cacheName, "      " << ldcKey);
    }
  }

  SAY(_cacheName, "Finished key comparision.");

  if ((notFoundOnDisk == 0) && (notFoundInMem == 0))
  {
    SAY(_cacheName,
        "CONGRATULATIONS! All [" << memVec.size() << "] keys in memory and on disk are in sync.");
  }
  else
  {
    if (notFoundOnDisk > 0)
    {
      if (notFoundOnDisk == 1)
      {
        SAY(_cacheName, "Uh oh.  Found 1 key in memory that was NOT found on disk.");
      }
      else
      {
        SAY(_cacheName,
            "Uh oh.  Found " << notFoundOnDisk
                             << " keys in memory that were NOT found on disk for type.");
      }
    }
    if (notFoundInMem > 0)
    {
      if (notFoundInMem == 1)
      {
        SAY(_cacheName, "Uh oh.  Found 1 key on disk that was NOT found in memory.");
      }
      else
      {
        SAY(_cacheName,
            "Uh oh.  Found " << notFoundInMem
                             << " keys on disk that were NOT found in memory for type.");
      }
    }
    SAY(_cacheName, "SORRY.  Keys in memory are out of sync with keys on disk.");
    retval = false;
  }

  return retval;
}

bool
CacheTestApp::promptHostPort()
{
  bool notExiting = true;

  std::string defHost(_host);
  int defPort(_port);

  std::string tempHost;
  std::string tempPort;

  stringstream hostPrompt;
  hostPrompt << "   Enter tseserver host [" << defHost << "]: ";
  notExiting = idop::Prompter::getInput(hostPrompt.str().c_str(), tempHost);
  if (tempHost.empty())
  {
    tempHost = defHost;
  }

  if (notExiting)
  {
    stringstream portPrompt;
    portPrompt << "   Enter tseserver port [" << defPort << "]: ";
    notExiting = idop::Prompter::getInput(portPrompt.str().c_str(), tempPort);
    if (tempPort.empty())
    {
      stringstream s;
      s << defPort;
      tempPort = s.str();
    }
  }

  if (notExiting)
  {
    _host = tempHost;
    _port = atoi(tempPort.c_str());
    SAY("", endl << "Server is now set to [" << _host << ":" << _port << "].");
  }

  return notExiting;
}

bool
CacheTestApp::invalidateKey(const std::string& flatKey)
{
  bool retval = false;

  std::string payload(_cacheName);
  payload.append("/");
  payload.append(flatKey);

  std::string response;
  if ((retval = sendToTseServer(CMD_INVALIDATE_KEY, payload, response)))
  {
    STRINGVEC responseLines;
    retval = separateResponse(response, &responseLines);
    for (STRINGVEC::iterator svit = responseLines.begin(); svit != responseLines.end(); ++svit)
    {
      SAY(_cacheName, " " << (*svit));
    }
  }

  if (!retval)
  {
    SAY(_cacheName, "ERROR: Problem invalidating specified key!");
  }

  return retval;
}

bool
CacheTestApp::insertDummyObject(Scope scope)
{
  bool retval = false;

  if (scope != SCOPE_SINGLE)
  {
    STRINGVEC badReturns;
    STRINGVEC goodReturns;
    std::string saveCacheName = _cacheName;

    STRINGVEC population;
    if (getCacheData(population, scope))
    {
      for (STRINGVEC::iterator ss = population.begin(); ss != population.end(); ++ss)
      {
        // recursive call
        _cacheName = firstWord((*ss));
        if (insertDummyObject(SCOPE_SINGLE))
        {
          goodReturns.push_back(_cacheName);
        }
        else
        {
          badReturns.push_back(_cacheName);
        }
      }
    }
    _cacheName = saveCacheName;
    SAY("", endl << "###");
    if (badReturns.size() == 0)
    {
      SAY("",
          " SUCCESS!! Dummy objects for all " << goodReturns.size()
                                              << " types have been inserted.");
    }
    else
    {
      SAY("",
          " SORRY.  Out of " << (goodReturns.size() + badReturns.size()) << " types, "
                             << badReturns.size() << " had problems inserting dummy objects:");
    }
    SAY("", "###");
    return (badReturns.size() == 0);
  }

  std::string payload(_cacheName);
  std::string response;
  retval = sendToTseServer(CMD_INSERT_DUMMY_OBJECT, payload, response);
  STRINGVEC responseLines;
  separateResponse(response, &responseLines, false);
  for (STRINGVEC::iterator svit = responseLines.begin(); svit != responseLines.end(); ++svit)
  {
    SAY(_cacheName, " " << (*svit));
  }

  return retval;
}

bool
CacheTestApp::flushCache(Scope scope)
{
  bool retval = false;

  if (scope != SCOPE_SINGLE)
  {
    STRINGVEC badReturns;
    STRINGVEC goodReturns;
    std::string saveCacheName = _cacheName;

    STRINGVEC population;
    if (getCacheData(population, scope))
    {
      for (STRINGVEC::iterator ss = population.begin(); ss != population.end(); ++ss)
      {
        // recursive call
        _cacheName = firstWord((*ss));
        if (flushCache(SCOPE_SINGLE))
        {
          goodReturns.push_back(_cacheName);
        }
        else
        {
          badReturns.push_back(_cacheName);
        }
      }
    }
    _cacheName = saveCacheName;
    SAY("", endl << "###");
    if (badReturns.size() == 0)
    {
      SAY("", " SUCCESS!! Cache flush performed for " << goodReturns.size() << " types.");
    }
    else
    {
      SAY("",
          " SORRY.  Out of " << (goodReturns.size() + badReturns.size()) << " types, "
                             << badReturns.size() << " had problems flushing cache:");
      for (STRINGVEC::const_iterator b = badReturns.begin(); b != badReturns.end(); ++b)
      {
        SAY("", "         " << (*b));
      }
    }
    SAY("", "###");
    return (badReturns.size() == 0);
  }

  std::string payload(_cacheName);

  std::string response;
  if ((retval = sendToTseServer(CMD_CACHE_FLUSH, payload, response)))
  {
    SAY(_cacheName, " " << (response == "1" ? "OK" : "ERROR"));
  }
  else
  {
    SAY(_cacheName, "ERROR: Problem flushing cache!");
  }

  return retval;
}

bool
CacheTestApp::cacheParm(Scope scope)
{
  bool retval = false;

  std::string payload;
  if (scope == SCOPE_SINGLE)
  {
    payload = _cacheName;
  }

  std::string response;
  if ((retval = sendToTseServer(CMD_CACHE_PARM, payload, response)))
  {
    STRINGVEC responseLines;
    retval = separateResponse(response, &responseLines);
    for (STRINGVEC::iterator svit = responseLines.begin(); svit != responseLines.end(); ++svit)
    {
      SAY("", (*svit));
    }
  }

  if (!retval)
  {
    SAY(_cacheName, "ERROR: Problem obtaining cache parms!");
  }

  return retval;
}

bool
CacheTestApp::statsLDC(const std::string& name)
{
  bool retval = false;

  std::string response;
  if ((retval = sendToTseServer(CMD_LDC_STATS, name, response)))
  {
    STRINGVEC responseLines;
    retval = separateResponse(response, &responseLines);
    for (STRINGVEC::iterator svit = responseLines.begin(); svit != responseLines.end(); ++svit)
    {
      SAY("", (*svit));
    }
  }

  if (!retval)
  {
    SAY(_cacheName, "ERROR: Problem obtaining LDC stats!");
  }

  return retval;
}

bool
CacheTestApp::causeBDBPanic()
{
  bool retval = false;

  std::string response;
  if ((retval = sendToTseServer(CMD_BDB_PANIC, "", response)))
  {
    STRINGVEC responseLines;
    retval = separateResponse(response, &responseLines);
    for (STRINGVEC::iterator svit = responseLines.begin(); svit != responseLines.end(); ++svit)
    {
      SAY("", (*svit));
    }
  }

  if (!retval)
  {
    SAY(_cacheName, "ERROR: Problem sending BDB panic message!");
  }

  return retval;
}

bool
CacheTestApp::injectCacheNotify(const std::string& type, const std::string& keyString)
{
  bool retval(false);

  std::string myType(type);
  STR_TOUPPER(myType);
  if ((myType != "A") && (myType != "C") && (myType != "D") && (myType != "F"))
  {
    SAY(_cacheName, "ERROR: Invalid notification type [" << type << "]!");
  }
  else
  {
    std::vector<std::string> notifications;

    addMatchingNotifications(_cacheName, cacheIds, notifications);
    addMatchingNotifications(_cacheName, historicalIds, notifications);

    if (notifications.size() == 0)
    {
      SAY(_cacheName, "ERROR: No Cache Notifications defined for this cache.");
    }
    else
    {
      for (std::vector<std::string>::const_iterator s = notifications.begin();
           s != notifications.end();
           ++s)
      {
        const std::string& notification(*s);
        std::string payload(notification);
        payload.append("/");
        payload.append(myType);
        payload.append("|");
        payload.append(keyString);

        std::string response;
        if ((retval = sendToTseServer(CMD_INJECT_CACHE_NOTIFY, payload, response)))
        {
          STRINGVEC responseLines;
          retval = separateResponse(response, &responseLines);
          for (STRINGVEC::iterator svit = responseLines.begin(); svit != responseLines.end();
               ++svit)
          {
            SAY(_cacheName, " " << (*svit));
          }
        }

        if (!retval)
        {
          SAY(_cacheName,
              "ERROR: Problem injecting specified key string for notification [" << notification
                                                                                 << "]!");
        }
      }
    }
  }

  return retval;
}

bool
CacheTestApp::objectExists(const std::string& flatKey)
{
  bool retval = false;

  std::string payload(_cacheName);
  payload.append("/");
  payload.append(flatKey);

  std::string response;
  if ((retval = sendToTseServer(CMD_OBJECT_EXISTS, payload, response)))
  {
    STRINGVEC responseLines;
    retval = separateResponse(response, &responseLines);
    for (STRINGVEC::iterator svit = responseLines.begin(); svit != responseLines.end(); ++svit)
    {
      SAY(_cacheName, " " << (*svit));
    }
  }

  if (!retval)
  {
    SAY(_cacheName, "ERROR: Problem looking up specified key!");
  }

  return retval;
}

bool
CacheTestApp::getObject(const std::string& command,
                        const std::string& format,
                        const std::string& flatKey)
{
  bool retval = false;

  std::string payload(command);
  payload.append("/");
  payload.append(format);
  payload.append("/");
  payload.append(_cacheName);
  payload.append("/");
  payload.append(flatKey);

  std::string response;
  std::string responseLine;
  if ((retval = sendToTseServer(CMD_QUERY_OBJECT, payload, response)))
  {
    STRINGVEC responseLines;
    retval = separateResponse(response, &responseLines);
    for (STRINGVEC::iterator svit = responseLines.begin(); svit != responseLines.end(); ++svit)
    {
      SAY(_cacheName, " " << (*svit));
    }
  }

  if (!retval)
  {
    SAY(_cacheName, "ERROR: Problem looking up specified key!");
  }

  return retval;
}

bool
CacheTestApp::runScript(const std::string& filename, bool& notExiting)
{
  bool retval = true;

  if (access(filename.c_str(), R_OK) == 0)
  {
    std::ifstream in(filename.c_str());
    if (in.is_open())
    {
      std::string line;
      while (std::getline(in, line, '\n'))
      {
        IDOP_TRIM_STRING(line);
        if (line.size() > 0)
        {
          if (line[0] != '#')
          {
            if (strncasecmp(line.c_str(), "run ", 4) != 0)
            {
              retval = (processCommand(line, notExiting, false, false) == EXIT_SUCCESS);
            }
          }
        }
      }
      in.close();
    }
  }
  else
  {
    SAY("", "ERROR: Unable to open specified file!");
    retval = false;
  }

  return retval;
}

bool
CacheTestApp::getCacheData(STRINGVEC& population, Scope scope)
{
  population.clear();

  if (scope == SCOPE_ALL_LDC)
  {
    SAY("", endl << "Getting eligible caches...");
  }
  else
  {
    SAY("", endl << "Getting all caches...");
  }

  if (cdMap.size() == 0)
  {
    std::string response;
    if (sendToTseServer(CMD_LDCTYPES, "", response))
    {
      if (response.empty())
      {
        SAY("", "SORRY!  No caches returned from the server!");
      }
      else
      {
        STRINGVEC lines;
        if (separateResponse(response, &lines, false))
        {
          for (STRINGVEC::const_iterator cit = lines.begin(); cit != lines.end(); ++cit)
          {
            const std::string& line = (*cit);
            uint32_t idx = 0;
            boost::char_separator<char> fieldSep(",", "", boost::keep_empty_tokens);
            boost::tokenizer<boost::char_separator<char> > fields(line, fieldSep);
            CacheData cd;
            bool eligible(false);
            for (boost::tokenizer<boost::char_separator<char> >::const_iterator
                     tok = fields.begin();
                 tok != fields.end();
                 ++tok, ++idx)
            {
              const std::string& item = (*tok);
              if (!item.empty())
              {
                switch (idx)
                {
                case 0:
                  cd._name = item;
                  STR_TOUPPER(cd._name);
                  if (cd._name.size() > longestCacheName)
                  {
                    longestCacheName = cd._name.size();
                  }
                  break;

                case 1:
                  cd._type = item;
                  if (cd._type.size() > longestCacheType)
                  {
                    longestCacheType = cd._type.size();
                  }
                  break;

                case 2:
                  cd._ldcEnabled = (item == "LDC=ON");
                  break;

                case 3:
                  cd._count = atoll(item.c_str());
                  break;

                default:
                  break;
                }
              }
            }

            cdMap[cd._name] = cd;
          }
        }
        else
        {
          for (STRINGVEC::const_iterator cit = lines.begin(); cit != lines.end(); ++cit)
          {
            const std::string& line = (*cit);
            SAY("", line);
          }
        }
      }
    }
  }

  for (CACHEDATAMAP::const_iterator i = cdMap.begin(); i != cdMap.end(); ++i)
  {
    const CacheData& cd((*i).second);
    if ((scope != SCOPE_ALL_LDC) || cd._ldcEnabled)
    {
      std::ostringstream pline;

      pline << cd._name;
      size_t sz(cd._name.size());
      while (sz++ < longestCacheName)
      {
        pline << " ";
      }

      pline << "   " << cd._type;
      sz = cd._type.size();
      while (sz++ < longestCacheType)
      {
        pline << " ";
      }

      pline << "   ";
      pline << (cd._ldcEnabled ? "LDC" : "   ");

      pline << "   ";
      pline << cd._count;

      population.push_back(pline.str());
    }
  }

  return (population.size() > 0);
}

std::string
CacheTestApp::firstWord(const std::string& val)
{
  std::string retval;
  if (!val.empty())
  {
    std::string::size_type space = val.find_first_of(" ");
    if (space != std::string::npos)
    {
      retval = val.substr(0, space);
    }
    else
    {
      retval = val;
    }
  }
  return retval;
}

void
CacheTestApp::promptCacheName(const std::string& filter, bool notExiting)
{
  int32 value = 0;
  STRINGVEC population;
  STRINGVEC selections;
  STRINGVEC filtered;
  STRINGVEC::iterator ss;

  if ((getCacheData(population, SCOPE_WORLD)) && (population.size() > 0))
  {
    if (filter.empty())
    {
      population.swap(filtered);
    }
    else
    {
      for (ss = population.begin(); ss != population.end(); ++ss)
      {
        const std::string& infoLine = (*ss);
        if (!infoLine.empty())
        {
          std::string cacheName(firstWord(infoLine));
          if (cacheName.size() >= filter.size())
          {
            if (strncasecmp(filter.c_str(), cacheName.c_str(), filter.size()) == 0)
            {
              filtered.push_back(infoLine);
            }
          }
        }
      }
    }
  }

  if (filtered.size() > 0)
  {
    if (filtered.size() > 1)
    {
      if (notExiting)
      {
        SAY("", endl << "   Select one of the following caches:");
        SAY("", "");
      }
      size_t idx = 0;
      for (ss = filtered.begin(); ss != filtered.end(); ++ss)
      {
        ++idx;
        const std::string& infoLine = (*ss);
        SAY("", "   " << std::setw(3) << idx << ". " << infoLine);
        selections.push_back(firstWord(infoLine));
      }
      if (notExiting)
      {
        idop::Prompter::getInteger("   Selection: ", value, true, 1, selections.size());
      }
    }
    else
    {
      const std::string& infoLine = (*filtered.begin());
      selections.push_back(firstWord(infoLine));
      value = 1;
    }
  }

  if (notExiting && (value > 0))
  {
    _cacheName = selections[value - 1];
    SAY("", endl << "Cache is now set to [" << _cacheName << "].");

    const CacheData& cd(cdMap[_cacheName]);
    if (!cd._ldcEnabled)
    {
      SAY("", "WARNING: This cache is not LDC-enabled.");
      SAY("", "         Some commands may not function as expected.");
    }
  }
}

void
CacheTestApp::statsTseServer()
{
  SAY("", endl << "Getting tseserver stats...");
  std::string response;
  if (sendToTseServer(CMD_STATS, "", response))
  {
    if (!response.empty())
    {
      SAY("", response);
    }
  }
}

void
CacheTestApp::stopTseServer()
{
  bool answeredYes = false;
  idop::Prompter::getYesNo("Are you SURE you want to STOP the tseserver (Y/N)? ", answeredYes);
  if (answeredYes)
  {
    answeredYes = false;
    idop::Prompter::getYesNo(
        "No, SERIOUSLY.  Do you REALLY want to being down the tseserver (Y/N)? ", answeredYes);
    if (answeredYes)
    {
      SAY("", endl << "Stopping tseserver...");
      std::string response;
      if (sendToTseServer(CMD_STOP, "", response))
      {
        if (!response.empty())
        {
          SAY("", response);
        }
      }
    }
    else
    {
      SAY("", endl << "Yeah.  I didn't think so.");
    }
  }
}

bool
CacheTestApp::separateResponse(const std::string& response, STRINGVEC* population, bool showCtlInfo)
{
  bool retval = false;

  if (!response.empty())
  {
    uint32_t itemsExpected = 0;
    uint32_t itemsReceived = 0;
    uint32_t idx = 0;

    boost::char_separator<char> fieldSep("\n", "", boost::keep_empty_tokens);
    boost::tokenizer<boost::char_separator<char> > fields(response, fieldSep);

    for (boost::tokenizer<boost::char_separator<char> >::const_iterator f = fields.begin();
         f != fields.end();
         ++f, ++idx)
    {
      const std::string& item = (*f);
      switch (idx)
      {
      case 0:
        retval = (atoi(item.c_str()) == 1);
        if (showCtlInfo)
        {
          SAY(_cacheName, "Server's RC = " << item);
        }
        break;

      case 1:
        itemsExpected = atoi(item.c_str());
        if (showCtlInfo)
        {
          SAY(_cacheName, "Expected Item Count = " << itemsExpected);
        }
        break;

      default:
        if ((!item.empty()) || (itemsReceived < itemsExpected))
        {
          ++itemsReceived;
          if ((population != NULL) && (retval))
          {
            population->push_back(item);
          }
          else
          {
            SAY(_cacheName, "   " << item);
          }
        }
        break;
      }
    }

    if (itemsExpected != itemsReceived)
    {
      SAY(_cacheName,
          "ERROR! Expected " << itemsExpected << " but received " << itemsReceived << "!");
    }
    else
    {
      if (showCtlInfo)
      {
        SAY(_cacheName, "Received " << itemsReceived << " items.");
      }
    }
  }

  return retval;
}

void
CacheTestApp::memkeysTseServer(STRINGVEC* population, Scope scope, bool inclValues)
{
  if (scope != SCOPE_SINGLE)
  {
    std::string saveCacheName = _cacheName;
    STRINGVEC caches;
    if (getCacheData(caches, scope))
    {
      for (STRINGVEC::iterator ss = caches.begin(); ss != caches.end(); ++ss)
      {
        // recursive call
        _cacheName = firstWord((*ss));
        memkeysTseServer(population, SCOPE_SINGLE, inclValues);
      }
    }
    _cacheName = saveCacheName;
    return;
  }

  std::string cmd;
  if (inclValues)
  {
    SAY(_cacheName, "Getting keys and values from memory cache...");
    cmd = CMD_MEMKEYSANDVALUES;
  }
  else
  {
    SAY(_cacheName, "Getting keys from memory cache...");
    cmd = CMD_MEMKEYS;
  }

  std::string response;

  if (sendToTseServer(cmd, _cacheName, response))
  {
    separateResponse(response, population);
  }
}

void
CacheTestApp::nationLoadList()
{
  std::string cmd(CMD_GET_NATION_LOADLIST);
  std::string response;
  if (sendToTseServer(cmd, _cacheName, response))
  {
    separateResponse(response, NULL, true);
  }
}

void
CacheTestApp::diskkeysTseServer(STRINGVEC* population, Scope scope, bool inclDates)
{
  if (scope != SCOPE_SINGLE)
  {
    std::string saveCacheName = _cacheName;
    STRINGVEC caches;
    if (getCacheData(caches, scope))
    {
      for (STRINGVEC::iterator ss = caches.begin(); ss != caches.end(); ++ss)
      {
        // recursive call
        _cacheName = firstWord((*ss));
        diskkeysTseServer(population, SCOPE_SINGLE, inclDates);
      }
    }
    _cacheName = saveCacheName;
    return;
  }

  SAY(_cacheName, "Getting keys from disk cache...");
  std::string response;
  std::string input(_cacheName);
  if (inclDates)
  {
    input.append("/WITHDATES");
  }
  if (sendToTseServer(CMD_DISKKEYS, input, response))
  {
    separateResponse(response, population);
  }
}

bool
CacheTestApp::sendToTseServer(const std::string& cmd,
                              const std::string& payload,
                              std::string& response)
{
  static int socketWaitTime = 6000000;
  bool retval(false);
  bool connOpen(false);
  response = "";

  idop::XMLInitiator appSock("cachetest", _host.c_str(), _port);

  try
  {
    if (appSock.openConnection())
    {
      connOpen = true;
      idop::XMLMessageAttributes appReq(cmd.c_str(), payload.c_str(), 0, false, 1, 0);
      idop::XMLMessageAttributes appRsp;
      if (appSock.send(appReq, &appRsp, socketWaitTime))
      {
        if (appRsp.getPayload() != NULL)
        {
#if 0
              char * p = const_cast<char *>( appRsp.getPayload() ) ;
              for( int counter = 0 ; counter < appRsp.getPayloadSize() ; ++counter, ++p )
              {
                if( *p == '\0' )
                {
                  SAY( _cacheName, "WARNING: Embedded NULL detected at byte " << counter << " of the response." ) ;
                }
              }
#endif

          response.append(appRsp.getPayload());
          retval = true;
        }
        else
        {
          response = "NULL RESPONSE. TSESERVER MAY BE DOWN.";
        }

        appSock.closeConnection(true);
        connOpen = false;
      }
      else
      {
        response = "SEND/RECEIVE FAILURE. TSESERVER MAY BE DOWN.";
      }
    }
    else
    {
      response = "COMM FAILURE. TSESERVER MAY BE DOWN.";
    }
  }
  catch (idop::XMLException& ex)
  {
    response = ex.what();
  }
  catch (std::exception& ex)
  {
    response = ex.what();
  }
  catch (...)
  {
    response = "UNKNOWN EXCEPTION";
  }

  if (connOpen)
  {
    appSock.closeConnection(true);
  }

  return retval;
}

} // namespace tse
