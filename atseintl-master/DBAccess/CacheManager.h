//
// CacheManager.h
//
#pragma once

#include "Common/KeyedFactory.h"
#include "DBAccess/Cache.h"
#include "DBAccess/DAOUtils.h"
#include "DBAccess/HashKey.h"

#include <algorithm>
#include <cctype>
#include <limits>
#include <string>

namespace tse
{
class CacheControl;
class ConfigMan;
class Logger;

class CacheManager
{
public:
  static CacheManager& instance();
  ConfigMan& config() { return _config; }
  int cacheSize(const std::string& key) const;
  bool cacheLoadOnStart(const std::string& key) const;
  bool cacheLoadOnUpdate(const std::string& key) const;
  int cacheFlushInterval(const std::string& id) const;
  const std::string& cacheType(const std::string& key) const;
  size_t cacheFullConsolidationLimit(const std::string& key) const;
  size_t cachePartialConsolidationLimit(const std::string& key) const;
  void dump(int level = log4cxx::Level::INFO_INT) const;
  void dump(std::ostream& s) const;
  DAOUtils::CacheBy cacheBy(const std::string& key) const;
  size_t getTotalCapacity(const std::string& key) const;
  size_t getThreshold(const std::string& key) const;
  inline int printCacheParm(const std::string& name, std::ostream& os) const
  {
    int linesPrinted(0);
    std::string key(name);
    std::transform(key.begin(), key.end(), key.begin(), (int (*)(int))toupper);
    std::map<std::string, CacheParm>::const_iterator i = cacheParms.find(key);
    if (i != cacheParms.end())
    {
      linesPrinted = i->second.print(name, os);
    }
    return linesPrinted;
  }

  struct CacheParm
  {
    CacheParm()
      : cacheMax(0),
        loadOnStart(true),
        loadOnUpdate(false),
        flushInterval(0),
        totalCapacity(0),
        threshold(0),
        cacheBy(DAOUtils::HALFMONTHLY),
        partialConsolidationLimit(std::numeric_limits<size_t>::max()),
        fullConsolidationLimit(std::numeric_limits<size_t>::max())
    {
    }

    int cacheMax;
    bool loadOnStart;
    bool loadOnUpdate;
    int flushInterval;
    std::string cacheType;
    size_t totalCapacity;
    size_t threshold;
    DAOUtils::CacheBy cacheBy;
    size_t partialConsolidationLimit;
    size_t fullConsolidationLimit;

    inline int print(const std::string& name, std::ostream& os) const
    {
      int l(0);
      os << name << ".cacheMax=" << cacheMax << std::endl;
      ++l;
      os << name << ".loadOnStart=" << loadOnStart << std::endl;
      ++l;
      os << name << ".loadOnUpdate=" << loadOnUpdate << std::endl;
      ++l;
      os << name << ".flushInterval=" << flushInterval << std::endl;
      ++l;
      os << name << ".cacheType=" << cacheType << std::endl;
      ++l;
      os << name << ".totalCapacity=" << totalCapacity << std::endl;
      ++l;
      os << name << ".threshold=" << threshold << std::endl;
      ++l;
      os << name << ".cacheBy=" << cacheBy << std::endl;
      ++l;
      os << name << ".partialConsolidationLimit=" << partialConsolidationLimit << std::endl;
      ++l;
      os << name << ".fullConsolidationLimit=" << fullConsolidationLimit << std::endl;
      ++l;
      return l;
    }
  };

  static bool initialized();

  static bool useGenericCache()
  {
    return _useGenericCache;
  }

protected:
  static CacheManager* _instance;
  ConfigMan& _config;

  static std::map<std::string, CacheParm> cacheParms;

  CacheManager(ConfigMan& config);
  virtual ~CacheManager();

private:
  CacheManager(const CacheManager&);
  CacheManager& operator=(const CacheManager&);
  friend class DataManager;
  friend class MockDataManager; // mock object for testing
  friend class QueryMockDataManager; // mock object for testing

  static void initCache(std::pair<const std::string, CacheControl*>& p);
  struct LogCache;
  static Logger& getLogger();
  static DAOUtils::CacheBy decodeCacheBy(const std::string& name);
  static const char* encodeCacheBy(DAOUtils::CacheBy cacheBy);
  static bool _useGenericCache;
  static bool _initialized;
};
} // namespace tse
