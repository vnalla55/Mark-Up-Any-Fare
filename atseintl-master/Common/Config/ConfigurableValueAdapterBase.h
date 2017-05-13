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

#include "Common/Config/ConfigurableValuesAdapterPool.h"

namespace tse
{
class Trx;

class ConfigurableValueAdapterBase
{
public:
  ConfigurableValueAdapterBase()
  {
    ConfigurableValuesAdapterPool::registerCfgValue(*this);
  }

  virtual ~ConfigurableValueAdapterBase()
  {
    ConfigurableValuesAdapterPool::unregisterCfgValue(*this);
  }

  virtual std::string getConfigMsg(const Trx* trx, const bool dynamicOnly) = 0;
};
}
