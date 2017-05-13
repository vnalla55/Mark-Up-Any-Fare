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

#include <set>
#include <bitset>
#include <vector>
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseEnums.h"
#include "Common/ErrorResponseException.h"
#include "Common/PaxTypeUtil.h"
#include "Common/CabinType.h"
#include "MinFares/MatchRuleLevelExclTable.h"

namespace tse
{
class PricingTrx;
class RepricingTrx;
class TravelSeg;
class PaxType;
class PaxTypeFare;
class MinFareAppl;
class MinFareDefaultLogic;
class MinFareRuleLevelExcl;
class DiagCollector;
class Loc;
class Itin;
class LocKey;
class FarePath;
class PricingUnit;
class FareMarket;

typedef std::pair<const PaxTypeFare*, const PaxTypeFare*> PtfPair;

/**
 * @class MinFareFareSelection
 *
 * This class is the base class for selecting a specific type fare for an
 * intermediate market in Minimum Fare modules. It uses the common fare
 * selection rules across all Minimum Fare modules.
 *
 */

class MinFareFareSelection
{
  friend class RoundTripFareSelection;

public:
  enum FareDirectionChoice
  {
    OUTBOUND,
    INBOUND,
    BOTH
  };

  enum EligibleFare
  {
    ONE_WAY,
    HALF_ROUND_TRIP
  };

  enum MinFareApplYY
  {
    INCLUDE = 'I',
    EXCLUDE = 'E'
  };

  enum ValidStatus
  {
    VALIDATED,
    NON_VALIDATED,
    NONE
  };

  // static CabinType cabinType(const Indicator& cabin);
  // static CabinType lowerCabin(const CabinType& cabin);
  // static CabinType higherCabin(const CabinType& cabin);

  static const std::string MinFareModuleName[];

  static bool isEligibleFare(const PaxTypeFare& fare,
                             const PaxTypeFare& thruFare,
                             MinFareFareSelection::EligibleFare eligibleFare);

  MinFareFareSelection(MinimumFareModule module,
                       EligibleFare eligibleFare,
                       FareDirectionChoice fareDirection,
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

  virtual ~MinFareFareSelection() = default;

  GlobalDirection globalDirection() { return _globalDirection; }

protected:
  static const std::string FareDirStr[];

  class AscendingOrder
  {
  public:
    bool operator()(const PaxTypeFare* fare1, const PaxTypeFare* fare2) const;
  };

  typedef std::multiset<const PaxTypeFare*, AscendingOrder> PaxTypeFareSet;

  class ApplLogicOrder
  {
  public:
    ApplLogicOrder(const PaxTypeFare* thruFare, bool compRbd, bool compStopOver);
    virtual ~ApplLogicOrder() {}

    bool operator()(const PaxTypeFare* fare1, const PaxTypeFare* fare2) const;

  protected:
    const PaxTypeFare* _thruFare;
    bool _compRbd;
    bool _compStopOver;
  };

  typedef std::multiset<const PaxTypeFare*, ApplLogicOrder> FareApplLogicSet;

  virtual const PaxTypeFare*
  selectNormalFareForConstruction(const std::vector<TravelSeg*>& travelSegs,
                                  PaxTypeStatus selPaxTypeStatus = PAX_TYPE_STATUS_UNKNOWN,
                                  bool selectNextCabin = false) = 0;

  bool isCxrAllowYYFare(const CarrierCode& govCxr, const std::vector<TravelSeg*>& travelSegs);
  bool matchDirectionality(const Directionality& directionality,
                           const LocKey& loc1,
                           const LocKey& loc2,
                           const std::vector<TravelSeg*>& tvlSegs);
  ValidStatus validateFare(const PaxTypeFare& fare);
  virtual bool ruleValidated(const std::vector<uint16_t>& categories,
                             const PaxTypeFare& paxTypeFare,
                             bool puScope = false);
  bool isRightPaxType(const PaxTypeFare& fare);
  bool isRightDirection(const PaxTypeFare& fare);
  bool isEligible(const PaxTypeFare& fare);
  const PaxTypeFare*
  selectFareByRuleLevelExcl(const PaxTypeFareSet& fares, bool selectLowest = true);
  bool passRuleLevelExclusion(const PaxTypeFare& paxTypeFare);
  bool checkRuleLevelExclusionComparison(const PaxTypeFare& paxTypeFare,
                                         const MinFareRuleLevelExcl*,
                                         MatchRuleLevelExclTable& matchedRuleLevelExcl);

  void displayFareHeader();
  void displayFare(const PaxTypeFare& paxTypeFare);
  bool isDefaultThruAll();
  bool validateCat25Cat35(const PaxTypeFare& paxTypeFare);
  void getFareSelPref();
  virtual void getDefaultLogic() = 0;
  virtual void getOverrideLogic() = 0;
  virtual void displayMinFareApplLogic() = 0;

  void getAlternateFareMarket(const std::vector<TravelSeg*>& travelSegs,
                              std::vector<FareMarket*>& retFareMarket);

  bool isRepriceNeeded();
  PricingTrx* getRepriceTrx(const std::vector<TravelSeg*>& tvlSegs);
  MinimumFareModule& module();

  MinimumFareModule _module;
  EligibleFare _eligibleFare;
  FareDirectionChoice _fareDirection;
  CabinType _cabin;
  PricingTrx& _trx;
  const Itin& _itin;
  const std::vector<TravelSeg*>& _travelSegs;
  const std::vector<PricingUnit*>& _pricingUnits;
  const PaxType* _paxType;
  DateTime _travelDate;
  const FarePath* _farePath;
  const PaxTypeFare* _thruFare;
  const MinFareAppl* _minFareAppl;
  const MinFareDefaultLogic* _minFareDefaultLogic;
  const RepricingTrx* _repricingTrx;
  PaxTypeCode _actualPaxType;
  CarrierCode _govCxr;
  PaxTypeStatus _paxTypeStatus;
  PaxTypeStatus _selPaxTypeStatus;

  GlobalDirection _globalDirection;

  DiagCollector* _diag;
  bool _isThruFareOw;
  LocCode _origin;
  LocCode _dest;

  PaxTypeFareSet _govCxrValidatedFares;
  PaxTypeFareSet _govCxrNonValidatedFares;
  PaxTypeFareSet _yyValidatedFares;
  PaxTypeFareSet _yyNonValidatedFares;

  bool _yyOverride;
  std::vector<CarrierCode> _govCxrOverrides;

  bool _exemptRuleVal;

  bool _isNetRemit;
};
}
