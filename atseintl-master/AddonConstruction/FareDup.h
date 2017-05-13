//-------------------------------------------------------------------
//
//  File:        FareDup.h
//  Created:     Jul 9, 2005
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Misc. util functions to remove constructed fare
//               duplicates
//
//  Copyright Sabre 2005
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

#include "AddonConstruction/ConstructionDefs.h"
#include "Common/CurrencyConversionFacade.h"
#include "DataModel/Fare.h"
#include "DBAccess/ConstructedFareInfo.h"

namespace tse
{
class ConstructedFare;
class ConstructedFareInfo;
class ConstructionJob;
class Diag257Collector;
class Fare;
class FareInfo;

class FareDup
{
public:
  // public types
  // ====== =====

  enum ComparisonFailureCode
  { EQUAL = 0,
    VENDORS_NE,
    CARRIERS_NE,
    MARKETS_NE,
    FARE_TARIFF_NE,
    GLOBAL_DIR_NE,
    FARE_CLASS_NE,
    CURRENCY_NE,
    RULE_NUMBER_NE,
    OWRT_NE,
    ROUTINGS_NE,
    FOOTNOTES_NE,
    DIRECTIONALITY_NE,
    ORIGADDON_NE,
    DESTADDON_NE};

  // main interface
  // ==== =========

  static ComparisonFailureCode
  isEqual(const ConstructedFareInfo& cfi1, const ConstructedFareInfo& cfi2);
  static ComparisonFailureCode
  isEqualWithAddons(const ConstructedFareInfo& cfi1, const ConstructedFareInfo& cfi2);

  static ComparisonFailureCode isEqual(const ConstructedFareInfo& cfi, const Fare& fare);
  static ComparisonFailureCode isEqualSimple(const ConstructedFareInfo& cfi, const Fare& fare);

  static void addFIToResponse(ConstructionJob& cj, ConstructedFareInfo* cfi);

protected:
  enum DupResolution
  { KEEP_GOING = 0,
    KEEP_EXISTED,
    REPLACE_EXISTED };

  typedef std::pair<TariffNumber, Footnote> FootnoteTariff;

  typedef std::vector<FootnoteTariff> FootnoteTariffVec;
  typedef FootnoteTariffVec::iterator FootnoteTariffVecI;
  typedef FootnoteTariffVec::const_iterator FootnoteTariffVecCI;

  // add to response
  // === == ========

  static DupResolution resolveFareClassPriority(const ConstructedFareInfo& cfi1,
                                                const ConstructedFareInfo& cfi2,
                                                Diag257Collector* diag257);

  static DupResolution singleVsDouble(const ConstructedFareInfo& cfi1,
                                      const ConstructedFareInfo& cfi2,
                                      Diag257Collector* diag257);

  static void checkHistoricalIntervals(DupResolution& dupResolution,
                                       const ConstructedFareInfo& cfi1,
                                       const ConstructedFareInfo& cfi2,
                                       const boost::gregorian::date& ticketingDate,
                                       const boost::gregorian::date& travelDate,
                                       Diag257Collector* diag257);

  static DupResolution keepLowestFare(const ConstructedFareInfo& cfi1,
                                      const ConstructedFareInfo& cfi2,
                                      Diag257Collector* diag257);

  // compare fareinfo
  // ======= ========

  static ComparisonFailureCode compareFareInfo(const FareInfo& fi1, const FareInfo& fi2);

  // routings
  // ========

  static ComparisonFailureCode
  compareRoutings(const ConstructedFare& cf1, const ConstructedFare& cf2);

  static ComparisonFailureCode
  compareRoutings(const ConstructedFareInfo& cfi1, const ConstructedFareInfo& cfi2);

  static ComparisonFailureCode compareRoutings(const ConstructedFareInfo& cfi, const Fare& fare);

  // routing utilities
  // ======= =========

  static void
  populateRoutingArray(const ConstructedFare& cf, RoutingNumber r[], unsigned int& rSize);

  static void
  populateRoutingArray(const ConstructedFareInfo& cfi, RoutingNumber r[], unsigned int& rSize);

  static ComparisonFailureCode compareRoutingArrays(const RoutingNumber r1[],
                                                    const unsigned int r1Size,
                                                    const RoutingNumber r2[],
                                                    const unsigned int r2Size);

  // footnotes
  // =========

  static ComparisonFailureCode
  compareFootnotes(const ConstructedFare& cf1, const ConstructedFare& cf2);

  static ComparisonFailureCode
  compareFootnotes(const ConstructedFareInfo& cfi1, const ConstructedFareInfo& cfi2);

  static ComparisonFailureCode compareFootnotes(const ConstructedFareInfo& cfi, const Fare& fare);

  // footnotes utilities
  // ========= =========

  static bool buildFootnoteTariffs(const ConstructedFare& cf, FootnoteTariffVec& fT);

  static bool buildFootnoteTariffs(const ConstructedFareInfo& cfi, FootnoteTariffVec& fT);

  static bool buildFootnoteTariffs(const FareInfo& fi, FootnoteTariffVec& fT);

  static bool compareFootnoteTariffs(const FootnoteTariffVec& fT1, const FootnoteTariffVec& fT2);

  static void addFootnoteTariff(const TariffNumber& tn,
                                const Footnote& ft,
                                const bool skipFT,
                                FootnoteTariffVec& ftVec);

  static void
  addFootnoteTariff(const FootnoteTariff& ft, const bool skipFT, FootnoteTariffVec& ftVec);

  static bool findFootnoteTariff(const FootnoteTariff& ft, const FootnoteTariffVec& ftVec);

private:
  // Placed here so they wont be called
  // ====== ==== == ==== ==== == ======

  FareDup(const FareDup& rhs);
  FareDup operator=(const FareDup& rhs);

}; // End of class FareDup

} // End namespace tse

