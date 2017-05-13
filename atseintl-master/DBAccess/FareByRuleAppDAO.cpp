//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/FareByRuleAppDAO.h"

#include "Common/Logger.h"
#include "DBAccess/CorpId.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/Queries/QueryGetFareByRuleApp.h"

namespace tse
{
log4cxx::LoggerPtr
FareByRuleAppDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FareByRuleAppDAO"));

struct FareByRuleAppDAO::FilterPaxType
{
public:
  FilterPaxType(const TktDesignator& tktDesignator,
                const std::vector<PaxTypeCode>& paxTypes,
                const DateTime& tvlDate,
                const DateTime& ticketDate,
                bool isFareDisplay)
    : _tktDesignator(tktDesignator),
      _paxTypes(paxTypes),
      _isNotEffective(tvlDate, ticketDate),
      _isFareDisplay(isFareDisplay)
  {
  }

  bool operator()(const FareByRuleApp* rec)
  {
    if (UNLIKELY(_isFareDisplay))
    {
      if (InhibitForFD<FareByRuleApp>(rec))
        return true;
    }
    else
    {
      if (Inhibit<FareByRuleApp>(rec))
        return true;
    }

    if (_isNotEffective(rec))
      return true;

    if (!rec->tktDesignator().empty() && rec->tktDesignator() != _tktDesignator)
      return true;

    if (std::find(_paxTypes.begin(), _paxTypes.end(), rec->primePaxType()) == _paxTypes.end() &&
        std::find_first_of(_paxTypes.begin(),
                           _paxTypes.end(),
                           rec->secondaryPaxTypes().begin(),
                           rec->secondaryPaxTypes().end()) == _paxTypes.end())
      return true;

    return false;
  }

private:
  const TktDesignator& _tktDesignator;
  const std::vector<PaxTypeCode>& _paxTypes;
  IsNotEffectiveG<FareByRuleApp> _isNotEffective;
  bool _isFareDisplay;
};

struct FareByRuleAppDAO::FilterCorpId : FareByRuleAppDAO::FilterPaxType
{
public:
  FilterCorpId(const TktDesignator& tktDesignator,
               const std::vector<PaxTypeCode>& paxTypes,
               const std::vector<CorpId*>& corpIds,
               const DateTime& tvlDate,
               const DateTime& ticketDate,
               bool isFareDisplay,
               bool skipBlankAcct)
    : FilterPaxType(tktDesignator, paxTypes, tvlDate, ticketDate, isFareDisplay),
      _corpIds(corpIds),
      _skipBlankAcct(skipBlankAcct)
  {
  }

  bool operator()(const FareByRuleApp* rec)
  {
    if (FilterPaxType::operator()(rec))
      return true;

    if (!matchCorpIds(rec, _corpIds))
      return true;

    return false;
  }

private:
  bool matchCorpIds(const FareByRuleApp* rec, const std::vector<CorpId*>& corpIds)
  {
    std::vector<CorpId*>::const_iterator i = _corpIds.begin();
    for (; i != _corpIds.end(); ++i)
    {
      CorpId& corpId = **i;
      if (UNLIKELY(_skipBlankAcct && corpId.accountCode().empty()))
        continue;
      if (UNLIKELY(!corpId.accountCode().empty() && !rec->accountCode().empty() &&
          corpId.accountCode() != rec->accountCode()))
        continue;
      if (corpId.vendor() != rec->vendor())
        continue;
      if (corpId.ruleTariff() != -1 && corpId.ruleTariff() != rec->ruleTariff())
        continue;
      if (!corpId.rule().empty() && corpId.rule() != rec->ruleNo())
        continue;
      return true;
    }
    return false;
  }

  const std::vector<CorpId*>& _corpIds;
  bool _skipBlankAcct;
};

struct FareByRuleAppDAO::FilterAdult : FareByRuleAppDAO::FilterPaxType
{
public:
  FilterAdult(const TktDesignator& tktDesignator,
              const std::vector<PaxTypeCode>& paxTypes,
              const DateTime& tvlDate,
              const DateTime& ticketDate,
              bool isFareDisplay)
    : FilterPaxType(tktDesignator, paxTypes, tvlDate, ticketDate, isFareDisplay)
  {
  }

