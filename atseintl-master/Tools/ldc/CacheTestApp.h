//----------------------------------------------------------------------------
//
//  File:        CacheTestApp.h
//  Description: Cache Test application class definition
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
#ifndef CACHE_TEST_APP_H
#define CACHE_TEST_APP_H

#include <set>
#include <string>
#include <vector>

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class CacheTestApp
{
public:
  CacheTestApp(const std::string& host,
               int port,
               const std::string& cacheType,
               const std::string& workingDir);

  ~CacheTestApp();
  int run(const std::string& cmd = "");

  static log4cxx::LoggerPtr _logger;

private:
  enum Scope
  { SCOPE_SINGLE = 0,
    SCOPE_ALL_LDC,
    SCOPE_WORLD };

  typedef std::set<std::string, std::less<std::string> > STRINGSET;
  typedef std::vector<std::string> STRINGVEC;

  std::string firstWord(const std::string& val);

  int processCommand(const std::string& response,
                     bool& notExiting,
                     bool allowPrompt = true,
                     bool interactive = true);
  int mainMenu(const std::string& oneCmd = "");
  bool compareValues(Scope scope = SCOPE_SINGLE);
  bool compareDbValues(Scope scope = SCOPE_SINGLE);
  bool compareKeys(Scope scope = SCOPE_SINGLE);
  bool invalidateKey(const std::string& flatKey);
  bool insertDummyObject(Scope scope = SCOPE_SINGLE);
  bool flushCache(Scope scope = SCOPE_SINGLE);
  bool cacheParm(Scope scope = SCOPE_SINGLE);
  bool injectCacheNotify(const std::string& type, const std::string& keyString);
  bool objectExists(const std::string& flatKey);
  bool getObject(const std::string& command, const std::string& format, const std::string& flatKey);
  bool promptHostPort();
  bool runScript(const std::string& filename, bool& notExiting);
  void promptCacheName(const std::string& filter = "", bool showAll = false);
  void statsTseServer();
  void stopTseServer();
  bool statsLDC(const std::string& name);
  bool causeBDBPanic();

  bool getCacheData(STRINGVEC& population, Scope scope);
  void memkeysTseServer(CacheTestApp::STRINGVEC* population = nullptr,
                        Scope scope = SCOPE_SINGLE,
                        bool inclValues = false);
  void diskkeysTseServer(CacheTestApp::STRINGVEC* population = nullptr,
                         Scope scope = SCOPE_SINGLE,
                         bool inclDates = false);
  void nationLoadList();

  bool separateResponse(const std::string& response,
                        CacheTestApp::STRINGVEC* population,
                        bool showCtlInfo = false);

  bool sendToTseServer(const std::string& cmd, const std::string& payload, std::string& response);

  CacheTestApp();
  CacheTestApp(const CacheTestApp& rhs);
  CacheTestApp& operator=(const CacheTestApp& rhs);

  std::string _cacheName;
  std::string _workingDir;
  std::string _host;
  int _port;
  bool _showMenu;

}; // class CacheTestApp

} // namespace tse

#endif // CACHE_TEST_APP_H
