#include "Pricing/Shopping/Diversity/DiversityItinerarySwapper.h"

#include "Common/ShoppingUtil.h"
#include "DataModel/Diversity.h"
#include "Diagnostic/Diag941Collector.h"
#include "Diagnostic/DiagCollector.h"
#include "Pricing/Shopping/Diversity/DsciDiversityItinerarySwapperAdapter.h"
#include "Pricing/Shopping/Diversity/SwapperBasicValidationStrategy.h"
#include "Pricing/Shopping/Diversity/SwapperIBFValidationStrategy.h"
#include "Pricing/Shopping/Diversity/SwapperTypes.h"
#include "Pricing/Shopping/Diversity/SwapperView.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"

#define ARRSIZE(x) (sizeof(x) / sizeof(0 [x]))

namespace tse
{

SwapperEvaluationResult
convertInterpreterResult(bool result, const DsciDiversityItinerarySwapperAdapter& interpreter)
{
  if (result)
    return SwapperEvaluationResult::SELECTED;

  if (interpreter.isTodDistanceResultLastUsed())
    return SwapperEvaluationResult::TOD_DISTANCE;
  else
    return SwapperEvaluationResult::NONE;
}

DiversityItinerarySwapper::DiversityItinerarySwapper(ShoppingTrx& trx,
                                                     ItinStatistic& stats,
                                                     DiversityModel* model,
                                                     DiagCollector* dc)
  : _trx(trx),
    _dc(nullptr),
    _dc941(nullptr),
    _model(model),
    _stats(stats),
    _flightMatrix(trx.flightMatrix()),
    _diversity(trx.diversity()),
    _validationStrategy(nullptr),
    _swapperView(nullptr),
    _fpKey(0),
    _diagSwapper(false),
    _diagNonStops(false),
    _isBrandedFaresPath(trx.getRequest()->isBrandedFaresRequest())
{
  if (_isBrandedFaresPath)
    _validationStrategy = &_trx.dataHandle().safe_create<SwapperIBFValidationStrategy>(trx, stats);
  else
    _validationStrategy =
        &_trx.dataHandle().safe_create<SwapperBasicValidationStrategy>(trx, stats, model);
  _swapperView = &_trx.dataHandle().safe_create<SwapperView>(*_validationStrategy);

  DiagnosticTypes diagType(trx.diagnostic().diagnosticType());
  const std::string& diagArg(trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL));

  if (diagType == Diagnostic941)
    _dc941 = dynamic_cast<Diag941Collector*>(dc);
  else if (diagType == Diagnostic942)
    _dc = dc;

  if (_dc)
  {
    if (diagArg == "SWAPPER")
      _diagSwapper = true;
    else if (diagArg == "NONSTOPS")
      _diagNonStops = true;
    else if (diagArg == "ALL")
      _diagSwapper = _diagNonStops = true;

    if (_diagSwapper)
      _fpKey = strtoul(trx.diagnostic().diagParamMapItem(Diagnostic::FARE_PATH).c_str(), nullptr, 16);
  }
}

