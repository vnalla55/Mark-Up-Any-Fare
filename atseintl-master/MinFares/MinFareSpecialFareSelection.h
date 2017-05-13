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

namespace tse
{
class PricingTrx;
class RepricingTrx;
class PaxType;
class PaxTypeFare;
class MinFareAppl;
class FareTypeMatrix;
class FarePath;
class PricingUnit;
class FareMarket;
class MinFareFareTypeGrp;

/**
 * @class MinFareSpecialFareSelection
 *
 * This class is used to select a Special fare for an market in
 * Minimum Fares.
 */

class MinFareSpecialFareSelection : public MinFareFareSelection
{
  friend class MinFareSpecialFareSelectionDerived2;
  friend class MinFareSpecialFareSelectionTest;

public:
  MinFareSpecialFareSelection(MinimumFareModule module,
                              EligibleFare eligibleFare,
                              FareDirectionChoice fareDirection,
                              const FareType& fareType,
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

  const PaxTypeFare*
  selectFare(PaxTypeStatus selPaxTypeStatus = PAX_TYPE_STATUS_UNKNOWN, bool selNormalFare = false);
  const PaxTypeFare* selectFare(const std::vector<TravelSeg*>& travelSegs,
                                PaxTypeStatus selPaxTypeStatus = PAX_TYPE_STATUS_UNKNOWN,
                                bool selNormalFare = false);
  const PaxTypeFare* selectNormalFare(const std::vector<TravelSeg*>& travelSegs,
                                      PaxTypeStatus selPaxTypeStatus = PAX_TYPE_STATUS_UNKNOWN);

  PtfPair newSelectFare(PaxTypeStatus selPaxTypeStatus);

private:
  const PaxTypeFare*
  selectNormalFareForConstruction(const std::vector<TravelSeg*>& travelSegs,
                                  PaxTypeStatus selPaxTypeStatus = PAX_TYPE_STATUS_UNKNOWN,
                                  bool selectNextCabin = false) override;
  virtual const PaxTypeFare* selectSpecialFare(const std::vector<TravelSeg*>& travelSegs);
  const PaxTypeFare* selectNormalCabinFare(const std::vector<TravelSeg*>& travelSegs);
  const PaxTypeFare* selectGovCxrValidatedFare(bool matchApplRec);
  const PaxTypeFare* selectValidatedFare(const PaxTypeFareSet& validatedFares, bool matchApplRec);
  const PaxTypeFare* selectFareByLogicPref(const PaxTypeFareSet& validatedFares);

  void getFareMarket(const std::vector<TravelSeg*>& travelSegs, bool forSpcl);
  void identifyValidatedFares(const std::vector<PaxTypeFare*>& fares, bool yyAllowed, bool forSpcl);

  bool matchSpecial(const PaxTypeFare& fare);

  virtual void getDefaultLogic() override;
  virtual void getOverrideLogic() override;

  void displayFareSelectReq(const std::vector<TravelSeg*>& travelSegs);
  virtual void displayMinFareApplLogic() override;

  void processFareMarket(const FareMarket& fareMarket, bool forSpcl);

  const PaxTypeFare* processHigherFareType(const std::vector<TravelSeg*>& travelSegs);

  bool higherFareType(const FareType& fareType, int32_t& workingSet);
  virtual const MinFareFareTypeGrp* getMinFareFareTypeGrp();
  virtual const FareTypeMatrix* getFareTypeMatrix(const FareType& key, const DateTime& date);
  bool matchedApplRecord();
  bool matchFareType(const FareType& fareType);

  const FareType& _fareType;
  std::vector<FareType> _curFareTypeVec;
  CabinType _cabin;
  bool _isProm = false;
  bool _sameTariffCat = false;
  bool _sameRuleTariff = false;
  bool _sameFareClass = false;
  bool _same1charFareBasis = false;
  bool _stopOverComp = false;
  std::string _spclProcName;
};
}
