//-------------------------------------------------------------------------------
// Copyright 2006, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/IsMultilateralDAO.h"

#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/IndustryFareAppl.h"
#include "DBAccess/Queries/QueryGetIndFareAppl.h"

namespace tse
{
log4cxx::LoggerPtr
IsMultilateralDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.IsMultilateralDAO"));
IsMultilateralDAO&
IsMultilateralDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

std::multimap<RuleIndexKey, IndustryFareAppl*>*
IsMultilateralDAO::create(CharKey key)
{
  std::vector<IndustryFareAppl*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllIndFareAppl ifa(dbAdapter->getAdapter());
    ifa.findAllIndustryFareAppl(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in IsMultilateralDAO::create");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::multimap<RuleIndexKey, IndustryFareAppl*>* ret =
      new std::multimap<RuleIndexKey, IndustryFareAppl*>;

  std::vector<IndustryFareAppl*>::iterator i = recs.begin();
  for (; i != recs.end(); ++i)
  {
    IndustryFareAppl* info = *i;

    if (info->selectionType() == key._a)
    {
      RuleIndexKey rkey(info->vendor(), info->rule());
      ret->insert(std::pair<RuleIndexKey, IndustryFareAppl*>(rkey, info));
    }
    else
    { // free the ones that we're not using
      delete info;
      *i = nullptr;
    }
  }
  return ret;
}

void
IsMultilateralDAO::destroy(CharKey key, std::multimap<RuleIndexKey, IndustryFareAppl*>* recs)
{
  std::multimap<RuleIndexKey, IndustryFareAppl*>::iterator i = recs->begin();
  for (; i != recs->end(); ++i)
  {
    delete i->second;
  }
}

bool
isMultilateralData(const VendorCode& vendor,
                   const RuleNumber& rule,
                   const LocCode& loc1,
                   const LocCode& loc2,
                   const DateTime& date,
                   const DateTime& ticketDate,
                   bool isHistorical)
{
  return isMultilateralData(vendor, rule, loc1, loc2, date, date, ticketDate, isHistorical);
}

bool
isMultilateralData(const VendorCode& vendor,
                   const RuleNumber& rule,
                   const LocCode& loc1,
                   const LocCode& loc2,
                   const DateTime& startDate,
                   const DateTime& endDate,
                   const DateTime& ticketDate,
                   bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    IsMultilateralHistoricalDAO& hdao = IsMultilateralHistoricalDAO::instance();
    return hdao.isMultilateral(vendor, rule, loc1, loc2, startDate, ticketDate);
  }
  else
  {
    IsMultilateralDAO& dao = IsMultilateralDAO::instance();
    return dao.isMultilateral(vendor, rule, loc1, loc2, startDate, endDate, ticketDate);
  }
}

bool
IsMultilateralDAO::isMultilateral(const VendorCode& vendor,
                                  const RuleNumber& rule,
                                  const LocCode& loc1,
                                  const LocCode& loc2,
                                  const DateTime& startDate,
                                  const DateTime& endDate,
                                  const DateTime& ticketDate)
{
  DAOCache::pointer_type ptr = cache().get(CharKey('M'));

  RuleIndexKey key(vendor, rule);
  IsEffectiveG<IndustryFareAppl> dateTest(startDate, endDate, ticketDate);
  std::multimap<RuleIndexKey, IndustryFareAppl*>::iterator i = ptr->find(key);
  while (i != ptr->end())
  {
    if (!(i->first == key))
      return false;
    if (dateTest(i->second))
    {
      IndustryFareAppl& ifa = *(i->second);
      switch (ifa.directionality())
      {
      case ' ': // any origin and destination
        return true;
      case 'B': // between
        if ((ifa.loc1().loc().empty() ||
             LocUtil::isInLoc(loc2, ifa.loc1().locType(), ifa.loc1().loc(), ifa.vendor())) &&
            (ifa.loc2().loc().empty() ||
             LocUtil::isInLoc(loc1, ifa.loc2().locType(), ifa.loc2().loc(), ifa.vendor())))
          return true;
      // fall through; F satisfies B also
      case 'F': // from
        if ((ifa.loc1().loc().empty() ||
             LocUtil::isInLoc(loc1, ifa.loc1().locType(), ifa.loc1().loc(), ifa.vendor())) &&
            (ifa.loc2().loc().empty() ||
             LocUtil::isInLoc(loc2, ifa.loc2().locType(), ifa.loc2().loc(), ifa.vendor())))
          return true;
        break;
      case 'W': // within
        if (LocUtil::isInLoc(loc1, ifa.loc1().locType(), ifa.loc1().loc(), ifa.vendor()) &&
            LocUtil::isInLoc(loc2, ifa.loc2().locType(), ifa.loc2().loc(), ifa.vendor()))
          return true;
      }
    }
    ++i;
  }
  return false;
}

CharKey
IsMultilateralDAO::createKey(IndustryFareAppl* info)
{
  return CharKey('M');
}

void
IsMultilateralDAO::load()
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
    CharKey key('M');
    std::multimap<RuleIndexKey, IndustryFareAppl*>* obj(create(key));
    cache().put(key, obj, true);
    setLoadSource(DiskCache::LOADSOURCE_DB);
  }
}

