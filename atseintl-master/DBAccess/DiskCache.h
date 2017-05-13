//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once

#include "Common/Thread/TSEFastMutex.h"
#include "DBAccess/CacheControl.h"

#include <boost/thread/lock_guard.hpp>
#include <boost/thread/mutex.hpp>

#include <db.h>
#include <openssl/sha.h>
#include <sys/timeb.h>
#include <sys/times.h>
#include <utime.h>

#include <atomic>
#include <set>
#include <string>
#include <sstream>

constexpr uint32_t TTL_FOREVER = 0;
#define DISKCACHE tse::DiskCache::instance()
#define PROTECT_DBENV                                                                              \
  tse::DiskCache::ConditionalLock dbEnvConditionalLock(DISKCACHE.serializeEnvAccess(),             \
                                                       DISKCACHE._envMutex);

namespace tse
{
static const char* DISKCACHE_ALL = "";
static const long DISKCACHE_ALL_VERSIONS = -1;

class Logger;

class DiskCache
{
  struct ConditionalLock
  {
    ConditionalLock(bool lock, boost::mutex& mutex) : _lock(lock), _mutex(mutex)
    {
      if (LIKELY(_lock))
      {
        _mutex.lock();
      }
    }

    ~ConditionalLock()
    {
      if (LIKELY(_lock))
      {
        _mutex.unlock();
      }
    }

    const bool _lock;
    boost::mutex& _mutex;
  };

public:
  enum LoadSource
  {
    LOADSOURCE_OTHER = 0,
    LOADSOURCE_DB,
    LOADSOURCE_LDC,
    LoadSource_END
  };

  enum StartAction
  {
    DISABLE_ON_START = 0,
    LOAD_ON_START,
    CLEAR_ON_START,
    StartAction_END // Keep this last!
  };

  enum DataFormat
  {
    DATAFMT_BN2 = 0,
    DataFormat_END // Keep this last!
  };

  enum DBRead
  {
    DBREAD_SUCCESS = 0,
    DBREAD_FAILURE,
    DBREAD_DONE,
    DBRead_END // Keep this last!
  };

  using STRINGSET = std::set<std::string, std::less<std::string>>;

  class Timer
  {
  public:
    void reset()
    {
      times(&_cpuStart);
      ftime(&_tmTmp);
      _tmStart = (static_cast<long long>(_tmTmp.time) * 1000) + _tmTmp.millitm;
    }

    void checkpoint()
    {
      times(&_cpuStop);
      ftime(&_tmTmp);
      _tmStop = (static_cast<long long>(_tmTmp.time) * 1000) + _tmTmp.millitm;
      _timeDiff = static_cast<int32_t>(_tmStop - _tmStart);
      _cpuDiff = static_cast<int32_t>(((_cpuStop.tms_utime - _cpuStart.tms_utime) +
                                       (_cpuStop.tms_stime - _cpuStart.tms_stime)) *
                                      1000) /
                 CLOCK_TICKS;
    }

    int elapsed() { return _timeDiff; }
    int cpu() { return _cpuDiff; }

  private:
    long long _tmStart = 0;
    long long _tmStop = 0;
    int _timeDiff = 0;
    int _cpuDiff = 0;
    tms _cpuStart;
    tms _cpuStop;
    timeb _tmTmp;

    static constexpr int CLOCK_TICKS = 100;
  };

  class DataBlob
  {
  public:
#pragma pack(1)
    struct BlobHeader
    {
      uint32_t headerSize = 0;
      uint32_t unCompressedSize = 0;
      char compressed = 0;
      time_t timestamp = 0;
      unsigned char sha1[SHA_DIGEST_LENGTH];
      char dataFormat = 0;
    };
#pragma pack()

    DataBlob() = default;
    DataBlob(const DataBlob&) = delete;
    DataBlob& operator=(const DataBlob&) = delete;
    virtual ~DataBlob();

    void clear();

    void setData(const char* data, uint32_t uncmpSize, uint32_t sizeLimit, DataFormat fmt);
    DiskCache::DataFormat getData(char*& buffer, size_t& size, char*& bufferToDelete);

    bool setRawData(char* data, uint32_t dataLen);
    char* getRawData(uint32_t* dataLen);
    BlobHeader* getHeader() { return reinterpret_cast<BlobHeader*>(_dataBuffer); }

  private:
#define COMPRESSED '1'
#define UNCOMPRESSED '0'

    char* _dataBuffer = nullptr;
    uint32_t _dataBufferSize = 0;
    uint32_t _dataSize = 0;
    uint32_t _headerSize = sizeof(BlobHeader);

    char* compress(const char* source, uint32_t sourceLen, uint32_t* resultSize, int level);
    char* decompress(const char* source, uint32_t sourceLen, uint32_t* resultSize);
  };

