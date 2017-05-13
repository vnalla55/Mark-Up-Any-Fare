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
#include "Pricing/Shopping/PQ/SoloOrchestrator.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/ConfigList.h"
#include "Common/ErrorResponseException.h"
#include "Common/Foreach.h"
#include "Common/Logger.h"
#include "Common/TravelSegAnalysis.h"
#include "Common/TSELatencyData.h"
#include "DataModel/DirFMPathListCollector.h"
#include "DataModel/Diversity.h"
#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag910Collector.h"
#include "Diagnostic/Diag923Collector.h"
#include "Diagnostic/Diag924Collector.h"
#include "Diagnostic/DiagManager.h"
#include "Pricing/GroupFarePath.h"
#include "Pricing/PaxFPFBaseData.h"
#include "Pricing/PricingOrchestrator.h"
#include "Pricing/PricingUtil.h"
#include "Pricing/Shopping/Diversity/DiversityModel.h"
#include "Pricing/Shopping/Diversity/DiversityModelFactory.h"
#include "Pricing/Shopping/Diversity/DmcBucketRequirement.h"
#include "Pricing/Shopping/FOS/FosBoss.h"
#include "Pricing/Shopping/FOS/SolFosGenerator.h"
#include "Pricing/Shopping/FOS/SolFosTaskScope.h"
#include "Pricing/Shopping/IBF/V2IbfManager.h"
#include "Pricing/Shopping/PQ/AltDatesTaxes.h"
#include "Pricing/Shopping/PQ/SoloBrandedFaresFlightOnlySolutions.h"
#include "Pricing/Shopping/PQ/SoloFarePathFactory.h"
#include "Pricing/Shopping/PQ/SoloFlightOnlySolutionsAltDates.h"
#include "Pricing/Shopping/PQ/SoloFmPathOrchestrator.h"
#include "Pricing/Shopping/PQ/SoloItinGenerator.h"
#include "Pricing/Shopping/PQ/SoloPQ.h"
#include "Pricing/Shopping/PQ/SoloPQItem.h"
#include "Pricing/Shopping/PQ/SoloTrxData.h"
#include "Pricing/Shopping/PQ/SolutionPattern.h"
#include "Pricing/Shopping/PQ/SolutionPatternPQItem.h"
#include "Pricing/Shopping/Utils/ShoppingUtils.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <set>

#include <boost/range/algorithm.hpp>

