//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/MarkupControlDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/MarkupControl.h"
#include "DBAccess/Queries/QueryGetMarkup.h"

namespace tse
{
log4cxx::LoggerPtr
MarkupControlDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.MarkupControlDAO"));

MarkupControlDAO&
MarkupControlDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

struct MarkupControlDAO::CheckPCC
{
public:
  CheckPCC(const PseudoCityCode& pcc,
           const PseudoCityCode& homePCC,
           long secondarySellerId,
           const DateTime& ticketDate)
    : _pcc(pcc),
      _homePCC(homePCC),
      _secondarySellerId(secondarySellerId),
      _isCurrent(ticketDate),
      _markupType('U')
  {
  }

  CheckPCC(const PseudoCityCode& pcc, const PseudoCityCode& homePCC, const DateTime& ticketDate)
    : _pcc(pcc), _homePCC(homePCC), _secondarySellerId(0), _isCurrent(ticketDate), _markupType('R')
  {
  }

  bool operator()(const MarkupControl* rec)
  {
    if (!_isCurrent(rec))
      return true;
    if (rec->markupType() != _markupType)
      return true;
    if (!((rec->ownerPseudoCityType() == 'T' && rec->ownerPseudoCity() == _pcc) ||
          (rec->ownerPseudoCityType() == 'U' && rec->ownerPseudoCity() == _pcc) ||
          (rec->ownerPseudoCityType() == 'U' && rec->ownerPseudoCity() == _homePCC)))
      return true;
    if (_markupType == 'U')
      if (rec->secondarySellerId() != _secondarySellerId)
        return true;
    return false;
  }

private:
  const PseudoCityCode& _pcc;
  const PseudoCityCode& _homePCC;
  long _secondarySellerId;
  IsCurrentG<MarkupControl> _isCurrent;
  const Indicator _markupType;
};

const std::vector<MarkupControl*>&
getMarkupBySecondSellerIdData(const PseudoCityCode& pcc,
                              const PseudoCityCode& homePCC,
                              const VendorCode& vendor,
                              const CarrierCode& carrier,
                              const TariffNumber& ruleTariff,
                              const RuleNumber& rule,
                              int seqNo,
                              long secondarySellerId,
                              const DateTime& date,
                              DeleteList& deleteList,
                              const DateTime& ticketDate,
                              bool isHistorical)
{
  if (isHistorical)
  {
    MarkupControlHistoricalDAO& dao = MarkupControlHistoricalDAO::instance();
    return dao.get(deleteList,
                   pcc,
                   homePCC,
                   vendor,
                   carrier,
                   ruleTariff,
                   rule,
                   seqNo,
                   secondarySellerId,
                   ticketDate);
  }
  else
  {
    MarkupControlDAO& dao = MarkupControlDAO::instance();
    return dao.get(deleteList,
                   pcc,
                   homePCC,
                   vendor,
                   carrier,
                   ruleTariff,
                   rule,
                   seqNo,
                   secondarySellerId,
                   ticketDate);
  }
}

size_t
MarkupControlDAO::invalidate(const ObjectKey& objectKey)
{
  size_t result(0);
  std::string vendor, carrier, ruleTariff, rule, seqNo, pcc;
  objectKey.getValue("VENDOR", vendor);
  objectKey.getValue("CARRIER", carrier);
  objectKey.getValue("RULETARIFF", ruleTariff);
  objectKey.getValue("RULE", rule);
  objectKey.getValue("SEQNO", seqNo);
  objectKey.getValue("OWNERPSEUDOCITY", pcc);
  if (ruleTariff == "?" || rule == "?" || seqNo == "?" || pcc == "?")
  {
    std::shared_ptr<std::vector<MarkupControlKey>> cacheKeys = cache().keys();
    int ruleTariffValue = -1;
    if (ruleTariff != "?")
      ruleTariffValue = atoi(ruleTariff.c_str());
    int seqNoValue = -1;
    if (seqNo != "?")
      seqNoValue = atoi(seqNo.c_str());
    for (size_t i = 0; i < cacheKeys->size(); ++i)
    {
      const MarkupControlKey& cacheKey = (*cacheKeys)[i];
      if ((vendor == cacheKey._a) && (carrier == cacheKey._b) &&
          ((ruleTariffValue == cacheKey._c) || (ruleTariff == "?")) &&
          ((rule == cacheKey._d) || (rule == "?")) &&
          ((seqNoValue == cacheKey._e) || (seqNo == "?")) && ((pcc == cacheKey._f) || (pcc == "?")))
      {
        if (_loadOnUpdate)
        {
          cache().put(cacheKey, create(cacheKey));
          result += 1;
        }
        else
        {
          result += cache().invalidate(cacheKey);
        }
      }
    }
  }
  else
  {
    return DataAccessObjectBase<MarkupControlKey, std::vector<MarkupControl*> >::invalidate(objectKey);
  }
  return result;
}

bool
MarkupControlDAO::translateKey(const ObjectKey& objectKey, MarkupControlKey& key) const
{
  objectKey.getValue("OWNERPSEUDOCITY", key._f);

  return key.initialized =
             objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
             objectKey.getValue("RULETARIFF", key._c) && objectKey.getValue("RULE", key._d) &&
             objectKey.getValue("SEQNO", key._e);
}

const std::vector<MarkupControl*>&
MarkupControlDAO::get(DeleteList& del,
                      const PseudoCityCode& pcc,
                      const PseudoCityCode& homePCC,
                      const VendorCode& vendor,
                      const CarrierCode& carrier,
                      const TariffNumber& ruleTariff,
                      const RuleNumber& rule,
                      int seqNo,
                      long secondarySellerId,
                      const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  MarkupControlKey key(vendor, carrier, ruleTariff, rule, seqNo, pcc);
  DAOCache::pointer_type ptr = cache().get(key);

  if (pcc == homePCC)
    return *(applyFilter(del, ptr, CheckPCC(pcc, homePCC, secondarySellerId, ticketDate)));

  MarkupControlKey key1(vendor, carrier, ruleTariff, rule, seqNo, homePCC);
  DAOCache::pointer_type ptr1 = cache().get(key1);
  if (ptr1->empty())
    return *(applyFilter(del, ptr, CheckPCC(pcc, homePCC, secondarySellerId, ticketDate)));

  del.copy(ptr);

  std::vector<MarkupControl*>* ret = new std::vector<MarkupControl*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 CheckPCC(pcc, homePCC, secondarySellerId, ticketDate));
  remove_copy_if(ptr1->begin(),
                 ptr1->end(),
                 back_inserter(*ret),
                 CheckPCC(pcc, homePCC, secondarySellerId, ticketDate));
  return *ret;
}

