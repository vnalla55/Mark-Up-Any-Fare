//------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
//

#include "DBAccess/BrandedFareAppDAO.h"

#include "Common/Logger.h"
#include "DBAccess/BrandedFareApp.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetBrandedFareApp.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
BrandedFareAppDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.BrandedFareAppDAO"));

BrandedFareAppDAO&
BrandedFareAppDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

struct BrandedFareAppDAO::isEffective : public std::unary_function<const BrandedFareApp*, bool>
{
  DateTime _travelDate;
  DateTime _ticketDate;
  bool _isHistorical;

  isEffective(const DateTime& travelDate, const DateTime& ticketDate, const bool isHistorical)
    : _travelDate(travelDate), _ticketDate(ticketDate), _isHistorical(isHistorical)
  {
  }

  bool operator()(const BrandedFareApp* rec) const
  {
    if (rec->tvlEffDate() > _travelDate)
      return false;
    if (_travelDate > rec->tvlDiscDate())
      return false;

    // Check Sale Effective and Discontinue Dates
    if (rec->effDate() > _ticketDate)
      return false;
    if (_ticketDate > rec->discDate())
      return false;

    if (_ticketDate > rec->expireDate())
      return false;

    if (_isHistorical)
    {
      if (rec->createDate().date() > _ticketDate.date())
        return false;
    }
    else
    {
      if (_travelDate > rec->expireDate())
        return false;
    }

    return true;
  }
};

const std::vector<BrandedFareApp*>&
getBrandedFareAppData(const Indicator& userApplType,
                      const UserApplCode& userAppl,
                      const CarrierCode& carrier,
                      const DateTime& travelDate,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical)
{
  BrandedFareAppDAO& dao = BrandedFareAppDAO::instance();
  return dao.get(deleteList, userApplType, userAppl, carrier, travelDate, ticketDate, isHistorical);
}

const std::vector<BrandedFareApp*>&
BrandedFareAppDAO::get(DeleteList& del,
                       const Indicator& userApplType,
                       const UserApplCode& userAppl,
                       const CarrierCode& carrier,
                       const DateTime& travelDate,
                       const DateTime& ticketDate,
                       bool isHistorical)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  BrandedFareAppKey key(userApplType, userAppl, carrier);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<BrandedFareApp*>* ret = new std::vector<BrandedFareApp*>;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), isEffective(travelDate, ticketDate, isHistorical));
  while (i != ptr->end())
  {
    ret->push_back(*i);
    i = find_if(++i, ptr->end(), isEffective(travelDate, ticketDate, isHistorical));
  }
  del.adopt(ret);
  return *ret;
}

std::vector<BrandedFareApp*>*
BrandedFareAppDAO::create(BrandedFareAppKey key)
{
  std::vector<BrandedFareApp*>* ret = new std::vector<BrandedFareApp*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetBrandedFareApp ds(dbAdapter->getAdapter());
    ds.findBrandedFareApp(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in BrandedFareAppDAO::create");
    throw;
  }

  return ret;
}

void
BrandedFareAppDAO::destroy(BrandedFareAppKey key, std::vector<BrandedFareApp*>* recs)
{
  destroyContainer(recs);
}

std::string
BrandedFareAppDAO::_name("BrandedFareApp");
std::string
BrandedFareAppDAO::_cacheClass("FareDisplay");

DAOHelper<BrandedFareAppDAO>
BrandedFareAppDAO::_helper(_name);

BrandedFareAppDAO* BrandedFareAppDAO::_instance = nullptr;

BrandedFareAppKey
BrandedFareAppDAO::createKey(const BrandedFareApp* info)
{
  return BrandedFareAppKey(info->userApplType(), info->userAppl(), info->carrier());
}

void
BrandedFareAppDAO::load()
{
  StartupLoaderNoDB<BrandedFareApp, BrandedFareAppDAO>();
}
} // namespace tse
