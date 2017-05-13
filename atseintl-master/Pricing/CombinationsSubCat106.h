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

#include "Pricing/CombinationsSubCat.h"

namespace tse
{

class CombinationsSubCat106 : public CombinationsSubCat
{
  friend class CombinationsSubCat106Test;

public:
  CombinationsSubCat106(PricingTrx& trx,
                        DiagCollector& diag,
                        const VendorCode& vendor,
                        const uint32_t itemNo,
                        const PricingUnit& prU,
                        const FareUsage& fu,
                        Combinations::ValidationFareComponents& components,
                        bool& negativeApplication)
    : CombinationsSubCat(trx,
                         diag,
                         vendor,
                         itemNo,
                         prU,
                         fu,
                         components,
                         negativeApplication,
                         106,
                         Diagnostic636)
  {
  }

  bool match();

private:
  virtual bool isInLoc(const LocCode& validatingLoc,
                       const Indicator restrictionLocType,
                       const LocCode& restrictionLoc,
                       const VendorCode& validatingVendor) const;
  void matchGEO(char& ret, const CarrierCombination& cxrComb, const PaxTypeFare& targetFare,
                const CombinabilityRuleInfo& targetCat10);
  void displayDiag(const CarrierCombination& cxrComb, const LocCode& orig, const LocCode& dest);
};
}

