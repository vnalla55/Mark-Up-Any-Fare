//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#include "DBAccess/MultiTransportLocsDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/MultiTransport.h"
#include "DBAccess/Queries/QueryGetMultiTransport.h"

namespace tse
{
log4cxx::LoggerPtr
MultiTransportLocsDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.MultiTransportLocsDAO"));

MultiTransportLocsDAO&
MultiTransportLocsDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<MultiTransport*>&
getMultiTransportLocsData(const LocCode& city,
                          const CarrierCode& carrierCode,
                          GeoTravelType tvlType,
                          const DateTime& tvlDate,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical)
{
  if (isHistorical)
  {
    MultiTransportLocsHistoricalDAO& dao = MultiTransportLocsHistoricalDAO::instance();
    return dao.get(deleteList, city, carrierCode, tvlType, tvlDate, ticketDate);
  }
  else
  {
    MultiTransportLocsDAO& dao = MultiTransportLocsDAO::instance();
    return dao.get(deleteList, city, carrierCode, tvlType, tvlDate, ticketDate);
  }
}

const LocCode
MultiTransportLocsDAO::get(DeleteList& del, const LocCode& locCode, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(LocCodeKey(locCode));
  std::vector<MultiTransport*>::iterator i = ptr->begin();
  for (; i != ptr->end(); ++i)
    if ((*i)->intlAppl() == 'Y')
      return (*i)->multitranscity();
  return "";
}

struct MultiTransportLocsDAO::matchKeys : public std::unary_function<MultiTransport*, bool>
{
public:
  const CarrierCode _carrier;
  const GeoTravelType _tvlType;
  const DateTime& _tvlDate;

  matchKeys(const CarrierCode carrier, GeoTravelType tvlType, const DateTime& tvlDate)
    : _carrier(carrier), _tvlType(tvlType), _tvlDate(tvlDate)
  {
  }

  bool operator()(MultiTransport* rec) const
  {
    return (rec->carrier() == _carrier &&
            (((_tvlType == GeoTravelType::Domestic || _tvlType == GeoTravelType::Transborder) && rec->domAppl() == 'Y') ||
             ((_tvlType != GeoTravelType::Domestic && _tvlType != GeoTravelType::Transborder) && rec->intlAppl() == 'Y')) &&
            (_tvlDate > rec->effDate() && _tvlDate < rec->expireDate()));
  }
}; // lint !e1510

const std::vector<MultiTransport*>&
MultiTransportLocsDAO::get(DeleteList& del,
                           const LocCode& locCode,
                           const CarrierCode& carrier,
                           GeoTravelType tvlType,
                           const DateTime& tvlDate,
                           const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  std::vector<MultiTransport*>* ret = new std::vector<MultiTransport*>;
  del.adopt(ret);
  DAOCache::pointer_type ptr = cache().get(LocCodeKey(locCode));
  del.copy(ptr);
  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), not1(matchKeys(carrier, tvlType, tvlDate)));
  if (ret->empty())
  {
    remove_copy_if(
        ptr->begin(), ptr->end(), back_inserter(*ret), not1(matchKeys("", tvlType, tvlDate)));
  }
  return *ret;
}

LocCodeKey
MultiTransportLocsDAO::createKey(MultiTransport* info)
{
  return LocCodeKey(info->multitranscity());
}

