//----------------------------------------------------------------------------
//
// Copyright Sabre 2014
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

#include "Service/Service.h"

namespace tse
{
class ConfigMan;
class DecodeTrx;
class FrequentFlyerTrx;
class TseServer;

class DecodeService final : public Service
{
  friend class DecodeServiceTest;

public:
  DecodeService(const std::string& name, tse::TseServer& server);

  bool initialize(int argc = 0, char* argv[] = 0) override { return true; }

  bool process(DecodeTrx& trx) override;
  bool process(FrequentFlyerTrx& trx) override;

private:
  ConfigMan& _config;
};
} // end namespace tse
