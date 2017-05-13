//----------------------------------------------------------------------------
//  File:        MarketRoutingDAO.cpp
//  Created:     2009-01-01
//
//  Description: Market routing DAO class
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

#include "DBAccess/MarketRoutingDAO.h"

#include "Common/TSEException.h"
#include "Common/TseUtil.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/MarketRoutingInfo.h"
#include "FileLoader/BFUtils.h"
#include "FileLoader/MarketRoutingCacheUpdater.h"
#include "FileLoader/MarketRoutingLoader.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
MarketRoutingDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.MarketRoutingDAO"));
std::string
MarketRoutingDAO::_name("MarketRouting");
std::string
MarketRoutingDAO::_cacheClass("MarketRoutings");
DAOHelper<MarketRoutingDAO>
MarketRoutingDAO::_helper(_name);
MarketRoutingDAO* MarketRoutingDAO::_instance = nullptr;
std::string
MarketRoutingDAO::_dataDir("");
int32_t
MarketRoutingDAO::_latestGeneration(-1);

const MarketRoutingInfo&
getMarketRoutingInfoData(const VendorCode& vendor,
                         const CarrierCode& carrier,
                         const RoutingNumber& routing,
                         TariffNumber routingTariff,
                         const LocCode& market1,
                         const LocCode& market2,
                         const bool getSingles,
                         const bool getDoubles,
                         DeleteList& deleteList)
{
  MarketRoutingInfo* routingInfo = new MarketRoutingInfo;
  deleteList.adopt(routingInfo);

  MarketRoutingDAO& dao = MarketRoutingDAO::instance();
  const MarketRouting* routingData = dao.get(deleteList, vendor, carrier, routing, routingTariff);

  if (routingData != nullptr)
  {
    MarketRoutingIndexInfo index = routingData->getIndexInfo(market1, market2);

    if (true == getSingles)
    {
      routingInfo->singles() = routingData->getSingles(index.singlesIndex());
    }

    if (true == getDoubles)
    {
      routingInfo->doubles() = routingData->getDoubles(index.doublesIndex());
    }
  }

  return *routingInfo;
}

MarketRoutingDAO::MarketRoutingDAO(int cacheSize, const std::string& cacheType)
  : DataAccessObject<MarketRoutingKey, MarketRouting>(cacheSize, cacheType)
{
}

MarketRoutingDAO::~MarketRoutingDAO() {}

const std::string&
MarketRoutingDAO::cacheClass()
{
  return _cacheClass;
}

