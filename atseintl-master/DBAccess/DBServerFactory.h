//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once
#include "Common/Config/ConfigMan.h"
#include "Common/KeyedFactory.h"
#include "Common/Logger.h"
#include "Common/TseSynchronizingValue.h"
#include "DBAccess/DBConnectionKey.h"
#include "DBAccess/DBServer.h"

#include <iostream>
#include <string>

namespace tse
{

class DBServerFactory : public sfc::KeyedFactory<tse::DBConnectionKey, DBServer>
{
protected:
  tse::ConfigMan& _config;

  bool isTimeForRefresh(void);

public:
  DBServerFactory(tse::ConfigMan& config);

  virtual ~DBServerFactory();

  DBServer* create(tse::DBConnectionKey key) override = 0;

  bool validate(tse::DBConnectionKey, DBServer* object) override = 0;

  void activate(tse::DBConnectionKey, DBServer* object) override = 0;

  void passivate(tse::DBConnectionKey, DBServer* object) override = 0;

  void destroy(tse::DBConnectionKey, DBServer* object) override = 0;

  virtual void dumpDbConnections(std::ostream& os) = 0;

  virtual void modifyCurrentSynchValue() = 0;

  virtual TseSynchronizingValue getSyncToken() = 0;

private:
  log4cxx::LoggerPtr& getLogger();

  time_t _refreshTime;
  unsigned int _refreshTimeout;
  unsigned int _refreshTimeoutPercentVary;
};
}
