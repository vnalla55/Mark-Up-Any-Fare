//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/TaxCodeDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetTaxCode.h"
#include "DBAccess/TaxCodeReg.h"


#include <algorithm>
#include <functional>

namespace tse
{
Logger
TaxCodeDAO::_logger("atseintl.DBAccess.TaxCodeDAO");

TaxCodeDAO&
TaxCodeDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<TaxCodeReg*>&
getTaxCodeData(const TaxCode& taxCode,
               const DateTime& date,
               DeleteList& deleteList,
               const DateTime& ticketDate,
               bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    TaxCodeHistoricalDAO& dao = TaxCodeHistoricalDAO::instance();
    return dao.get(deleteList, taxCode, date, ticketDate);
  }
  else
  {
    TaxCodeDAO& dao = TaxCodeDAO::instance();
    return dao.get(deleteList, taxCode, date, ticketDate);
  }
}

const std::vector<TaxCodeReg*>&
TaxCodeDAO::get(DeleteList& del,
                const TaxCode& key,
                const DateTime& date,
                const DateTime& ticketDate)
{
  DAOCache::pointer_type ptr = cache().get(TaxCodeKey(key));
  /*
  del.copy(ptr);
  std::vector<TaxCodeReg*>* ret = new std::vector<TaxCodeReg*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), IsNotEffectiveG<TaxCodeReg>(date,
  ticketDate));
  return *ret;
  */
  return *(applyFilter(del, ptr, IsNotEffectiveG<TaxCodeReg>(date, ticketDate)));
}

void
TaxCodeDAO::load()
{
  StartupLoader<QueryGetAllTaxCodeRegs, TaxCodeReg, TaxCodeDAO>();
}

TaxCodeKey
TaxCodeDAO::createKey(TaxCodeReg* info)
{
  return TaxCodeKey(info->taxCode());
}

std::vector<TaxCodeReg*>*
TaxCodeDAO::create(TaxCodeKey key)
{
  std::vector<TaxCodeReg*>* ret = new std::vector<TaxCodeReg*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTaxCode tc(dbAdapter->getAdapter());
    tc.findTaxCodeReg(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxCodeDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

sfc::CompressedData*
TaxCodeDAO::compress(const std::vector<TaxCodeReg*>* vect) const
{
  return compressVector(vect);
}

std::vector<TaxCodeReg*>*
TaxCodeDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<TaxCodeReg>(compressed);
}

void
TaxCodeDAO::destroy(TaxCodeKey key, std::vector<TaxCodeReg*>* recs)
{
  destroyContainer(recs);
}

std::string
TaxCodeDAO::_name("TaxCode");
std::string
TaxCodeDAO::_cacheClass("Taxes");

DAOHelper<TaxCodeDAO>
TaxCodeDAO::_helper(_name);

TaxCodeDAO* TaxCodeDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: TaxCodeHistoricalDAO
// --------------------------------------------------

Logger
TaxCodeHistoricalDAO::_logger("atseintl.DBAccess.TaxCodeHistoricalDAO");

TaxCodeHistoricalDAO&
TaxCodeHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<TaxCodeReg*>&
TaxCodeHistoricalDAO::get(DeleteList& del,
                          const TaxCode& key,
                          const DateTime& date,
                          const DateTime& ticketDate)
{
  TaxCodeHistoricalKey cacheKey(key);
  DAOUtils::getDateRange(ticketDate, cacheKey._b, cacheKey._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(cacheKey);
  del.copy(ptr);
  std::vector<TaxCodeReg*>* ret = new std::vector<TaxCodeReg*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveHist<TaxCodeReg>(date, ticketDate));

  for (TaxCodeReg* taxCodeReg : *ret)
  {
    std::vector<TaxCodeGenText*> texts;
    remove_copy_if(taxCodeReg->taxCodeGenTexts().begin(),
                   taxCodeReg->taxCodeGenTexts().end(),
                   back_inserter(texts),
                   IsNotEffectiveHist<TaxCodeGenText>(date, ticketDate));
    taxCodeReg->taxCodeGenTexts() = texts;
  }
  return *ret;
}

std::vector<TaxCodeReg*>*
TaxCodeHistoricalDAO::create(TaxCodeHistoricalKey key)
{
  std::vector<TaxCodeReg*>* ret = new std::vector<TaxCodeReg*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTaxCodeHistorical tch(dbAdapter->getAdapter());
    tch.findTaxCodeRegHistorical(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxCodeHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TaxCodeHistoricalDAO::destroy(const TaxCodeHistoricalKey key, std::vector<TaxCodeReg*>* recs)
{
  std::vector<TaxCodeReg*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}
sfc::CompressedData*
TaxCodeHistoricalDAO::compress(const std::vector<TaxCodeReg*>* vect) const
{
  return compressVector(vect);
}

std::vector<TaxCodeReg*>*
TaxCodeHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<TaxCodeReg>(compressed);
}

std::string
TaxCodeHistoricalDAO::_name("TaxCodeHistorical");
std::string
TaxCodeHistoricalDAO::_cacheClass("Taxes");

DAOHelper<TaxCodeHistoricalDAO>
TaxCodeHistoricalDAO::_helper(_name);

TaxCodeHistoricalDAO* TaxCodeHistoricalDAO::_instance = nullptr;

} // namespace tse