namespace tse
{
namespace
{
ConfigurableValue<ConfigSet<std::string>>
excludedSolutionPatterns("SHOPPING_DIVERSITY", "EXCLUDED_SOLUTION_PATTERNS");
}
namespace shpq
{
Logger
SoloOrchestrator::_logger("atseintl.SoloOrchestrator");

typedef std::set<uint8_t> SPNumbers;

SPNumbers
getExcludedSP()
{
  Logger logger("atseintl.SoloOrchestrator");
  SPNumbers excludedSP;
  for (const auto pattern : excludedSolutionPatterns.getValue())
  {
    try
    {
      excludedSP.insert(boost::lexical_cast<uint8_t>(pattern));
    }
    catch (boost::bad_lexical_cast& exception)
    {
      LOG4CXX_DEBUG(logger, "Conversion exception in getExcludedSP() function");
    }
  }
  return excludedSP;
}

SoloOrchestrator::SoloOrchestrator(ShoppingTrx& trx, PricingOrchestrator& po) : _trx(trx), _po(po)
{
}

SoloOrchestrator::~SoloOrchestrator()
{
}

void
SoloOrchestrator::generateDiagnostic923(const SoloSurcharges& soloSurcharges) const
{
  if (_trx.diagnostic().diagnosticType() != Diagnostic923)
    return;

  DCFactory* dcFactory = DCFactory::instance();
  Diag923Collector* dc923 = dynamic_cast<Diag923Collector*>(dcFactory->create(_trx));
  dc923->activate();

  if (soloSurcharges.isEnabled() != 0)
    dc923->setSurchargesDetailsMap(&soloSurcharges.surchargesDetailsMap());

  (*dc923) << _trx;
  dc923->flushMsg();
}

void
SoloOrchestrator::generateDiagnostic924(const SoloFmPathPtr& soloFmPath) const
{
  if (_trx.diagnostic().diagnosticType() != Diagnostic924)
    return;

  DCFactory* dcFactory = DCFactory::instance();
  Diag924Collector* dc924 = dynamic_cast<Diag924Collector*>(dcFactory->create(_trx));
  dc924->shoppingTrx() = &_trx;
  dc924->activate();
  (*dc924) << *soloFmPath;
  dc924->flushMsg();
}

void
SoloOrchestrator::process()
{
  TSELatencyData metrics(_trx, "SOLO PROCESS");
  LOG4CXX_DEBUG(_logger, "SoloOrchestrator::process()");

  SoloSurcharges soloSurcharges(_trx);
  {
    TSELatencyData metrics(_trx, "SOLO SURCHARGES CALCULATE");
    soloSurcharges.process();
    generateDiagnostic923(soloSurcharges);
  }

  SoloFmPathPtr soloFmPath = SoloFmPath::create(_trx);

  {
    TSELatencyData metrics(_trx, "SOLO FM PATH");

    SoloFmPathOrchestrator soloPathOrchestrator(_trx);
    soloPathOrchestrator.process(soloFmPath);
  }
  generateDiagnostic924(soloFmPath);

  ItinStatistic stats(_trx);
  subscribeTo(stats);

  SoloTrxData soloTrxData(_trx, _po);
  soloTrxData.setUpDiags(true);

  DiagCollector* diag942 = soloTrxData.getShoppingDC(Diagnostic942);

  SoloPQ pq(_trx, stats, diag942);

  DiversityModelFactory modelFactory(_trx, stats, diag942);
  DiversityModel* model(modelFactory.createModel(soloTrxData));

  initializePQ(pq, model, soloFmPath, soloTrxData);

  // BEGIN Temporary code. If  NUMBER_OF_EXPANSIONS diagnostics argument is set to non-zero value
  // for diag 929,
  // then we run the following special PQ processing to collect data for diag929 only.
  // When NUMBER_OF_EXPANSIONS is not set, then diag 929 shows data collected by
  // SoloPQ::getNextFarepath().
  // TODO: possibly remove the code later, together with expandPQDiag929() and getNoOfExpansions()
  const size_t noOfExpansions = pq.diagCollector().getNoOfExpansions();
  if (noOfExpansions)
  {
    expandPQDiag929(pq, soloTrxData, noOfExpansions);
    return;
  }
  // END Temporary code.

  // Generating Direct Combinations as FOSes:

  // We don't generate All direct combinations in All Flights Represented Diversity Mode
  // Because first we need to make sure all sops are represented. DIrects come in this mode later
  if (_trx.getRequest()->isParityBrandsPath())
  {
    if (!utils::isAllFlightsRepresentedDiversity(_trx))
    {
      // We are only limited in the number of generates SOPs
      // by the overall number of requested solutions
      const PricingOptions* options = _trx.getOptions();
      TSE_ASSERT(nullptr != options);
      utils::SopCombinationList generatedDirectFoses;
      generateInitialFosInNgs(_trx, options->getRequestedNumberOfSolutions(), generatedDirectFoses);

      utils::addToFlightMatrix(_trx, stats, generatedDirectFoses);

      if (generatedDirectFoses.size() ==
          static_cast<unsigned int>(options->getRequestedNumberOfSolutions()))
      {
        if (diag942)
        {
          *diag942 << "IBF: Skipping bucket goal calculations. We have enough options already\n";
          *diag942 << "***************************************************************\n";
        }

        model->printSummary();
        return;
      }
    }
    stats.setGoalsForIBF(diag942);
  }

  SoloItinGenerator ig(soloTrxData, pq, model, stats, _po.getBitmapOpOrderer());

  SoloPQItemPtr pqItem;
  while ((pqItem = pq.getNextFarepathCapableItem(soloTrxData)).get())
    ig.generateSolutions(pqItem, soloTrxData);
  removeUnwantedSolutions(model, soloSurcharges);
  model->printSolutions();

  ig.generateEstimatedSolutions();
  ig.flushDiag();

  if (!isAltDateTaxProcessingRequired())
    soloSurcharges.restoreTotalNUCAmount();
  if (_trx.startShortCutPricingItin())
  {
    SortedFlightsMap fMap;
    sortSolutions(fMap);
    moveShortcutPricingThreshold(fMap);
    determineBaseFare(fMap);
    setSegmentStatuses(fMap, _trx);
    soloSurcharges.calculateTaxesForShortCutPricing(fMap);
  }

  generateFlightOnlySolutions(stats, pq.isOnlyThruFM());

  if (_trx.getRequest()->isParityBrandsPath())
  {
    removeSuperfluousOptionsIfNeeded(_trx.flightMatrix(), stats);
  }

  model->printSummary();

  soloTrxData.setUpDiags(false);

  if (pq.isOnlyThruFM())
    _trx.setSolNoLocalInd(true);

  if (_trx.flightMatrix().empty() && _trx.diagnostic().diagnosticType() == DiagnosticNone)
  {
    throw ErrorResponseException(ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS);
  }
}

void
SoloOrchestrator::sortSolutions(SortedFlightsMap& flightsMap) const
{
  for (const ShoppingTrx::FlightMatrix::value_type& i : _trx.flightMatrix())
  {
    if (i.second != nullptr)
      flightsMap.insert(std::make_pair(i.second->getTotalNUCAmount(),
                                       std::make_tuple(i.first, i.second, std::vector<int>())));
  }

  for (const ShoppingTrx::EstimateMatrix::value_type& i : _trx.estimateMatrix())
  {
    if (i.second.second != nullptr)
      flightsMap.insert(std::make_pair(i.second.second->getTotalNUCAmount(),
                                       std::make_tuple(i.first, i.second.second, i.second.first)));
  }
}

void
SoloOrchestrator::determineBaseFare(SortedFlightsMap& flightMap)
{
  for (SortedFlightsMap::value_type& iter : flightMap)
  {
    FarePath* const farePath = std::get<1>(iter.second)->groupFPPQItem().back()->farePath();
    if ((std::get<1>(iter.second))->isShortCutPricing())
      PricingUtil::determineBaseFare(farePath, _trx, farePath->itin());
  }
}

void
SoloOrchestrator::moveShortcutPricingThreshold(SortedFlightsMap& flightMap)
{
  SortedFlightsMap::iterator fmIter = flightMap.begin();
  SortedFlightsMap::iterator fmEnd = flightMap.end();

  std::set<SOPVec> families;

  uint32_t shortcutThreshold = _trx.startShortCutPricingItin();
  uint32_t counter = 0;

  if (_trx.diversity().isAdditionalNonStopsEnabled())
  {
    shortcutThreshold += uint32_t(_trx.diversity().getNonStopOptionsPercentage() *
                                  static_cast<float>(shortcutThreshold));
  }

  if (_trx.startShortCutPricingItin() == 0)
    return;

  if (std::distance(fmIter, fmEnd) <= shortcutThreshold)
    return;

  for (; fmIter != fmEnd && counter < shortcutThreshold; ++fmIter)
  {
    const SOPVec& solution = std::get<0>(fmIter->second);
    const SOPVec& baseSolution = std::get<2>(fmIter->second);
    ++counter;
    if (!baseSolution.empty())
    {
      families.insert(baseSolution);
    }
    else
    {
      families.insert(solution);
    }
  }

  for (; fmIter != fmEnd; ++fmIter)
  {
    GroupFarePath* gfp = std::get<1>(fmIter->second);
    const SOPVec& baseSolution = std::get<2>(fmIter->second);
    if (!baseSolution.empty() && std::binary_search(families.begin(), families.end(), baseSolution))
    {
      ++counter;
    }
    else
    {
      gfp->setShortCutPricing(true);
    }
  }

  _trx.startShortCutPricingItin(counter);
}

void
SoloOrchestrator::setSegmentStatuses(SortedFlightsMap& flightMap, ShoppingTrx& trx)
{
  shpq::CxrKeyPerLeg cxrKeyPerLeg;

  SortedFlightsMap::const_iterator flight = flightMap.begin();
  SortedFlightsMap::const_iterator end = flightMap.end();

  for (; flight != end; ++flight)
  {
    FarePath* farePath = std::get<1>(flight->second)->groupFPPQItem().back()->farePath();
    ShoppingUtil::collectSopsCxrKeys(_trx, std::get<0>(flight->second), cxrKeyPerLeg);

    for (PricingUnit* pricingUnit : farePath->pricingUnit())
    {
      for (FareUsage* fareUsage : pricingUnit->fareUsage())
      {
        LegId legIndex = fareUsage->paxTypeFare()->fareMarket()->legIndex();

        fareUsage->paxTypeFare()->setComponentValidationForCarrier(
            cxrKeyPerLeg[legIndex], false, 0);

        const ShoppingUtil::ExternalSopId extId =
            ShoppingUtil::createExternalSopId(legIndex, std::get<0>(flight->second)[legIndex]);

        const uint32_t bitmapNumber = ShoppingUtil::getFlightBitIndex(_trx, extId);

        PaxTypeFare* ptf = fareUsage->paxTypeFare();

        fareUsage->segmentStatus().clear();

        if (ptf->getFlightSegmentStatus(bitmapNumber))
        {
          fareUsage->segmentStatus().insert(fareUsage->segmentStatus().end(),
                                            ptf->getFlightSegmentStatus(bitmapNumber)->begin(),
                                            ptf->getFlightSegmentStatus(bitmapNumber)->end());
        }

        TSE_ASSERT(fareUsage->segmentStatus().size() == fareUsage->travelSeg().size());

        setBreakAvailbility(*fareUsage);
      }
    }
    addMissingSegmentStatuses(farePath);
  }
}

void
SoloOrchestrator::addMissingSegmentStatuses(FarePath* farePath)
{
  const Itin* itin = farePath->itin();

  if (!itin)
  {
    LOG4CXX_ERROR(_logger, "FarePath is not assosiated with itin");
    return;
  }

  std::vector<std::tuple<FareUsage*, TravelSeg*, uint32_t>> allTravelSegments;
  std::vector<PaxTypeFare::SegmentStatus*> results;

  for (PricingUnit* pricingUnit : farePath->pricingUnit())
  {
    for (FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      uint32_t index = 0;
      for (TravelSeg* tvl : fareUsage->travelSeg())
        allTravelSegments.emplace_back(fareUsage, tvl, index++);
    }
  }

  for (TravelSeg* tvlSeg : itin->travelSeg())
  {
    int index = -1;

    for (uint32_t i = 0; i < allTravelSegments.size(); ++i)
    {
      TravelSeg* segment = std::get<1>(allTravelSegments[i]);

      if (segment->boardMultiCity() == tvlSeg->boardMultiCity() &&
          segment->offMultiCity() == tvlSeg->offMultiCity() &&
          segment->departureDT().get64BitRepDateOnly() ==
              tvlSeg->departureDT().get64BitRepDateOnly())
      {
        index = i;
        break;
      }
    }

    if (index == -1)
    {
      continue;
    }

    FareUsage* fu = std::get<0>(allTravelSegments[index]);
    uint32_t tvlSegIndex = std::get<2>(allTravelSegments[index]);

    results.push_back(&fu->segmentStatus()[tvlSegIndex]);
  }

  bool allEmpty = true;

  if (results[results.size() - 1]->_bkgCodeReBook.empty())
  {
    REVERSE_FOREACH (PaxTypeFare::SegmentStatus* seg, results)
    {
      if (!seg->_bkgCodeReBook.empty())
      {
        results[results.size() - 1]->_bkgCodeReBook = seg->_bkgCodeReBook;
        results[results.size() - 1]->_reBookCabin = seg->_reBookCabin;
        allEmpty = false;
      }
    }
  }
  else
  {
    allEmpty = false;
  }

  if (allEmpty)
  {
    results[results.size() - 1]->_bkgCodeReBook = "Y";
    results[results.size() - 1]->_reBookCabin.setEconomyClass();
  }

  bool changed = true;

  while (changed)
  {
    changed = false;
    for (uint32_t i = 0; i < results.size(); ++i)
    {
      if (results[i]->_bkgCodeReBook.empty())
      {
        changed = true;

        if (i + 1 < results.size())
        {
          results[i]->_bkgCodeReBook = results[i + 1]->_bkgCodeReBook;
          results[i]->_reBookCabin = results[i + 1]->_reBookCabin;
        }
        else if (i - 1 >= 0)
        {
          results[i]->_bkgCodeReBook = results[i - 1]->_bkgCodeReBook;
          results[i]->_reBookCabin = results[i - 1]->_reBookCabin;
        }
      }
    }
  }
}

void
SoloOrchestrator::setBreakAvailbility(FareUsage& fu)
{
  size_t travelSegSize = fu.travelSeg().size();

  if (travelSegSize > 0)
  {
    std::vector<TravelSeg*>::iterator tvlIter = fu.travelSeg().begin();
    std::vector<TravelSeg*>::iterator tvlLast = fu.travelSeg().end();

    size_t segStatusIndex = std::distance(tvlIter, tvlLast - 1);

    fu.segmentStatus()[segStatusIndex]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_AVAIL_BREAK, true);

    if (travelSegSize == 1)
      return;

    size_t i = travelSegSize - 1;
    --tvlLast;
    AirSeg* airSeg = nullptr;

    for (; tvlLast != tvlIter; --tvlLast, --i)
    {
      airSeg = dynamic_cast<AirSeg*>(*tvlLast);
      if (airSeg != nullptr)
        break;
    }
    fu.segmentStatus()[i]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_AVAIL_BREAK, true);
  }
}

