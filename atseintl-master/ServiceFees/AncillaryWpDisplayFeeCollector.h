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
#include "ServiceFees/OptionalFeeCollector.h"


namespace tse
{
class PricingTrx;
class AncillaryPricingTrx;
class ServiceFeesGroup;
class TravelSeg;

class AncillaryWpDisplayFeeCollector : public OptionalFeeCollector
{
  friend class AncillaryWpDisplayFeeCollectorTest;

public:
  AncillaryWpDisplayFeeCollector(PricingTrx& trx);
  virtual ~AncillaryWpDisplayFeeCollector();

  void process() override;
  AncillaryWpDisplayFeeCollector(const AncillaryWpDisplayFeeCollector& rhs)
    : OptionalFeeCollector(rhs)
  {
  }

private:
  AncillaryWpDisplayFeeCollector& operator=(const AncillaryWpDisplayFeeCollector& rhs);
  void checkDiag878And879() const;

  void processSubCodes(ServiceFeesGroup& srvFeesGrp,
                       const CarrierCode& candCarrier,
                       int unitNo,
                       bool isOneCarrier) const override;
  bool samePaxType(AncillaryPricingTrx& ancTrx);
  void
  multiThreadSliceAndDice(std::vector<std::pair<std::vector<TravelSeg*>::const_iterator,
                                                std::vector<TravelSeg*>::const_iterator>>& routes,
                          int unitNo) override;
};

} // tse namespace

