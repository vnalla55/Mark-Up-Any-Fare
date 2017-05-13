//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------

#pragma once

#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "ItinAnalyzer/ItinAnalyzerServiceWrapper.h"

namespace tse
{
class ExchangePricingTrx;
class ItinAnalyzerService;

class ExcItinAnalyzerServiceWrapper : public ItinAnalyzerServiceWrapper
{
  friend class ExcItinAnalyzerServiceWrapperTest;

public:
  ExcItinAnalyzerServiceWrapper(ItinAnalyzerService& itinAnalyzer);

private:
  void processExchangeItin(ExchangePricingTrx& trx) override {}
  void determineChanges(ExchangePricingTrx& trx) override {}
  bool selectTicketingCarrier(PricingTrx& trx) override { return true; }
  void setRetransits(PricingTrx& trx) override {}
  void setOpenSegFlag(PricingTrx& trx) override {}
  void setTripCharacteristics(PricingTrx& trx) override {}
  void checkJourneyActivation(PricingTrx& trx) override
  {
    trx.getOptions()->journeyActivatedForPricing() = false;
  }
  void checkSoloActivation(PricingTrx& trx) override
  {
    trx.getOptions()->soloActiveForPricing() = false;
  }
  void setATAEContent(PricingTrx& trx) override {}
  void setATAEAvailContent(PricingTrx& trx) override {}
  void setATAESchedContent(PricingTrx& trx) override {}
  void setItinRounding(PricingTrx& trx) override {}
  void setInfoForSimilarItins(PricingTrx& trx) override {}
  bool buildFareMarket(PricingTrx& trx, Itin& itin) override;
}; // End class ExcItinAnalyzerServiceWrapper

} // End namespace tse



