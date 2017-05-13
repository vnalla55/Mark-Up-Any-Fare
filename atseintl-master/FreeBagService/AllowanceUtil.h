// ----------------------------------------------------------------
//
//   Copyright Sabre 2015
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

#include "DBAccess/OptionalServicesInfo.h"

namespace tse
{
class AirSeg;
class BaggageTravel;
class BaggageTripType;
class PricingTrx;
class SubCodeInfo;
class TravelSeg;

namespace AllowanceUtil
{
const Indicator DEFER_BAGGAGE_RULES_FOR_MC = 'D';
const Indicator DEFER_BAGGAGE_RULES_FOR_OPC = 'O';

// Predicate for checking if marketing carrier on the segment is contained
// in USDOTCARRIER or CTACARRIER db table.
struct DotTableChecker
{
  DotTableChecker(PricingTrx& trx, const BaggageTripType& btt) : _trx(trx), _btt(btt) {}
  bool operator()(const TravelSeg* travelSeg) const;

private:
  PricingTrx& _trx;
  const BaggageTripType& _btt;
};

inline bool
isDefer(const OptionalServicesInfo& s7)
{
  return s7.notAvailNoChargeInd() == DEFER_BAGGAGE_RULES_FOR_MC ||
         s7.notAvailNoChargeInd() == DEFER_BAGGAGE_RULES_FOR_OPC;
}

CarrierCode getAllowanceCxrNonUsDot(const PricingTrx& trx, const AirSeg& as);
CarrierCode getDeferTargetCxrNonUsDot(const PricingTrx& trx, const AirSeg& as);
CarrierCode getDeferTargetCxr(const BaggageTravel& bt);
const SubCodeInfo* retrieveS5(const PricingTrx& trx, const CarrierCode cxr);
}
}