const std::vector<MarkupControl*>&
getMarkupBySecurityItemNoData(const PseudoCityCode& pcc,
                              const PseudoCityCode& homePCC,
                              const VendorCode& vendor,
                              const CarrierCode& carrier,
                              const TariffNumber& ruleTariff,
                              const RuleNumber& rule,
                              int seqNo,
                              const DateTime& date,
                              DeleteList& deleteList,
                              const DateTime& ticketDate,
                              bool isHistorical)
{
  if (isHistorical)
  {
    MarkupControlHistoricalDAO& dao = MarkupControlHistoricalDAO::instance();
    return dao.get(deleteList, pcc, homePCC, vendor, carrier, ruleTariff, rule, seqNo, ticketDate);
  }
  else
  {
    MarkupControlDAO& dao = MarkupControlDAO::instance();
    return dao.get(deleteList, pcc, homePCC, vendor, carrier, ruleTariff, rule, seqNo, ticketDate);
  }
}

const std::vector<MarkupControl*>&
MarkupControlDAO::get(DeleteList& del,
                      const PseudoCityCode& pcc,
                      const PseudoCityCode& homePCC,
                      const VendorCode& vendor,
                      const CarrierCode& carrier,
                      const TariffNumber& ruleTariff,
                      const RuleNumber& rule,
                      int seqNo,
                      const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  MarkupControlKey key(vendor, carrier, ruleTariff, rule, seqNo, pcc);
  DAOCache::pointer_type ptr = cache().get(key);
  if (pcc == homePCC)
    return *(applyFilter(del, ptr, CheckPCC(pcc, homePCC, ticketDate)));

  MarkupControlKey key1(vendor, carrier, ruleTariff, rule, seqNo, homePCC);
  DAOCache::pointer_type ptr1 = cache().get(key1);
  if (ptr1->empty())
    return *(applyFilter(del, ptr, CheckPCC(pcc, homePCC, ticketDate)));

  del.copy(ptr);

  std::vector<MarkupControl*>* ret = new std::vector<MarkupControl*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), CheckPCC(pcc, homePCC, ticketDate));
  remove_copy_if(
      ptr1->begin(), ptr1->end(), back_inserter(*ret), CheckPCC(pcc, homePCC, ticketDate));
  return *ret;
}

