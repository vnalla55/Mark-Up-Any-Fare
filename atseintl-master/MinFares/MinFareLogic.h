#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseEnums.h"
#include "MinFares/MinFareFareSelection.h"

namespace tse
{

class Itin;
class PricingUnit;
class PaxTypeFare;
class PaxType;
class FarePath;
class FareUsage;
class MinFareRuleLevelExcl;

namespace MinFareLogic
{
bool
isMixedCabin(const PricingUnit& pu, CabinType& lowestCabin);

MinFareFareSelection::EligibleFare
eligibleFare(const PricingUnit& pu);

MinFareFareSelection::FareDirectionChoice
fareDirection(const FareUsage& fu);

const PaxTypeFare*
selectQualifyFare(MinimumFareModule module,
                  PricingTrx& trx,
                  const Itin& itin,
                  const PaxTypeFare& paxTypeFare,
                  const PaxType& requestedPaxType,
                  const CabinType& cabin,
                  bool selectNormalFare,
                  MinFareFareSelection::FareDirectionChoice fareDirection,
                  MinFareFareSelection::EligibleFare eligibleFare,
                  const std::vector<TravelSeg*>& travelSeg,
                  const DateTime& travelDate,
                  const MinFareAppl* matchedApplItem = nullptr,
                  const MinFareDefaultLogic* matchedDefaultItem = nullptr,
                  const RepricingTrx* repricingTrx = nullptr,
                  const FarePath* farePath = nullptr,
                  const PricingUnit* pu = nullptr,
                  const PaxTypeCode& actualPaxTypeCode = "");

const PtfPair
selectQualifyConstFare(MinimumFareModule module,
                       PricingTrx& trx,
                       const Itin& itin,
                       const PaxTypeFare& paxTypeFare,
                       const PaxType& requestedPaxType,
                       const CabinType& cabin,
                       bool selectNormalFare,
                       MinFareFareSelection::FareDirectionChoice fareDirection,
                       MinFareFareSelection::EligibleFare eligibleFare,
                       const std::vector<TravelSeg*>& travelSeg,
                       const DateTime& travelDate,
                       const MinFareAppl* matchedApplItem = nullptr,
                       const MinFareDefaultLogic* matchedDefaultItem = nullptr,
                       const RepricingTrx* repricingTrx = nullptr,
                       const FarePath* farePath = nullptr,
                       const PricingUnit* pu = nullptr,
                       const PaxTypeCode& actualPaxTypeCode = "");

/**
 * Select fare + HIP processing for the whole PU
 */
MoneyAmount
selectLowestCabinFare(DiagCollector* diag,
                      MinimumFareModule module,
                      const PricingUnit& pu,
                      const CabinType& lowestCabin,
                      PricingTrx& trx,
                      const FarePath& farePath,
                      const PaxType& requestedPaxType,
                      const DateTime& travelDate,
                      bool normalFareSelection = true,
                      const MinFareAppl* matchedApplItem = nullptr,
                      const MinFareDefaultLogic* matchedDefaultItem = nullptr,
                      const RepricingTrx* repricingTrx = nullptr);

const PaxTypeFare*
getRepricedNormalFare(PricingTrx& trx,
                      const FarePath& farePath,
                      const PricingUnit& pu,
                      const FareUsage& originalFu,
                      FareUsage*& repricedFu,
                      MinFareFareSelection::EligibleFare eligibleFare,
                      const CabinType& lowestCabin,
                      const DateTime& travelDate,
                      bool sameFareClass = false);

const PaxTypeFare*
getRevNigeriaFare(PricingTrx& trx,
                  const FarePath& farePath,
                  const PricingUnit& pu,
                  const PaxTypeFare* paxTypeFare,
                  const DateTime& travelDate);

bool
checkDomesticExclusion(PricingTrx& trx,
                       const Itin& itin,
                       const PaxTypeFare& paxTypeFare,
                       const MinFareAppl* applItem,
                       const MinFareDefaultLogic* defaultItem,
                       const std::vector<TravelSeg*>& tvlSeg);

const MinFareRuleLevelExcl*
getRuleLevelExcl(PricingTrx& trx,
                 const Itin& itin,
                 const PaxTypeFare& paxTypeFare,
                 MinimumFareModule module,
                 std::multimap<uint16_t, const MinFareRuleLevelExcl*>& ruleMap,
                 const DateTime& travelDate);

const MinFareAppl*
getApplication(PricingTrx& trx,
               const FarePath& farePath,
               const Itin& itin,
               const PaxTypeFare& paxTypeFare,
               MinimumFareModule module,
               std::multimap<uint16_t, const MinFareAppl*>& applMap,
               std::map<uint16_t, const MinFareDefaultLogic*>& defaultLogicMap,
               const DateTime& travelDate);

const MinFareDefaultLogic*
getDefaultLogic(MinimumFareModule module,
                PricingTrx& trx,
                const Itin& itin,
                const PaxTypeFare& paxTypeFare,
                std::map<uint16_t, const MinFareDefaultLogic*>& defaultLogicMap);

uint16_t
getMapKey(const PaxTypeFare& paxTypeFare, const Itin& itin);

/**
 * Return the MinFareAppl indicator for the specified module
 */
Indicator
getMinFareApplIndicator(MinimumFareModule module, const MinFareAppl& appl);

/**
 * Return the RuleLevelExclusion indicator for the specified module
 */
Indicator
getMinFareExclIndicator(MinimumFareModule module, const MinFareRuleLevelExcl& rule);

/**
 * Retrieve the Global Direction for the specified travel segment
 */
GlobalDirection
getGlobalDirection(PricingTrx& trx, const std::vector<TravelSeg*>& tvlSegs, DateTime travelDate);

/**
 * Return PaxTypeStatus for a PaxTypeFare
 */
PaxTypeStatus
paxTypeStatus(const PaxTypeFare& fare);

/**
 * Return lowest PaxTypeStatus within a PricingUnit
 */
PaxTypeStatus
paxTypeStatus(const PricingUnit& pu);

FareType
getHigherFareType(PricingTrx& trx,
                  const PaxTypeFare& paxTypeFare,
                  const FareType& fareType,
                  const Itin& itin,
                  const DateTime& travelDate);

FareType
getHigherPromFareType(PricingTrx& trx,
                      const PaxTypeFare& paxTypeFare,
                      const FareType& fareType,
                      const Itin& itin,
                      const DateTime& travelDate);

FareType
getHigherSpclFareType(PricingTrx& trx, const FareType& fareType, const DateTime& travelDate);

bool
isDomestic(const Itin& itin,
           const std::vector<TravelSeg*>& travelSegs,
           bool checkHiddenStop = true);

} // end of MinFareLogic
} // end of tse

