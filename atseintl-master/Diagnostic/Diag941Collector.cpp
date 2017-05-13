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

#include "Diagnostic/Diag941Collector.h"

#include "Common/Assert.h"
#include "Common/Money.h"
#include "Common/ShoppingAltDateUtil.h"
#include "Common/ShoppingUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Diversity.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/InterlineTicketCarrierData.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "Diagnostic/SopVecOutputDecorator.h"
#include "Pricing/GroupFarePath.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/PQ/SoloADGroupFarePath.h"
#include "Pricing/Shopping/PQ/SOPInfo.h"
#include "Taxes/LegacyTaxes/TaxItem.h"

#include <algorithm>
#include <cmath>
#include <iomanip>

namespace tse
{
struct CombResultInfo
{
  char abbrev[4];
  std::string descr;
};

static const CombResultInfo combResultInfos[] = {
    {"EXS", "SOP combination already generated"},
    {"FLB", "Flight bit delayed validation failed"},
    {"FSC", "FBR same carrier check failed"},
    {"TSC", "FBR use carrier in carrier flight table failed"},
    {"35C", "Category 35 validating carrier check failed"},
    {"RCF", "Rule controller validation failed"},
    {"VTI", "Interline ticketing agreement validation failed"},
    {"---", "Passed validation"},
    {"MCT", "Minimum connection time requirements between SOPs is violated"},
    {"INT", "Interline solution only failed"},
    {"ALT", "Alternate dates validation failed"},
    {"CST", "Custom solutions limit"},
    {"LNG", "Long connections limit"},
    {"SRL", "Schedule Repeat Limit"},
    {"DIV", "Skipped by diversity model"},
    {"SMP", "Specify Maximum Penalty validation failed"},
    {"***",
     "Wasn't considered (too many failed combinations per farepath or delayed bits failed)"}};

namespace
{
namespace CombMatrix
{

const int OB_SOP_ID_WIDTH = 3;
const int IB_SOP_ID_WIDTH = 5;
const int COMB_RESULT_WIDTH = IB_SOP_ID_WIDTH;
}
} // anon ns

void
Diag941Collector::initParam(Diagnostic& root)
{
  _dumpVITA = (root.diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "VITA");
  _diagFlights = (root.diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "FLIGHTS");
  _diagNonStops = (root.diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "NONSTOPS");
  _diagDirectFlightsOnly =
      (root.diagParamMapItem("DF") == "Y" || root.diagParamMapItem("DF") == "T");
  _altDatesDetails = (root.diagParamMapItem("DD") == "ADDETAILS");
}

void
Diag941Collector::printHeader()
{
  const ShoppingTrx* shTrx = dynamic_cast<ShoppingTrx*>(trx());

  if(!shTrx)
  {
    DiagCollector::printHeader();
    return;
  }

  _sopsPerLeg.resize(shTrx->legs().size());
  _sopResults.resize(shTrx->legs().size());
  DiagCollector::printHeader();
  *this << "\n====== 941 - SOL Itineraries generator =======\n";
  *this << "Abbreviations:\n";
  *this << "PU - Pricing Unit\n";
  *this << "FU - Fare Usage\n";
  *this << "\nImportant SOP Bitmap indicators (any other value indicates SOP failed):\n";
  *this << " -  - Passed\n";
  *this << " S  - Not validated yet\n";
  *this << "\nValidations results legend:\n";
  for (const auto& combResultInfo : combResultInfos)
    *this << combResultInfo.abbrev << " - " << combResultInfo.descr << "\n";
  *this << "\n";
  printAltDatesDetails();
  printNonStopsHeader();
  *this << "PU |FU |L|FARE    |ORI|DES|CARRIER LIST\n";
  *this << "IDX|IDX|E|CLASS   |GIN|TIN|OR FLIGHT BITMAP\n";
  *this << "   |   |G|        |   |   |OR SELECTED SOPS\n";
  *this << "---|---|-|--------|---|---|-----------------\n";
}

void
Diag941Collector::printFarePath(const FarePath* fp, size_t fpKey)
{
  _combResults.clear();
  for (auto& elem : _sopsPerLeg)
    elem.clear();
  for (auto& elem : _sopResults)
    elem.clear();
  _fpTotalCombNum = 0;
  _optsPriceAdjustedByRC.clear();
  _fpKey = fpKey;

  if (_diagDirectFlightsOnly)
    _fpCtx = &_fpCtxBuf;

  *_fpCtx << "***************************************************************\n";
  *_fpCtx << "Farepath with " << fp->pricingUnit().size() << " PU(s), total NUC "
          << fp->getTotalNUCAmount() << " [" << std::hex << fpKey << "]" << std::dec << "\n";
}

void
Diag941Collector::printCarrierLine(size_t puIdx, size_t fuIdx, const PaxTypeFare* ptf)
{
  printPUFUInfo(puIdx, fuIdx, ptf);
  const ShoppingTrx* shTrx = dynamic_cast<ShoppingTrx*>(trx());
  if (shTrx->isAltDates())
  {
    const VecMap<uint32_t, VecMap<uint64_t, PaxTypeFare::FlightBitmap> >& fbpc =
        ptf->durationFlightBitmapPerCarrier();
    VecMap<uint32_t, VecMap<uint64_t, PaxTypeFare::FlightBitmap> >::const_iterator fbpcIt =
        fbpc.begin();
    for (; fbpcIt != fbpc.end(); ++fbpcIt)
      *_fpCtx << " " << shTrx->diversity().getCarrierMap().find(fbpcIt->first)->second;
  }
  else
  {
    const VecMap<uint32_t, PaxTypeFare::FlightBitmap>& fbpc = ptf->flightBitmapPerCarrier();
    VecMap<uint32_t, PaxTypeFare::FlightBitmap>::const_iterator fbpcIt = fbpc.begin();
    for (; fbpcIt != fbpc.end(); ++fbpcIt)
      *_fpCtx << " " << shTrx->diversity().getCarrierMap().find(fbpcIt->first)->second;
  }
  *_fpCtx << " " << std::setw(7) << Money(ptf->fareAmount(), ptf->currency());
  *_fpCtx << "\n";
}

void
Diag941Collector::printCarrierTotals(const std::vector<uint32_t>& carriers)
{
  *_fpCtx << "Selected carrier(s) per leg:";
  const ShoppingTrx* shTrx = dynamic_cast<ShoppingTrx*>(trx());
  for (size_t legIdx = 0; legIdx < carriers.size(); ++legIdx)
  {
    *_fpCtx << "  " << legIdx << ":"
            << shTrx->diversity().getCarrierMap().find(carriers[legIdx])->second;
  }
  *_fpCtx << "\n";
}

void
Diag941Collector::printSOPLine(size_t puIdx, size_t fuIdx, const PaxTypeFare* ptf)
{
  printPUFUInfo(puIdx, fuIdx, ptf);
  *_fpCtx << " ";
  const PaxTypeFare::FlightBitmap& flbm = ptf->flightBitmap();
  for (const auto& elem : flbm)
  {
    if (elem._flightBit == 0)
      *_fpCtx << "-";
    else
      *_fpCtx << static_cast<char>(elem._flightBit);
  }

  *_fpCtx << "\n";
}

void
Diag941Collector::printDurationFltBitmap(uint32_t carrierKey, PaxTypeFare* ptf)
{
  if (!_altDatesDetails)
    return;

  *_fpCtx << "  *** Flight Bitmap per duration *** \n";

  typedef VecMap<uint64_t, PaxTypeFare::FlightBitmap> DurationFltBitmap;
  const DurationFltBitmap& durFltBitmap = ptf->durationFlightBitmapPerCarrier()[carrierKey];
  for (const DurationFltBitmap::value_type& durFltBm : durFltBitmap)
  {
    const PaxTypeFare::FlightBitmap& flbm = durFltBm.second;
    *_fpCtx << "    DURATION: " << std::left << std::setw(12)
            << ShoppingAltDateUtil::getNoOfDays(durFltBm.first) << " ";
    for (const auto& elem : flbm)
    {
      if (elem._flightBit == 0)
        *_fpCtx << "-";
      else
        *_fpCtx << static_cast<char>(elem._flightBit);
    }
    *_fpCtx << "\n";
  }
}

void
Diag941Collector::printNonStopAction(NonStopAction action,
                                     const SopIdVec& comb,
                                     const ItinStatistic& stats)
{
  if (!_diagNonStops)
    return;

  PricingTrx* prTrx = dynamic_cast<PricingTrx*>(trx());
  std::ostream& os = _fpNSActionsBuf;

  os.setf(std::ios::left, std::ios::adjustfield);
  os << "- " << std::setw(15) << getNSActionString(action) << ": ";
  os.setf(std::ios::right, std::ios::adjustfield);
  os << std::setw(4) << ShoppingUtil::findSopId(*prTrx, 0, comb[0]);
  os.setf(std::ios::left, std::ios::adjustfield);
  if (comb.size() > 1)
    os << "x" << std::setw(4) << ShoppingUtil::findSopId(*prTrx, 1, comb[1]) << ":";

  os << " O[" << stats.getAdditionalOnlineNonStopsCount() << "]";
  os << " I[" << stats.getAdditionalInterlineNonStopsCount() << "]";
  os << " T[" << stats.getAdditionalNonStopsCount() << "]\n";
}

void
Diag941Collector::flushNonStopActions()
{
  if (!_diagNonStops)
    return;

  *_fpCtx << "Non-stop actions:\n";
  *_fpCtx << _fpNSActionsBuf.str();
  _fpNSActionsBuf.str("");
}

void
Diag941Collector::printSOPTotalsPerLeg(size_t legIndex, const std::vector<SOPInfo>& sopInfoVec)
{
  *_fpCtx << "SOPs for leg " << legIndex << ":            ";

  for (const auto& elem : sopInfoVec)
  {
    if (elem.getStatus() == 0)
      *_fpCtx << "-";
    else
      *_fpCtx << static_cast<char>(elem.getStatus());
  }

  *_fpCtx << "\n";
  *_fpCtx << "Bit to original SOP ID mapping for leg " << legIndex << ":";

  const PricingTrx& prTrx = dynamic_cast<const PricingTrx&>(*trx());

  for (size_t flbIdx = 0; flbIdx < sopInfoVec.size(); ++flbIdx)
  {
    if (sopInfoVec[flbIdx].getStatus() != 'N')
    {
      *_fpCtx << " " << flbIdx << "=" << ShoppingUtil::findSopId(prTrx,
                                                                 static_cast<uint16_t>(legIndex),
                                                                 sopInfoVec[flbIdx]._sopIndex);
    }
  }

  *_fpCtx << "\n";
}

void
Diag941Collector::addCombinationResult(const SopIdVec& oSopVec, CombinationResult result, bool isLongConnectFlag)
{
  if (_diagDirectFlightsOnly && !isDirectFlightsSolution(oSopVec))
    return;

  if (isLongConnectFlag)
  {
    *_fpCtx << "Maximum number of long connect options reached. Following fare is set from PO - Diversity:\n";
    *_fpCtx << "SOPIdVector values:\n";
  }

  for (size_t i = 0; i < oSopVec.size(); ++i)
  {
    if (isLongConnectFlag)
    {
      *_fpCtx << " " << oSopVec[i];
    }
    _sopsPerLeg[i].insert(oSopVec[i]);
  }

  if (isLongConnectFlag)
  {
    *_fpCtx << "\n";
  }


  const auto insResult = _combResults.insert(std::make_pair(oSopVec, result));
  const bool isOk = insResult.second;

  // When the same combination is reported multiple times using
  // different code, the code, which is more important to know (and is less by value)
  // will be displayed in validation matrix
  if (!isOk)
  {
    CombinationResult prevReportedCode = insResult.first->second;

    insResult.first->second = std::min(prevReportedCode, result);
  }
}

void
Diag941Collector::addMaximumPenaltyFailedFarePath(const FarePath& fp)
{

}

void
Diag941Collector::addSopResult(std::size_t legId, int sopId, CombinationResult result, bool isLongConnectFlag)
{
  if (isLongConnectFlag)
  {
    *_fpCtx << "Maximum number of long connect options reached. Following fare details set from PO - Diversity:\n";
    *_fpCtx << "legId:" << legId << " sopId:"<< sopId << "\n";
  }
  _sopsPerLeg[legId].insert(sopId);
  if (_sopResults[legId].count(sopId) == 0)
    _sopResults[legId][sopId] = result;
  else
  {
    CombinationResult& prevResult = _sopResults[legId][sopId];
    prevResult = std::min(prevResult, result);
  }
}

void
Diag941Collector::printDelayedFlightBitValidationResult(FarePath* fp,
                                                        const std::vector<uint32_t>& carrierKey)
{
  *_fpCtx << "Flight bitmap(s) after delayed validation:\n";

  for (size_t puIdx = 0; puIdx < fp->pricingUnit().size(); ++puIdx)
  {
    PricingUnit* pu = fp->pricingUnit()[puIdx];
    for (size_t fuIdx = 0; fuIdx < pu->fareUsage().size(); ++fuIdx)
    {
      FareUsage* fu = pu->fareUsage()[fuIdx];
      PaxTypeFare* ptf = fu->paxTypeFare();
      printSOPLine(puIdx, fuIdx, ptf);
      if (_altDatesDetails)
      {
        PricingTrx* prTrx = dynamic_cast<PricingTrx*>(trx());
        if (prTrx && prTrx->isAltDates())
        {
          uint16_t legIdx = ptf->fareMarket()->legIndex();
          printDurationFltBitmap(carrierKey[legIdx], ptf);
        }
      }
    }
  }
}

void
Diag941Collector::printCombinationMatrix()
{
  if (_combResults.empty())
    return;

  const PricingTrx& prTrx = dynamic_cast<const PricingTrx&>(*trx());
  *_fpCtx << "Validation results";
  if (_diagDirectFlightsOnly)
  {
    *_fpCtx << " (" << _combResults.size() << " direct flights "
            << " / " << _fpTotalCombNum << " combinations"
            << ")";
  }
  *_fpCtx << ":\n";

  // Print orig. sop ids for inbound in a row
  if (_sopsPerLeg.size() == 2)
  {
    *_fpCtx << std::setw(CombMatrix::OB_SOP_ID_WIDTH + 1) << ""; // keep padding

    for (const auto index : _sopsPerLeg[1])
    {
      *_fpCtx << std::setw(CombMatrix::IB_SOP_ID_WIDTH) << ShoppingUtil::findSopId(prTrx, 1, index);
    }
    *_fpCtx << "\n";
  }

  // Print combination matrix
  SopIdVec oSopVec(_sopsPerLeg.size());
  for (std::set<int>::const_iterator outIt = _sopsPerLeg[0].begin(); outIt != _sopsPerLeg[0].end();
       ++outIt)
  {
    oSopVec[0] = *outIt;

    // Print orig. sop id for outbound
    *_fpCtx << std::setw(CombMatrix::OB_SOP_ID_WIDTH) << ShoppingUtil::findSopId(prTrx, 0, *outIt)
            << " ";

    if (_sopsPerLeg.size() == 2)
    {
      // Print validation results for outbound X all inbound sops in a row
      for (const auto index : _sopsPerLeg[1])
      {
        oSopVec[1] = index;
        printCombResult(oSopVec);
      }
    }
    else
    {
      printCombResult(oSopVec);
    }
    *_fpCtx << "\n";
  }
}

void
Diag941Collector::printPUFUInfo(size_t puIdx, size_t fuIdx, const PaxTypeFare* ptf)
{
  *_fpCtx << "PU" << puIdx + 1 << " FU" << fuIdx + 1 << " " << ptf->fareMarket()->legIndex() << " "
          << std::setw(8) << ptf->fareClass() << " " << ptf->fareMarket()->boardMultiCity() << " "
          << ptf->fareMarket()->offMultiCity();
}

void
Diag941Collector::printCombResult(const SopIdVec& oSopVec)
{
  CombinationResult combResult = NOT_CONSIDERED;

  for (std::size_t legId = 0; legId < oSopVec.size(); ++legId)
  {
    const auto sopResIt = _sopResults[legId].find(oSopVec[legId]);

    if (sopResIt != _sopResults[legId].end())
      combResult = std::min(combResult, sopResIt->second);
  }

  const auto combResIt = _combResults.find(oSopVec);

  if (combResIt != _combResults.end())
    combResult = std::min(combResult, combResIt->second);

  *_fpCtx << std::setw(CombMatrix::COMB_RESULT_WIDTH) << combResultInfos[combResult].abbrev;
}

bool
Diag941Collector::isCarrierAgreementsPrinted(const CarrierCode& validatingCarrier) const
{
  return (_printedCarrierAgreements.find(validatingCarrier) != _printedCarrierAgreements.end());
}

void
Diag941Collector::addOptPriceAdjustedByRC(const SopIdVec& oSopVec,
                                          const FarePath& newFP,
                                          const FarePath& origFP)
{
  MoneyAmount adjustedBy;
  if (std::fabs(adjustedBy = newFP.getTotalNUCAmount() - origFP.getTotalNUCAmount()) > EPSILON)
  {
    _optsPriceAdjustedByRC[adjustedBy].push_back(oSopVec);
  }
}

void
Diag941Collector::printOptsPriceAdjustedByRC()
{
  const ShoppingTrx& shTrx = dynamic_cast<const ShoppingTrx&>(*trx());

  for (const MoneyAmountToSopIdxVec::value_type& priceToSopVec : _optsPriceAdjustedByRC)
  {
    *_fpCtx << "RC adjusted FP amount by " << std::fixed << std::setprecision(2) << std::showpos
            << priceToSopVec.first << std::noshowpos << " NUC for the next options:\n";
    for (const SopIdVec& sops : priceToSopVec.second)
    {
      *_fpCtx << " " << SopVecOutputDecorator(shTrx, sops);
    }
    *_fpCtx << "\n";
  }
}

void
Diag941Collector::printAltDatesDetails()
{
  ShoppingTrx* shoppingTrx = dynamic_cast<ShoppingTrx*>(trx());
  if (!shoppingTrx)
    return;

  if (!shoppingTrx->isAltDates() || !_altDatesDetails ||
      shoppingTrx->durationAltDatePairs().empty())
    return;

  *_fpCtx << "ALT DATES MAIN DURATION :"
          << ShoppingAltDateUtil::getNoOfDays(shoppingTrx->mainDuration()) << std::endl;
  PricingTrx::DurationAltDatePairs::const_iterator iter =
      shoppingTrx->durationAltDatePairs().begin();

  for (; iter != shoppingTrx->durationAltDatePairs().end(); iter++)
  {
    *_fpCtx << "DURATION :" << ShoppingAltDateUtil::getNoOfDays(iter->first) << std::endl;

    PricingTrx::AltDatePairs::const_iterator itBegin = iter->second.begin();
    PricingTrx::AltDatePairs::const_iterator itEnd = iter->second.end();
    for (; itBegin != itEnd; itBegin++)
      *_fpCtx << " " << itBegin->first.first.dateToString(MMDDYY, "") << "  "
              << itBegin->first.second.dateToString(MMDDYY, "") << std::endl;
  }
  *_fpCtx << "\n";
}

void
Diag941Collector::printAltDatesTaxHeader(size_t numSolutionPerDatePair)
{
  *this << "\nNUMBER OF SOLUTIONS TO GENERATE PER DATEPAIR: " << numSolutionPerDatePair << "\n"
        << "\nFLIGHT MATRIX STATUS AFTER ALTDATE TAX PROCESSING:\n"
        << "\n";
  if (!_diagFlights)
  {
    *this << "                  TAXED     NO TAX                          \n"
          << "DATEPAIR          PRICE     PRICE     SOP_IDS     ISREMOVED \n"
          << "--------------    --------  --------  ----------  ----------\n";
  }
}

void
Diag941Collector::printADSolution(const Diag941Collector::Solution& solution,
                                  MoneyAmount totalPrice,
                                  AltDatesTaxes::TaxComponentVec taxCompVec,
                                  bool isKept)
{
  using shpq::SoloADGroupFarePath;
  const ShoppingTrx& shTrx = dynamic_cast<ShoppingTrx&>(*trx());

  if (_diagFlights)
  {
    *this << "\n\nOPTION " << std::setw(3) << _adOptIdxNum++ << "  SOPS "
          << SopVecOutputDecorator(shTrx, solution.first) << "\n";

    printSolutionFlights(solution, taxCompVec);

    *this << (isKept ? "SOLUTION KEPT\n" : "SOLUTION REMOVED\n");
  }
  else
  {
    const SoloADGroupFarePath& gfp = dynamic_cast<SoloADGroupFarePath&>(*solution.second);

    *this << std::setw(6) << gfp._datePair.first.dateToString(MMDDYY, "") << "  " << std::setw(6)
          << gfp._datePair.second.dateToString(MMDDYY, "") << "    " << std::fixed << std::setw(8)
          << std::setprecision(2) << totalPrice << "  " << std::setw(8) << std::setprecision(2)
          << gfp.getTotalNUCBaseFareAmount() << "  "
          << SopVecOutputDecorator(shTrx, solution.first).width(10)
          << (isKept ? "  No" : "  Yes"); // inverting Yes/No as far column name is ISREMOVED
  }

  *this << "\n";
}

void
Diag941Collector::printCarrierAgreements(InterlineTicketCarrier& interlineTicketCarrierData,
                                         const CarrierCode& validatingCarrier)
{
  if (isCarrierAgreementsPrinted(validatingCarrier))
    return;

  _printedCarrierAgreements.insert(validatingCarrier);

  const PricingTrx& prTrx = dynamic_cast<const PricingTrx&>(*trx());
  const InterlineTicketCarrier::CarrierInfoMap& interlineCarriers =
      interlineTicketCarrierData.getInterlineCarriers(prTrx, validatingCarrier);

  *this << "Interline carriers for " << validatingCarrier << " validating carrier:\n";
  for (InterlineTicketCarrier::CarrierInfoMap::const_iterator carrierIt = interlineCarriers.begin();
       carrierIt != interlineCarriers.end();
       ++carrierIt)
  {
    if (carrierIt != interlineCarriers.begin())
    {
      *this << " ";
    }

    *this << carrierIt->first;
  }

  *this << "\n";
}

void
Diag941Collector::printSolutionFlights(const Diag941Collector::Solution& solution,
                                       AltDatesTaxes::TaxComponentVec taxCompVec)
{
  const SopIdVec* fmv = &(solution.first);
  const GroupFarePath gpath = *(solution.second);

  for (const auto elem : gpath.groupFPPQItem())
  {
    const FarePath& fPath = *elem->farePath();

    // We only want to output the path once
    printFarePathData(fmv, fPath);
    break;
  }

  *this << "TOTAL ESTIMATED FARE " << std::setw(8) << gpath.getTotalNUCBaseFareAmount()
        << "   PLUS TAX  " << std::setw(8) << gpath.getTotalNUCAmount() << "\n";

  // Begin - print tax detail
  size_t printedItem = 0;
  for (const auto& elem : taxCompVec)
  {
    if (nullptr == elem.second)
      continue;

    const TaxItem* taxItem = elem.second;
    *this << std::setw(8) << taxItem->taxAmount() << std::setw(3) << taxItem->paymentCurrency()
          << " " << std::setw(2) << elem.first << "  ";
    printedItem++;
    if ((0 != printedItem) && (printedItem % 4 == 0))
      *this << "\n";
  }
  *this << "\n";
  // End - print tax detail

  printGroupFarePath(gpath, *fmv);
}

void
Diag941Collector::printFarePathData(const SopIdVec* fmv, const FarePath& path)
{
  if (fmv->empty())
  {
    *this << "NO FARE PATH TO DISPLAY"
          << "\n";
    return;
  }

  size_t puIndex = 1;
  SopIdVec::const_iterator fmvIter = fmv->begin();
  SopIdVec::const_iterator fmvEIter = fmv->end();
  ShoppingTrx& shTrx = dynamic_cast<ShoppingTrx&>(*this->trx());
  const std::vector<ShoppingTrx::Leg>& legs = shTrx.legs();

  for (uint32_t legIdx = 0; fmvIter != fmvEIter; ++fmvIter, ++puIndex, ++legIdx)
  {
    const ShoppingTrx::Leg& curLeg = legs[legIdx];
    const int& curMatVal = *fmvIter;
    const ShoppingTrx::SchedulingOption& curSop = curLeg.sop()[curMatVal];
    const Itin* curItin = curSop.itin();
    std::vector<TravelSeg*>::const_iterator tSegIter = curItin->travelSeg().begin();
    std::vector<TravelSeg*>::const_iterator tSegEIter = curItin->travelSeg().end();

    for (; tSegIter != tSegEIter; ++tSegIter)
    {
      if ((*tSegIter)->segmentType() == Arunk)
      {
        continue;
      }

      const AirSeg* aSegPtr = dynamic_cast<const AirSeg*>(*tSegIter);

      if (nullptr == aSegPtr)
      {
        continue;
      }

      const AirSeg& aSeg = *aSegPtr;
      *this << std::setw(2) << puIndex << " " << std::setw(3) << aSeg.carrier() << " "
            << std::setw(4) << aSeg.flightNumber() << " " << std::setw(2)
            << (*tSegIter)->getBookingCode() << " ";

      const DateTime& depDT = aSeg.departureDT();
      const DateTime& arrDT = aSeg.arrivalDT();

      std::string depDTStr = depDT.timeToString(HHMM_AMPM, "");
      std::string arrDTStr = arrDT.timeToString(HHMM_AMPM, "");
      // Cut off the M from the time
      depDTStr = depDTStr.substr(0, depDTStr.length() - 1);
      arrDTStr = arrDTStr.substr(0, arrDTStr.length() - 1);

      *this << std::setw(5) << depDT.dateToString(DDMMM, "") << " " << std::setw(2) << " "
            << std::setw(3) << aSeg.origin()->loc() << "  " << std::setw(3)
            << aSeg.destination()->loc() << " " << std::setw(5) << depDTStr << " " << std::setw(5)
            << arrDTStr << " " << std::setw(4) << aSeg.equipmentType() << " " << std::setw(2)
            << aSeg.hiddenStops().size() << " ";

      if (aSeg.eticket())
      {
        *this << std::setw(3) << "/E";
      }

      *this << "\n";
    }
  }
}

void
Diag941Collector::printGroupFarePath(const GroupFarePath& groupFarePath, const SopIdVec& sops)
{
  const bool multiplePaths = groupFarePath.groupFPPQItem().size() > 1;

  for (const auto elem : groupFarePath.groupFPPQItem())
  {
    const FarePath& fpath = *elem->farePath();

    if (multiplePaths)
    {
      *this << "\n    PAX TYPE '" << fpath.paxType()->paxType() << "'"
            << "       SUB-TOTAL /" << std::setw(8) << Money(fpath.getTotalNUCAmount(), "NUC");
    }

    printFarePath(fpath, sops);
  }
}

void
Diag941Collector::printFPFailedMessage(const char* msg)
{
  *_fpCtx << msg << "\n";
}

void
Diag941Collector::printFPTooManyFailedCombMessage(const char* action)
{
  *_fpCtx << "TOO MANY FAILED COMBINATIONS: " << action << "\n";
}

void
Diag941Collector::printFPSolutionsFound(std::size_t numSolutionsFound,
                                        std::size_t numSolutionsInFlightMatrix)
{
  *_fpCtx << "Solutions found with this FP:       " << numSolutionsFound << "\n"
          << "New flight matrix solutions number: " << numSolutionsInFlightMatrix << "\n";
}

void
Diag941Collector::logFPDiversityCpuMetrics(double wallTime, double userTime, double sysTime)
{
  FPCpuTimeMeter::CpuTimeMetric metric(wallTime, userTime, sysTime);
  _fpDiversityCpuTimeMeter.addStat(_fpKey, _fpTotalCombNum, metric);
}

void
Diag941Collector::printFPCustomSolutionsFound(std::size_t numCustomSolutionsFound)
{
  *_fpCtx << "Custom solutions found with this FP: " << numCustomSolutionsFound << "\n";
}

void
Diag941Collector::flushFarePath()
{
  if (!_diagDirectFlightsOnly)
    return;

  if (!_combResults.empty())
    *this << _fpCtxBuf.str();
  _fpCtxBuf.str("");
}

void
Diag941Collector::flushFPDiversityCpuMetrics()
{
  if (!_fpDiversityCpuTimeMeter.isBlank())
  {
    *this << "\n" << _fpDiversityCpuTimeMeter;
  }
}

bool
Diag941Collector::isDirectFlightsSolution(const SopIdVec& oSopVec) const
{
  ShoppingTrx& shTrx = dynamic_cast<ShoppingTrx&>(const_cast<Trx&>(*trx()));

  std::size_t segCount = ShoppingUtil::getTravelSegCount(shTrx, oSopVec);
  return (oSopVec.size() == segCount);
}

void
Diag941Collector::printFarePath(const FarePath& farePath, const SopIdVec& sops)
{
  *this << "\n";
  size_t index = 1;
  size_t puIndex = 1;

  ShoppingTrx& shTrx = dynamic_cast<ShoppingTrx&>(*trx());
  shpq::CxrKeyPerLeg cxrKeyPerLeg;
  ShoppingUtil::collectSopsCxrKeys(shTrx, sops, cxrKeyPerLeg);

  for (std::vector<PricingUnit*>::const_iterator puIt = farePath.pricingUnit().begin();
       puIt != farePath.pricingUnit().end();
       ++puIt, ++puIndex)
  {
    for (std::vector<FareUsage*>::const_iterator fuIt = (**puIt).fareUsage().begin();
         fuIt != (**puIt).fareUsage().end();
         ++fuIt, ++index)
    {
      *this << "  ";
      setf(std::ios::right, std::ios::adjustfield);
      *this << std::setw(3) << index << " " << std::setw(2) << puIndex << " ";

      const PaxTypeFare& fare = *(**fuIt).paxTypeFare();
      PaxTypeFare* pFare = (**fuIt).paxTypeFare();

      pFare->setComponentValidationForCarrier(cxrKeyPerLeg[pFare->fareMarket()->legIndex()],
                                              shTrx.isAltDates(),
                                              pFare->getDurationUsedInFVO());
      const char* fvo = " ";
      if (pFare->isFareCallbackFVO())
        fvo = "%";
      else if (pFare->isFltIndependentValidationFVO())
        fvo = "#";

      *this << fare.carrier() << " ";
      setf(std::ios::left, std::ios::adjustfield);
      const size_t FareBasisWidth = 15;
      std::string fareBasis = fare.createFareBasis(shTrx);

      if (fareBasis.size() > FareBasisWidth)
      {
        fareBasis.resize(FareBasisWidth);
      }

      *this << std::setw(FareBasisWidth) << fareBasis << " " << fare.fareMarket()->origin()->loc()
            << "-" << fare.fareMarket()->destination()->loc() << "/";
      setf(std::ios::right, std::ios::adjustfield);
      std::string gd;
      globalDirectionToStr(gd, fare.fare()->globalDirection());
      const std::string& nucAmount =
          ShoppingUtil::stripMoneyStr(Money(fare.totalFareAmount(), "USD"), "USD");
      *this << std::setw(8) << nucAmount << "/" << std::setw(8)
            << Money(fare.fareAmount(), fare.currency()) << "/" << gd << fvo << " "
            << "\n";
    }
  }
}

void
Diag941Collector::printNonStopsHeader()
{
  if (!_diagNonStops)
    return;

  *this << "Non-stop statistic legend:\n";
  *this << "\tO - number of additional online non-stops\n";
  *this << "\tI - number of additional interline non-stops\n";
  *this << "\tT - total number of additional non-stops\n\n";

  ShoppingTrx* shTrx = dynamic_cast<ShoppingTrx*>(trx());
  const Diversity& div = shTrx->diversity();

  *this << "Non-stop diversity:\n";
  *this << "\tNumber of additional non-stop options to generate: " << div.getNonStopOptionsCount()
        << "\n";
  *this << "\tMax number of non-stop options: " << div.getMaxNonStopCount() << "\n";
  *this << "\tMax number of online non-stop options: " << div.getMaxOnlineNonStopCount() << "\n";
  *this << "\tMax number of interline non-stop options: " << div.getMaxInterlineNonStopCount()
        << "\n\n";
}

const char*
Diag941Collector::getNSActionString(NonStopAction action)
{
  switch (action)
  {
  case ADD_NS:
    return "ADD_NS";
  case ADD_ANS:
    return "ADD_ANS";
  case SWAPPER_ADD_ANS:
    return "SWAPPER_ADD_ANS";
  case SWAPPER_REM_NS:
    return "SWAPPER_REM_NS";
  case SWAPPER_REM_ANS:
    return "SWAPPER_REM_ANS";
  default:
    return "UNKNOWN";
  }
}

void
Diag941Collector::printVITA(const SopIdVec& sopVec,
                            InterlineTicketCarrier& interlineTicketCarrierData,
                            const Itin& itin,
                            bool validationResult,
                            const std::string& validationMessage)
{
  if (!_dumpVITA)
    return;

  const PricingTrx& prTrx = dynamic_cast<const PricingTrx&>(*trx());
  const CarrierCode& validatingCarrier = itin.validatingCarrier();

  printCarrierAgreements(interlineTicketCarrierData, validatingCarrier);

  *_fpCtx << "Validating interline ticketing agreement for SOP combination: ";
  for (size_t legId = 0; legId < sopVec.size(); ++legId)
  {
    *_fpCtx << ShoppingUtil::findSopId(prTrx, legId, sopVec[legId]) << " ";
  }
  *_fpCtx << "\nValidating carrier: " << validatingCarrier << "\nFlights:\n";
  for (const TravelSeg* travelSeg : itin.travelSeg())
  {
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(travelSeg);
    if (airSeg != nullptr)
    {
      const bool valid =
          interlineTicketCarrierData.validateAgreementBetweenValidatingAndInterlineCarrier(
              prTrx, validatingCarrier, airSeg->marketingCarrierCode());

      *_fpCtx << "  " << std::setw(4) << airSeg->origAirport() << std::setw(4)
              << airSeg->destAirport() << std::setw(5) << airSeg->flightNumber() << std::setw(3)
              << airSeg->marketingCarrierCode() << " " << (valid ? "PASSED" : "FAILED") << "\n";
    }
    else
    {
      *_fpCtx << "  ARUNK\n";
    }
  }

  if (validationResult)
  {
    *_fpCtx << "Interline ticketing agreement: PASSED\n";
  }
  else
  {
    *_fpCtx << "Interline ticketing agreement: FAILED " << validationMessage << "\n";
  }
}

void
Diag941Collector::printVITAPriceInterlineNotActivated()
{
  if (_dumpVITA)
  {
    *_fpCtx << "IET PRICING IS NOT ACTIVE\n";
  }
}
} /* namespace tse */
