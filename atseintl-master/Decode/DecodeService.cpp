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

#include "Decode/DecodeService.h"

#include "Common/Global.h"
#include "Common/Logger.h"
#include "DataModel/DecodeTrx.h"
#include "DataModel/FrequentFlyerTrx.h"
#include "Decode/DecodeGenerator.h"
#include "Decode/FrequentFlyerStatusGenerator.h"
#include "Server/TseServer.h"

namespace tse
{
static LoadableModuleRegister<Service, DecodeService>
_("libDecode.so");

static Logger
logger("atseintl.Decode.DecodeService");

DecodeService::DecodeService(const std::string& name, tse::TseServer& server)
  : Service(name, server), _config(Global::config())
{
}

bool
DecodeService::process(DecodeTrx& trx)
{
  LOG4CXX_INFO(logger, "Entering DecodeService::process");

  LocCode loc = trx.getLocation();

  DecodeGenerator dg(trx, loc);
  dg.generate();

  return true;
}

bool
DecodeService::process(FrequentFlyerTrx& trx)
{
  FrequentFlyerStatusGenerator generator(trx);
  generator.generateStatusList();

  return true;
}
}
