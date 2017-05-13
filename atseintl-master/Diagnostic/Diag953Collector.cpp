//----------------------------------------------------------------------------
//  File:        Diag953Collector.C
//  Created:     2008-02-06
//
//  Description: Diagnostic 953 formatter
//
//  Updates:
//
//  Copyright Sabre 2008
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

#include "Diagnostic/Diag953Collector.h"

#include "Common/Money.h"
#include "Common/ShoppingUtil.h"
#include "Common/Vendor.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/BoundFareAdditionalInfo.h"
#include "Diagnostic/Diag200Collector.h"
#include "Diagnostic/DiagCollector.h"
#include "Rules/RuleConst.h"

#include <iomanip>

namespace tse
{
Diag953Collector& Diag953Collector::operator<<(const ShoppingTrx& shoppingTrx)
{
  if (false == _active)
  {
    return (*this);
  }

  DiagCollector& dc(*this);

  dc << std::endl;
  dc << "**********************************************************" << std::endl;
  dc << "953 : FLIGHT RELATED VALIDATION RESULTS" << std::endl;
  dc << "**********************************************************" << std::endl;
  dc << std::endl;

  dc << "**********************************************************" << std::endl;
  dc << "RULE VALIDATION LEGEND" << std::endl;
  dc << "DIR: DIRECTIONALITY FAILED" << std::endl;
  dc << "R  : FAIL ROUTING" << std::endl;
  dc << "B  : FAIL BOOKING CODE" << std::endl;
  dc << "10 : FAIL CAT 10 COMBINATIONS PRE VALIDATION" << std::endl;
  dc << "10R: FARE HAVE CAT 10 RESTRICTIONS " << std::endl;
  dc << "OK : PASS" << std::endl;
  dc << "-  : NOT PROCESSED" << std::endl;
  dc << "**********************************************************" << std::endl;
  dc << std::endl;

  _trx = (PricingTrx*)&shoppingTrx;

  if (shoppingTrx.legs().empty())
  {
    dc << "ERROR: "
       << "Legs vector is empty for current shopping transaction." << std::endl;

    return (*this);
  }

  if ("Y" == dc.rootDiag()->diagParamMapItem("PRINTRULES"))
  {
    _printRules = true;
  }

  // Go thorough all legs
  for (uint32_t legId = 0; legId != shoppingTrx.legs().size(); legId++)
  {
    const ShoppingTrx::Leg& leg = shoppingTrx.legs()[legId];

    _legIndex = legId;

    // Print diagnostics for each carrier index
    (*this) << leg.carrierIndex();
  }

  return (*this);
}

Diag953Collector& Diag953Collector::operator<<(const ItinIndex& itinIndex)
{
  if (false == _active)
  {
    return (*this);
  }

  DiagCollector& dc(*this);

  // Adjust output to left justified
  dc.setf(std::ios::left, std::ios::adjustfield);

  // Read carrier parameters
  const std::string& filterCxr = dc.rootDiag()->diagParamMapItem("CX");

  // Go thorough all carriers keys
  ItinIndex::ItinMatrixConstIterator matrixIter;

  for (matrixIter = itinIndex.root().begin(); matrixIter != itinIndex.root().end(); matrixIter++)
  {
    const ItinIndex::ItinRow& itinRow = matrixIter->second;

    // Go thorough all connection keys
    ItinIndex::ItinRowConstIterator rowIter;

    for (rowIter = itinRow.begin(); rowIter != itinRow.end(); rowIter++)
    {
      const ItinIndex::ItinColumn& itinColumn = rowIter->second;

      // Go thorough all itin cells
      ItinIndex::ItinColumnConstIterator itinColumnIter;

      for (itinColumnIter = itinColumn.begin(); itinColumnIter != itinColumn.end();
           itinColumnIter++)
      {
        const ItinIndex::ItinCell& itinCell = (*itinColumnIter);

        const ItinIndex::ItinCellInfo& itinCellInfo = itinCell.first;

        // Skip processing of dummy itineraries
        if (true == (itinCellInfo.flags() & ItinIndex::ITININDEXCELLINFO_FAKEDIRECTFLIGHT))
        {
          continue;
        }

        _sopIndex = itinCellInfo.sopIndex();

        const Itin* itin = itinCell.second;

        // Check if itinerary object is not NULL
        if (nullptr == itin)
        {
          dc << "ERROR: Itinerary object is NULL." << std::endl;

          return (*this);
        }

        ShoppingTrx::SchedulingOption& sop =
            (dynamic_cast<ShoppingTrx*>(_trx))->legs()[_legIndex].sop()[itinCellInfo.sopIndex()];

        // Check if diagnostic parameters specify to exclude this
        // governing carrier
        if ((filterCxr.empty() == false) && (filterCxr != sop.governingCarrier()))
        {
          continue;
        }

        // Print information about leg
        const LocCode& origin = itin->travelSeg().front()->origin()->loc();
        const LocCode& destination = itin->travelSeg().back()->destination()->loc();

        dc << "**********************************************************" << std::endl;
        dc << "LEG: " << (_legIndex + 1) << " OF "
           << (dynamic_cast<ShoppingTrx*>(_trx))->legs().size() << ": " << origin << "-"
           << destination << std::endl;

        // Print scheduling option index number
        dc << "SCHEDULING OPTION ID: " << sop.originalSopId() << std::endl;

        // Print governing carrier for this sop
        dc << "GOVERNING CARRIER: " << sop.governingCarrier() << std::endl;

        // Display travel segments information
        dc << "TRAVEL SEGMENTS: ";

        // Go thorough all travel segments
        std::vector<TravelSeg*>::const_iterator segIter;

        for (segIter = itin->travelSeg().begin(); segIter != itin->travelSeg().end(); segIter++)
        {
          const TravelSeg* travelSeg = (*segIter);

          // Check if travel segment object is not NULL
          if (nullptr == travelSeg)
          {
            dc << "ERROR: Travel segment object is NULL." << std::endl;

            return (*this);
          }

          const AirSeg& airSegment = dynamic_cast<const AirSeg&>(*travelSeg);

          dc << travelSeg->origin()->loc() << "-" << travelSeg->destination()->loc() << " ("
             << airSegment.carrier() << ") ";
        }

        // print JCB flag
        if (PricingTrx::ESV_TRX == _trx->getTrxType())
        {
          dc << std::endl;
          if (itin->isJcb())
          {
            dc << "JCB FLAG: TRUE";
          }
          else
          {
            dc << "JCB FLAG: FALSE";
          }
        }

        dc << std::endl << "**********************************************************" << std::endl
           << std::endl;

        // Go thorough all fare markets
        uint32_t fareMarketId = 1;

        for (std::vector<FareMarket*>::const_iterator fareMarketIter = itin->fareMarket().begin();
             fareMarketIter != itin->fareMarket().end();
             fareMarketIter++, fareMarketId++)
        {
          const FareMarket* fareMarket = (*fareMarketIter);

          const LocCode& origin = fareMarket->origin()->loc();
          const LocCode& destination = fareMarket->destination()->loc();

          // Print fare market number, origin and destination
          dc << "----------------------------------------------------------" << std::endl;
          dc << "FARE MARKET: " << fareMarketId << " OF " << itin->fareMarket().size() << " : "
             << origin << "-" << destination;

          // Print fare market direction
          dc << " #  " << fareMarket->getDirectionAsString() << std::endl;

          // Print travle date
          dc << "TRAVEL DATE: " << std::setw(6) << fareMarket->travelDate().dateToString(DDMMM, "")
             << std::endl;
          dc << "----------------------------------------------------------" << std::endl;

          // Print diagnostics for each fare market
          (*this) << (*fareMarket);

          dc << std::endl;
        }
      }
    }
  }

  return (*this);
}

Diag953Collector& Diag953Collector::operator<<(const FareMarket& fareMarket)
{
  if (false == _active)
  {
    return *this;
  }

  DiagCollector& dc(*this);

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
        // Print information about requested passenger type
        dc << std::endl;
        dc << "REQUESTED PAXTYPE: " << paxTypeCortege.requestedPaxType()->paxType() << std::endl;

        uint32_t countOfPrintedFares = 0;

        for (const auto elem : paxTypeCortege.paxTypeFare())
        {
          PaxTypeFare& paxTypeFare = *elem;
          // Print header if it's a first valid fare
          if (0 == countOfPrintedFares)
          {
            dc << std::endl;

            dc << "  GI V RULE   FARE BASIS    TRF RTG  O O SAME CARRIER     AMT CUR FAR PAX RULE  "
                  "\n"
               << "                            NUM NUM  R I 102 103 104              TYP TYP "
                  "FAILED\n"
               << "- -- - ---- --------------- --- ---- - - --- --- --- -------- --- --- --- "
                  "------\n";
          }

          (*this) << paxTypeFare;

          countOfPrintedFares++;
        }

        if (0 == countOfPrintedFares)
        {
          dc << std::endl << "NO VALID FARES FOUND FOR MARKET: " << fareMarket.origin()->loc()
             << '-' << fareMarket.destination()->loc() << std::endl;
        }
      }
      else
      {
        dc << std::endl << "NO FARES FOUND FOR MARKET: " << fareMarket.origin()->loc() << '-'
           << fareMarket.destination()->loc() << std::endl;
      }
    }
  }
  else
  {
    dc << std::endl << "NO FARES FOUND FOR MARKET : " << fareMarket.origin()->loc() << '-'
       << fareMarket.destination()->loc() << std::endl;
  }

  dc << std::endl;

  return (*this);
}

