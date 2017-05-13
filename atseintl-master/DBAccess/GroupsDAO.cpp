//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/GroupsDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Groups.h"
#include "DBAccess/Queries/QueryGetGroups.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
GroupsDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.GroupsDAO"));

GroupsDAO&
GroupsDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const Groups*
getGroupsData(const VendorCode& vendor,
              int itemNo,
              DeleteList& deleteList,
              const DateTime& ticketDate,
              bool isHistorical)
{
  if (isHistorical)
  {
    GroupsHistoricalDAO& dao = GroupsHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    GroupsDAO& dao = GroupsDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const Groups*
GroupsDAO::get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  GroupsKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentG<Groups> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<Groups>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<Groups*>*
GroupsDAO::create(GroupsKey key)
{
  std::vector<Groups*>* ret = new std::vector<Groups*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetGroups grp(dbAdapter->getAdapter());
    grp.findGroups(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in GroupsDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
GroupsDAO::destroy(GroupsKey key, std::vector<Groups*>* recs)
{
  std::vector<Groups*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
GroupsDAO::_name("Groups");
std::string
GroupsDAO::_cacheClass("Rules");
DAOHelper<GroupsDAO>
GroupsDAO::_helper(_name);
GroupsDAO* GroupsDAO::_instance = nullptr;

GroupsKey
GroupsDAO::createKey(const Groups* info)
{
  return GroupsKey(info->vendor(), info->itemNo());
}

void
GroupsDAO::load()
{
  StartupLoaderNoDB<Groups, GroupsDAO>();
}

// --------------------------------------------------
// Historical DAO: GroupsHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
GroupsHistoricalDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.GroupsHistoricalDAO"));
GroupsHistoricalDAO&
GroupsHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const Groups*
GroupsHistoricalDAO::get(DeleteList& del,
                         const VendorCode& vendor,
                         int itemNo,
                         const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  GroupsHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<Groups> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<Groups>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<Groups*>*
GroupsHistoricalDAO::create(GroupsHistoricalKey key)
{
  std::vector<Groups*>* ret = new std::vector<Groups*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetGroupsHistorical grp(dbAdapter->getAdapter());
    grp.findGroups(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in GroupsHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
GroupsHistoricalDAO::destroy(GroupsHistoricalKey key, std::vector<Groups*>* recs)
{
  std::vector<Groups*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
GroupsHistoricalDAO::_name("GroupsHistorical");
std::string
GroupsHistoricalDAO::_cacheClass("Rules");
DAOHelper<GroupsHistoricalDAO>
GroupsHistoricalDAO::_helper(_name);
GroupsHistoricalDAO* GroupsHistoricalDAO::_instance = nullptr;

} // namespace tse
