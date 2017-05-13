//----------------------------------------------------------------------------
//  Copyright Sabre 2003
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

#include "MinFares/MinimumFare.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Diagnostic/Diagnostic.h"
#include "Diagnostic/DiagManager.h"

#include <iosfwd>
#include <set>
#include <sstream>
#include <vector>

/**
 *  @class COPMinimumFare
 *
 * Description: Class for the Country of Payment (COP) Check.
 *              If the check succeeds then apply the COP processing logic.
 *              Determines if the travel is to or via the qualifying country
 *              in which the travel payment is made.
 *
 *              The COP table (see COP.h) contains the countries that
 *              require a COP check and the carriers that apply COP check.
 */

namespace tse
{
class CopMinimum;
class FarePath;
class PricingTrx;
class PricingUnit;
class RepricingTrx;
class TravelSeg;
class CopCarrier;

class COPMinimumFare : public MinimumFare
{
  friend class COPMinimumFareTest; // CPP UNIT tests

public:
  COPMinimumFare(PricingTrx& trx, FarePath& farePath);
  virtual ~COPMinimumFare();

  MoneyAmount process();

private:
  enum TravelAppl // COP Travel Application.
  {
    TO = 'T', // COP point must be fare break point.
    VIA = 'V' // COP point can be any point.
  };

  enum PuTypeAppl // Indicates if COP applies to certain PU type
  {
    NORMAL = 'N', // Normal PU only.
    SPECIAL = 'S', // Special PU only.
    EITHER_TYPE = 'E' // Either Normal or Special PU.
  };

  enum PuTripTypeAppl // Pricing Unit Trip Type.
  {
    RT = 'R', // Round Trip only.
    CT = 'C', // Circle Trip only.
    EITHER_TRIP_TYPE = 'E' // Either Trip Type.
  };

  enum CarrierParticipationAppl // COP Carrier Ticketing/Participation
  // Application.
  {
    TKTG_CARRIER = 'T', // COP Carrier must be the Ticketing Carrier.
    PARTICIPATE_CARRIER = 'P', // COP Carrier must be a Participating
    // Carrier in the PU.
    EITHER_CARRIER = 'E', // COP Carrier may be either Ticketing
    // Carrier or Participating Carrier.
    BOTH_CARRIER = 'B' // COP Carrier must be both Ticketing
    // Carrier and Participating Carrier.
  };

  enum PuAppl
  {
    APPLY = 'A', // Apply COP for qulified PU
    NOT_APPLY = 'N' // Does not apply COP for qulified PU
  };

  bool processPU(PricingUnit& pu);

  /**
   *   @method qualifyPu
   *
   *   Description: Checks if Pricing Unit qualifies for COP application.
   *
   *   @param  PricingUnit - Pricing Unit
   *   @return bool        - true if the PU is prequalified for COP.
   */

  bool qualifyPu(const PricingUnit& pu);

  /**
   *   @method matchPu
   *
   *   Description: Matches Pricing Unit against COP table.
   *
   *   @param  PricingUnit - Pricing Unit
   *   @return bool        - true if COP is applicable
   */

  bool matchPu(const PricingUnit& pu);

  bool matchTktCopItems(const std::vector<CopMinimum*>& copItems,
                        const PricingUnit& pu,
                        const CarrierCode& carrier,
                        const std::set<CarrierCode>& participatingCxrs,
                        bool& matchPartCxr);

  bool matchPrtCopItems(const std::vector<CopMinimum*>& copItems,
                        const PricingUnit& pu,
                        const CarrierCode& carrier);

  const CopCarrier*
  checkCopCxr(const CopMinimum& cop, const CarrierCode& carrier, bool isTktCarrier = true);

  bool checkTktCxrException(const CarrierCode& tktCxr, const CopMinimum& cop);

  bool isCOPLoc(const Loc& loc) const;
  bool isCOPLoc(const NationCode& country, const NationCode& saleCountry) const;
  bool isViaCOPNation(const std::vector<TravelSeg*>& tvlSegs, TravelAppl tvlAppl) const;

  bool isFrenchNationGroup(const NationCode& nation) const;
  bool isUSSRNationGroup(const NationCode& nation) const;
  bool isGulfRegion(const NationCode& nation) const;

  void displayCOPHeader();
  void displayCOPItem(CopMinimum& cop);
  void displayCOPItem(CopMinimum& cop, int copCxrIdx);
  void displayPu(const PricingUnit& pu);

  const std::vector<CopMinimum*>& getCopInfo(const NationCode& saleLoc);

  bool checkExclusionByTableEntry(const PricingUnit& pu);

  bool calculateFare(PricingUnit& pu);

  MoneyAmount calculateBaseFare(const PricingUnit& pu,
                                MinFarePlusUpItem& minFarePlusUp,
                                bool mixedCabin,
                                const CabinType& lowestCabin);

  /**
   * Retrieve the fare component that contains, or originate from the given
   * COP location
   **/
  const FareUsage* getCopFareComponent(const PricingUnit& pu, const Loc* origin) const;

  void calculateFarePlusUp(const PricingUnit& pu,
                           MinFarePlusUpItem& minFarePlusUp,
                           bool mixedCabin,
                           const CabinType& lowestCabin);

  void calculateFarePlusUp(MinFarePlusUpItem& minFarePlusUp,
                           const PricingUnit& pu,
                           const PaxTypeFare* obFare,
                           const PaxTypeFare* ibFare,
                           const LocCode& copPoint,
                           const LocCode& endPoint,
                           bool mixedCabin,
                           const CabinType& cabinType);

  bool selectFare(const PricingUnit& pu,
                  std::vector<TravelSeg*>& tvlSeg,
                  MinFarePlusUpItem& minFarePlusUp,
                  bool mixedCabin,
                  const CabinType& lowestCabin);

  const PaxTypeFare* repriceAndSelectFare(const PricingUnit& pu,
                                          const PaxTypeFare* thruFare,
                                          std::vector<TravelSeg*>& tvlSeg,
                                          MinFareFareSelection::FareDirectionChoice fareDirection,
                                          bool mixedCabin,
                                          const CabinType& lowestCabin);

  MoneyAmount accumulateFareAmount(const FareUsage& fu) const;

  GlobalDirection getGlobalDirection(PricingTrx& trx,
                                     const std::vector<TravelSeg*>& tvlSegs,
                                     const DateTime& travelDate) const;

  CarrierCode getGoverningCarrier(const std::vector<TravelSeg*>& tvlSegs) const;

  bool
  getTvlSegFromCopPoint(const PricingUnit& pu, std::vector<std::vector<TravelSeg*>>& copTvlSegs) const;

  bool diagEnabled(DiagnosticTypes diagType) const;

  RepricingTrx* getRepricingTrx(std::vector<TravelSeg*>& tvlSeg);

  MoneyAmount repriceFare(const PricingUnit& pu, const FareUsage& fu, const CabinType& lowestCabin);

  PricingTrx& _trx;
  FarePath& _farePath;
  NationCode _saleCountry;
  CarrierCode _tktgCxr;

  std::ostringstream _diag;
  bool _diagEnabled = false;
  DiagnosticTypes _diagType;

  MoneyAmount _copFare = 0.0;
  LocCode _copBoardPoint;
  LocCode _copOffPoint;

  // mutable so that even the const member function can use this handle
  // to cache object.
  mutable DataHandle _dataHandle;
};
}
