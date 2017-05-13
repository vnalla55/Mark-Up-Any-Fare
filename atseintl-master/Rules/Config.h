// ----------------------------------------------------------------
//
//   Copyright Sabre 2014
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#pragma once

#include "Common/Config/ConfigurableValue.h"
#include "Common/ConfigList.h"

#include <cstdint>
#include <vector>

namespace tse
{
class ConfigurableCategories
{
public:
  ConfigurableCategories(const std::string key) : _value("RULECATEGORY", key) {}

  std::vector<uint16_t> read()
  {
    const auto& v = _value.getValue();
    return std::vector<uint16_t>(v.begin(), v.end());
  }

private:
  ConfigurableValue<ConfigVector<uint16_t>> _value;
};
}
