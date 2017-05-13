//  Copyright Sabre 2011
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
#include "ServiceFees/AncillaryPostTktFeeCollector.h"

#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "ServiceFees/AncillaryPostTktValidator.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/ServiceFeesGroup.h"

namespace tse
{
using namespace std;

static Logger
logger("atseintl.ServiceFees.AncillaryPostTktFeeCollector");

AncillaryPostTktFeeCollector::AncillaryPostTktFeeCollector(PricingTrx& trx)
  : AncillaryWpDisplayFeeCollector(trx)
{
}

AncillaryPostTktFeeCollector::~AncillaryPostTktFeeCollector() {}

void
AncillaryPostTktFeeCollector::processSubCodes(ServiceFeesGroup& srvFeesGrp,
                                              const CarrierCode& candCarrier,
                                              int unitNo,
                                              bool isOneCarrier) const
{
  OcValidationContext ctx(_trx, *_farePath->itin(), *_farePath->paxType(), _farePath);
  AncillaryPostTktValidator ancillaryPostTktValidator(
      ctx,
      _beginsOfUOT[unitNo],
      _beginsOfUOT[unitNo + 1],
      _beginsOfLargestUOT[2],
      _ts2ss,
      _isInternational.getValForKey(_farePath->itin()),
      isOneCarrier,
      getOperatingMarketingInd(),
      diag877());
  OptionalFeeCollector::processSubCodes(
      ancillaryPostTktValidator, srvFeesGrp, candCarrier, unitNo, isOneCarrier);
}

void
AncillaryPostTktFeeCollector::multiThreadSliceAndDice(
    std::vector<std::pair<std::vector<TravelSeg*>::const_iterator,
                          std::vector<TravelSeg*>::const_iterator> >& routes,
    int unitNo)
{
  multiThreadSliceAndDiceImpl<AncillaryPostTktFeeCollector>(routes, unitNo);
}

} // namespace
