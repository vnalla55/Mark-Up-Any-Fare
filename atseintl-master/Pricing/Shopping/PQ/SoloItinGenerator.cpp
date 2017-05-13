// ----------------------------------------------------------------
//
//   Copyright Sabre 2011
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#include "Pricing/Shopping/PQ/SoloItinGenerator.h"

#include "Common/Assert.h"
#include "Common/ClassOfService.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/FarePathCopier.h"
#include "Common/Hasher.h"
#include "Common/ItinUtil.h"
#include "Common/Logger.h"
#include "Common/MultiDimensionalPQ.h"
#include "Common/PaxTypeUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/ShpqTypes.h"
#include "Common/TravelSegAnalysis.h"
#include "Common/TSELatencyData.h"
#include "Common/ValidatingCarrierUpdater.h"
#include "Common/Vendor.h"
#include "DataModel/FareUsage.h"
#include "DataModel/InterlineTicketCarrierData.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/ShoppingTrx.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/NegFareRest.h"
#include "Diagnostic/Diag941Collector.h"
#include "Pricing/FPPQItem.h"
#include "Pricing/MaximumPenaltyValidator.h"
#include "Pricing/Shopping/Diversity/DiversityUtil.h"
#include "Pricing/Shopping/PQ/AltDatesTaxes.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/PQ/ScopedFMGlobalDirSetter.h"
#include "Pricing/Shopping/PQ/SoloADGroupFarePath.h"
#include "Pricing/Shopping/PQ/SoloAltDatesInfo.h"
#include "Pricing/Shopping/PQ/SoloFamilyGrouping.h"
#include "Pricing/Shopping/PQ/SoloGroupFarePath.h"
#include "Pricing/Shopping/PQ/SoloPQ.h"
#include "Pricing/Shopping/PQ/SoloTrxData.h"
#include "Pricing/Shopping/PQ/SOPCombinationBuilder.h"
#include "Pricing/Shopping/PQ/SOPInfo.h"
#include "Rules/RuleControllerWithChancelor.h"
#include "Rules/RuleUtil.h"

#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scope_exit.hpp>

#include <algorithm>
#include <functional>
#include <iterator>
#include <limits>

namespace tse
{
FALLBACK_DECL(fallbackISAvailTuning);
FALLBACK_DECL(fallbackValidatingCxrGTC);
FALLBACK_DECL(ngsMaximumPenaltyFarePathValidation)
FALLBACK_DECL(skippedBitValidationInIsOpt)

namespace
{
ConfigurableValue<double>
thruFarePopulationRateLimit("SHOPPING_DIVERSITY", "THRU_FARE_POPULATION_RATE_LIMIT", 0.9);
ConfigurableValue<bool>
changeFamilyGroupingForThruOnly("SHOPPING_DIVERSITY", "CHANGE_FAMILY_GROUPING_FOR_THRUONLY", false);
ConfigurableValue<uint32_t>
hintingStartsAt("SHOPPING_DIVERSITY", "HINTING_STARTS_AT");
ConfigurableValue<uint32_t>
maxFailedDelayedBitValidationCountPerFP("SHOPPING_DIVERSITY", "MAX_FAILED_BIT_VALIDATIONS_PER_FP");
ConfigurableValue<uint32_t>
maxFailedCombPerFP("SHOPPING_DIVERSITY", "MAX_FAILED_COMBINATIONS_PER_FP");
ConfigurableValue<uint32_t>
maxFailedComb("SHOPPING_DIVERSITY", "MAX_FAILED_COBINATIONS");
ConfigurableValue<uint32_t>
familyGroupingOptCfg("SHOPPING_DIVERSITY", "SOL_FAMILYGROUPING", 1);

void
addCombinationResult(Diag941Collector* dc,
                     DiversityModel::SOPCombinationList::const_iterator combIt,
                     DiversityModel::SOPCombinationList::const_iterator combItEnd,
                     Diag941Collector::CombinationResult result)
{
  for (; combIt != combItEnd; ++combIt)
  {
    dc->addCombinationResult(combIt->oSopVec, result);
  }
}

void
printSopsPerDateDiagnostic(ShoppingTrx& trx,
                           Diag941Collector* dc,
                           shpq::SOPCombinationBuilder::SopsByDateByLeg& sopsByDateByLeg)
{
  if (!dc)
    return;

  for (uint16_t legId = 0; legId < sopsByDateByLeg.size(); ++legId)
  {
    *dc << "SOPs by date for leg " << legId << ":\n";
    for (const shpq::SOPCombinationBuilder::SopsByDate::value_type& sopsByDate :
         sopsByDateByLeg[legId])
    {
      *dc << "  " << sopsByDate.first.dateToString(MMDDYY, "") << ": ";
      for (std::set<int>::value_type sopId : sopsByDate.second)
      {
        *dc << ShoppingUtil::findSopId(trx, legId, sopId) << " ";
      }
      *dc << "\n";
    }
  }
}

class CombinationEraseDelegate : public DiversityModel::SOPCombinationList::DiagDelegate
{
public:
  CombinationEraseDelegate(Diag941Collector* dc,
                           Diag941Collector::CombinationResult combResult,
                           DiversityModel::SOPCombinationList& combList)
    : _dc(dc), _reportCombResult(combResult), _combList(combList), _delegatePtr(this)
  {
    if (UNLIKELY(dc))
      combList.swapDelegate(_delegatePtr);
  }

  ~CombinationEraseDelegate()
  {
    if (UNLIKELY(_dc))
      _combList.swapDelegate(_delegatePtr);
  }

  void onErase(const DiversityModel::SOPCombination& comb) override
  {
    if (_dc)
      _dc->addCombinationResult(comb.oSopVec, _reportCombResult);
  }

private:
  Diag941Collector* const _dc;
  const Diag941Collector::CombinationResult _reportCombResult;

