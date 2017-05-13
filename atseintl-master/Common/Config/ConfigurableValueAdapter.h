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

#include "Common/Config/ConfigurableValueAdapterBase.h"
#include "Common/Config/ConfigurableValueUtil.h"

#include <type_traits>

namespace tse
{
class Trx;
class ConfigMan;

template <typename T, typename P, typename R>
class ConfigurableValueAdapter : public ConfigurableValueAdapterBase
{
  friend class ConfigurableValueAdapterTest;
public:
  using ValueType = typename T::ValueType;

  ConfigurableValueAdapter(const std::string& section,
                           const std::string& option,
                           const ValueType& value)
    : _cv(section, option, value)  {  }

  bool isValid(const Trx* trx)
  {
    trx = ConfigurableValueUtil::getTrx(trx);
    ConfigurableValueUtil::checkTrx(trx);
    R referenceValue;
    return isValid(trx, referenceValue(trx));
  }

  bool isValid(const Trx* trx, const ValueType& valueToCheck)
  {
    return _predicate(valueToCheck, _cv.getValue(trx));
  }

  ValueType getValue(const Trx* trx) { return _cv.getValue(trx); }

  std::string getConfigMsg(const Trx* trx, const bool dynamicOnly) override final
  {
    std::string configMsg(_cv.getConfigMsg(trx, dynamicOnly));
    if (!configMsg.empty())
    {
      configMsg += '/';
      configMsg += (isValid(trx) ? "APPLIED" : "NOT APPLIED");
    }

    return configMsg;
  }

  bool set(const ConfigMan* config)
  {
    return _cv.set(config);
  }

  void set(const ValueType& value)
  {
    _cv.set(value);
  }

private:
  T _cv;
  P _predicate;
};
}
