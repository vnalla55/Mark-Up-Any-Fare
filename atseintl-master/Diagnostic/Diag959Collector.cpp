//----------------------------------------------------------------------------
//  File:        Diag959Collector.C
//  Created:     2008-07-05
//
//  Description: Diagnostic 959 formatter
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
#include "Diagnostic/Diag959Collector.h"

#include "Common/ClassOfService.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/Money.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/TravelSeg.h"

#include <iterator>
#include <vector>

namespace tse
{
namespace
{
ConfigurableValue<bool>
groupItinCfg("SHOPPING_OPT", "GROUP_ITINS", true);
ConfigurableValue<bool>
groupInterlineItinCfg("SHOPPING_OPT", "GROUP_INTERLINE_ITINS", true);
ConfigurableValue<int>
maxGroupCfg("SHOPPING_OPT", "MAX_GROUP", -1);
ConfigurableValue<bool>
regroupConnectingItinsCfg("SHOPPING_OPT", "REGROUP_CONNECTING_ITINS", false);
ConfigurableValue<int>
maxFamiliesInGroupCfg("SHOPPING_OPT", "MAX_FAMILIES_IN_GROUP", 15);
}

Diag959Collector&
Diag959Collector::
operator<<(const ShoppingTrx& shoppingTrx)
{
  if (false == _active)
  {
    return (*this);
  }

  DiagCollector& dc(*this);

  dc << std::endl;
  dc << "**********************************************************" << std::endl;
  dc << "959 : ESV RESPONSE DETAILS" << std::endl;
  dc << "**********************************************************" << std::endl;
  dc << std::endl;

  // Print server options
  int maxGroup = maxGroupCfg.getValue();
  int maxFamiliesInGroup = maxFamiliesInGroupCfg.getValue();

  const char groupItin = groupItinCfg.getValue() ? 'Y' : 'N';
  const char groupInterlineItin = groupInterlineItinCfg.getValue() ? 'Y' : 'N';
  const char regroupConnectingItins = regroupConnectingItinsCfg.getValue() ? 'Y' : 'N';

  dc << "GROUP ITINS : " << groupItin << std::endl;
  dc << "GROUP INTERLINE_ITINS : " << groupInterlineItin << std::endl;
  dc << "MAX GROUP : " << maxGroup << std::endl;
  dc << "REGROUP CONNECTING ITINS : " << regroupConnectingItins << std::endl;
  dc << "MAX FAMILIES IN GROUP : " << maxFamiliesInGroup << std::endl;
  dc << std::endl;

  // Print number of solutions
  dc << "SOLUTIONS COUNT: " << shoppingTrx.flightMatrixESV().size() << std::endl;
  dc << "ESTIMATED SOLUTIONS COUNT: " << shoppingTrx.estimateMatrixESV().size() << std::endl;
  dc << std::endl;

  _trx = (PricingTrx*)&shoppingTrx;

  printGroupingSummaryInfo(shoppingTrx);

  if ("Y" == dc.rootDiag()->diagParamMapItem("SV"))
  {
    _printSVOutput = true;
  }

  // Print flight matrix
  uint32_t solutionId = 0;
  std::vector<ESVSolution*>::const_iterator esvSolutionIter;

  for (esvSolutionIter = shoppingTrx.flightMatrixESV().begin();
       esvSolutionIter != shoppingTrx.flightMatrixESV().end();
       esvSolutionIter++, solutionId++)
  {
    ESVSolution* esvSolution = (*esvSolutionIter);

    if (nullptr == esvSolution)
    {
      dc << "ERROR: "
         << "ERROR: ESV Solution object is NULL." << std::endl;

      return (*this);
    }

    // Print solution
    dc << std::endl << "**********************************************************" << std::endl;
    dc << "SOLUTION: " << solutionId << std::endl;
    (*this) << (*esvSolution);
    dc << std::endl;
  }

  // Print estimate matrix
  solutionId = 0;

  for (esvSolutionIter = shoppingTrx.estimateMatrixESV().begin();
       esvSolutionIter != shoppingTrx.estimateMatrixESV().end();
       esvSolutionIter++, solutionId++)
  {
    ESVSolution* esvSolution = (*esvSolutionIter);

    if (nullptr == esvSolution)
    {
      dc << "ERROR: "
         << "ERROR: ESV Estimated solution object is NULL." << std::endl;

      return (*this);
    }

    // Print estimated solution
    dc << "**********************************************************" << std::endl;
    dc << "ESTIMATED SOLUTION: " << solutionId << std::endl;
    (*this) << (*esvSolution);
    dc << "**********************************************************" << std::endl;
    dc << std::endl;
  }

  return (*this);
}

Diag959Collector&
Diag959Collector::
operator<<(const ESVSolution& esvSolution)
{
  if (false == _active)
  {
    return (*this);
  }

  DiagCollector& dc(*this);

  dc.setf(std::ios::left, std::ios::adjustfield);

  if (!(dynamic_cast<ShoppingTrx*>(_trx))->legs().empty())
  {
    dc << "  GROUP ID: " << esvSolution.groupId()
       << ", CARRIER GROUP ID: " << esvSolution.carrierGroupId() << std::endl;

    std::string queueType;

    switch (esvSolution.queueType())
    {
    case MPNonstopOnline:
      queueType = "Must price, non stop online";
      break;

    case MPOutNonstopOnline:
      queueType = "Must price, outbound non stop online";
      break;

    case MPNonstopInterline:
      queueType = "Must price, non stop interline";
      break;

    case MPOutNonstopInterline:
      queueType = "Must price, outbound non stop interline";
      break;

    case MPSingleStopOnline:
      queueType = "Must price, single stop online";
      break;

    case MPRemainingOnline:
      queueType = "Must price, remaining online";
      break;

    case MPRemaining:
      queueType = "Must price, remaining";
      break;

    case LFSOnline:
      queueType = "Low fare search, online";
      break;

    case LFSOnlineByCarrier:
      queueType = "Low fare search, online by carrier";
      break;

    case LFSRemaining:
      queueType = "Low fare search, remaining";
      break;

    default:
      queueType = "Unsupported queue type";
      break;
    }

    dc << "  QUEUE TYPE: " << queueType << std::endl;

    // Print money amounts and currency
    dc << std::setw(20) << "  TOTAL PRICE: " << std::setw(8)
       << Money(esvSolution.totalPrice(), "NUC") << std::endl;

    dc << std::setw(20) << "  TOTAL PENALTY: " << std::setw(8)
       << Money(esvSolution.totalPenalty(), "NUC") << std::endl;

    dc << std::setw(20) << "  TOTAL AMOUNT: " << std::setw(8)
       << Money(esvSolution.totalAmt(), "NUC") << std::endl;

    dc << "  JCB FLAG: " << (esvSolution.isJcb() ? "TRUE" : "FALSE") << std::endl;

    dc << "  PRICING UNIT TYPE: ";

    switch (esvSolution.pricingUnitType())
    {
    case PricingUnit::Type::ONEWAY:
      dc << "ONE WAY" << std::endl;
      break;

    case PricingUnit::Type::ROUNDTRIP:
      dc << "ROUND TRIP" << std::endl;
      break;

    case PricingUnit::Type::CIRCLETRIP:
      dc << "CIRCLE TRIP" << std::endl;
      break;

    case PricingUnit::Type::OPENJAW:
      dc << "OPEN JAW" << std::endl;
      break;

    default:
      dc << "UNKNOWN" << std::endl;
    }

    // Print outbound option
    _legIndex = 0;
    dc << "----------------------------------------------------------" << std::endl;
    (*this) << esvSolution.outboundOption();

    // Print inbound option if it exist
    if ((dynamic_cast<ShoppingTrx*>(_trx))->legs().size() == 2)
    {
      _legIndex = 1;
      dc << "----------------------------------------------------------" << std::endl;
      (*this) << esvSolution.inboundOption();
    }

    if (true == _printSVOutput)
    {
      dc << std::endl;
      dc << "----------------------SABRE VIEW--------------------------" << std::endl;
      printSabreViewCommnads(esvSolution);
      dc << "----------------------SABRE VIEW--------------------------" << std::endl;
    }
  }

  return (*this);
}

Diag959Collector&
Diag959Collector::
operator<<(const ESVOption& esvOption)
{
  if (false == _active)
  {
    return (*this);
  }

  DiagCollector& dc(*this);

  ShoppingTrx::SchedulingOption& sop =
      (dynamic_cast<ShoppingTrx*>(_trx))->legs()[_legIndex].sop()[esvOption.sopId()];

  _sopIndex = esvOption.sopId();

  if (0 == _legIndex)
  {
    // Print scheduling option index number
    dc << "OUTBOUND FLIGHT ID: " << sop.originalSopId() << std::endl;
  }
  else
  {
    // Print scheduling option index number
    dc << "INBOUND FLIGHT ID: " << sop.originalSopId() << std::endl;
  }

  // Display travel segments information
  dc << "TRAVEL SEGMENTS: " << std::endl;

  // Go thorough all travel segments
  std::vector<TravelSeg*>::const_iterator segIter;
  uint32_t travelSegId = 0;

  for (segIter = sop.itin()->travelSeg().begin(); segIter != sop.itin()->travelSeg().end();
       segIter++, travelSegId++)
  {
    const TravelSeg* travelSeg = (*segIter);

    // Check if travel segment object is not NULL
    if (nullptr == travelSeg)
    {
      dc << "ERROR: Travel segment object is NULL." << std::endl;

      return (*this);
    }

    const AirSeg& airSegment = dynamic_cast<const AirSeg&>(*travelSeg);

    dc << "  " << (travelSegId + 1) << " " << travelSeg->origin()->loc() << "-"
       << travelSeg->destination()->loc() << " " << airSegment.carrier() << " "
       << airSegment.flightNumber() << " (" << airSegment.operatingCarrierCode() << ")"
       << std::endl;

    dc << "    DEPARTURE: " << airSegment.departureDT().dateToString(YYYYMmmDD, "-") << " "
       << airSegment.departureDT().timeToString(HHMM, ":") << std::endl;

    dc << "    ARRIVAL:   " << airSegment.arrivalDT().dateToString(YYYYMmmDD, "-") << " "
       << airSegment.arrivalDT().timeToString(HHMM, ":") << std::endl;
  }

  // Display information about fares
  dc << "FARES: " << std::endl;
  dc << "  GI V RULE   FARE BASIS    TRF RTG  O O SAME CARRIER     AMT CUR FAR PAX RULE  \n"
     << "                            NUM NUM  R I 102 103 104              TYP TYP FAILED\n"
     << "- -- - ---- --------------- --- ---- - - --- --- --- -------- --- --- --- ------\n";

  std::vector<ESVFareComponent*>::const_iterator fareCompIter;

  for (fareCompIter = esvOption.esvFareComponentsVec().begin();
       fareCompIter != esvOption.esvFareComponentsVec().end();
       fareCompIter++)
  {
    const ESVFareComponent* esvFareComponent = (*fareCompIter);

    if (nullptr == esvFareComponent)
    {
      dc << "ERROR: ESV Fare Component object is NULL." << std::endl;

      return (*this);
    }

    if (nullptr == esvFareComponent->paxTypeFare())
    {
      dc << "ERROR: Pax Type Fare object is NULL." << std::endl;

      return (*this);
    }

    dc << *(esvFareComponent->paxTypeFare());

    dc << "  FARE BKG CODES: ";
    printBkgCodes(esvFareComponent->paxTypeFare());
    dc << std::endl;

    dc << "    FOR SEGMENT(S): ";

    VecMap<uint32_t, PaxTypeFare::FlightBit>::const_iterator flightBitIter;
    flightBitIter = esvFareComponent->paxTypeFare()->flightBitmapESV().find(esvOption.sopId());

    if (flightBitIter == esvFareComponent->paxTypeFare()->flightBitmapESV().end())
    {
      dc << "ERROR: Flight bit object wasn't found." << std::endl;

      return (*this);
    }

    const PaxTypeFare::FlightBit& bitMap = flightBitIter->second;

    // Display travel segments ID's used for currently processed fare
    bool outputSegment = false;
    uint32_t travelSegId = 0;
    std::vector<TravelSeg*>::const_iterator segIter;

    for (segIter = sop.itin()->travelSeg().begin(); segIter != sop.itin()->travelSeg().end();
         segIter++, travelSegId++)
    {
      if ((*segIter)->origin() == esvFareComponent->paxTypeFare()->fareMarket()->origin())
      {
        outputSegment = true;
      }

      if (true == outputSegment)
      {
        dc << (travelSegId + 1) << " ";
      }

      if ((*segIter)->destination() == esvFareComponent->paxTypeFare()->fareMarket()->destination())
      {
        outputSegment = false;
      }
    }

    dc << std::endl;

    if (bitMap._segmentStatus.empty())
    {
      dc << "    SEGMENT STATUS IS EMPTY" << std::endl;
    }
    else
    {
      uint32_t travelSegId = 0;
      std::vector<PaxTypeFare::SegmentStatus>::const_iterator segStatIter;

      for (segStatIter = bitMap._segmentStatus.begin(); segStatIter != bitMap._segmentStatus.end();
           segStatIter++, travelSegId++)
      {
        const PaxTypeFare::SegmentStatus& segmentStatus = (*segStatIter);

        dc << "    " << (travelSegId + 1) << ". ";

        if (segmentStatus._bkgCodeReBook.empty())
        {
          dc << "    EMPTY BOOKING CODES FOR SEGMENT" << std::endl;
        }
        else
        {
          dc << "    BOOKING CODE: " << segmentStatus._bkgCodeReBook
             << " ,BOOKED CABIN: " << segmentStatus._reBookCabin << std::endl;
        }
      }
    }
  }

  return (*this);
}

Diag959Collector&
Diag959Collector::
operator<<(const PaxTypeFare& paxTypeFare)
{
  Diag953Collector::operator<<(paxTypeFare);

  return (*this);
}

void
Diag959Collector::printBkgCodes(const PaxTypeFare* paxTypeFare)
{
  DiagCollector& dc(*this);
  std::vector<BookingCode> bookingCodes;
  std::vector<BookingCode>* bookingCodesPtr = &bookingCodes;
  const std::vector<BookingCode>* bookingCodesBF =
      paxTypeFare->fare()->fareInfo()->getBookingCodes(*_trx);
  if ((nullptr == bookingCodesBF) || (bookingCodesBF->empty()))
  {
    const FareClassCode& fareClass = paxTypeFare->fare()->fareInfo()->fareClass();
    if (!fareClass.empty())
    {
      bookingCodesPtr->push_back(BookingCode(fareClass[0]));
    }
  }
  else
  {
    bookingCodesPtr = (std::vector<BookingCode>*)bookingCodesBF;
  }

  std::vector<BookingCode>::iterator bkgCodeIter = bookingCodesPtr->begin();
  std::vector<BookingCode>::iterator bkgCodeIterEnd = bookingCodesPtr->end();
  for (; bkgCodeIter != bkgCodeIterEnd; ++bkgCodeIter)
  {
    dc << (*bkgCodeIter) << " ";
  }
}

void
Diag959Collector::printSabreViewCommnads(const ESVSolution& esvSolution)
{
  DiagCollector& dc(*this);

  printSabreViewBooking(esvSolution.outboundOption());

  if ((dynamic_cast<ShoppingTrx*>(_trx))->legs().size() == 2)
  {
    printSabreViewBooking(esvSolution.inboundOption());
  }

  int segIndex = 1;
  std::string wpString = "WPPADT‡";

  printSabreViewFares(esvSolution.outboundOption(), wpString, segIndex);

  if ((dynamic_cast<ShoppingTrx*>(_trx))->legs().size() == 2)
  {
    printSabreViewFares(esvSolution.inboundOption(), wpString, segIndex);
  }

  wpString += "BET-ITL";

  dc << std::endl << wpString << std::endl;
}

void
Diag959Collector::printSabreViewFares(const ESVOption& esvOption,
                                      std::string& wpString,
                                      int& segIndex)
{
  std::vector<ESVFareComponent*>::const_iterator fareCompIter;

  for (fareCompIter = esvOption.esvFareComponentsVec().begin();
       fareCompIter != esvOption.esvFareComponentsVec().end();
       ++fareCompIter)
  {
    const ESVFareComponent* esvFareComponent = (*fareCompIter);

    int segSize = esvFareComponent->esvSegmentInfoVec().size();

    std::string segString = "";

    std::stringstream s1;
    s1 << segIndex;
    segString += s1.str();

    segIndex += segSize;

    if (segSize > 1)
    {
      std::stringstream s2;
      s2 << (segIndex - 1);
      segString += "-" + s2.str();
    }

    wpString += ("S" + segString + "*Q" + esvFareComponent->fareBasisCode() + "‡");
  }
}

void
Diag959Collector::printSabreViewBooking(const ESVOption& esvOption)
{
  DiagCollector& dc(*this);

  std::vector<ESVFareComponent*>::const_iterator fareCompIter;

  for (fareCompIter = esvOption.esvFareComponentsVec().begin();
       fareCompIter != esvOption.esvFareComponentsVec().end();
       ++fareCompIter)
  {
    const ESVFareComponent* esvFareComponent = (*fareCompIter);

    std::vector<ESVSegmentInfo*>::const_iterator esvSegInfoIter;

    for (esvSegInfoIter = esvFareComponent->esvSegmentInfoVec().begin();
         esvSegInfoIter != esvFareComponent->esvSegmentInfoVec().end();
         ++esvSegInfoIter)
    {
      ESVSegmentInfo* esvSegInfo = (*esvSegInfoIter);

      const AirSeg& airSegment = dynamic_cast<const AirSeg&>(*(esvSegInfo->travelSeg()));

      dc << "0" << airSegment.carrier() << airSegment.flightNumber() << esvSegInfo->bookingCode()
         << airSegment.departureDT().dateToString(DDMMM, nullptr) << airSegment.origin()->loc()
         << airSegment.destination()->loc() << "NN1" << std::endl;
    }
  }
}

void
Diag959Collector::printGroupingSummaryInfo(const ShoppingTrx& shoppingTrx)
{
  DiagCollector& dc(*this);

  std::vector<const ESVSolution*> esvSolutions;
  esvSolutions.insert(esvSolutions.end(),
                      shoppingTrx.flightMatrixESV().begin(),
                      shoppingTrx.flightMatrixESV().end());
  esvSolutions.insert(esvSolutions.end(),
                      shoppingTrx.estimateMatrixESV().begin(),
                      shoppingTrx.estimateMatrixESV().end());

  std::map<int, std::vector<const ESVSolution*>> familyGroupsMap;
  std::map<int, std::vector<const ESVSolution*>> carrierGroupsMap;
  std::map<int, std::set<int>> noOfFamiliesinCarrierGroupsMap;

  std::vector<const ESVSolution*>::const_iterator esvSolIter = esvSolutions.begin();
  std::vector<const ESVSolution*>::const_iterator esvSolIterEnd = esvSolutions.end();

  for (; esvSolIter != esvSolIterEnd; ++esvSolIter)
  {
    const ESVSolution* esvSol = (*esvSolIter);

    if (familyGroupsMap.find(esvSol->groupId()) == familyGroupsMap.end())
    {
      std::vector<const ESVSolution*> esvSolutionsVec;
      esvSolutionsVec.push_back(esvSol);

      familyGroupsMap[esvSol->groupId()] = esvSolutionsVec;
    }
    else
    {
      familyGroupsMap[esvSol->groupId()].push_back(esvSol);
    }

    if (carrierGroupsMap.find(esvSol->carrierGroupId()) == carrierGroupsMap.end())
    {
      std::vector<const ESVSolution*> esvSolutionsVec;
      esvSolutionsVec.push_back(esvSol);

      carrierGroupsMap[esvSol->carrierGroupId()] = esvSolutionsVec;

      std::set<int> familiesSet;
      familiesSet.insert(esvSol->groupId());

      noOfFamiliesinCarrierGroupsMap[esvSol->carrierGroupId()] = familiesSet;
    }
    else
    {
      carrierGroupsMap[esvSol->carrierGroupId()].push_back(esvSol);
      noOfFamiliesinCarrierGroupsMap[esvSol->carrierGroupId()].insert(esvSol->groupId());
    }
  }

  if (familyGroupsMap.empty() || carrierGroupsMap.empty())
  {
    return;
  }

  dc << std::endl << "GROUPING SUMMARY INFO:" << std::endl;
  dc << std::endl;

  dc << "  NO OF FAMILY GROUPS: " << familyGroupsMap.size() << std::endl;
  dc << std::endl;

  int allItems = 0;
  int minItems = 0;
  int maxItems = 0;

  std::map<int, std::vector<const ESVSolution*>>::iterator fdMapIter = familyGroupsMap.begin();
  std::map<int, std::vector<const ESVSolution*>>::iterator fdMapIterEnd = familyGroupsMap.end();

  for (; fdMapIter != fdMapIterEnd; ++fdMapIter)
  {
    int groupSize = fdMapIter->second.size();

    allItems += groupSize;

    if (groupSize > maxItems)
    {
      maxItems = groupSize;
    }

    if ((0 == minItems) || (groupSize < minItems))
    {
      minItems = groupSize;
    }
  }

  dc << "  AVG NO OF SOLUTIONS IN FAMILY GROUP: "
     << ((float)(allItems) / (float)(familyGroupsMap.size())) << std::endl;
  dc << "  MIN NO OF SOLUTIONS IN FAMILY GROUP: " << minItems << std::endl;
  dc << "  MAX NO OF SOLUTIONS IN FAMILY GROUP: " << maxItems << std::endl;
  dc << std::endl;

  dc << "  NO OF CARRIER GROUPS: " << carrierGroupsMap.size() << std::endl;
  dc << std::endl;

  int numLegs = shoppingTrx.legs().size();
  allItems = 0;
  minItems = 0;
  maxItems = 0;
  int allFamilies = 0;

  int noOfNonStopGroups = 0;
  int allNonStopItems = 0;
  int minNonStopItems = 0;
  int maxNonStopItems = 0;
  int allNonStopFamilies = 0;

  int noOfMixedGroups = 0;
  int allMixedItems = 0;
  int minMixedItems = 0;
  int maxMixedItems = 0;
  int allMixedFamilies = 0;

  int noOfConnectingOnlyGroups = 0;
  int allConnectingOnlyItems = 0;
  int minConnectingOnlyItems = 0;
  int maxConnectingOnlyItems = 0;
  int allConnectingOnlyFamilies = 0;

  std::map<int, std::vector<const ESVSolution*>>::iterator cgMapIter = carrierGroupsMap.begin();
  std::map<int, std::vector<const ESVSolution*>>::iterator cgMapIterEnd = carrierGroupsMap.end();

  for (; cgMapIter != cgMapIterEnd; ++cgMapIter)
  {
    int groupSize = cgMapIter->second.size();
    int noOfFamilies = noOfFamiliesinCarrierGroupsMap[cgMapIter->first].size();

    allItems += groupSize;
    allFamilies += noOfFamilies;

    if (groupSize > maxItems)
    {
      maxItems = groupSize;
    }

    if ((0 == minItems) || (groupSize < minItems))
    {
      minItems = groupSize;
    }

    std::vector<const ESVSolution*>::iterator solIter = cgMapIter->second.begin();
    std::vector<const ESVSolution*>::iterator solIterEnd = cgMapIter->second.end();

    bool foundNonstop = false;
    bool foundConnecting = false;

    for (; solIter != solIterEnd; ++solIter)
    {
      const ESVSolution* esvSol = (*solIter);

      if (esvSol->numberOfSegments() == numLegs)
      {
        foundNonstop = true;
      }

      if (esvSol->numberOfSegments() > numLegs)
      {
        foundConnecting = true;
      }
    }

    if (foundNonstop && foundConnecting)
    {
      ++noOfMixedGroups;

      allMixedItems += groupSize;
      allMixedFamilies += noOfFamilies;

      if (groupSize > maxMixedItems)
      {
        maxMixedItems = groupSize;
      }

      if ((0 == minMixedItems) || (groupSize < minMixedItems))
      {
        minMixedItems = groupSize;
      }
    }
    else if (foundNonstop)
    {
      ++noOfNonStopGroups;

      allNonStopItems += groupSize;
      allNonStopFamilies += noOfFamilies;

      if (groupSize > maxNonStopItems)
      {
        maxNonStopItems = groupSize;
      }

      if ((0 == minNonStopItems) || (groupSize < minNonStopItems))
      {
        minNonStopItems = groupSize;
      }
    }
    else if (foundConnecting)
    {
      ++noOfConnectingOnlyGroups;

      allConnectingOnlyItems += groupSize;
      allConnectingOnlyFamilies += noOfFamilies;

      if (groupSize > maxConnectingOnlyItems)
      {
        maxConnectingOnlyItems = groupSize;
      }

      if ((0 == minConnectingOnlyItems) || (groupSize < minConnectingOnlyItems))
      {
        minConnectingOnlyItems = groupSize;
      }
    }
  }

  dc << "  AVG NO OF SOLUTIONS IN CARRIER GROUP: "
     << ((float)(allItems) / (float)(carrierGroupsMap.size())) << std::endl;
  dc << "  MIN NO OF SOLUTIONS IN CARRIER GROUP: " << minItems << std::endl;
  dc << "  MAX NO OF SOLUTIONS IN CARRIER GROUP: " << maxItems << std::endl;
  dc << "  AVG NO OF FAMILIES IN CARRIER GROUP: "
     << ((float)(allFamilies) / (float)(carrierGroupsMap.size())) << std::endl;
  dc << std::endl;

  dc << "  NO OF NONSTOP CARRIER GROUPS: " << noOfNonStopGroups << std::endl;

  if (noOfNonStopGroups != 0)
  {
    dc << "    AVG NO OF SOLUTIONS IN NONSTOP CARRIER GROUP: "
       << ((float)(allNonStopItems) / (float)(noOfNonStopGroups)) << std::endl;
    dc << "    MIN NO OF SOLUTIONS IN NONSTOP CARRIER GROUP: " << minNonStopItems << std::endl;
    dc << "    MAX NO OF SOLUTIONS IN NONSTOP CARRIER GROUP: " << maxNonStopItems << std::endl;
    dc << "    AVG NO OF FAMILIES IN NONSTOP CARRIER GROUP: "
       << ((float)(allNonStopFamilies) / (float)(noOfNonStopGroups)) << std::endl;
  }

  dc << std::endl;

  dc << "  NO OF MIXED CARRIER GROUPS: " << noOfMixedGroups << std::endl;

  if (noOfMixedGroups != 0)
  {
    dc << "    AVG NO OF SOLUTIONS IN MIXED CARRIER GROUP: "
       << ((float)(allMixedItems) / (float)(noOfMixedGroups)) << std::endl;
    dc << "    MIN NO OF SOLUTIONS IN MIXED CARRIER GROUP: " << minMixedItems << std::endl;
    dc << "    MAX NO OF SOLUTIONS IN MIXED CARRIER GROUP: " << maxMixedItems << std::endl;
    dc << "    AVG NO OF FAMILIES IN MIXED CARRIER GROUP: "
       << ((float)(allMixedFamilies) / (float)(noOfMixedGroups)) << std::endl;
  }

  dc << std::endl;

  dc << "  NO OF CONNECTING ONLY CARRIER GROUPS: " << noOfConnectingOnlyGroups << std::endl;

  if (noOfConnectingOnlyGroups != 0)
  {
    dc << "    AVG NO OF SOLUTIONS IN CONNECTING ONLY GROUP: "
       << ((float)(allConnectingOnlyItems) / (float)(noOfConnectingOnlyGroups)) << std::endl;
    dc << "    MIN NO OF SOLUTIONS IN CONNECTING ONLY GROUP: " << minConnectingOnlyItems
       << std::endl;
    dc << "    MAX NO OF SOLUTIONS IN CONNECTING ONLY GROUP: " << maxConnectingOnlyItems
       << std::endl;
    dc << "    AVG NO OF FAMILIES IN CONECTING ONLY CARRIER GROUP: "
       << ((float)(allConnectingOnlyFamilies) / (float)(noOfConnectingOnlyGroups)) << std::endl;
  }

  dc << std::endl;
}
}
