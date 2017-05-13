//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/PaxTypeDAO.h"

#include "Common/Logger.h"
#include "Common/TseUtil.h"
#include "Common/Vendor.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/Queries/QueryGetPaxType.h"

namespace tse
{
log4cxx::LoggerPtr
PaxTypeDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.PaxTypeDAO"));
PaxTypeDAO&
PaxTypeDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const PaxTypeInfo*
getPaxTypeData(const PaxTypeCode& paxType,
               const VendorCode& vendor,
               DeleteList& deleteList,
               const DateTime& ticketDate,
               bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    PaxTypeHistoricalDAO& dao = PaxTypeHistoricalDAO::instance();
    return dao.get(deleteList, paxType, vendor, ticketDate);
  }
  else
  {
    PaxTypeDAO& dao = PaxTypeDAO::instance();
    return dao.get(deleteList, paxType, vendor, ticketDate);
  }
}

const PaxTypeInfo*
PaxTypeDAO::get(DeleteList& del,
                const PaxTypeCode& paxType,
                const VendorCode& vendor,
                const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(IntKey(0));

  PaxTypeKey key(paxType, vendor);
  DAOCache::value_type::iterator i = ptr->find(key);
  if (i == ptr->end())
  {
    VendorCode vendor2(vendor);
    if (isDigit(paxType[2]) && isalpha(paxType[0]) && isalpha(paxType[1]))
      vendor2 = Vendor::SABRE;
    else if (vendor != Vendor::ATPCO && vendor != Vendor::SITA)
      vendor2 = Vendor::ATPCO;

    PaxTypeKey key2(paxType, vendor2);
    i = ptr->find(key2);
    if (i == ptr->end())
    {
      return nullptr;
    }
    else
    {
      del.copy(ptr);
      return i->second;
    }
  }
  else
  {
    del.copy(ptr);
    return i->second;
  }
}

const std::vector<PaxTypeInfo*>&
getAllPaxTypeData(DeleteList& deleteList, const DateTime& ticketDate, bool isHistorical)
{
  if (isHistorical)
  {
    PaxTypeHistoricalDAO& dao = PaxTypeHistoricalDAO::instance();
    return dao.getAll(deleteList, ticketDate);
  }
  else
  {
    PaxTypeDAO& dao = PaxTypeDAO::instance();
    return dao.getAll(deleteList);
  }
}

const std::vector<PaxTypeInfo*>&
PaxTypeDAO::getAll(DeleteList& del)
{
  // Track calls for code coverage
  _codeCoverageGetAllCallCount++;

  std::vector<PaxTypeInfo*>* recs = new std::vector<PaxTypeInfo*>;
  del.adopt(recs);
  DAOCache::pointer_type ptr = cache().get(IntKey(0));
  del.copy(ptr);
  DAOCache::value_type::const_iterator iter = ptr->begin();
  DAOCache::value_type::const_iterator iterEnd = ptr->end();
  for (; iter != iterEnd; iter++)
  {
    recs->push_back(iter->second);
  }
  return *recs;
}

void
PaxTypeDAO::load()
{
  // Track calls for code coverage
  incrementLoadCallCount();

  setLoadSource(DiskCache::LOADSOURCE_OTHER);
  if (ldcHelper().loadFromLDC())
  {
    setLoadSource(DiskCache::LOADSOURCE_LDC);
  }
  else
  {
    IntKey key(0);
    std::multimap<PaxTypeKey, PaxTypeInfo*>* obj(create(key));
    cache().put(key, obj, true);
    setLoadSource(DiskCache::LOADSOURCE_DB);
  }
}

IntKey
PaxTypeDAO::createKey(PaxTypeInfo* info)
{
  return IntKey(0);
}

bool
PaxTypeDAO::insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
{
  PaxTypeInfo* info(new PaxTypeInfo);
  PaxTypeInfo::dummyData(*info);
  DAOCache::pointer_type ptr = cache().get(IntKey(0));
  PaxTypeKey key(info->paxType(), info->vendor());
  DAOCache::value_type::iterator i = ptr->find(key);
  if (i == ptr->end())
  {
    ptr->insert(std::pair<PaxTypeKey, PaxTypeInfo*>(key, info));
    cache().getCacheImpl()->queueDiskPut(IntKey(0), true);
  }
  return true;
}

