//----------------------------------------------------------------------------
//
// Copyright Sabre 2007
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/BaggageTripType.h"
#include "Common/EmdInterlineAgreementInfoMapBuilder.h"
#include "DataModel/BaggageTravel.h"
#include "Service/Service.h"

#include <map>

namespace tse
{

class AncillaryPricingTrx;
class BaggageItinAnalyzer;
class BaggageTrx;
class FreeBagService;
class TseServer;

class FreeBagService final : public Service
{
  friend class FreeBagServiceTest;
  friend class DeleteList;

public:
  FreeBagService(const std::string& sname, TseServer& tseServer) : Service(sname, tseServer) {}

  bool initialize(int argc = 0, char* argv[] = nullptr) override;

  virtual bool process(PricingTrx& trx) override;
  virtual bool process(AltPricingTrx& trx) override;
  virtual bool process(ExchangePricingTrx& trx) override;
  virtual bool process(NoPNRPricingTrx& trx) override;
  virtual bool process(RexPricingTrx& trx) override;
  virtual bool process(RexExchangeTrx& trx) override;
  virtual bool process(BaggageTrx& trx) override;
  virtual bool process(AncillaryPricingTrx& trx) override;

  // TEMP method used for reference only - should never go to production
  bool processBagInPqBruteForce(PricingTrx& trx, FarePath& fp);

  virtual uint32_t getActiveThreads() override;

private:
  void processIata302Baggage(PricingTrx& trx, Itin* itn = nullptr);
  void processIata302BaggageMip(PricingTrx& trx);
  void processIata302BaggageWebService(AncillaryPricingTrx& trx);

  bool checkFarePath(const PricingTrx& trx) const;

  void adjustTimeOut(AncillaryPricingTrx& trx) const;

  void processCarryOnBaggage(PricingTrx& trx,
                             const BaggageItinAnalyzer& analyzer,
                             const CheckedPoint& furthestCheckedPoint,
                             BaggageTripType btt,
                             const Itin* itin);

  void processCarryOnBaggage(AncillaryPricingTrx& trx,
                             const BaggageItinAnalyzer& analyzer,
                             const CheckedPoint& furthestCheckedPoint,
                             BaggageTripType btt,
                             const Itin* itin,
                             const EmdInterlineAgreementInfoMap& emdInfoMap);

  void processCarryOnDisclosure(PricingTrx& trx,
                             BaggageTripType btt,
                             const Itin* itin);



  void processEmbargoes(PricingTrx& trx,
                        const BaggageItinAnalyzer& analyzer,
                        const CheckedPoint& furthestCheckedPoint,
                        BaggageTripType btt,
                        const Itin* itin);

  void populateEmdInfoMap(AncillaryPricingTrx& trx, EmdInterlineAgreementInfoMap& emdMap) const;
};
} // tse