  bool operator()(const FareByRuleApp* rec)
  {
    if (rec->primePaxType() != "ADT")
      return true;

    if (FilterPaxType::operator()(rec))
      return true;

    return false;
  }
};

FareByRuleAppDAO&
FareByRuleAppDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<tse::FareByRuleApp*>&
getFareByRuleAppData(const CarrierCode& carrier,
                     const std::string& corpId,
                     const AccountCode& accountCode,
                     const TktDesignator& tktDesignator,
                     const DateTime& tvlDate,
                     std::vector<PaxTypeCode>& paxTypes,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical,
                     bool isFareDisplay)
{
  std::vector<FareByRuleApp*>* fbrList = new std::vector<FareByRuleApp*>;
  deleteList.adopt(fbrList);

  if (UNLIKELY(isHistorical))
  {
    FareByRuleAppHistoricalDAO& dao = FareByRuleAppHistoricalDAO::instance();
    if (corpId.empty())
    {
      const std::vector<FareByRuleApp*>& fbr = dao.get(deleteList,
                                                       carrier,
                                                       accountCode,
                                                       tktDesignator,
                                                       paxTypes,
                                                       tvlDate,
                                                       ticketDate,
                                                       isFareDisplay);

      fbrList->insert(fbrList->end(), fbr.begin(), fbr.end());

      if (!accountCode.empty())
      {
        const std::vector<FareByRuleApp*>& fbr2 = dao.get(
            deleteList, carrier, "", tktDesignator, paxTypes, tvlDate, ticketDate, isFareDisplay);

        fbrList->insert(fbrList->end(), fbr2.begin(), fbr2.end());
      }
    } // if (corpId.empty())
    else
    {
      std::vector<CorpId*> corpAccts;

      const std::vector<CorpId*>& corpIds =
          getCorpIdData(corpId, carrier, tvlDate, deleteList, ticketDate, isHistorical);

      std::vector<CorpId*>::const_iterator i = corpIds.begin();
      while (i != corpIds.end())
      {
        AccountCode acct = (*i)->accountCode().c_str();
        corpAccts.push_back(*i);
        ++i;

        if (i != corpIds.end() && acct == (*i)->accountCode())
          continue;

        if (acct.empty()) // process rule tariffs
        {
          const std::vector<FareByRuleApp*>& fbr = getFareByRuleAppRuleTariffData(carrier,
                                                                                  tktDesignator,
                                                                                  tvlDate,
                                                                                  paxTypes,
                                                                                  corpAccts,
                                                                                  deleteList,
                                                                                  ticketDate,
                                                                                  isHistorical,
                                                                                  isFareDisplay);

          fbrList->insert(fbrList->end(), fbr.begin(), fbr.end());
        }
        else // process account codes
        {
          const std::vector<FareByRuleApp*>& fbr = dao.get(deleteList,
                                                           carrier,
                                                           acct,
                                                           tktDesignator,
                                                           paxTypes,
                                                           corpAccts,
                                                           tvlDate,
                                                           ticketDate,
                                                           isFareDisplay);

          fbrList->insert(fbrList->end(), fbr.begin(), fbr.end());
        }
        corpAccts.clear();
      } // while (i != corpIds.end())

      // get rec 8's with blank account code
      const std::vector<FareByRuleApp*>& fbr2 = dao.get(
          deleteList, carrier, "", tktDesignator, paxTypes, tvlDate, ticketDate, isFareDisplay);

      fbrList->insert(fbrList->end(), fbr2.begin(), fbr2.end());
    } // else (corpId not empty)

    return *fbrList;
  } // if (isHistorical)

  // Non-Historical Processing
  FareByRuleAppDAO& dao = FareByRuleAppDAO::instance();
  if (corpId.empty())
  {
    const std::vector<FareByRuleApp*>& fbr = dao.get(deleteList,
                                                     carrier,
                                                     accountCode,
                                                     tktDesignator,
                                                     paxTypes,
                                                     tvlDate,
                                                     ticketDate,
                                                     isFareDisplay);

    fbrList->insert(fbrList->end(), fbr.begin(), fbr.end());

    if (!accountCode.empty())
    {
      const std::vector<FareByRuleApp*>& fbr2 = dao.get(
          deleteList, carrier, "", tktDesignator, paxTypes, tvlDate, ticketDate, isFareDisplay);

      fbrList->insert(fbrList->end(), fbr2.begin(), fbr2.end());
    }
  } // if (corpId.empty())
  else // corpId not Empty
  {
    std::vector<CorpId*> corpAccts;

    const std::vector<CorpId*>& corpIds =
        getCorpIdData(corpId, carrier, tvlDate, deleteList, ticketDate, isHistorical);

    std::vector<CorpId*>::const_iterator i = corpIds.begin();

    while (i != corpIds.end())
    {
      AccountCode acct = (*i)->accountCode().c_str();
      corpAccts.push_back(*i);
      ++i;

      if (i != corpIds.end() && acct == (*i)->accountCode())
        continue;

      if (acct.empty()) // process rule tariffs
      {
        const std::vector<FareByRuleApp*>& fbr = getFareByRuleAppRuleTariffData(carrier,
                                                                                tktDesignator,
                                                                                tvlDate,
                                                                                paxTypes,
                                                                                corpAccts,
                                                                                deleteList,
                                                                                ticketDate,
                                                                                isHistorical,
                                                                                isFareDisplay);

        fbrList->insert(fbrList->end(), fbr.begin(), fbr.end());
      }
      else // process account codes
      {
        const std::vector<FareByRuleApp*>& fbr = dao.get(deleteList,
                                                         carrier,
                                                         acct,
                                                         tktDesignator,
                                                         paxTypes,
                                                         corpAccts,
                                                         tvlDate,
                                                         ticketDate,
                                                         isFareDisplay);

        fbrList->insert(fbrList->end(), fbr.begin(), fbr.end());
      }
      corpAccts.clear();
    } // while (i != corpIds.end())

    // get rec 8's with blank account code
    const std::vector<FareByRuleApp*>& fbr2 = dao.get(
        deleteList, carrier, "", tktDesignator, paxTypes, tvlDate, ticketDate, isFareDisplay);

    fbrList->insert(fbrList->end(), fbr2.begin(), fbr2.end());
  }

  return *fbrList;
}

// no corpid, match by account code (which could be blank)
const std::vector<FareByRuleApp*>&
FareByRuleAppDAO::get(DeleteList& del,
                      const CarrierCode& carrier,
                      const AccountCode& accountCode,
                      const TktDesignator& tktDesignator,
                      const std::vector<PaxTypeCode>& paxTypes,
                      const DateTime& tvlDate,
                      const DateTime& ticketDate,
                      bool isFareDisplay)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  std::vector<FareByRuleApp*>* ret = new std::vector<FareByRuleApp*>;
  del.adopt(ret);