void
DiversityItinerarySwapper::swapSolution(const ShoppingTrx::FlightMatrix::value_type& solution,
                                        size_t fpKey)
{
  const ShoppingTrx::SchedulingOption* newOutbound = nullptr;
  const ShoppingTrx::SchedulingOption* newInbound = nullptr;
  SopCombinationUtil::getSops(_trx, solution.first, &newOutbound, &newInbound);
  SopCombinationUtil::NonStopType newNonStop =
      SopCombinationUtil::detectNonStop(newOutbound, newInbound);

  NewSolutionAttributes newSolution;
  newSolution.carrier = SopCombinationUtil::detectCarrier(newOutbound, newInbound);
  newSolution.isNonStop = (newNonStop & SopCombinationUtil::ONLINE_NON_STOP);
  newSolution.bucket =
      _stats.detectBucket(newOutbound, newInbound, solution.second->getTotalNUCAmount());

  //Adjust bucket ditribution if carrier in DCL map
  //Adjust bucket ditribution if carrier in DCL map
    _stats.adjustBucketDistribution(newSolution.carrier);

  newSolution.todBucket = newOutbound->itin()->getTODBucket(_diversity.getTODRanges());

  Diversity::BucketType bucketsToCheck[] = { Diversity::JUNK, Diversity::UGLY, Diversity::LUXURY,
                                             Diversity::GOLD, newSolution.bucket };

  printSwapSolution(fpKey,
                    solution.first,
                    newSolution.carrier,
                    newNonStop,
                    newSolution.bucket,
                    newSolution.todBucket);
  const ItinStatistic::Solution* worstSol = nullptr;

  const size_t bktToCheckCnt = sizeof(bucketsToCheck) / sizeof(Diversity::BucketType);
  for (size_t bktIdx = 0; bktIdx < bktToCheckCnt; bktIdx++)
  {
    Diversity::BucketType bucket = bucketsToCheck[bktIdx];
    int bktStatus = _model->getBucketStatus(bucket);
    printStartBucket(fpKey, bucket, bktStatus);

    if (bktIdx != bktToCheckCnt - 1 && bktStatus <= 0)
      continue;
    if (bktIdx == bktToCheckCnt - 1 && bktStatus != 0)
      continue;

    const ItinStatistic::BucketPairing& bktPairing = _stats.getBucketPairing(bucket);
    const ShoppingTrx::SchedulingOption* worstOutbound = nullptr;
    const ShoppingTrx::SchedulingOption* worstInbound = nullptr;
    worstSol = nullptr;

    _validationStrategy->setNewSolutionAttributes(&newSolution);
    _validationStrategy->setCurrentBucket(bucket);

    DsciDiversityItinerarySwapperAdapter interpreter(
        _trx, _stats, newSolution.todBucket, bucket, _isBrandedFaresPath);

    for (ItinStatistic::BucketPairing::const_iterator bucketPairingIt = bktPairing.begin();
         bucketPairingIt != bktPairing.end();
         ++bucketPairingIt)
    {
      size_t pairing = bucketPairingIt->first;

      SwapperView::Iterator solutionIt = _swapperView->initialize(bucketPairingIt->second);
      SwapperEvaluationResult oldSolutionResult = SwapperEvaluationResult::SELECTED;
      const ItinStatistic::Solution* oldSolution = nullptr;

      while (_swapperView->getNextSolution(solutionIt, oldSolutionResult, oldSolution))
      {
        if (oldSolutionResult != SwapperEvaluationResult::SELECTED)
        {
          printSolutionEvaluation(fpKey,
                                  oldSolution->first,
                                  pairing,
                                  newSolution.todBucket,
                                  bucket,
                                  oldSolution->second->getTotalNUCAmount(),
                                  oldSolutionResult);
          continue;
        }

        const ShoppingTrx::SchedulingOption* oldOutbound = nullptr;
        const ShoppingTrx::SchedulingOption* oldInbound = nullptr;
        SopCombinationUtil::getSops(_trx, oldSolution->first, &oldOutbound, &oldInbound);

        // Got first candidate
        if (worstSol == nullptr)
        {
          printSolutionEvaluation(fpKey,
                                  oldSolution->first,
                                  pairing,
                                  newSolution.todBucket,
                                  bucket,
                                  oldSolution->second->getTotalNUCAmount(),
                                  SwapperEvaluationResult::SELECTED);
          worstSol = oldSolution;
          worstOutbound = oldOutbound;
          worstInbound = oldInbound;
          // If branded Fares Path it's enough that the options is interline to remove it
          if (_isBrandedFaresPath && !ShoppingUtil::isOnlineOptionForCarrier(
                                         _trx, worstSol->first, _stats.getRequestingCarrier()))
            break;
          else
            continue;
        }

        // we have at least one candidate, so worstOutbound, worstInbound and worstSol are not null
        DsciDiversityItinerarySwapperAdapter::Operand lhs(
            oldOutbound, oldInbound, oldSolution->second->getTotalNUCAmount()),
            rhs(worstOutbound, worstInbound, worstSol->second->getTotalNUCAmount());
        bool shouldBeSwapped = interpreter(lhs, rhs);

        printSolutionEvaluation(fpKey,
                                oldSolution->first,
                                pairing,
                                newSolution.todBucket,
                                bucket,
                                oldSolution->second->getTotalNUCAmount(),
                                convertInterpreterResult(shouldBeSwapped, interpreter));

        if (shouldBeSwapped)
        {
          worstSol = oldSolution;
          worstOutbound = oldOutbound;
          worstInbound = oldInbound;

          if (UNLIKELY(_isBrandedFaresPath))
            break;
        }
      }

      if (worstSol != nullptr)
      {
        removeSolution(bucket, pairing, worstSol, worstOutbound, worstInbound);
        return;
      }
    }
  }

  printNoSolutionForRemoval();
}

