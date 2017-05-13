//----------------------------------------------------------------------------
//  File:        Diag911Collector.C
//  Created:     2004-10-29
//
//  Description: Diagnostic 911 formatter
//
//  Updates:
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

#include "Diagnostic/Diag911Collector.h"

#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "Common/Code.h"
#include "Common/DateTime.h"
#include "Common/Money.h"
#include "Common/ShoppingAltDateUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/VecMap.h"
#include "Common/Vendor.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FlightFinderTrx.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "Diagnostic/Diagnostic.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Diagnostic/BrandedDiagnosticUtil.h"
#include "Rules/RuleConst.h"

#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <string>

#include <stddef.h>

namespace tse
{
Diag911Collector& Diag911Collector::operator<<(const ShoppingTrx& shoppingTrx)
{
  if (!_active || shoppingTrx.legs().empty())
    return (*this);

  DiagCollector& dc(*this);

  _shoppingTrx = &shoppingTrx;
  _isBrandedFaresPath = _shoppingTrx->getRequest()->isBrandedFaresRequest();
  _brandProgramVec = _shoppingTrx->brandProgramVec();

  const auto largeFamilyDiscountLevel = _shoppingTrx->getOptions()->getSpanishLargeFamilyDiscountLevel();
  _isSpanishDiscountTrx = largeFamilyDiscountLevel != SLFUtil::DiscountLevel::NO_DISCOUNT;

  _flightFinderTrx = dynamic_cast<const FlightFinderTrx*>(&shoppingTrx);

  _isAllPaxTypeFareAsSource = _rootDiag->diagParamMapItem("SR") == "ALLFARES";
  printHeader(dc);

  if (_shoppingTrx->isAltDates() && _rootDiag->diagParamMapItem("DD") == "ADDETAILS")
    printAltDatesMainDetails();

  uint32_t count = 1;
  std::vector<ShoppingTrx::Leg>::const_iterator legIter = shoppingTrx.legs().begin();

  if (_flightFinderTrx != nullptr && (_flightFinderTrx->bffStep() == FlightFinderTrx::STEP_4 ||
                                _flightFinderTrx->bffStep() == FlightFinderTrx::STEP_6))
  {
    ++count;
    ++legIter;
  }

  for (; legIter != shoppingTrx.legs().end(); ++legIter, ++count)
  {
    _legIndex = count;
    dc << (*legIter).carrierIndex();
  }

  return (*this);
}

void
Diag911Collector::printHeader(DiagCollector& dc)
{
  dc << "911 : FARE AND BITMAP VALIDATION" << std::endl;
  dc << std::endl;
  dc << "RULE FAILED OR ROUTE NUMBER LEGEND" << std::endl;
  dc << "CAT-n : FARE FAILED DURING SHOPPING COMPONENT VALIDATION ON RULE" << std::endl;
  dc << "nnnn  : ROUTE NUMBER / FARE PASSED SHOPPING COMPONENT VALIDATION" << std::endl;
  dc << "FAIL  : FARE CONSIDERED FAILED DUE TO ALL BITS IN BITMAP INVALID" << std::endl;
  dc << "DIR   : FARE CONSIDERED FAILED DUE TO DIRECTIONALITY CHECK" << std::endl;
  dc << "#     : FARE VALIDATED BY FIRST PASS VALIDATION" << std::endl;
  dc << "%     : FARE VALIDATED BY CALLBACK VALIDATION" << std::endl;
  dc << "" << std::endl;
  dc << "BITMAP LEGEND" << std::endl;
  dc << "G : FAIL GLOBAL DIRECTION" << std::endl;
  dc << "2 : FAIL CAT 2 DATE TIME" << std::endl;
  dc << "4 : FAIL CAT 4 FLIGHT APPLICATION" << std::endl;
  dc << "9 : FAIL CAT 9 TRANSFER" << std::endl;
  dc << "T : FAIL CAT 14 TRAVEL RESTRICTION" << std::endl;
  dc << "8 : FAIL CAT 8 STOPOVER" << std::endl;
  dc << "Q : FAIL QUALIFY CAT 4" << std::endl;
  dc << "R : FAIL ROUTING" << std::endl;
  dc << "B : FAIL BOOKING CODE" << std::endl;
  dc << "X : FAIL EXCEEDS CONNECTION POINT LIMIT FOR ASO LEG" << std::endl;
  dc << "E : FAIL FARE EFFECTIVE DATE" << std::endl;
  dc << "F : FAIL FARE EXPIRED DATE" << std::endl;
  dc << "D : NO DATE FOUND" << std::endl;
  dc << "N : NOT APPLICABLE FOR THIS CARRIER" << std::endl;
  dc << "L : FAIL DUE TO TOO MANY FAILED BITS EARLIER" << std::endl;
  dc << "P : NOT APPLICABLE FOR ORIGIN AND/OR DESTINATION AIRPORT" << std::endl;
  if (isFFGsaApplicable())
    dc << "V : FAIL VALIDATING CXR FOR FLIGHT FINDER" << std::endl;
  dc << "- : PASS" << std::endl;
  dc << "***************************************************" << std::endl;
  if (_isBrandedFaresPath)
  {
    dc << "BR : LIST OF AVAILABLE BRANDS" << std::endl;
    dc << "     H - HARD PASS" << std::endl;
    dc << "     S - SOFT PASS" << std::endl;
    dc << "     F - FAIL" << std::endl;
    dc << "***************************************************" << std::endl;
  }
  dc << "" << std::endl;

  if (_isBrandedFaresPath)
  {
    BrandedDiagnosticUtil::displayAllBrandIndices(dc, _brandProgramVec);
    dc << "* * * * * * * * * * * * * * * * * * * * * * * * * * * *\n";
    dc << "*******************************************************\n\n";
  }
}

Diag911Collector&
Diag911Collector::operator<<(const ItinIndex& itinIndex)
{
  if (!_active || itinIndex.root().empty())
    return (*this);

  DiagCollector& dc(*this);
  const uint32_t totalCarrierCount(itinIndex.root().size());
  // Adjust dc output to left justified
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << std::endl;
  const std::string& filterCxr = dc.rootDiag()->diagParamMapItem("CX");
  const std::string& filterFM = dc.rootDiag()->diagParamMapItem("FM");
  std::string filterFMOrigin, filterFMDest;

  if (filterFM.size() == 6)
  {
    filterFMOrigin = filterFM.substr(0, 3);
    filterFMDest = filterFM.substr(3, 3);
  }

  ItinIndex::ItinMatrixConstIterator iGIter = itinIndex.root().begin();
  ItinIndex::ItinMatrixConstIterator iGEndIter = itinIndex.root().end();
  const ItinIndex::ItinCell* curCell(nullptr); // Cell instance
  uint32_t count = 1;

  for (; iGIter != iGEndIter; ++iGIter, ++count)
  {
    curCell = ShoppingUtil::retrieveDirectItin(
        itinIndex, iGIter->first, ItinIndex::CHECK_FAKEDIRECTFLIGHT);

    // If the leaf retrieval failed, go to the next itinerary
    if (!curCell)
      continue;

    _firstNonFakedCell = curCell;
    // Retrieve the itinerary
    const Itin* curItin = curCell->second;

    // Find the governing carrier from the first fare market of the carrier grouping
    const CarrierCode& curCarrier = curItin->fareMarket().front()->governingCarrier();
    const LocCode& origin = curItin->travelSeg().front()->boardMultiCity();
    const LocCode& dest = curItin->travelSeg().back()->offMultiCity();

    // see if parameters to the diagnostic specify to exclude this
    // fare market
    if ((filterCxr.empty() == false && filterCxr != curCarrier) ||
        (filterFMOrigin.empty() == false && filterFMOrigin != origin) ||
        (filterFMDest.empty() == false && filterFMDest != dest))
    {
      continue;
    }

    curCell = ShoppingUtil::retrieveDirectItin(itinIndex, iGIter->first, ItinIndex::CHECK_NOTHING);
    curItinRow = static_cast<const ItinIndex::ItinRow*>(&(iGIter->second));

    if (_shoppingTrx->isSumOfLocalsProcessingEnabled() ||
        _shoppingTrx->isIataFareSelectionApplicable())
    {
      if (!printSolFareMarkets(dc, curCell, curItin, count, totalCarrierCount, iGIter->first))
        continue;
    }
    else
    {
      if (!printFareMarkets(dc, curCell, curItin, count, totalCarrierCount, iGIter->first))
        continue;
    }
  }

  dc << "--------------------------------------------------------" << std::endl;
  return (*this);
}

bool
Diag911Collector::printSolFareMarkets(DiagCollector& dc,
                                      const ItinIndex::ItinCell* curCell,
                                      const Itin* curItin,
                                      const uint32_t currentCarrierIndex,
                                      const uint32_t totalCarrierCount,
                                      const uint32_t carrierKey)
{
  if (!curCell)
    return false;

  const CarrierCode& curCarrier = curItin->fareMarket().front()->governingCarrier();
  const Itin* topItin = curCell->second;
  const std::vector<FareMarket*>& fareMarkets(topItin->fareMarket());
  std::vector<FareMarket*>::const_iterator it(fareMarkets.begin());

  for (; it != fareMarkets.end(); ++it)
  {
    dc << "--------------------------------------------------------" << std::endl;
    dc << "LEG " << _legIndex << " GOVCXR " << currentCarrierIndex << " OF " << totalCarrierCount
       << " : " << curCarrier << std::endl;
    dc << "--------------------------------------------------------" << std::endl;
    dc << (*it)->boardMultiCity() << "-" << (*it)->offMultiCity()
       << ((*it)->getFmTypeSol() == FareMarket::SOL_FM_LOCAL ? ", LOCAL FM" : "");
    std::vector<PaxTypeFare*>& fares((*it)->allPaxTypeFare());
    std::vector<PaxTypeFare*>::iterator fareIt(fares.begin());

    // set current carrier
    for (; fareIt != fares.end(); ++fareIt)
      (*fareIt)->setComponentValidationForCarrier(
          carrierKey, _shoppingTrx->isAltDates(), _shoppingTrx->mainDuration());

    dc << **it;
  }

  return true;
}

bool
Diag911Collector::isFFGsaApplicable()
{
  return (_flightFinderTrx && _flightFinderTrx->isValidatingCxrGsaApplicable());
}

bool
Diag911Collector::printFareMarkets(DiagCollector& dc,
                                   const ItinIndex::ItinCell* curCell,
                                   const Itin* curItin,
                                   const uint32_t currentCarrierIndex,
                                   const uint32_t totalCarrierCount,
                                   const uint32_t carrierKey)
{
  if (!curCell)
    return false;

  const CarrierCode& curCarrier = curItin->fareMarket().front()->governingCarrier();
  const LocCode& origin = curItin->travelSeg().front()->boardMultiCity();
  const LocCode& dest = curItin->travelSeg().back()->offMultiCity();
  const Itin* topItin = curCell->second;
  const std::vector<FareMarket*>& fareMarkets(topItin->fareMarket());

  for (FareMarket* fm : fareMarkets)
  {
    dc << "--------------------------------------------------------" << std::endl;
    dc << "LEG " << _legIndex << " GOVCXR " << currentCarrierIndex << " OF " << totalCarrierCount
      << " : " << curCarrier << std::endl;
    dc << "--------------------------------------------------------" << std::endl;
    dc << origin << "-" << dest;

    if (_shoppingTrx->isIataFareSelectionApplicable())
    {
      const unsigned legIndex = fm->legIndex();
      const ShoppingTrx::Leg& leg = _shoppingTrx->legs().at(legIndex);

      if (!leg.stopOverLegFlag())
      {
        std::vector<PaxTypeFare*>& fares(fm->allPaxTypeFare());
        std::vector<PaxTypeFare*>::iterator fareIt(fares.begin());

        // set current carrier
        for (; fareIt != fares.end(); ++fareIt)
          (*fareIt)->setComponentValidationForCarrier(
              carrierKey, _shoppingTrx->isAltDates(), _shoppingTrx->mainDuration());
      }
    }

    dc << *fm;
    if (!_shoppingTrx->isIataFareSelectionApplicable())
      break;
  }

  return true;
}

Diag911Collector&
Diag911Collector::operator<<(const FareMarket& fareMarket)
{
  if (!_active || fareMarket.travelSeg().size() == 0)
    return *this;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << " " << fareMarket.getDirectionAsString() << (fareMarket.isDualGoverning() ? "DualGov" : "")
     << "\n";
  dc << "  Custom SOP FareMarket: "
     << (_shoppingTrx->isCustomSolutionFM(&fareMarket) ? "Yes" : "No") << std::endl;

  if (_isSpanishDiscountTrx)
  {
    dc << "  Spanish Family Discount: "
       << (_shoppingTrx->isSpanishDiscountFM(&fareMarket) ? "Yes" : "No") << std::endl;
  }

  bool hasNormal(false);
  bool hasSpecial(false);
  bool hasTag1(false);
  bool hasTag2(false);
  bool hasTag3(false);

  if (_isAllPaxTypeFareAsSource)
  {
    const std::vector<PaxTypeFare*>& paxTypeFares = fareMarket.allPaxTypeFare();

    if (paxTypeFares.empty())
    {
      dc << '\n' << "NO FARES FOUND FOR MARKET : " << fareMarket.origin()->description() << '-'
         << fareMarket.destination()->description() << "\n\n";
      return *this;
    }
    dc << "  Total Fares: " << std::setw(6) << fareMarket.allPaxTypeFare().size() << '\n';
    processPaxTypeFares(
        dc, fareMarket, paxTypeFares, hasNormal, hasSpecial, hasTag1, hasTag2, hasTag3);
  }
  else
  {
    const std::vector<PaxTypeBucket>& paxTypeCortegeVec = fareMarket.paxTypeCortege();

    if (paxTypeCortegeVec.empty())
    {
      dc << '\n' << "NO FARES FOUND FOR MARKET : " << fareMarket.origin()->description() << '-'
         << fareMarket.destination()->description() << "\n\n";
      return *this;
    }
    std::vector<PaxTypeBucket>::const_iterator ptcIt = paxTypeCortegeVec.begin();
    while (ptcIt != paxTypeCortegeVec.end())
    {
      const PaxTypeBucket& cortege = *(ptcIt++);
      if (cortege.paxTypeFare().empty())
      {
        dc << '\n' << "NO FARES FOUND FOR MARKET : " << fareMarket.origin()->loc() << '-'
           << fareMarket.destination()->loc()
           << ". REQUESTED PAXTYPE : " << cortege.requestedPaxType()->paxType() << '\n';
        continue;
      }

      dc << '\n';
      dc << "REQUESTED PAXTYPE : " << cortege.requestedPaxType()->paxType()
         << "  Total Fares: " << std::setw(6) << cortege.paxTypeFare().size() << '\n';
      dc << " INBOUND CURRENCY : " << cortege.inboundCurrency() << '\n';
      dc << "OUTBOUND CURRENCY : " << cortege.outboundCurrency() << '\n';

      processPaxTypeFares(
          dc, fareMarket, cortege.paxTypeFare(), hasNormal, hasSpecial, hasTag1, hasTag2, hasTag3);
    }
  }

  if (_shoppingTrx->isSumOfLocalsProcessingEnabled())
  {
    dc << "FARE MARKET SUMMARY:\n"
       << "  NORMAL FARES:  " << (hasNormal ? "T" : "F") << std::endl
       << "  SPECIAL FARES: " << (hasSpecial ? "T" : "F") << std::endl
       << "  TAG1         : " << (hasTag1 ? "T" : "F") << std::endl
       << "  TAG2         : " << (hasTag2 ? "T" : "F") << std::endl
       << "  TAG3         : " << (hasTag3 ? "T" : "F") << std::endl;
  }

  dc << '\n';
  return *this;
}

void
Diag911Collector::processPaxTypeFares(DiagCollector& dc,
                                      const FareMarket& fareMarket,
                                      const std::vector<PaxTypeFare*>& paxTypeFares,
                                      bool& hasNormal,
                                      bool& hasSpecial,
                                      bool& hasTag1,
                                      bool& hasTag2,
                                      bool& hasTag3)
{
  dc << "GOVERNING CARRIER : " << fareMarket.governingCarrier() << '\n';

  if (fareMarket.isThroughFarePrecedenceNGS())
    dc << "THROUGH FARE PRECEDENCE ENABLED : Yes" << '\n';

  if (_isBrandedFaresPath)
  {
    dc << "VALID BRANDS/PROGRAMS : ";
    BrandCodeSet validBrandCodes;
    for (const auto currentBrandId : fareMarket.brandProgramIndexVec())
    {
      dc << currentBrandId << " ";
      validBrandCodes.insert(
          _shoppingTrx->brandProgramVec().at(currentBrandId).second->brandCode());
    }
    dc << "\nVALID BRAND CODES : ";
    for (const auto& validBrandCode : validBrandCodes)
      dc << validBrandCode << " ";
    dc << "\n";
  }
  dc << '\n';

  if (_shoppingTrx->awardRequest())
  {
    dc << "                                                                 RULE  \n";
    dc << "                                                                 FAILED\n";
    dc << "                                                                 OR    \n";
    dc << "  GI V RULE   FARE BASIS    TRF O O      AMT CUR     MIL FAR PAX ROUTE \n";
    dc << "                            NUM R I                      TYP TYP NUMBER\n";
    dc << "- -- - ---- --------------- --- - - -------- --- ------- --- --- ------\n";
  }
  else if (isFFGsaApplicable())
  {
    dc << "                                                         RULE             \n";
    dc << "                                                         FAILED           \n";
    dc << "                                                         OR     VALIDATING\n";
    dc << "  GI V RULE   FARE BASIS    TRF O O      AMT CUR FAR PAX ROUTE  CARRIER   \n";
    dc << "                            NUM R I              TYP TYP NUMBER LIST      \n";
    dc << "- -- - ---- --------------- --- - - -------- --- --- --- ------ ----------\n";
  }
  else
  {
    dc << "                                                         RULE  \n";
    dc << "                                                         FAILED\n";
    dc << "                                                         OR    \n";
    dc << "  GI V RULE   FARE BASIS    TRF O O      AMT CUR FAR PAX ROUTE \n";
    dc << "                            NUM R I              TYP TYP NUMBER\n";
    dc << "- -- - ---- --------------- --- - - -------- --- --- --- ------\n";
  }

  std::vector<PaxTypeFare*>::const_iterator ptfIt = paxTypeFares.begin();

  while (ptfIt != paxTypeFares.end())
  {
    PaxTypeFare& paxFare = **(ptfIt++);
    dc << paxFare;
    {
      hasNormal = hasNormal || paxFare.isNormal();
      hasSpecial = hasSpecial || paxFare.isSpecial();
      hasTag1 = hasTag1 || (paxFare.owrt() == ONE_WAY_MAY_BE_DOUBLED);
      hasTag2 = hasTag2 || (paxFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED);
      hasTag3 = hasTag3 || (paxFare.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED);
    }
  }
}

Diag911Collector&
Diag911Collector::operator<<(const PaxTypeFare& paxFare)
{
  if (!_active)
    return *this;

  DiagCollector& dc(*this);

  _displayR1 = true; // display rule validation
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << std::setw(2) << cnvFlags(paxFare);
  std::string gd;
  globalDirectionToStr(gd, paxFare.fare()->globalDirection());
  dc << std::setw(3) << gd << std::setw(2) << Vendor::displayChar(paxFare.vendor()) << std::setw(5)
     << paxFare.ruleNumber();
  std::string fareBasis = paxFare.createFareBasis(*_trx, false);

  if (fareBasis.size() > 15)
    fareBasis = fareBasis.substr(0, 15) + "*"; // Cross-of-lorraine?

  dc << std::setw(16) << fareBasis << std::setw(4) << paxFare.fareTariff();

  dc << std::setw(2) << DiagnosticUtil::getOwrtChar(paxFare);

  if (paxFare.directionality() == FROM)
    dc << std::setw(2) << "O";
  else if (paxFare.directionality() == TO)
    dc << std::setw(2) << "I";
  else
    dc << " ";

  dc << std::setw(8) << Money(paxFare.fareAmount(), paxFare.currency()) << " ";

  if (_shoppingTrx->awardRequest())
  {
    dc.setf(std::ios::right);
    dc << std::setw(8) << paxFare.mileage() << " ";
  }

  if (!paxFare.isFareClassAppMissing())
    dc << std::setw(4) << paxFare.fcaFareType();
  else
    dc << "UNK ";

  if (!paxFare.isFareClassAppSegMissing())
  {
    if (paxFare.fcasPaxType().empty())
      dc << "***";
    else
      dc << std::setw(3) << paxFare.fcasPaxType();
  }
  else
  {
    dc << "UNK";
  }

  const char* fvo = " ";

  if (paxFare.isFareCallbackFVO())
  {
    fvo = "%";
  }
  else if (paxFare.isFltIndependentValidationFVO() &&
           (paxFare.getFlightBitmapSize() ||
            paxFare.shoppingComponentValidationFailed())) // second one is check in case of local FM
                                                          // shared across carriers
  {
    fvo = "#";
  }

  dc << std::setw(1) << fvo;

  if (!paxFare.isValidForBranding())
  {
    dc << "NO_BRAND\n";
  }
  else if (paxFare.shoppingComponentValidationFailed() || !paxFare.areAllCategoryValid())
  {
    if (paxFare.fare()->isDirectionalityFail())
      dc << "DIR  ";
    else if (!paxFare.isCat15SecurityValid())
      dc << "CAT-15";
    else
    {
      const size_t numCategories = 50;
      size_t cat = 1;

      for (cat = 1; cat != numCategories; ++cat)
        if (paxFare.isCategoryValid(cat) == false)
          break;

      if (cat < numCategories)
        dc << "CAT-" << cat;
      else
        dc << "UNK";
    }

    if ((_rootDiag->diagParamMapItem("DD") == "DETAILS") && (paxFare.isLongConnectFare()))
      dc << "\n THE ABOVE FARE CAN ONLY PRODUCE LONG CONNECT SOPS";

    // JIRA SCI-141: Adding code to print bitmaps for better tracking of the invalid bits
    printFlightBitmaps(dc, paxFare);

    if (Vendor::displayChar(paxFare.vendor()) == '*')
      dc << "    " << paxFare.vendor() << "\n";
  }
  else
  {
    const bool solProcess = _shoppingTrx->isSumOfLocalsProcessingEnabled();

    // fare passed component CAT validations
    if (solProcess && paxFare.isFltIndependentValidationFVO() && paxFare.getFlightBitmapSize() &&
        !hasAnyValidFlightBitmap(paxFare))
      dc << "FAIL   ";
    else if (!solProcess && paxFare.flightBitmapAllInvalid())
      dc << "FAIL   ";
    else
      dc << std::setw(7) << paxFare.routingNumber();

    if (isFFGsaApplicable())
    {
      for (const auto carrier : paxFare.validatingCarriers())
      {
        dc << carrier << " ";
      }
    }

    if ((_rootDiag->diagParamMapItem("DD") == "DETAILS") && (paxFare.isLongConnectFare()))
      dc << "\n THE ABOVE FARE CAN ONLY PRODUCE LONG CONNECT SOPS";

    printFlightBitmaps(dc, paxFare);

    if (Vendor::displayChar(paxFare.vendor()) == '*')
      dc << "    " << paxFare.vendor() << '\n';
  }

  if (_isBrandedFaresPath && paxFare.getBrandStatusVec().size() > 0)
  {
    dc << "BR  ";
    for (const auto& carrier : _brandProgramVec)
      dc << (char)paxFare.getBrandStatus(*_shoppingTrx, &carrier.second->brandCode());

    dc << std::endl;
  }

  return *this;
}

void
Diag911Collector::printFlightBitmaps(DiagCollector& dc, const PaxTypeFare& paxFare)
{
  if (paxFare.isKeepForRoutingValidation())
    dc << " SR";

  if (_shoppingTrx->getTrxType() == PricingTrx::FF_TRX)
    printFFBitmaps(paxFare);
  else if (_shoppingTrx->isAltDates() && (_rootDiag->diagParamMapItem("DD") == "ADDETAILS"))
    printAltDatesBitmaps(paxFare);
  else
    printBitmaps(paxFare);

  dc << '\n';
}

void
Diag911Collector::printBitmaps(const PaxTypeFare& paxFare)
{
  DiagCollector& dc(*this);

  if (!paxFare.flightBitmap().empty())
  {
    if (_shoppingTrx->diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "DURATION" &&
        ((paxFare.getDurationUsedInFVO() != _shoppingTrx->mainDuration()) ||
         (_shoppingTrx->isAltDates())))
    {
      dc << "\n Used Duration: "
         << ShoppingAltDateUtil::getNoOfDays(paxFare.getDurationUsedInFVO());
    }

    dc << "\n ";
  }

  printFlightBitmap(paxFare.flightBitmap().begin(), paxFare.flightBitmap().end(), dc);
}

void
Diag911Collector::printFFBitmaps(const PaxTypeFare& paxFare)
{
  const FlightFinderTrx* ffTrx = dynamic_cast<const FlightFinderTrx*>(_shoppingTrx);
  bool isPromoShopping = false;

  if ((ffTrx != nullptr) && (true == ffTrx->avlInS1S3Request()))
  {
    isPromoShopping = true;
  }

  DiagCollector& dc(*this);

  if (_shoppingTrx->isAltDates())
  {
    VecMap<uint64_t, PaxTypeFare::FlightBitmap>::const_iterator iter =
        paxFare.durationFlightBitmap().begin();
    VecMap<uint64_t, PaxTypeFare::FlightBitmap>::const_iterator iterEnd =
        paxFare.durationFlightBitmap().end();
    dc << std::endl;

    for (; iter != iterEnd; iter++)
    {
      dc << std::setw(6) << (iter->first / 1000000) << ": ";
      PaxTypeFare::FlightBitmapConstIterator bitMapIter = iter->second.begin();
      PaxTypeFare::FlightBitmapConstIterator bitMapEndIter = iter->second.end();

      // Do not display dummy sop state for promotional shopping
      if ((isPromoShopping) && !(iter->second.empty()))
      {
        bitMapEndIter--;
      }

      for (uint32_t col = 1; bitMapIter != bitMapEndIter; ++bitMapIter, ++col)
      {
        if (col == (DEFAULT_LINEWRAP_LENGTH - 7))
        {
          dc << "\n ";
          col = 1;
        }

        const uint8_t& fareBit = static_cast<const uint8_t&>(bitMapIter->_flightBit);

        if (fareBit == 0)
        {
          dc << '-';
        }
        else
        {
          dc << std::setw(1) << fareBit;
        }
      }

      dc << std::endl;
    } // ENF iter
  }
  else
  {
    if ((_firstNonFakedCell != nullptr) &&
        (_firstNonFakedCell->first.flags() & ItinIndex::ITININDEXCELLINFO_FAKEDFLIGHT))
    {
      dc << "\n"
         << "NO VALID FLIGHT"
         << "\n";
      return;
    }

    PaxTypeFare::FlightBitmapConstIterator bitMapIter = paxFare.flightBitmap().begin();
    PaxTypeFare::FlightBitmapConstIterator bitMapEndIter = paxFare.flightBitmap().end();

    // Do not display dummy sop state for promotional shopping
    if ((isPromoShopping) && !(paxFare.flightBitmap().empty()))
    {
      bitMapEndIter--;
    }

    dc << "\n ";
    printFlightBitmap(bitMapIter, bitMapEndIter, dc);
  }
}

void
Diag911Collector::printAltDatesBitmaps(const PaxTypeFare& paxFare)
{
  if (!_shoppingTrx->isAltDates() || paxFare.durationFlightBitmap().empty())
    return;

  DiagCollector& dc(*this);
  dc << std::endl;
  VecMap<uint64_t, PaxTypeFare::FlightBitmap>::const_iterator iter =
      paxFare.durationFlightBitmap().begin();

  for (; iter != paxFare.durationFlightBitmap().end(); iter++)
  {
    if (iter->second.empty())
      continue;
    dc << "DURATION: " << std::setw(2) << ShoppingAltDateUtil::getNoOfDays(iter->first)
       << std::endl;
    printFlightBitmap(iter->second.begin(), iter->second.end(), dc);
    dc << std::endl;
  }
}

void
Diag911Collector::printAltDatesMainDetails()
{
  if (!_shoppingTrx->isAltDates() || _shoppingTrx->durationAltDatePairs().empty())
    return;

  DiagCollector& dc(*this);
  dc << "MAIN DURATION :" << ShoppingAltDateUtil::getNoOfDays(_shoppingTrx->mainDuration())
     << std::endl;
  PricingTrx::DurationAltDatePairs::const_iterator iter =
      _shoppingTrx->durationAltDatePairs().begin();

  for (; iter != _shoppingTrx->durationAltDatePairs().end(); iter++)
  {
    dc << "DURATION :" << ShoppingAltDateUtil::getNoOfDays(iter->first) << std::endl;
    printDatePair(iter->second.begin(), iter->second.end(), dc);
  }
}

void
Diag911Collector::printFlightBitmap(PaxTypeFare::FlightBitmapConstIterator iBegin,
                                    PaxTypeFare::FlightBitmapConstIterator iEnd,
                                    DiagCollector& dc)
{
  uint32_t column = 1;

  for (; iBegin != iEnd; ++iBegin, ++column)
  {
    if (column == DEFAULT_LINEWRAP_LENGTH)
    {
      dc << "\n ";
      column = 1;
    }

    const uint8_t& fareBit = static_cast<const uint8_t&>(iBegin->_flightBit);

    if (fareBit == 0)
    {
      dc << '-';
    }
    else
    {
      if (fareBit != RuleConst::FLIGHT_EXCEEDS_CONXN_POINT_LIMIT)
        dc << std::setw(1) << fareBit;
      else
        dc << std::setw(1) << 'X';
    }
  }
}

void
Diag911Collector::printDatePair(PricingTrx::AltDatePairs::const_iterator iBegin,
                                PricingTrx::AltDatePairs::const_iterator iEnd,
                                DiagCollector& dc)
{
  for (; iBegin != iEnd; iBegin++)
    dc << iBegin->first.first.dateToString(MMDDYY, "") << "  "
       << iBegin->first.second.dateToString(MMDDYY, "") << std::endl;
}

bool
Diag911Collector::hasAnyValidFlightBitmap(const PaxTypeFare& paxFare)
{
  if (!_shoppingTrx->isAltDates())
    return !paxFare.isFlightBitmapInvalid();

  if (paxFare.durationFlightBitmap().empty())
    return false;

  VecMap<uint64_t, PaxTypeFare::FlightBitmap>::const_iterator iter =
      paxFare.durationFlightBitmap().begin();
  for (; iter != paxFare.durationFlightBitmap().end(); iter++)
    if (!paxFare.isFlightBitmapInvalid(iter->second))
      return true;
  return false;
}

} /* namespace tse */