  DiversityModel::SOPCombinationList& _combList;
  DiversityModel::SOPCombinationList::DiagDelegate* _delegatePtr;
};

} // namespace anon

Logger
SoloItinGenerator::_logger("atseintl.ShoppingPQ.SoloItinGenerator");

class SoloItinGenerator::ValidateCarrierRestrictionsPtfVisitor
{
public:
  ValidateCarrierRestrictionsPtfVisitor(const SoloItinGenerator& ctx,
                                        shpq::SopIdxVecArg oSopVec,
                                        const Itin* itin)
    : _isFbrSameCxrValidated(false), _ctx(ctx), _oSopVec(oSopVec), _itin(itin)
  {
  }

  bool visitAndCheck(const PaxTypeFare* fare)
  {
    if (fare->isFareByRule())
    {
      const FareByRuleApp& fbrApp = fare->fbrApp();

      if (UNLIKELY(!checkFbrSameCarrier(fare, fbrApp) || !checkFbrUseCarrierInTable(fare, fbrApp)))
      {
        return false;
      }
    }

    return checkCat35ValidatingCarrier(fare);
  }

private:
  bool checkFbrSameCarrier(const PaxTypeFare* fare, const FareByRuleApp& fbrApp)
  {
    if (LIKELY(fbrApp.sameCarrier() == BLANK ||
               _isFbrSameCxrValidated)) // validate only once, as far _itin is immutable
    {
      return true;
    }

    if (RuleUtil::useSameCarrier(_itin->travelSeg()))
    {
      return (_isFbrSameCxrValidated = true);
    }
    else
    {
      if (_ctx._dc)
        _ctx._dc->addCombinationResult(_oSopVec, Diag941Collector::FBR_SAME_CARRIER);
      return false;
    }
  }
  bool checkFbrUseCarrierInTable(const PaxTypeFare* fare, const FareByRuleApp& fbrApp)
  {
    if (LIKELY(fbrApp.sameCarrier() != BLANK || fbrApp.carrierFltTblItemNo() == 0))
    {
      return true;
    }

    if (RuleUtil::useCxrInCxrFltTbl(_itin->travelSeg(),
                                    fbrApp.vendor(),
                                    fbrApp.carrierFltTblItemNo(),
                                    _ctx._trx.ticketingDate()))
    {
      return true;
    }
    else
    {
      if (_ctx._dc)
        _ctx._dc->addCombinationResult(_oSopVec, Diag941Collector::FBR_USE_CARRIER_IN_TABLE);

      return false;
    }
  }
  bool checkCat35ValidatingCarrier(const PaxTypeFare* fare)
  {
    const PaxTypeFareRuleData* ptfrd = fare->paxTypeFareRuleData(RuleConst::NEGOTIATED_RULE);
    if (!ptfrd)
      return true;

    const NegFareRest* negFareRest = dynamic_cast<const NegFareRest*>(ptfrd->ruleItemInfo());
    if (!negFareRest->carrier().empty())
    {
      if ((negFareRest->tktAppl() == ' ' && negFareRest->carrier() != _itin->validatingCarrier()) ||
          (negFareRest->tktAppl() == 'X' && negFareRest->carrier() == _itin->validatingCarrier()))
      {
        if (_ctx._dc)
          _ctx._dc->addCombinationResult(_oSopVec, Diag941Collector::CAT35_VALIDATING_CARRIER);

        return false;
      }
    }

    return true;
  }