void
SoloOrchestrator::initializePQ(SoloPQ& pq,
                               DiversityModel* model,
                               const SoloFmPathPtr& soloFmPath,
                               SoloTrxData& soloTrxData) const
{
  LOG4CXX_DEBUG(_logger, "SoloOrchestrator::initializePQ()");
  using namespace shpq;

  pq.setDiversityModel(model);

  const bool isOneWayRequest(_trx.legs().size() == 1);
  DiagSoloPQFilter spFilter;
  spFilter.initialize(soloTrxData.getDiagCollector().rootDiag(), true);

  SPNumbers excludedSP = getExcludedSP();

  SoloPQItemManager& pqItemManager = soloTrxData.getPQItemManager();
  for (const SolutionPattern& solutionPattern : SolutionPatternStorage::instance())
  {
    if (excludedSP.count(solutionPattern.getSPNumber()))
      continue;
    if (isOneWayRequest == solutionPattern.hasInboundSol())
      continue;

    {
      const size_t expectedLegsCount(static_cast<int>(solutionPattern.hasOutboundSol()) +
                                     static_cast<int>(solutionPattern.hasInboundSol()));
      VALIDATE_OR_THROW(expectedLegsCount == soloFmPath->size(),
                        UNKNOWN_EXCEPTION,
                        "Internal processing error during PQ initialization");
    }

    const DirFMPathListPtr outboundFMPLPtr(
        solutionPattern.hasOutboundSol()
            ? (*soloFmPath)[0]->getDirFMPathList(solutionPattern.getOutboundSolution())
            : DirFMPathListPtr());

    const DirFMPathListPtr inboundFMPLPtr(
        solutionPattern.hasInboundSol()
            ? (*soloFmPath)[1]->getDirFMPathList(solutionPattern.getInboundSolution())
            : DirFMPathListPtr());

    if (!outboundFMPLPtr.get() || (solutionPattern.hasInboundSol() && !inboundFMPLPtr.get()))
    {
      LOG4CXX_DEBUG(_logger,
                    "Cannot create " << solutionPattern.getSPIdStr() << ": leg "
                                     << (outboundFMPLPtr.get() ? "IB" : "OB") << " not found");
      continue;
    }

    SolutionPatternPQItemPtr spPqItemPtr(
        pqItemManager.constructSPPQItem(solutionPattern, outboundFMPLPtr, inboundFMPLPtr));

    if (!spFilter.isItemFilteredOut(spPqItemPtr, 0u))
      pq.enqueue(spPqItemPtr);
  }
}