const std::vector<MarkupControl*>&
getMarkupByPccData(const PseudoCityCode& pcc,
                   const VendorCode& vendor,
                   const CarrierCode& carrier,
                   const TariffNumber& ruleTariff,
                   const RuleNumber& rule,
                   int seqNo,
                   const DateTime& date,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    MarkupControlHistoricalDAO& dao = MarkupControlHistoricalDAO::instance();
    return dao.get(deleteList, pcc, vendor, carrier, ruleTariff, rule, seqNo, ticketDate);
  }
  else
  {
    MarkupControlDAO& dao = MarkupControlDAO::instance();
    return dao.get(deleteList, pcc, vendor, carrier, ruleTariff, rule, seqNo, ticketDate);
  }
}

const std::vector<MarkupControl*>&
MarkupControlDAO::get(DeleteList& del,
                      const PseudoCityCode& pcc,
                      const VendorCode& vendor,
                      const CarrierCode& carrier,
                      const TariffNumber& ruleTariff,
                      const RuleNumber& rule,
                      int seqNo,
                      const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  MarkupControlKey key(vendor, carrier, ruleTariff, rule, seqNo, pcc);
  DAOCache::pointer_type ptr = cache().get(key);
  return *(applyFilter(del, ptr, IsNotCurrentG<MarkupControl>(ticketDate)));
}

