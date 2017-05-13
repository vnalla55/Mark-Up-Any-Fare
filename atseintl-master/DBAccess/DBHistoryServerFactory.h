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
#include "DBAccess/DBHistoryServer.h"

#include <string>

namespace tse
{

class DBHistoryServerFactory : public sfc::KeyedFactory<DBConnectionKey, DBHistoryServer>
{
protected:
  tse::ConfigMan& _config;

  bool isTimeForRefresh(void);

private:
  log4cxx::LoggerPtr& getLogger();

  time_t _refreshTime;
  unsigned int _refreshTimeout;
  unsigned int _refreshTimeoutPercentVary;

public:
  DBHistoryServerFactory(tse::ConfigMan& config);

  virtual ~DBHistoryServerFactory();

  DBHistoryServer* create(tse::DBConnectionKey key) override = 0;

  bool validate(tse::DBConnectionKey, DBHistoryServer* object) override = 0;

  void activate(tse::DBConnectionKey, DBHistoryServer* object) override = 0;

  void passivate(tse::DBConnectionKey, DBHistoryServer* object) override = 0;

  void destroy(tse::DBConnectionKey, DBHistoryServer* object) override = 0;

  virtual void modifyCurrentSynchValue() = 0;

  virtual TseSynchronizingValue getSyncToken() = 0;

private:
};
}
