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

#include "Common/BaggageTripType.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/BaggageTravel.h"

#include <vector>

namespace tse
{
class PricingTrx;
class SubCodeInfo;
class Diag852Collector;
class BaggageTravelInfo;

class DataStrategyBase
{
  friend class DataStrategyBaseTest;

public:
  DataStrategyBase(PricingTrx& trx);
  virtual ~DataStrategyBase();

  virtual void processBaggageTravel(BaggageTravel* baggageTravel,
                                    const BaggageTravelInfo& bagInfo,
                                    const CheckedPoint& furthestCheckedPoint,
                                    BaggageTripType btt,
                                    Diag852Collector* dc) const = 0;

protected:
  void printS7ProcessingContext(BaggageTravel* baggageTravel,
                                const BaggageTravelInfo& bagInfo,
                                const SubCodeInfo* s5,
                                bool isUsDot,
                                Diag852Collector* dc,
                                bool defer = false,
                                bool isCarrierOverride = false) const;

  const std::vector<SubCodeInfo*>&
  retrieveS5Records(const VendorCode& vendor, const CarrierCode& carrier) const;

  bool checkFareLineAndCheckedPortion(const BaggageTravel* baggageTravel,
                                      const BaggageTravelInfo& bagInfo,
                                      const Diag852Collector* dc) const;

  bool isAllowanceCarrierOverridden() const;
  bool isChargesCarrierOverridden() const;
  CarrierCode getAllowanceCarrierOverridden() const;
  CarrierCode getChargesCarrierOverridden() const;

  PricingTrx& _trx;
};
} // tse

