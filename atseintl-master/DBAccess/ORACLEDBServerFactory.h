//----------------------------------------------------------------------------
//
//     File:           ORACLEDBServerFactory.h
//     Description:    Header for the ORACLE DB Server Factory.
//     Created:        04/28/2009
//     Authors:        Emad Girgis
//
//     Updates:
//
//     Copyright 2009, Sabre Inc.  All rights reserved.
//         This software/documentation is the confidential and proprietary
//         product of Sabre Inc.
//         Any unauthorized use, reproduction, or transfer of this
//         software/documentation, in any medium, or incorporation of this
//         software/documentation into any system or publication,
//         is strictly prohibited
//
//----------------------------------------------------------------------------

#pragma once
#include "Common/Config/ConfigMan.h"
#include "Common/KeyedFactory.h"
#include "Common/Logger.h"
#include "Common/TseSynchronizingValue.h"
#include "DBAccess/DBConnectionKey.h"
#include "DBAccess/DBServer.h"
#include "DBAccess/DBServerFactory.h"

#include <iostream>
#include <string>

namespace tse
{

class ORACLEDBServerFactory : public DBServerFactory
{
private:
  bool initORACLE(std::string DBConnGroup, DBServer* dbServer, std::string& key);
  log4cxx::LoggerPtr& getLogger();

public:
  ORACLEDBServerFactory(tse::ConfigMan& config) : DBServerFactory(config) {}
  virtual ~ORACLEDBServerFactory() {}
  virtual DBServer* create(tse::DBConnectionKey key) override;

  virtual bool validate(tse::DBConnectionKey, DBServer* object) override;

  virtual void activate(tse::DBConnectionKey, DBServer* object) override;

  virtual void passivate(tse::DBConnectionKey, DBServer* object) override;

  virtual void destroy(tse::DBConnectionKey, DBServer* object) override;

  virtual void dumpDbConnections(std::ostream& os) override;

  virtual void modifyCurrentSynchValue() override;

  virtual TseSynchronizingValue getSyncToken() override;
};
}
