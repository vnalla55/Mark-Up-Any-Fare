//------------------------------------------------------------------------------
// Copyright 2006, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly
// prohibited.
//------------------------------------------------------------------------------
//

#include "DBAccess/ETicketCarrierDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetETicketCxrs.h"

#include <string>

#include <time.h>

namespace tse
{
static const std::vector<CarrierCode> emptyList;

log4cxx::LoggerPtr
ETicketCarrierDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.ETicketCarrierDAO"));

ETicketCarrierDAO&
ETicketCarrierDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<ETicketCarrierInfo*>&
ETicketCarrierDAO::get(DeleteList& del, const PseudoCityCode& agentPcc, const CarrierCode& carrier)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  ETicketCarrierKey key(agentPcc, carrier);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  return *ptr;
}

std::vector<ETicketCarrierInfo*>*
ETicketCarrierDAO::create(ETicketCarrierKey key)
{
  std::vector<ETicketCarrierInfo*>* ret = new std::vector<ETicketCarrierInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetETicketCxrs etktCxrs(dbAdapter->getAdapter());

    ETicketCarrierInfo* info = new ETicketCarrierInfo;
    info->pseudoCity() = key._a;
    info->carrier() = key._b;
    ret->push_back(info);

    etktCxrs.getETicketCarriers(info->interlineCarriers(), key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in ETicketCarrierDAO::load");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
ETicketCarrierDAO::destroy(ETicketCarrierKey key, std::vector<ETicketCarrierInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
ETicketCarrierDAO::_name("ETicketCarrier");
std::string
ETicketCarrierDAO::_cacheClass("Common");

DAOHelper<ETicketCarrierDAO>
ETicketCarrierDAO::_helper(_name);

ETicketCarrierDAO* ETicketCarrierDAO::_instance = nullptr;

ETicketCarrierKey
ETicketCarrierDAO::createKey(const ETicketCarrierInfo* info)
{
  return ETicketCarrierKey(info->pseudoCity(), info->carrier());
}

void
ETicketCarrierDAO::load()
{
  StartupLoaderNoDB<ETicketCarrierInfo, ETicketCarrierDAO>();
}

} // namespace tse
