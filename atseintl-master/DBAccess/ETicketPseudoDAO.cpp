//------------------------------------------------------------------------------
// Copyright 2006, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly
// prohibited.
//------------------------------------------------------------------------------
//

#include "DBAccess/ETicketPseudoDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetETicketPseudo.h"

#include <string>

#include <time.h>

namespace tse
{
static const CarrierCode emptyCarrier = "";

log4cxx::LoggerPtr
ETicketPseudoDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.ETicketPseudoDAO"));

ETicketPseudoDAO&
ETicketPseudoDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<ETicketPseudoInfo*>&
ETicketPseudoDAO::get(DeleteList& del, const PseudoCityCode& agentPcc, const CarrierCode& carrier)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  ETicketPseudoKey key(agentPcc, carrier);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  return *ptr;
}

std::vector<ETicketPseudoInfo*>*
ETicketPseudoDAO::create(ETicketPseudoKey key)
{
  std::vector<ETicketPseudoInfo*>* ret = new std::vector<ETicketPseudoInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetETicketPseudo etktPseudo(dbAdapter->getAdapter());

    ETicketPseudoInfo* info = new ETicketPseudoInfo;
    info->pseudoCity() = key._a;
    info->interlineCarrier() = key._b;
    ret->push_back(info);

    info->eTicketCarrier() = etktPseudo.getETicketPseudo(key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in ETicketPseudoDAO::load");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
ETicketPseudoDAO::destroy(ETicketPseudoKey key, std::vector<ETicketPseudoInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
ETicketPseudoDAO::_name("ETicketPseudo");
std::string
ETicketPseudoDAO::_cacheClass("Common");

DAOHelper<ETicketPseudoDAO>
ETicketPseudoDAO::_helper(_name);

ETicketPseudoDAO* ETicketPseudoDAO::_instance = nullptr;

ETicketPseudoKey
ETicketPseudoDAO::createKey(const ETicketPseudoInfo* info)
{
  return ETicketPseudoKey(info->pseudoCity(), info->interlineCarrier());
}

void
ETicketPseudoDAO::load()
{
  StartupLoaderNoDB<ETicketPseudoInfo, ETicketPseudoDAO>();
}

} // namespace tse
