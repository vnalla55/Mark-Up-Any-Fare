// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

#include "DataModel/Common/Types.h"

namespace tax
{

// This type corresponds to ATPCO entity Tariff: Table 167 bytes 48-50
// According to ATPCO the allowed values are:
// "001" to "999", or two special values "PRI" or "PUB" or a blank field.
//
// However, in order to handle the V2 data structure which holds and changes
// the number and the short text independently, we also store it in two fields.
// Practically, we expect the following predicate to hold:
//
// bool valid_value(Tariff t)
// {
//   return (t.asCode().empty() && t.asNumber() >= 0 && t.asNumber() <= 999)
//       || (t.asCode().empty() && t.asNumber() == -1)
//       || (t.asCode() == "PUB" && t.asNumber() == -1)
//       || (t.asCode() == "PRI" && t.asNumber() == -1)
// }
class Tariff
{
  type::FareTariff _asNumber {-1};
  type::FareTariffInd _asCode {UninitializedCode};

public:
  Tariff() = default;
  Tariff(type::FareTariff num, type::FareTariffInd code) : _asNumber(num), _asCode(code) {}

  type::FareTariff asNumber() const { return _asNumber; }
  type::FareTariff& asMutableNumber() { return _asNumber; }

  type::FareTariffInd asCode() const { return _asCode; }
  type::FareTariffInd& asMutableCode() { return _asCode; }

  friend bool matches(Tariff ref, Tariff val);
};

} // namespace tax

