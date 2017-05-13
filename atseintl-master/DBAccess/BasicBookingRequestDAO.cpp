//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/BasicBookingRequestDAO.h"

#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DBAccess/BasicBookingRequest.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetBasicBookingRequest.h"

#include <algorithm>
#include <functional>

struct IsCurrentRecord : public std::binary_function<tse::BasicBookingRequest*, tse::DateTime, bool>
{
  bool operator()(tse::BasicBookingRequest* bbr, const tse::DateTime& date) const
  {
    return (bbr->effDate() <= date && date <= bbr->discDate());
  }
};

struct IsNotCurrentRecord
    : public std::binary_function<tse::BasicBookingRequest*, tse::DateTime, bool>
{
  bool operator()(tse::BasicBookingRequest* bbr, const tse::DateTime& date) const
  {
    if (bbr->effDate() > date && bbr->discDate() > date)
      return true;

    return false;
  }
};

namespace tse
{
log4cxx::LoggerPtr
BasicBookingRequestDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.BasicBookingRequestDAO"));

BasicBookingRequestDAO&
BasicBookingRequestDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const BasicBookingRequest*
BasicBookingRequestDAO::get(DeleteList& del, const CarrierCode& cxr, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(CarrierKey(cxr));
  del.copy(ptr);
  BasicBookingRequest* ret = nullptr;

  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), bind2nd(IsCurrentRecord(), ticketDate));

  if (i != ptr->end())
    ret = *i;

  return ret;
}

const std::vector<BasicBookingRequest*>&
BasicBookingRequestDAO::getAll(DeleteList& del, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetAllCallCount++;

  std::vector<BasicBookingRequest*>* recs = new std::vector<BasicBookingRequest*>;
  del.adopt(recs);

  remove_copy_if(loadList.begin(),
                 loadList.end(),
                 back_inserter(*recs),
                 bind2nd(IsNotCurrentRecord(), ticketDate));
  return *recs;
}

std::vector<BasicBookingRequest*>*
BasicBookingRequestDAO::create(CarrierKey key)
{
  std::vector<BasicBookingRequest*>* ret = new std::vector<BasicBookingRequest*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetBasicBookingRequest queryGetBBR(dbAdapter->getAdapter());
    queryGetBBR.findBasicBookingRequest(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in BasicBookingRequestDAO::create");
    throw;
  }

  return ret;
}

void
BasicBookingRequestDAO::destroy(CarrierKey key, std::vector<BasicBookingRequest*>* recs)
{
  destroyContainer(recs);
}

CarrierKey
BasicBookingRequestDAO::createKey(BasicBookingRequest* info)
{
  return CarrierKey(info->carrier());
}

void
BasicBookingRequestDAO::load()
{
  StartupLoader<QueryGetBasicBookingRequests, BasicBookingRequest, BasicBookingRequestDAO>();
  deepCopyCache(loadList);
}

std::string
BasicBookingRequestDAO::_name("BasicBookingRequest");
std::string
BasicBookingRequestDAO::_cacheClass("BasicBookingRequest");

DAOHelper<BasicBookingRequestDAO>
BasicBookingRequestDAO::_helper(_name);

BasicBookingRequestDAO* BasicBookingRequestDAO::_instance = nullptr;

} // namespace tse
