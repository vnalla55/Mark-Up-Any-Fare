//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/FootNoteCtrlDAO.h"

#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DAOUtils.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DBHistoryServer.h"
#include "DBAccess/FootNoteCtrlInfo.h"
#include "DBAccess/Queries/QueryGetNonCombCatCtrl.h"

namespace tse
{

log4cxx::LoggerPtr
FootNoteCtrlDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FootNoteCtrlDAO"));

namespace
{
struct InhibitStruct
{
  bool operator()(const FootNoteCtrlInfo* rec) const { return Inhibit(rec); }
};

struct InhibitForFDStruct
{
  bool operator()(const FootNoteCtrlInfo* rec) const { return InhibitForFD(rec); }
};

template <typename T, typename U>
struct OrFilter
{
  OrFilter(const T& filter1, const U& filter2) : _filter1(filter1), _filter2(filter2) {}

  bool operator()(const FootNoteCtrlInfo* rec) { return _filter1(rec) || _filter2(rec); }

  const T _filter1;
  const U _filter2;
};
}

FootNoteCtrlDAO&
FootNoteCtrlDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FootNoteCtrlInfo*>&
getFootNoteCtrlData(const VendorCode vendor,
                    const CarrierCode carrier,
                    const TariffNumber tariffNumber,
                    const Footnote footnote,
                    int category)
{
  // currently only for non historical, can be extended when optimizing historical applications
  return FootNoteCtrlDAO::instance().getAll(vendor, carrier, tariffNumber, footnote, category);
}

const std::vector<FootNoteCtrlInfo*>&
getFootNoteCtrlData(const VendorCode& vendor,
                    const CarrierCode& carrier,
                    const TariffNumber& fareTariff,
                    const Footnote& footnote,
                    const int category,
                    const DateTime& date,
                    DeleteList& deleteList,
                    const DateTime& ticketDate,
                    bool isHistorical,
                    bool isFareDisplay)
{
  if (UNLIKELY(isHistorical))
  {
    FootNoteCtrlHistoricalDAO& dao = FootNoteCtrlHistoricalDAO::instance();

    const std::vector<FootNoteCtrlInfo*>& ret =
        isFareDisplay
            ? dao.getForFD(
                  deleteList, vendor, carrier, fareTariff, footnote, category, date, ticketDate)
            : dao.get(
                  deleteList, vendor, carrier, fareTariff, footnote, category, date, ticketDate);

    return ret;
  }
  else
  {
    FootNoteCtrlDAO& dao = FootNoteCtrlDAO::instance();

    const std::vector<FootNoteCtrlInfo*>& ret =
        isFareDisplay
            ? dao.getForFD(
                  deleteList, vendor, carrier, fareTariff, footnote, category, date, ticketDate)
            : dao.get(
                  deleteList, vendor, carrier, fareTariff, footnote, category, date, ticketDate);

    return ret;
  }
}

const std::vector<FootNoteCtrlInfo*>&
FootNoteCtrlDAO::get(DeleteList& del,
                     const VendorCode& vendor,
                     const CarrierCode& carrier,
                     const TariffNumber& ruleTariff,
                     const Footnote& footnote,
                     int category,
                     const DateTime& date,
                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  FootNoteCtrlKey key(vendor, carrier, ruleTariff, footnote, category);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<FootNoteCtrlInfo*>* ret(new std::vector<FootNoteCtrlInfo*>);
  del.adopt(ret);

  OrFilter<IsNotEffectiveG<FootNoteCtrlInfo>, InhibitStruct> filter(
      IsNotEffectiveG<FootNoteCtrlInfo>(date, ticketDate), InhibitStruct());

  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), filter);

  return *ret;
}

const std::vector<FootNoteCtrlInfo*>&
FootNoteCtrlDAO::getForFD(DeleteList& del,
                          const VendorCode& vendor,
                          const CarrierCode& carrier,
                          const TariffNumber& ruleTariff,
                          const Footnote& footnote,
                          int category,
                          const DateTime& date,
                          const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetFDCallCount;

  FootNoteCtrlKey key(vendor, carrier, ruleTariff, footnote, category);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<FootNoteCtrlInfo*>* ret(new std::vector<FootNoteCtrlInfo*>);
  del.adopt(ret);

  OrFilter<IsNotEffectiveG<FootNoteCtrlInfo>, InhibitForFDStruct> filter(
      IsNotEffectiveG<FootNoteCtrlInfo>(date, ticketDate), InhibitForFDStruct());

  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), filter);

  return *ret;
}