void
SoloOrchestrator::expandPQDiag929(SoloPQ& pq, SoloTrxData& trxData, const size_t noOfExpansions)
    const
{
  TSELatencyData metrics(_trx, "SOLOORCHESTRATOR EXPAND DIAG929");
  LOG4CXX_DEBUG(_logger, "SoloOrchestrator::expandPQDiag929()");

  size_t i(0);
  while (!pq.empty() && ++i <= noOfExpansions)
  {
    SoloPQItemPtr item = pq.dequeue();
    LOG4CXX_TRACE(_logger,
                  "SoloOrchestrator::expandPQDiag929(): (" << pq.size() << ") " << item.get()
                                                           << " - " << item->str());
    item->expand(trxData, pq);
  }
}

void
SoloOrchestrator::generateFlightOnlySolutions(ItinStatistic& stats,
                                              const bool pqConditionOverride /* = false*/)
{
  if (_trx.isAltDates())
  {
    fos::SoloFlightOnlySolutionsAltDates fosAltDates(_trx);
    fosAltDates.process();
  }
  else
  {
    if (_trx.getRequest()->isParityBrandsPath())
    {
      generateBrandedFaresFlightOnlySolutions(stats);
    }
    else
    {
      generateRegularFlightOnlySolutions(stats, pqConditionOverride);
    }
  }
}

