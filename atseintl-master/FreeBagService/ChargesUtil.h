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

#include "Common/TseCodeTypes.h"

namespace tse
{
class BaggageCharge;
class Itin;
class PricingTrx;
class SubCodeInfo;

namespace ChargesUtil
{
bool
isOnlineNonCodeShare(const Itin& itin);

std::vector<const SubCodeInfo*>
retrieveS5(const PricingTrx& trx, const Itin& itin, const CarrierCode cxr);

void
selectCheaper(BaggageCharge*& target, BaggageCharge& newCharge);
void
selectForPricing(BaggageCharge*& target, BaggageCharge& newCharge);
}
}
