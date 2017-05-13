//----------------------------------------------------------------------------
//
//  File:    DiskCache.cpp
//
//  Description:
//     DiskCache holds the disk version all the caches.
//     It is called from the server when the shared library is loaded.
//
//  Copyright (c) Sabre 2008
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//
//----------------------------------------------------------------------------

#include "DBAccess/DiskCache.h"

#include "Common/Config/ConfigMan.h"
#include "Common/DateTime.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/TimeUtil.h"
#include "Common/TseUtil.h"
#include "DBAccess/CacheControl.h"
#include "DBAccess/CacheRegistry.h"
#include "DBAccess/DistCache.h"

#include <boost/tokenizer.hpp>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <string>

#include <dirent.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <sys/fcntl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <utime.h>
#include <zlib.h>

// Default of zero [ no size limit ]. Only enable the size limit if
// set per cache.
const size_t
DEFAULT_MAX_LDC_BLOB_BYTES(0);
const size_t
DEFAULT_MAX_OPERATIONS_PER_CYCLE(0);

namespace
{
sighandler_t prevABRT = nullptr;
}

extern "C" {
void
handleShutdownBeforeAbort(int number)
{
  signal(SIGABRT, prevABRT);

  tse::DiskCache::shutdown();
  if (prevABRT)
    prevABRT(number);
}
}

#define BDB_ERROR_STRING(ret, desc)                                                                \
  "BerkeleyDB " << desc << " error (" << ret << "): " << db_strerror(ret)

int
touch_file(const std::string& fileName)
{
  int retval(0);
  FILE* fp(fopen(fileName.c_str(), "a"));
  if (fp != nullptr)
  {
    fclose(fp);
  }
  else
  {
    retval = errno;
  }
  return retval;
}

std::string static trim(const std::string& rhs)
{
  const char* c = " \t\n";
  std::string s(rhs);
  size_t i = s.find_first_not_of(c);
  if (i != std::string::npos)
    s.erase(0, i);

  size_t e = s.find_last_not_of(c);
  if (e != std::string::npos && e < s.length())
    s.erase(e + 1, s.length() - 1);
  return s;
}

#define STR_TOUPPER(a) std::transform(a.begin(), a.end(), a.begin(), (int (*)(int))std::toupper);

