//-------------------------------------------------------------------
//  Copyright Sabre 2012
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

#include "Common/EmdInterlineAgreementInfoMapBuilder.h"
#include "Common/TseCodeTypes.h"
#include "FreeBagService/ChargesDataStrategy.h"

#include <boost/optional.hpp>
#include <log4cxx/helpers/objectptr.h>

namespace tse
{
class Diag852Collector;
class OCEmdDataProvider;
class PricingTrx;

enum class AncRequestPath : unsigned;

class AncillaryChargesDataStrategy : public ChargesDataStrategy
{
  friend class AncillaryChargesDataStrategyTest;

public:
  AncillaryChargesDataStrategy(PricingTrx& trx, boost::optional<const EmdInterlineAgreementInfoMap&> emdInfoMap = boost::optional<const EmdInterlineAgreementInfoMap&>());
  virtual ~AncillaryChargesDataStrategy();

  virtual void processBaggageTravel(BaggageTravel* baggageTravel,
                                    const BaggageTravelInfo& bagInfo,
                                    const CheckedPoint& furthestCheckedPoint,
                                    BaggageTripType btt,
                                    Diag852Collector* dc) const override;

protected:
  virtual void retrieveS5Records(const BaggageTravel* baggageTravel,
                                 std::vector<const SubCodeInfo*>& subCodes) const;
  virtual void matchS7s(BaggageTravel& baggageTravel,
                        const SubCodeInfo* s5,
                        const CheckedPoint& furthestCheckedPoint,
                        bool isUsDot,
                        Diag852Collector* dc,
                        ChargeVector& charges) const override;
  virtual void findLowestCharges(const std::vector<BaggageTravel*>& /*baggageTravels*/,
                                 uint32_t /*bgIndex*/,
                                 ChargeVector& /*charges*/,
                                 Diag852Collector* /*dc*/) const;
  virtual CarrierCode getS5CarrierCode(const BaggageTravel* baggageTravel) const;

  bool validateEmd(BaggageTravel* baggageTravel, const BaggageTravelInfo& bagInfo, bool isUsDot, Diag852Collector* dc) const;
  void collectEmdData(const BaggageTravel* baggageTravel, const BaggageTravelInfo& bagInfo, OCEmdDataProvider& dataForEmdValidation, bool isUsDot,
                      Diag852Collector* dc) const;
  bool shouldvalidateEmd(AncRequestPath rp, const BaggageTravel* baggageTravel) const;
  void propagateEmdValidationResult(bool emdValidationResult, AncRequestPath rp, BaggageTravel* baggageTravel, const BaggageTravelInfo& bagInfo, Diag852Collector* dc) const;
  virtual std::string getEmdValidatingCarrier() const;
  void displayEmdDiagnostic(const BaggageTravelInfo& bagInfo, const BaggageTravel* baggageTravel, const OCEmdDataProvider& dataForEmdValidation, Diag852Collector* dc) const;
  virtual bool shouldDisplayEmdDaignostic(const Diag852Collector* dc) const;

private:
  void sort(std::vector<const SubCodeInfo*>& subCodes) const;
  bool isEmdRecord(const BaggageCharge* baggageCharge) const;
  boost::optional<const EmdInterlineAgreementInfoMap&> _carrierEmdInfoMap;
};
} // tse
