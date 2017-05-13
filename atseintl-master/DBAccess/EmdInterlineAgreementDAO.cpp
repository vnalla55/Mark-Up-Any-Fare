//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/EmdInterlineAgreementDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DAOUtils.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/EmdInterlineAgreementInfo.h"
#include "DBAccess/HistoricalDataAccessObject.h"
#include "DBAccess/Queries/QueryGetEmdInterlineAgreement.h"

#include <algorithm>

namespace tse
{

log4cxx::LoggerPtr
EmdInterlineAgreementDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.EmdInterlineAgreementDAO"));

EmdInterlineAgreementDAO&
EmdInterlineAgreementDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<EmdInterlineAgreementInfo*>&
getEmdInterlineAgreementData(const NationCode& country,
                             const CrsCode& gds,
                             const CarrierCode& validatingCarrier,
                             DeleteList& deleteList,
                             const DateTime& ticketDate,
                             bool isHistorical)
{
  EmdInterlineAgreementDAO& dao = EmdInterlineAgreementDAO::instance();
  return dao.get(deleteList, country, gds, validatingCarrier, ticketDate);
}

const std::vector<EmdInterlineAgreementInfo*>&
EmdInterlineAgreementDAO::get(DeleteList& del,
                              const NationCode& country,
                              const CrsCode& gds,
                              const CarrierCode& validatingCarrier,
                              const DateTime& ticketDate)
{
  _codeCoverageGetCallCount++;

  EmdInterlineAgreementKey eiaKey(country, gds, validatingCarrier);
  DAOCache::pointer_type ptr = cache().get(eiaKey);
  del.copy(ptr);
  return *applyFilter(del,
                      ptr,
                      IsNotEffectiveG<EmdInterlineAgreementInfo>(ticketDate));
}

std::vector<EmdInterlineAgreementInfo*>*
EmdInterlineAgreementDAO::create(EmdInterlineAgreementKey eiaKey)
{
  std::vector<EmdInterlineAgreementInfo*>* eiaList =
      new std::vector<EmdInterlineAgreementInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetEmdInterlineAgreement eiaQuery(dbAdapter->getAdapter());
    eiaQuery.findEmdInterlineAgreement(*eiaList, eiaKey._a, eiaKey._b, eiaKey._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in EmdInterlineAgreementDAO::create");
    destroyContainer(eiaList);
    throw;
  }

  return eiaList;
}

EmdInterlineAgreementKey
EmdInterlineAgreementDAO::createKey(const EmdInterlineAgreementInfo* eia)
{
  return EmdInterlineAgreementKey(
      eia->getCountryCode(), eia->getGds(), eia->getValidatingCarrier());
}

bool
EmdInterlineAgreementDAO::translateKey(const ObjectKey& objectKey,
                                       EmdInterlineAgreementKey& eiaKey) const
{
  eiaKey.initialized = objectKey.getValue("NATION", eiaKey._a) &&
                       objectKey.getValue("GDS", eiaKey._b) &&
                       objectKey.getValue("VALIDATINGCARRIER", eiaKey._c);

  return eiaKey.initialized;
}

void
EmdInterlineAgreementDAO::translateKey(const EmdInterlineAgreementKey& eiaKey,
                                       ObjectKey& objectKey) const
{
  objectKey.setValue("NATION", eiaKey._a);
  objectKey.setValue("GDS", eiaKey._b);
  objectKey.setValue("VALIDATINGCARRIER", eiaKey._c);
}

bool
EmdInterlineAgreementDAO::insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
{
  return DummyObjectInserter<EmdInterlineAgreementInfo, EmdInterlineAgreementDAO>(
             flatKey, objectKey).success();
}

void
EmdInterlineAgreementDAO::destroy(EmdInterlineAgreementKey key,
                                  std::vector<EmdInterlineAgreementInfo*>* eiaList)
{
  destroyContainer(eiaList);
}

void
EmdInterlineAgreementDAO::load()
{
  StartupLoaderNoDB<EmdInterlineAgreementInfo, EmdInterlineAgreementDAO>();
}

size_t
EmdInterlineAgreementDAO::clear()
{
  size_t result (cache().clear());
  LOG4CXX_ERROR(_logger, "EmdInterlineAgreement cache cleared");
  return result;
}

std::string
EmdInterlineAgreementDAO::_name("EmdInterlineAgreement");
std::string
EmdInterlineAgreementDAO::_cacheClass("Common");
DAOHelper<EmdInterlineAgreementDAO>
EmdInterlineAgreementDAO::_helper(_name);
EmdInterlineAgreementDAO* EmdInterlineAgreementDAO::_instance = nullptr;

}
