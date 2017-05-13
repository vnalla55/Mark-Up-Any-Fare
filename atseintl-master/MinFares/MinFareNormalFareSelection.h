//----------------------------------------------------------------------------
//  Copyright Sabre 2004
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

#include "MinFares/MinFareFareSelection.h"
#include "MinFares/MinimumFare.h"

namespace tse
{
class PricingTrx;
class RepricingTrx;
class PaxTypeFare;
class PaxType;
class MinFareAppl;
class FarePath;
class PricingUnit;
class TravelSeg;
class FareMarket;

/**
 * @class MinFareNormalFareSelection
 *
 * This class is used to select a Normal fare for an market in
 * Minimum Fares.
 */

class MinFareNormalFareSelection : public MinFareFareSelection
{
  friend class MinFareNormalFareSelectionTest;

public:
  static constexpr char UNRESTRICTED = 'U';
  static constexpr char RESTRICTED = 'R';

  static constexpr char ANY_TYPE = 'A';
  static constexpr char SAME_TYPE = 'S';

  MinFareNormalFareSelection(MinimumFareModule module,
                             EligibleFare eligibleFare,
                             FareDirectionChoice fareDirection,
                             CabinType cabin,
                             PricingTrx& trx,
                             const Itin& itin,
                             const std::vector<TravelSeg*>& travelSegs,
                             const std::vector<PricingUnit*>& pricingUnits,
                             const PaxType* paxType,
                             const DateTime& travelDate,
                             const FarePath* farePath = nullptr,
                             const PaxTypeFare* thruFare = nullptr,
                             const MinFareAppl* minFareAppl = nullptr,
                             const MinFareDefaultLogic* minFareDefaultLogic = nullptr,
                             const RepricingTrx* repricingTrx = nullptr,
                             const PaxTypeCode& actualPaxType = "");

  const PaxTypeFare* selectFare(PaxTypeStatus selPaxTypeStatus = PAX_TYPE_STATUS_UNKNOWN,
                                bool selectNextCabin = false);

  PtfPair newSelectFare(PaxTypeStatus selPaxTypeStatus = PAX_TYPE_STATUS_UNKNOWN,
                        bool selectNextCabin = false);

private:
  const PaxTypeFare* selectFare(const std::vector<TravelSeg*>& travelSegs,
                                PaxTypeStatus selPaxTypeStatus = PAX_TYPE_STATUS_UNKNOWN,
                                bool selectNextCabin = false);

  const PaxTypeFare*
  selectNormalFareForConstruction(const std::vector<TravelSeg*>& travelSegs,
                                  PaxTypeStatus selPaxTypeStatus = PAX_TYPE_STATUS_UNKNOWN,
                                  bool selectNextCabin = false) override;

  virtual const PaxTypeFare*
  selectFareForCabin(const std::vector<TravelSeg*>& travelSegs, CabinType cabin, bool selectLowest);

  const PaxTypeFare* selectFareForHigherCabin(const std::vector<TravelSeg*>& travelSegs);
  const PaxTypeFare* selectFareForLowerCabin(const std::vector<TravelSeg*>& travelSegs);
  bool getLowerCabin(CabinType& modifiedCabin);

  const PaxTypeFare* selectGovCxrValidatedFare(const PaxTypeFareSet& govCxrValidatedFares,
                                               const PaxTypeFareSet& govCxrNonValidatedFares,
                                               bool selectLowest,
                                               bool forSameCabin);
  const PaxTypeFare* selectNonValidatedFare(const PaxTypeFareSet& nonValidatedFares);
  const PaxTypeFare*
  selectValidatedFare(const PaxTypeFareSet& validatedFares, bool selectLowest, bool forSameCabin);
  const PaxTypeFare* selectWithinAllFare(const PaxTypeFareSet& validatedFares, bool selectLowest);

  const PaxTypeFare* selectMpmFare(const PaxTypeFareSet& validatedFares);
  const PaxTypeFare* selectRoutingFare(const PaxTypeFareSet& validatedFares);
  const PaxTypeFare* selectFareByLogicPref(const PaxTypeFareSet& fares);

  void getFareMarket(const std::vector<TravelSeg*>& travelSegs, CabinType cabin);
  void
  identifyValidatedFares(const std::vector<PaxTypeFare*>& fares, CabinType cabin, bool yyAllowed);
  bool isAllSameType(const PaxTypeFareSet& fares);

  void displayFareSelectReq(const std::vector<TravelSeg*>& travelSegs, CabinType cabin);
  virtual void displayMinFareApplLogic() override;

  virtual void getDefaultLogic() override;
  virtual void getOverrideLogic() override;

  void processFareMarket(const FareMarket& fareMarket, CabinType cabin);

  FareType _preferedFareType;
  bool _sameFareType = false;

  bool _selHighestInHigherCabin = false;
  bool _selMpmBeforeRouting = false;
  bool _selRoutingBeforeMpm = false;
  int _selTariffCat = MinimumFare::BLANK_TRF_CAT;
  bool _selSameFareType = false;
  bool _selSameRbd = false;
  bool _compStopOver = false;
};
}
