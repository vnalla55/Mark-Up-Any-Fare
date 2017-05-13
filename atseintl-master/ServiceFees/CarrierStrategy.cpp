//-------------------------------------------------------------------
//  Copyright Sabre 2009
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
#include "ServiceFees/CarrierStrategy.h"

#include "ServiceFees/OptionalFeeCollector.h"

namespace tse
{
using namespace std;

bool
CarrierStrategy::isIndInd(const Indicator& indCrxInd) const
{
  return indCrxInd == OptionalFeeCollector::S5_INDCRXIND_INDUSTRY;
}

bool
MarketingCarrierStrategy::checkIndCrxInd(const Indicator& indCrxInd) const
{
  return isIndInd(indCrxInd);
}

bool
OperatingCarrierStrategy::checkIndCrxInd(const Indicator& indCrxInd) const
{
  return true;
}

bool
MultipleOperatingCarrierStrategy::checkIndCrxInd(const Indicator& indCrxInd) const
{
  return indCrxInd != OptionalFeeCollector::S5_INDCRXIND_CARRIER;
}
}