std::vector<MarkupControl*>*
MarkupControlDAO::create(MarkupControlKey key)
{
  std::vector<MarkupControl*>* ret = new std::vector<MarkupControl*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMarkup mkupnew(dbAdapter->getAdapter());
    mkupnew.findMarkupControl(*ret, key._a, key._b, key._c, key._d, key._e, key._f);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MarkupControlDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MarkupControlDAO::destroy(MarkupControlKey key, std::vector<MarkupControl*>* recs)
{
  std::vector<MarkupControl*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

MarkupControlKey
MarkupControlDAO::createKey(MarkupControl* info)
{
  return MarkupControlKey(info->vendor(),
                          info->carrier(),
                          info->ruleTariff(),
                          info->rule(),
                          info->seqNo(),
                          info->ownerPseudoCity());
}

void
MarkupControlDAO::load()
{
  StartupLoaderNoDB<MarkupControl, MarkupControlDAO>();
}

sfc::CompressedData*
MarkupControlDAO::compress(const std::vector<MarkupControl*>* vect) const
{
  return compressVector(vect);
}

std::vector<MarkupControl*>*
MarkupControlDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MarkupControl>(compressed);
}

std::string
MarkupControlDAO::_name("MarkupControl");
std::string
MarkupControlDAO::_cacheClass("Rules");
DAOHelper<MarkupControlDAO>
MarkupControlDAO::_helper(_name);
MarkupControlDAO* MarkupControlDAO::_instance = nullptr;

//////////////////////////////////////////////////////
// Historical DAO Object
//////////////////////////////////////////////////////
log4cxx::LoggerPtr
MarkupControlHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.MarkupControlHistoricalDAO"));
MarkupControlHistoricalDAO&
MarkupControlHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

struct MarkupControlHistoricalDAO::CheckPCC
{
public:
  CheckPCC(const PseudoCityCode& pcc,
           const PseudoCityCode& homePCC,
           long secondarySellerId,
           const DateTime& ticketDate)
    : _pcc(pcc),
      _homePCC(homePCC),
      _secondarySellerId(secondarySellerId),
      _isCurrent(ticketDate),
      _markupType('U')
  {
  }

  CheckPCC(const PseudoCityCode& pcc, const PseudoCityCode& homePCC, const DateTime& ticketDate)
    : _pcc(pcc), _homePCC(homePCC), _secondarySellerId(0), _isCurrent(ticketDate), _markupType('R')
  {
  }

  bool operator()(const MarkupControl* rec)
  {
    if (!_isCurrent(rec))
      return true;
    if (rec->markupType() != _markupType)
      return true;
    if (!((rec->ownerPseudoCityType() == 'T' && rec->ownerPseudoCity() == _pcc) ||
          (rec->ownerPseudoCityType() == 'U' && rec->ownerPseudoCity() == _pcc) ||
          (rec->ownerPseudoCityType() == 'U' && rec->ownerPseudoCity() == _homePCC)))
      return true;
    if (_markupType == 'U')
      if (rec->secondarySellerId() != _secondarySellerId)
        return true;
    return false;
  }

private:
  const PseudoCityCode& _pcc;
  const PseudoCityCode& _homePCC;
  long _secondarySellerId;
  IsCurrentH<MarkupControl> _isCurrent;
  const Indicator _markupType;
};

const std::vector<MarkupControl*>&
MarkupControlHistoricalDAO::get(DeleteList& del,
                                const PseudoCityCode& pcc,
                                const PseudoCityCode& homePCC,
                                const VendorCode& vendor,
                                const CarrierCode& carrier,
                                const TariffNumber& ruleTariff,
                                const RuleNumber& rule,
                                int seqNo,
                                long secondarySellerId,
                                const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  MarkupControlHistoricalKey key(vendor, carrier, ruleTariff, rule, seqNo, pcc);
  DAOUtils::getDateRange(ticketDate, key._g, key._h, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  std::vector<MarkupControl*>* ret = new std::vector<MarkupControl*>;
  del.copy(ptr);
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 CheckPCC(pcc, homePCC, secondarySellerId, ticketDate));
  if (pcc == homePCC)
    return *ret;

  MarkupControlHistoricalKey key1(vendor, carrier, ruleTariff, rule, seqNo, homePCC);
  DAOUtils::getDateRange(ticketDate, key1._g, key1._h, _cacheBy);
  DAOCache::pointer_type ptr1 = cache().get(key1);
  if (ptr1->empty())
    return *ret;

  remove_copy_if(ptr1->begin(),
                 ptr1->end(),
                 back_inserter(*ret),
                 CheckPCC(pcc, homePCC, secondarySellerId, ticketDate));
  return *ret;
}

size_t
MarkupControlHistoricalDAO::invalidate(const ObjectKey& objectKey)
{
  size_t result(0);
  std::string vendor, carrier, ruleTariff, rule, seqNo, pcc;
  objectKey.getValue("VENDOR", vendor);
  objectKey.getValue("CARRIER", carrier);
  objectKey.getValue("RULETARIFF", ruleTariff);
  objectKey.getValue("RULE", rule);
  objectKey.getValue("SEQNO", seqNo);
  objectKey.getValue("OWNERPSEUDOCITY", pcc);
  if (ruleTariff == "?" || rule == "?" || seqNo == "?" || pcc == "?")
  {
    std::shared_ptr<std::vector<MarkupControlHistoricalKey>> cacheKeys = cache().keys();
    int ruleTariffValue = -1;
    if (ruleTariff != "?")
      ruleTariffValue = atoi(ruleTariff.c_str());
    int seqNoValue = -1;
    if (seqNo != "?")
      seqNoValue = atoi(seqNo.c_str());
    for (size_t i = 0; i < cacheKeys->size(); ++i)
    {
      const MarkupControlHistoricalKey& cacheKey = (*cacheKeys)[i];
      if ((vendor == cacheKey._a) && (carrier == cacheKey._b) &&
          ((ruleTariffValue == cacheKey._c) || (ruleTariff == "?")) &&
          ((rule == cacheKey._d) || (rule == "?")) &&
          ((seqNoValue == cacheKey._e) || (seqNo == "?")) && ((pcc == cacheKey._f) || (pcc == "?")))
      {
        if (_loadOnUpdate)
        {
          cache().put(cacheKey, create(cacheKey));
          result += 1;
        }
        else
        {
          result += cache().invalidate(cacheKey);
        }
      }
    }
  }
  else
  {
    return HistoricalDataAccessObject<MarkupControlHistoricalKey,
                               std::vector<MarkupControl*> >::invalidate(objectKey);
  }
  return result;
}

bool
MarkupControlHistoricalDAO::translateKey(const ObjectKey& objectKey,
                                         MarkupControlHistoricalKey& key) const
{
  objectKey.getValue("OWNERPSEUDOCITY", key._f);

  return key.initialized =
             objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
             objectKey.getValue("RULETARIFF", key._c) && objectKey.getValue("RULE", key._d) &&
             objectKey.getValue("SEQNO", key._e) && objectKey.getValue("STARTDATE", key._g) &&
             objectKey.getValue("ENDDATE", key._h);
}

bool
MarkupControlHistoricalDAO::translateKey(const ObjectKey& objectKey,
                                         MarkupControlHistoricalKey& key,
                                         const DateTime ticketDate) const
{
  DAOUtils::getDateRange(ticketDate, key._g, key._h, _cacheBy);

  objectKey.getValue("OWNERPSEUDOCITY", key._f);

  return key.initialized =
             objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
             objectKey.getValue("RULETARIFF", key._c) && objectKey.getValue("RULE", key._d) &&
             objectKey.getValue("SEQNO", key._e);
}

const std::vector<MarkupControl*>&
MarkupControlHistoricalDAO::get(DeleteList& del,
                                const PseudoCityCode& pcc,
                                const PseudoCityCode& homePCC,
                                const VendorCode& vendor,
                                const CarrierCode& carrier,
                                const TariffNumber& ruleTariff,
                                const RuleNumber& rule,
                                int seqNo,
                                const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  MarkupControlHistoricalKey key(vendor, carrier, ruleTariff, rule, seqNo, pcc);
  DAOUtils::getDateRange(ticketDate, key._g, key._h, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  std::vector<MarkupControl*>* ret = new std::vector<MarkupControl*>;
  del.copy(ptr);
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), CheckPCC(pcc, homePCC, ticketDate));
  if (pcc == homePCC)
    return *ret;

  MarkupControlHistoricalKey key1(vendor, carrier, ruleTariff, rule, seqNo, homePCC);
  DAOUtils::getDateRange(ticketDate, key1._g, key1._h, _cacheBy);
  DAOCache::pointer_type ptr1 = cache().get(key1);
  if (ptr1->empty())
    return *ret;

  remove_copy_if(
      ptr1->begin(), ptr1->end(), back_inserter(*ret), CheckPCC(pcc, homePCC, ticketDate));
  return *ret;
}

