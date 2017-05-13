//----------------------------------------------------------------------------
//
//     File:           ORACLEDBResultSet.cpp
//     Description:
//     Created:
//     Authors:
//
//     Updates:
//
//     Copyright @2009, Sabre Inc.  All rights reserved.
//         This software/documentation is the confidential and proprietary
//         product of Sabre Inc.
//         Any unauthorized use, reproduction, or transfer of this
//         software/documentation, in any medium, or incorporation of this
//         software/documentation into any system or publication,
//         is strictly prohibited.
//----------------------------------------------------------------------------

#include "DBAccess/ORACLEDBResultSet.h"

#include "Allocator/TrxMalloc.h"
#include "Common/Logger.h"
#include "Common/TSEException.h"
#include "Common/TseSrvStats.h"
#include "Common/TseUtil.h"
#include "DBAccess/CacheManager.h"
#include "DBAccess/ORACLEAdapter.h"
#include "DBAccess/ORACLEConnectionTimer.h"
#include "DBAccess/SQLQuery.h"

#include <string>

namespace tse
{

namespace
{

log4cxx::LoggerPtr& getLogger()
{
  static log4cxx::LoggerPtr logger(
      log4cxx::Logger::getLogger("atseintl.DBAccess.ORACLEDBResultSet"));
  return logger;
}

bool
isDebugEnabled()
{
  static const bool enabled(IS_DEBUG_ENABLED(getLogger()));
  return enabled;
}

}// namespace

#define ORACLE_TIMER_START                                                                         \
  do                                                                                               \
  {                                                                                                \
    if (oracleConnectionTimer())                                                                   \
    {                                                                                              \
      LOG4CXX_DEBUG(getLogger(),                                                                   \
                    __PRETTY_FUNCTION__ << ":" << __LINE__ << ": at[" << oracleConnectionTimer()   \
                                        << "] starting");                                          \
      oracleConnectionTimer()->stop();                                                             \
      oracleConnectionTimer()->start();                                                            \
    }                                                                                              \
  } while (0)

#define ORACLE_TIMER_STOP                                                                          \
  do                                                                                               \
  {                                                                                                \
    if (oracleConnectionTimer())                                                                   \
    {                                                                                              \
      LOG4CXX_DEBUG(getLogger(),                                                                   \
                    __PRETTY_FUNCTION__ << ":" << __LINE__ << ": at[" << oracleConnectionTimer()   \
                                        << "] stopping");                                          \
      oracleConnectionTimer()->stop();                                                             \
    }                                                                                              \
  } while (0)

ORACLEDBResultSet::ORACLEDBResultSet(DBAdapter* dbAdapt) : ResultSet(dbAdapt)
{
  _dbAdapt = dynamic_cast<ORACLEAdapter*>(dbAdapt);
  _currentRow = new ORACLEDBRow();
  _oracleConnectionTimer = _dbAdapt->oracleConnectionTimer();
  if (isDebugEnabled())
  {
    LOG4CXX_DEBUG(getLogger(),
                  __PRETTY_FUNCTION__ << ":" << __LINE__ << ": initialized to["
                                      << oracleConnectionTimer() << "]");
  }
}

ORACLEDBResultSet::~ORACLEDBResultSet()
{
  freeResult();
  releaseDefineBuffers();
  if (LIKELY(_currentRow != nullptr))
    delete _currentRow;
}

bool
ORACLEDBResultSet::executeQuery(SQLQuery* queryObject)
{
  try
  {
    freeResult();
    _queryID = -1;
    _sqlStatement = queryObject;
    _sqlQueryObject = queryObject;
    _currentRowIndex = 0;
    _execAndFetch = false;
    _numFetched = 0;
    static const int maxTryCount(_dbAdapt->getConfigMaxNumberTries());

    _currentRow->init(_dbAdapt, this, queryObject);

    ORACLE_TIMER_START;

    for (int tryCount = 0; tryCount < maxTryCount; ++tryCount)
    {
      _ociStmt = _dbAdapt->prepareQuery(queryObject);

      // Check for define buffers
      if (reuseDefineBuffers())
      {
        // Do the OCI Defines
        if (UNLIKELY(!setOracleDefines()))
        {
          LOG4CXX_ERROR(getLogger(), "Error setting Oracle defines. Will retry.");

          freeResult();
          continue; // Allow retry
        }

        // Do bundled execute and fetch
        int32_t status = _dbAdapt->executeQuery(queryObject, _arrayFetchSize);

        if (UNLIKELY(status != OCI_SUCCESS && status != OCI_NO_DATA))
        {
          freeResult();
          continue; // Allow retry
        }

        uint32_t rowsFetched = 0;
        status = OCIAttrGet((void*)_dbAdapt->getOCIStatement(),
                            OCI_HTYPE_STMT,
                            (void*)&rowsFetched,
                            nullptr,
                            OCI_ATTR_ROWS_FETCHED,
                            _dbAdapt->getOCIError());

        if (UNLIKELY(status != OCI_SUCCESS))
        {
          std::string ociErrMsg;
          LOG4CXX_ERROR(getLogger(),
                        "Oracle error getting fetch count. Will retry. Error: "
                            << getOCIErrorString(status, _dbAdapt->getOCIError(), ociErrMsg));

          freeResult();
          continue; // Allow retry
        }
        if (isDebugEnabled())
        {
          LOG4CXX_DEBUG(getLogger(),
                        "Bundled execute and fetch successful for " << rowsFetched << " rows");
        }
        _lastFetchSize = rowsFetched;
        _execAndFetch = true;
        if (rowsFetched > 0)
        {
          _numFetched += rowsFetched;
          _currentRowIndex = 1;
        }
      }
      else
      {
        // Do non-fetch execute
        //
        int32_t status = _dbAdapt->executeQuery(queryObject, 0);

        if (status != OCI_SUCCESS && status != OCI_NO_DATA)
        {
          freeResult();
          continue; // Allow retry
        }

        if (!createDefineBuffers())
        {
          LOG4CXX_ERROR(getLogger(), "Error initializing define buffers. Will retry.");

          freeResult();
          continue; // Allow retry
        }

        if (!setOracleDefines())
        {
          LOG4CXX_ERROR(getLogger(), "Error setting defines. Will retry.");

          freeResult();
          continue; // Allow retry
        }
      }
      _currentRow->setFieldCount(getFieldCount());

      return _ociStmt != nullptr;
    }
  }
  catch (...)
  {
    freeResult();
    ORACLE_TIMER_STOP;
    TseSrvStats::recordDBCall(0, false);
    throw;
  }

  // If we reach this point, then we've retried up to the maximum number
  // of retries with no success. There's nothing left to do except log
  // an error and throw an exception.
  //
  TseSrvStats::recordDBCall(0, false);
  std::string queryName;
  std::string sqlString;
  std::string parameterString;
  if (queryObject != nullptr)
  {
    queryName = queryObject->getQueryName();
    queryObject->getSQLString(sqlString);
    queryObject->fillParameterString(parameterString);
  }
  LOG4CXX_ERROR(getLogger(), "Query failed. "
                << " QUERY_NAME: " << queryName
                << "\nSQLString: " << sqlString
                << "\nparameters:" << parameterString);
  ORACLE_TIMER_STOP;
  throw TSEException(TSEException::NO_ERROR, "Database error");
}

void
ORACLEDBResultSet::freeResult()
{
  _numFetched = 0;
  releaseDefineBuffers();
  if (_ociStmt)
  {
    _dbAdapt->closeResultSet();
    _ociStmt = nullptr;
  }
}

// This is called only for debug purposes from the Query directory
// We will use the accumulated number of rows fetched!
int
ORACLEDBResultSet::numRows() const
{
  return _numFetched;
}

Row*
ORACLEDBResultSet::nextRow()
{

  if (UNLIKELY(oracleConnectionTimer() && oracleConnectionTimer()->ocibreak))
  {
    static DateTime lastReportedAlert(
        DateTime(time(nullptr)).subtractSeconds(_dbAdapt->alertSupressionInterval()));
    TseSrvStats::recordDBCall(0, false);
    LOG4CXX_ERROR(getLogger(),
                  std::string("ORACLE DATABASE QUERY TIMEOUT: ") + _dbAdapt->getDatabase());

    DateTime now(time(nullptr));
    if (UNLIKELY(now.subtractSeconds(_dbAdapt->alertSupressionInterval()) > lastReportedAlert))
    {
      lastReportedAlert = now;
      TseUtil::alert(
          (std::string("ORACLE DATABASE QUERY TIMEOUT: ") + _dbAdapt->getDatabase()).c_str());
    }
    ORACLE_TIMER_STOP;
    freeResult();
    throw ErrorResponseException(ErrorResponseException::DB_TIMEOUT_ERROR);
  }
  if (_execAndFetch)
  {
    if (_currentRowIndex == 1)
    {
      _execAndFetch = false;
      ORACLE_TIMER_STOP;
      return _currentRow;
    }
    else if (LIKELY(_currentRowIndex == 0))
    {
      if (isDebugEnabled())
      {
        LOG4CXX_DEBUG(getLogger(), "No rows to process");
      }
      ORACLE_TIMER_STOP;
      return nullptr;
    }
  }
  else if (_currentRowIndex == 0 ||
           (_currentRowIndex >= getArrayFetchSize() && _lastFetchSize == getArrayFetchSize()))
  {
    clearDefineBuffers();

    if (isDebugEnabled())
    {
      LOG4CXX_DEBUG(getLogger(), "Fetching " << getArrayFetchSize() << " rows");
    }
    int32_t status;
    std::string ociErrMsg;
    status = OCIStmtFetch2(
        _ociStmt, _dbAdapt->getOCIError(), getArrayFetchSize(), OCI_FETCH_NEXT, 0, OCI_DEFAULT);

    // Partial fetch or end-of-fetch reached
    if (status == OCI_SUCCESS)
    {
      if (isDebugEnabled())
      {
        LOG4CXX_DEBUG(getLogger(),
                      "Full array fetch successful for " << getArrayFetchSize() << " rows");
      }
      _lastFetchSize = getArrayFetchSize();
      _currentRowIndex = 1;
      _numFetched += _lastFetchSize;
      return _currentRow;
    }
    else if (LIKELY(status == OCI_NO_DATA))
    {
      uint32_t rowsFetched = 0;
      int32_t stat = OCIAttrGet((void*)_dbAdapt->getOCIStatement(),
                                OCI_HTYPE_STMT,
                                (void*)&rowsFetched,
                                nullptr,
                                OCI_ATTR_ROWS_FETCHED,
                                _dbAdapt->getOCIError());

      if (UNLIKELY(stat != OCI_SUCCESS))
      {
        TseSrvStats::recordDBCall(0, false);
        LOG4CXX_ERROR(getLogger(),
                      "Oracle error getting fetch count: "
                          << getOCIErrorString(stat, _dbAdapt->getOCIError(), ociErrMsg));

        ORACLE_TIMER_STOP;
        throw TSEException(TSEException::NO_ERROR, "DATABASE ERROR");
      }

      if (isDebugEnabled())
      {
        LOG4CXX_DEBUG(getLogger(), "Partial array fetch successful for " << rowsFetched << " rows");
      }
      _lastFetchSize = rowsFetched;
      _currentRowIndex = 1;
      if (rowsFetched > 0)
      {
        _numFetched += rowsFetched;
        return _currentRow;
      }
      ORACLE_TIMER_STOP;
      return nullptr;
    }
    else
    {
      TseSrvStats::recordDBCall(0, false);
      LOG4CXX_ERROR(getLogger(),
                    "Oracle error during fetch: "
                        << getOCIErrorString(status, _dbAdapt->getOCIError(), ociErrMsg));

      ORACLE_TIMER_STOP;
      throw TSEException(TSEException::NO_ERROR, "DATABASE ERROR");
    }
  }
  else if (_lastFetchSize < getArrayFetchSize() && _currentRowIndex >= _lastFetchSize)
  {
    // No more rows
    if (isDebugEnabled())
    {
      LOG4CXX_DEBUG(getLogger(), "No more rows to process");
    }
    ORACLE_TIMER_STOP;
    return nullptr;
  }
  else
  {
    ++_currentRowIndex;
    return _currentRow;
  }
  ORACLE_TIMER_STOP;
  return nullptr;
}

int
ORACLEDBResultSet::getInt(int columnIndex)
{
  return _defineBuffers->getInt(columnIndex, _currentRowIndex);
}

long int
ORACLEDBResultSet::getLong(int columnIndex)
{
  return _defineBuffers->getLong(columnIndex, _currentRowIndex);
}

long long int
ORACLEDBResultSet::getLongLong(int columnIndex)
{
  return _defineBuffers->getLongLong(columnIndex, _currentRowIndex);
}

const char*
ORACLEDBResultSet::getString(int columnIndex)
{
  return _defineBuffers->getString(columnIndex, _currentRowIndex);
}

char
ORACLEDBResultSet::getChar(int columnIndex)
{
  return _defineBuffers->getChar(columnIndex, _currentRowIndex);
}

DateTime
ORACLEDBResultSet::getDateTime(int columnIndex)
{
  return _defineBuffers->getDateTime(columnIndex, _currentRowIndex, *_dbAdapt);
}

bool
ORACLEDBResultSet::isNull(int columnIndex)
{
  return _defineBuffers->isNull(columnIndex, _currentRowIndex);
}

bool
ORACLEDBResultSet::reuseDefineBuffers()
{
  _defineBuffers =
      ORACLEDefineBufferCollection::findDefineBufferCollection(_sqlQueryObject->getQueryName());
  if (_defineBuffers)
  {
    _arrayFetchSize = _defineBuffers->getArrayFetchSize();
    return true;
  }
  return false;
}

bool
ORACLEDBResultSet::createDefineBuffers()
{
  int32_t status;
  std::string ociErrMsg;

  _defineBuffers = ORACLEDefineBufferCollection::createDefineBufferCollection(
      _sqlQueryObject->getQueryName(),
      *_dbAdapt,
      _sqlQueryObject->getArrayFetchSize(),
      _sqlQueryObject->getMaxDefineBufferStackSize(),
      status);

  if (!_defineBuffers)
  {
    LOG4CXX_ERROR(getLogger(),
                  "Error creating OCI define buffers: "
                      << getOCIErrorString(status, _dbAdapt->getOCIError(), ociErrMsg));
    return false;
  }
  _arrayFetchSize = _defineBuffers->getArrayFetchSize();
  return true;
}

bool
ORACLEDBResultSet::releaseDefineBuffers()
{
  if (_defineBuffers)
  {
    ORACLEDefineBufferCollection::releaseDefineBufferCollection(_defineBuffers, *_dbAdapt);
  }
  return true;
}

void
ORACLEDBResultSet::clearDefineBuffers()
{
  int32_t status;
  std::string ociErrMsg;

  _defineBuffers->clearDefineBuffers(*_dbAdapt, status);
}

bool
ORACLEDBResultSet::setOracleDefines()
{
  int32_t status;
  std::string ociErrMsg;

  if (UNLIKELY(!_defineBuffers->setOracleDefines(*_dbAdapt, status)))
  {
    LOG4CXX_ERROR(getLogger(),
                  "Error setting OCI define buffers: "
                      << getOCIErrorString(status, _dbAdapt->getOCIError(), ociErrMsg));
    return false;
  }
  return true;
}

int32_t
ORACLEDBResultSet::getFieldCount()
{
  return _defineBuffers->getBufferCount();
}

ORACLEConnectionTimer*
ORACLEDBResultSet::oracleConnectionTimer()
{
  return _oracleConnectionTimer;
}
}
