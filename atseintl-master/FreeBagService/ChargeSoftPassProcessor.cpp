//-------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "FreeBagService/ChargeSoftPassProcessor.h"

#include "Common/FreeBaggageUtil.h"
#include "DataModel/BaggagePolicy.h"
#include "DataModel/PrecalcBaggageData.h"
#include "DataModel/PricingTrx.h"
#include "FreeBagService/BaggageOcValidationAdapter.h"
#include "FreeBagService/BagValidationOpt.h"
#include "FreeBagService/ChargesUtil.h"

#include <algorithm>

#include <boost/container/flat_map.hpp>

namespace tse
{
using FreeBagsPerCxr = boost::container::flat_map<CarrierCode, uint32_t>;

inline FreeBagsPerCxr
calcFreeBags(const PrecalcBaggage::BagTravelData& bagTravelData)
{
  FreeBagsPerCxr freeBagsPerCxr;

  for (const auto& cxrsAllowance : bagTravelData.allowance)
  {
    const PrecalcBaggage::CxrPair& cxrs = cxrsAllowance.first;
    const PrecalcBaggage::AllowanceRecords& allowance = cxrsAllowance.second;

    if (!allowance.s5Found)
      continue;

    const auto insertResult = freeBagsPerCxr.emplace(cxrs.allowanceCxr(), 0);
    uint32_t& freeBags = insertResult.first->second;

    if (insertResult.second && !allowance.s7s.empty())
      freeBags = MAX_BAG_PIECES;

    for (const OCFees* const oc : allowance.s7s)
      freeBags = std::min(freeBags, FreeBaggageUtil::calcFirstChargedPiece(oc));
  }

  return freeBagsPerCxr;
}

void
ChargeSoftPassProcessor::matchCharges(const BagValidationOpt& optPrototype)
{
  BagValidationOpt opt(optPrototype);

  BaggageTravel& bt = opt._bt;
  const FreeBagsPerCxr freeBagsPerCxr = calcFreeBags(_bagTravelData);
  const uint32_t requestedBags = bt._trx->getBaggagePolicy().getRequestedBagPieces();

  for (const auto& cxrFreeBags : freeBagsPerCxr)
  {
    if (cxrFreeBags.second >= requestedBags)
      continue;

    bt._carrierTravelSeg = nullptr; // shouldn't change anything in charges
    bt._allowanceCxr = cxrFreeBags.first;
    PrecalcBaggage::ChargeRecords& charges = _bagTravelData.charges[bt._allowanceCxr];

    for (const SubCodeInfo* s5 : ChargesUtil::retrieveS5(*bt._trx, *bt.itin(), bt._allowanceCxr))
    {
      const auto& s7s =
          BaggageOcValidationAdapter::matchS7ChargeSkipFareChecks(opt, *s5, cxrFreeBags.second);

      if (!s7s.empty())
        charges.s7s[s5] = s7s;
    }
  }
}
} // ns
