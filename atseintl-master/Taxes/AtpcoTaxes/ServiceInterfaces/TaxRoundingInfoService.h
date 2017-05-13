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
#include "DataModel/Services/TaxRounding.h"

namespace tax
{

class TaxRoundingInfoService
{
public:
  TaxRoundingInfoService() {}
  virtual ~TaxRoundingInfoService() {}

  virtual void getFareRoundingInfo(const type::CurrencyCode& currency,
                                   type::MoneyAmount& unit,
                                   type::TaxRoundingDir& dir) const = 0;

  virtual void getNationRoundingInfo(const type::Nation& nation,
                                     type::MoneyAmount& unit,
                                     type::TaxRoundingDir& dir) const = 0;

  virtual void getTrxRoundingInfo(const type::Nation& nation,
                                  type::MoneyAmount& unit,
                                  type::TaxRoundingDir& dir) const = 0;

  virtual void doStandardRound(type::MoneyAmount& amount,
                               type::MoneyAmount& unit,
                               type::TaxRoundingDir& dir,
                               type::MoneyAmount currencyUnit = -1,
                               bool isOcFee = false) const = 0;
};

} // namespace tax