namespace tse
{
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Constants
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
static const char* DataFormats[DiskCache::DataFormat_END] = { "BN2" };

static const char* TABLE_CTL_FILE_EXT = ".ctl";
static const char* TABLE_OPEN_FILE_EXT = ".open";
static const char* LAST_UPDATE_FILE = ".lastupdate";
static const char* CLEAR_NEXT_START_FILE = ".clear";
static const char* VERIFY_NEXT_CYCLE_FILE = ".verify";

static const char* LDC_CACHE_NAME = "LDC_CACHE_NAME";
static const char* LDC_CACHE_TYPE = "LDC_CACHE_TYPE";
static const char* LDC_TABLE_VERSION = "LDC_TABLE_VERSION";
static const char* LDC_DATA_FORMAT = "LDC_DATA_FORMAT";
static const char* LDC_CREATE_TIMESTAMP = "LDC_CREATE_TIMESTAMP";
static const char* LDC_CREATE_HOST = "LDC_CREATE_HOST";
static const char* LDC_PROCESS_DIR = "LDC_PROCESS_DIR";

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Helper Functions
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

extern "C" void
bdbEventHandler(DB_ENV* dbenv, u_int32_t event, void* event_info)
{
  DISKCACHE.bdbEvent(dbenv, event, event_info);
}

namespace
{
bool
stringToUint32(const std::string& value, uint32_t& result)
{
  bool retval(false);

  errno = 0;
  long temp = strtol(value.c_str(), nullptr, 10);
  if ((temp != 0) || (errno == 0))
  {
    result = static_cast<uint32_t>(temp);
    retval = true;
  }

  return retval;
}

bool
validateNotEmpty(const std::string& value, std::string& final)
{
  bool retval(false);
  if (!value.empty())
  {
    final = value;
    retval = true;
  }
  return retval;
}

bool
validateNum(const std::string& value, uint32_t& final)
{
  bool retval(false);
  if (!value.empty())
  {
    retval = stringToUint32(value, final);
  }
  return retval;
}

bool
validateNumGreaterThanZero(const std::string& value, uint32_t& final)
{
  bool retval(false);
  uint32_t temp(0);
  if (validateNum(value, temp))
  {
    if (temp > 0)
    {
      final = temp;
      retval = true;
    }
  }
  return retval;
}

bool
validateSize(const std::string& value, size_t& final)
{
  bool retval(false);
  uint32_t temp(0);
  if (validateNum(value, temp))
  {
    if (temp > 0)
    {
      final = static_cast<size_t>(temp);
      retval = true;
    }
  }
  return retval;
}

bool
validateBoolean(const std::string& value, bool& final)
{
  bool retval(true);

  if ((value == "Y") || (value == "y") || (value == "1") || (value == "true") ||
      (value == "TRUE") || (value == "yes") || (value == "YES"))
  {
    final = true;
  }
  else if ((value == "N") || (value == "n") || (value == "0") || (value == "false") ||
           (value == "FALSE") || (value == "no") || (value == "NO"))
  {
    final = false;
  }
  else
  {
    retval = false;
  }

  return retval;
}

bool
validateDataFormat(const std::string& value, DiskCache::DataFormat& final)
{
  return DiskCache::dataFormatFromString(value, final);
}

bool
validateStartupAction(const std::string& value, DiskCache::StartAction& final)
{
  bool retval(true);

  std::string uc(value);
  STR_TOUPPER(uc);

  if (uc == "DISABLE")
  {
    final = DiskCache::DISABLE_ON_START;
  }
  else if (uc == "LOAD")
  {
    final = DiskCache::LOAD_ON_START;
  }
  else if (uc == "CLEAR")
  {
    final = DiskCache::CLEAR_ON_START;
  }
  else
  {
    retval = false;
  }

  return retval;
}

bool
getLDCControlFileValue(const std::string& ldcDir,
                       const std::string& tableName,
                       const std::string& variable,
                       std::string& value)
{
  bool retval(false);

  std::string dbFileName(DISKCACHE.constructDBFileName(tableName));
  if (!dbFileName.empty())
  {
    std::ostringstream ctlFile;
    ctlFile << ldcDir << "/" << dbFileName << TABLE_CTL_FILE_EXT;
    if (access(ctlFile.str().c_str(), F_OK) == 0)
    {
      std::ifstream ifs(ctlFile.str().c_str());
      if (ifs.is_open())
      {
        std::string line;
        while (std::getline(ifs, line, '\n'))
        {
          size_t keySize = variable.size();
          if ((line.size() > (keySize + 1)) && (line.substr(0, keySize) == variable) &&
              (line[keySize] == '='))
          {
            value = line.substr(keySize + 1);
            retval = true;
            break;
          }
        }
        ifs.close();
      }
    }
  }
  else
  {
    LOG4CXX_ERROR(DiskCache::getLogger(),
                  "Unable to find cache [" << tableName << "]"
                                           << " in the Cache Registry !!!");
  }

  return retval;
}
}

DiskCache::DataBlob::~DataBlob()
{
  if (_dataBuffer != nullptr)
  {
    delete[] _dataBuffer;
  }
}

void
DiskCache::DataBlob::clear()
{
  if (UNLIKELY(_dataBuffer != nullptr))
  {
    delete[] _dataBuffer;

    _dataBuffer = nullptr;
    _dataBufferSize = 0;
    _dataSize = 0;
  }
}

char*
DiskCache::DataBlob::compress(const char* source,
                              uint32_t sourceLen,
                              uint32_t* resultSize,
                              int level)
{
  // destination buffer must be size of input buffer + 0.1% + 12
  // according to zlib documentation

  uLongf targetSize = 12 + sourceLen + sourceLen / 1000 + 1;
  Bytef* target = new Bytef[targetSize];

  int stat =
      compress2(target, &targetSize, reinterpret_cast<const Bytef*>(source), sourceLen, level);
  if (stat == Z_OK)
  {
    *resultSize = targetSize;
  }
  else
  {
    delete[] target;
    target = nullptr;

    switch (stat)
    {
    case Z_BUF_ERROR:
    {
      fprintf(stderr, "Error compressing data: buffer not big enough\n");
      break;
    }

    case Z_MEM_ERROR:
    {
      fprintf(stderr, "Error compressing data: memory error\n");
      break;
    }

    default:
    {
      fprintf(stderr, "Error compressing data: unknown error %d", stat);
      break;
    }
    }
  }
  return reinterpret_cast<char*>(target);
}

char*
DiskCache::DataBlob::decompress(const char* source, uint32_t sourceLen, uint32_t* resultSize)
{
  uLongf targetSize = *resultSize;
  Bytef* target = new Bytef[targetSize];

  int stat = uncompress(target, &targetSize, reinterpret_cast<const Bytef*>(source), sourceLen);

  if (LIKELY(stat == Z_OK))
  {
    *resultSize = targetSize;
  }
  else
  {
    delete[] target;
    target = nullptr;

    switch (stat)
    {
    case Z_BUF_ERROR:
    {
      fprintf(stderr, "Error uncompressing data: buffer not big enough\n");
      break;
    }
    case Z_MEM_ERROR:
    {
      fprintf(stderr, "Error uncompressing data: memory error\n");
      break;
    }
    case Z_DATA_ERROR:
    {
      fprintf(stderr, "Error uncompressing data: data is corrupt\n");
      break;
    }

    default:
    {
      fprintf(stderr, "Error uncompressing data: unknown error %d", stat);
      break;
    }
    }
  }

  return reinterpret_cast<char*>(target);
}

void
DiskCache::DataBlob::setData(const char* data,
                             uint32_t uncmpSize,
                             uint32_t sizeLimit,
                             DataFormat fmt)
{
  clear();

  const char* curData = data;
  uint32_t curSize = uncmpSize;
  char cmp = UNCOMPRESSED;
  char* cmpData = nullptr;
  uint32_t cmpSize = 0;

  if ((sizeLimit != 0) && (curSize > sizeLimit))
  {
    cmpData = compress(curData, curSize, &cmpSize, 1);
    if ((cmpData != nullptr) && (cmpSize < curSize))
    {
      curSize = cmpSize;
      curData = cmpData;
      cmp = COMPRESSED;
    }
  }

  _dataSize = curSize;
  _dataBufferSize = curSize + _headerSize;
  _dataBuffer = new char[_dataBufferSize];

  memcpy(&_dataBuffer[_headerSize], curData, _dataSize);

  if (cmpData != nullptr)
  {
    delete[] cmpData;
  }

  BlobHeader* header = reinterpret_cast<BlobHeader*>(_dataBuffer);

  header->headerSize = _headerSize;
  header->unCompressedSize = uncmpSize;
  header->compressed = cmp;
  header->timestamp = time(nullptr);

  SHA1(reinterpret_cast<const unsigned char*>(&_dataBuffer[_headerSize]),
       _dataSize,
       reinterpret_cast<BlobHeader*>(_dataBuffer)->sha1);

  header->dataFormat = static_cast<char>(fmt);
}

DiskCache::DataFormat
DiskCache::DataBlob::getData(char*& buffer, size_t& size, char*& bufferToDelete)
{
  DiskCache::DataFormat retval(DATAFMT_BN2);
  if (LIKELY(_dataBuffer != nullptr))
  {
    BlobHeader* header = reinterpret_cast<BlobHeader*>(_dataBuffer);

    retval = static_cast<DiskCache::DataFormat>(header->dataFormat);

    if (header->compressed == UNCOMPRESSED)
    {
      buffer = &_dataBuffer[_headerSize];
      size = _dataSize;
      bufferToDelete = nullptr;
    }
    else
    {
      char* uncmpData = nullptr;
      uint32_t uncmpSize = reinterpret_cast<BlobHeader*>(_dataBuffer)->unCompressedSize;

      uncmpData = decompress(&_dataBuffer[_headerSize], _dataSize, &uncmpSize);
      if (LIKELY(uncmpData != nullptr))
      {
        buffer = uncmpData;
        size = uncmpSize;
        bufferToDelete = uncmpData;
      }
    }
  }
  return retval;
}

bool
DiskCache::DataBlob::setRawData(char* data, uint32_t dataLen)
{
  bool retval = true;
  unsigned char sha1_buf[SHA_DIGEST_LENGTH];

  if (LIKELY((dataLen > 0) && (data != nullptr)))
  {
    clear();

    _dataBufferSize = dataLen;
    _dataSize = _dataBufferSize - _headerSize;
    _dataBuffer = new char[_dataBufferSize];

    memcpy(_dataBuffer, data, _dataBufferSize);

    BlobHeader* header = reinterpret_cast<BlobHeader*>(_dataBuffer);

    SHA1(reinterpret_cast<const unsigned char*>(&_dataBuffer[_headerSize]), _dataSize, sha1_buf);

    if (UNLIKELY(memcmp(header->sha1, sha1_buf, SHA_DIGEST_LENGTH) != 0))
    {
      retval = false;
      clear();
    }
  }

  return retval;
}

char*
DiskCache::DataBlob::getRawData(uint32_t* dataLen)
{
  *dataLen = _dataBufferSize;

  return _dataBuffer;
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CacheTypeOptions
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

void
DiskCache::CacheTypeOptions::disable()
{
  enabled = false;
}

void
DiskCache::CacheTypeOptions::dbOpen(bool readOnly)
{
  if (UNLIKELY(!enabled))
  {
    return;
  }

  if (UNLIKELY(DISKCACHE.bdbDisabled()))
  {
    return;
  }

  // If the DB is already open in read-only mode, but we need to WRITE,
  // then close it and re-open it in write mode.
  //
  bool upgradeAccessMode(false);
  {
    boost::lock_guard<boost::mutex> g(dbMutex);
    if (db != nullptr) // DB is already open
    {
      if (isReadOnly) // DB is was opened read-only
      {
        if (UNLIKELY(!readOnly)) // this request needs to WRITE
        {
          upgradeAccessMode = true;
          LOG4CXX_INFO(getLogger(),
                       "Upgrading BDB access mode for cache [" << name << "] to WRITE");
        }
      }
    }
  }

  if (UNLIKELY(upgradeAccessMode))
  {
    dbClose();
  }

  { // Scope the mutex
    boost::lock_guard<boost::mutex> g(dbMutex);
    if (db != nullptr)
    {
      return;
    }

    // Get the CacheRegistry pointer for this cache if necessary
    if (ctrl == nullptr)
    {
      ctrl = CacheRegistry::instance().getCacheControl(name);
      if (ctrl == nullptr)
      {
        LOG4CXX_ERROR(getLogger(), "Unable to find cache [" << name << "] in Cache Registry !!!");
        return;
      }
    }

    // Set up file paths
    std::string dbFileName = DISKCACHE.constructDBFileName(name, ctrl->tableVersion());

    std::string dbFilePath = DISKCACHE._directory;
    dbFilePath.append("/");
    dbFilePath.append(dbFileName);

    bool dbFileExists(access(dbFilePath.c_str(), F_OK) == 0);

    std::string ctlFile = DISKCACHE._directory;
    ctlFile.append("/");
    ctlFile.append(dbFileName);
    ctlFile.append(TABLE_CTL_FILE_EXT);

    std::string openSentinel = DISKCACHE._directory;
    openSentinel.append("/");
    openSentinel.append(dbFileName);
    openSentinel.append(TABLE_OPEN_FILE_EXT);

    // say what I'm doing
    LOG4CXX_DEBUG(DISKCACHE.getLogger(),
                  "Opening Berkeley DB [" << dbFileName << "], Page Size [" << compressionLimit
                                          << "].");

    // get or create the environment as needed
    int ret(0);
    if (!DISKCACHE.openEnvironment())
    {
      ret = -1;
    }

    // do '.open' sentinel processing (if configured )
    if (LIKELY(ret == 0))
    {
      if (DISKCACHE.openFileSentinels())
      {
        if (access(openSentinel.c_str(), F_OK) == 0)
        {
          // If an "open" sentinel file exists, assume that the BDB was not closed
          // properly, so get rid of it and start fresh in case it might be corrupted.
          {
            PROTECT_DBENV
            DISKCACHE._dbEnv->dbremove(DISKCACHE._dbEnv, nullptr, dbFileName.c_str(), nullptr, 0);
          }
          removeFile(dbFilePath);
          removeFile(openSentinel);
        }
        else
        {
          // Otherwise, if the file does NOT exist, create it.
          ret = touch_file(openSentinel);
          if (ret != 0)
          {
            std::stringstream msg;
            msg << "Problem creating sentinel file [" << openSentinel << "] - " << strerror(ret);
            LOG4CXX_ERROR(getLogger(), msg.str());
            TseUtil::alert(msg.str().c_str());
            {
              PROTECT_DBENV
              DISKCACHE._dbEnv->dbremove(DISKCACHE._dbEnv, nullptr, dbFileName.c_str(), nullptr, 0);
            }
            removeFile(dbFilePath);
            removeFile(openSentinel);
          }
        }
      }
    }

    // Create the DB handle
    if (LIKELY(ret == 0))
    {
      {
        PROTECT_DBENV
        ret = db_create(&db, DISKCACHE._dbEnv, 0);
      }

      if (ret != 0)
      {
        LOG4CXX_ERROR(DISKCACHE.getLogger(),
                      "Unable to create BerkeleyDB handle for ["
                          << dbFileName << "]: " << BDB_ERROR_STRING(ret, "create"));
        db = nullptr;
      }
    }

    // do verify if configured
    if ((db != nullptr) && (DISKCACHE.dbVerifyOnOpen()) && dbFileExists)
    {
      // before we open an existing db for real, run a verification on it
      LOG4CXX_INFO(DISKCACHE.getLogger(),
                   "Verifying Berkeley DB [" << dbFileName << "] (db_verify).");
      ret = db->verify(db, dbFilePath.c_str(), nullptr, nullptr, 0);
      if (ret != 0)
      {
        // if it doesn't pass verification, all we can do now is just remove it
        // and start fresh
        std::ostringstream vmsg;
        vmsg << "Removing LDC file [" << dbFileName
             << "] - verification failed: " << BDB_ERROR_STRING(ret, "open");
        LOG4CXX_ERROR(DISKCACHE.getLogger(), vmsg.str());
        TseUtil::alert(vmsg.str().c_str());
        {
          PROTECT_DBENV
          DISKCACHE._dbEnv->dbremove(DISKCACHE._dbEnv, nullptr, dbFileName.c_str(), nullptr, 0);
        }
        removeFile(dbFilePath);
      }

      // cannot reuse the db handle after a verification, so we must recreate it
      {
        PROTECT_DBENV
        ret = db_create(&db, DISKCACHE._dbEnv, 0);
      }

      if (ret != 0)
      {
        LOG4CXX_ERROR(DISKCACHE.getLogger(),
                      "Unable to create BerkeleyDB handle for ["
                          << dbFileName << "]: " << BDB_ERROR_STRING(ret, "create"));
        db = nullptr;
      }
    }

    // Open the DB file
    if (LIKELY(db != nullptr))
    {
      db->set_pagesize(db, compressionLimit);
      db->set_flags(db, DB_CHKSUM);

      u_int32_t openFlags = DB_THREAD;

      if (DISKCACHE.useTransactionModel())
      {
        openFlags |= DB_AUTO_COMMIT;
      }

      if (!dbFileExists)
      {
        openFlags |= DB_CREATE;
      }

      isReadOnly = false;
      if (LIKELY(dbFileExists))
      {
        if (readOnly)
        {
          openFlags |= DB_RDONLY;
          isReadOnly = true;
        }
      }

      ret = db->open(db, nullptr, dbFileName.c_str(), nullptr, DB_BTREE, openFlags, 0);

      //  If we are unable to open the db we also need to check if there is an existing DB file.
      //  If there there is, it *may* indicate corruption, so we will remove it and reopen a new
      // one.
      if ((ret != 0) && dbFileExists)
      {
        std::string errmsg(strerror(ret));
        LOG4CXX_WARN(getLogger(), "Problem opening BDB file [" << dbFileName << "] - " << errmsg);
        {
          PROTECT_DBENV
          DISKCACHE._dbEnv->dbremove(DISKCACHE._dbEnv, nullptr, dbFileName.c_str(), nullptr, 0);
        }
        removeFile(dbFilePath);
        dbFileExists = false;
        LOG4CXX_WARN(DISKCACHE.getLogger(),
                     "Existing BDB file [" << dbFileName << "] was removed due to 'open' error.");

        openFlags = DB_THREAD | DB_CREATE;
        if (DISKCACHE.useTransactionModel())
        {
          openFlags |= DB_AUTO_COMMIT;
        }

        ret = db->open(db, nullptr, dbFileName.c_str(), nullptr, DB_BTREE, openFlags, 0);
      }

      if (UNLIKELY(ret != 0))
      {
        LOG4CXX_ERROR(DISKCACHE.getLogger(),
                      "Unable to open BerkeleyDB handle for ["
                          << dbFileName << "]: " << BDB_ERROR_STRING(ret, "write"));
        db = nullptr;
      }
    }

    if (db != nullptr)
    {
      LOG4CXX_DEBUG(DISKCACHE.getLogger(), "Opened BerkeleyDB handle for [" << dbFileName << "].");

      // Create the "control" info file if needed
      if (access(ctlFile.c_str(), F_OK) != 0)
      {
        char hostname[1024] = "\0";
        ::gethostname(hostname, 1023);

        char pwd[PATH_MAX + 1] = "\0";
        getcwd(pwd, PATH_MAX);

        std::ofstream ofs(ctlFile.c_str(), std::ios::out);
        ofs << LDC_CACHE_NAME << "=" << name << std::endl;
        ofs << LDC_TABLE_VERSION << "=" << ctrl->tableVersion() << std::endl;
        ofs << LDC_CACHE_TYPE << "=" << ctrl->getCacheType() << std::endl;
        ofs << LDC_DATA_FORMAT << "=" << DISKCACHE.getDataFormatAsString(name) << std::endl;
        ofs << LDC_CREATE_TIMESTAMP << "="
            << "\"" << DateTime(time(nullptr)).toSimpleString() << "\"" << std::endl;
        ofs << LDC_CREATE_HOST << "=" << hostname << std::endl;
        ofs << LDC_PROCESS_DIR << "=" << pwd << std::endl;
        ofs.close();
      }
    }

  } // end of dbMutex scope

  if (UNLIKELY(db == nullptr))
  {
    std::stringstream msg;
    msg << "Unable to acquire LDC handle for cache [" << name << "]."
        << "  LDC will be TURNED OFF for this cache !!!";
    LOG4CXX_ERROR(DISKCACHE.getLogger(), msg.str());
    TseUtil::alert(msg.str().c_str());
    disableCloseAndRemove();
  }
}

void
DiskCache::CacheTypeOptions::dbClose()
{
  { // scope for dbMutex
    boost::lock_guard<boost::mutex> g(dbMutex);
    if (db != nullptr)
    {
      db->close(db, 0);
      db = nullptr;
    }
  }

  if (UNLIKELY(DISKCACHE.openFileSentinels()))
  {
    // Get the CacheRegistry pointer for this cache if necessary
    if (ctrl == nullptr)
    {
      ctrl = CacheRegistry::instance().getCacheControl(name);
      if (ctrl == nullptr)
      {
        LOG4CXX_ERROR(getLogger(), "Unable to find cache [" << name << "] in Cache Registry !!!");
        return;
      }
    }

    std::string dbFileName(DISKCACHE.constructDBFileName(name, ctrl->tableVersion()));
    std::string openSentinel(DISKCACHE._directory);
    openSentinel.append("/");
    openSentinel.append(dbFileName);
    openSentinel.append(TABLE_OPEN_FILE_EXT);
    if (access(openSentinel.c_str(), F_OK) == 0)
    {
      removeFile(openSentinel);
    }
  }
}

bool
DiskCache::CacheTypeOptions::dbRemove()
{
  bool retval(true);

  boost::lock_guard<boost::mutex> g(dbMutex);

  std::string dbFileName(DISKCACHE.constructDBFileName(name));
  if (!dbFileName.empty())
  {
    if (db != nullptr)
    {
      db->close(db, 0);
      db = nullptr;
    }

    {
      PROTECT_DBENV
      if (DISKCACHE._dbEnv != nullptr)
      {
        DISKCACHE._dbEnv->dbremove(DISKCACHE._dbEnv, nullptr, dbFileName.c_str(), nullptr, 0);
      }
    }

    retval = removeFile(dbFileName);
  }

  return retval;
}

DB*
DiskCache::CacheTypeOptions::dbGet(bool readOnly)
{
  // Open the DB file if not already open
  dbOpen(readOnly);

  // return the DB handle (could be NULL if there was a problem)
  return db;
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// DiskCache
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

DiskCache::~DiskCache()
{
  for (auto& elem : _cacheCtrl)
  {
    delete elem.second;
  }
  _cacheCtrl.clear();
}

Logger&
DiskCache::getLogger()
{
  static Logger logger("atseintl.DBAccess.DiskCache");
  return logger;
}

DiskCache* DiskCache::_instance = nullptr;
boost::mutex DiskCache::_mutex;

void
DiskCache::disableCloseAndRemove(const std::string& name)
{
  std::string uc(name);
  STR_TOUPPER(uc);
  CACHE_TYPE_CTRL::const_iterator ctc = _cacheCtrl.find(uc);
  if (ctc != _cacheCtrl.end())
  {
    CacheTypeOptions* cto((*ctc).second);
    cto->disable();
    cto->dbClose();
    cto->dbRemove();
  }
}

DiskCache::CacheTypeOptions*
DiskCache::getCacheTypeOptions(const std::string& name) const
{
  std::string uc(name);
  STR_TOUPPER(uc);
  CACHE_TYPE_CTRL::const_iterator ctc = _cacheCtrl.find(uc);
  if (ctc != _cacheCtrl.end())
  {
    return (*ctc).second;
  }
  else
  {
    return nullptr;
  }
}

DiskCache::DataFormat
DiskCache::getDataFormat(const std::string& name) const
{
  CacheTypeOptions* cto = getCacheTypeOptions(name);
  if (cto != nullptr)
  {
    return cto->dataFormat;
  }
  else
  {
    return _defaultDataFormat;
  }
}

const char*
DiskCache::getDataFormatAsString(const std::string& name) const
{
  return DataFormats[getDataFormat(name)];
}

bool
DiskCache::doVerifyNextCycle()
{
  std::string fileName(_directory);
  fileName.append("/");
  fileName.append(VERIFY_NEXT_CYCLE_FILE);
  bool retval(access(fileName.c_str(), F_OK) == 0);
  if (retval)
  {
    removeFile(fileName);
  }
  return retval;
}

bool
DiskCache::isEnabled(const std::string& name) const
{
  CacheTypeOptions* cto = getCacheTypeOptions(name);
  if (cto != nullptr)
  {
    return cto->enabled;
  }
  else
  {
    return false;
  }
}

void
DiskCache::initialize(ConfigMan& config)
{
  boost::lock_guard<boost::mutex> g(_mutex);

  if (_instance == nullptr)
  {
    // Force the construction of boost date map before threads are
    // created.  Otherwise we have a constructor race condition in
    // multiple threads for ldc DeserializeTask's object keys.
    std::string date_time_string("2010-12-31 23:59:59.000");
    tse::DateTime translated(date_time_string);

    char buffer[32];
    memset(buffer, 0, sizeof(buffer));
    std::ostringstream os;
    std::string temp(buffer, sizeof(buffer));
    std::istringstream is(temp);
    os.rdbuf()->pubsetbuf(buffer, sizeof(buffer));
    os << translated;
    is >> translated;
    std::cerr << "DateTime object / boost preinitialization [" << buffer << "]" << std::endl;
    LOG4CXX_INFO(getLogger(), "DateTime object / boost preinitialization");

    _instance = new DiskCache(config);

    _instance->makeLDCDirectory();

    if (_instance->isActivated())
    {
      LOG4CXX_INFO(getLogger(), "LDC initialized and activated.");
    }
    else
    {
      LOG4CXX_INFO(getLogger(), "LDC initialized but NOT activated.");
    }
  }
}

bool
DiskCache::isActivated()
{
  return ((_instance != nullptr) && (_instance->_isActivated));
}

void
DiskCache::shutdown()
{
  static const timespec magic_sleep_interval = { 0, 100 };
  boost::lock_guard<boost::mutex> g(_mutex);
  if (DiskCache::isActivated())
  {
    _instance->_shuttingDown = true;
    _instance->_isActivated = false;

    while (_instance->_activeIO.load(std::memory_order_acquire) > 0)
    {
      nanosleep(&magic_sleep_interval, nullptr);
    }

    _instance->doneWithEnvironment(true);
    _instance->_shuttingDown = false;
    LOG4CXX_INFO(getLogger(), "LDC has been shut down.");
  }
}

bool
DiskCache::dataFormatFromString(const std::string& value, DataFormat& retval)
{
  bool success(true);
  std::string uc(value);
  STR_TOUPPER(uc);

  if (uc == DataFormats[DATAFMT_BN2])
  {
    retval = DATAFMT_BN2;
  }
  else
  {
    success = false;
  }

  return success;
}

DiskCache::DiskCache(tse::ConfigMan& config)
  : _config(config),
    _directory("../ldc"),
    _queueLogDir("."),
    _ldcMaxBlobSize(DEFAULT_MAX_LDC_BLOB_BYTES),
    _maxPerCycle(DEFAULT_MAX_OPERATIONS_PER_CYCLE)
{
  //----------------------------------------------
  // [DISK_CACHE_OPTIONS]
  //----------------------------------------------
  //
  // BERKELEY_CACHE_SIZE is used by the Berkeley DB API to allocate its internal
  // structures.  Expressed in MB.
  getConfigOption("BERKELEY_CACHE_SIZE", _berkeleyCacheSize, validateNum);

  // BERKELEY_DB_PAGE_SIZE sets the pagesize to use for the berkeley db's that are used to store the
  // ldc
  // This size is also used as the default size limit for compression of the objects being stored.
  // The object will be compressed if it is larger than the page size.  This can be overridded
  // for each cache below.  If compression is not desired then the page size should be set to 0 in
  // the
  // individual page sizes for each cache in the [DISK_CACHE_TYPES] config section.
  getConfigOption("BERKELEY_DB_PAGE_SIZE", _berkeleyDBPageSize, validateNumGreaterThanZero);

  // BERKELEY_REMOVE_DB_ENV causes BerkeleyDB environment files to wiped out on startup.  Not
  // recommended.
  getConfigOption("BERKELEY_REMOVE_DB_ENV", _shouldRemoveBerkeleyDBEnv, validateBoolean);

  // DB_VERIFY_ON_OPEN caused BerkeleyDB file verify utility to be run on a file every time it is
  // opened.
  // Not recommended.
  getConfigOption("DB_VERIFY_ON_OPEN", _dbVerifyOnOpen, validateBoolean);

  // DEFAULT_DATA_FORMAT is one of the following and can be overridden by cache type
  // in the [DISK_CACHE_TYPES] configuration section:
  //
  //  bn2 - TSE binary archive developed in-house
  //  bin - BOOST binary archive
  //  xml - BOOST xml archive
  //  txt - BOOST text archive
  getConfigOption("DEFAULT_DATA_FORMAT", _defaultDataFormat, validateDataFormat);

  // DEFAULT_MAX_CATCHUP ia the default maximim period of time between the most recent disk
  // update for a given cache and the server starting up, before it is no longer considered
  // viable to load the cache from disk.  This is expressed in minutes, and can be overridden
  // by cache type in the [DISK_CACHE_TYPES] configuration section.
  getConfigOption("DEFAULT_MAX_CATCHUP", _maxCatchupMinutes, validateNumGreaterThanZero);

  // DIRECTORY is the where the LDC data will be stored.
  getConfigOption("DIRECTORY", _directory, validateNotEmpty);

  // ENABLE_PRESTART_HOOK turns on LDC prestart routines in appmon_hook.sh.  Not really meaningful
  // at this point except for informational purposes.
  getConfigOption("ENABLE_PRESTART_HOOK", _enablePrestartHook, validateBoolean);

  // FULL_VERIFY_HOUR sets the our at which the DiskCacheAdapter verifies all files before
  // doing it's normal work.
  getConfigOption("FULL_VERIFY_HOUR", _fullVerifyHour, validateNum);

  // KEEP_DB_HANDLES_OPEN determines whether DB handles are kept open when not needed.
  getConfigOption("KEEP_DB_HANDLES_OPEN", _keepDbHandlesOpen, validateBoolean);

  // KEEP_ENV_HANDLE_OPEN determines whether the ENV handle is kept open when not needed.
  // Note that if ENV handle is closed, it implies that all DB handles will be closed as well.
  getConfigOption("KEEP_ENV_HANDLE_OPEN", _keepEnvHandleOpen, validateBoolean);

  // LOG_QUEUE_ACTIVITY is for developer use only!  It causes LDC action queue PUSH/POP
  // events to be written to a file (ldcq.<tablename>.log).  Don't turn this on unless
  // you are willing to take a performance hit as well as eat up disk space!
  getConfigOption("LOG_QUEUE_ACTIVITY", _logQueueActivity, validateBoolean);

  // OPEN_FILE_SENTINELS activates a scheme whereby sentinel files are are created on DB file opens
  // and removed on close.  This allows us to determine if a DB file was not properly closed.
  getConfigOption("OPEN_FILE_SENTINELS", _openFileSentinels, validateBoolean);

  // PRESTART_DB_VERIFY enables db_verify in appmon_hook.sh.  Not really meaningful at this point
  // except for informational purposes.
  getConfigOption("PRESTART_DB_VERIFY", _prestartDbVerify, validateBoolean);

  // PRESTART_RECOVER enables db_recover in appmon_hook.sh.  Not really meaningful at this point
  // except for informational purposes.
  getConfigOption("PRESTART_RECOVER", _prestartRecover, validateBoolean);

  // QUEUE_LOG_DIR is used in conjunction with the LOG_QUEUE_ACTIVITY switch, and specifies the
  // directory where the queue activity logs will be placed.
  getConfigOption("QUEUE_LOG_DIR", _queueLogDir, validateNotEmpty);

  // QUIESCE_BEFORE_READY determines whether tseserver "ready" state is delayed until LDC action
  // queues
  // (populated from a fresh DB startup) have been processed.
  getConfigOption("QUIESCE_BEFORE_READY", _quiesceBeforeReady, validateBoolean);

  // SERIALIZE_ENV_ACCESS ensure that only one API call involving dbEnv can occur at a time.
  getConfigOption("SERIALIZE_ENV_ACCESS", _serializeEnvAccess, validateBoolean);

  // SERIALIZE_QUEUE_THREAD_MAX determines how many simultaneous LDC serialization operations
  // can be processed.
  getConfigOption("SERIALIZE_QUEUE_THREAD_MAX", _serializeQueueThreadMax, validateSize);

  // MAX_ACTIVE_DESERIALIZE_TASKS controls the number of active DeserializeTask objects that
  // can be present the ThreadPool Task Queue
  getConfigOption("MAX_ACTIVE_DESERIALIZE_TASKS", _maxActiveDeserializeTasks, validateSize);

  // DESERIALIZE_SLEEP_MILLIS control the number of milli seconds to sleep when the deseralize task
  // queue exceeds the configured max limit set in the param MAX_ACTIVE_DESERIALIZE_TASKS
  getConfigOption("DESERIALIZE_SLEEP_MILLIS", _deserializeSleepMillis, validateNumGreaterThanZero);

  // STARTUP_ACTION is one of the following:
  //
  //  load    - load cache from local disk
  //  clear   - delete local disk cache and start over
  //  disable - turn off local disk cache; all other
  //            related options are moot
  getConfigOption("STARTUP_ACTION", _startAction, validateStartupAction);

  // THREAD_SLEEP_SECS is the interval at which the local disk cache synchronization
  // thread wakes up to do its work.
  getConfigOption("THREAD_SLEEP_SECS", _threadSleepSecs, validateNumGreaterThanZero);

  // USE_TXN_MODEL switches to the Berkeley DB transaction model, instead of the Concurrent
  // Data Store product.
  getConfigOption("USE_TXN_MODEL", _useTransactionModel, validateBoolean);

  // SHUTDOWN_DISKCACHE_BEFORE_ABORT shutdown the diskacache adapter before the abort call.
  bool shutdownDiskCacheBeforeAbort = false;
  getConfigOption("SHUTDOWN_DISKCACHE_BEFORE_ABORT", shutdownDiskCacheBeforeAbort, validateBoolean);

  //----------------------------------------------
  // stuff related to memCacheD
  //----------------------------------------------

  // DIST_CACHE_LOAD_MASTER - startup push to memcached.
  getConfigOption("DIST_CACHE_LOAD_MASTER", _distCacheLoadMaster, validateBoolean);

  // DIST_CACHE_EVENT_MASTER - delete cache event update items from dist cache
  getConfigOption("DIST_CACHE_EVENT_MASTER", _distCacheEventMaster, validateBoolean);

  // DIST_CACHE_ENABLED enables or disables using a distributed cache -- currently using
  // the libmemcached API.  Can be disabled per cache type via the [DISK_CACHE_TYPES] config
  // section.
  getConfigOption("DIST_CACHE_ENABLED", _distCacheEnabled, validateBoolean);

  // DEFAULT_DIST_CACHE_TTL - the default time to live in the distributed cache
  // in minutes. The default of 0 is unlimited.
  getConfigOption("DEFAULT_DIST_CACHE_TTL", _defaultTTL, validateNum);

  // DIST_CACHE_SERVERS - the server list in server colon port notation:
  // server:port pairs in the list are comma separated
  // libmemcache uses a commma, but config.getValue parses
  // commas, use a pipe '|' symbol to replace the comma
  // server1:port1[|server2:port2|...]
  getConfigOption("DIST_CACHE_SERVERS", _servers, validateNotEmpty);

  // DIST_CACHE_POOL_SIZE - the number of simultaneous connections to hold
  getConfigOption("DIST_CACHE_POOL_SIZE", _poolSize, validateNumGreaterThanZero);

  // MAX_OPERATIONS_PER_CYCLE - the maximum number of operations to process for one cycle per cache.
  getConfigOption("MAX_OPERATIONS_PER_CYCLE", _maxPerCycle, validateNum);

  //----------------------------------------------
  // End of DISK_CACHE_OPTIONS
  //----------------------------------------------

  // Option dependencies
  if (!_keepEnvHandleOpen)
  {
    _keepDbHandlesOpen = false;
  }

  // Fix up the format from tseserver.cfg compatible delimiter '|' to memcache delimiter of ','
  std::string& lhs(_servers);
  std::replace(lhs.begin(), lhs.end(), '|', ',');

  //----------------------------------------------
  // [DISK_CACHE_TYPES]
  //
  // Defines those cache types for which local disk
  // caching is to be enabled.  The value for each cache
  // type consists of a Y or N, followed by zero or more
  // options separated by a vertical bar (|).  Position
  // is important for the options, so be sure to include
  // vertical bars for each unspecified option preceding
  // the final option that IS specified.
  //
  // Options are:
  //
  //    DATA_FORMAT       (overrides DEFAULT_DATA_FORMAT)
  //    MAX_CATCHUP       (overrides DEFAULT_MAX_CATCHUP)
  //    COMPRESSION_LIMIT (overrides BERKELEY_DB_PAGE_SIZE)
  //
  // NOTE: COMPRESSION_LIMIT is used to override the default
  //       setting for which objects are compressed.  The
  //       default is the BERKELEY_DB_PAGE_SIZE which is an
  //       optimal value.  If compression is not desired
  //       then the limit should be set to 0.
  //
  // Example (no options specified):
  //
  //    VENDORCROSSREF = Y
  //
  // Example (all options specified):
  //
  //    VENDORCROSSREF = Y|xml|30
  //
  // Example (only "data format" specified):
  //
  //    VENDORCROSSREF = Y|xml
  //
  // Example (only "max catchup" specified):
  //
  //    VENDORCROSSREF = Y||30
  //
  // CAREFUL!  No embedded spaces in the "value" field.
  // By default, unlisted cache types are NOT enabled.
  //
  // <tablename>=<enabled>|<format>|<max_catchup>|<bdb_page_size>|<dist_cache_enabled>|<TTL>|<ldcMaxBlobSize>
  //
  //----------------------------------------------

  enum eOptionFieldSymbolicName
  {
    eEnableDisableTableField = 1,
    eSerializeDataFormatField,
    eMaxCatchUpMinutesField,
    eBerkeleyPageSize,
    eUseDistCacheField,
    eTTLField,
    eLdcMaxBlobSize
  };

  std::vector<tse::ConfigMan::NameValue> cfgValues;
  if (config.getValues(cfgValues, "DISK_CACHE_TYPES"))
  {
    boost::char_separator<char> fieldSep("|", "", boost::keep_empty_tokens);
    for (auto& cfgValue : cfgValues)
    {
      std::string& cacheName(cfgValue.name);
      CacheTypeOptions* cto(nullptr);
      boost::tokenizer<boost::char_separator<char>> fields(cfgValue.value, fieldSep);

      int num = 1;
      for (boost::tokenizer<boost::char_separator<char> >::const_iterator f = fields.begin();
           f != fields.end();
           ++f, ++num)
      {
        const std::string& field = (*f);
        switch (num)
        {
        case eEnableDisableTableField:
          cto = getCacheTypeOptions(cacheName);
          if (cto == nullptr)
          {
            cto = new CacheTypeOptions(cacheName,
                                       field == "Y" && _startAction != DISABLE_ON_START,
                                       _defaultDataFormat,
                                       _maxCatchupMinutes,
                                       nullptr,
                                       _berkeleyDBPageSize,
                                       _distCacheEnabled,
                                       _defaultTTL,
                                       _ldcMaxBlobSize);
            _cacheCtrl[cacheName] = cto;
          }
          break;

        case eSerializeDataFormatField:
          if (!field.empty())
          {
            DataFormat fmt(_defaultDataFormat);
            DiskCache::dataFormatFromString(field, fmt);
            if (fmt != _defaultDataFormat)
            {
              cto->dataFormat = fmt;
              LOG4CXX_INFO(getLogger(),
                           "NOTE: Data format [" << DataFormats[fmt] << "] for cache [" << cacheName
                                                 << "] overrides the default setting ["
                                                 << DataFormats[_defaultDataFormat] << "].");
            }

            // Sanity check against ACTUAL table directory in LDC for this type (if exists)
            std::string ctlFileValue;
            if (getLDCControlFileValue(_directory, cacheName, LDC_DATA_FORMAT, ctlFileValue))
            {
              if ((DiskCache::dataFormatFromString(ctlFileValue, fmt)) && (fmt != cto->dataFormat))
              {
                LOG4CXX_WARN(getLogger(),
                             "WARNING: Actual LDC format ["
                                 << DataFormats[fmt] << "] for cache [" << cacheName
                                 << "] trumps the configured format ["
                                 << DataFormats[cto->dataFormat] << "].");
                cto->dataFormat = fmt;
              }
            }
          }
          break;

        case eMaxCatchUpMinutesField:
          if (!field.empty())
          {
            uint32_t m(_maxCatchupMinutes);
            if (!stringToUint32(field, m))
            {
              LOG4CXX_ERROR(getLogger(), "Invalid value [" << field << "] for 'MAX_CATCHUP'");
            }
            if ((m != _maxCatchupMinutes) && (m != 0))
            {
              cto->maxCatchupMinutes = m;
              LOG4CXX_INFO(getLogger(),
                           "NOTE: Max catchup [" << m << "] for cache [" << cacheName
                                                 << "] overrides the default setting ["
                                                 << _maxCatchupMinutes << "].");
            }
          }
          break;

        case eBerkeleyPageSize:
          if (!field.empty())
          {
            uint32_t s(_berkeleyDBPageSize);
            if (!stringToUint32(field, s))
            {
              LOG4CXX_ERROR(getLogger(), "Invalid value [" << field << "] for 'COMPRESSION_LIMIT'");
            }
            if (s != _berkeleyDBPageSize)
            {
              cto->compressionLimit = s;
              LOG4CXX_INFO(getLogger(),
                           "NOTE: Limit for compression [" << s << "] for cache [" << cacheName
                                                           << "] overrides the default setting ["
                                                           << _berkeleyDBPageSize << "].");
            }
          }
          break;

        case eUseDistCacheField
            : // can disable a table/cache, but can't enable if DistCache is not enabled
        {
          std::string trimmed(trim(field));
          if (validateBoolean(trimmed, cto->useDistCache))
          {
            if (!_distCacheEnabled && cto->useDistCache)
            {
              LOG4CXX_DEBUG(getLogger(), "disabling memcached for " << cacheName);
              cto->useDistCache = false;
            }
            else if (_distCacheEnabled && !cto->useDistCache)
              LOG4CXX_DEBUG(getLogger(), "memcached disabled for " << cacheName);
          }
        }
        break;

        case eTTLField: // ttl
          if (!field.empty())
          {
            size_t ttl = atoi(field.c_str());
            if (ttl && ttl != _defaultTTL)
            {
              cto->ttl = ttl;
              LOG4CXX_DEBUG(getLogger(),
                            "NOTE: ttl [" << cto->ttl << "] for type [" << cacheName
                                          << "] overrides the default of [" << _defaultTTL << "].");
            }
          }
          break;
        case eLdcMaxBlobSize: // Maximum size to write to LDC
          if (!field.empty())
          {
            size_t size = atoi(field.c_str());
            if (size)
            {
              cto->ldcMaxBlobSize = size;
              LOG4CXX_DEBUG(getLogger(),
                            "NOTE: ldcMaxBlobSize [" << size << "] for type [" << cacheName
                                                     << "] overrides the default of ["
                                                     << _ldcMaxBlobSize << "].");
            }
          }
          break;
        }
      }
    }
  }

  if (_startAction != DISABLE_ON_START)
  {
    _isActivated = true;
    if (_startAction == CLEAR_ON_START)
    {
      removeDirectory(_directory);
    }
    else
    {
      std::string clearNextSentinel(_directory);
      clearNextSentinel.append("/");
      clearNextSentinel.append(CLEAR_NEXT_START_FILE);
      if (access(clearNextSentinel.c_str(), F_OK) == 0)
      {
        LOG4CXX_INFO(getLogger(),
                     "Clearing LDC directory [" << _directory << "] - found "
                                                << CLEAR_NEXT_START_FILE << " file.");
        removeDirectory(_directory);
      }
    }

    if (shutdownDiskCacheBeforeAbort)
    {
      LOG4CXX_INFO(getLogger(), "SHUTDOWN_DISKCACHE_BEFORE_ABORT signal handler installed");
      prevABRT = signal(SIGABRT, handleShutdownBeforeAbort);
    }
  }

  if (_logQueueActivity)
  {
    if (access(_queueLogDir.c_str(), F_OK) != 0)
    {
      std::string cmd("mkdir -p -m 755 ");
      cmd.append(_queueLogDir.c_str());
      if (system(cmd.c_str()) != 0)
      {
        LOG4CXX_ERROR(getLogger(),
                      "Unable to create queue log directory [" << _queueLogDir << "] !!!");
        _logQueueActivity = false;
      }
    }
  }

  showEligible();
}

bool
DiskCache::makeLDCDirectory()
{
  static bool firstTime = true;

  std::string path(_directory);

  bool retval = true;
  int rc(0);

  if (access(path.c_str(), F_OK) != 0)
  {
    std::string cmd("mkdir -p -m 755 ");
    cmd.append(path.c_str());
    rc = system(cmd.c_str());
    if (rc != 0)
    {
      std::string errmsg(strerror(rc));
      LOG4CXX_ERROR(getLogger(), "Problem creating LDC directory [" << path << "] - " << errmsg);
      retval = false;
    }
  }
  else if (DISKCACHE.shouldRemoveBerkeleyDBEnv() && firstTime)
  {
    LOG4CXX_INFO(getLogger(), "Removing pre-existing BerkeleyDB environment files.");
    std::string bakPath(path);
    bakPath.append("/bak");

    std::string cmd("mkdir -p -m 755 ");
    cmd.append(bakPath.c_str());
    int rc(system(cmd.c_str()));

    if (rc != 0)
    {
      std::string errmsg(strerror(rc));
      LOG4CXX_ERROR(getLogger(),
                    "Problem creating directory [" << bakPath << "] - ERRNO=" << rc
                                                   << " MSG=" << errmsg);
      retval = false;
    }
    else
    {
      DIR* dir = opendir(path.c_str());
      if (dir != nullptr)
      {
        struct dirent* entry = nullptr;
        std::string dbFilePath;
        for (; nullptr != (entry = readdir(dir));)
        {
          std::string fname(entry->d_name);
          size_t len = fname.size();
          if ((len > 5) && (fname.compare(0, 5, "__db.") == 0))
          {
            int num(atoi(fname.substr(5).c_str()));
            if (num > 0)
            {
              std::string dbFilePath(path);
              dbFilePath += '/';
              dbFilePath.append(fname);
              cmd = "/bin/mv -f ";
              cmd.append(dbFilePath);
              cmd.append(" ");
              cmd.append(bakPath);
              if (system(cmd.c_str()) == 0)
              {
                LOG4CXX_INFO(getLogger(), "Moved [" << dbFilePath << "] to [" << bakPath << "].");
              }
              else
              {
                std::string errmsg(strerror(errno));
                LOG4CXX_ERROR(getLogger(),
                              "Problem moving [" << dbFilePath << "] to [" << bakPath << "] - "
                                                 << errmsg);
                retval = false;
              }
            }
          }
        }
        closedir(dir);
      }
    }
  }
  else
  {
    LOG4CXX_INFO(getLogger(), "Pre-existing BerkeleyDB environment files will be retained.");
  }

  firstTime = false;

  return retval;
}

bool
DiskCache::removeDirectory(const std::string& path)
{
  bool retval = true;
  int rc(0);

  DIR* pdir = opendir(path.c_str());
  if (pdir != nullptr)
  {
    struct dirent* pent = nullptr;
    struct stat st;

    while ((pent = readdir(pdir)) != nullptr)
    {
      std::string baseName(pent->d_name);
      if ((baseName != "..") && (baseName != "."))
      {
        std::string dbFilePath(path);
        dbFilePath += '/';
        dbFilePath.append(baseName);

        rc = lstat(dbFilePath.c_str(), &st);
        if (rc == 0)
        {
          if (S_ISDIR(st.st_mode))
          {
            // recursive call
            if (!removeDirectory(dbFilePath))
            {
              retval = false;
            }
          }
          else
          {
            retval = removeFile(dbFilePath);
          }
        }
        else
        {
          std::string errmsg(strerror(rc));
          LOG4CXX_WARN(getLogger(),
                       "Problem getting file information for [" << path << "] - " << errmsg);
          retval = false;
        }
      }
    }

    closedir(pdir);

    rc = rmdir(path.c_str());
    if (rc != 0)
    {
      std::string errmsg(strerror(rc));
      LOG4CXX_WARN(getLogger(), "Problem removing directory [" << path << "] - " << errmsg);
      retval = false;
    }
  }
  else
  {
    std::string errmsg(strerror(errno));
    LOG4CXX_WARN(getLogger(), "Problem opening directory [" << path << "] - " << errmsg);
    retval = false;
  }

  return retval;
}

bool
DiskCache::removeFile(const std::string& path)
{
  bool retval = true;
  int rc(0);
  if (access(path.c_str(), F_OK) == 0)
  {
    rc = unlink(path.c_str());
    if (rc != 0)
    {
      std::string errmsg(strerror(rc));
      LOG4CXX_WARN(getLogger(), "Problem removing file [" << path << "] - " << errmsg);
      retval = false;
    }
  }
  return retval;
}

std::string
DiskCache::constructDBFileName(const std::string& cacheName, int version)
{
  std::ostringstream os;

  std::string uc(cacheName);
  STR_TOUPPER(uc);

  if (version == -1)
  {
    CacheControl* ctrl = CacheRegistry::instance().getCacheControl(cacheName);
    if (ctrl == nullptr)
    {
      LOG4CXX_ERROR(getLogger(),
                    "Unable to find cache [" << cacheName << "] in Cache Registry !!!");
      return std::string();
    }
    else
    {
      version = ctrl->tableVersion();
    }
  }

  os << uc << "." << version << ".db";

  return os.str();
}

DB_ENV*
DiskCache::createDBEnvironment(u_int32_t openFlags, const char* mode)
{
  DB_ENV* retval(nullptr);

  if (bdbDisabled())
  {
    return retval;
  }

  LOG4CXX_DEBUG(getLogger(),
                "Initializing the Berkeley DB subsystem for LDC (" << mode << " mode).");
  int ret(db_env_create(&retval, 0));
  if (ret != 0)
  {
    LOG4CXX_ERROR(getLogger(),
                  "Berkeley DB environment failure: " << BDB_ERROR_STRING(ret, "db_env_create"));
    retval = nullptr;
  }

  if (retval)
  {
    LOG4CXX_DEBUG(getLogger(), "Setting BDB event notifier LDC (" << mode << " mode).");
    ret = retval->set_event_notify(retval, bdbEventHandler);
    if (ret != 0)
    {
      LOG4CXX_ERROR(
          getLogger(),
          "Berkeley DB environment failure: " << BDB_ERROR_STRING(ret, "set_event_notify"));
      retval = nullptr;
    }
  }

  if (retval)
  {
    LOG4CXX_DEBUG(getLogger(), "Enabling deadlock detector LDC (" << mode << " mode).");
    ret = retval->set_lk_detect(retval, DB_LOCK_YOUNGEST);
    if (ret != 0)
    {
      LOG4CXX_ERROR(getLogger(),
                    "Berkeley DB environment failure: " << BDB_ERROR_STRING(ret, "set_lk_detect"));
      retval = nullptr;
    }
  }

  if (retval && (_berkeleyCacheSize > 0))
  {
    LOG4CXX_DEBUG(getLogger(),
                  "Setting the Berkeley DB Cache size to [" << _berkeleyCacheSize << "] MB ("
                                                            << mode << " mode).");
    ret = retval->set_cachesize(retval, 0, (_berkeleyCacheSize * 1024 * 1024), 1);
    if (ret != 0)
    {
      LOG4CXX_ERROR(getLogger(),
                    "Berkeley DB environment failure: " << BDB_ERROR_STRING(ret, "set_cachesize"));
      retval = nullptr;
    }
  }

  if (retval)
  {
    LOG4CXX_DEBUG(getLogger(), "Opening Berkeley DB environment (" << mode << " mode).");
    ret = retval->open(retval, _directory.c_str(), openFlags, 0);
    if (ret != 0)
    {
      LOG4CXX_ERROR(getLogger(),
                    "Berkeley DB environment failure: " << BDB_ERROR_STRING(ret, "open"));
      retval = nullptr;
    }
  }

  return retval;
}

void
DiskCache::doneWithEnvironment(bool forceClose)
{
  bool closeEnv(forceClose || (!keepEnvHandleOpen()));
  bool closeDB(forceClose || (!keepDbHandlesOpen()) || closeEnv);

  if (closeEnv)
  {
    LOG4CXX_INFO(getLogger(), "Closing all Berkeley DB and ENV handles.");
  }
  else if (closeDB)
  {
    LOG4CXX_INFO(getLogger(), "Closing all Berkeley DB handles.");
  }

  if (closeDB)
  {
    for (auto& elem : _cacheCtrl)
    {
      CacheTypeOptions* cto(elem.second);
      LOG4CXX_DEBUG(getLogger(), "Closing Berkeley DB for cache [" << elem.first << "].");
      cto->dbClose();
    }

    if (useTransactionModel())
    {
      PROTECT_DBENV
      int rc(_dbEnv->txn_checkpoint(_dbEnv, 0, 0, DB_FORCE));
      if (rc == 0)
      {
        rc = _dbEnv->log_archive(_dbEnv, nullptr, DB_ARCH_REMOVE);
        if (rc != 0)
        {
          LOG4CXX_ERROR(getLogger(),
                        "Problem closing BDB environment:" << BDB_ERROR_STRING(rc, "log_archive"));
        }
      }
      else
      {
        LOG4CXX_ERROR(getLogger(),
                      "Problem closing BDB environment:" << BDB_ERROR_STRING(rc, "txn_checkpoint"));
      }
    }
  }

  PROTECT_DBENV
  if ((closeEnv) && (_dbEnv != nullptr))
  {
    _dbEnv->close(_dbEnv, 0);
    _dbEnv = nullptr;
  }
}

bool
DiskCache::openEnvironment()
{
  if (bdbDisabled())
  {
    return false;
  }

  if (!isActivated())
  {
    return false;
  }

  PROTECT_DBENV

  if (_dbEnv == nullptr)
  {
    // Open in recovery mode first
    LOG4CXX_INFO(getLogger(), "Opening Berkeley DB environment in RECOVER mode.");
    DB_ENV* recoverEnv = createDBEnvironment(DB_CREATE | DB_INIT_TXN | DB_RECOVER, "RECOVER");
    if (recoverEnv != nullptr)
    {
      recoverEnv->close(recoverEnv, 0);
      if (db_env_create(&recoverEnv, 0) == 0)
      {
        recoverEnv->remove(recoverEnv, _directory.c_str(), DB_FORCE);
        recoverEnv = nullptr;

        // All is well, so open for real

        u_int32_t flags(DB_CREATE | DB_THREAD | DB_INIT_MPOOL);
        std::string envType;

        if (useTransactionModel())
        {
          flags |= (DB_INIT_LOCK | DB_INIT_LOG | DB_INIT_TXN);
          envType = "TXN";
        }
        else
        {
          flags |= DB_INIT_CDB;
          envType = "CDB";
        }

        LOG4CXX_INFO(getLogger(), "Opening Berkeley DB environment in " << envType << "  mode.");
        _dbEnv = createDBEnvironment(flags, envType.c_str());
      }
    }

    if (_dbEnv == nullptr)
    {
      LOG4CXX_ERROR(getLogger(), "Unable to open Berkeley DB Environment !!!");
      sendPanicEvent();
    }
  }

  return (_dbEnv != nullptr);
}

bool
DiskCache::removeDB(const std::string& name)
{
  bool retval(false);

  LOG4CXX_INFO(getLogger(), "Removing Berkeley DB for cache [" << name << "].");

  CacheTypeOptions* cto = getCacheTypeOptions(name);
  if (cto == nullptr)
  {
    LOG4CXX_ERROR(getLogger(),
                  "DiskCache::removeDB: Cache [" << name << "] not defined in CacheTypeOptions!!!");
  }
  else
  {
    retval = cto->dbRemove();
  }

  return retval;
}

void
DiskCache::doneWithDB(const std::string& name)
{
  if (DISKCACHE.isEnabled(name))
  {
    if (LIKELY(!keepDbHandlesOpen()))
    {
      CacheTypeOptions* cto(getCacheTypeOptions(name));
      if (cto == nullptr)
      {
        LOG4CXX_ERROR(getLogger(),
                      "DiskCache::doneWithDB: Cache [" << name
                                                       << "] not defined in CacheTypeOptions!!!");
      }
      else
      {
        cto->dbClose();
      }
    }
  }
}

DB*
DiskCache::getDB(const std::string& name, bool readOnly, CacheTypeOptions* cto)
{
  DB* db = nullptr;

  if (cto == nullptr)
  {
    cto = getCacheTypeOptions(name);
  }

  if (UNLIKELY(cto == nullptr))
  {
    LOG4CXX_ERROR(getLogger(),
                  "DiskCache::getDB: Cache [" << name << "] not defined in CacheTypeOptions!!!");
  }
  else
  {
    db = cto->dbGet(readOnly);
  }

  return db;
}

bool
DiskCache::writeToDB(const std::string& name, const std::string& key, DataBlob& blob)
{
  CacheTypeOptions* cto = getCacheTypeOptions(name);
  if (UNLIKELY((cto == nullptr) || (!cto->enabled) || (bdbDisabled())))
  {
    return false;
  }

  {
    boost::lock_guard<boost::mutex> g(_mutex);
    if (UNLIKELY(!isActivated()))
    {
      return false;
    }
    _activeIO.fetch_add(1, std::memory_order_release);
  }

  bool retval(true);
  std::string reason;
  DBT dbKey;
  DBT dbData;
  int ret(0);

  try
  {
    DB* db = getDB(name, false, cto);
    if (db != nullptr)
    {
      memset(&dbKey, 0x00, sizeof(DBT));
      memset(&dbData, 0x00, sizeof(DBT));

      dbKey.size = key.size();
      dbKey.data = reinterpret_cast<void*>(const_cast<char*>(key.data()));

      dbData.data = blob.getRawData(&dbData.size);

      if (UNLIKELY((ret = db->put(db, nullptr, &dbKey, &dbData, 0)) != 0))
      {
        std::ostringstream os;
        os << BDB_ERROR_STRING(ret, "write");
        reason = os.str();
        retval = false;
      }
    }
    else
    {
      retval = false;
    }
  }
  catch (std::exception& e)
  {
    retval = false;
    reason = e.what();
  }
  catch (...)
  {
    retval = false;
    reason = "unknown exception was thrown";
  }

  if (retval)
  {
    LOG4CXX_INFO(getLogger(),
                 "Wrote object to LDC.  Cache=[" << name << "].  Key=[" << key << "].");
  }
  else
  {
    std::ostringstream msg;
    msg << "ALERT!  LDC write failure for [" << name << "] key [" << key << "] !!!  " << reason;
    LOG4CXX_ERROR(DISKCACHE.getLogger(), msg.str());
    tse::TseUtil::alert(msg.str().c_str());
    disableCloseAndRemove(name);
  }

  _activeIO.fetch_sub(1, std::memory_order_release);
  return retval;
}

DiskCache::DBRead
DiskCache::readFromDB(const std::string& name, std::string& key, DataBlob* blob)
{
  CacheTypeOptions* cto = getCacheTypeOptions(name);
  if ((cto == nullptr) || (!cto->enabled) || (bdbDisabled()))
  {
    return DBREAD_FAILURE;
  }

  {
    boost::lock_guard<boost::mutex> g(_mutex);
    if (!isActivated())
    {
      return DBREAD_FAILURE;
    }
    _activeIO.fetch_add(1, std::memory_order_release);
  }

  DBRead retval(DBREAD_FAILURE);
  std::string reason;
  DBT dbKey;
  DBT dbData;
  int ret = 0;

  try
  {
    DB* db = getDB(name, true, cto);
    if (db)
    {
      memset(&dbKey, 0x00, sizeof(DBT));
      memset(&dbData, 0x00, sizeof(DBT));

      dbKey.size = key.size();
      dbKey.data = reinterpret_cast<void*>(const_cast<char*>(key.data()));
      dbKey.flags = DB_DBT_USERMEM;

      dbData.flags = DB_DBT_MALLOC;

      LOG4CXX_DEBUG(getLogger(),
                    "Reading Object from LDC Cache [" << name << "], Key [" << key << "].");

      ret = db->get(db, nullptr, &dbKey, &dbData, 0);
      if (ret == 0)
      {
        if (blob->setRawData(reinterpret_cast<char*>(dbData.data), dbData.size) != false)
        {
          retval = DBREAD_SUCCESS;
        }
      }

      if (dbData.data != nullptr)
      {
        free(dbData.data);
      }
    }
  }
  catch (std::exception& e)
  {
    retval = DBREAD_FAILURE;
    reason = e.what();
  }
  catch (...)
  {
    retval = DBREAD_FAILURE;
    reason = "unknown exception was thrown";
  }

  if (reason.length())
  {
    std::ostringstream msg;
    msg << "ALERT!  LDC read failure for [" << name << "] key [" << key << "] !!!  " << reason;
    LOG4CXX_ERROR(DISKCACHE.getLogger(), msg.str());
    tse::TseUtil::alert(msg.str().c_str());
    disableCloseAndRemove(name);
  }

  _activeIO.fetch_sub(1, std::memory_order_release);
  return retval;
}

DiskCache::DBRead
DiskCache::readNextFromDB(DBC** dbCur, const std::string& name, std::string& key, DataBlob* blob)
{
  CacheTypeOptions* cto = getCacheTypeOptions(name);
  if (UNLIKELY((cto == nullptr) || (!cto->enabled) || (bdbDisabled())))
  {
    return DBREAD_FAILURE;
  }

  {
    boost::lock_guard<boost::mutex> g(_mutex);
    if (UNLIKELY(!isActivated()))
    {
      return DBREAD_FAILURE;
    }
    _activeIO.fetch_add(1, std::memory_order_release);
  }

  DBRead retval(DBREAD_FAILURE);
  DBT dbKey;
  DBT dbData;
  int ret = 0;
  std::string failureMessage;

  DB* db = getDB(name, true);
  if (LIKELY(db))
  {
    memset(&dbKey, 0x00, sizeof(DBT));
    memset(&dbData, 0x00, sizeof(DBT));

    if (*dbCur == nullptr)
    {
      ret = db->cursor(db, nullptr, dbCur, 0);
    }

    if (LIKELY(ret == 0))
    {
      ret = (*dbCur)->get(*dbCur, &dbKey, &dbData, DB_NEXT);
      if (ret == 0)
      {
        if (LIKELY(blob->setRawData(reinterpret_cast<char*>(dbData.data), dbData.size)))
        {
          key.clear();
          key.insert(0, reinterpret_cast<char*>(dbKey.data), dbKey.size);

          LOG4CXX_DEBUG(getLogger(),
                        "Read Next Object from LDC Cache [" << name << "], Key [" << key << "].");

          retval = DBREAD_SUCCESS;
        }
        else
        {
          failureMessage = "CRC check failure (SHA1)";
        }
      }
      else
      {
        if (ret == DB_NOTFOUND)
        {
          retval = DBREAD_DONE;
        }

        (*dbCur)->close(*dbCur);
        *dbCur = nullptr;
      }
    }

    if (UNLIKELY(retval == DiskCache::DBREAD_FAILURE))
    {
      std::ostringstream msg;
      msg << "ALERT!  LDC 'read next' failure for [" << name << "] !!!  ";
      if (failureMessage.empty())
      {
        msg << BDB_ERROR_STRING(ret, "read");
      }
      else
      {
        msg << failureMessage;
      }
      LOG4CXX_ERROR(DISKCACHE.getLogger(), msg.str());
      tse::TseUtil::alert(msg.str().c_str());
    }
  }

  _activeIO.fetch_sub(1, std::memory_order_release);
  return retval;
}

bool
DiskCache::deleteFromDB(const std::string& name, const std::string& key)
{
  CacheTypeOptions* cto = getCacheTypeOptions(name);
  if (UNLIKELY((cto == nullptr) || (!cto->enabled) || (DISKCACHE.bdbDisabled())))
  {
    return false;
  }

  {
    boost::lock_guard<boost::mutex> g(_mutex);
    if (UNLIKELY(!isActivated()))
    {
      return false;
    }
    _activeIO.fetch_add(1, std::memory_order_release);
  }

  bool retval(true);
  std::string reason;

  int ret(0);

  DB* db = getDB(name);
  if (LIKELY(db))
  {
    DBT dbKey;

    memset(&dbKey, 0x00, sizeof(DBT));

    dbKey.size = key.size();
    dbKey.data = reinterpret_cast<void*>(const_cast<char*>(key.data()));

    ret = db->del(db, nullptr, &dbKey, 0);
    if (UNLIKELY((ret != 0) && (ret != DB_NOTFOUND)))
    {
      std::ostringstream os;
      os << BDB_ERROR_STRING(ret, "delete");
      reason = os.str();
      retval = false;
    }
  }

  if (LIKELY(retval))
  {
    LOG4CXX_INFO(getLogger(),
                 "Removed object from LDC.  Cache=[" << name << "].  Key=[" << key << "].");
  }
  else
  {
    std::ostringstream msg;
    msg << "ALERT!  LDC delete failure for [" << name << "] key [" << key << "] !!!  " << reason;
    LOG4CXX_ERROR(DISKCACHE.getLogger(), msg.str());
    tse::TseUtil::alert(msg.str().c_str());
  }

  _activeIO.fetch_sub(1, std::memory_order_release);
  return retval;
}

bool
DiskCache::truncateDB(const std::string& name)
{
  CacheTypeOptions* cto = getCacheTypeOptions(name);
  if ((cto == nullptr) || (!cto->enabled) || (DISKCACHE.bdbDisabled()))
  {
    return false;
  }

  {
    boost::lock_guard<boost::mutex> g(_mutex);
    if (!isActivated())
    {
      return false;
    }
    _activeIO.fetch_add(1, std::memory_order_release);
  }

  bool retval(true);
  int ret(0);
  uint32_t numDeleted(0);
  DB* db(getDB(name));

  if (db != nullptr)
  {
    ret = db->truncate(db, nullptr, &numDeleted, 0);
    if (ret == 0)
    {
      LOG4CXX_INFO(getLogger(), "Truncated LDC file.  Cache=[" << name << "].");
    }
    else
    {
      std::ostringstream os;
      os << "ALERT!  LDC truncate failure for [" << name << "] !!!  "
         << BDB_ERROR_STRING(ret, "truncate");
      LOG4CXX_ERROR(DISKCACHE.getLogger(), os.str());
      tse::TseUtil::alert(os.str().c_str());
      retval = false;
    }
  }
  else
  {
    retval = false;
  }

  _activeIO.fetch_sub(1, std::memory_order_release);
  return retval;
}

size_t
DiskCache::sizeAllQueues() const
{
  size_t retval(0);

  CacheRegistry& registry = CacheRegistry::instance();
  for (const auto& ctcit : _cacheCtrl)
  {
    CacheControl* ctl = registry.getCacheControl(ctcit.first);
    if (ctl != nullptr)
    {
      size_t sz(ctl->actionQueueSize());
      if (sz > 0)
      {
        retval += sz;
        LOG4CXX_DEBUG(getLogger(),
                      "LDC action queue for [" << ctl->getID() << "] contains [" << sz
                                               << "] entries.");
      }
    }
  }

  return retval;
}

void
DiskCache::clearAllActionQueues()
{
  CacheRegistry& registry = CacheRegistry::instance();
  for (CACHE_TYPE_CTRL::const_iterator ctcit = _cacheCtrl.begin(); ctcit != _cacheCtrl.end();
       ++ctcit)
  {
    CacheControl* ctl = registry.getCacheControl((*ctcit).first);
    if (ctl != nullptr)
    {
      ctl->actionQueueClear();
    }
  }
}

void
DiskCache::waitForQueueToSubside()
{
  if ((_isActivated) && (_quiesceBeforeReady))
  {
    size_t startSize(sizeAllQueues());
    if (startSize > 0)
    {
      size_t msgThreshold(startSize);
      size_t sz(startSize);
      LOG4CXX_INFO(getLogger(), "Waiting for LDC action queues to clear...");
      while ((sz > 0) && (_isActivated))
      {
        if (sz <= msgThreshold)
        {
          LOG4CXX_INFO(getLogger(), "Total size of LDC action queues = [" << sz << "]");
          msgThreshold /= 2;
        }
        sleep(1);
        sz = sizeAllQueues();
      }
      LOG4CXX_INFO(getLogger(), "LDC action queues have been cleared.");
    }
  }
}

void
DiskCache::removeObsoleteTableVersions()
{
  if (isActivated() && (!bdbDisabled()) && !shuttingDown())
  {
    {
      boost::lock_guard<boost::mutex> g(_mutex);
      if (!isActivated())
      {
        return;
      }
      _activeIO.fetch_add(1, std::memory_order_release);
    }

    LOG4CXX_INFO(getLogger(), "Removing LDC's that are unrecognized, obsolete, or disabled...");
    CacheRegistry& registry = CacheRegistry::instance();
    STRINGSET dbNames;
    getDBNames(dbNames);
    size_t numRemoved = 0;
    for (auto thisDbName : dbNames)
    {
      std::string thisCacheName(thisDbName);
      uint32_t thisTableVersion(0);
      std::string::size_type pos1 = thisDbName.find_first_of(".");
      std::string::size_type pos2 = thisDbName.find_last_of(".");
      if (pos1 != std::string::npos)
      {
        stringToUint32(thisDbName.substr(pos1 + 1, pos2 - 1), thisTableVersion);
        thisCacheName = thisDbName.substr(0, pos1);
      }
      CacheControl* ctrl = registry.getCacheControl(thisCacheName);
      if ((ctrl == nullptr) || (thisTableVersion == 0) || (ctrl->tableVersion() != thisTableVersion) ||
          (!ctrl->ldcEnabled()))
      {
        LOG4CXX_INFO(getLogger(), "   ==> " << thisDbName);
        std::string file(_directory);
        file += '/';
        file.append(thisDbName);

        PROTECT_DBENV
        if (_dbEnv != nullptr)
        {
          int rc(0);
          if ((rc = _dbEnv->dbremove(_dbEnv, nullptr, file.c_str(), nullptr, 0)) == 0)
          {
            ++numRemoved;
          }
          else
          {
            LOG4CXX_WARN(getLogger(),
                         "Problem removing Berkeley DB file [" << file << "] - RC = " << rc);
          }
        }
        else
        {
          if (removeFile(file))
          {
            ++numRemoved;
          }
        }
      }
    }

    if (numRemoved == 0)
    {
      LOG4CXX_INFO(getLogger(), "No tables were removed.");
    }
    else if (numRemoved == 1)
    {
      LOG4CXX_INFO(getLogger(), "1 table was removed.");
    }
    else
    {
      LOG4CXX_INFO(getLogger(), numRemoved << " tables were removed.");
    }
    _activeIO.fetch_sub(1, std::memory_order_release);
  }
}

void
DiskCache::showEligible() const
{
  for (const auto& elem : _cacheCtrl)
  {
    if (elem.second->enabled)
    {
      LOG4CXX_DEBUG(getLogger(), "Cache type [" << elem.first << "] is enabled.");
    }
  }
}

DateTime
DiskCache::getLastUpdateTime()
{
  boost::lock_guard<boost::mutex> g(_mutex);
  DateTime earliestUpdate(1980, 1, 1);
  std::string updateFile(_directory);
  updateFile.append("/");
  updateFile.append(LAST_UPDATE_FILE);
  int rc(0);
  bool updateFilePresent(false);

  if (access(updateFile.c_str(), F_OK) == 0)
  {
    updateFilePresent = true;
    struct stat st;
    rc = stat(updateFile.c_str(), &st);
    if (rc == 0)
    {
      struct tm* tminfo = localtime(&(st.st_mtime));
      earliestUpdate = DateTime(tminfo->tm_year + 1900,
                                tminfo->tm_mon + 1,
                                tminfo->tm_mday,
                                tminfo->tm_hour,
                                tminfo->tm_min,
                                tminfo->tm_sec);
    }
    else
    {
      std::string errmsg(strerror(rc));
      LOG4CXX_ERROR(getLogger(), "Problem get file stats for [" << updateFile << "] - " << errmsg);
    }
  }
  else
  {
    // No "last update file" yet?   Okay, we will determine
    // last update time by finding the earliest cache file mod time,
    // just to be safe, and we'll create the file so we don't have
    // to go through this whole process again.

    for (CACHE_TYPE_CTRL::const_iterator ctci = _cacheCtrl.begin(); ctci != _cacheCtrl.end();
         ++ctci)
    {
      const std::string& cacheName = (*ctci).first;
      std::string dbFileName(constructDBFileName(cacheName));
      if (!dbFileName.empty())
      {
        std::string dbFile(_directory);
        dbFile.append("/");
        dbFile.append(dbFileName);
        LOG4CXX_DEBUG(getLogger(), "Getting Last Mod Time for [" << dbFile << "]");
        if (access(dbFile.c_str(), F_OK) == 0)
        {
          struct stat st;
          if (stat(dbFile.c_str(), &st) == 0)
          {
            struct tm* tminfo = localtime(&(st.st_mtime));
            DateTime lastModThisType(tminfo->tm_year + 1900,
                                     tminfo->tm_mon + 1,
                                     tminfo->tm_mday,
                                     tminfo->tm_hour,
                                     tminfo->tm_min,
                                     tminfo->tm_sec);
            if (earliestUpdate == DateTime::emptyDate())
            {
              earliestUpdate = lastModThisType;
            }
            else if (lastModThisType < earliestUpdate)
            {
              earliestUpdate = lastModThisType;
            }
          }
        }
      }
    }
  }

  if (!updateFilePresent)
  {
    struct tm tminfo;

    tminfo.tm_sec = earliestUpdate.seconds(); /* seconds */
    tminfo.tm_min = earliestUpdate.minutes(); /* minutes */
    tminfo.tm_hour = earliestUpdate.hours(); /* hours */
    tminfo.tm_mday = earliestUpdate.day(); /* day of the month */
    tminfo.tm_mon = earliestUpdate.month(); /* month */
    tminfo.tm_year = earliestUpdate.year(); /* year */
    tminfo.tm_isdst = -1; /* daylight saving time */

    time_t theTime = mktime(&tminfo);
    if (theTime != -1)
    {
      rc = touch_file(updateFile);
      if (rc == 0)
      {
        struct utimbuf utb;
        utb.actime = theTime;
        utb.modtime = theTime;

        rc = utime(updateFile.c_str(), &utb);
        if (rc != 0)
        {
          std::string errmsg(strerror(rc));
          LOG4CXX_ERROR(getLogger(),
                        "Problem updating mod time for [" << updateFile << "] - " << errmsg);
          removeFile(updateFile);
        }
      }
      else
      {
        std::string errmsg(strerror(rc));
        LOG4CXX_ERROR(getLogger(), "Problem touching file [" << updateFile << "] - " << errmsg);
      }
    }
    else
    {
      LOG4CXX_ERROR(getLogger(),
                    "Problem calling mktime for [" << updateFile << "] date [" << earliestUpdate
                                                   << "]");
    }
  }

  return earliestUpdate;
}

void
DiskCache::updateLastUpdateTime()
{
  boost::lock_guard<boost::mutex> g(_mutex);
  int rc(0);
  std::string updateFile(_directory);
  updateFile.append("/");
  updateFile.append(LAST_UPDATE_FILE);

  if (access(updateFile.c_str(), F_OK) == 0)
  {
    rc = utime(updateFile.c_str(), nullptr);
    if (rc != 0)
    {
      std::string errmsg(strerror(rc));
      LOG4CXX_ERROR(getLogger(),
                    "Problem updating mod time for [" << updateFile << "] - " << errmsg);
    }
  }
  else
  {
    rc = touch_file(updateFile);
    if (rc != 0)
    {
      std::string errmsg(strerror(rc));
      LOG4CXX_ERROR(getLogger(), "Problem touching file [" << updateFile << "] - " << errmsg);
    }
  }
}

std::string
DiskCache::loadSourceToString(LoadSource val)
{
  if (val == LOADSOURCE_DB)
  {
    return "DB";
  }
  else if (val == LOADSOURCE_LDC)
  {
    return "LDC";
  }
  else
  {
    return "OTHER";
  }
}

uint32_t
DiskCache::getDBNames(STRINGSET& list)
{
  list.clear();
  const std::string& parentDir = DISKCACHE.directory();
  if (!parentDir.empty())
  {
    DIR* dir = opendir(parentDir.c_str());
    if (dir != nullptr)
    {
      struct stat st;
      struct dirent* entry = nullptr;
      std::string dbFilePath;

      for (; nullptr != (entry = readdir(dir));)
      {
        std::string fname(entry->d_name);
        size_t len = fname.size();

        // At a minimum, file name must be long enough
        // to support:  "X.n.db" - at least 6 characters

        if (len > 5)
        {
          if ((fname.compare(len - 3, 3, ".db") == 0) && (fname.compare(0, 5, "__db.") != 0) &&
              (fname.compare(0, 3, "log") != 0))
          {
            std::string dbFilePath(parentDir);
            dbFilePath += '/';
            dbFilePath.append(fname);

            if (lstat(dbFilePath.c_str(), &st) == 0)
            {
              if (!S_ISDIR(st.st_mode))
              {
                list.insert(fname);
              }
            }
          }
        }
      }
      closedir(dir);
    }
  }
  return list.size();
}

uint32_t
DiskCache::getKeys(const std::string& name, STRINGSET& list, bool withDates)
{
  DBT dbKey;
  DBT dbData;
  int ret = 0;

  list.clear();

  if (DISKCACHE.bdbDisabled())
  {
    // Act like everything's okay.  We've already printed our
    // errors and warnings...
    return 0;
  }

  if ((_instance != nullptr) && _instance->isActivated() && (!name.empty()))
  {
    DBC* dbCur = nullptr;

    DB* db = getDB(name, true);
    if (db)
    {
      memset(&dbKey, 0x00, sizeof(DBT));
      memset(&dbData, 0x00, sizeof(DBT));

      ret = db->cursor(db, nullptr, &dbCur, 0);
      if (ret == 0)
      {
        while (dbCur->get(dbCur, &dbKey, &dbData, DB_NEXT) == 0)
        {
          std::ostringstream item;
          if (withDates)
          {
            DataBlob::BlobHeader* header = reinterpret_cast<DataBlob::BlobHeader*>(dbData.data);
            tm ltime;
            localtime_r(&(header->timestamp), &ltime);
            DateTime dt(ltime.tm_year + 1900,
                        ltime.tm_mon + 1,
                        ltime.tm_mday,
                        ltime.tm_hour,
                        ltime.tm_min,
                        ltime.tm_sec);
            item << " {" << dt.dateToString(YYYYMMDD, "-") << "." << dt.timeToString(HHMMSS, ":")
                 << "} ";
          }
          item << std::string(reinterpret_cast<char*>(dbKey.data), dbKey.size);
          list.insert(item.str());
        }

        dbCur->close(dbCur);
      }
    }
  }

  return list.size();
}

uint32_t
DiskCache::readKeys(const std::string& name)
{
  uint32_t retval(0);
  DBC* dbCur(nullptr);
  DB* db(getDB(name, true));
  if (db)
  {
    DBT dbKey;
    DBT dbData;
    memset(&dbKey, 0x00, sizeof(DBT));
    memset(&dbData, 0x00, sizeof(DBT));
    if (db->cursor(db, nullptr, &dbCur, 0) == 0)
    {
      while (dbCur->get(dbCur, &dbKey, &dbData, DB_NEXT) == 0)
      {
        ++retval;
      }
      dbCur->close(dbCur);
    }
  }
  return retval;
}

bool
DiskCache::remove(time_t reqTime,
                  bool whileLoading,
                  const std::string& tableName,
                  const std::string& flatKey)
{
  bool retval(_isActivated);

  if (LIKELY(retval))
  {
    if (flatKey == DISKCACHE_ALL)
    {
      retval = DISKCACHE.truncateDB(tableName);
    }
    else
    {
      retval = DISKCACHE.deleteFromDB(tableName, flatKey);
    }
  }

  return retval;
}

void
DiskCache::sendPanicEvent()
{
  bdbEvent(nullptr, DB_EVENT_PANIC, nullptr);
}

void
DiskCache::bdbEvent(DB_ENV* dbenv, u_int32_t event, void* event_info)
{
  static bool inPanicMode(false);

  if (event == DB_EVENT_PANIC)
  {
    bool doPanicProcessing(false);

    {
      TSEGuard lock(_panicMutex);
      if (!inPanicMode)
      {
        inPanicMode = true;
        doPanicProcessing = true;
      }
    }

    if (doPanicProcessing)
    {
      disableBDB();

      std::string msg("BDB PANIC ALERT!  LDC encountered an unrecoverable environment failure "
                      "(DB_RUNRECOVERY)!");
      LOG4CXX_ERROR(getLogger(), msg);
      tse::TseUtil::alert(msg.c_str());

      _instance->_isActivated = false;

      for (auto& elem : _cacheCtrl)
      {
        CacheTypeOptions* cto(elem.second);
        cto->enabled = false;
      }

      if (!getDistCacheEnabled())
      {
        clearAllActionQueues();
      }

      LOG4CXX_ERROR(getLogger(), "LDC has been DISABLED until next restart.");
    }
  }
}

template <class U, typename Validator>
void
DiskCache::getConfigOption(const char* arg_name, U& arg_variable, Validator valueValidator)
{
  std::string tempValue;
  if (_config.getValue(arg_name, tempValue, "DISK_CACHE_OPTIONS"))
  {
    if (!valueValidator(tempValue, arg_variable))
    {
      LOG4CXX_ERROR(getLogger(), "Invalid value [" << tempValue << "] for '" << arg_name << "'");
    }
  }
  else
  {
    LOG4CXX_INFO(getLogger(), "No config entry for '" << arg_name << "'");
  }
  LOG4CXX_DEBUG(getLogger(), "DISK_CACHE_OPTIONS:" << arg_name << " = [" << arg_variable << "]");
}
}
