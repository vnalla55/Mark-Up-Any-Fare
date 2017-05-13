//----------------------------------------------------------------------------
//  File:        Diag904Collector.C
//  Created:     2004-08-20
//
//  Description: Diagnostic 904 formatter
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
#include "Diagnostic/Diag904Collector.h"

#include "Common/Money.h"
#include "Common/ShoppingUtil.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/ItinIndex.h"
#include "DataModel/PaxType.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/Diag200Collector.h"
#include "Diagnostic/Diag902Collector.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Diagnostic/BrandedDiagnosticUtil.h"
#include "Rules/RuleConst.h"

#include <iomanip>

namespace tse
{
Diag904Collector& Diag904Collector::operator<<(const ShoppingTrx& shoppingTrx)
{
  if (false == _active)
  {
    return *this;
  }

  DiagCollector& dc(*this);

  _trx = (PricingTrx*)&shoppingTrx;
  _shoppingTrx = &shoppingTrx;
  _isSpanishDiscountTrx =
      _shoppingTrx->getOptions()->getSpanishLargeFamilyDiscountLevel() !=
          SLFUtil::DiscountLevel::NO_DISCOUNT;
  _isBrandedFaresPath = _shoppingTrx->getRequest()->isBrandedFaresRequest();
  _brandProgramVec = _shoppingTrx->brandProgramVec();

  if (shoppingTrx.legs().empty())
  {
    dc << "ERROR: "
       << "Legs vector is empty for current shopping transaction." << std::endl;

    return *this;
  }

  dc << "904 : FARE MARKETS WITH RULE VALIDATION RESULTS" << std::endl;

  if (_isBrandedFaresPath)
  {
    BrandedDiagnosticUtil::displayAllBrandIndices(dc, _brandProgramVec);
    dc << "* * * * * * * * * * * * * * * * * * * * * * * * * * * *\n";
    dc << "*******************************************************\n\n";
    dc << "BR : LIST OF AVAILABLE BRANDS" << std::endl;
    dc << "     H - HARD PASS" << std::endl;
    dc << "     S - SOFT PASS" << std::endl;
    dc << "     F - FAIL" << std::endl;
    dc << "***************************************************" << std::endl;
  }

  if (shoppingTrx.isAltDates())
  {
    dc << "DATEPAIR FAIL CODE" << std::endl;
    dc << "2 : FAIL CAT 2 DAY TIME" << std::endl;
    dc << "3 : FAIL CAT 3 SEASON" << std::endl;
    dc << "5 : FAIL CAT 5 ADVANCE PURCHASE" << std::endl;
    dc << "6 : FAIL CAT 6 MINIMUM STAY" << std::endl;
    dc << "7 : FAIL CAT 7 MAXIMUM STAY" << std::endl;
    dc << "O : FAIL CAT 11 BLACKOUT" << std::endl;
    dc << "T : FAIL CAT 14 TRAVEL RESTRICTION" << std::endl;
    dc << "X : FAIL CAT 15 SALE RESTRICTION" << std::endl;
    dc << "E : FAIL FARE EFFECTIVE DATE" << std::endl;
    dc << "F : FAIL FARE EXPIRED DATE" << std::endl;
    dc << "D : NO DATE FOUND" << std::endl;
  }

  // Go thorough all legs
  for (uint32_t legId = 0; legId != shoppingTrx.legs().size(); legId++)
  {
    const ShoppingTrx::Leg& curLeg = shoppingTrx.legs()[legId];
    _legIndex = (legId + 1);

    // Print diagnostics for each carrier index
    dc << curLeg.carrierIndex();
  }

  return *this;
}

Diag904Collector& Diag904Collector::operator<<(const ItinIndex& itinIndex)
{
  if (false == _active)
  {
    return *this;
  }

  DiagCollector& dc(*this);

  if (itinIndex.root().empty())
  {
    dc << "ERROR: "
       << "Carrier index for currently processed leg is empty." << std::endl;

    return *this;
  }

  // Adjust output to left justified
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << std::endl;

  // Read carrier and market filter parameters
  const std::string& filterCxr = dc.rootDiag()->diagParamMapItem("CX");
  const std::string& filterFM = dc.rootDiag()->diagParamMapItem("FM");

  std::string filterFMOrigin = "";
  std::string filterFMDest = "";

  if (6 == filterFM.size())
  {
    filterFMOrigin = filterFM.substr(0, 3);
    filterFMDest = filterFM.substr(3, 3);
  }

  // Go thorough all carrier rows in carrier index for currently processed leg
  uint32_t matrixId = 1;

  for (ItinIndex::ItinMatrixConstIterator matrixIter = itinIndex.root().begin();
       matrixIter != itinIndex.root().end();
       matrixIter++, matrixId++)
  {
    const Itin* curItin = nullptr;

    if ((nullptr != _trx) && (PricingTrx::ESV_TRX == _trx->getTrxType()))
    {
      curItin = ShoppingUtil::getDummyItineraryFromCarrierIndex((tse::ItinIndex&)itinIndex,
                                                                (uint32_t&)matrixIter->first);

      if ((nullptr == curItin) || (curItin->travelSeg().empty()))
      {
        dc << "ERROR: "
           << "Dummy itinerary object is NULL or travel segment vector for this itinerary is empty."
           << std::endl;

        continue;
      }

      if (curItin->fareMarket().empty())
      {
        dc << "ERROR: "
           << "Fare markets vector for dummy itinerary is empty." << std::endl;

        continue;
      }

      // Get governing carrier for currently processed carrier index
      const AirSeg& airSegment = dynamic_cast<const AirSeg&>(*(curItin->travelSeg().front()));
      const CarrierCode& curCarrier = airSegment.carrier();

      // Go thorough all fare markets
      uint32_t fareMarketId = 1;

      for (std::vector<FareMarket*>::const_iterator fareMarketIter = curItin->fareMarket().begin();
           fareMarketIter != curItin->fareMarket().end();
           fareMarketIter++, fareMarketId++)
      {
        const FareMarket* curFareMarket = *fareMarketIter;

        const LocCode& origin = curFareMarket->boardMultiCity();
        const LocCode& destination = curFareMarket->offMultiCity();

        // Check if diagnostic parameters specify to exclude this fare
        // market
        if (((filterCxr.empty() == false) && (filterCxr != curCarrier)) ||
            ((filterFMOrigin.empty() == false) && (filterFMOrigin != origin)) ||
            ((filterFMDest.empty() == false) && (filterFMDest != destination)))
        {
          continue;
        }

        // Print leg number and governing carrier
        dc << "LEG " << _legIndex << " GOVCXR " << matrixId << " OF " << itinIndex.root().size()
           << " : " << curCarrier << std::endl;

        // Print fare market number, origin and destination
        dc << "FARE MARKET " << fareMarketId << " OF " << curItin->fareMarket().size() << " : "
           << origin << "-" << destination;

        // Print diagnostics for each fare market
        dc << *curFareMarket;
      }
    }
    else
    {
      const ItinIndex::ItinCell* curCell =
          ShoppingUtil::retrieveDirectItin(itinIndex, matrixIter->first, ItinIndex::CHECK_NOTHING);

      if (nullptr == curCell)
      {
        dc << "ERROR: "
           << "Itin cell object for direct flight is NULL." << std::endl;

        continue;
      }

      curItin = curCell->second;

      if ((nullptr == curItin) || (curItin->fareMarket().empty()))
      {
        dc << "ERROR: "
           << "Direct flight itinerary object is NULL or fare markets vector for this itinerary is "
              "empty." << std::endl;

        continue;
      }

      const CarrierCode& curCarrier = curItin->fareMarket().front()->governingCarrier();
      const LocCode& origin = curItin->travelSeg().front()->boardMultiCity();
      const LocCode& dest = curItin->travelSeg().back()->offMultiCity();

      // Check if diagnostic parameters specify to exclude this fare
      // market
      if (((filterCxr.empty() == false) && (filterCxr != curCarrier)) ||
          ((filterFMOrigin.empty() == false) && (filterFMOrigin != origin)) ||
          ((filterFMDest.empty() == false) && (filterFMDest != dest)))
      {
        continue;
      }

      // Print leg number and governing carrier
      dc << "LEG " << _legIndex << " GOVCXR " << matrixId << " OF " << itinIndex.root().size()
         << " : " << curCarrier << std::endl;

      const std::vector<FareMarket*> fareMarkets(curItin->fareMarket());
      size_t fmIdx(1);
      // Print diagnostics for each fare market
      for (FareMarket* fm : fareMarkets)
      {
        dc << "--------------------------------------------------------" << std::endl;
        if (_shoppingTrx->isIataFareSelectionApplicable())
        {
          dc << "MARKET #" << fmIdx++ << ":" << std::endl;
        }
        // Print origin and destination from travel segments
        dc << curItin->travelSeg().front()->origin()->loc() << "-"
           << curItin->travelSeg().back()->destination()->loc();

        dc << *fm;
        if (!_shoppingTrx->isIataFareSelectionApplicable())
          break;
      }
      dc << "--------------------------------------------------------" << std::endl;

      ShoppingTrx* shoppingTrx(dynamic_cast<ShoppingTrx*>(dc.trx()));
      if (shoppingTrx && shoppingTrx->isSumOfLocalsProcessingEnabled())
      {
        std::vector<FareMarket*>::const_iterator fareMarketIt(fareMarkets.begin());
        fareMarketIt++;
        for (; fareMarketIt != fareMarkets.end(); ++fareMarketIt)
        {
          dc << "LEG " << _legIndex << " GOVCXR " << matrixId << " OF " << itinIndex.root().size()
             << " : " << curCarrier << std::endl;

          dc << (*fareMarketIt)->origin()->loc() << "-" << (*fareMarketIt)->destination()->loc()
             << ", LOCAL FM";
          dc << *(*fareMarketIt);
        }
      }
    }
  }

  return *this;
}

Diag904Collector& Diag904Collector::operator<<(const FareMarket& fareMarket)
{
  if (false == _active)
  {
    return *this;
  }

  DiagCollector& dc(*this);

  // Print fare market direction
  dc << " #  " << fareMarket.getDirectionAsString() << std::endl;

  // Print travel date
  dc << "Travel Date : " << std::setw(6) << fareMarket.travelDate().dateToString(DDMMM, "")
     << std::endl;

  dc << "  Custom SOP FareMarket: "
     << (_shoppingTrx->isCustomSolutionFM(&fareMarket) ? "Yes" : "No") << std::endl;

  if (_isSpanishDiscountTrx)
  {
    dc << "  Spanish Family Discount: "
       << (_shoppingTrx->isSpanishDiscountFM(&fareMarket) ? "Yes" : "No") << std::endl;
  }

  // Print JCB flag
  if (PricingTrx::ESV_TRX == _trx->getTrxType())
  {
    dc << std::endl << "JCB FLAG: ";
    if (fareMarket.isJcb())
    {
      dc << "TRUE";
    }
    else
    {
      dc << "FALSE";
    }
  }

  if (!fareMarket.paxTypeCortege().empty())
  {
    for (std::vector<PaxTypeBucket>::const_iterator paxTypeCortegeIter =
             fareMarket.paxTypeCortege().begin();
         paxTypeCortegeIter != fareMarket.paxTypeCortege().end();
         paxTypeCortegeIter++)
    {
      const PaxTypeBucket& paxTypeCortege = *paxTypeCortegeIter;

      if (!paxTypeCortege.paxTypeFare().empty())
      {
        dc << std::endl;

        dc << "REQUESTED PAXTYPE : " << paxTypeCortege.requestedPaxType()->paxType() << std::endl;

        dc << " INBOUND CURRENCY : " << paxTypeCortege.inboundCurrency() << std::endl;

        dc << "OUTBOUND CURRENCY : " << paxTypeCortege.outboundCurrency() << std::endl;
        dc << "GOVERNING CARRIER : " << fareMarket.governingCarrier() << '\n';

        if (fareMarket.isThroughFarePrecedenceNGS())
          dc << "THROUGH FARE PRECEDENCE ENABLED : Yes" << std::endl;

        if (_isBrandedFaresPath)
        {
          dc << "VALID BRANDS/PROGRAMS : ";
          BrandCodeSet validBrandCodes;
          for (const auto& elem : fareMarket.brandProgramIndexVec())
          {
            dc << elem << " ";
            validBrandCodes.insert(_trx->brandProgramVec()[elem].second->brandCode());
          }
          dc << "\nVALID BRAND CODES : ";
          for (const auto& validBrandCode : validBrandCodes)
            dc << validBrandCode << " ";
          dc << "\n";
        }

        dc << std::endl;

        if (_trx->awardRequest())
        {
          dc << "  GI V RULE   FARE BASIS    TRF O O     AMT CUR      MIL FAR PAX RULE  \n"
             << "                            NUM R I                      TYP TYP FAILED\n"
             << "- -- - ---- --------------- --- - - ------- --- -------- --- --- ------\n";
        }
        else
        {
          dc << "  GI V RULE   FARE BASIS    TRF O O      AMT CUR FAR PAX RULE  \n"
             << "                            NUM R I              TYP TYP FAILED\n"
             << "- -- - ---- --------------- --- - - -------- --- --- --- ------\n";
        }

        for (const auto elem : paxTypeCortege.paxTypeFare())
        {
          // Print diagnostics for each pax type fare
          PaxTypeFare& paxTypeFare = *elem;

          if ((nullptr != _trx) && (PricingTrx::ESV_TRX == _trx->getTrxType()))
          {
            // Check if fare directionality and fare market direction are matched
            // correctly if not skip displaying diagnostic for this fare
            if (((fareMarket.direction() == FMDirection::OUTBOUND) &&
                 (paxTypeFare.directionality() == TO)) ||
                ((fareMarket.direction() == FMDirection::INBOUND) &&
                 (paxTypeFare.directionality() == FROM)))
            {
              continue;
            }
          }

          dc << paxTypeFare;
        }
      }
      else
      {
        dc << std::endl << "NO FARES FOUND FOR MARKET : " << fareMarket.origin()->loc() << '-'
           << fareMarket.destination()->loc()
           << ". REQUESTED PAXTYPE : " << paxTypeCortege.requestedPaxType()->paxType() << std::endl;
      }
    }
  }
  else
  {
    dc << std::endl << "NO FARES FOUND FOR MARKET : " << fareMarket.origin()->description() << '-'
       << fareMarket.destination()->description() << std::endl;
  }

  dc << std::endl;

  return *this;
}

Diag904Collector& Diag904Collector::operator<<(const PaxTypeFare& paxTypeFare)
{

  if (false == _active)
  {
    return *this;
  }

  DiagCollector& dc(*this);

  _displayR1 = true; // Display rule validation

  // Adjust output to left justified
  dc.setf(std::ios::left, std::ios::adjustfield);

  // Print type of fare
  dc << std::setw(2) << cnvFlags(paxTypeFare);

  // Print global direction, vendor and rule number
  std::string globalDirection;
  globalDirectionToStr(globalDirection, paxTypeFare.fare()->globalDirection());

  dc << std::setw(3) << globalDirection << std::setw(2) << Vendor::displayChar(paxTypeFare.vendor())
     << std::setw(5) << paxTypeFare.ruleNumber();

  // Print fare basis code and tariff
  std::string fareBasis = paxTypeFare.createFareBasis(*_trx, false);

  if (fareBasis.size() > 15)
  {
    fareBasis = fareBasis.substr(0, 15) + "*";
  }

  dc << std::setw(16) << fareBasis << std::setw(4) << paxTypeFare.fareTariff();

  // Print trip type
  dc << std::setw(2) << DiagnosticUtil::getOwrtChar(paxTypeFare);

  // Print fare directionality
  if (paxTypeFare.directionality() == FROM)
  {
    dc << std::setw(2) << "O";
  }
  else if (paxTypeFare.directionality() == TO)
  {
    dc << std::setw(2) << "I";
  }
  else
  {
    dc << " ";
  }

  // Print money amount and currency
  dc << std::setw(8) << Money(paxTypeFare.fareAmount(), paxTypeFare.currency()) << " ";

  if (_trx->awardRequest())
  {
    dc.setf(std::ios::right);
    dc << std::setw(8) << paxTypeFare.mileage() << " ";
  }

  // Print fare type
  if (!paxTypeFare.isFareClassAppMissing())
  {
    dc << std::setw(4) << paxTypeFare.fcaFareType();
  }
  else
  {
    dc << "UNK ";
  }

  // Print passenger type code
  if (!paxTypeFare.isFareClassAppSegMissing())
  {
    if (paxTypeFare.fcasPaxType().empty())
    {
      dc << "*** ";
    }
    else
    {
      dc << std::setw(4) << paxTypeFare.fcasPaxType();
    }
  }
  else
  {
    dc << "UNK ";
  }

  // Print information about rule number which fail this fare
  if (((nullptr != _trx) && (PricingTrx::ESV_TRX == _trx->getTrxType())) &&
      (!paxTypeFare.shoppingComponentValidationPerformed()))
  {
    dc << "  -  ";
  }
  else if (!paxTypeFare.isValidForBranding())
  {
    dc << "NO_BRAND";
  }
  else if (!paxTypeFare.shoppingComponentValidationFailed() && paxTypeFare.areAllCategoryValid())
  {
    dc << " ";
  }
  else
  {
    if (paxTypeFare.fare()->isDirectionalityFail())
    {
      // fare invalidated by directionality check
      //
      dc << "DIR  ";
    }
    else if (!paxTypeFare.isCat15SecurityValid())
    {
      dc << "CAT-15";
    }
    else
    {
      const size_t numCategories = 50;
      size_t cat = 1;

      for (cat = 1; cat != numCategories; ++cat)
      {
        if (paxTypeFare.isCategoryValid(cat) == false)
        {
          break;
        }
      }

      if (cat < numCategories)
      {
        dc << "CAT-" << cat;
      }
      else
      {
        dc << "UNK";
      }

      if ((paxTypeFare.altDateStatus().empty() == false) &&
          ((cat == RuleConst::SALE_RESTRICTIONS_RULE) || (cat == RuleConst::ELIGIBILITY_RULE)) &&
          (_trx->getTrxType() != PricingTrx::FF_TRX))
      {
        dc << std::endl;
        return *this;
      }
    }
  }

  if (paxTypeFare.isKeepForRoutingValidation())
  {
    dc << " SR";
  }

  if (_isBrandedFaresPath && (paxTypeFare.getBrandStatusVec().size() > 0))
  {
    dc << "\nBR  ";
    for (const auto& elem : _brandProgramVec)
      dc << (char)paxTypeFare.getBrandStatus(*_trx, &elem.second->brandCode());
  }

  dc << std::endl;

  if (Vendor::displayChar(paxTypeFare.vendor()) == '*')
  {
    dc << "    " << paxTypeFare.vendor();
  }

  dc << std::endl;

  if (_trx->getTrxType() == PricingTrx::FF_TRX)
  {

    if (_trx->diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ALTDATES")
    {

      outputAltDateStatus(paxTypeFare);
      dc << std::endl;
    }
  }
  else
  {
    // Print additional information about alternate dates
    int8_t count = 0;
    for (const auto& elem : paxTypeFare.altDateStatus())
    {
      DatePair myPair = elem.first;

      dc << myPair.first.dateToString(MMDDYY, "") << "-" << myPair.second.dateToString(MMDDYY, "");

      if (0 == elem.second)
      {
        dc << " P"
           << "      ";
      }
      else
      {
        dc << " F" << (uint32_t)elem.second << "     ";
      }

      if (2 == count)
      {
        count = 0;
        dc << std::endl;
      }
      else
      {
        count++;
      }
    }
    if (count > 0)
    {
      dc << std::endl;
    }
  }

  return *this;
}

void
Diag904Collector::outputAltDateStatus(const PaxTypeFare& paxFare)
{

  DiagCollector& dc(*this);

  dc << "---------------------------------------------------" << std::endl
     << "Alt Date Pairs Status: " << std::endl //<< paxFare.altDateStatus().size() << "\n";
     << "---------------------------------------------------" << std::endl;
  int8_t count = 0;

  for (const auto& elem : paxFare.altDateStatus())
  {
    DatePair myPair = elem.first;
    dc << myPair.first.dateToString(DDMMYYYY, ".") << "-"
       << myPair.second.dateToString(DDMMYYYY, ".");

    switch (elem.second)
    {
    case 0:
      dc << " P"
         << "      ";
      break;
    case RuleConst::CAT2_FAIL:
      dc << " 2"
         << "      ";
      break;
    case RuleConst::CAT3_FAIL:
      dc << " 3"
         << "      ";
      break;

    case RuleConst::CAT5_FAIL:
      dc << " 5"
         << "      ";
      break;
    case RuleConst::CAT6_FAIL:
      dc << " 6"
         << "      ";
      break;

    case RuleConst::CAT7_FAIL:
      dc << " 7"
         << "      ";
      break;
    case RuleConst::CAT11_FAIL:
      dc << " O"
         << "      ";
      break;
    case RuleConst::CAT14_FAIL:
      dc << " T"
         << "      ";
      break;
    case RuleConst::CAT15_FAIL:
      dc << " X"
         << "      ";
      break;
    default:
      dc << " F"
         << "      ";
      break;
    }

    count %= 2;
    if (!count++)
    {
      dc << "\n";
    }
  } // ENF_FOR
}
}