void
MarketRoutingDAO::load()
{
  const MallocContextDisabler context;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  LOG4CXX_DEBUG(_logger, "MarketRoutingDAO::load()");

  tse::ConfigMan& config = Global::config();

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
          "MarketRoutingDAO::load() - Couldn't find config entry for 'DATA_DIR' in 'BOUND_FARE'");
    }
  }

  if (!_dataDir.empty())
  {
    if (_dataDir[_dataDir.length() - 1] != '/')
    {
      _dataDir.append(1, '/');
    }

    std::string marketsFile = "";
    std::string singlesFile = "";
    std::string doublesFile = "";

    int32_t initialGeneration = -1;
    std::vector<int32_t> marketsGenerationsVec;
    std::vector<int32_t> singlesGenerationsVec;
    std::vector<int32_t> doublesGenerationsVec;

    // Markets
    BFUtils::getAllFileGenerations(
        _dataDir, MRFilePatterns::MARKETS, "." + MRFilePatterns::GZ, marketsGenerationsVec);

    if (marketsGenerationsVec.empty())
    {
      LOG4CXX_ERROR(_logger, "MarketRoutingDAO::load() - Unable to access Binder routing file(s)");
      TseUtil::alert("Unable to access Binder routing file(s)");
      return;
    }

    // Singles
    BFUtils::getAllFileGenerations(
        _dataDir, MRFilePatterns::SINGLES, "." + MRFilePatterns::GZ, singlesGenerationsVec);

    if (singlesGenerationsVec.empty())
    {
      LOG4CXX_ERROR(_logger, "MarketRoutingDAO::load() - Unable to access Binder routing file(s)");
      TseUtil::alert("Unable to access Binder routing file(s)");
      return;
    }

    // Doubles
    BFUtils::getAllFileGenerations(
        _dataDir, MRFilePatterns::DOUBLES, "." + MRFilePatterns::GZ, doublesGenerationsVec);

    if (doublesGenerationsVec.empty())
    {
      LOG4CXX_ERROR(_logger, "MarketRoutingDAO::load() - Unable to access Binder routing file(s)");
      TseUtil::alert("Unable to access Binder routing file(s)");
      return;
    }

    // Try to findh highest generation of file which exist for markets,
    // routings and singles files
    for (int32_t i = marketsGenerationsVec.size() - 1; i >= 0; --i)
    {
      int32_t highestMarketsGen = marketsGenerationsVec.at(i);

      if ((singlesGenerationsVec.end() != std::find(singlesGenerationsVec.begin(),
                                                    singlesGenerationsVec.end(),
                                                    highestMarketsGen)) &&
          (doublesGenerationsVec.end() != std::find(doublesGenerationsVec.begin(),
                                                    doublesGenerationsVec.end(),
                                                    highestMarketsGen)))
      {
        initialGeneration = highestMarketsGen;
        break;
      }
    }

    if (-1 == initialGeneration)
    {
      LOG4CXX_ERROR(_logger, "MarketRoutingDAO::load() - Could not find markets, singles and "
                             "doubles files with the same generation number.");
      TseUtil::alert("Could not find markets, singles and doubles files with the same generation.");
      return;
    }

    time_t marketsTimestamp = BFUtils::getLatestFile(
        _dataDir, MRFilePatterns::MARKETS, MRFilePatterns::GZ, marketsFile, initialGeneration);

    time_t singlesTimestamp = BFUtils::getLatestFile(
        _dataDir, MRFilePatterns::SINGLES, MRFilePatterns::GZ, singlesFile, initialGeneration);

    time_t doublesTimestamp = BFUtils::getLatestFile(
        _dataDir, MRFilePatterns::DOUBLES, MRFilePatterns::GZ, doublesFile, initialGeneration);

    if ((marketsTimestamp != 0) && (singlesTimestamp != 0) && (doublesTimestamp != 0))
    {

      _latestGeneration = initialGeneration;

      MarketRoutingLoader loader(_dataDir,
                                 marketsFile,
                                 singlesFile,
                                 doublesFile,
                                 _latestGeneration,
                                 cache().getCacheImpl());

      try
      {
        loader.parse();

        LOG4CXX_INFO(_logger,
                     "MarketRoutingDAO::load() - Routing markets file: \""
                         << _dataDir << marketsFile << "\", Timestamp: " << marketsTimestamp
                         << ", Generation: " << initialGeneration << " loaded.");
        LOG4CXX_INFO(_logger,
                     "MarketRoutingDAO::load() - Routing singles file: \""
                         << _dataDir << singlesFile << "\", Timestamp: " << singlesTimestamp
                         << ", Generation: " << initialGeneration << " loaded.");
        LOG4CXX_INFO(_logger,
                     "MarketRoutingDAO::load() - Routing doubles file: \""
                         << _dataDir << doublesFile << "\", Timestamp: " << doublesTimestamp
                         << ", Generation: " << initialGeneration << " loaded.");

        return;
      }
      catch (const std::exception& e)
      {
        LOG4CXX_ERROR(_logger, e.what());
        TseUtil::alert("Binder routing load failed");
      }
    }
  }

  LOG4CXX_ERROR(_logger, "MarketRoutingDAO::load() - Unable to access Binder routing file(s)");
  TseUtil::alert("Unable to access Binder routing file(s)");

  throw TSEException(TSEException::UNKNOWN_DATABASE, "Unable to access Binder routing file(s)");
}

bool
MarketRoutingDAO::translateKey(const ObjectKey& objectKey, MarketRoutingKey& key) const
{
  return key.initialized =
             objectKey.getValue("VENDOR", key._a) && objectKey.getValue("TARIFF", key._b) &&
             objectKey.getValue("CARRIER", key._c) && objectKey.getValue("ROUTING", key._d);
}

const MarketRouting*
MarketRoutingDAO::get(DeleteList& del,
                      const VendorCode& vendor,
                      const CarrierCode& carrier,
                      const RoutingNumber& routing,
                      TariffNumber routingTariff)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  LOG4CXX_DEBUG(_logger, "MarketRoutingDAO::get()");

  MarketRoutingKey key(vendor, routingTariff, carrier, routing);

  MarketRouting* ret = nullptr;

  DAOCache::pointer_type ptr = cache().getIfResident(key);

  if (ptr)
  {
    del.copy(ptr);
    ret = ptr.get();
  }

  return ret;
}

MarketRoutingDAO&
MarketRoutingDAO::instance()
{
  LOG4CXX_DEBUG(_logger, "MarketRoutingDAO::instance()");

  if (nullptr == _instance)
  {
    _helper.init();
  }

  return *_instance;
}

void
MarketRoutingDAO::checkForUpdates()
{
  MarketRoutingCacheUpdater::checkForUpdates(
      instance().cache().getCacheImpl(), _dataDir, _latestGeneration);
}

MarketRouting*
MarketRoutingDAO::create(MarketRoutingKey key)
{
  LOG4CXX_DEBUG(_logger, "MarketRoutingDAO::create()");

  return nullptr;
}

void
MarketRoutingDAO::destroy(MarketRoutingKey key, MarketRouting* rec)
{
  LOG4CXX_DEBUG(_logger, "MarketRoutingDAO::destroy()");

  if (nullptr == rec)
  {
    return;
  }

  delete rec;
}
}
