//----------------------------------------------------------------------------
//
//     File:           ORACLEDBHistoryServerFactory.cpp
//     Description:    Implementation for the ORACLE History DB Server Factory.
//     Created:        04/28/2009
//     Authors:        Emad Girgis
//
//     Updates:
//
//     Copyright 2009, Sabre Inc.  All rights reserved.
//         This software/documentation is the confidential and proprietary
//         product of Sabre Inc. Any unauthorized use, reproduction, or
//         transfer of this software/documentation, in any medium, or
//         incorporation of this software/documentation into any system
//         or publication, is strictly prohibited.
//
//----------------------------------------------------------------------------

#include "DBAccess/ORACLEDBHistoryServerFactory.h"

#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/TSEException.h"
#include "DBAccess/DBConnectionInfoManager.h"
#include "DBAccess/DBHistoryServer.h"
#include "DBAccess/ORACLEDBHistoryServer.h"

using namespace std;
using namespace DBAccess;

namespace tse
{

log4cxx::LoggerPtr&
ORACLEDBHistoryServerFactory::getLogger()
{
  static log4cxx::LoggerPtr logger(
      log4cxx::Logger::getLogger("atseintl.DBAccess.ORACLEDBHistoryServerFactory"));
  return logger;
}

DBHistoryServer*
ORACLEDBHistoryServerFactory::create(tse::DBConnectionKey compoundKey)
{
  DBHistoryServer* dbServer = new tse::ORACLEDBHistoryServer(_config);
  if (!_config.getValue(compoundKey._pool, compoundKey._key, "Connection"))
  {
    compoundKey._key = "Historical";
  }
  if (!initORACLE(compoundKey._pool, dbServer, compoundKey._key))
  {
    LOG4CXX_ERROR(getLogger(),
                  "Cannot create connection for " << compoundKey._key << " in pool "
                                                  << compoundKey._pool);
    throw TSEException(TSEException::UNKNOWN_DATABASE, "UNABLE TO CONNECT TO ORACLE DATABASE");
  }
  return dbServer;
}

void
ORACLEDBHistoryServerFactory::destroy(tse::DBConnectionKey, DBHistoryServer* db)
{
  delete db;
}

bool
ORACLEDBHistoryServerFactory::validate(tse::DBConnectionKey, DBHistoryServer* db)
{
  ORACLEDBHistoryServer* dbServer = dynamic_cast<ORACLEDBHistoryServer*>(db);
  if (dbServer == nullptr)
  {
    return false;
  }

  if (isTimeForRefresh())
  {
    DBConnectionInfoManager::clearConnectInfo();
    modifyCurrentSynchValue();
  }

  return dbServer->isValid();
}

void
ORACLEDBHistoryServerFactory::activate(DBConnectionKey, DBHistoryServer* db)
{
}

void
ORACLEDBHistoryServerFactory::passivate(DBConnectionKey, DBHistoryServer* db)
{
}

void
ORACLEDBHistoryServerFactory::modifyCurrentSynchValue()
{
  ORACLEAdapter::modifyCurrentSynchValue();
}

TseSynchronizingValue
ORACLEDBHistoryServerFactory::getSyncToken()
{
  return ORACLEAdapter::getCurrentSynchValue();
}

bool
ORACLEDBHistoryServerFactory::initORACLE(std::string DBConnGroup,
                                         DBHistoryServer* dbServer,
                                         std::string& key)
{
  std::string host, database, service, user, password;
  uint16_t port(0);

  ORACLEDBHistoryServer* oracleDBServer = dynamic_cast<ORACLEDBHistoryServer*>(dbServer);

  if (!oracleDBServer)
  {
    LOG4CXX_ERROR(getLogger(),
                  "Wrong DBServer type. This should never happen."
                      << " Something is really messed up.");

    return false;
  }

  bool usePriorityOrder = true;
  for (int retry = 0; retry < 5; ++retry)
  {
    if (DBConnectionInfoManager::getConnectInfo(
            key, host, port, database, service, user, password, true))
    {
      if (oracleDBServer->init(key, key, user, password, service, host, 0))
      {
        return true;
      }
    }

    if (retry > 0)
      usePriorityOrder = false;

    DBConnectionInfoManager::processConnectionFailure(
        host, port, database, service, user, usePriorityOrder);

    modifyCurrentSynchValue();
  }
  return false;
}

} // namespace tse
