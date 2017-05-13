//----------------------------------------------------------------------------
//          File:           CxrScheduleSegmentLocsDAO.cpp
//          Description:    CxrScheduleSegmentLocsDAO
//          Created:        11/01/2010
//          Authors:
//
//          Updates:
//
//     � 2010, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/CxrScheduleSegmentLocsDAO.h"

#include "Common/Logger.h"
#include "DBAccess/CxrScheduleSegmentLocs.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetCxrScheduleSegmentLocs.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
CxrScheduleSegmentLocsDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.CxrScheduleSegmentLocsDAO"));

CxrScheduleSegmentLocsDAO&
CxrScheduleSegmentLocsDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const CxrScheduleSegmentLocs&
CxrScheduleSegmentLocsDAO::get(DeleteList& del, const CarrierCode& cxr)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  CxrScheduleSegmentLocsKey key(cxr);
  DAOCache::pointer_type ptr = cache().get(key);

  del.copy(ptr);

  return *ptr;
}

CxrScheduleSegmentLocs*
CxrScheduleSegmentLocsDAO::create(CxrScheduleSegmentLocsKey key)
{
  CxrScheduleSegmentLocs* ret = new CxrScheduleSegmentLocs();

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCxrScheduleSegmentLocs at(dbAdapter->getAdapter());
    at.findCxrScheduleSegmentLocs(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CxrScheduleSegmentLocsDAO::create");
    delete ret;
    throw;
  }

  return ret;
}

void
CxrScheduleSegmentLocsDAO::destroy(CxrScheduleSegmentLocsKey key, CxrScheduleSegmentLocs* recs)
{
  delete recs;
}

CxrScheduleSegmentLocsKey
CxrScheduleSegmentLocsDAO::createKey(CxrScheduleSegmentLocs*& info)
{
  return CxrScheduleSegmentLocsKey(info->carrier());
}

std::string
CxrScheduleSegmentLocsDAO::_name("CxrScheduleSegmentLocs");
std::string
CxrScheduleSegmentLocsDAO::_cacheClass("Common");
DAOHelper<CxrScheduleSegmentLocsDAO>
CxrScheduleSegmentLocsDAO::_helper(_name);
CxrScheduleSegmentLocsDAO* CxrScheduleSegmentLocsDAO::_instance = nullptr;
} // namespace tse