  class CacheTypeOptions
  {
  public:
    CacheTypeOptions(const std::string& nameArg,
                     bool enabledArg,
                     DiskCache::DataFormat dataFormatArg,
                     uint32_t maxCatchupMinutesArg,
                     DB* dbArg,
                     uint32_t compressionLimitArg,
                     bool useDistCacheArg,
                     size_t ttlArg,
                     size_t ldcMaxBlobSize)
      : name(nameArg),
        enabled(enabledArg),
        dataFormat(dataFormatArg),
        maxCatchupMinutes(maxCatchupMinutesArg),
        compressionLimit(compressionLimitArg),
        useDistCache(useDistCacheArg),
        ttl(ttlArg),
        ldcMaxBlobSize(ldcMaxBlobSize),
        db(dbArg)
    {
    }

    void dbOpen(bool readOnly = false);
    void dbClose();
    bool dbRemove();
    DB* dbGet(bool readOnly = false);

    void disable();

    void disableCloseAndRemove()
    {
      disable();
      dbClose();
      dbRemove();
    }

    std::string name;
    bool enabled = false;
    DiskCache::DataFormat dataFormat = DataFormat::DATAFMT_BN2;
    uint32_t maxCatchupMinutes = 60;
    uint32_t compressionLimit = 0;
    bool useDistCache = false;
    size_t ttl = 0;
    size_t ldcMaxBlobSize = 0;

  private:
    CacheControl* ctrl = nullptr;
    DB* db = nullptr;
    bool isReadOnly = true;
    boost::mutex dbMutex;
  };

  using CACHE_TYPE_CTRL = std::map<std::string, CacheTypeOptions*, std::less<std::string>>;

  ~DiskCache();

  static DiskCache& instance() { return *_instance; }

  static bool isActivated();
  static void shutdown();
  static Logger& getLogger();
  static std::string loadSourceToString(LoadSource val);
  static uint32_t getDBNames(STRINGSET& list);
  static bool dataFormatFromString(const std::string& value, DataFormat& retval);
  static void initialize(ConfigMan& config);
  static bool removeDirectory(const std::string& path);
  static bool removeFile(const std::string& path);

  void sendPanicEvent();

  void disableBDB() { _bdbDisabled = true; }

  void setDbVerifyOnOpen(bool rhs) { _dbVerifyOnOpen = rhs; }

  // Should only be used by TseAppConsole
  bool setKeepDbHandlesOpen(bool rhs)
  {
    boost::lock_guard<boost::mutex> g(DISKCACHE._envMutex);
    bool priorSetting(_keepDbHandlesOpen);
    _keepDbHandlesOpen = rhs;
    return priorSetting;
  }

  // Should only be used by TseAppConsole
  bool setKeepEnvHandleOpen(bool rhs)
  {
    boost::lock_guard<boost::mutex> g(DISKCACHE._envMutex);
    bool priorSetting(_keepEnvHandleOpen);
    _keepEnvHandleOpen = rhs;
    return priorSetting;
  }

  bool makeLDCDirectory();
  void clearAllActionQueues();
  void removeObsoleteTableVersions();
  void waitForQueueToSubside();
  void disableCloseAndRemove(const std::string& name);

  size_t sizeAllQueues() const;
  void showEligible() const;
  DiskCache::CacheTypeOptions* getCacheTypeOptions(const std::string& name) const;
  const char* getDataFormatAsString(const std::string& name) const;
  DataFormat getDataFormat(const std::string& name) const;
  bool isEnabled(const std::string& name) const;

  bool bdbDisabled() const { return _bdbDisabled; }
  bool logQueueActivity() const { return _logQueueActivity; }
  bool openFileSentinels() const { return _openFileSentinels; }
  bool dbVerifyOnOpen() const { return _dbVerifyOnOpen; }
  bool keepDbHandlesOpen() const { return _keepDbHandlesOpen; }
  bool keepEnvHandleOpen() const { return _keepEnvHandleOpen; }
  bool serializeEnvAccess() const { return _serializeEnvAccess; }
  bool useTransactionModel() const { return _useTransactionModel; }
  bool shouldRemoveBerkeleyDBEnv() const { return _shouldRemoveBerkeleyDBEnv; }
  bool enablePrestartHook() const { return _enablePrestartHook; }
  bool prestartRecover() const { return _prestartRecover; }
  bool prestartDbVerify() const { return _prestartDbVerify; }
  uint32_t getThreadSleepSecs() const { return _threadSleepSecs; }
  uint32_t getMaxCatchupMinutes() const { return _maxCatchupMinutes; }
  uint32_t getFullVerifyHour() const { return _fullVerifyHour; }
  size_t getSerializeQueueThreadMax() const { return _serializeQueueThreadMax; }
  const std::string& getQueueLogDir() const { return _queueLogDir; }
  bool shuttingDown() const { return _shuttingDown; }
  bool getDistCacheEnabled() const { return _distCacheEnabled; }
  bool getDistCacheLoadMaster() const { return _distCacheLoadMaster; }
  bool getDistCacheEventMaster() const { return _distCacheEventMaster; }
  const std::string& servers() const { return _servers; }
  size_t poolSize() const { return _poolSize; }