bool
DiversityItinerarySwapper::swapAdditionalNS(const ShoppingTrx::FlightMatrix::value_type& solution)
{
  typedef Diversity::NSBucketType NSBucketType;
  typedef ItinStatistic::Solution Solution;
  typedef ItinStatistic::BucketPairing BucketPairing;

  const ShoppingTrx::SchedulingOption* newOutbound = nullptr;
  const ShoppingTrx::SchedulingOption* newInbound = nullptr;
  SopCombinationUtil::getSops(_trx, solution.first, &newOutbound, &newInbound);

  NSBucketType newBucket = _stats.detectNSBucket(newOutbound, newInbound);
  size_t newScore = static_cast<size_t>(solution.second->getTotalNUCAmount());

  const Solution* worstSolution = nullptr;
  size_t worstScore = std::numeric_limits<size_t>::min(); // less is better
  NSBucketType worstBucket = Diversity::NSINTERLINE; // the value doesn't matter
  size_t worstPairing = 0;

  NSBucketType bucketsToCheck[] = { Diversity::NSINTERLINE, Diversity::NSONLINE };

  // iterate over buckets
  for (uint32_t i = 0; i < ARRSIZE(bucketsToCheck); i++)
  {
    NSBucketType curBucket = bucketsToCheck[i];
    const BucketPairing& curBucketPairing = _stats.getNSBucketPairing(curBucket);
    bool swapSameScoreAsNew =
        (newBucket == Diversity::NSONLINE && curBucket == Diversity::NSINTERLINE);

    // bpIt = <pairing, scored combinations>
    // Note that scored combinations are descending sorted by score (price in this case)
    for (const BucketPairing::value_type& bpIt : curBucketPairing)
    {
      size_t minScore = swapSameScoreAsNew ? newScore : newScore + 1;
      if (worstSolution)
      {
        bool swapSameScoreAsWorst =
            (worstBucket == Diversity::NSONLINE && curBucket == Diversity::NSINTERLINE);
        minScore = std::max(minScore, swapSameScoreAsWorst ? worstScore : worstScore + 1);
      }

      if (getMaxScoredSolution(bpIt.second, minScore, &worstSolution, &worstScore))
      {
        worstBucket = curBucket;
        worstPairing = bpIt.first;
      }
    }
  }

  if (worstSolution)
  {
    removeAdditionalNS(worstBucket, worstPairing, worstSolution);
    return true;
  }

  return false;
}

