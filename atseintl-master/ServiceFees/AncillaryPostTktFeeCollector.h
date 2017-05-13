//-------------------------------------------------------------------
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
#pragma once
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "ServiceFees/AncillaryWpDisplayFeeCollector.h"


namespace tse
{
class PricingTrx;
class ServiceFeesGroup;
class TravelSeg;

class AncillaryPostTktFeeCollector : public AncillaryWpDisplayFeeCollector
{
  friend class AncillaryPostTktFeeCollectorTest;

public:
  AncillaryPostTktFeeCollector(PricingTrx& trx);
  virtual ~AncillaryPostTktFeeCollector();

  AncillaryPostTktFeeCollector(const AncillaryPostTktFeeCollector& rhs)
    : AncillaryWpDisplayFeeCollector(rhs)
  {
  }

private:
  AncillaryPostTktFeeCollector& operator=(const AncillaryPostTktFeeCollector& rhs);

  void processSubCodes(ServiceFeesGroup& srvFeesGrp,
                       const CarrierCode& candCarrier,
                       int unitNo,
                       bool isOneCarrier) const override;
  void
  multiThreadSliceAndDice(std::vector<std::pair<std::vector<TravelSeg*>::const_iterator,
                                                std::vector<TravelSeg*>::const_iterator>>& routes,
                          int unitNo) override;
};

} // tse namespace