std::vector<MultiTransport*>*
MultiTransportLocsDAO::create(LocCodeKey key)
{
  std::vector<MultiTransport*>* ret = new std::vector<MultiTransport*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMultiTransportLocs mtl(dbAdapter->getAdapter());
    mtl.findMultiTransportLocs(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MultiTransportLocsDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MultiTransportLocsDAO::destroy(LocCodeKey key, std::vector<MultiTransport*>* recs)
{
  destroyContainer(recs);
}

void
MultiTransportLocsDAO::load()
{
  StartupLoader<QueryGetAllMultiTransportLocs, MultiTransport, MultiTransportLocsDAO>();
}

sfc::CompressedData*
MultiTransportLocsDAO::compress(const std::vector<MultiTransport*>* vect) const
{
  return compressVector(vect);
}

std::vector<MultiTransport*>*
MultiTransportLocsDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MultiTransport>(compressed);
}

std::string
MultiTransportLocsDAO::_name("MultiTransportLocs");
std::string
MultiTransportLocsDAO::_cacheClass("Common");

DAOHelper<MultiTransportLocsDAO>
MultiTransportLocsDAO::_helper(_name);

MultiTransportLocsDAO* MultiTransportLocsDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: MultiTransportLocsHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
MultiTransportLocsHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.MultiTransportLocsHistoricalDAO"));
MultiTransportLocsHistoricalDAO&
MultiTransportLocsHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const LocCode
MultiTransportLocsHistoricalDAO::get(DeleteList& del,
                                     const LocCode& locCode,
                                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  std::vector<MultiTransport*>* ret = new std::vector<MultiTransport*>;
  del.adopt(ret);
  MultiTransportLocsHistoricalKey cacheKey(locCode);
  DAOUtils::getDateRange(ticketDate, cacheKey._b, cacheKey._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(cacheKey);
  del.copy(ptr);
  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotCurrentH<MultiTransport>(ticketDate));
  std::vector<MultiTransport*>::iterator i = ret->begin();
  for (; i != ret->end(); ++i)
    if ((*i)->intlAppl() == 'Y')
      return (*i)->multitranscity();
  return "";
}

struct MultiTransportLocsHistoricalDAO::matchKeysH
    : public std::unary_function<MultiTransport*, bool>
{
public:
  const CarrierCode _carrier;
  const GeoTravelType _tvlType;
  const DateTime& _tvlDate;
  const DateTime& _tktDate;

  IsEffectiveH<MultiTransport> _isEffectiveH;

  matchKeysH(const CarrierCode carrier,
             GeoTravelType tvlType,
             const DateTime& tvlDate,
             const DateTime& ticketDate)
    : _carrier(carrier),
      _tvlType(tvlType),
      _tvlDate(tvlDate),
      _tktDate(ticketDate),
      _isEffectiveH(_tvlDate, _tktDate)
  {
  }

  bool operator()(MultiTransport* rec) const
  {
    return (rec->carrier() == _carrier &&
            (((_tvlType == GeoTravelType::Domestic || _tvlType == GeoTravelType::Transborder) && rec->domAppl() == 'Y') ||
             ((_tvlType != GeoTravelType::Domestic && _tvlType != GeoTravelType::Transborder) && rec->intlAppl() == 'Y')) &&
            (_isEffectiveH(rec)));
  }
}; // lint !e1510

const std::vector<MultiTransport*>&
MultiTransportLocsHistoricalDAO::get(DeleteList& del,
                                     const LocCode& locCode,
                                     const CarrierCode& carrier,
                                     GeoTravelType tvlType,
                                     const DateTime& tvlDate,
                                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  MultiTransportLocsHistoricalKey cacheKey(locCode);
  DAOUtils::getDateRange(ticketDate, cacheKey._b, cacheKey._c, _cacheBy);
  std::vector<MultiTransport*>* ret = new std::vector<MultiTransport*>;
  del.adopt(ret);
  DAOCache::pointer_type ptr = cache().get(cacheKey);
  del.copy(ptr);

  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 not1(matchKeysH(carrier, tvlType, tvlDate, ticketDate)));
  if (ret->empty())
  {
    remove_copy_if(ptr->begin(),
                   ptr->end(),
                   back_inserter(*ret),
                   not1(matchKeysH("", tvlType, tvlDate, ticketDate)));
  }
  return *ret;
}

std::vector<MultiTransport*>*
MultiTransportLocsHistoricalDAO::create(MultiTransportLocsHistoricalKey key)
{
  std::vector<MultiTransport*>* ret = new std::vector<MultiTransport*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMultiTransportLocsHistorical mtl(dbAdapter->getAdapter());
    mtl.findMultiTransportLocsHistorical(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MultiTransportLocsHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MultiTransportLocsHistoricalDAO::destroy(MultiTransportLocsHistoricalKey key,
                                         std::vector<MultiTransport*>* recs)
{
  destroyContainer(recs);
}

sfc::CompressedData*
MultiTransportLocsHistoricalDAO::compress(const std::vector<MultiTransport*>* vect) const
{
  return compressVector(vect);
}

std::vector<MultiTransport*>*
MultiTransportLocsHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MultiTransport>(compressed);
}

std::string
MultiTransportLocsHistoricalDAO::_name("MultiTransportLocsHistorical");
std::string
MultiTransportLocsHistoricalDAO::_cacheClass("Common");
DAOHelper<MultiTransportLocsHistoricalDAO>
MultiTransportLocsHistoricalDAO::_helper(_name);

MultiTransportLocsHistoricalDAO* MultiTransportLocsHistoricalDAO::_instance = nullptr;

} // namespace tse