float
DiversityItinerarySwapper::calcTODDistance(size_t decreasedBkt, size_t increasedBkt) const
{
  float result = 0.0;
  for (size_t bkt = 0; bkt < _diversity.getTODDistribution().size(); bkt++)
  {
    size_t bktCount = _stats.getTODBucketSize(bkt);
    if (bkt == decreasedBkt)
      bktCount--;
    if (bkt == increasedBkt)
      bktCount++;
    size_t totalCount = _diversity.getNumberOfOptionsToGenerate();
    if (_stats.considerAdditionalNsInTodAndOIPairing())
      totalCount += _stats.getAdditionalNonStopsCount();
    float bktDif = (float)(bktCount) / totalCount - _diversity.getTODDistribution()[bkt];
    result += bktDif * bktDif;
  }
  return sqrtf(result);
}

double
DiversityItinerarySwapper::calcSolutionScore(Diversity::BucketType bucket,
                                             MoneyAmount price,
                                             const ShoppingTrx::SchedulingOption* outbound,
                                             const ShoppingTrx::SchedulingOption* inbound) const
{
  double result = 0.0;
  int32_t duration = SopCombinationUtil::getDuration(outbound, inbound);

  switch (bucket)
  {
  case Diversity::GOLD:
  case Diversity::JUNK:
    result = (price / _stats.getAvgPrice()) * (duration / _stats.getAvgDuration());
    break;
  case Diversity::LUXURY:
    result = duration;
    break;
  case Diversity::UGLY:
    result = price;
    break;
  default:
    break;
  }
  return result;
}

void
DiversityItinerarySwapper::removeSolution(Diversity::BucketType bucket,
                                          size_t pairing,
                                          const ItinStatistic::Solution* solution,
                                          const ShoppingTrx::SchedulingOption* outbound,
                                          const ShoppingTrx::SchedulingOption* inbound)
{
  typedef SopCombinationUtil::NonStopType NonStopType;
  typedef ShoppingTrx::FlightMatrix::value_type FMSolution;

  FMSolution fmSolution = *solution;

  NonStopType nsType = SopCombinationUtil::detectNonStop(outbound, inbound);
  bool isNS = nsType & SopCombinationUtil::NON_STOP;
  bool isAdditionalNS = false;

  printRemoveSolution(fmSolution.first, bucket);
  _stats.removeSolution(bucket, pairing, *solution);

  if (isNS)
  {
    printNSRemoved(fmSolution.first);

    isAdditionalNS = _model->isAdditionalNonStopOptionNeeded();
    if (!isAdditionalNS && _model->isAdditionalNonStopEnabled())
      isAdditionalNS = swapAdditionalNS(fmSolution);
  }

  if (isAdditionalNS)
  {
    _stats.addNonStopSolution(fmSolution);
    printAdditionalNSAdded(fmSolution.first);
  }
  else
    _flightMatrix.erase(fmSolution.first);
}

void
DiversityItinerarySwapper::printSwapSolution(size_t fpKey,
                                             const std::vector<int>& sopVec,
                                             const CarrierCode& cxr,
                                             SopCombinationUtil::NonStopType nonStopType,
                                             Diversity::BucketType bucket,
                                             size_t todBucket) const
{
  if (LIKELY(!_diagSwapper))
    return;

  *_dc << "Swapper: [" << std::hex << fpKey << std::dec << "] New SOP combination: ";
  printSopCombination(sopVec);

  *_dc << " CXR: " << (cxr.empty() ? "**" : cxr)
       << " NS: " << SopCombinationUtil::getDiagNonStopType(nonStopType)
       << " BKT: " << bucketToString(bucket) << " TODB: " << todBucket << "\n";

  if (_fpKey == fpKey)
  {
    *_dc << "R|OUT|";
    if (sopVec.size() == 2)
      *_dc << " IN|";
    *_dc << "CXR|NS|PRN|TODB|TODDST|   SCORE|COMMENT\n";
  }
}

void
DiversityItinerarySwapper::printStartBucket(size_t fpKey,
                                            Diversity::BucketType bucket,
                                            int bktStatus) const
{
  if (LIKELY(!_diagSwapper || _fpKey != fpKey))
    return;

  *_dc << "Swapper: Inspecting " << bucketToString(bucket) << " bucket: is ";
  if (bktStatus > 0)
    *_dc << "overflown\n";
  else if (bktStatus < 0)
    *_dc << "underflown\n";
  else
    *_dc << "exactly as requested\n";
}