  FilterPaxType filter(tktDesignator, paxTypes, tvlDate, ticketDate, isFareDisplay);
  FareByRuleAppKey key(carrier, accountCode);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), filter);
  if (carrier != "YY")
  {
    FareByRuleAppKey key2("YY", accountCode);
    ptr = cache().get(key2);
    del.copy(ptr);
    remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), filter);
  }
  return *ret;
}

// match rec8s with account codes against corpids with account codes
const std::vector<FareByRuleApp*>&
FareByRuleAppDAO::get(DeleteList& del,
                      const CarrierCode& carrier,
                      const AccountCode& accountCode,
                      const TktDesignator& tktDesignator,
                      const std::vector<PaxTypeCode>& paxTypes,
                      const std::vector<CorpId*>& corpIds,
                      const DateTime& tvlDate,
                      const DateTime& ticketDate,
                      bool isFareDisplay)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  std::vector<FareByRuleApp*>* ret = new std::vector<FareByRuleApp*>;
  del.adopt(ret);

  FilterCorpId filter(tktDesignator, paxTypes, corpIds, tvlDate, ticketDate, isFareDisplay, false);
  FareByRuleAppKey key(carrier, accountCode);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), filter);
  if (carrier != "YY")
  {
    FareByRuleAppKey key2("YY", accountCode);
    ptr = cache().get(key2);
    del.copy(ptr);
    remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), filter);
  }
  return *ret;
}