std::vector<FootNoteCtrlInfo*>*
FootNoteCtrlDAO::create(FootNoteCtrlKey key)
{
  std::vector<FootNoteCtrlInfo*>* ret = new std::vector<FootNoteCtrlInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFootNoteCtrl fnc(dbAdapter->getAdapter());
    fnc.findFootNoteCtrlInfo(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FootNoteCtrlDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FootNoteCtrlDAO::destroy(FootNoteCtrlKey key, std::vector<FootNoteCtrlInfo*>* recs)
{
  destroyContainer(recs);
}

FootNoteCtrlKey
FootNoteCtrlDAO::createKey(FootNoteCtrlInfo* info)
{
  return FootNoteCtrlKey(info->vendorCode(),
                         info->carrierCode(),
                         info->fareTariff(),
                         info->footNote(),
                         info->categoryNumber());
}

void
FootNoteCtrlDAO::load()
{
  StartupLoader<QueryGetAllFootNoteCtrl, FootNoteCtrlInfo, FootNoteCtrlDAO>();
}

sfc::CompressedData*
FootNoteCtrlDAO::compress(const std::vector<FootNoteCtrlInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<FootNoteCtrlInfo*>*
FootNoteCtrlDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<FootNoteCtrlInfo>(compressed);
}

std::string
FootNoteCtrlDAO::_name("FootNoteCtrl");
std::string
FootNoteCtrlDAO::_cacheClass("Rules");
DAOHelper<FootNoteCtrlDAO>
FootNoteCtrlDAO::_helper(_name);
FootNoteCtrlDAO* FootNoteCtrlDAO::_instance = nullptr;

///////////////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////////////

log4cxx::LoggerPtr
FootNoteCtrlHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FootNoteCtrlHistDAO"));

FootNoteCtrlHistoricalDAO&
FootNoteCtrlHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FootNoteCtrlInfo*>&
FootNoteCtrlHistoricalDAO::get(DeleteList& del,
                               const VendorCode& vendor,
                               const CarrierCode& carrier,
                               const TariffNumber& ruleTariff,
                               const Footnote& footnote,
                               const int category,
                               const DateTime& date,
                               const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  FootNoteCtrlHistoricalKey key(vendor, carrier, ruleTariff, footnote, category);
  DAOUtils::getDateRange(ticketDate, key._f, key._g, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<FootNoteCtrlInfo*>* ret(new std::vector<FootNoteCtrlInfo*>);
  del.adopt(ret);

  OrFilter<IsNotEffectiveH<FootNoteCtrlInfo>, InhibitStruct> filter(
      IsNotEffectiveH<FootNoteCtrlInfo>(date, ticketDate), InhibitStruct());

  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), filter);

  return *ret;
}

const std::vector<FootNoteCtrlInfo*>&
FootNoteCtrlHistoricalDAO::getForFD(DeleteList& del,
                                    const VendorCode& vendor,
                                    const CarrierCode& carrier,
                                    const TariffNumber& ruleTariff,
                                    const Footnote& footnote,
                                    const int category,
                                    const DateTime& date,
                                    const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetFDCallCount;

  FootNoteCtrlHistoricalKey key(vendor, carrier, ruleTariff, footnote, category);
  DAOUtils::getDateRange(ticketDate, key._f, key._g, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<FootNoteCtrlInfo*>* ret(new std::vector<FootNoteCtrlInfo*>);
  del.adopt(ret);

  OrFilter<IsNotEffectiveH<FootNoteCtrlInfo>, InhibitForFDStruct> filter(
      IsNotEffectiveH<FootNoteCtrlInfo>(date, ticketDate), InhibitForFDStruct());

  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), filter);

  return *ret;
}

std::vector<FootNoteCtrlInfo*>*
FootNoteCtrlHistoricalDAO::create(FootNoteCtrlHistoricalKey key)
{
  std::vector<FootNoteCtrlInfo*>* ret = new std::vector<FootNoteCtrlInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetFootNoteCtrlHistorical fnc(dbAdapter->getAdapter());
    fnc.findFootNoteCtrlInfo(*ret, key._a, key._b, key._c, key._d, key._e, key._f, key._g);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FootNoteCtrlHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FootNoteCtrlHistoricalDAO::load()
{
  StartupLoaderNoDB<FootNoteCtrlInfo, FootNoteCtrlHistoricalDAO>();
}

FootNoteCtrlHistoricalKey
FootNoteCtrlHistoricalDAO::createKey(const FootNoteCtrlInfo* info,
                                     const DateTime& startDate,
                                     const DateTime& endDate)
{
  return FootNoteCtrlHistoricalKey(info->vendorCode(),
                                   info->carrierCode(),
                                   info->fareTariff(),
                                   info->footNote(),
                                   info->categoryNumber(),
                                   startDate,
                                   endDate);
}

void
FootNoteCtrlHistoricalDAO::destroy(FootNoteCtrlHistoricalKey key,
                                   std::vector<FootNoteCtrlInfo*>* recs)
{
  std::vector<FootNoteCtrlInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
FootNoteCtrlHistoricalDAO::compress(const std::vector<FootNoteCtrlInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<FootNoteCtrlInfo*>*
FootNoteCtrlHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<FootNoteCtrlInfo>(compressed);
}

std::string
FootNoteCtrlHistoricalDAO::_name("FootNoteCtrlHistorical");
std::string
FootNoteCtrlHistoricalDAO::_cacheClass("Rules");
DAOHelper<FootNoteCtrlHistoricalDAO>
FootNoteCtrlHistoricalDAO::_helper(_name);
FootNoteCtrlHistoricalDAO* FootNoteCtrlHistoricalDAO::_instance = nullptr;

} // namespace tse