void
SoloOrchestrator::generateBrandedFaresFlightOnlySolutions(ItinStatistic& stats)
{
  fos::SoloBrandedFaresFlightOnlySolutions ibfFosGenerator(_trx, stats);

  fos::MissingFlightsIterator missingSops(_trx, stats.getUnusedSopIdsPerLeg());
  ibfFosGenerator(missingSops);

  if (_trx.legs().size() == 1) // If we add missing sops here the remaining generators will have
    // nothing to do for one leg requests
    return;

  size_t missingOptionsToMeetQ0S =
      _trx.getOptions()->getRequestedNumberOfSolutions() - _trx.flightMatrix().size();

  if (_trx.getRequest()->isAllFlightsRepresented())
  {
    // In this mode we don't have directs generated at the beginning of the processing
    // so if there's a riin fir them we need to generate them now
    size_t missingDirectOptionsCount = stats.getMissingDirectOptionsCount();
    size_t directsThatCouldStillBeGenerated =
        _trx.diversity().getMaxNonStopCount() - stats.getDirectOptionsCount();

    if (directsThatCouldStillBeGenerated < 0)
      directsThatCouldStillBeGenerated = 0;

    if (missingOptionsToMeetQ0S > missingDirectOptionsCount)
      missingDirectOptionsCount =
          std::min(directsThatCouldStillBeGenerated, missingOptionsToMeetQ0S);

    if (missingDirectOptionsCount > 0)
    {
      fos::MissingDirectsIterator missingDirects(_trx, stats);
      ibfFosGenerator(missingDirects, missingDirectOptionsCount);
    }
    missingOptionsToMeetQ0S =
        _trx.getOptions()->getRequestedNumberOfSolutions() - _trx.flightMatrix().size();

    if (missingOptionsToMeetQ0S < 0)
      missingOptionsToMeetQ0S = 0;
  }

  size_t missingRCOnlineOptions = stats.getMissingRCOnlineOptionsCount();
  size_t RCOnlinesThatCouldStillBeGenerated =
      _trx.diversity().getMaxRCOnlineOptionsCount() - stats.getRCOnlineOptionsCount();

  if (missingOptionsToMeetQ0S > missingRCOnlineOptions)
    missingRCOnlineOptions = std::min(RCOnlinesThatCouldStillBeGenerated, missingOptionsToMeetQ0S);

  if (missingRCOnlineOptions > 0)
  {
    fos::MissingRCOnlinesIterator missingRco(_trx, stats);
    ibfFosGenerator(missingRco, missingRCOnlineOptions);
  }
  // If there are still some options missing to meet Q0S
  missingOptionsToMeetQ0S =
      _trx.getOptions()->getRequestedNumberOfSolutions() - _trx.flightMatrix().size();
  if (missingOptionsToMeetQ0S > 0)
  {
    fos::MissingOptionsIterator missingOptions(_trx, stats);
    ibfFosGenerator(missingOptions, missingOptionsToMeetQ0S);
  }
}

