//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/SubCodeDAO.h"

#include "DBAccess/Queries/QueryGetSubCode.h"
#include "DBAccess/SubCodeInfo.h"

namespace
{

template <typename T>
struct IsSame
{
  IsSame<T>(T& data) : _data(data) {}
  template <typename S>
  bool operator()(S* obj, T& (S::*mhod)() const) const
  {
    return (obj->*mhod)() == _data;
  }

private:
  T& _data;
};
}

namespace tse
{
log4cxx::LoggerPtr
SubCodeDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SubCodeDAO"));

std::string
SubCodeDAO::_name("SubCode");
std::string
SubCodeDAO::_cacheClass("Rules");
DAOHelper<SubCodeDAO>
SubCodeDAO::_helper(_name);
SubCodeDAO* SubCodeDAO::_instance = nullptr;

SubCodeDAO&
SubCodeDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<SubCodeInfo*>&
getSubCodeData(const VendorCode& vendor,
               const CarrierCode& carrier,
               const ServiceTypeCode& serviceTypeCode,
               const ServiceGroup& serviceGroup,
               const DateTime& date,
               DeleteList& deleteList,
               const DateTime& ticketDate,
               bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    SubCodeHistoricalDAO& dao = SubCodeHistoricalDAO::instance();
    return dao.get(deleteList, vendor, carrier, serviceTypeCode, serviceGroup, date, ticketDate);
  }
  else
  {
    SubCodeDAO& dao = SubCodeDAO::instance();
    return dao.get(deleteList, vendor, carrier, serviceTypeCode, serviceGroup, date, ticketDate);
  }
}

const std::vector<SubCodeInfo*>&
SubCodeDAO::get(DeleteList& del,
                const VendorCode& vendor,
                const CarrierCode& carrier,
                const ServiceTypeCode& serviceTypeCode,
                const ServiceGroup& serviceGroup,
                const DateTime& date,
                const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  SubCodeKey key(vendor, carrier);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<SubCodeInfo*>* ret = new std::vector<SubCodeInfo*>;
  del.adopt(ret);

  remove_copy_if(
      ptr->begin(),
      ptr->end(),
      back_inserter(*ret),
      boost::bind<bool>(IsNotEffectiveG<SubCodeInfo>(date, ticketDate), _1) ||
          !boost::bind<bool>(IsSame<const ServiceTypeCode>(serviceTypeCode),
                             _1,
                             static_cast<const ServiceTypeCode& (SubCodeInfo::*)(void) const>(
                                 &SubCodeInfo::serviceTypeCode)) ||
          !boost::bind<bool>(IsSame<const ServiceGroup>(serviceGroup),
                             _1,
                             static_cast<const ServiceGroup& (SubCodeInfo::*)(void) const>(
                                 &SubCodeInfo::serviceGroup)));

  return *ret;
}

std::vector<SubCodeInfo*>*
SubCodeDAO::create(SubCodeKey key)
{
  std::vector<SubCodeInfo*>* ret = new std::vector<SubCodeInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSubCode fnc(dbAdapter->getAdapter());
    fnc.findSubCodeInfo(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SubCodeDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SubCodeDAO::destroy(SubCodeKey key, std::vector<SubCodeInfo*>* recs)
{
  destroyContainer(recs);
}

SubCodeKey
SubCodeDAO::createKey(SubCodeInfo* info)
{
  return SubCodeKey(info->vendor(), info->carrier());
}

void
SubCodeDAO::load()
{
  // not pre_loading
  StartupLoaderNoDB<SubCodeInfo, SubCodeDAO>();
}

sfc::CompressedData*
SubCodeDAO::compress(const std::vector<SubCodeInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<SubCodeInfo*>*
SubCodeDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<SubCodeInfo>(compressed);
}

///////////////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////////////
log4cxx::LoggerPtr
SubCodeHistoricalDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SubCodeHistoricalDAO"));

std::string
SubCodeHistoricalDAO::_name("SubCodeHistorical");
std::string
SubCodeHistoricalDAO::_cacheClass("Rules");
DAOHelper<SubCodeHistoricalDAO>
SubCodeHistoricalDAO::_helper(_name);
SubCodeHistoricalDAO* SubCodeHistoricalDAO::_instance = nullptr;

SubCodeHistoricalDAO&
SubCodeHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<SubCodeInfo*>&
SubCodeHistoricalDAO::get(DeleteList& del,
                          const VendorCode& vendor,
                          const CarrierCode& carrier,
                          const ServiceTypeCode& serviceTypeCode,
                          const ServiceGroup& serviceGroup,
                          const DateTime& date,
                          const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  SubCodeHistoricalKey key(vendor, carrier);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<SubCodeInfo*>* ret = new std::vector<SubCodeInfo*>;
  del.adopt(ret);

  remove_copy_if(
      ptr->begin(),
      ptr->end(),
      back_inserter(*ret),
      boost::bind<bool>(IsNotEffectiveH<SubCodeInfo>(date, ticketDate), _1) ||
          !boost::bind<bool>(IsSame<const ServiceTypeCode>(serviceTypeCode),
                             _1,
                             static_cast<const ServiceTypeCode& (SubCodeInfo::*)(void) const>(
                                 &SubCodeInfo::serviceTypeCode)) ||
          !boost::bind<bool>(IsSame<const ServiceGroup>(serviceGroup),
                             _1,
                             static_cast<const ServiceGroup& (SubCodeInfo::*)(void) const>(
                                 &SubCodeInfo::serviceGroup)));

  return *ret;
}

std::vector<SubCodeInfo*>*
SubCodeHistoricalDAO::create(SubCodeHistoricalKey key)
{
  std::vector<SubCodeInfo*>* ret = new std::vector<SubCodeInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSubCodeHistorical fnc(dbAdapter->getAdapter());
    fnc.findSubCodeInfo(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SubCodeHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SubCodeHistoricalDAO::destroy(SubCodeHistoricalKey key, std::vector<SubCodeInfo*>* recs)
{
  std::vector<SubCodeInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
SubCodeHistoricalDAO::compress(const std::vector<SubCodeInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<SubCodeInfo*>*
SubCodeHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<SubCodeInfo>(compressed);
}

} // tse