const std::vector<MarkupControl*>&
MarkupControlHistoricalDAO::get(DeleteList& del,
                                const PseudoCityCode& pcc,
                                const VendorCode& vendor,
                                const CarrierCode& carrier,
                                const TariffNumber& ruleTariff,
                                const RuleNumber& rule,
                                int seqNo,
                                const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  MarkupControlHistoricalKey key(vendor, carrier, ruleTariff, rule, seqNo, pcc);
  DAOUtils::getDateRange(ticketDate, key._g, key._h, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  std::vector<MarkupControl*>* ret = new std::vector<MarkupControl*>;
  del.copy(ptr);
  del.adopt(ret);
  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotCurrentH<MarkupControl>(ticketDate));
  return *ret;
}

std::vector<MarkupControl*>*
MarkupControlHistoricalDAO::create(MarkupControlHistoricalKey key)
{
  std::vector<MarkupControl*>* ret = new std::vector<MarkupControl*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetMarkupHistorical mkpnew(dbAdapter->getAdapter());
    mkpnew.findMarkupControl(*ret, key._a, key._b, key._c, key._d, key._e, key._f, key._g, key._h);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MarkupControlHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

MarkupControlHistoricalKey
MarkupControlHistoricalDAO::createKey(MarkupControl* info,
                                      const DateTime& startDate,
                                      const DateTime& endDate)
{
  return MarkupControlHistoricalKey(info->vendor(),
                                    info->carrier(),
                                    info->ruleTariff(),
                                    info->rule(),
                                    info->seqNo(),
                                    info->ownerPseudoCity(),
                                    startDate,
                                    endDate);
}

void
MarkupControlHistoricalDAO::load()
{
  StartupLoaderNoDB<MarkupControl, MarkupControlHistoricalDAO>();
}

void
MarkupControlHistoricalDAO::destroy(MarkupControlHistoricalKey key,
                                    std::vector<MarkupControl*>* recs)
{
  std::vector<MarkupControl*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
MarkupControlHistoricalDAO::compress(const std::vector<MarkupControl*>* vect) const
{
  return compressVector(vect);
}

std::vector<MarkupControl*>*
MarkupControlHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MarkupControl>(compressed);
}

std::string
MarkupControlHistoricalDAO::_name("MarkupControlHistorical");
std::string
MarkupControlHistoricalDAO::_cacheClass("Rules");
DAOHelper<MarkupControlHistoricalDAO>
MarkupControlHistoricalDAO::_helper(_name);
MarkupControlHistoricalDAO* MarkupControlHistoricalDAO::_instance = nullptr;

} // namespace tse
