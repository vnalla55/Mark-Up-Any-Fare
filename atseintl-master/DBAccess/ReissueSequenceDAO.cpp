//-------------------------------------------------------------------------------
// @ 2007, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/ReissueSequenceDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetTable988.h"
#include "DBAccess/ReissueSequence.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

namespace tse
{
log4cxx::LoggerPtr
ReissueSequenceDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.ReissueSequenceDAO"));
ReissueSequenceDAO&
ReissueSequenceDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<ReissueSequence*>&
getReissueData(const VendorCode& vendor,
               int itemNo,
               const DateTime& ticketDate,
               DeleteList& deleteList,
               bool isHistorical,
               const DateTime& applDate)
{
  if (isHistorical)
  {
    ReissueSequenceHistoricalDAO& dao = ReissueSequenceHistoricalDAO::instance();
    if (applDate != DateTime::emptyDate())
    {
      const std::vector<ReissueSequence*>& ret = dao.get(deleteList, vendor, itemNo, applDate);
      if (ret.size() > 0)
        return ret;
    }
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    ReissueSequenceDAO& dao = ReissueSequenceDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

std::vector<ReissueSequence*>&
ReissueSequenceDAO::get(DeleteList& del,
                        const VendorCode& vendor,
                        int itemNo,
                        const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  ReissueSequenceKey key(vendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<ReissueSequence*>* ret = new std::vector<ReissueSequence*>;
  del.adopt(ret);

  IsCurrentG<ReissueSequence> isCurrent(ticketDate);
  std::vector<ReissueSequence*>::const_iterator i = ptr->begin();
  std::vector<ReissueSequence*>::const_iterator ie = ptr->end();
  for (; i != ie; i++)
  {
    if (isCurrent(*i) && NotInhibit<ReissueSequence>(*i))
      ret->push_back(*i);
  }
  return *ret;
}

std::vector<ReissueSequence*>*
ReissueSequenceDAO::create(ReissueSequenceKey key)
{
  std::vector<ReissueSequence*>* ret = new std::vector<ReissueSequence*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTable988 query(dbAdapter->getAdapter());
    query.findReissue(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in ReissueSequenceDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
ReissueSequenceDAO::destroy(ReissueSequenceKey key, std::vector<ReissueSequence*>* recs)
{
  std::vector<ReissueSequence*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
ReissueSequenceDAO::_name("ReissueSequence");
std::string
ReissueSequenceDAO::_cacheClass("Rules");
DAOHelper<ReissueSequenceDAO>
ReissueSequenceDAO::_helper(_name);
ReissueSequenceDAO* ReissueSequenceDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: ReissueSequenceHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
ReissueSequenceHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.ReissueSequenceHistoricalDAO"));
ReissueSequenceHistoricalDAO&
ReissueSequenceHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

std::vector<ReissueSequence*>&
ReissueSequenceHistoricalDAO::get(DeleteList& del,
                                  const VendorCode& vendor,
                                  int itemNo,
                                  const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  ReissueSequenceHistoricalKey key(vendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<ReissueSequence*>* ret = new std::vector<ReissueSequence*>;
  del.adopt(ret);

  IsCurrentH<ReissueSequence> isCurrent(ticketDate);
  std::vector<ReissueSequence*>::const_iterator i = ptr->begin();
  std::vector<ReissueSequence*>::const_iterator ie = ptr->end();
  for (; i != ie; i++)
  {
    if (isCurrent(*i) && NotInhibit<ReissueSequence>(*i))
      ret->push_back(*i);
  }

  return *ret;
}

std::vector<ReissueSequence*>*
ReissueSequenceHistoricalDAO::create(ReissueSequenceHistoricalKey key)
{
  std::vector<ReissueSequence*>* ret = new std::vector<ReissueSequence*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTable988Historical query(dbAdapter->getAdapter());
    query.findReissue(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in ReissueSequenceHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
ReissueSequenceHistoricalDAO::destroy(ReissueSequenceHistoricalKey key,
                                      std::vector<ReissueSequence*>* recs)
{
  std::vector<ReissueSequence*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
ReissueSequenceHistoricalDAO::_name("ReissueSequenceHistorical");
std::string
ReissueSequenceHistoricalDAO::_cacheClass("Rules");
DAOHelper<ReissueSequenceHistoricalDAO>
ReissueSequenceHistoricalDAO::_helper(_name);
ReissueSequenceHistoricalDAO* ReissueSequenceHistoricalDAO::_instance = nullptr;

} // namespace tse