std::string
IsMultilateralDAO::_name("IsMultilateral");
std::string
IsMultilateralDAO::_cacheClass("Fares");
DAOHelper<IsMultilateralDAO>
IsMultilateralDAO::_helper(_name);
IsMultilateralDAO* IsMultilateralDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: IsMultilateralHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
IsMultilateralHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.IsMultilateralHistoricalDAO"));
IsMultilateralHistoricalDAO&
IsMultilateralHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

std::multimap<RuleIndexKey, IndustryFareAppl*>*
IsMultilateralHistoricalDAO::create(Indicator key)
{
  std::vector<IndustryFareAppl*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllIndFareApplHistorical ifa(dbAdapter->getAdapter());
    ifa.findAllIndustryFareAppl(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in IsMultilateralHistoricalDAO::create");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::multimap<RuleIndexKey, IndustryFareAppl*>* ret =
      new std::multimap<RuleIndexKey, IndustryFareAppl*>;

  std::vector<IndustryFareAppl*>::iterator i = recs.begin();
  for (; i != recs.end(); ++i)
  {
    IndustryFareAppl* info = *i;

    if (info->selectionType() == key)
    {
      RuleIndexKey rkey(info->vendor(), info->rule());
      ret->insert(std::pair<RuleIndexKey, IndustryFareAppl*>(rkey, info));
    }
    else
    { // free the ones that we're not using
      delete info;
      *i = nullptr;
    }
  }
  return ret;
}

void
IsMultilateralHistoricalDAO::destroy(Indicator key,
                                     std::multimap<RuleIndexKey, IndustryFareAppl*>* recs)
{
  std::multimap<RuleIndexKey, IndustryFareAppl*>::iterator i = recs->begin();
  for (; i != recs->end(); ++i)
  {
    delete i->second;
  }
}

bool
IsMultilateralHistoricalDAO::isMultilateral(const VendorCode& vendor,
                                            const RuleNumber& rule,
                                            const LocCode& loc1,
                                            const LocCode& loc2,
                                            const DateTime& date,
                                            const DateTime& ticketDate)
{
  DAOCache::pointer_type ptr = cache().get('M');

  RuleIndexKey key(vendor, rule);
  IsEffectiveHist<IndustryFareAppl> dateTest(date, ticketDate);
  std::multimap<RuleIndexKey, IndustryFareAppl*>::iterator i = ptr->find(key);
  while (i != ptr->end())
  {
    if (!(i->first == key))
      return false;
    if (dateTest(i->second))
    {
      IndustryFareAppl& ifa = *(i->second);
      switch (ifa.directionality())
      {
      case ' ': // any origin and destination
        return true;
      case 'B': // between
        if ((ifa.loc1().loc().empty() ||
             LocUtil::isInLoc(loc2, ifa.loc1().locType(), ifa.loc1().loc(), ifa.vendor())) &&
            (ifa.loc2().loc().empty() ||
             LocUtil::isInLoc(loc1, ifa.loc2().locType(), ifa.loc2().loc(), ifa.vendor())))
          return true;
      // fall through; F satisfies B also
      case 'F': // from
        if ((ifa.loc1().loc().empty() ||
             LocUtil::isInLoc(loc1, ifa.loc1().locType(), ifa.loc1().loc(), ifa.vendor())) &&
            (ifa.loc2().loc().empty() ||
             LocUtil::isInLoc(loc2, ifa.loc2().locType(), ifa.loc2().loc(), ifa.vendor())))
          return true;
        break;
      case 'W': // within
        if (LocUtil::isInLoc(loc1, ifa.loc1().locType(), ifa.loc1().loc(), ifa.vendor()) &&
            LocUtil::isInLoc(loc2, ifa.loc2().locType(), ifa.loc2().loc(), ifa.vendor()))
          return true;
      }
    }
    ++i;
  }
  return false;
}

void
IsMultilateralHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  cache().put('M', create('M'));
}

std::string
IsMultilateralHistoricalDAO::_name("IsMultilateralHistorical");
std::string
IsMultilateralHistoricalDAO::_cacheClass("Fares");
DAOHelper<IsMultilateralHistoricalDAO>
IsMultilateralHistoricalDAO::_helper(_name);
IsMultilateralHistoricalDAO* IsMultilateralHistoricalDAO::_instance = nullptr;

} // namespace tse
