//-------------------------------------------------------------------
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
//-------------------------------------------------------------------

#pragma once

#include "Common/OcTypes.h"
#include "ServiceFees/OCFees.h"

#include <boost/logic/tribool.hpp>

namespace tse
{
class BaggageAllowanceValidator;
class BaggageTravel;
class PricingTrx;
class RegularBtaSubvalidator;
class RegularNonBtaFareSubvalidator;

class BaggageRevalidator
{
public:
  BaggageRevalidator(const PricingTrx& trx, BaggageTravel& bt, const Ts2ss& ts2ss);

  void onFaresUpdated(FarePath* fp);
  void setDeferTargetCxr(const CarrierCode& cxr);

  boost::logic::tribool revalidateS7(const OCFees& savedItem, const bool fcLevel = false);

private:
  BaggageAllowanceValidator* _s7Validator = nullptr;
  RegularBtaSubvalidator* _btaSubvalidator = nullptr;
  RegularNonBtaFareSubvalidator* _nonBtaSubvalidator = nullptr;
  OCFees _dummyOc;
};
}