// match rec8s with no account code against corpids with account codes
const std::vector<FareByRuleApp*>&
FareByRuleAppDAO::get(DeleteList& del,
                      const CarrierCode& carrier,
                      const TktDesignator& tktDesignator,
                      const std::vector<PaxTypeCode>& paxTypes,
                      const std::vector<CorpId*>& corpIds,
                      const DateTime& tvlDate,
                      const DateTime& ticketDate,
                      bool isFareDisplay)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  std::vector<FareByRuleApp*>* ret = new std::vector<FareByRuleApp*>;
  del.adopt(ret);

  FilterCorpId filter(tktDesignator, paxTypes, corpIds, tvlDate, ticketDate, isFareDisplay, true);
  FareByRuleAppKey key(carrier, "");
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), filter);
  if (carrier != "YY")
  {
    FareByRuleAppKey key2("YY", "");
    ptr = cache().get(key2);
    del.copy(ptr);
    remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), filter);
  }
  return *ret;
}

// match ADT rec8s
const std::vector<FareByRuleApp*>&
FareByRuleAppDAO::get(DeleteList& del,
                      const CarrierCode& carrier,
                      const TktDesignator& tktDesignator,
                      const std::vector<PaxTypeCode>& paxTypes,
                      const DateTime& tvlDate,
                      const DateTime& ticketDate,
                      bool isFareDisplay)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  std::vector<FareByRuleApp*>* ret = new std::vector<FareByRuleApp*>;
  del.adopt(ret);

  FilterAdult filter(tktDesignator, paxTypes, tvlDate, ticketDate, isFareDisplay);
  FareByRuleAppKey key(carrier, "");
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), filter);
  if (carrier != "YY")
  {
    FareByRuleAppKey key2("YY", "");
    ptr = cache().get(key2);
    del.copy(ptr);
    remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), filter);
  }
  return *ret;
}

