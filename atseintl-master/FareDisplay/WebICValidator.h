//-------------------------------------------------------------------
//
//  File:        WebICValidator.h
//  Created:     June 10, 2005
//  Authors:     Mike Carroll
//
//  Updates:
//
//  Copyright Sabre 2005
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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxType.h"
#include "DBAccess/FareDisplayWeb.h"
#include "FareDisplay/Validator.h"


#include <map>
#include <vector>

const int16_t NOT_USEABLE = -1;

namespace tse
{

class WebICValidator : public Validator
{
  friend class WebICValidatorTest;

public:
  //--------------------------------------------------------------------------
  // @function WebICValidator::validate
  //
  // Description: Validate a paxTypeFare
  //
  // @param trx - a valid transaction reference
  // @param paxTypeFare - pax type fare to be validated
  //
  // @return true if is valid, false otherwise
  //--------------------------------------------------------------------------
  bool validate(const PaxTypeFare& paxTypeFare) override;
  bool initialize(const FareDisplayTrx& trx) override;

private:

  virtual Validator::ValidatorType name() const override { return Validator::WEB_VALIDATOR; }

  virtual Restriction restriction(const PaxTypeFare& fare) const override { return Validator::AND; }
  virtual const std::vector<FareDisplayWeb*>& getFareDisplayWeb(const FareDisplayTrx& trx,
                                                                const Indicator& dispInd,
                                                                const VendorCode& vendor,
                                                                const CarrierCode& carrier,
                                                                const TariffNumber& ruleTariff,
                                                                const RuleNumber& rule,
                                                                const PaxTypeCode& paxTypeCode);
};

} // namespace tse

