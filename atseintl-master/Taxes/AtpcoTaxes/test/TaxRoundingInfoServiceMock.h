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

#include "DataModel/Common/SafeEnums.h"
#include "DataModel/Common/Types.h"
#include "ServiceInterfaces/TaxCodeTextService.h"
#include "ServiceInterfaces/TaxRoundingInfoService.h"

namespace tax
{

class TaxRoundingInfoServiceMock : public TaxRoundingInfoService
{
public:
  TaxRoundingInfoServiceMock() :
    _dir(type::TaxRoundingDir::Blank),
    _unit(-1) {}

  virtual ~TaxRoundingInfoServiceMock() {}

  void getFareRoundingInfo(const type::CurrencyCode&,
                           type::MoneyAmount& unit,
                           type::TaxRoundingDir& dir) const override { setData(unit, dir); }

  void getNationRoundingInfo(const type::Nation& /*nation*/,
                             type::MoneyAmount& unit,
                             type::TaxRoundingDir& dir) const override { setData(unit, dir); }

  void getTrxRoundingInfo(const type::Nation& /*nation*/,
                          type::MoneyAmount& unit,
                          type::TaxRoundingDir& dir) const override { setData(unit, dir); }

  void doStandardRound(type::MoneyAmount& /*amount*/,
                       type::MoneyAmount& unit,
                       type::TaxRoundingDir& dir,
                       type::MoneyAmount /* currencyUnit */,
                       bool /* isOcFee */) const override { setData(unit, dir); }

  type::TaxRoundingDir& dir() { return _dir; }
  type::MoneyAmount& unit() { return _unit; }

private:
  void setData(type::MoneyAmount& unit, type::TaxRoundingDir& dir) const
  {
    unit = _unit;
    dir = _dir;
  }

  type::TaxRoundingDir _dir;
  type::MoneyAmount _unit;
};

}