  bool _isFbrSameCxrValidated;
  const SoloItinGenerator& _ctx;
  const shpq::SopIdxVec _oSopVec;
  const Itin* const _itin;
};

SoloItinGenerator::SoloItinGenerator(shpq::SoloTrxData& soloTrx,
                                     shpq::SoloPQ& pq,
                                     DiversityModel* dm,
                                     const ItinStatistic& stats,
                                     BitmapOpOrderer& bitmapOpOrderer)
  : _trx(soloTrx.getShoppingTrx()),
    _pq(pq),
    _dm(dm),
    _stats(stats),
    _bitmapOpOrderer(bitmapOpOrderer),
    _dc(dynamic_cast<Diag941Collector*>(soloTrx.getShoppingDC(Diagnostic941))),
    _fpHashFuncVer(FP_HASH_FUNC_VER_2),
    _interlineTicketCarrierData(nullptr),
    _thruFarePopulationRateLimit(0.9),
    _changeFamilyGroupingForThruOnly(true),
    _asoCandidateChecker(_trx),
    _hintingStartsAt(0),
    _initialSolutionsCount(0),
    _maxFailedCombPerFP(0),
    _maxFailedComb(0),
    _failedCombCount(0),
    _maxFailedDelayedBitValidationCountPerFP(0),
    _sopIdxStdVecAdapter(soloTrx.getShoppingTrx().legs().size())
{
  initializeDiagnostic();
  initializeAcmsVars();

  if (_trx.getRequest()->processVITAData() && _trx.getOptions()->validateTicketingAgreement())
  {
    // Validate the interline ticketing agreements (VITA)
    if (InterlineTicketCarrier::isPriceInterlineActivated(_trx))
    {
      _trx.dataHandle().get(_interlineTicketCarrierData);
    }
    else if (_dc)
    {
      _dc->printVITAPriceInterlineNotActivated();
    }
  }
}

SoloItinGenerator::~SoloItinGenerator()
{
  if (_dc)
    _dc->flushMsg();
}

bool
SoloItinGenerator::validateDelayedFlightStatuses(
    const CarrierKeyVec& carrierKeys,
    shpq::SopIdxVecArg bitVec,
    const std::vector<std::vector<SOPInfo>>& sopInfos,
    const DiversityModel::SOPCombination& combination,
    size_t& failedLegIdx,
    PaxTypeFareBitmapValidator::SkippedBitValidator* skippedBitValidator)
{
  const bool fltBitmapOpt = !fallback::skippedBitValidationInIsOpt(&_trx);

  for (failedLegIdx = 0; failedLegIdx < bitVec.size(); ++failedLegIdx)
  {
    const uint32_t cxrKey = carrierKeys[failedLegIdx];
    const uint32_t bitNo = bitVec[failedLegIdx];

    for (PaxTypeFare* const ptf : sopInfos[failedLegIdx][bitNo].ptfsToValidate)
    {
      ptf->setComponentValidationForCarrier(cxrKey, _trx.isAltDates(), combination.duration);

      if (fltBitmapOpt && !ptf->isFlightSkipped(bitNo))
        continue;

      PaxTypeFareBitmapValidator ptfbv(
          _trx, *ptf, combination.datePair, skippedBitValidator, nullptr);
      if (!ptfbv.validate(bitNo, cxrKey) || !ptf->isFlightValid(bitNo))
        return false;
    }
  }

  return true;
}

void
SoloItinGenerator::generateSolutions(const shpq::SoloPQItemPtr& pqItem,
                                     shpq::SoloTrxData& soloTrxData)
{
  TSELatencyData metrics(_trx, "SOLO GENERATE SOLUTIONS");

  const size_t noLegs = _trx.legs().size();
  FarePath* fp = pqItem->getFarePath();
  shpq::CxrKeysPerLeg cxrKeysPerLeg;

  TSE_ASSERT(fp);
  if (!ShoppingUtil::collectFPCxrKeysNew(_trx, *fp, noLegs, cxrKeysPerLeg))
    return;

  shpq::CxrKeyPerLeg cxrKey;

  for (uint32_t obCxrKey : cxrKeysPerLeg[0])
  {
    if (UNLIKELY(cxrKeysPerLeg[0].size() > 1u && !checkFaresValidForCxr(*fp, 0, obCxrKey)))
      continue;

    cxrKey[0] = obCxrKey;

    if (noLegs == 1)
    {
      generateSolutionsImpl(pqItem, cxrKey, soloTrxData);
      continue;
    }

    for (uint32_t ibCxrKey : cxrKeysPerLeg[1])
    {
      if (UNLIKELY(cxrKeysPerLeg[1].size() > 1u && !checkFaresValidForCxr(*fp, 1, ibCxrKey)))
        continue;
      cxrKey[1] = ibCxrKey;
      generateSolutionsImpl(pqItem, cxrKey, soloTrxData);
    }
  }
}

bool
SoloItinGenerator::checkFaresValidForCxr(const FarePath& fp,
                                         const LegId legId,
                                         const uint32_t cxrKey) const
{
  for (const PricingUnit* pu : fp.pricingUnit())
  {
    for (const FareUsage* fu : pu->fareUsage())
    {
      const PaxTypeFare& ptf = *fu->paxTypeFare();

      if (ptf.fareMarket()->legIndex() != legId)
        continue;
      if (!ptf.isValid())
        return false;
      PaxTypeFare::FlightBitmapPerCarrier::const_iterator bitmapIt =
          ptf.flightBitmapPerCarrier().find(cxrKey);
      if (bitmapIt == ptf.flightBitmapPerCarrier().end() || (*bitmapIt).second.empty())
        return false;
    }
  }

  return true;
}

void
SoloItinGenerator::generateSolutionsImpl(const shpq::SoloPQItemPtr& pqItem,
                                         const shpq::CxrKeyPerLeg& cxrKey,
                                         shpq::SoloTrxData& soloTrxData)
{
  DataHandle scopedDH;
  shpq::SoloAltDatesInfo scopedAltDatesInfo(_trx);

  FarePath* originalFP = pqItem->getFarePath();

  size_t fpKey = 0;
  if (UNLIKELY(_trx.diagnostic().diagnosticType() == Diagnostic941 ||
               _trx.diagnostic().diagnosticType() == Diagnostic942))
    fpKey = hash_value(*originalFP);

  if (UNLIKELY(_dc))
    _dc->printFarePath(originalFP, fpKey);
  BOOST_SCOPE_EXIT((_dc))
  {
    if (UNLIKELY(_dc))
      _dc->flushFarePath();
  }
  BOOST_SCOPE_EXIT_END

  shpq::SOPCombinationBuilder* combBuilder = createSOPCombinationBuilder(scopedDH, pqItem, cxrKey);
  // Build SOP combinations
  DiversityModel::SOPCombinationList& combinations = combBuilder->getOrCreateSOPCombinationList();
  const CarrierKeyVec& carrierKeys = combBuilder->getCxrKeys();
  const std::vector<std::vector<SOPInfo>>& sopInfos = combBuilder->getSOPInfos();

  if (UNLIKELY(_dc))
    _dc->setFPTotalCombNum(combinations.size());

  PaxTypeFareBitmapValidator::SkippedBitValidator* skippedBitValidator(
      PaxTypeFareBitmapValidator::createSoloSkippedBitValidator(
          &_trx, &_bitmapOpOrderer, carrierKeys));

  _initialSolutionsCount = _trx.flightMatrix().size();

  // collected = added or swapped to/within flight matrix
  size_t numSolutionsCollectedFromFP = 0;
  size_t numCustomSolutionsCollectedFromFP = 0;

  DiversityModel::SOPCombinationList::iterator combIt;
  uint32_t failedCombCountPerFP = 0;
  uint32_t failedDelayedBitValidationCountPerFP = 0;
  while (!combinations.empty())
  {
    {
      TSELatencyData::Callback diagCallback;
      if (UNLIKELY(_dc))
      {
        diagCallback = std::bind(&Diag941Collector::logFPDiversityCpuMetrics,
                                 _dc,
                                 std::placeholders::_1,
                                 std::placeholders::_2,
                                 std::placeholders::_3);
      }
      TSELatencyData metrics(_trx, "DIVERSITY MODEL", diagCallback);

      CombinationEraseDelegate diagDelegate(_dc, Diag941Collector::DIVERSITY, combinations);

      combIt = _dm->getDesiredSOPCombination(combinations, originalFP->getTotalNUCAmount(), fpKey);
    }

    if (combIt == combinations.end())
    {
      if (_dc)
      {
        addCombinationResult(
            _dc, combinations.begin(), combinations.end(), Diag941Collector::DIVERSITY);
      }

      break;
    }

    const shpq::SopIdxVec& bitVec = combBuilder->getFlbIdxVec(combIt->oSopVec);

    if (combIt->status != 0)
    {
      size_t legIdx = 0;
      if (!validateDelayedFlightStatuses(
              carrierKeys, bitVec, sopInfos, *combIt, legIdx, skippedBitValidator))
      {
        CombinationEraseDelegate diagDelegate(
            _dc, Diag941Collector::Diag941Collector::DELAYED_FLIGHT_BIT, combinations);
        combinations.removeCombinations(legIdx, combIt->oSopVec[legIdx]);
        if (LIKELY(_maxFailedDelayedBitValidationCountPerFP))
        {
          ++failedDelayedBitValidationCountPerFP;
          if (failedDelayedBitValidationCountPerFP >= _maxFailedDelayedBitValidationCountPerFP)
          {
            if (_dc)
              _dc->printFPFailedMessage("TOO MANY FAILED BIT VALIDATIONS PER FP: DROPPING");

            break; // while (!combinations.empty())
          }
        }
        continue;
      }
    }

    // Send to validateItin
    FarePath* newFP = validateItin(originalFP, carrierKeys, bitVec, combIt->oSopVec);
    if (!newFP)
    {
      if (_maxFailedComb && !_pq.isOnlyThruFM())
      {
        _failedCombCount++;
        if (_failedCombCount >= _maxFailedComb)
        {
          if (_dc)
            _dc->printFPTooManyFailedCombMessage("SWITCHING PQ TO THRU-ONLY MODE");
          _pq.setOnlyThruFM(true);
        }
      }
      if (LIKELY(_maxFailedCombPerFP))
      {
        failedCombCountPerFP++;
        if (failedCombCountPerFP >= _maxFailedCombPerFP)
        {
          if (_dc)
            _dc->printFPTooManyFailedCombMessage("DROPPING");

          break; // while (!combinations.empty())
        }
      }
    }
    else
      _failedCombCount = 0;

    if (newFP && addSolution(*combIt, originalFP, newFP, pqItem, fpKey))
    {
      scopedAltDatesInfo.addSolution(combIt->datePair);
      combinations.clearCombinationsCache();
      if (UNLIKELY(_dc))
      {
        ++numSolutionsCollectedFromFP;
        if (_trx.getNumOfCustomSolutions() && ShoppingUtil::isCustomSolution(_trx, combIt->oSopVec))
        {
          ++numCustomSolutionsCollectedFromFP;
        }
      }
    }

    combinations.erase(combIt);
  } // while (!combinations.empty())

  if (UNLIKELY(_dc))
  {
    if (_trx.isAltDates())
      printSopsPerDateDiagnostic(_trx, _dc, combBuilder->getSopsByDateByLeg());
    _dc->printDelayedFlightBitValidationResult(originalFP, carrierKeys);
    _dc->printCombinationMatrix();
    _dc->printOptsPriceAdjustedByRC();
    *_dc << scopedAltDatesInfo;
    _dc->printFPSolutionsFound(numSolutionsCollectedFromFP, _trx.flightMatrix().size());
    if (_trx.getNumOfCustomSolutions())
      _dc->printFPCustomSolutionsFound(numCustomSolutionsCollectedFromFP);
    _dc->flushNonStopActions();
  }
}

bool
SoloItinGenerator::addSolution(const DiversityModel::SOPCombination& sopComb,
                               FarePath* origFP,
                               FarePath* newFP,
                               const shpq::SoloPQItemPtr& pqItem,
                               std::size_t fpKey)
{
  shpq::SoloGroupFarePath* gfp = buildFakeGroupFarePath(newFP, pqItem, fpKey, sopComb.datePair);

  bool isAcceptedByDiversity = true;
  {
    TSELatencyData metrics(_trx, "DIVERSITY MODEL");

    isAcceptedByDiversity =
        _dm->addSolution(ShoppingTrx::FlightMatrix::value_type(sopComb.oSopVec, gfp),
                         _trx.flightMatrix(),
                         fpKey,
                         sopComb.datePair);
  }

  if (isAcceptedByDiversity)
  {
    bool thruOnlyFlag = false;
    bool asoCandidate = false;

    if (_asoCandidateChecker.match(sopComb.oSopVec))
      asoCandidate = true;
    else
      setProcessThruOnlyHint(sopComb.oSopVec, gfp);

    if (LIKELY(_changeFamilyGroupingForThruOnly))
    {
      thruOnlyFlag =
          (_trx.getSolNoLocalInd() || (_pq.isOnlyThruFM() && !gfp->getProcessThruOnlyHint()));
    }

    _familyGroupingInfo.setOnlyThruFlag(origFP, thruOnlyFlag);
    _familyGroupingInfo.addBaseFarePath(
        origFP, newFP, asoCandidate, gfp->getProcessThruOnlyHint()); // store origin fare path for
    // family grouping
  }
  else if (_dc)
  {
    _dc->addCombinationResult(sopComb.oSopVec, Diag941Collector::DIVERSITY);
  }

  return isAcceptedByDiversity;
}

void
SoloItinGenerator::generateEstimatedSolutions()
{
  TSELatencyData metrics(_trx, "SOLO GENERATE ESTIMATED SOLUTIONS");

  if (_trx.flightMatrix().empty() || _trx.isAltDates() ||
      _trx.getRequest()->isBrandedFaresRequest())
    return;

  // FamilyGrouping Options:
  // 0 - disable family grouping
  // 1 - group thru and local farepaths the same way (default option)
  // 2 - use sops comparison only for grouping local farepaths.
  uint32_t familyGroupingOpt = familyGroupingOptCfg.getValue();

  const shpq::SoloFamilyGrouping::GroupingOption groupingOption =
      shpq::SoloFamilyGrouping::getGroupingOption(familyGroupingOpt);
  if (groupingOption == shpq::SoloFamilyGrouping::DISABLE_GROUPING)
    return;

  ShoppingTrx::FlightMatrix flightMatrixCopy;
  flightMatrixCopy.swap(_trx.flightMatrix());

  SimilarSolutionsMap solutionsMap, asoSolutionsMap, hintSolutionsMap, thruOnlySolutions;
  for (ShoppingTrx::FlightMatrix::iterator it = flightMatrixCopy.begin(),
                                           endIt = flightMatrixCopy.end();
       it != endIt;
       ++it)
  {
    if (LIKELY(it->second))
    {
      GroupFarePath* groupFarePath = it->second;
      FarePath* farePath = groupFarePath->groupFPPQItem().front()->farePath();
      FarePath* baseFarePath = _familyGroupingInfo.getBaseFarePath(farePath);
      if (LIKELY(baseFarePath))
      {
        if (_familyGroupingInfo.getAsoStatus(farePath))
          asoSolutionsMap[baseFarePath].push_back(*it); // grouping:1
        else if (_familyGroupingInfo.isOnlyThruFlag(baseFarePath))
          thruOnlySolutions[baseFarePath].push_back(*it); // grouping:1
        else if (_familyGroupingInfo.getHintStatus(farePath))
          hintSolutionsMap[baseFarePath].push_back(*it); // grouping:2
        else
          solutionsMap[baseFarePath].push_back(*it); // grouping:default
      }
    }
  }

  // regular FP processing
  groupFPSolutions(solutionsMap, familyGroupingOpt);
  // aso processing
  groupFPSolutions(asoSolutionsMap, shpq::SoloFamilyGrouping::GROUP_ALL_SOLUTIONS);
  // thruOnly processing
  groupFPSolutions(thruOnlySolutions, shpq::SoloFamilyGrouping::GROUP_ALL_SOLUTIONS);
  // hint processing
  groupFPSolutions(hintSolutionsMap, shpq::SoloFamilyGrouping::GROUP_SOL_FAREPATHS_ONLY);
}

void
SoloItinGenerator::flushDiag()
{
  if (_dc)
    _dc->flushFPDiversityCpuMetrics();
}

void
SoloItinGenerator::groupFPSolutions(SimilarSolutionsMap& solutionsMap, uint32_t familyGroupingOpt)
{
  const shpq::SoloFamilyGrouping::GroupingOption groupingOption =
      shpq::SoloFamilyGrouping::getGroupingOption(familyGroupingOpt);

  for (SimilarSolutionsMap::iterator it = solutionsMap.begin(), endIt = solutionsMap.end();
       it != endIt;
       ++it)
  {
    FarePath* farePath = it->first;
    SolutionVector& solutionsVector = it->second;
    shpq::SoloFamilyGrouping fGrouping(_trx, groupingOption, farePath);
    fGrouping.process(solutionsVector);
  }
}

shpq::SoloGroupFarePath*
SoloItinGenerator::buildFakeGroupFarePath(FarePath* fp,
                                          const shpq::SoloPQItemPtr& origPQItem,
                                          size_t fpKey,
                                          const DatePair* datePair) const
{
  // Create fake GroupFarePath
  shpq::SoloGroupFarePath* gfp;
  if (_trx.isAltDates())
  {
    gfp = &_trx.dataHandle().safe_create<shpq::SoloADGroupFarePath>(*datePair);
  }
  else
  {
    _trx.dataHandle().get(gfp);
  }

  FPPQItem* fppqItem = _trx.dataHandle().create<FPPQItem>();
  fppqItem->farePath() = fp;
  gfp->groupFPPQItem().push_back(fppqItem);
  gfp->setTotalNUCAmount(fp->getTotalNUCAmount());
  gfp->setTotalNUCBaseFareAmount(fp->getTotalNUCAmount());
  gfp->setSolutionPattern(origPQItem->getSolPattern());
  gfp->setFPKey(fpKey);

  return gfp;
}

void
SoloItinGenerator::setProcessThruOnlyHint(shpq::SopIdxVecArg sopVec, shpq::SoloGroupFarePath* gfp)
{
  // Hint is disabled by default
  if (!gfp->getSolutionPattern()->isThruPattern() || _trx.getSolNoLocalInd())
    return;

  const Loc& origin = *_trx.legs()[0].sop()[sopVec[0]].itin()->travelSeg().front()->origin();

  if (UNLIKELY(LocUtil::isInZone(origin, Vendor::SABRE, SABRE_WESTERN_AFRICA, MANUAL) ||
               LocUtil::isInLoc(origin, LOCTYPE_NATION, NATION_ISRAEL, Vendor::SABRE, MANUAL)))
    return;

  if (_pq.isOnlyThruFM())
  {
    double thruFarePopulationRate =
        (double)_stats.getNumOfThruFPOptions() / _stats.getTotalOptionsCount();
    if (UNLIKELY(thruFarePopulationRate < _thruFarePopulationRateLimit))
      return;
  }

  if (!isHintAllowed())
    return;

  gfp->setProcessThruOnlyHint(true);
}

bool
SoloItinGenerator::isHintAllowed()
{
  if (UNLIKELY(!_hintingStartsAt))
    return true;

  if (_initialSolutionsCount >= _hintingStartsAt)
    return true;
  return false;
}

ScopedFMGlobalDirSetter*
SoloItinGenerator::createGlobalDirSetter(PaxTypeFare* fare, uint32_t legIndex, int sopIndex) const
{
  GlobalDirection* globalDir(nullptr);
  if (fare->fareMarket()->getFmTypeSol() != FareMarket::SOL_FM_LOCAL)
    globalDir = &_trx.legs()[legIndex].sop()[sopIndex].globalDirection();

  return new ScopedFMGlobalDirSetter(&_trx, fare->fareMarket(), globalDir);
}

void
SoloItinGenerator::initializePricingUnitsSegments(
    FarePath* farePath,
    const SoloItinGenerator::CarrierKeyVec& carrierKeys,
    shpq::SopIdxVecArg sopVec,
    shpq::SopIdxVecArg oSopVec,
    std::vector<ScopedFMGlobalDirSetterPtr>& globalDirSetters) const
{
  for (PricingUnit* pricingUnit : farePath->pricingUnit())
  {
    std::set<TravelSeg*> puSegs;
    for (FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      FareMarket* fareMarket = fareUsage->paxTypeFare()->fareMarket();
      size_t legIndex = fareMarket->legIndex();

      const SOPUsage& sopUsage =
          (*(fareMarket->getApplicableSOPs()))[carrierKeys[legIndex]][sopVec[legIndex]];

      // It works, because travel segments are stored in vector
      TravelSeg** firstTS = &sopUsage.itin_->travelSeg()[sopUsage.startSegment_];
      TravelSeg** lastTS = firstTS + (sopUsage.endSegment_ - sopUsage.startSegment_ + 1);
      fareUsage->travelSeg().assign(firstTS, lastTS);
      fareMarket->travelSeg() = fareUsage->travelSeg();

      // setup correct global direction in fare market
      globalDirSetters.push_back(ScopedFMGlobalDirSetterPtr(
          createGlobalDirSetter(fareUsage->paxTypeFare(), legIndex, oSopVec[legIndex])));

      puSegs.insert(firstTS, lastTS);
    }

    pricingUnit->travelSeg().clear();

    for (TravelSeg* travelSegment : farePath->itin()->travelSeg())
    {
      if (puSegs.find(travelSegment) != puSegs.end())
        pricingUnit->travelSeg().push_back(travelSegment);
    }
  }
}

bool
SoloItinGenerator::validateCarrierRestrictions(std::vector<FareUsage*>& fareUsages,
                                               shpq::SopIdxVecArg oSopVec,
                                               const Itin* itin) const
{
  ValidateCarrierRestrictionsPtfVisitor validatingVisitor(*this, oSopVec, itin);

  for (FareUsage* fareUsage : fareUsages)
  {
    const PaxTypeFare* fare = fareUsage->paxTypeFare();
    if (!validatingVisitor.visitAndCheck(fare))
      return false;
  }

  return true;
}

bool
SoloItinGenerator::isLocalJourneyCarrier(const std::vector<TravelSeg*>& tSegs) const
{
  std::set<CarrierCode> carriers;

  for (TravelSeg* tSeg : tSegs)
  {
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(tSeg);
    if (airSeg && !airSeg->localJourneyCarrier())
      return false;

    carriers.insert(airSeg->carrier());
  }

  return (carriers.size() == 1);
}

bool
SoloItinGenerator::checkAvailibility(const std::vector<TravelSeg*>& tSegs,
                                     const std::vector<BookingCode>& bookingCodes,
                                     const uint16_t segIndex) const
{
  std::vector<ClassOfServiceList>& cos = ShoppingUtil::getClassOfService(_trx, tSegs);
  if (cos.empty() || (cos.size() != tSegs.size()))
    return true;

  uint16_t requestedNumberOfSeats = PaxTypeUtil::totalNumSeats(_trx);
  std::vector<TravelSeg*>::const_iterator tvlI = tSegs.begin();
  std::vector<TravelSeg*>::const_iterator tvlIEnd = tSegs.end();

  for (uint16_t index = 0; tvlI != tvlIEnd; ++tvlI, ++index)
  {
    TravelSeg* tSeg = *tvlI;
    if (tSeg->segmentType() == Arunk)
      continue;

    BookingCode bookingCode = bookingCodes[segIndex + index];
    ClassOfServiceList& cosList = cos[index];

    bool bookingValid = false;
    std::vector<ClassOfService*>::iterator cosI = cosList.begin();
    std::vector<ClassOfService*>::iterator cosIEnd = cosList.end();
    for (; cosI != cosIEnd; ++cosI)
    {
      if (((*cosI)->bookingCode() == bookingCode) &&
          ((*cosI)->numSeats() >= requestedNumberOfSeats))
      {
        bookingValid = true;
        break;
      }
    }

    if (!bookingValid)
      return false;
  }

  return true;
}

uint16_t
SoloItinGenerator::flowLength(const std::vector<TravelSeg*> tSegs,
                              uint16_t startId,
                              const std::vector<BookingCode>& bookingCodes) const
{
  uint16_t flowLen = 1;
  uint16_t segSize = tSegs.size();

  TravelSeg* firstSeg = tSegs[startId];
  if (firstSeg->segmentType() == Arunk)
  {
    ++startId;
    if (startId == segSize)
      return 1;

    ++flowLen;
  }

  BookingCode bookingCode = bookingCodes[startId];

  for (uint16_t i = startId + 1; i < segSize; ++i)
  {
    TravelSeg* tSeg = tSegs[i];

    if (tSeg->segmentType() == Arunk)
    {
      if (tSeg->arunkMultiAirportForAvailability())
      {
        ++flowLen;
        continue;
      }
      else
        return flowLen;
    }

    if (bookingCodes[i] == bookingCode)
      ++flowLen;
    else
      return flowLen;
  }

  return flowLen;
}

void
SoloItinGenerator::getBookingCodes(const std::vector<int>& oSopVec,
                                   const std::vector<TravelSeg*>& tSegs,
                                   const PaxTypeFare* ptf,
                                   const uint16_t legIdx,
                                   std::vector<BookingCode>& bookingCodes) const
{
  ShoppingUtil::ExternalSopId id = ShoppingUtil::createExternalSopId(legIdx, oSopVec[legIdx]);
  uint32_t bitMapNumber = ShoppingUtil::getFlightBitIndex(_trx, id);
  TSE_ASSERT(bitMapNumber < ptf->flightBitmap().size());

  const PaxTypeFare::FlightBit& bitMap = ptf->flightBitmap()[bitMapNumber];

  if (bitMap._segmentStatus.empty())
  {
    for (const TravelSeg* tSeg : tSegs)
    {
      if (tSeg->segmentType() == Arunk)
        bookingCodes.push_back("");
      else
        bookingCodes.push_back(tSeg->getBookingCode());
    }
  }
  else
  {
    uint16_t tvlSegSize = tSegs.size();

    std::vector<PaxTypeFare::SegmentStatus>::const_iterator i = bitMap._segmentStatus.begin();
    std::vector<PaxTypeFare::SegmentStatus>::const_iterator iEnd = bitMap._segmentStatus.end();

    for (uint16_t tvlItem = 0; i != iEnd; ++i, ++tvlItem)
    {
      TSE_ASSERT(tvlItem < tvlSegSize);

      const PaxTypeFare::SegmentStatus& segItem = *i;

      if (tSegs[tvlItem]->segmentType() == Arunk)
        bookingCodes.push_back("");
      else if (segItem._bkgCodeReBook.empty())
        bookingCodes.push_back(tSegs[tvlItem]->getBookingCode());
      else
        bookingCodes.push_back(segItem._bkgCodeReBook);
    }
  }
}

bool
SoloItinGenerator::validateJourneyRestrictions(const std::vector<FareUsage*>& fareUsages,
                                               const Itin* itin,
                                               const std::vector<int>& oSopVec) const
{
  if (fallback::fallbackISAvailTuning(&_trx))
    return true;

  for (FareUsage* fareUsage : fareUsages)
  {
    const PaxTypeFare* ptf = fareUsage->paxTypeFare();
    const FareMarket* fm = ptf->fareMarket();
    if (fm->getFmTypeSol() != FareMarket::SOL_FM_THRU)
      continue;

    uint16_t legIdx = fm->legIndex();
    const ShoppingTrx::SchedulingOption& sop = _trx.legs()[legIdx].sop()[oSopVec[legIdx]];
    if (sop.isLngCnxSop())
      continue;

    const std::vector<TravelSeg*> tSegs = fm->travelSeg();
    if (!isLocalJourneyCarrier(tSegs))
      continue;

    std::vector<BookingCode> bookingCodes;
    getBookingCodes(oSopVec, tSegs, ptf, legIdx, bookingCodes);
    uint16_t segIndex = 0;
    uint16_t segSize = tSegs.size();

    while (segIndex < segSize)
    {
      uint16_t flowLen = flowLength(tSegs, segIndex, bookingCodes);
      uint16_t lastIndex = segIndex + flowLen;

      TSE_ASSERT(lastIndex <= segSize);

      std::vector<TravelSeg*> journeySegs(tSegs.begin() + segIndex, tSegs.begin() + lastIndex);

      if (!checkAvailibility(journeySegs, bookingCodes, segIndex))
        return false;

      segIndex = lastIndex;
    }
  }

  return true;
}

FarePath*
SoloItinGenerator::validateItin(const FarePath* fp,
                                const SoloItinGenerator::CarrierKeyVec& carrierKeys,
                                shpq::SopIdxVecArg sopVec,
                                shpq::SopIdxVecArg oSopVec) const
{
  TSELatencyData metrics(_trx, "SOLO VALIDATE ITIN");

  // Constructing our "brand-new" itin from given SOP combination
  Itin* itin = _trx.dataHandle().create<Itin>();

  for (size_t legIdx = 0; legIdx < oSopVec.size(); ++legIdx)
  {
    const Itin* sopItin = _trx.legs()[legIdx].sop()[oSopVec[legIdx]].itin();
    itin->travelSeg().insert(
        itin->travelSeg().end(), sopItin->travelSeg().begin(), sopItin->travelSeg().end());
  }

  itin->setTravelDate(itin->travelSeg().front()->departureDT());
  // Set fare markets to itin
  itin->fareMarket() = fp->itin()->fareMarket();
  // Set itin calculation currency
  itin->calculationCurrency() = fp->itin()->calculationCurrency();
  // Set geoTraveType to itin
  TravelSegAnalysis tvlSegAnalysis;
  Boundary tvlBoundary = tvlSegAnalysis.selectTravelBoundary(itin->travelSeg());
  ItinUtil::setGeoTravelType(tvlSegAnalysis, tvlBoundary, *itin);
  // Set validating carrier
  ValidatingCarrierUpdater valCxrUpdater(_trx);
  const std::vector<int>& tempSopVec = oSopVec;
  valCxrUpdater.update(*itin, false, &tempSopVec);

  if (_trx.isValidatingCxrGsaApplicable())
  {
    bool isValCxrFound = !itin->validatingCarrier().empty();
    if (fallback::fallbackValidatingCxrGTC(&_trx))
      isValCxrFound = (itin->validatingCxrGsaData() != nullptr);

    if (!isValCxrFound)
    {
      if (UNLIKELY(_dc))
        _dc->addCombinationResult(oSopVec, Diag941Collector::VALIDATE_TICKETING_INTERLINE);

      return nullptr;
    }
  }
  else if (!validateInterlineTicketCarrierAgreement(oSopVec, *itin))
    return nullptr;

  // As we get here, we need to construct new farepath
  FarePath* newFP = FarePathCopier(_trx.dataHandle()).getDuplicate(*fp);

  // Assign "brand-new" itin to fare path
  newFP->itin() = itin;
  newFP->baseFareCurrency() = itin->calculationCurrency();
  newFP->calculationCurrency() = itin->calculationCurrency();
  size_t puCount = newFP->pricingUnit().size();

  std::vector<ScopedFMGlobalDirSetterPtr> globalDirSetters;
  globalDirSetters.reserve(puCount * 2 /*avg no of components*/);

  initializePricingUnitsSegments(newFP, carrierKeys, sopVec, oSopVec, globalDirSetters);

  bool validationFailed = false;
  RuleControllerWithChancelor<PricingUnitRuleController> rc(
      (_trx.isAltDates() ? ShoppingAltDateItinBasedValidation : ShoppingItinBasedValidation));

  if (!fallback::ngsMaximumPenaltyFarePathValidation(&_trx) && !validateMaximumPenalty(*newFP))
  {
    validationFailed = true;

    if (_dc)
      _dc->addMaximumPenaltyFailedFarePath(*newFP);
  }

  if (!validationFailed)
    for (size_t puIdx = 0; puIdx < puCount; ++puIdx)
    {
      PricingUnit& pu = *(newFP->pricingUnit()[puIdx]);

      // Validate Record 8 same carrier/Table 986
      if (!validateCarrierRestrictions(pu.fareUsage(), oSopVec, itin))
      {
        validationFailed = true;
        break;
      }

      if (UNLIKELY(!validateJourneyRestrictions(pu.fareUsage(), itin, oSopVec)))
      {
        validationFailed = true;
        break;
      }

      // clear total transfer for each pu
      pu.totalTransfers() = 0;
      pu.mostRestrictiveMaxTransfer() = -1;

      if (!rc.validate(_trx, *newFP, pu))
      {
        validationFailed = true;

        if (UNLIKELY(_dc))
          _dc->addCombinationResult(oSopVec, Diag941Collector::RULE_CONTROLLER);

        break;
      }
    }

  if (UNLIKELY(!validationFailed && _dc))
  {
    _dc->addCombinationResult(oSopVec, Diag941Collector::PASSED);
    _dc->addOptPriceAdjustedByRC(oSopVec, *newFP, *fp);
  }

  // TODO: drop newFP if validation failed
  return validationFailed ? nullptr : newFP;
}

//---------------------------------------------------------------------
// Validate the interline ticketing (VITA)
//
// The idea of the VITA is to verify whether all carrier in the same itin
// has the interline ticketing agreement with the validating carrier or not.
// If it does not have the agreement, shopping will not return it when
// the request has PRO/@VTI="T".
//
// Returns true if the validation passed or it is not applicable.
//---------------------------------------------------------------------
bool
SoloItinGenerator::validateInterlineTicketCarrierAgreement(shpq::SopIdxVecArg sopVec,
                                                           const Itin& itin) const
{
  // _interlineTicketCarrierData != NULL if PRO/@SEV="T" and PRO/@VTI="T"
  // PRO/@SEV (processVITAData) is the activation flag for the VITA
  // PRO/@VTI (validateTicketingAgreement) controls interline ticketing agreement validation
  if (!_interlineTicketCarrierData)
    return true;

  std::string validationMessage;
  bool result = _interlineTicketCarrierData->validateInterlineTicketCarrierAgreement(
      _trx, itin.validatingCarrier(), itin.travelSeg(), &validationMessage);

  if (UNLIKELY(_dc))
  {
    _dc->printVITA(sopVec, *_interlineTicketCarrierData, itin, result, validationMessage);
    if (!result)
      _dc->addCombinationResult(sopVec, Diag941Collector::VALIDATE_TICKETING_INTERLINE);
  }
  return result;
}

shpq::SOPCombinationBuilder*
SoloItinGenerator::createSOPCombinationBuilder(DataHandle& dh,
                                               const shpq::SoloPQItemPtr& pqItem,
                                               const shpq::CxrKeyPerLeg& cxrKey)
{
  const bool processDirectFlightsOnly = _dm->isNonStopNeededOnlyFrom(pqItem.get());

  return &dh.safe_create<shpq::SOPCombinationBuilder>(
      pqItem, cxrKey, processDirectFlightsOnly, *_dm, _dc, _trx);
}

bool
SoloItinGenerator::validateMaximumPenalty(FarePath& fp) const
{
  MaximumPenaltyValidator maxPenValidator(_trx);

  return maxPenValidator.validateFarePath(fp).first;
}

void
SoloItinGenerator::initializeDiagnostic()
{
  if (_trx.diagnostic().diagnosticType() == Diagnostic941 ||
      _trx.diagnostic().diagnosticType() == Diagnostic942)
  {
    initializeDiagHashFunc();
  }
}

void
SoloItinGenerator::initializeAcmsVars()
{
  _thruFarePopulationRateLimit = thruFarePopulationRateLimit.getValue();
  _changeFamilyGroupingForThruOnly = changeFamilyGroupingForThruOnly.getValue();
  _hintingStartsAt = hintingStartsAt.getValue();
  _maxFailedDelayedBitValidationCountPerFP = maxFailedDelayedBitValidationCountPerFP.getValue();
  _maxFailedCombPerFP = maxFailedCombPerFP.getValue();
  _maxFailedComb = maxFailedComb.getValue();
}

void
SoloItinGenerator::initializeDiagHashFunc()
{
  const std::string& diagFPHashFuncVersion = _trx.diagnostic().diagParamMapItem("FPHASH");
  if (!diagFPHashFuncVersion.empty())
  {
    try
    {
      _fpHashFuncVer =
          static_cast<FPHashFuncVersion>(boost::lexical_cast<int>(diagFPHashFuncVersion));
    }
    catch (const boost::bad_lexical_cast&)
    {
      LOG4CXX_ERROR(_logger, "Invalid diag param fare path hash function version specified");
      LOG4CXX_DEBUG(_logger, "Using fare path hash function version " << _fpHashFuncVer);
    }
  }
}

size_t
SoloItinGenerator::hash_value(const FarePath& fp) const
{
  // round up to 2 digits after the point
  int totalNucAmountRounded = static_cast<int>(fp.getTotalNUCAmount() * 100.);

  if (FP_HASH_FUNC_VER_1 == _fpHashFuncVer)
  {
    size_t seed = boost::hash_value(totalNucAmountRounded);
    for (size_t puIdx = 0; puIdx < fp.pricingUnit().size(); ++puIdx)
    {
      PricingUnit* pu = fp.pricingUnit()[puIdx];
      for (size_t fuIdx = 0; fuIdx < pu->fareUsage().size(); ++fuIdx)
      {
        PaxTypeFare* ptf = pu->fareUsage()[fuIdx]->paxTypeFare();
        boost::hash_combine(seed, ptf->fareClass());
      }
    }

    return seed;
  }
  else
  {
    Hasher hasher(HasherMethod::METHOD_0);

    hasher << totalNucAmountRounded;
    for (size_t puIdx = 0; puIdx < fp.pricingUnit().size(); ++puIdx)
    {
      PricingUnit* pu = fp.pricingUnit()[puIdx];
      for (size_t fuIdx = 0; fuIdx < pu->fareUsage().size(); ++fuIdx)
      {
        PaxTypeFare* ptf = pu->fareUsage()[fuIdx]->paxTypeFare();
        hasher << ptf->fareClass();

        hash_carriers(*ptf, hasher);
      }
    }

    return hasher.hash();
  }
}

void
SoloItinGenerator::hash_carriers(const PaxTypeFare& ptf, Hasher& hasher) const
{
  typedef VecMap<uint32_t, PaxTypeFare::FlightBitmap>::value_type VecMapItem;
  for (const VecMapItem& item : ptf.flightBitmapPerCarrier())
  {
    const std::map<ItinIndex::Key, CarrierCode>& cxrMap = _trx.diversity().getCarrierMap();
    std::map<ItinIndex::Key, CarrierCode>::const_iterator cxrIt = cxrMap.find(item.first);
    TSE_ASSERT(cxrIt != cxrMap.end() || !"bad carrier key");

    hasher << cxrIt->second;
  }
}

} /* namespace tse */