std::vector<FareByRuleApp*>*
FareByRuleAppDAO::create(FareByRuleAppKey key)
{
  std::vector<FareByRuleApp*>* ret = new std::vector<FareByRuleApp*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareByRuleApp fbr(dbAdapter->getAdapter());
    fbr.findFareByRuleApp(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareByRuleAppDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FareByRuleAppDAO::destroy(FareByRuleAppKey key, std::vector<FareByRuleApp*>* recs)
{
  destroyContainer(recs);
}

sfc::CompressedData*
FareByRuleAppDAO::compress(const std::vector<FareByRuleApp*>* vect) const
{
  return compressVector(vect);
}

std::vector<FareByRuleApp*>*
FareByRuleAppDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<FareByRuleApp>(compressed);
}

std::string
FareByRuleAppDAO::_name("FareByRuleApp");
std::string
FareByRuleAppDAO::_cacheClass("Fares");
DAOHelper<FareByRuleAppDAO>
FareByRuleAppDAO::_helper(_name);
FareByRuleAppDAO* FareByRuleAppDAO::_instance = nullptr;

FareByRuleAppKey
FareByRuleAppDAO::createKey(const FareByRuleApp* info)
{
  return FareByRuleAppKey(info->carrier(), info->accountCode());
}

void
FareByRuleAppDAO::load()
{
  StartupLoaderNoDB<FareByRuleApp, FareByRuleAppDAO>();
}

///////////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////////
log4cxx::LoggerPtr
FareByRuleAppHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareByRuleAppHistoricalDAO"));

struct FareByRuleAppHistoricalDAO::FilterPaxType
{
public:
  FilterPaxType(const TktDesignator& tktDesignator,
                const std::vector<PaxTypeCode>& paxTypes,
                const DateTime& tvlDate,
                const DateTime& ticketDate,
                bool isFareDisplay)
    : _tktDesignator(tktDesignator),
      _paxTypes(paxTypes),
      _isNotEffective(tvlDate, ticketDate),
      _isFareDisplay(isFareDisplay)
  {
  }

  bool operator()(const FareByRuleApp* rec)
  {
    if (_isFareDisplay)
    {
      if (InhibitForFD<FareByRuleApp>(rec))
        return true;
    }
    else
    {
      if (Inhibit<FareByRuleApp>(rec))
        return true;
    }

    if (_isNotEffective(rec))
      return true;

    if (!rec->tktDesignator().empty() && rec->tktDesignator() != _tktDesignator)
      return true;

    if (std::find(_paxTypes.begin(), _paxTypes.end(), rec->primePaxType()) == _paxTypes.end() &&
        std::find_first_of(_paxTypes.begin(),
                           _paxTypes.end(),
                           rec->secondaryPaxTypes().begin(),
                           rec->secondaryPaxTypes().end()) == _paxTypes.end())
      return true;

    return false;
  }

private:
  const TktDesignator& _tktDesignator;
  const std::vector<PaxTypeCode>& _paxTypes;
  IsNotEffectiveH<FareByRuleApp> _isNotEffective;
  bool _isFareDisplay;
};

struct FareByRuleAppHistoricalDAO::FilterCorpId : FareByRuleAppHistoricalDAO::FilterPaxType
{
public:
  FilterCorpId(const TktDesignator& tktDesignator,
               const std::vector<PaxTypeCode>& paxTypes,
               const std::vector<CorpId*>& corpIds,
               const DateTime& tvlDate,
               const DateTime& ticketDate,
               bool isFareDisplay,
               bool skipBlankAcct)
    : FilterPaxType(tktDesignator, paxTypes, tvlDate, ticketDate, isFareDisplay),
      _corpIds(corpIds),
      _skipBlankAcct(skipBlankAcct)
  {
  }

  bool operator()(const FareByRuleApp* rec)
  {
    if (FilterPaxType::operator()(rec))
      return true;

    if (!matchCorpIds(rec, _corpIds))
      return true;

    return false;
  }

private:
  bool matchCorpIds(const FareByRuleApp* rec, const std::vector<CorpId*>& corpIds)
  {
    std::vector<CorpId*>::const_iterator i = _corpIds.begin();
    for (; i != _corpIds.end(); ++i)
    {
      CorpId& corpId = **i;
      if (_skipBlankAcct && corpId.accountCode().empty())
        continue;
      if (!corpId.accountCode().empty() && !rec->accountCode().empty() &&
          corpId.accountCode() != rec->accountCode())
        continue;
      if (corpId.vendor() != rec->vendor())
        continue;
      if (corpId.ruleTariff() != -1 && corpId.ruleTariff() != rec->ruleTariff())
        continue;
      if (!corpId.rule().empty() && corpId.rule() != rec->ruleNo())
        continue;
      return true;
    }
    return false;
  }

  const std::vector<CorpId*>& _corpIds;
  bool _skipBlankAcct;
};

struct FareByRuleAppHistoricalDAO::FilterAdult : FareByRuleAppHistoricalDAO::FilterPaxType
{
public:
  FilterAdult(const TktDesignator& tktDesignator,
              const std::vector<PaxTypeCode>& paxTypes,
              const DateTime& tvlDate,
              const DateTime& ticketDate,
              bool isFareDisplay)
    : FilterPaxType(tktDesignator, paxTypes, tvlDate, ticketDate, isFareDisplay)
  {
  }

  bool operator()(const FareByRuleApp* rec)
  {
    if (rec->primePaxType() != "ADT")
      return true;

    if (FilterPaxType::operator()(rec))
      return true;

    return false;
  }
};

FareByRuleAppHistoricalDAO&
FareByRuleAppHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

// no corpid, match by account code (which could be blank)
const std::vector<FareByRuleApp*>&
FareByRuleAppHistoricalDAO::get(DeleteList& del,
                                const CarrierCode& carrier,
                                const AccountCode& accountCode,
                                const TktDesignator& tktDesignator,
                                const std::vector<PaxTypeCode>& paxTypes,
                                const DateTime& tvlDate,
                                const DateTime& ticketDate,
                                bool isFareDisplay)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  std::vector<FareByRuleApp*>* ret = new std::vector<FareByRuleApp*>;
  del.adopt(ret);

  FilterPaxType filter(tktDesignator, paxTypes, tvlDate, ticketDate, isFareDisplay);
  FareByRuleAppHistoricalKey key(carrier, accountCode);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), filter);
  return *ret;
}

