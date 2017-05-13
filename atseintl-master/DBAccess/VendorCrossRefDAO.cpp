//-------------------------------------------------------------------------------
// Copyright 2004, 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/VendorCrossRefDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetVendXref.h"
#include "DBAccess/VendorCrossRef.h"

#include <algorithm>
#include <functional>

namespace tse
{

log4cxx::LoggerPtr
VendorCrossRefDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.VendorCrossRefDAO"));

VendorCrossRefDAO&
VendorCrossRefDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

// ATP and SITA vendors are queried frequently, and they never change.
// To avoid having to go to the cache, and causing contention over
// the cache to get these vendors, we instead store them here.
// This data is read-only and should only be written to at load-time.
//
// This code specifically assumes that ATP and SITA never change. If
// they did change, the application will not recognize the change
// until restart. This optimization saves a significant amount of time,
// since Vendor Cross Reference information is queried from many other
// DAOs, often resulting in millions of queries in a transaction.
// Without this optimization, these queries would result in having
// to lock the cache, and cause contention between threads.
VendorCrossRef* VendorCrossRefDAO::ATPVendor = nullptr;
VendorCrossRef* VendorCrossRefDAO::SITAVendor = nullptr;

const VendorCrossRef*
VendorCrossRefDAO::get(DeleteList& del,
                       const VendorCode& vendor,
                       const DateTime& ticketDate,
                       bool isHistorical)
{
  // Track calls for code coverage
  // The tarcking is commented to avoid L1 Cache Miss - it wasn found when
  // profiling the application using Intel VTune PerformanceAnalyzer.
  //_codeCoverageGetCallCount++;

  std::vector<VendorCrossRef*>* vcr = nullptr;
  if (UNLIKELY(vendor == VENDOR_ATP))
  {
    return ATPVendor;
  }
  else if (UNLIKELY(vendor == VENDOR_SITA))
  {
    return SITAVendor;
  }

  if (LIKELY(vcr == nullptr))
  {
    VendorCrossRefKey key(vendor);
    vcr = cache().get(key).get();
    // Cache Notification for this table is disabled because the rows in
    // this table don't change unless there is a supporting activity to
    // change the vendor code of the R3/TCR. If such a situation arises
    // the servers will be bounced to pickup the changes.
    // Since cache notification is disabled, the cache pointer doesn't
    // doesn't need to be copied/registered in the DeleteList.
    // del.copy(ptr);
  }

  if (vcr->empty())
    return nullptr;

  // "Historical" check cuz Tony Sez so
  if (UNLIKELY(isHistorical && (vcr->front()->createDate() > ticketDate)))
    return nullptr;

  return vcr->front();
}

VendorCrossRefKey
VendorCrossRefDAO::createKey(VendorCrossRef* info)
{
  return VendorCrossRefKey(info->vendor());
}

void
VendorCrossRefDAO::load()
{
  StartupLoader<QueryGetAllVendXref, VendorCrossRef, VendorCrossRefDAO>();
  ATPVendor = cache().get(VendorCrossRefKey(VENDOR_ATP))->front();
  SITAVendor = cache().get(VendorCrossRefKey(VENDOR_SITA))->front();
}

std::vector<VendorCrossRef*>*
VendorCrossRefDAO::create(VendorCrossRefKey key)
{
  std::vector<VendorCrossRef*>* ret = new std::vector<VendorCrossRef*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetVendXref vxr(dbAdapter->getAdapter());
    vxr.findVendorCrossRef(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in VendorCrossRefDAO::create");
    throw;
  }
  return ret;
}

void
VendorCrossRefDAO::destroy(VendorCrossRefKey key, std::vector<VendorCrossRef*>* recs)
{
  destroyContainer(recs);
}

std::string
VendorCrossRefDAO::_name("VendorCrossRef");

DAOHelper<VendorCrossRefDAO>
VendorCrossRefDAO::_helper(_name);

VendorCrossRefDAO* VendorCrossRefDAO::_instance = nullptr;

} // namespace tse
