//----------------------------------------------------------------------------
//
//        File: CurrencyService.h
// Description: Currency service class
//     Created: 06/07/2004
//     Authors: Mark Kasprowicz
//
// Copyright Sabre 2004
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/Global.h"
#include "Service/Service.h"

#include <string>

namespace tse
{
class Trx;
class CurrencyTrx;
class TseServer;
class CurrencyService;
class ConfigMan;

class CurrencyService final : public Service
{

public:
  CurrencyService(const std::string& sname, TseServer& tseServer)
    : Service(sname, tseServer), _config(Global::config())
  {
  }

  bool initialize(int argc = 0, char* argv[] = nullptr) override { return true; }

  bool process(CurrencyTrx& trx) override;

private:
  ConfigMan& _config;
};
} // end namespace tse
