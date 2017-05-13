//----------------------------------------------------------------------------
//
//     File:           ORACLEDBHistoryServerFactory.h
//     Description:    Header for the ORACLE History DB Server Factory.
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

#pragma once
#include "Common/Config/ConfigMan.h"
#include "Common/KeyedFactory.h"
#include "Common/Logger.h"
#include "Common/TseSynchronizingValue.h"
#include "DBAccess/DBConnectionKey.h"
#include "DBAccess/DBHistoryServer.h"
#include "DBAccess/DBHistoryServerFactory.h"

#include <string>

namespace tse
{

class ORACLEDBHistoryServerFactory : public DBHistoryServerFactory
{
private:
  bool initORACLE(std::string DBConnGroup, DBHistoryServer* dbServer, std::string& key);
  log4cxx::LoggerPtr& getLogger();

public:
  ORACLEDBHistoryServerFactory(tse::ConfigMan& config) : DBHistoryServerFactory(config) {}
  virtual ~ORACLEDBHistoryServerFactory() {}
  virtual DBHistoryServer* create(tse::DBConnectionKey key) override;
  virtual bool validate(tse::DBConnectionKey, DBHistoryServer* object) override;
  virtual void activate(tse::DBConnectionKey, DBHistoryServer* object) override;
  virtual void passivate(tse::DBConnectionKey, DBHistoryServer* object) override;
  virtual void destroy(tse::DBConnectionKey, DBHistoryServer* object) override;

  virtual void modifyCurrentSynchValue() override;

  virtual TseSynchronizingValue getSyncToken() override;
};
}
