#include "DBAccess/FareFocusFareClassDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareFocusFareClassInfo.h"
#include "DBAccess/Queries/QueryGetFareFocusFareClass.h"

namespace tse
{
log4cxx::LoggerPtr
FareFocusFareClassDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareFocusFareClassDAO"));

FareFocusFareClassDAO&
FareFocusFareClassDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareFocusFareClassInfo*>&
getFareFocusFareClassData(uint64_t fareClassItemNo,
                          DateTime adjustedTicketDate,
                          DeleteList& deleteList,
                          DateTime ticketDate,
                          bool isHistorical)
{
  if (isHistorical)
  {
    FareFocusFareClassHistoricalDAO& dao(FareFocusFareClassHistoricalDAO::instance());

    const std::vector<FareFocusFareClassInfo*>& ret(dao.get(deleteList, fareClassItemNo, ticketDate));

    return ret;
  }
  else
  {
    FareFocusFareClassDAO& dao(FareFocusFareClassDAO::instance());

    const std::vector<FareFocusFareClassInfo*>& ret(dao.get(deleteList, fareClassItemNo, adjustedTicketDate, ticketDate));

    return ret;
  }
}

const std::vector<FareFocusFareClassInfo*>&
FareFocusFareClassDAO::get(DeleteList& del,
                           uint64_t fareClassItemNo,
                           DateTime adjustedTicketDate,
                           DateTime ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  FareFocusFareClassKey key(fareClassItemNo);
  DAOCache::pointer_type ptr(cache().get(key));
  IsNotCurrentG<FareFocusFareClassInfo> isNotCurrent(ticketDate);
  return *(applyFilter(del, ptr, isNotCurrent));
}

std::vector<FareFocusFareClassInfo*>*
FareFocusFareClassDAO::create(FareFocusFareClassKey key)
{
  std::vector<FareFocusFareClassInfo*>* ret(new std::vector<FareFocusFareClassInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get(this->cacheClass()));
  try
  {
    QueryGetFareFocusFareClass q(dbAdapter->getAdapter());
    q.findFareFocusFareClass(*ret, key._a);
    /*
    std::cerr << "  !! QueryGetFareFocusFareClass: " << ' ' << ret->size() << " !!" << std::endl;
    std::vector<FareFocusFareClassInfo*> *all(new std::vector<FareFocusFareClassInfo*>);
    QueryGetAllFareFocusFareClass qall(dbAdapter->getAdapter());
    qall.findAllFareFocusFareClass(*all);
    std::cerr << "  !! QueryGetAllFareFocusFareClass: " << ' ' << all->size() << " !!" << std::endl;
    */
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareFocusFareClassDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareFocusFareClassKey
FareFocusFareClassDAO::createKey(const FareFocusFareClassInfo* info)
{
  return FareFocusFareClassKey(info->fareClassItemNo());
}

void
FareFocusFareClassDAO::destroy(FareFocusFareClassKey, std::vector<FareFocusFareClassInfo*>* recs)
{
  destroyContainer(recs);
}

sfc::CompressedData*
FareFocusFareClassDAO::compress(const std::vector<FareFocusFareClassInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<FareFocusFareClassInfo*>*
FareFocusFareClassDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<FareFocusFareClassInfo>(compressed);
}

std::string
FareFocusFareClassDAO::_name("FareFocusFareClass");
std::string
FareFocusFareClassDAO::_cacheClass("Rules");
DAOHelper<FareFocusFareClassDAO>
FareFocusFareClassDAO::_helper(_name);
FareFocusFareClassDAO*
FareFocusFareClassDAO::_instance(nullptr);

// Historical DAO Object
log4cxx::LoggerPtr
FareFocusFareClassHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareFocusFareClassHistoricalDAO"));

FareFocusFareClassHistoricalDAO&
FareFocusFareClassHistoricalDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareFocusFareClassInfo*>&
FareFocusFareClassHistoricalDAO::get(DeleteList& del,
                                     uint64_t fareClassItemNo,
                                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  FareFocusFareClassHistoricalKey key(fareClassItemNo);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr(cache().get(key));
  del.copy(ptr);
  std::vector<FareFocusFareClassInfo*>* ret(new std::vector<FareFocusFareClassInfo*>);
  del.adopt(ret);
  IsNotCurrentDH<FareFocusFareClassInfo> isNotCurrent(ticketDate);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), isNotCurrent);
  return *ret;
}

std::vector<FareFocusFareClassInfo*>*
FareFocusFareClassHistoricalDAO::create(FareFocusFareClassHistoricalKey key)
{
  std::vector<FareFocusFareClassInfo*>* ret(new std::vector<FareFocusFareClassInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get("Historical", true));
  try
  {
    QueryGetFareFocusFareClassHistorical q(dbAdapter->getAdapter());
    q.findFareFocusFareClass(*ret, key._a, key._b, key._c);
    //std::cerr << "!!!! QueryGetFareFocusFareClassHistorical: " << ' ' << ret->size() << std::endl;
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareFocusFareClassHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareFocusFareClassHistoricalKey
FareFocusFareClassHistoricalDAO::createKey(const FareFocusFareClassInfo* info,
                                           const DateTime& startDate,
                                           const DateTime& endDate)
{
  return FareFocusFareClassHistoricalKey(info->fareClassItemNo(), startDate, endDate);
}

void
FareFocusFareClassHistoricalDAO::destroy(FareFocusFareClassHistoricalKey,
                                         std::vector<FareFocusFareClassInfo*>* recs)
{
  destroyContainer(recs);
}

sfc::CompressedData*
FareFocusFareClassHistoricalDAO::compress(const std::vector<FareFocusFareClassInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<FareFocusFareClassInfo*>*
FareFocusFareClassHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<FareFocusFareClassInfo>(compressed);
}

std::string
FareFocusFareClassHistoricalDAO::_name("FareFocusFareClassHistorical");
std::string
FareFocusFareClassHistoricalDAO::_cacheClass("Rules");
DAOHelper<FareFocusFareClassHistoricalDAO>
FareFocusFareClassHistoricalDAO::_helper(_name);
FareFocusFareClassHistoricalDAO*
FareFocusFareClassHistoricalDAO::_instance(nullptr);

} // namespace tse
