//----------------------------------------------------------------------------
//
//     File:           ORACLEDBServerFactory.cpp
//     Description:    Implementation for the Oracle DB Server Factory.
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

#include "DBAccess/ORACLEDBServerFactory.h"

#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/TSEException.h"
#include "Common/TseUtil.h"
#include "DBAccess/DBConnectionInfoManager.h"
#include "DBAccess/DBServer.h"
#include "DBAccess/ORACLEAdapter.h"
#include "DBAccess/ORACLEDBServer.h"

using namespace std;
using namespace DBAccess;

namespace tse
{

log4cxx::LoggerPtr&
ORACLEDBServerFactory::getLogger()
{
  static log4cxx::LoggerPtr logger(
      log4cxx::Logger::getLogger("atseintl.DBAccess.ORACLEDBServerFactory"));
  return logger;
}

DBServer*
ORACLEDBServerFactory::create(tse::DBConnectionKey compoundKey)
{
  DBServer* dbServer = new tse::ORACLEDBServer(_config);
  if (!initORACLE(compoundKey._pool, dbServer, compoundKey._key))
  {
    LOG4CXX_ERROR(getLogger(),
                  "Cannot create connection for " << compoundKey._key << " in pool "
                                                  << compoundKey._pool);
    std::string text("UNABLE TO CONNECT TO ORACLE DATABASE");
    text += ": key: " + compoundKey._key += " pool: " + compoundKey._pool;
    LOG4CXX_ERROR(getLogger(), text);
    TseUtil::alert(text.c_str());
    throw TSEException(TSEException::UNKNOWN_DATABASE, text.c_str());
  }
  return dbServer;
}

void
ORACLEDBServerFactory::destroy(tse::DBConnectionKey, DBServer* db)
{
  delete db;
}

bool
ORACLEDBServerFactory::validate(tse::DBConnectionKey, DBServer* db)
{
  ORACLEDBServer* dbServer = dynamic_cast<ORACLEDBServer*>(db);
  if (UNLIKELY(dbServer == nullptr))
  {
    return false;
  }

  if (UNLIKELY(isTimeForRefresh()))
  {
    DBConnectionInfoManager::clearConnectInfo();
    modifyCurrentSynchValue();
  }

  return dbServer->isValid();
}

void
ORACLEDBServerFactory::activate(tse::DBConnectionKey, DBServer* db)
{
}

void
ORACLEDBServerFactory::passivate(tse::DBConnectionKey, DBServer* db)
{
}

void
ORACLEDBServerFactory::dumpDbConnections(std::ostream& os)
{
  ORACLEAdapter::dumpDbConnections(os);
}

void
ORACLEDBServerFactory::modifyCurrentSynchValue()
{
  ORACLEAdapter::modifyCurrentSynchValue();
}

TseSynchronizingValue
ORACLEDBServerFactory::getSyncToken()
{
  return ORACLEAdapter::getCurrentSynchValue();
}

bool
ORACLEDBServerFactory::initORACLE(std::string DBConnGroup, DBServer* dbServer, std::string& key)
{
  std::string host, database, service, user, password;
  uint16_t port(0);

  ORACLEDBServer* oracleDBServer = dynamic_cast<ORACLEDBServer*>(dbServer);

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
    if (DBConnectionInfoManager::getConnectInfo(key, host, port, database, service, user, password))
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
