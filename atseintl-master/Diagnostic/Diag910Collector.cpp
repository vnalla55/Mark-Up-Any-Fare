//----------------------------------------------------------------------------
//  File:        Diag910Collector.C
//
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
#include "Diagnostic/Diag910Collector.h"

#include "BrandedFares/BrandInfo.h"
#include "Common/Assert.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/Money.h"
#include "Common/ShoppingAltDateUtil.h"
#include "Common/ShoppingUtil.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Pricing/FPPQItem.h"
#include "Pricing/GroupFarePath.h"
#include "Pricing/MaximumPenaltyValidator.h"
#include "Pricing/Shopping/FOS/FosStatistic.h"
#include "Pricing/Shopping/FOS/FosTaskScope.h"
#include "Pricing/Shopping/PQ/SoloGroupFarePath.h"
#include "Pricing/Shopping/PQ/SolutionPattern.h"

#include <boost/date_time/gregorian/gregorian.hpp>

#include <algorithm>
#include <iosfwd>
#include <iostream>
#include <numeric>
#include <sstream>

namespace tse
{
namespace
{
ConfigurableValue<int>
port("SERVER_SOCKET_ADP", "PORT");
}

Diag910Collector&
Diag910Collector::
operator<<(const PricingTrx& trx)
{
  const ShoppingTrx* const shoppingTrx = dynamic_cast<const ShoppingTrx*>(&trx);
  if (!shoppingTrx)
  {
    *this << "DIAG 910 REQUIRES ShoppingTrx\n";
    return *this;
  }

  printServerInfo();

  *this << "AGGREGATED RESULTS: \n";

  std::vector<TravelSeg*> segments = shoppingTrx->journeyItin()->travelSeg();
  for (std::vector<TravelSeg*>::const_iterator segment = segments.begin();
       segment != segments.end();
       ++segment)
  {
    if (segment != segments.begin())
      *this << "; ";
    *this << (**segment).origAirport() << " - " << (**segment).destAirport();
  }

  *this << "\n";
  *this << "TOTAL NUMBER OF SOLUTIONS: "
        << shoppingTrx->flightMatrix().size() + shoppingTrx->estimateMatrix().size() << std::endl;
  *this << "TOTAL NUMBER OF FAMILY HEADS: " << shoppingTrx->flightMatrix().size() << std::endl;
  *this << "TOTAL NUMBER OF ESTIMATED SOLUTIONS: " << shoppingTrx->estimateMatrix().size()
        << std::endl;
  if (shoppingTrx->getNumOfCustomSolutions())
  {
    uint32_t numOfCustomSolutions = 0;
    for (ShoppingTrx::FlightMatrix::const_iterator solution = shoppingTrx->flightMatrix().begin();
         solution != shoppingTrx->flightMatrix().end();
         ++solution)
    {
      if (ShoppingUtil::isCustomSolution(*shoppingTrx, solution->first))
        ++numOfCustomSolutions;
    }
    for (ShoppingTrx::EstimateMatrix::const_iterator solution =
             shoppingTrx->estimateMatrix().begin();
         solution != shoppingTrx->estimateMatrix().end();
         ++solution)
    {
      if (ShoppingUtil::isCustomSolution(*shoppingTrx, solution->first))
        ++numOfCustomSolutions;
    }
    *this << "TOTAL NUMBER OF CUSTOM SOLUTIONS: " << numOfCustomSolutions << std::endl;
  }
  *this << "\n\n";
  if (_rootDiag->diagParamMapItem("DD") == "FAMILIES")
  {
    printFamilies(shoppingTrx->flightMatrix(), shoppingTrx->estimateMatrix(), *shoppingTrx);
  }
  else
  {
    printHeader();
    printFarePaths(shoppingTrx->flightMatrix(), shoppingTrx->estimateMatrix(), *shoppingTrx);
  }
  *this << "\n";
  return *this;
}

void
Diag910Collector::printServerInfo()
{
  *this << "HOSTNAME/PORT:  ";
  std::string hostname = getenv("HOSTNAME");
  std::transform(hostname.begin(), hostname.end(), hostname.begin(), (int (*)(int))toupper);
  *this << hostname << "/" << port.getValue() << "\n";
}

void
Diag910Collector::printHeader(bool showOwFareKeyDetails)
{
  *this << "SEQ LEG PU CX/FARE BASIS           /TOTAL AMOUNT/ AMT CUR/GI\n";

  if (showOwFareKeyDetails)
    *this << "         VENDOR TARIFF RULE\n";
}

namespace
{
typedef std::pair<SopIdVec, GroupFarePath*> Solution;
bool
comparePaths(const Solution& a, const Solution& b)
{
  if (a.second == nullptr && b.second == nullptr)
  {
    const int sum1 = accumulate(a.first.begin(), a.first.end(), 0);
    const int sum2 = accumulate(b.first.begin(), b.first.end(), 0);
    return sum1 < sum2;
  }

  if (b.second == nullptr)
    return false;

  if (a.second == nullptr)
    return true;

  if (a.second->getTotalNUCAmount() == b.second->getTotalNUCAmount())
  {
    const int sum1 = accumulate(a.first.begin(), a.first.end(), 0);
    const int sum2 = accumulate(b.first.begin(), b.first.end(), 0);
    return sum1 < sum2;
  }

  return a.second->getTotalNUCAmount() < b.second->getTotalNUCAmount();
}

/**
 * @return empty string in case path is null or not a SoloGroupFarePath
 */
std::string
getSoloSolutionPattern(const GroupFarePath* path)
{
  const shpq::SoloGroupFarePath* soloGfp = dynamic_cast<const shpq::SoloGroupFarePath*>(path);
  if (!soloGfp)
    return "";

  const shpq::SolutionPattern* solutionPatterCPtr = soloGfp->getSolutionPattern();
  TSE_ASSERT(solutionPatterCPtr != nullptr);
  std::ostringstream ss;
  ss << solutionPatterCPtr->getSPIdStr() << " " << solutionPatterCPtr->getPUPathStr();
  return ss.str();
}
}

void
Diag910Collector::printFarePaths(const ShoppingTrx::FlightMatrix& flightMatrix,
                                 const ShoppingTrx::EstimateMatrix& estimateMatrix,
                                 const ShoppingTrx& trx,
                                 bool showOwFareKeyDetails)
{
  std::vector<GroupFarePath*> paths;
  ShoppingUtil::preparePathsFromFlightMatrix(flightMatrix, paths);
  std::vector<Solution> solutions(flightMatrix.begin(), flightMatrix.end());
  std::sort(solutions.begin(), solutions.end(), comparePaths);

  size_t index = 1;

  for (std::vector<Solution>::const_iterator solution = solutions.begin();
       solution != solutions.end();
       ++solution, ++index)
  {
    setf(std::ios::right, std::ios::adjustfield);

    // if an option is present to show the position in the flight matrix
    // of each fare path
    if (_rootDiag->diagParamMapItem("DD") == "MATRIX")
      *this << " " << sopsToStr(trx, solution->first) << " \n";

    if (_rootDiag->diagParamMapItem("DD") == "FLIGHTS")
    {
      const SopIdVec& sopsCombination = solution->first;

      *this << " BASE SOLUTION " << sopsToStr(trx, sopsCombination);
      *this << "\n";

      printFlights(trx, sopsCombination);

      if (trx.isAltDates())
      {
        DatePair datePair = ShoppingAltDateUtil::getDatePairSops(trx, sopsCombination);
        *this << "Departure Date: " << std::setw(6) << datePair.first.year() << "-"
              << datePair.first.month() << "-" << datePair.first.day() << std::endl;

        if (!datePair.second.isEmptyDate())
          *this << " Arrival Date: " << std::setw(6) << datePair.second.year() << "-"
                << datePair.second.month() << "-" << datePair.second.day() << std::endl;
      }
    }

    const GroupFarePath* path = solution->second;
    if (_rootDiag->diagParamMapItem("DD") != "MATRIX" &&
        _rootDiag->diagParamMapItem("DD") != "FLIGHTS")
    {
      if (trx.getNumOfCustomSolutions() && ShoppingUtil::isCustomSolution(trx, solution->first))
      {
        *this << "[CUS]\n";
      }
    }
    *this << std::setw(3) << index;
    *this << std::setw(6) << " ";
    *this << std::setw(28) << std::left << getSoloSolutionPattern(path);
    *this << " TOTAL /";

    if (path)
      printGroupFarePath(*path, trx, showOwFareKeyDetails);
    else
      *this << "0 NUC";

    *this << "\n";
  }

  if (paths.empty())
    *this << "\n(NO VALID FARE PATHS)\n";
  if (paths.empty() && trx.maxNumOfLngCnxSolutions() != 0)
    *this << "(IT'S NOT POSSIBLE TO USE LONG CONNECTION OPTION WITH ONLINE QUEUES)\n";

  for (const auto& elem : flightMatrix)
  {
    if (elem.second == nullptr)
    {
      *this << "FLIGHT-ONLY SOLUTION:  " << sopsToStr(trx, elem.first);
      *this << " \n";
    }
  }

  if (_rootDiag->diagParamMapItem("DD") == "FLIGHTS")
  {
    if (!estimateMatrix.empty())
      *this << " ESTIMATED SOLUTIONS \n";

    for (const auto& elem : estimateMatrix)
    {
      *this << " ";
      *this << " ESTIMATE SOP " << sopsToStr(trx, elem.first) << " BASE SOP "
            << sopsToStr(trx, elem.second.first);
      *this << "\n\n";
      printFlights(trx, elem.first);
      *this << "\n";
    }
  }

  *this << "\n";
}

void
Diag910Collector::printVITAData(const ShoppingTrx& trx,
                                const SopIdVec& sopsCombination,
                                const std::vector<TravelSeg*>& travelSeg,
                                const CarrierCode& validatingCarrier,
                                bool intTicketValidationResult,
                                bool validatingCarrierFromCache,
                                const std::string& validationMessage)
{
  *this << "  SOLUTION " << sopsToStr(trx, sopsCombination) << " \n";
  *this << "  FLIGHTS: ";

  for (const auto elem : travelSeg)
  {
    AirSeg* airSegPtr = elem->toAirSeg();

    if (airSegPtr != nullptr)
    {
      const AirSeg& aSeg = *airSegPtr;
      *this << aSeg.marketingCarrierCode() << " " << aSeg.flightNumber() << " ";
      if (!aSeg.operatingCarrierCode().empty() &&
          aSeg.operatingCarrierCode() != aSeg.marketingCarrierCode())
      {
        *this << "OPERATED BY " << aSeg.operatingCarrierCode() << " ";
      }
    }
    else
    {
      *this << "ARUNK ";
    }
  }

  *this << "\n  VALIDATING CARRIER: " << validatingCarrier;

  if (validatingCarrierFromCache)
    *this << " (FROM FIRST SOP VALIDATING CRX MAP)";

  *this << "\n  INTERLINETICKETINGAGREEMENT " << (intTicketValidationResult ? "PASSED" : "FAILED");
  if (!intTicketValidationResult && !validationMessage.empty())
  {
    *this << " " << validationMessage;
  }
  *this << "\n";
}

void
Diag910Collector::printVITAData(const ShoppingTrx& trx,
                                const SopIdVec& sopsCombination,
                                const CarrierCode& validatingCarrier,
                                bool intTicketValidationResult,
                                const std::string& validationMessage)
{
  *this << "  SOLUTION " << sopsToStr(trx, sopsCombination);

  *this << "\n  VALIDATING CARRIER: " << validatingCarrier << "\n";
  *this << "  INTERLINETICKETINGAGREEMENT " << (intTicketValidationResult ? "PASSED" : "FAILED");
  if (!intTicketValidationResult && !validationMessage.empty())
  {
    *this << " " << validationMessage;
  }
  *this << "\n";
  *this << "  READING THE VALIDATION RESULT FROM SHOPPING PQ SOP VITA MAP \n";
}

void
Diag910Collector::printGroupFarePath(const GroupFarePath& groupFarePath,
                                     const ShoppingTrx& trx,
                                     bool showOwFareKeyDetails)
{
  bool isHinted = false;
  if (trx.isSumOfLocalsProcessingEnabled())
  {
    const shpq::SoloGroupFarePath* soloGFP =
        static_cast<const shpq::SoloGroupFarePath*>(&groupFarePath);
    isHinted = soloGFP && soloGFP->getProcessThruOnlyHint();
  }

  *this << std::setw(8) << Money(groupFarePath.getTotalNUCAmount(), "NUC") << " ("
        << Money(groupFarePath.getTotalNUCBaseFareAmount(), "NUC") << ")" << (isHinted ? "H" : "");

  const bool multiplePaths = groupFarePath.groupFPPQItem().size() > 1;

  for (const auto elem : groupFarePath.groupFPPQItem())
  {
    const FarePath& fpath = *elem->farePath();

    if (multiplePaths)
    {
      *this << "\n    PAX TYPE '" << fpath.paxType()->paxType() << "'";
      *this << "       SUB-TOTAL /";
      *this << std::setw(8) << Money(fpath.getTotalNUCAmount(), "NUC");
    }

    printFarePath(fpath, trx, showOwFareKeyDetails);
  }
}

static char
determineFvoIndicatorSol(const ShoppingTrx& trx, const shpq::CxrKeys& cxrKeys, PaxTypeFare& ptf)
{
  char fvoInd = ' ';

  for (uint32_t cxrKey : cxrKeys)
  {
    ptf.setComponentValidationForCarrier(cxrKey, trx.isAltDates(), ptf.getDurationUsedInFVO());
    if (ptf.isFareCallbackFVO())
      return '%';
    if (ptf.isFltIndependentValidationFVO())
      fvoInd = '#';
  }

  return fvoInd;
}

static char
determineFvoIndicator(const PaxTypeFare& ptf)
{
  if (ptf.isFareCallbackFVO())
    return '%';
  if (ptf.isFltIndependentValidationFVO())
    return '#';
  return ' ';
}

void
Diag910Collector::printFarePath(const FarePath& farePath,
                                const ShoppingTrx& trx,
                                bool showOwFareKeyDetails)
{
  *this << "\n";
  shpq::CxrKeysPerLeg cxrKeysPerLeg;

  if (trx.isSumOfLocalsProcessingEnabled())
    ShoppingUtil::collectFPCxrKeys(farePath, trx.legs().size(), cxrKeysPerLeg);

  for (size_t puIndex = 0; puIndex < farePath.pricingUnit().size(); ++puIndex)
  {
    const PricingUnit& pu = *farePath.pricingUnit()[puIndex];

    for (size_t fuIndex = 0; fuIndex < pu.fareUsage().size(); ++fuIndex)
    {
      PaxTypeFare& fare = *pu.fareUsage()[fuIndex]->paxTypeFare();
      const FareMarket& fm = *fare.fareMarket();

      *this << "  ";
      setf(std::ios::right, std::ios::adjustfield);
      *this << std::setw(3) << fuIndex << " " << std::setw(2) << puIndex << " ";
      *this << fare.carrier() << " ";

      // TODO: change PaxTypeFare::createFareBasis to take a non-const PricingTrx, so we can remove
      // the below const_cast
      const size_t FareBasisWidth = 15;
      std::string fareBasis = fare.createFareBasis(const_cast<ShoppingTrx&>(trx));

      if (fareBasis.size() > FareBasisWidth)
        fareBasis.resize(FareBasisWidth);

      setf(std::ios::left, std::ios::adjustfield);
      *this << std::setw(FareBasisWidth) << fareBasis << " ";
      *this << fm.origin()->loc() << "-" << fm.destination()->loc() << "/";

      char fvo = ' ';
      if (trx.isSumOfLocalsProcessingEnabled())
        fvo = determineFvoIndicatorSol(trx, cxrKeysPerLeg[fm.legIndex()], fare);
      else
        fvo = determineFvoIndicator(fare);

      std::string gd;
      globalDirectionToStr(gd, fare.fare()->globalDirection());
      const std::string& nucAmount =
          ShoppingUtil::stripMoneyStr(Money(fare.totalFareAmount(), "USD"), "USD");
      setf(std::ios::right, std::ios::adjustfield);
      *this << std::setw(8) << nucAmount << "/" << std::setw(8)
            << Money(fare.fareAmount(), fare.currency()) << "/" << gd << fvo << " ";

      if (trx.awardRequest())
        *this << std::setw(8) << fare.mileage() << " MIL ";

      *this << "\n";

      if (showOwFareKeyDetails)
      {
        *this << std::setw(9) << " " << std::left << std::setw(7) << fare.vendor() << std::left
              << std::setw(7) << fare.fareTariff() << fare.ruleNumber() << "\n";
      }

      if (trx.getRequest()->isBrandedFaresRequest())
      {
        *this << "        BR: ";

        std::vector<BrandCode> brands;
        fare.getValidBrands(trx, brands, false);

        *this << (brands.empty() ? "-" : DiagnosticUtil::containerToString(brands)) << "\n";
      }
    }
  }

  if (_rootDiag->diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == Diagnostic::MAX_PEN &&
      farePath.paxType()->maxPenaltyInfo())
  {
    ShoppingTrx& mutableTrx = const_cast<ShoppingTrx&>(trx);
    FarePath* fp = farePath.clone(mutableTrx.dataHandle());
    MaximumPenaltyValidator(mutableTrx).completeResponse(*fp);
    *this << "    CHANGE PENALTY: " << fp->maxPenaltyResponse()->_changeFees << "\n"
          << "    REFUND PENALTY: " << fp->maxPenaltyResponse()->_refundFees << "\n";
  }
}

void
Diag910Collector::outputExcessiveOptions(const ShoppingTrx& trx,
                                         const SopIdVec& sopsCombination,
                                         GroupFarePath* gfp)
{
  setf(std::ios::right, std::ios::adjustfield);

  if (_rootDiag->diagParamMapItem("DD") == "MATRIX")
    *this << sopsToStr(trx, sopsCombination) << "\n";

  if (_rootDiag->diagParamMapItem("DD") == "FLIGHTS")
  {
    *this << " BASE SOLUTION " << sopsToStr(trx, sopsCombination);
    *this << "\n\n";
    printFlights(trx, sopsCombination);
    *this << "\n";
  }

  *this << "                          TOTAL /";

  if (gfp)
    printGroupFarePath(*gfp, trx);

  *this << "\n";

  if (!gfp)
    *this << "FLIGHT-ONLY SOLUTION:  " << sopsToStr(trx, sopsCombination) << " \n";

  *this << "\n";
}

void
Diag910Collector::printFlights(const ShoppingTrx& trx, const SopIdVec& sopsCombination)
{
  int leg = 0;
  for (SopIdVec::const_iterator sopIdIt = sopsCombination.begin(); sopIdIt != sopsCombination.end();
       ++sopIdIt, ++leg)
  {
    const Itin* itin = trx.legs()[leg].sop()[*sopIdIt].itin();
    TSE_ASSERT(itin);

    std::vector<TravelSeg*>::const_iterator travelSegIter = itin->travelSeg().begin();
    for (; travelSegIter != itin->travelSeg().end(); ++travelSegIter)
    {
      *this << std::setw(3) << (*travelSegIter)->origin()->loc();
      *this << "-" << std::setw(3) << (*travelSegIter)->destination()->loc();
      *this << "  ";
      AirSeg* airSegPtr = dynamic_cast<AirSeg*>(*travelSegIter);

      if (airSegPtr != nullptr)
      {
        const AirSeg& aSeg = *airSegPtr;
        *this << aSeg.carrier() << aSeg.flightNumber();
        const int FlightNumberWidth = 6;
        int numberWidth = boost::lexical_cast<std::string>(aSeg.flightNumber()).size();

        for (; numberWidth < FlightNumberWidth; ++numberWidth)
          *this << " ";

        const DateTime& depDT = aSeg.departureDT();
        const DateTime& arrDT = aSeg.arrivalDT();
        std::string depDTStr = depDT.timeToString(HHMM_AMPM, "");
        std::string arrDTStr = arrDT.timeToString(HHMM_AMPM, "");
        *this << std::setw(6) << depDTStr;
        *this << " ";
        *this << std::setw(6) << arrDTStr;
      }
      else
      {
        *this << "ARUNK";
      }

      *this << "\n";
    }
  }
}

std::string
Diag910Collector::sopsToStr(const ShoppingTrx& trx, const SopIdVec& sops, bool specialSolIndicators)
    const
{
  std::ostringstream os;
  os << "(";
  for (uint32_t leg = 0; leg < sops.size(); ++leg)
  {
    if (leg != 0)
      os << ",";
    os << ShoppingUtil::findSopId(trx, leg, static_cast<uint32_t>(sops[leg]));
  }
  os << ")";

  if (specialSolIndicators)
  {
    if (trx.getNumOfCustomSolutions() && ShoppingUtil::isCustomSolution(trx, sops))
      os << " [CUS]";
    if (trx.maxNumOfLngCnxSolutions() && ShoppingUtil::isLongConnection(trx, sops))
      os << " [LC]";
  }

  return os.str();
}

void
Diag910Collector::initFosDiagnostic()
{
  if (rootDiag()->diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "FLIGHTS")
    _diagFlights = true;
  else if (rootDiag()->diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "FOS")
    _diagFos = true;
  _diagTracedFos.clear();
  std::istringstream issSops(rootDiag()->diagParamMapItem(Diagnostic::SOP_IDS));
  std::copy(std::istream_iterator<int>(issSops),
            std::istream_iterator<int>(),
            std::back_inserter(_diagTracedFos));
}

void
Diag910Collector::printFosProcessing(const fos::FosTaskScope& task)
{
  if (_diagFlights || _diagFos)
    *this << "FLIGHT ONLY SOLUTIONS PROCESSING\n\n";
  if (_diagFos)
  {
    printFosTaskScope(task);
    printFosValidationLegend();
  }
}

void
Diag910Collector::printFosDeferredProcessing(const fos::FosStatistic& stats,
                                             const fos::FosGeneratorStats& genStats)
{
  std::ostringstream& os = *this;
  if (_diagFos)
  {
    printFosStatistic(stats);
    os << "FOS GENERATOR STATISTIC:\n";
    os << "\tTOTAL PROCESSED COMBINATIONS: " << genStats.totalProcessedCombinations << "\n";
    os << "\tUNIQUE PROCESSED COMBINATIONS: " << genStats.uniqueProcessedCombinations << "\n";
    os << "\tVALIDATED COMBINATIONS: " << genStats.validatedCombinations << "\n";
  }
  if (_diagFlights || _diagFos)
    os << "FLIGHT ONLY SOLUTIONS DEFERRED COMBINATIONS PROCESSING\n\n";
}

void
Diag910Collector::printFosProcessingFinished(const fos::FosStatistic& stats)
{
  if (_diagFos)
    printFosStatistic(stats);
  if (_diagFlights || _diagFos)
    *this << "FLIGHT ONLY SOLUTIONS PROCSESING FINISHED\n";
}

void
Diag910Collector::printValidatorAdded(fos::ValidatorType vt)
{
  if (_diagFos)
  {
    *this << "VALIDATOR ADDED: " << getFosValidatorStr(vt) << "\n";
  }
}

void
Diag910Collector::printFilterAdded(fos::FilterType ft)
{
  if (_diagFos)
  {
    *this << "FILTER ADDED: " << getFosFilterStr(ft) << "\n";
  }
}

void
Diag910Collector::printFos(const ShoppingTrx& trx,
                           const SopIdVec& combination,
                           const SopIdVec& base,
                           fos::ValidatorBitMask validBitMask)
{
  if (_diagFlights || _diagFos)
  {
    if (base.empty())
      *this << "BASE SOP " << sopsToStr(trx, combination);
    else
      *this << "ESTIMATE SOP " << sopsToStr(trx, combination) << " BASE SOP "
            << sopsToStr(trx, base);
  }
  if (_diagFlights)
  {
    *this << "\n";
    printFlights(trx, combination);
    *this << "\n";
  }
  else if (_diagFos)
  {
    *this << " VALIDATION STATUS (" << getFosValidatorBitMaskStr(validBitMask) << ")\n";
  }
}

void
Diag910Collector::printTracedFosStatus(const ShoppingTrx& trx,
                                       const SopIdVec& sops,
                                       fos::ValidatorBitMask validBM,
                                       fos::ValidatorBitMask deferredBM,
                                       fos::ValidatorBitMask invalidSopDetailsBM)
{
  if (!_diagFos)
    return;
  if (!isTracedFos(trx, sops))
    return;

  *this << "TRACED FOS STATUS " << sopsToStr(trx, sops) << "\n";
  *this << "\tVALID (" << getFosValidatorBitMaskStr(validBM) << ")\n";
  *this << "\tDEFERRED (" << getFosValidatorBitMaskStr(deferredBM) << ")\n";
  *this << "\tINVALID SOP DETAILS (" << getFosValidatorBitMaskStr(invalidSopDetailsBM) << ")\n";
}

void
Diag910Collector::printTracedFosThrownAway(const ShoppingTrx& trx, const SopIdVec& sops)
{
  if (!_diagFos)
    return;
  if (!isTracedFos(trx, sops))
    return;

  *this << "TRACED FOS THROWN AWAY " << sopsToStr(trx, sops) << "\n";
}

void
Diag910Collector::printTracedFosFailedPredicate(
    const ShoppingTrx& trx,
    const utils::SopCombination& sops,
    const utils::INamedPredicate<utils::SopCombination>& failedPredicate)
{
  if (!_diagFos)
    return;
  if (!isTracedFos(trx, sops))
    return;
  *this << "TRACED FOS " << sopsToStr(trx, sops) << " FAILED BY " << failedPredicate.getName()
        << "\n";
}

void
Diag910Collector::printFamilies(const ShoppingTrx::FlightMatrix& flightMatrix,
                                const ShoppingTrx::EstimateMatrix& estimateMatrix,
                                const ShoppingTrx& trx)
{
  typedef std::vector<SopIdVec> Children;
  typedef std::map<SopIdVec, Children> FamiliesCollection;

  FamiliesCollection families;
  Children orphans;

  for (const auto& elem : flightMatrix)
    families.insert(std::make_pair(elem.first, Children()));

  for (const auto& elem : estimateMatrix)
  {
    FamiliesCollection::iterator it = families.find(elem.second.first);
    if (it != families.end())
      it->second.push_back(elem.first);
    else
      orphans.push_back(elem.first);
  }

  for (FamiliesCollection::const_iterator it = families.begin(); it != families.end(); ++it)
  {
    const SopIdVec& sopsCombination = it->first;
    *this << " FAMILY HEAD (BASE SOLUTION) " << sopsToStr(trx, sopsCombination) << "\n\n";
    printFlights(trx, sopsCombination);
    *this << "\n";
    const GroupFarePath* gfp = flightMatrix.find(sopsCombination)->second;

    if (gfp)
    {
      printHeader(false);
      printGroupFarePath(*gfp, trx, false);
    }
    else
    {
      *this << " FLIGHT-ONLY SOLUTION\n";
    }
    *this << "\n";

    if (it->second.empty())
      continue;

    *this << " CHILDREN (ESTIMATED SOLUTION(S))\n";

    for (const auto& childSopsCombination : it->second)
    {
      *this << "\n CHILD " << sopsToStr(trx, childSopsCombination) << std::endl;
      printFlights(trx, childSopsCombination);
    }
    *this << "\n";
  }

  if (orphans.empty())
    return;

  *this << " \nORPHANS (ESTIMATED SOLUTION(S))\n";

  for (const SopIdVec& childSopsCombination : orphans)
  {
    *this << "\n CHILD " << sopsToStr(trx, childSopsCombination) << std::endl;
    printFlights(trx, childSopsCombination);
  }
}

void
Diag910Collector::printFosTaskScope(const fos::FosTaskScope& task)
{
  std::ostringstream& os = *this;

  os << "FOS TASK SCOPE:\n";
  os << "\tDIAMOND: " << task.getNumDiamondFos() << "\n";
  os << "\tSNOWMAN: " << task.getNumSnowmanFos() << "\n";
  os << "\tTRIANGLE: " << task.getNumTriangleFos() << "\n";
  os << "\tONLINE: " << task.getNumOnlineFos() << "\n";
  os << "\tCUSTOM: " << task.getNumCustomFos() << "\n";
  os << "\tLONG CONX: " << task.getNumLongConxFos() << "\n";
  os << "\tNON-STOP: " << task.getNumDirectFos() << "\n";

  os << "\tONLINE PER CXR: ";
  for (const auto& elem : task.getNumFosPerCarrier())
  {
    os << (elem.first.empty() ? "**" : elem.first);
    os << "[" << elem.second << "] ";
  }
  os << "\n\tNON-STOP PER CXR: ";
  for (const auto& elem : task.getNumDirectFosPerCarrier())
  {
    os << (elem.first.empty() ? "**" : elem.first);
    os << "[" << elem.second << "] ";
  }
  os << "\n";

  os << "\tCHECK CONNECTING FLIGHTS: " << (task.checkConnectingFlights() ? "T" : "F") << "\n";
  os << "\tCHECK CONNECTING CITIES: " << (task.checkConnectingCities() ? "T" : "F") << "\n";
  os << "\tPQ CONDITION OVERRIDE: " << (task.pqConditionOverride() ? "T" : "F") << "\n\n";
}

void
Diag910Collector::printFosValidationLegend()
{
  std::ostringstream& os = *this;

  os << "FOS VALIDATION LEGEND:\n";
  for (uint32_t i = 0; i <= fos::VALIDATOR_LAST; ++i)
  {
    fos::ValidatorType vt = static_cast<fos::ValidatorType>(i);
    os << getFosValidatorShortcut(vt) << ": " << getFosValidatorStr(vt) << "\n";
  }
  os << "\n";
}

void
Diag910Collector::printFosStatistic(const fos::FosStatistic& stats)
{
  std::ostringstream& os = *this;

  os << "FOS STATISTIC:\n";
  for (uint32_t i = 0; i <= fos::VALIDATOR_LAST; ++i)
  {
    fos::ValidatorType vt = static_cast<fos::ValidatorType>(i);
    os << "\t" << getFosValidatorStr(vt) << " ";
    os << stats.getCounter(vt) << "/" << stats.getCounterLimit(vt) << "\n";
  }

  os << "\tONLINE PER CXR: ";
  for (const auto& elem : stats.getCarrierCounterMap())
  {
    os << (elem.first.empty() ? "**" : elem.first);
    os << "[" << elem.second.value << "/" << elem.second.limit << "] ";
  }
  os << "\n\tNON-STOP PER CXR: ";
  for (const auto& elem : stats.getDirectCarrierCounterMap())
  {
    os << (elem.first.empty() ? "**" : elem.first);
    os << "[" << elem.second.value << "/" << elem.second.limit << "] ";
  }
  os << "\n\n";
}

std::string
Diag910Collector::getFosValidatorStr(fos::ValidatorType vt) const
{
  switch (vt)
  {
  case fos::VALIDATOR_DIAMOND:
    return "DIAMOND";
  case fos::VALIDATOR_SNOWMAN:
    return "SNOWMAN";
  case fos::VALIDATOR_TRIANGLE:
    return "TRIANGLE";
  case fos::VALIDATOR_ONLINE:
    return "ONLINE";
  case fos::VALIDATOR_CUSTOM:
    return "CUSTOM";
  case fos::VALIDATOR_LONGCONX:
    return "LONG CONX";
  case fos::VALIDATOR_NONSTOP:
    return "NON-STOP";
  default:
    return "UNKNOWN";
  }
}

std::string
Diag910Collector::getFosValidatorBitMaskStr(fos::ValidatorBitMask bitMask) const
{
  std::string result(fos::VALIDATOR_LAST + 1, '-');
  for (uint32_t i = 0; i <= fos::VALIDATOR_LAST; ++i)
  {
    fos::ValidatorType vt = static_cast<fos::ValidatorType>(i);
    if (validatorBitMask(vt) & bitMask)
      result[i] = getFosValidatorShortcut(vt);
  }
  return result;
}

std::string
Diag910Collector::getFosFilterStr(fos::FilterType ft) const
{
  switch (ft)
  {
  case fos::FILTER_RESTRICTION:
    return "RESTRICTION";
  case fos::FILTER_NONRESTRICTION:
    return "NONRESTRICTION";
  case fos::FILTER_OWTHREESEGS:
    return "OWTHREESEGS";
  case fos::FILTER_CUSTOM:
    return "CUSTOM";
  case fos::FILTER_NONSTOP:
    return "NON-STOP";
  default:
    return "Unknown";
  }
}

char
Diag910Collector::getFosValidatorShortcut(fos::ValidatorType vt) const
{
  switch (vt)
  {
  case fos::VALIDATOR_DIAMOND:
    return 'D';
  case fos::VALIDATOR_SNOWMAN:
    return 'S';
  case fos::VALIDATOR_TRIANGLE:
    return 'T';
  case fos::VALIDATOR_ONLINE:
    return 'O';
  case fos::VALIDATOR_CUSTOM:
    return 'C';
  case fos::VALIDATOR_LONGCONX:
    return 'L';
  case fos::VALIDATOR_NONSTOP:
    return 'N';
  default:
    return '#';
  }
}

bool
Diag910Collector::isTracedFos(const ShoppingTrx& trx, const SopIdVec& sops) const
{
  if (_diagTracedFos.empty() || _diagTracedFos.size() != sops.size())
    return false;
  for (std::size_t legIdx = 0; legIdx < sops.size(); ++legIdx)
  {
    uint32_t extSopId = ShoppingUtil::findSopId(trx, legIdx, static_cast<uint32_t>(sops[legIdx]));
    if (extSopId != static_cast<uint32_t>(_diagTracedFos[legIdx]))
      return false;
  }
  return true;
}

/**
   This function is written for logging FarePath with through fare precedence.
*/
void
Diag910Collector::displayFarePath(PricingTrx& trx, FarePath& fPath)
{
  if (!_active)
  {
    return;
  }

  ShoppingTrx& shoppingTrx = dynamic_cast<ShoppingTrx&>(trx);
  printFarePath(fPath, shoppingTrx, false);
}

void
Diag910Collector::printFosFailedPredicate(
    const ShoppingTrx& trx,
    const utils::SopCombination& comb,
    const utils::INamedPredicate<utils::SopCombination>& failedPredicate)
{
  if (!_diagFos)
    return;

  *this << "FOS " << sopsToStr(trx, comb) << " FAILED BY " << failedPredicate.getName()
        << std::endl;
}
}
