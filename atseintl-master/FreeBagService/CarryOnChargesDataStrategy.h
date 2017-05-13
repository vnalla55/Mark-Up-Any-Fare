//-------------------------------------------------------------------
//  Copyright Sabre 2013
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

#include "FreeBagService/AncillaryChargesDataStrategy.h"

#include <boost/optional.hpp>


namespace tse
{
class CarryOnChargesDataStrategy : public AncillaryChargesDataStrategy
{
  friend class CarryOnChargesDataStrategyTest;

public:
  CarryOnChargesDataStrategy(PricingTrx& trx, boost::optional<const EmdInterlineAgreementInfoMap&> emdInfoMap = boost::optional<const EmdInterlineAgreementInfoMap&>());
  virtual ~CarryOnChargesDataStrategy();

  virtual void processBaggageTravel(BaggageTravel* baggageTravel,
                                    const BaggageTravelInfo& bagInfo,
                                    const CheckedPoint& furthestCheckedPoint,
                                    BaggageTripType btt,
                                    Diag852Collector* dc) const override;

private:
  virtual void retrieveS5Records(const BaggageTravel* baggageTravel,
                                 std::vector<const SubCodeInfo*>& subCodes) const override;
  virtual bool shouldDisplayChargesDiagnostic(BaggageTravel* baggageTravel,
                                              const BaggageTravelInfo& bagInfo,
                                              const Diag852Collector* dc) const override;
  bool shouldProcessCharges(const OptionalServicesInfo* s7) const;
  virtual void printS7ProcessingContext(BaggageTravel* baggageTravel,
                                        const BaggageTravelInfo& bagInfo,
                                        const SubCodeInfo* s5,
                                        bool isUsDot,
                                        Diag852Collector* dc,
                                        bool /*defer*/,
                                        bool /*isCarrierOverride*/) const override;
  virtual CarrierCode getS5CarrierCode(const BaggageTravel* baggageTravel) const override;
  virtual void matchS7s(BaggageTravel& baggageTravel,
                        const SubCodeInfo* s5,
                        const CheckedPoint& furthestCheckedPoint,
                        bool isUsDot,
                        Diag852Collector* dc,
                        ChargeVector& charges) const override;
  bool shouldDisplayEmdDaignostic(const Diag852Collector* dc) const override;
};
} // tse