Diag953Collector& Diag953Collector::operator<<(const PaxTypeFare& paxTypeFare)
{
  if (false == _active)
  {
    return (*this);
  }

  DiagCollector& dc(*this);

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

  dc << std::setw(16) << fareBasis << std::setw(4) << paxTypeFare.fareTariff() << std::setw(5)
     << paxTypeFare.routingNumber();

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
    dc << "  ";
  }

  // Print same carrier 102 RT indicator
  std::string sameCarrierRT =
      (false == paxTypeFare.fare()->fareInfo()->sameCarrier102()) ? " F " : " T ";
  dc << std::setw(3) << sameCarrierRT << " ";

  // Print same carrier 103 CT indicator
  std::string sameCarrierCT =
      (false == paxTypeFare.fare()->fareInfo()->sameCarrier103()) ? " F " : " T ";
  dc << std::setw(3) << sameCarrierCT << " ";

  // Print same carrier 104 EOE indicator
  std::string sameCarrierEOE =
      (false == paxTypeFare.fare()->fareInfo()->sameCarrier104()) ? " F " : " T ";
  dc << std::setw(3) << sameCarrierEOE << " ";

  // Print money amount and currency
  dc << std::setw(8) << Money(paxTypeFare.fareAmount(), paxTypeFare.currency()) << " ";

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

  // Print flight related validation results
  uint8_t fareBit = 0;
  VecMap<uint32_t, PaxTypeFare::FlightBit>::const_iterator flightBitIter;
  flightBitIter = paxTypeFare.flightBitmapESV().find(_sopIndex);

  if (flightBitIter != paxTypeFare.flightBitmapESV().end())
  {
    PaxTypeFare::FlightBit flightBit = flightBitIter->second;
    fareBit = flightBit._flightBit;
  }

  dc << std::setw(6);
  bool scvResult = true;
  // Check if fare directionality and fare market direction
  // are matched correctly if not skip displaying diagnostic
  // for this fare
  if (((paxTypeFare.fareMarket()->direction() == FMDirection::OUTBOUND) &&
       (paxTypeFare.directionality() == TO)) ||
      ((paxTypeFare.fareMarket()->direction() == FMDirection::INBOUND) &&
       (paxTypeFare.directionality() == FROM)))
  {
    dc << "DIR";
    scvResult = false;
  }
  // Shopping Component Validation results
  else if (!paxTypeFare.isCat15SecurityValid())
  {
    dc << "SCV15S";
    scvResult = false;
  }
  else
  {
    for (int cat = 1; cat <= 15; ++cat)
    {
      if (!paxTypeFare.isCategoryValid(cat))
      {
        dc << std::setw(4) << "SCV " << std::setw(2) << cat;
        scvResult = false;
        break;
      }
    }
  }

  if (scvResult)
  {
    if (fareBit == 0)
    {
      dc << "-";
    }
    else if (fareBit == RuleConst::PASSED)
    {
      dc << "OK";
    }
    else if (fareBit == RuleConst::CAT_10_FAIL)
    {
      dc << "10";
    }
    else if (fareBit == RuleConst::CAT_10_RESTRICTIONS)
    {
      dc << "10R";
    }
    else if (fareBit == RuleConst::CAT14_FAIL)
    {
      dc << "FRV 14";
    }
    else if (fareBit == RuleConst::CAT11_FAIL)
    {
      dc << "FRV 11";
    }
    else if (fareBit == RuleConst::QUALIFYCAT4_FAIL)
    {
      dc << "FRV 4Q";
    }
    else
    {
      dc << std::setw(4) << "FRV " << std::setw(1) << fareBit;
    }
  }

  // Print rule bindings for fare

  if (true == _printRules)
  {
    const Record2ReferenceVector* recor2ReferencesVec = paxTypeFare.fare()->_fareInfo->references();

    if (nullptr != recor2ReferencesVec)
    {
      Record2ReferenceVector::const_iterator record2ReferenceIter;

      dc << " FARE RULES: ";

      for (record2ReferenceIter = recor2ReferencesVec->begin();
           record2ReferenceIter != recor2ReferencesVec->end();
           ++record2ReferenceIter)
      {
        const Record2Reference record2Reference = (*record2ReferenceIter);

        if (FARERULE == record2Reference._matchType)
        {
          dc << "(" << record2Reference._catNumber << "," << record2Reference._sequenceNumber << ","
             << record2Reference._flipIndicator << ") ";
        }
      }

      dc << "FOOTNOTES: ";

      for (record2ReferenceIter = recor2ReferencesVec->begin();
           record2ReferenceIter != recor2ReferencesVec->end();
           ++record2ReferenceIter)
      {
        const Record2Reference record2Reference = (*record2ReferenceIter);

        if (FOOTNOTE == record2Reference._matchType)
        {
          dc << "(" << record2Reference._catNumber << "," << record2Reference._sequenceNumber << ","
             << record2Reference._flipIndicator << ") ";
        }
      }
    }
    else
    {
      dc << " No RECORD 2 references found.";
    }
  }

  dc << std::endl;

  return (*this);
}
}