void
SoloOrchestrator::generateRegularFlightOnlySolutions(ItinStatistic& stats,
                                                     const bool pqConditionOverride)
{
  TSELatencyData metrics(_trx, "SOLO FOS BOSS");

  fos::FosBoss fosBoss(_trx, &stats);
  fos::SolFosTaskScope taskScope(_trx, stats, pqConditionOverride);
  fosBoss.process<fos::SolFosGenerator>(taskScope);
}

struct SortAccordingToIbfPriorityPred
{
  bool operator()(const std::pair<shpq::SopIdxVecArg, int>& comb1,
                  const std::pair<shpq::SopIdxVecArg, int>& comb2)
  {
    return comb1.second > comb2.second;
  }
};

void
SoloOrchestrator::sortCurrentOptionsAccordingtoIbf(ItinStatistic::CombinationWithStatusVec& combs)
{
  SortAccordingToIbfPriorityPred sortAccordingToIbfPriority;
  boost::stable_sort(combs, sortAccordingToIbfPriority);
}

void
SoloOrchestrator::updateBucketMatching(ItinStatistic& stats)
{
  ItinStatistic::CombinationWithStatusVec& bucketMatching = stats.getBucketMatching();
  for (ItinStatistic::CombinationWithStatus& combWithStatus : bucketMatching)
  {
    if (stats.getSopPairing(0, combWithStatus.first[0]) == 1)
      combWithStatus.second |= DmcRequirement::NEED_OUTBOUNDS;
    else
      combWithStatus.second &= ~DmcRequirement::NEED_OUTBOUNDS;

    if (combWithStatus.first.size() > 1)
    {
      if (stats.getSopPairing(1, combWithStatus.first[1]) == 1)
        combWithStatus.second |= DmcRequirement::NEED_INBOUNDS;
      else
        combWithStatus.second &= ~DmcRequirement::NEED_INBOUNDS;
    }
  }
  sortCurrentOptionsAccordingtoIbf(bucketMatching);
}