std::multimap<PaxTypeKey, PaxTypeInfo*>*
PaxTypeDAO::create(IntKey key)
{
  std::vector<PaxTypeInfo*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetPaxTypes pt(dbAdapter->getAdapter());
    pt.findAllPaxType(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in PaxTypeDAO::create");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::multimap<PaxTypeKey, PaxTypeInfo*>* ret = new std::multimap<PaxTypeKey, PaxTypeInfo*>;

  std::vector<PaxTypeInfo*>::iterator i = recs.begin();
  for (; i != recs.end(); ++i)
  {
    PaxTypeInfo* info = *i;
    PaxTypeKey rkey(info->paxType(), info->vendor());
    ret->insert(std::pair<PaxTypeKey, PaxTypeInfo*>(rkey, info));
  }
  return ret;
}

void
PaxTypeDAO::destroy(IntKey key, std::multimap<PaxTypeKey, PaxTypeInfo*>* recs)
{
  std::multimap<PaxTypeKey, PaxTypeInfo*>::const_iterator iter = recs->begin();
  std::multimap<PaxTypeKey, PaxTypeInfo*>::const_iterator iterEnd = recs->end();
  for (; iter != iterEnd; iter++)
  {
    delete iter->second;
  }
  delete recs;
}

std::string
PaxTypeDAO::_name("PaxType");
std::string
PaxTypeDAO::_cacheClass("Common");
DAOHelper<PaxTypeDAO>
PaxTypeDAO::_helper(_name);
PaxTypeDAO* PaxTypeDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: PaxTypeHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
PaxTypeHistoricalDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.PaxTypeHistoricalDAO"));
PaxTypeHistoricalDAO&
PaxTypeHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const PaxTypeInfo*
PaxTypeHistoricalDAO::get(DeleteList& del,
                          const PaxTypeCode& paxType,
                          const VendorCode& vendor,
                          const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(dummy);

  PaxTypeKey key(paxType, vendor);
  DAOCache::value_type::iterator i = ptr->find(key);
  if (i == ptr->end())
  {
    VendorCode vendor2(vendor);
    if (isDigit(paxType[2]) && isalpha(paxType[0]) && isalpha(paxType[1]))
      vendor2 = Vendor::SABRE;
    else if (vendor != Vendor::ATPCO && vendor != Vendor::SITA)
      vendor2 = Vendor::ATPCO;

    PaxTypeKey key2(paxType, vendor2);
    i = ptr->find(key2);
    if (i == ptr->end())
    {
      return nullptr;
    }
    else
    {
      del.copy(ptr);
      return i->second;
    }
  }
  else
  {
    del.copy(ptr);
    return i->second;
  }
}

const std::vector<PaxTypeInfo*>&
PaxTypeHistoricalDAO::getAll(DeleteList& del, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetAllCallCount++;

  std::vector<PaxTypeInfo*>* recs = new std::vector<PaxTypeInfo*>;
  del.adopt(recs);
  DAOCache::pointer_type ptr = cache().get(dummy);
  del.copy(ptr);
  DAOCache::value_type::const_iterator iter = ptr->begin();
  DAOCache::value_type::const_iterator iterEnd = ptr->end();
  IsCurrentCreateDateOnly<PaxTypeInfo> isCurrent(ticketDate);
  for (; iter != iterEnd; iter++)
  {
    if (isCurrent(iter->second))
      recs->push_back(iter->second);
  }
  return *recs;
}

void
PaxTypeHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  cache().put(dummy, create(dummy));
}

std::multimap<PaxTypeKey, PaxTypeInfo*>*
PaxTypeHistoricalDAO::create(int key)
{
  std::vector<PaxTypeInfo*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetPaxTypes pt(dbAdapter->getAdapter());
    pt.findAllPaxType(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in PaxTypeHistoricalDAO::create");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::multimap<PaxTypeKey, PaxTypeInfo*>* ret = new std::multimap<PaxTypeKey, PaxTypeInfo*>;

  std::vector<PaxTypeInfo*>::iterator i = recs.begin();
  for (; i != recs.end(); ++i)
  {
    PaxTypeInfo* info = *i;

    PaxTypeKey rkey(info->paxType(), info->vendor());
    ret->insert(std::pair<PaxTypeKey, PaxTypeInfo*>(rkey, info));
  }
  return ret;
}

void
PaxTypeHistoricalDAO::destroy(int key, std::multimap<PaxTypeKey, PaxTypeInfo*>* recs)
{
  std::multimap<PaxTypeKey, PaxTypeInfo*>::const_iterator iter = recs->begin();
  std::multimap<PaxTypeKey, PaxTypeInfo*>::const_iterator iterEnd = recs->end();
  for (; iter != iterEnd; iter++)
  {
    delete iter->second;
  }
  delete recs;
}

std::string
PaxTypeHistoricalDAO::_name("PaxTypeHistorical");
std::string
PaxTypeHistoricalDAO::_cacheClass("Common");
const int PaxTypeHistoricalDAO::dummy;
DAOHelper<PaxTypeHistoricalDAO>
PaxTypeHistoricalDAO::_helper(_name);
PaxTypeHistoricalDAO* PaxTypeHistoricalDAO::_instance = nullptr;

} // namespace tse
