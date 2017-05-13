//------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
//

#include "DBAccess/FareDisplayWebDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareDisplayWeb.h"
#include "DBAccess/Queries/QueryGetFareDisplayWeb.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
FareDisplayWebDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FareDisplayWebDAO"));

FareDisplayWebDAO&
FareDisplayWebDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareDisplayWeb*>&
getFareDisplayWebForCxrData(const CarrierCode& carrier, DeleteList& deleteList)
{
  FareDisplayWebDAO& dao = FareDisplayWebDAO::instance();
  return dao.get(deleteList, carrier);
}

const std::vector<FareDisplayWeb*>&
FareDisplayWebDAO::get(DeleteList& del, const CarrierCode& carrier)
{
  CarrierKey key(carrier);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  return *ptr;
}

const std::set<std::pair<PaxTypeCode, VendorCode> >&
getFareDisplayWebPaxForCxrData(const CarrierCode& carrier, DeleteList& deleteList)
{
  FareDisplayWebDAO& dao = FareDisplayWebDAO::instance();
  return dao.getPsgForCxr(deleteList, carrier);
}

const std::set<std::pair<PaxTypeCode, VendorCode> >&
FareDisplayWebDAO::getPsgForCxr(DeleteList& del, const CarrierCode& carrier)
{
  CarrierKey key(carrier);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::set<std::pair<PaxTypeCode, VendorCode> >* ret =
      new std::set<std::pair<PaxTypeCode, VendorCode> >;
  del.adopt(ret);

  DAOCache::value_type::iterator iter = ptr->begin();
  DAOCache::value_type::iterator iterEnd = ptr->end();

  for (; iter != iterEnd; iter++)
  {
    std::pair<PaxTypeCode, VendorCode> retPair((*iter)->paxType(), (*iter)->vendor());
    ret->insert(retPair);
  }
  return *ret;
}

const std::vector<FareDisplayWeb*>&
getFareDisplayWebData(const Indicator& dispInd,
                      const VendorCode& vendor,
                      const CarrierCode& carrier,
                      const TariffNumber& ruleTariff,
                      const RuleNumber& rule,
                      const PaxTypeCode& paxTypeCode,
                      DeleteList& deleteList)
{
  FareDisplayWebDAO& dao = FareDisplayWebDAO::instance();
  return dao.get(deleteList, dispInd, vendor, carrier, ruleTariff, rule, paxTypeCode);
}

const std::vector<FareDisplayWeb*>&
FareDisplayWebDAO::get(DeleteList& del,
                       const Indicator& displayInd,
                       const VendorCode& vendor,
                       const CarrierCode& carrier,
                       const TariffNumber& ruleTariff,
                       const RuleNumber& rule,
                       const PaxTypeCode& paxTypeCode)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  CarrierKey key(carrier);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<FareDisplayWeb*>* ret = new std::vector<FareDisplayWeb*>;
  del.adopt(ret);
  DAOCache::value_type::iterator iter = ptr->begin();
  DAOCache::value_type::iterator iterEnd = ptr->end();
  for (; iter != iterEnd; iter++)
  {
    if ((((*iter)->vendor().empty()) || ((*iter)->vendor() == vendor)) &&
        (((*iter)->ruleTariff() == 0) || ((*iter)->ruleTariff() == ruleTariff)) &&
        (((*iter)->rule().empty()) || ((*iter)->rule() == rule)) &&
        ((*iter)->paxType() == paxTypeCode) &&
        (((*iter)->displayInd() == RuleConst::BLANK) ||
         (((*iter)->displayInd() != RuleConst::BLANK) && ((*iter)->displayInd() == displayInd))))
    {
      ret->push_back(*iter);
    }
  }
  return *ret;
}

std::vector<FareDisplayWeb*>*
FareDisplayWebDAO::create(CarrierKey key)
{
  std::vector<FareDisplayWeb*>* ret = new std::vector<FareDisplayWeb*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareDisplayWeb dw(dbAdapter->getAdapter());
    dw.findFareDisplayWeb(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareDisplayWebDAO::create");
    throw;
  }

  return ret;
}

void
FareDisplayWebDAO::destroy(CarrierKey key, std::vector<FareDisplayWeb*>* recs)
{
  destroyContainer(recs);
}

CarrierKey
FareDisplayWebDAO::createKey(FareDisplayWeb* info)
{
  return CarrierKey(info->carrier());
}

void
FareDisplayWebDAO::load()
{
  StartupLoader<QueryGetAllFareDisplayWeb, FareDisplayWeb, FareDisplayWebDAO>();
}

std::string
FareDisplayWebDAO::_name("FareDisplayWeb");
std::string
FareDisplayWebDAO::_cacheClass("FareDisplay");

DAOHelper<FareDisplayWebDAO>
FareDisplayWebDAO::_helper(_name);

FareDisplayWebDAO* FareDisplayWebDAO::_instance = nullptr;

} // namespace tse