void
SoloOrchestrator::removeSuperfluousOptionsIfNeeded(ShoppingTrx::FlightMatrix& flightMatrix,
                                                   ItinStatistic& stats)
{
  int optionsOverRequestedCount =
      static_cast<int>(stats.getTotalOptionsCount()) -
      static_cast<int>(_trx.getOptions()->getRequestedNumberOfSolutions());

  if (optionsOverRequestedCount <= 0)
    return;

  Diag910Collector* dc(nullptr);
  if (_trx.diagnostic().diagnosticType() == Diagnostic910)
  {
    dc = dynamic_cast<Diag910Collector*>(DCFactory::instance()->create(_trx));
    *dc << " There are " << optionsOverRequestedCount
        << " options more than requested. Removing the least important ones : \n\n";
  }
  updateBucketMatching(stats);

  int optionsRemoved = 0;

  ItinStatistic::CombinationWithStatusVec& sortedOptions = stats.getBucketMatching();

  // In IBF we don't use NGS buckets and this attribute will be ignored anyway
  // but since removeSolution function requires this parameter we have to pass something
  const Diversity::BucketType DUMMY_BUCKET_TYPE = Diversity::BucketType(3);

  while (optionsRemoved < optionsOverRequestedCount && sortedOptions.size() > 0)
  {
    ItinStatistic::CombinationWithStatus worstCandidate = sortedOptions.back();
    sortedOptions.pop_back();

    // There may be more combinations in bucket matching than in the flight matrix
    // ( as some may have already been removed )
    // so removing them now can be unsuccessful
    if (flightMatrix.erase(worstCandidate.first) > 0)
    {
      GroupFarePath* nullfare = nullptr;
      stats.removeSolution(DUMMY_BUCKET_TYPE, 0, std::make_pair(worstCandidate.first, nullfare));
      updateBucketMatching(stats);
      optionsRemoved++;
      if (dc)
      {
        std::ostringstream ss;
        ss << optionsRemoved << " Removing solution with sopIds: ";
        for (auto& elem : worstCandidate.first)
          ss << elem << " ";
        ss << " with bucket satisfaction status " << DmcRequirement::print(worstCandidate.second)
           << "\n";
        *dc << ss.str();
      }
    }
  }

  if (dc)
  {
    *dc << "\n";
    dc->flushMsg();
  }
}

void
SoloOrchestrator::subscribeTo(ItinStatistic& stats)
{
  if (_trx.isAltDates() || !_trx.diversity().isDirectCarriersCapable())
    return;

  stats.setEnabledStatistics(ItinStatistic::STAT_NON_STOP_CARRIERS, this);
}

bool
SoloOrchestrator::isAltDateTaxProcessingRequired()
{
  if (!_trx.isAltDates())
  {
    return false;
  }

  std::string altDateTaxProcessingEnabled("N");
  if (!Global::config().getValue(
          "ALTDATE_TAX_PROCESSING", altDateTaxProcessingEnabled, "SHOPPING_DIVERSITY"))
  {
    CONFIG_MAN_LOG_KEY_ERROR(_logger, "ALTDATE_TAX_PROCESSING", "SHOPPING_DIVERSITY");
  }
  return ("Y" == altDateTaxProcessingEnabled);
}

void
SoloOrchestrator::removeUnwantedSolutions(DiversityModel* model, SoloSurcharges& soloSurcharges)
{
  {
    TSELatencyData metrics(_trx, "DIVERSITY MODEL");
    model->removeUnwantedSolutions(_trx.flightMatrix());
  }

  if (isAltDateTaxProcessingRequired())
  {
    AltDatesTaxes altDatesTaxes(_trx, soloSurcharges);
    altDatesTaxes.removeUnwantedSolutions(_trx.flightMatrix());
  }
}
}
}
