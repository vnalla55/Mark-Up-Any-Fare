//-------------------------------------------------------------------------------
// @ 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/NegFareRestExtSeqDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/NegFareRestExtSeq.h"
#include "DBAccess/Queries/QueryGetNegFareRestExtSeq.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

namespace tse
{
log4cxx::LoggerPtr
NegFareRestExtSeqDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.NegFareRestExtSeqDAO"));
NegFareRestExtSeqDAO&
NegFareRestExtSeqDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<NegFareRestExtSeq*>&
getNegFareRestExtSeqData(const VendorCode& vendor,
                         int itemNo,
                         DeleteList& deleteList,
                         const DateTime& ticketDate,
                         bool isHistorical)
{
  if (isHistorical)
  {
    NegFareRestExtSeqHistoricalDAO& dao = NegFareRestExtSeqHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    NegFareRestExtSeqDAO& dao = NegFareRestExtSeqDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

std::vector<NegFareRestExtSeq*>&
NegFareRestExtSeqDAO::get(DeleteList& del,
                          const VendorCode& vendor,
                          int itemNo,
                          const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  NegFareRestExtSeqKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<NegFareRestExtSeq*>* ret = new std::vector<NegFareRestExtSeq*>;
  del.adopt(ret);

  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotCurrentG<NegFareRestExtSeq>(ticketDate));
  return *ret;
}

std::vector<NegFareRestExtSeq*>*
NegFareRestExtSeqDAO::create(NegFareRestExtSeqKey key)
{
  std::vector<NegFareRestExtSeq*>* ret = new std::vector<NegFareRestExtSeq*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetNegFareRestExtSeq query(dbAdapter->getAdapter());
    query.findNegFareRestExtSeq(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in NegFareRestExtSeqDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
NegFareRestExtSeqDAO::destroy(NegFareRestExtSeqKey key, std::vector<NegFareRestExtSeq*>* recs)
{
  std::vector<NegFareRestExtSeq*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
NegFareRestExtSeqDAO::compress(const std::vector<NegFareRestExtSeq*>* vect) const
{
  return compressVector(vect);
}

std::vector<NegFareRestExtSeq*>*
NegFareRestExtSeqDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<NegFareRestExtSeq>(compressed);
}

std::string
NegFareRestExtSeqDAO::_name("NegFareRestExtSeq");
std::string
NegFareRestExtSeqDAO::_cacheClass("Rules");
DAOHelper<NegFareRestExtSeqDAO>
NegFareRestExtSeqDAO::_helper(_name);
NegFareRestExtSeqDAO* NegFareRestExtSeqDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: NegFareRestExtSeqHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
NegFareRestExtSeqHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.NegFareRestExtSeqHistoricalDAO"));
NegFareRestExtSeqHistoricalDAO&
NegFareRestExtSeqHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

std::vector<NegFareRestExtSeq*>&
NegFareRestExtSeqHistoricalDAO::get(DeleteList& del,
                                    const VendorCode& vendor,
                                    int itemNo,
                                    const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  NegFareRestExtSeqHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<NegFareRestExtSeq*>* ret = new std::vector<NegFareRestExtSeq*>;
  del.adopt(ret);

  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotCurrentH<NegFareRestExtSeq>(ticketDate));

  return *ret;
}

std::vector<NegFareRestExtSeq*>*
NegFareRestExtSeqHistoricalDAO::create(NegFareRestExtSeqHistoricalKey key)
{
  std::vector<NegFareRestExtSeq*>* ret = new std::vector<NegFareRestExtSeq*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetNegFareRestExtSeqHistorical query(dbAdapter->getAdapter());
    query.findNegFareRestExtSeq(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in NegFareRestExtSeqHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
NegFareRestExtSeqHistoricalDAO::destroy(NegFareRestExtSeqHistoricalKey key,
                                        std::vector<NegFareRestExtSeq*>* recs)
{
  std::vector<NegFareRestExtSeq*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
NegFareRestExtSeqHistoricalDAO::compress(const std::vector<NegFareRestExtSeq*>* vect) const
{
  return compressVector(vect);
}

std::vector<NegFareRestExtSeq*>*
NegFareRestExtSeqHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<NegFareRestExtSeq>(compressed);
}

std::string
NegFareRestExtSeqHistoricalDAO::_name("NegFareRestExtSeqHistorical");
std::string
NegFareRestExtSeqHistoricalDAO::_cacheClass("Rules");
DAOHelper<NegFareRestExtSeqHistoricalDAO>
NegFareRestExtSeqHistoricalDAO::_helper(_name);
NegFareRestExtSeqHistoricalDAO* NegFareRestExtSeqHistoricalDAO::_instance = nullptr;

} // namespace tse
