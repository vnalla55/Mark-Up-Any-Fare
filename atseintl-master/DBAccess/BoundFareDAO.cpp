//----------------------------------------------------------------------------
//  File:        BoundFareDAO.cpp
//  Created:     2009-01-01
//
//  Description: Bound Fare DAO class
//
//  Updates:
//
//  Copyright Sabre 2009
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

#include "DBAccess/BoundFareDAO.h"

#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/TSEException.h"
#include "Common/TseUtil.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/HistoricalDataAccessObject.h"
#include "FileLoader/BFCacheUpdate.h"
#include "FileLoader/BFUtils.h"
#include "FileLoader/FareLoaderFactory.h"
#include "FileLoader/FileLoaderBase.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
BoundFareDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.BoundFareDAO"));
std::string
BoundFareDAO::_name("BoundFare");
std::string
BoundFareDAO::_cacheClass("Fares");
DAOHelper<BoundFareDAO>
BoundFareDAO::_helper(_name);
BoundFareDAO* BoundFareDAO::_instance = nullptr;
std::string
BoundFareDAO::_dataDir("");
int32_t BoundFareDAO::_latestGeneration;

const std::vector<const FareInfo*>&
getBoundFaresByMarketCxrData(const LocCode& market1,
                             const LocCode& market2,
                             const CarrierCode& cxr,
                             const DateTime& startDate,
                             const DateTime& endDate,
                             DeleteList& deleteList,
                             const DateTime& ticketDate,
                             bool isHistorical,
                             bool isFareDisplay)
{
  const LocCode& mkt1 = (market1 < market2) ? market1 : market2;
  const LocCode& mkt2 = (market1 < market2) ? market2 : market1;

  BoundFareDAO& dao = BoundFareDAO::instance();

  return dao.get(deleteList, mkt1, mkt2, cxr, startDate, endDate, ticketDate, isFareDisplay);
}

BoundFareDAO::BoundFareDAO(int cacheSize, const std::string& cacheType)
  : DataAccessObject<FareKey, std::vector<const FareInfo*> >(cacheSize, cacheType)
{
}

BoundFareDAO::~BoundFareDAO() {}

const std::string&
BoundFareDAO::cacheClass()
{
  return _cacheClass;
}

void
BoundFareDAO::load()
{
  const MallocContextDisabler context;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  LOG4CXX_DEBUG(_logger, "BoundFareDAO::load()");

  ConfigMan& config = Global::config();

  char esvLogic = 'N';

  if (!config.getValue("ESV_LOGIC", esvLogic, "SHOPPING_OPT") || (esvLogic != 'Y'))
  {
    return;
  }

  if (_dataDir.empty())
  {
    if (!config.getValue("DATA_DIR", _dataDir, "BOUND_FARE"))
    {
      LOG4CXX_ERROR(
          _logger,
          "BoundFareDAO::load() - Couldn't find config entry for 'DATA_DIR' in 'BOUND_FARE'");
    }
  }

  if (!_dataDir.empty())
  {
    if (_dataDir[_dataDir.length() - 1] != '/')
    {
      _dataDir.append(1, '/');
    }

    std::string latestFile = "";
    int32_t initialGeneration = -1;
    std::vector<int32_t> generationsVec;

    BFUtils::getAllFileGenerations(
        _dataDir, BFFilePatterns::LOAD, "." + BFFilePatterns::GZ, generationsVec);
    if (generationsVec.empty())
    {
      LOG4CXX_ERROR(_logger, "BoundFareDAO::load() - Unable to access Binder fare file");
      TseUtil::alert("Unable to access Binder fare file");
      return;
    }
    else
    {
      // Pick highest found generation
      initialGeneration = generationsVec.at(generationsVec.size() - 1);
    }

    time_t timestamp = BFUtils::getLatestFile(
        _dataDir, BFFilePatterns::LOAD, BFFilePatterns::GZ, latestFile, initialGeneration);

    if (timestamp != 0)
    {
      _latestGeneration = initialGeneration;

      std::unique_ptr<FileLoaderBase> loader(
          FareLoaderFactory::create(false, _dataDir + latestFile, cache().getCacheImpl()));
      try
      {
        if (loader.get() != nullptr)
        {
          loader->parse();

          LOG4CXX_INFO(_logger,
                       "BoundFareDAO::load() - Bound fare file: \""
                           << _dataDir << latestFile << "\", Timestamp: " << timestamp
                           << ", Generation: " << _latestGeneration << " loaded.");

          return;
        }
      }
      catch (const std::exception& e)
      {
        LOG4CXX_ERROR(_logger, e.what());
        TseUtil::alert("Binder fare load failed");
      }
    }
  }

  LOG4CXX_ERROR(_logger, "BoundFareDAO::load() - Unable to access Binder fare file");
  TseUtil::alert("Unable to access Binder fare file");

  throw TSEException(TSEException::UNKNOWN_DATABASE, "Unable to access Binder fare file");
}

bool
BoundFareDAO::translateKey(const ObjectKey& objectKey, FareKey& key) const
{
  return key.initialized = objectKey.getValue("MARKET1", key._a) &&
                           objectKey.getValue("MARKET2", key._b) &&
                           objectKey.getValue("CARRIER", key._c);
}

FareKey
BoundFareDAO::createKey(const FareInfo* info)
{
  return FareKey(info->market1(), info->market2(), info->carrier());
}

const std::vector<const FareInfo*>&
BoundFareDAO::get(DeleteList& del,
                  const LocCode& market1,
                  const LocCode& market2,
                  const CarrierCode& cxr,
                  const DateTime& startDate,
                  const DateTime& endDate,
                  const DateTime& ticketDate,
                  bool isFareDisplay)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareKey key(market1, market2, cxr);

  DAOCache::pointer_type ptr = cache().getIfResident(key);

  return *(applyFilter(del, ptr, IsNotEffectiveG<FareInfo>(startDate, endDate, ticketDate)));
}

BoundFareDAO&
BoundFareDAO::instance()
{
  LOG4CXX_DEBUG(_logger, "BoundFareDAO::instance()");

  if (nullptr == _instance)
  {
    _helper.init();
  }

  return *_instance;
}

void
BoundFareDAO::checkForUpdates()
{
  BFCacheUpdate::checkForUpdates(instance().cache().getCacheImpl(), _dataDir, _latestGeneration);
}

std::vector<const FareInfo*>*
BoundFareDAO::create(FareKey key)
{
  LOG4CXX_DEBUG(_logger, "BoundFareDAO::create()");
  return nullptr;
}

void
BoundFareDAO::destroy(FareKey key, std::vector<const FareInfo*>* recs)
{
  LOG4CXX_DEBUG(_logger, "BoundFareDAO::destroy()");

  if (nullptr == recs)
  {
    return;
  }

  std::vector<const FareInfo*>::iterator i;

  for (i = recs->begin(); i != recs->end(); ++i)
  {
    delete *i;
  }

  delete recs;
}
}
