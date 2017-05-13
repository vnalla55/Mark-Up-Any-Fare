// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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
#include "Common/MoneyUtil.h"

namespace tax
{

enum IncludedTaxType { FLAT = 0, PERCENTAGE };

struct RoundingInfo
{
  RoundingInfo() : unit(type::MoneyAmount(1, 100)),
      dir(type::TaxRoundingDir::Nearest), standardRounding(false) {}
  RoundingInfo(const type::MoneyAmount& newUnit,
               const type::TaxRoundingDir& newDir,
               bool newStandardRounding)
      : unit(newUnit), dir(newDir), standardRounding(newStandardRounding) {}

  type::MoneyAmount unit;
  type::TaxRoundingDir dir;
  bool standardRounding;
};

class TaxAndRounding
{
public:
  TaxAndRounding() {}
  TaxAndRounding(const IncludedTaxType& includedTaxType,
                 const type::MoneyAmount& value,
                 const RoundingInfo& roundingInfo,
                 bool doTruncation)
    : _includedTaxType(includedTaxType),
      _value(value),
      _roundingInfo(roundingInfo),
      _doTruncation(doTruncation){}

  type::MoneyAmount getRoundedPercentageTax(const type::MoneyAmount& amount,
                                            const type::MoneyAmount& totalTaxPercentage);
  type::MoneyAmount getRoundedFlatTax();
  type::MoneyAmount getValue() const { return _value; }
  void setValue(const type::MoneyAmount& newValue) { _value = newValue; }
  IncludedTaxType getIncludedTaxType() const { return _includedTaxType; }
  void setIncludedTaxType(const IncludedTaxType& type) { _includedTaxType = type; }

  void setTruncation(bool doTruncation) { _doTruncation = doTruncation; }

  void setRoundingInfo(const RoundingInfo& roundingInfo) { _roundingInfo = roundingInfo; }
  RoundingInfo& getRoundingInfo() { return _roundingInfo; }
  const RoundingInfo& getRoundingInfo() const { return _roundingInfo; }

  type::MoneyAmount& getUnit() { return _roundingInfo.unit; }
  type::TaxRoundingDir& getDir() { return _roundingInfo.dir; }
  bool isStandardRounding() const { return _roundingInfo.standardRounding; }

private:
  type::MoneyAmount
  doRound(const type::MoneyAmount& amount,
          const type::MoneyAmount& unit,
          const type::TaxRoundingDir& dir) const;

  IncludedTaxType _includedTaxType{FLAT};
  type::MoneyAmount _value{0, 1};
  RoundingInfo _roundingInfo{};
  bool _doTruncation{false};
};

} // end of tax namespace