// match rec8s with account codes against corpids with account codes
const std::vector<FareByRuleApp*>&
FareByRuleAppHistoricalDAO::get(DeleteList& del,
                                const CarrierCode& carrier,
                                const AccountCode& accountCode,
                                const TktDesignator& tktDesignator,
                                const std::vector<PaxTypeCode>& paxTypes,
                                const std::vector<CorpId*>& corpIds,
                                const DateTime& tvlDate,
                                const DateTime& ticketDate,
                                bool isFareDisplay)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  std::vector<FareByRuleApp*>* ret = new std::vector<FareByRuleApp*>;
  del.adopt(ret);

  FilterCorpId filter(tktDesignator, paxTypes, corpIds, tvlDate, ticketDate, isFareDisplay, false);
  FareByRuleAppHistoricalKey key(carrier, accountCode);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), filter);
  return *ret;
}

// match rec8s with no account code against corpids with account codes
const std::vector<FareByRuleApp*>&
FareByRuleAppHistoricalDAO::get(DeleteList& del,
                                const CarrierCode& carrier,
                                const TktDesignator& tktDesignator,
                                const std::vector<PaxTypeCode>& paxTypes,
                                const std::vector<CorpId*>& corpIds,
                                const DateTime& tvlDate,
                                const DateTime& ticketDate,
                                bool isFareDisplay)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  std::vector<FareByRuleApp*>* ret = new std::vector<FareByRuleApp*>;
  del.adopt(ret);

  FilterCorpId filter(tktDesignator, paxTypes, corpIds, tvlDate, ticketDate, isFareDisplay, true);
  FareByRuleAppHistoricalKey key(carrier, "");
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), filter);
  return *ret;
}

// match ADT rec8s
const std::vector<FareByRuleApp*>&
FareByRuleAppHistoricalDAO::get(DeleteList& del,
                                const CarrierCode& carrier,
                                const TktDesignator& tktDesignator,
                                const std::vector<PaxTypeCode>& paxTypes,
                                const DateTime& tvlDate,
                                const DateTime& ticketDate,
                                bool isFareDisplay)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  std::vector<FareByRuleApp*>* ret = new std::vector<FareByRuleApp*>;
  del.adopt(ret);

  FilterAdult filter(tktDesignator, paxTypes, tvlDate, ticketDate, isFareDisplay);
  FareByRuleAppHistoricalKey key(carrier, "");
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), filter);
  return *ret;
}

std::vector<FareByRuleApp*>*
FareByRuleAppHistoricalDAO::create(FareByRuleAppHistoricalKey key)
{
  std::vector<FareByRuleApp*>* ret = new std::vector<FareByRuleApp*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetFareByRuleAppHistorical fbr(dbAdapter->getAdapter());
    fbr.findFareByRuleApp(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareByRuleAppHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FareByRuleAppHistoricalDAO::destroy(FareByRuleAppHistoricalKey key,
                                    std::vector<FareByRuleApp*>* recs)
{
  destroyContainer(recs);
}

FareByRuleAppHistoricalKey
FareByRuleAppHistoricalDAO::createKey(const FareByRuleApp* info,
                                      const DateTime& startDate,
                                      const DateTime& endDate)
{
  return FareByRuleAppHistoricalKey(info->carrier(), info->accountCode(), startDate, endDate);
}

void
FareByRuleAppHistoricalDAO::load()
{
  StartupLoaderNoDB<FareByRuleApp, FareByRuleAppHistoricalDAO>();
}

sfc::CompressedData*
FareByRuleAppHistoricalDAO::compress(const std::vector<FareByRuleApp*>* vect) const
{
  return compressVector(vect);
}

std::vector<FareByRuleApp*>*
FareByRuleAppHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<FareByRuleApp>(compressed);
}

std::string
FareByRuleAppHistoricalDAO::_name("FareByRuleAppHistorical");
std::string
FareByRuleAppHistoricalDAO::_cacheClass("Fares");
DAOHelper<FareByRuleAppHistoricalDAO>
FareByRuleAppHistoricalDAO::_helper(_name);
FareByRuleAppHistoricalDAO* FareByRuleAppHistoricalDAO::_instance = nullptr;

} // namespace tse
