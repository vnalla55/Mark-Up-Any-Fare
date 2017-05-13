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

class CombinationsSubCat109 : public CombinationsSubCat
{
  friend class CombinationsSubCat109Test;

public:
  CombinationsSubCat109(PricingTrx& trx,
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
                         109,
                         Diagnostic639)
  {
  }

  bool match();

protected:
  using ojrIt = std::vector<OpenJawRestriction*>::const_iterator;

  static constexpr Indicator OJ_BETWEEN_POINTS = '1';
  static constexpr Indicator OJ_BETWEEN_AND = '2';
  static constexpr Indicator OJ_SAME_COUNTRY = '3';
  static constexpr Indicator OJ_NEGATIVE_LOC = 'X';

  enum SetNumber
  {
    NO_MATCH_SET,
    FIRST_SET,
    SECOND_SET
  };

  void matchOriginPart();
  void matchDestinationPart();
  bool getDestinationOpenSegment(const FareMarket*& ojFrom, const FareMarket*& ojTo) const;

  void matchGenericPart(const Loc* ojFrom,
                        const Loc* ojTo,
                        const LocCode& ojFromCity,
                        const LocCode& ojToCity);

  SetNumber passOJSetRestriction(const Loc& ojFrom, const Loc& ojTo);

  bool matchPositiveSet(const SetNumber& setNumber, const Loc& ojFrom, const Loc& ojTo) const;
  bool
  matchPositiveBetweenSet(const SetNumber& setNumber, const Loc& ojFrom, const Loc& ojTo) const;

  bool matchNegativeSequence(const Loc& ojFrom,
                             const Loc& ojTo,
                             Indicator setApplInd,
                             const LocKey& setLoc1,
                             const LocKey& setLoc2) const;
  bool matchPositiveSequence(const Loc& ojFrom,
                             const Loc& ojTo,
                             Indicator setApplInd,
                             const LocKey& setLoc1,
                             const LocKey& setLoc2) const;

  bool sameCountrySegment(const Loc& ojFrom, const Loc& ojTo) const;
  virtual bool
  isBetween(const Loc& ojFrom, const Loc& ojTo, const LocKey& setLoc1, const LocKey& setLoc2) const;
  virtual bool isInLoc(const Loc& loc, const LocKey& locKey) const;
  bool isInLoc(const Loc& loc, std::vector<const LocKey*>& locVec) const;

  void displayDiag();

private:
  const std::vector<OpenJawRestriction*>* _openJawSetVector = nullptr;
};
}

