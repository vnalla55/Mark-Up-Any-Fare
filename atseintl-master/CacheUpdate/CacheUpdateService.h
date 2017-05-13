//----------------------------------------------------------------------------
//
//  File:               CacheUpdateService.h
//  Description:        Service class for the Transaction Orchestrator
//  Created:            11/16/2003
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Service/Service.h"
#include "Common/TseStringTypes.h"

#include <string>

namespace tse
{
class CacheTrx;
class TseServer;

class CacheUpdateService final : public Service
{
private:
  friend class CacheUpdateServiceTest;

  virtual bool process(CacheTrx& trx) override;

public:
  CacheUpdateService(const std::string& name, TseServer& server) : Service(name, server) {}

  bool initialize(int argc = 0, char* argv[] = nullptr) override { return true; }
};
} // tse namespace

