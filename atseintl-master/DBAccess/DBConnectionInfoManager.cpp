//----------------------------------------------------------------------------
//
//     File:           DBConnectionInfoManager.cpp
//     Description:
//     Created:        03/05/2010
//     Authors:        Andrew Ahmad
//
//     Updates:
//
//     Copyright 2010, Sabre Inc.  All rights reserved.
//         This software/documentation is the confidential and proprietary
//         product of Sabre Inc. Any unauthorized use, reproduction, or
//         transfer of this software/documentation, in any medium, or
//         incorporation of this software/documentation into any system
//         or publication, is strictly prohibited.
//
//----------------------------------------------------------------------------

#include "DBAccess/DBConnectionInfoManager.h"

#include "Common/Logger.h"
#include "DBAccess/DBConnectionInfoManagerImplBase.h"
#include "Util/BranchPrediction.h"

using namespace DBAccess;

DBConnectionInfoManagerImplBase* DBConnectionInfoManager::_impl = nullptr;

bool
DBConnectionInfoManager::getConnectInfo(const std::string& pool,
                                        std::string& host,
                                        uint16_t& port,
                                        std::string& database,
                                        std::string& service,
                                        std::string& user,
                                        std::string& password,
                                        bool isHistorical)
{
  if (getImpl())
    return getImpl()->getConnectInfo(
        pool, host, port, database, service, user, password, isHistorical);
  return false;
}

bool
DBConnectionInfoManager::isCurrentConnectInfo(const std::string& pool,
                                              const std::string& host,
                                              uint16_t port,
                                              const std::string& database,
                                              const std::string& service,
                                              const std::string& user,
                                              const std::string& password)
{
  if (LIKELY(getImpl()))
    return getImpl()->isCurrentConnectInfo(pool, host, port, database, service, user, password);
  return false;
}

void
DBConnectionInfoManager::processConnectionFailure(const std::string& host,
                                                  uint16_t port,
                                                  const std::string& database,
                                                  const std::string& service,
                                                  const std::string& user,
                                                  bool resetPriorityOrder)
{
  if (getImpl())
    getImpl()->processConnectionFailure(host, port, database, service, user, resetPriorityOrder);
}

bool
DBConnectionInfoManager::clearConnectInfo()
{
  if (getImpl())
    return getImpl()->clearConnectInfo();
  return false;
}

void
DBConnectionInfoManager::setImpl(DBConnectionInfoManagerImplBase* impl)
{
  _impl = impl;
}

DBConnectionInfoManagerImplBase*
DBConnectionInfoManager::getImpl()
{
  return _impl;
}