  size_t getMaxActiveDeserializeTasks() const { return _maxActiveDeserializeTasks; }
  uint32_t getDeserializeSleepMillis() const { return _deserializeSleepMillis; }

  bool remove(time_t reqTime = time(nullptr),
              bool whileLoading = false,
              const std::string& tableName = DISKCACHE_ALL,
              const std::string& flatKey = DISKCACHE_ALL);

  std::string constructDBFileName(const std::string& cacheName, int version = -1);
  DateTime getLastUpdateTime();
  void updateLastUpdateTime();

  bool isTooOld() { return isTooOld(getLastUpdateTime()); }

  bool isTooOld(const DateTime& lastUpdate)
  {
    bool retval(true);
    if (lastUpdate != DateTime::emptyDate())
    {
      DateTime now(DateTime::localTime());
      int64_t diffMinutes = (DateTime::diffTime(now, lastUpdate) / 60);
      retval = (diffMinutes > _maxCatchupMinutes);
    }
    return retval;
  }

  DB_ENV* createDBEnvironment(u_int32_t openFlags, const char* mode);
  bool openEnvironment();
  void doneWithEnvironment(bool forceClose = false);
  void doneWithDB(const std::string& name);
  DB* getDB(const std::string& name, bool readOnly = false, CacheTypeOptions* cto = nullptr);
  bool writeToDB(const std::string& name, const std::string& key, DataBlob& blob);
  DBRead readFromDB(const std::string& name, std::string& key, DataBlob* blob);
  DBRead readNextFromDB(DBC** dbCur, const std::string& name, std::string& key, DataBlob* blob);
  bool deleteFromDB(const std::string& name, const std::string& key);
  bool truncateDB(const std::string& name);
  bool removeDB(const std::string& name);
  uint32_t getKeys(const std::string& name, STRINGSET& list, bool withDates = false);
  uint32_t readKeys(const std::string& name);

  bool doVerifyNextCycle();

  const std::string& directory() const { return _directory; }

  const CACHE_TYPE_CTRL& getCacheControl() { return _cacheCtrl; }

  void bdbEvent(DB_ENV* dbenv, u_int32_t event, void* event_info);

  uint32_t maxPerCycle() { return _maxPerCycle; }

private:
  friend class CacheTypeOptions;

  static DiskCache* _instance;
  static boost::mutex _mutex;

  ConfigMan& _config;
  std::string _directory;
  DataFormat _defaultDataFormat = DataFormat::DATAFMT_BN2;
  uint32_t _threadSleepSecs = 20;
  uint32_t _maxCatchupMinutes = 240;
  uint32_t _fullVerifyHour = 0;
  size_t _serializeQueueThreadMax = 10;
  size_t _maxActiveDeserializeTasks = 0;
  uint32_t _deserializeSleepMillis = 5;
  uint32_t _berkeleyDBPageSize = 4096;
  uint32_t _berkeleyCacheSize = 1;
  bool _quiesceBeforeReady = false;

  StartAction _startAction = StartAction::DISABLE_ON_START;
  bool _isActivated = false;
  bool _bdbDisabled = false;
  bool _logQueueActivity = false;
  bool _openFileSentinels = false;
  bool _dbVerifyOnOpen = false;
  bool _keepDbHandlesOpen = false;
  bool _keepEnvHandleOpen = false;
  bool _serializeEnvAccess = false;
  bool _useTransactionModel = false;
  bool _shouldRemoveBerkeleyDBEnv = false;
  bool _enablePrestartHook = false;
  bool _prestartRecover = false;
  bool _prestartDbVerify = false;
  std::string _queueLogDir;

  DB_ENV* _dbEnv = nullptr;
  boost::mutex _envMutex;

  CACHE_TYPE_CTRL _cacheCtrl;

  bool _shuttingDown = false;

  bool _distCacheEventMaster;
  bool _distCacheLoadMaster;
  bool _distCacheEnabled = false;
  uint32_t _defaultTTL = TTL_FOREVER;
  std::string _servers;
  uint32_t _poolSize = DEFAULT_DISTCACHE_POOL_SIZE;
  size_t _ldcMaxBlobSize;

  TSEFastMutex _panicMutex;
  std::atomic_int _activeIO{0};
  uint32_t _maxPerCycle;

  static const size_t DEFAULT_DISTCACHE_POOL_SIZE = 5;

  template <class U, typename Validator = bool (*)(const std::string&, U&)>
  void getConfigOption(const char* name, U& value, Validator);

  DiskCache(ConfigMan& config);
  DiskCache(const DiskCache&) = delete;
  DiskCache& operator=(const DiskCache&) = delete;
};
} // namespace tse