void
DiversityItinerarySwapper::printRemoveSolution(const std::vector<int>& sopVec,
                                               Diversity::BucketType bucket) const
{
  if (LIKELY(!_diagSwapper))
    return;

  *_dc << "Swapper: Removing SOP combination: ";
  printSopCombination(sopVec);
  *_dc << " from bucket: " << bucketToString(bucket);
  *_dc << "\n";
}

void
DiversityItinerarySwapper::printNoSolutionForRemoval() const
{
  if (!_diagSwapper)
    return;

  *_dc << "Swapper: No suitable solution for removal found\n";
}

void
DiversityItinerarySwapper::printSolutionEvaluation(size_t fpKey,
                                                   const std::vector<int>& sopVec,
                                                   size_t pairing,
                                                   size_t newTODBucket,
                                                   Diversity::BucketType bucket,
                                                   MoneyAmount price,
                                                   SwapperEvaluationResult result) const
{
  if (LIKELY(!_diagSwapper || _fpKey != fpKey))
    return;

  const ShoppingTrx::SchedulingOption* outbound = nullptr, *inbound = nullptr;
  SopCombinationUtil::getSops(_trx, sopVec, &outbound, &inbound);

  CarrierCode cxr = SopCombinationUtil::detectCarrier(outbound, inbound);
  if (cxr.empty())
    cxr = "**";

  const std::vector<std::pair<uint16_t, uint16_t> >& todRanges = _diversity.getTODRanges();
  size_t todBucket = outbound->itin()->getTODBucket(todRanges);

  float todDistance = calcTODDistance(todBucket, newTODBucket);
  double score = calcSolutionScore(bucket, price, outbound, inbound);

  *_dc << (result == SwapperEvaluationResult::SELECTED ? "* " : "  ") << std::setw(3)
       << ShoppingUtil::findSopId(static_cast<PricingTrx&>(*_dc->trx()), 0, sopVec[0]) << ' ';

  if (sopVec.size() == 2)
    *_dc << std::setw(3)
         << ShoppingUtil::findSopId(static_cast<PricingTrx&>(*_dc->trx()), 1, sopVec[1]) << ' ';

  *_dc << ' ' << cxr << ' ' << ' ' << SopCombinationUtil::getDiagNonStopType(outbound, inbound)
       << ' ' << std::setw(3) << pairing << ' ' << std::setw(4) << todBucket << ' ' << std::fixed
       << std::setprecision(4) << todDistance << std::resetiosflags(std::ios_base::floatfield)
       << ' ' << std::setw(8) << std::setprecision(6) << score << ' ';

  switch (result)
  {
  case SwapperEvaluationResult::SELECTED:
    *_dc << "SELECTED";
    break;
  case SwapperEvaluationResult::CARRIERS:
    *_dc << "REMOVAL AFFECTS CARRIER DIVERSITY";
    break;
  case SwapperEvaluationResult::NON_STOPS:
    *_dc << "REMOVAL AFFECTS NON-STOP DIVERSITY";
    break;
  case SwapperEvaluationResult::LAST_MIN_PRICED:
    *_dc << "LAST MINIMALLY PRICED ITINERARY IN BUCKET";
    break;
  case SwapperEvaluationResult::TOD_DISTANCE:
    *_dc << "HAS WORSE/SAME TOD DISTANCE";
    break;
  case SwapperEvaluationResult::SCORE:
    *_dc << "HAS WORSE/SAME SCORE";
    break;
  case SwapperEvaluationResult::CUSTOM_SOLUTION:
    *_dc << "CUSTOM SOLUTION";
    break;
  case SwapperEvaluationResult::ALL_SOPS:
    *_dc << "AFFECTS ALL SOPS DIVERSITY";
    break;
  case SwapperEvaluationResult::RC_ONLINES:
    *_dc << "AFFECTS REQ.CARRIER ONLINE DIVERSITY";
    break;
  case SwapperEvaluationResult::IBF_PREFERENCE:
    *_dc << "HAS WORSE IBF PREFERENCE SCORE";
    break;
  case SwapperEvaluationResult::NONE:
    break;
  }
  *_dc << "\n";
}

