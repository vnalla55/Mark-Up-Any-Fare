//----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Util/FlatSet.h"

#include <vector>

namespace tse
{
class Trx;

class ConfigurableValueAdapterBase;

struct ConfigurableValuesAdapterPool
{
  static FlatSet<ConfigurableValueAdapterBase*>& getAdapterPool();
  static void registerCfgValue(ConfigurableValueAdapterBase& adapter);
  static void unregisterCfgValue(ConfigurableValueAdapterBase& adapter);
  static void collectConfigMsg(const Trx& trx, std::vector<std::string>& messages);
};

} // ns tse
