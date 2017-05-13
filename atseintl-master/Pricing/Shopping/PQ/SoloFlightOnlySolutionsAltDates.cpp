#include "Pricing/Shopping/PQ/SoloFlightOnlySolutionsAltDates.h"

#include "Common/MultiDimensionalPQ.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseDateTimeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Diagnostic/Diag910Collector.h"
#include "Diagnostic/DiagManager.h"
#include "Pricing/Shopping/PQ/SoloFlightOnlySolutionsSopCollector.h"
#include "Pricing/Shopping/PQ/SoloFlightOnlySolutionsValidators.h"

#include <algorithm>

#include <tr1/tuple>

namespace tse
{
namespace fos
{

namespace
{
const CarrierCode InterlineCxrCode = "INT";
const CarrierCode AnySolutionCxrCode = "___";
}

SoloFlightOnlySolutionsAltDates::SoloFlightOnlySolutionsAltDates(ShoppingTrx& trx)
  : _trx(trx), _dc(nullptr), _validatingCarrierUpdater(trx), _numOfSolutionsNeeded(0)
{
  if (_trx.diagnostic().diagnosticType() == Diagnostic910)
  {
    if (_trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "FLIGHTS")
      _dc = dynamic_cast<Diag910Collector*>(DCFactory::instance()->create(_trx));
    else if (_trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "DETAILS")
    {
      _dc = dynamic_cast<Diag910Collector*>(DCFactory::instance()->create(_trx));
    }
  }

  if (_trx.isAltDates())
  {
    _validator = SoloFlightOnlySolutionsValidators::getValidator(
        _trx, SoloFlightOnlySolutionsValidators::ALTDATES);
    _numOfSolutionsNeeded = _trx.getOptions()->getRequestedNumberOfSolutions();
    for (PricingTrx::AltDatePairs::value_type& adPair : _trx.altDatePairs())
    {
      if (adPair.second && adPair.second->numOfSolutionNeeded == _numOfSolutionsNeeded)
        _datePairSet.insert(adPair.first);
    }
  }
}

SoloFlightOnlySolutionsAltDates::~SoloFlightOnlySolutionsAltDates()
{
  if (_validator)
    _validator->printDiagnostics();
}

void
SoloFlightOnlySolutionsAltDates::process()
{
  if (!_datePairSet.empty())
  {
    if (_dc)
      *_dc << "\nFLIGHT ONLY SOLUTION PROCESSING (ALT DATES)\n\n";

    SoloFlightOnlySolutionsSopCollector collector(_trx);
    SolutionsByCarrierByDatePair onlineSolutions;
    SolutionsByCarrierByDatePair interlineSolutions;
    generateCandidates(collector, onlineSolutions, interlineSolutions);
    collectSolutions(onlineSolutions, interlineSolutions);
  }

  if (_dc)
    _dc->flushMsg();
}

bool
SoloFlightOnlySolutionsAltDates::isSolutionExists(SopsCombination& sops)
{
  return _trx.flightMatrix().count(sops) || _trx.estimateMatrix().count(sops) ||
         _totalMatrix.count(sops);
}

SopsCombination
SoloFlightOnlySolutionsAltDates::getActualSops(Sops& candidate)
{
  SopsCombination sops;
  sops.reserve(candidate.size());
  for (const SOPWrapper& sop : candidate)
    sops.push_back(sop._sopId);

  return sops;
}

bool
SoloFlightOnlySolutionsAltDates::addFOSCandidate(GroupedSolutions& solutions,
                                                 SopsCombination& sops,
                                                 bool online)
{
  const ShoppingTrx::FlightMatrix::value_type item(sops, nullptr);
  std::pair<ShoppingTrx::FlightMatrix::iterator, bool> insertResult = _totalMatrix.insert(item);
  if (insertResult.second)
  {
    solutions.addFOS(sops);
    if (online)
      solutions.getOnlineSolutions().push_back(sops);
    else
      solutions.getInterlineSolutions().push_back(sops);
  }
  return insertResult.second;
}

void
SoloFlightOnlySolutionsAltDates::addFOS(SolutionsContainer::const_iterator itBegin,
                                        SolutionsContainer::const_iterator itEnd)
{
  if (itBegin == itEnd)
    return;

  for (; itBegin != itEnd; ++itBegin)
  {
    const ShoppingTrx::FlightMatrix::value_type item(*itBegin, nullptr);
    _trx.flightMatrix().insert(item);
  }
}

void
SoloFlightOnlySolutionsAltDates::removeEmptyCarriers(SopsByLegByCxrByDate& cxrSopsCollection)
{
  CarrierSet removeCxr;

  for (const SopsByLegByCxrByDate::value_type& cxrIt : cxrSopsCollection)
  {
    if (cxrIt.second.size() != 2)
    {
      removeCxr.insert(cxrIt.first);
      continue;
    }
    if (cxrIt.second[0].empty() || cxrIt.second[1].empty())
      removeCxr.insert(cxrIt.first);
  }

  for (const CarrierSet::value_type& cxr : removeCxr)
    cxrSopsCollection.erase(cxr);
}

void
SoloFlightOnlySolutionsAltDates::generateSingleLegCandidate(
    SOPCollections& allSopsCollection, SolutionsByCarrierByDatePair& owSolutions)
{
  for (const DatePair& datePair : _datePairSet)
  {
    int solutionsNeeded = _trx.altDatePairs().find(datePair)->second->numOfSolutionNeeded;
    int solutionsByCxr = 0;
    int numOfCxr = allSopsCollection.getSopsByLegByCxrByDate().size();
    if (numOfCxr > 0)
      solutionsByCxr = std::max((solutionsNeeded / numOfCxr), 1);

    for (SopsByLegByCxrByDate::value_type& sopsByLegByCxr :
         allSopsCollection.getSopsByLegByCxrByDate())
    {
      if (solutionsNeeded == 0)
        break;
      if (solutionsByCxr > solutionsNeeded)
        solutionsByCxr = solutionsNeeded;

      SopsByLegByDate& sopsByLegByDate = sopsByLegByCxr.second;
      const CarrierCode govCxr = sopsByLegByCxr.first;
      if (sopsByLegByDate.empty() || sopsByLegByDate[0].empty())
        continue;

      SopsByLeg sopsByLeg(sopsByLegByDate.size());

      SopsByDate::iterator itLeg = sopsByLegByDate[0].find(datePair.first);
      if (itLeg == sopsByLegByDate[0].end())
        continue;

      sopsByLeg[0].assign(itLeg->second.begin(), itLeg->second.end());
      SolutionsByCarrier& cxrSolutions = owSolutions[datePair];
      SolutionsByCarrier::iterator it =
          cxrSolutions.insert(std::make_pair(govCxr, GroupedSolutions())).first;
      solutionsNeeded -=
          generateOnlineSolutions(govCxr, it->second, sopsByLeg, allSopsCollection, solutionsByCxr);
    }
  }
}

void
SoloFlightOnlySolutionsAltDates::generateCandidates(
    SoloFlightOnlySolutionsSopCollector& collector,
    SolutionsByCarrierByDatePair& onlineSolutions,
    SolutionsByCarrierByDatePair& interlineSolutions)
{
  SOPCollections allSopsCollection;
  collector.collectApplicableSopsAltDates(allSopsCollection);

  if (_trx.legs().size() == 1)
  {
    generateSingleLegCandidate(allSopsCollection, onlineSolutions);
    return;
  }

  // remove empty carriers
  removeEmptyCarriers(allSopsCollection.getSopsByLegByCxrByDate());

  for (const DatePair& datePair : _datePairSet)
  {
    int solutionsNeeded = _trx.altDatePairs().find(datePair)->second->numOfSolutionNeeded;
    int solutionsByCxr = 0;

    int numOfCxr = allSopsCollection.getSopsByLegByCxrByDate().size();
    if (numOfCxr > 0)
        solutionsByCxr = std::max((solutionsNeeded / numOfCxr), 1);

    for (SopsByLegByCxrByDate::value_type& sopsByLegByCxr :
         allSopsCollection.getSopsByLegByCxrByDate())
    {
      if (solutionsNeeded == 0)
        break;
      if (solutionsByCxr > solutionsNeeded)
        solutionsByCxr = solutionsNeeded;

      SopsByLegByDate& sopsByLegByDate = sopsByLegByCxr.second;

      if (sopsByLegByDate.size() != 2 || sopsByLegByDate[0].empty() || sopsByLegByDate[1].empty())
        continue;

      const CarrierCode& govCxr = sopsByLegByCxr.first;

      SopsByLeg sopsByLeg(sopsByLegByDate.size());

      SopsByDate::iterator itLeg0 = sopsByLegByDate[0].find(datePair.first);
      if (itLeg0 == sopsByLegByDate[0].end())
        continue;
      SopsByDate::iterator itLeg1 = sopsByLegByDate[1].find(datePair.second);
      if (itLeg1 == sopsByLegByDate[1].end())
        continue;

      sopsByLeg[0].assign(itLeg0->second.begin(), itLeg0->second.end());
      sopsByLeg[1].assign(itLeg1->second.begin(), itLeg1->second.end());
      SolutionsByCarrier& cxrSolutions = onlineSolutions[datePair];
      SolutionsByCarrier::iterator it =
          cxrSolutions.insert(std::make_pair(govCxr, GroupedSolutions())).first;
      solutionsNeeded -=
          generateOnlineSolutions(govCxr, it->second, sopsByLeg, allSopsCollection, solutionsByCxr);
    }

    SopsByLeg sopsByLegInt(2);
    std::tr1::tie(sopsByLegInt[0], sopsByLegInt[1]) = allSopsCollection.getSopsByDatePair(datePair);

    if ((solutionsNeeded > 0) && !sopsByLegInt[0].empty() && !sopsByLegInt[1].empty())
    {
      SolutionsByCarrier& cxrSolutions = interlineSolutions[datePair];
      SolutionsByCarrier::iterator it =
          cxrSolutions.insert(std::make_pair(InterlineCxrCode, GroupedSolutions())).first;
      solutionsNeeded -= generateInterlineSolutions(
          InterlineCxrCode, it->second, sopsByLegInt, allSopsCollection, solutionsNeeded);

      if (solutionsNeeded > 0)
      {
        SolutionsByCarrier::iterator it =
            cxrSolutions.insert(std::make_pair(AnySolutionCxrCode, GroupedSolutions())).first;
        generateInterlineSolutions(
            AnySolutionCxrCode, it->second, sopsByLegInt, allSopsCollection, solutionsNeeded);
      }
    }
  }
}

uint32_t
SoloFlightOnlySolutionsAltDates::generateOnlineSolutions(const CarrierCode& carrierCode,
                                                         GroupedSolutions& solutions,
                                                         const SopsByLeg& sopsByLeg,
                                                         SOPCollections& sopsCollection,
                                                         const uint32_t numOfSolutionsToGenerate)
{
  uint32_t found(0);
  SopsCombination sops(sopsByLeg.size());

  MultiDimensionalPQ<SOPWrapper, int> mpq(sopsByLeg);

  Sops candidate;
  candidate.reserve(sopsByLeg.size());
  while (found < numOfSolutionsToGenerate)
  {
    Sops candidate = mpq.next();
    if (candidate.empty())
      break;

    sops = getActualSops(candidate);

    if (isSolutionExists(sops))
      continue;

    if (!_validator->isValidSolution(sops))
      continue;

    if (addFOSCandidate(solutions, sops, true))
      ++found;
  }

  return found;
}

uint32_t
SoloFlightOnlySolutionsAltDates::generateInterlineSolutions(const CarrierCode& carrierCode,
                                                            GroupedSolutions& solutions,
                                                            const SopsByLeg& sopsByLeg,
                                                            SOPCollections& sopsCollection,
                                                            const uint32_t numOfSolutionsToGenerate)
{
  uint32_t found(0);
  uint32_t anySolutions(0);
  SopsCombination sops;

  MultiDimensionalPQ<SOPWrapper, int> mpq(sopsByLeg);

  while (found < numOfSolutionsToGenerate)
  {
    Sops candidate = mpq.next();
    if (candidate.empty())
      break;

    sops = getActualSops(candidate);

    if (isSolutionExists(sops))
      continue;

    if (_trx.isValidatingCxrGsaApplicable() && !_validatingCarrierUpdater.processSops(_trx, sops))
      continue;

    if (!_validator->isValidSolution(sops))
      continue;

    if (carrierCode == InterlineCxrCode &&
        _validator->validateSolutionShape(sopsCollection, sops) == ShapeSolution::VALID_SNOWMAN)
    {
      if (addFOSCandidate(solutions, sops, false))
        ++found;
    }
    else if (carrierCode == AnySolutionCxrCode && !_validator->checkDirectFlight(sops))
    {
      if (addFOSCandidate(solutions, sops, false))
        ++anySolutions;
    }
  }

  return found + anySolutions;
}

void
SoloFlightOnlySolutionsAltDates::collectSolutions(SolutionsByCarrierByDatePair& onlineSolutions,
                                                  SolutionsByCarrierByDatePair& interlineSolutions)
{
  SolutionType solutionType((_trx.legs().size() == 2) ? ONLINE : OWSOLUTION);
  for (const DatePair& datePair : _datePairSet)
  {
    SolutionsByCarrier& cxrSolutions = onlineSolutions[datePair];
    size_t solutionsNeeded = _trx.altDatePairs().find(datePair)->second->numOfSolutionNeeded;
    for (SolutionsByCarrier::value_type& solutionsByCxr : cxrSolutions)
    {
      const CarrierCode& govCxr = solutionsByCxr.first;
      GroupedSolutions& groupedSolutions = solutionsByCxr.second;

      size_t onlineSolutionsNo(groupedSolutions.getOnlineSolutions().size());
      size_t found = std::min(solutionsNeeded, onlineSolutionsNo);

      SolutionsContainer::iterator itBegin = groupedSolutions.getOnlineSolutions().begin();
      SolutionsContainer::iterator itEnd = groupedSolutions.getOnlineSolutions().end();
      if (found != onlineSolutionsNo)
      {
        itEnd = itBegin;
        std::advance(itEnd, found);
      }

      addFOS(itBegin, itEnd);
      solutionsNeeded -= found;
      printAddFOS(datePair, solutionsNeeded, solutionType, found, itBegin, itEnd, govCxr);

      if (solutionsNeeded == 0)
        break;
    }

    // interline solutions
    if (solutionsNeeded > 0)
    {
      SolutionsByCarrier& intSolutions = interlineSolutions[datePair];
      SolutionsByCarrier::iterator intSolIt = intSolutions.find(InterlineCxrCode);
      if (intSolIt != intSolutions.end())
      {
        GroupedSolutions& groupedIntSolutions = intSolIt->second;
        size_t interlineSolutionsNo(groupedIntSolutions.getSolutions().size());
        size_t found = std::min(solutionsNeeded, interlineSolutionsNo);

        SolutionsContainer::iterator itBegin = groupedIntSolutions.getSolutions().begin();
        SolutionsContainer::iterator itEnd = groupedIntSolutions.getSolutions().end();
        if (found != interlineSolutionsNo)
        {
          itEnd = itBegin;
          std::advance(itEnd, found);
        }

        addFOS(itBegin, itEnd);
        solutionsNeeded -= found;
        printAddFOS(datePair, solutionsNeeded, INTERLINE, found, itBegin, itEnd);
      }
    }

    // any solutions with non direct flights on both direction
    if (solutionsNeeded > 0)
    {
      SolutionsByCarrier& intSolutions = interlineSolutions[datePair];
      SolutionsByCarrier::iterator anySolIt = intSolutions.find(AnySolutionCxrCode);
      if (anySolIt != intSolutions.end())
      {
        GroupedSolutions& groupedAnySolutions = anySolIt->second;
        size_t anySolutionsNo(groupedAnySolutions.getSolutions().size());
        size_t found = std::min(solutionsNeeded, anySolutionsNo);

        SolutionsContainer::iterator itBegin = groupedAnySolutions.getSolutions().begin();
        SolutionsContainer::iterator itEnd = groupedAnySolutions.getSolutions().end();
        if (found != anySolutionsNo)
        {
          itEnd = itBegin;
          std::advance(itEnd, found);
        }

        addFOS(itBegin, itEnd);
        solutionsNeeded -= found;
        printAddFOS(datePair, solutionsNeeded, ANYSOLUTION, found, itBegin, itEnd);
      }
    }
  }
}

void
SoloFlightOnlySolutionsAltDates::printAddFOS(const DatePair& datePair,
                                             size_t solutionsNeeded,
                                             SolutionType type,
                                             size_t found,
                                             SolutionsContainer::const_iterator itBegin,
                                             SolutionsContainer::const_iterator itEnd,
                                             const CarrierCode& cxrCode)
{
  if (!_dc)
    return;

  std::string govCxr((cxrCode.empty() ? "" : " CXR: " + cxrCode));
  *_dc << "Date pair: " << datePair.first.dateToSimpleString() << " - "
       << datePair.second.dateToSimpleString() << ", still needs " << solutionsNeeded
       << " solutions, found " << found;
  switch (type)
  {
  case ONLINE:
    *_dc << " online solutions " + govCxr + "\n";
    break;
  case INTERLINE:
    *_dc << " interline solutions\n";
    break;
  case ANYSOLUTION:
    *_dc << " any solutions\n";
    break;
  case OWSOLUTION:
    *_dc << " OW solutions " + govCxr + "\n";
    break;
  default:
    *_dc << '\n';
    break;
  }

  for (; itBegin != itEnd; ++itBegin)
  {
    *_dc << "SOP combination: " << _dc->sopsToStr(_trx, *itBegin) << '\n';
    _dc->printFlights(_trx, *itBegin);
  }
}
}
} // end namespace tse::fos