void
DiversityItinerarySwapper::printSopCombination(const std::vector<int>& sopVec) const
{
  PricingTrx& prTrx = static_cast<PricingTrx&>(*_dc->trx());
  *_dc << ShoppingUtil::findSopId(prTrx, 0, sopVec[0]);
  if (sopVec.size() == 2)
    *_dc << "x" << ShoppingUtil::findSopId(prTrx, 1, sopVec[1]);
}

const char*
DiversityItinerarySwapper::bucketToString(const Diversity::BucketType bucket) const
{
  switch (bucket)
  {
  case Diversity::GOLD:
    return "GOLD";
  case Diversity::JUNK:
    return "JUNK";
  case Diversity::LUXURY:
    return "LUXURY";
  case Diversity::UGLY:
    return "UGLY";
  case Diversity::BUCKET_COUNT:
    return "UNKNOWN"; // to remove compiler warning...
  }

  return "UNKNOWN";
}

bool
DiversityItinerarySwapper::getMaxScoredSolution(const ItinStatistic::ScoredCombinations& sc,
                                                size_t minScore,
                                                const ItinStatistic::Solution** worstSolution,
                                                size_t* worstScore)
{
  // scIt = <score, list of solutions>
  for (const ItinStatistic::ScoredCombinations::value_type& scIt : sc)
  {
    size_t curScore = scIt.first;

    if (curScore < minScore)
      break;

    if (scIt.second.size() > 0)
    {
      *worstSolution = &(scIt.second.front());
      *worstScore = curScore;
      return true;
    }
  }
  return false;
}

void
DiversityItinerarySwapper::removeAdditionalNS(Diversity::NSBucketType bucket,
                                              size_t pairing,
                                              const ItinStatistic::Solution* solution)
{
  const std::vector<int> sopCombination = solution->first;

  _stats.removeNonStopSolution(bucket, pairing, *solution);
  printAdditionalNSRemoved(sopCombination);
  _flightMatrix.erase(sopCombination);
}

void
DiversityItinerarySwapper::printNSRemoved(const std::vector<int>& sopVec)
{
  if (_dc941)
    _dc941->printNonStopAction(Diag941Collector::SWAPPER_REM_NS, sopVec, _stats);
}

void
DiversityItinerarySwapper::printAdditionalNSRemoved(const std::vector<int>& sopVec)
{
  if (_diagSwapper || _diagNonStops)
  {
    CarrierCode cxr = SopCombinationUtil::detectCarrier(_trx, sopVec);
    *_dc << "Swapper: Additional non stop combination removed: ";
    printSopCombination(sopVec);
    *_dc << " CXR: " << (cxr == Diversity::INTERLINE_CARRIER ? "**" : cxr) << std::endl;
  }
  else if (_dc941)
    _dc941->printNonStopAction(Diag941Collector::SWAPPER_REM_ANS, sopVec, _stats);
}

void
DiversityItinerarySwapper::printAdditionalNSAdded(const std::vector<int>& sopVec)
{
  if (_diagSwapper || _diagNonStops)
  {
    CarrierCode cxr = SopCombinationUtil::detectCarrier(_trx, sopVec);
    *_dc << "Swapper: Additional non stop combination added: ";
    printSopCombination(sopVec);
    *_dc << " CXR: " << (cxr == Diversity::INTERLINE_CARRIER ? "**" : cxr) << std::endl;
  }
  if (_dc941)
    _dc941->printNonStopAction(Diag941Collector::SWAPPER_ADD_ANS, sopVec, _stats);
}
}
