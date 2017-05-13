//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/GeneralFareRuleBackDatingDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DBHistoryServer.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/Queries/QueryGetNonCombCatCtrl.h"

namespace tse
{
log4cxx::LoggerPtr
GeneralFareRuleBackDatingDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.GeneralFareRuleBackDatingDAO"));

GeneralFareRuleBackDatingDAO&
GeneralFareRuleBackDatingDAO::instance()
{
  if (_instance == nullptr)
    _helper.init();

  return *_instance;
}

const std::vector<GeneralFareRuleInfo*>&
getGeneralFareRuleBackDatingData(const VendorCode& vendor,
                                 const CarrierCode& carrier,
                                 const TariffNumber& ruleTariff,
                                 const RuleNumber& rule,
                                 const CatNumber& category,
                                 const DateTime& backDate,
                                 DeleteList& deleteList,
                                 bool isFareDisplay)
{
  GeneralFareRuleBackDatingDAO& dao = GeneralFareRuleBackDatingDAO::instance();

  return isFareDisplay
             ? dao.getForFD(deleteList, vendor, carrier, ruleTariff, rule, category, backDate)
             : dao.get(deleteList, vendor, carrier, ruleTariff, rule, category, backDate);
}

std::vector<GeneralFareRuleInfo*>&
GeneralFareRuleBackDatingDAO::commonGet(DeleteList& del,
                                        const VendorCode& vendor,
                                        const CarrierCode& carrier,
                                        const TariffNumber& ruleTariff,
                                        const RuleNumber& rule,
                                        const CatNumber& category,
                                        const DateTime& backDate)
{
  GeneralFareRuleBackDatingKey key(vendor, carrier, ruleTariff, rule, category);
  DAOUtils::getDateRange(backDate, key._f, key._g, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);

  std::vector<GeneralFareRuleInfo*>* ret = new std::vector<GeneralFareRuleInfo*>;
  del.adopt(ret);

  if (ptr.get() == nullptr)
    return *ret;

  del.copy(ptr);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveCatHist<GeneralFareRuleInfo>(backDate, backDate, backDate));

  return *ret;
}

const std::vector<GeneralFareRuleInfo*>&
GeneralFareRuleBackDatingDAO::get(DeleteList& del,
                                  const VendorCode& vendor,
                                  const CarrierCode& carrier,
                                  const TariffNumber& ruleTariff,
                                  const RuleNumber& rule,
                                  const CatNumber& category,
                                  const DateTime& backDate)
{
  std::vector<GeneralFareRuleInfo*>& ret =
      commonGet(del, vendor, carrier, ruleTariff, rule, category, backDate);
  return ret;
}

const std::vector<GeneralFareRuleInfo*>&
GeneralFareRuleBackDatingDAO::getForFD(DeleteList& del,
                                       const VendorCode& vendor,
                                       const CarrierCode& carrier,
                                       const TariffNumber& ruleTariff,
                                       const RuleNumber& rule,
                                       const CatNumber& category,
                                       const DateTime& backDate)
{
  std::vector<GeneralFareRuleInfo*>& ret =
      commonGet(del, vendor, carrier, ruleTariff, rule, category, backDate);
  ret.erase(std::remove_if(ret.begin(), ret.end(), InhibitForFD<GeneralFareRuleInfo>), ret.end());
  return ret;
}

std::vector<GeneralFareRuleInfo*>*
GeneralFareRuleBackDatingDAO::create(GeneralFareRuleBackDatingKey key)
{
  std::vector<GeneralFareRuleInfo*>* ret = new std::vector<GeneralFareRuleInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetNonCombCtrlBackDating ncc(dbAdapter->getAdapter());
    ncc.findGeneralFareRule(*ret, key._a, key._b, key._c, key._d, key._e, key._f, key._g);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in GeneralFareRuleBackDatingDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
GeneralFareRuleBackDatingDAO::destroy(GeneralFareRuleBackDatingKey key,
                                      std::vector<GeneralFareRuleInfo*>* recs)
{
  std::vector<GeneralFareRuleInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
GeneralFareRuleBackDatingDAO::compress(const std::vector<GeneralFareRuleInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<GeneralFareRuleInfo*>*
GeneralFareRuleBackDatingDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<GeneralFareRuleInfo>(compressed);
}

std::string
GeneralFareRuleBackDatingDAO::_name("GeneralFareRuleBackDating");

std::string
GeneralFareRuleBackDatingDAO::_cacheClass("Rules");

DAOHelper<GeneralFareRuleBackDatingDAO>
GeneralFareRuleBackDatingDAO::_helper(_name);

GeneralFareRuleBackDatingDAO* GeneralFareRuleBackDatingDAO::_instance = nullptr;
}
