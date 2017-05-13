//----------------------------------------------------------------------------
//  File:        Diag958Collector.C
//  Created:     2009-03-25
//
//  Description: Diagnostic 958 formatter
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

#include "Diagnostic/Diag958Collector.h"

#include "Common/Config/ConfigurableValue.h"
#include "Pricing/EstimatedSeatValue.h"
#include "Pricing/ESVPQItem.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

namespace tse
{
namespace
{
ConfigurableValue<bool>
generateIVOptionsCfg("VIS_OPTIONS", "GENERATE_IV_OPTIONS", true);
}

Diag958Collector&
Diag958Collector::
operator<<(const ShoppingTrx& shoppingTrx)
{
  if (false == _active)
  {
    return (*this);
  }

  initDiag(shoppingTrx);

  if (false == _eisLogic)
  {
    printDiagHeader(shoppingTrx);
  }

  return (*this);
}

void
Diag958Collector::initDiag(const ShoppingTrx& shoppingTrx)
{
  // Check if VIS logic is enabled
  if (nullptr == ((ShoppingTrx*)_trx)->visOptions())
  {
    _eisLogic = false;
  }
  else
  {
    _eisLogic = ((ShoppingTrx*)_trx)->visOptions()->valueBasedItinSelection().enableVIS();
    std::stringstream iss(this->rootDiag()->diagParamMapItem("DL"));
    if ((iss >> _displayLevel).fail())
      _displayLevel = -1;
  }
}

Diag958Collector&
Diag958Collector::printDiagHeader(const ShoppingTrx& shoppingTrx)
{
  DiagCollector& dc(*this);

  dc << std::endl;
  dc << "**********************************************************\n"
     << "958 : VIS DIAGNOSTIC\n"
     << "**********************************************************\n";
  dc << std::endl;

  _trx = (PricingTrx*)&shoppingTrx;

  if (true == _eisLogic)
  {
    dc << "VIS logic is ENABLED" << std::endl;
    dc << std::endl;

    dc << "DIAGNOSTIC ARGUMENTS:" << std::endl;
    dc << std::endl;

    dc << "1. INPUT_PARAMETERS = Y/N (Displays VIS input parameters from request)" << std::endl;
    dc << "2. BETA_PARAMETER = Y/N (Displays beta parameter values for current request)"
       << std::endl;
    dc << "3. DOMINATED_FLIGHTS = Y/N (Displays list of outbound and inbound flights with "
          "dominated flights flag)" << std::endl;
    dc << "4. LFS_SOLUTIONS = Y/N (Displays requested number of LFS solutions)" << std::endl;
    dc << "5. OUTBOUND_SELECTION = Y/N (Displays requested number of selected outbound flights)"
       << std::endl;
    dc << "6. INBOUND_SELECTION = Y/N (Displays requested number of selected inbound flights for "
          "each outbound)" << std::endl;
    dc << "7. ALL = Y/N (Displays all information)" << std::endl;
  }
  else
  {
    dc << "VIS logic is DISABLED" << std::endl;
  }

  dc << std::endl;

  flushMsg();

  return (*this);
}

Diag958Collector&
Diag958Collector::printInitialInfo(const Loc& origin,
                                   const Loc& destination,
                                   const short utcoffset,
                                   const DateTime& departureDate,
                                   const DateTime& bookingDate,
                                   const uint16_t mileage,
                                   const uint16_t roundMileage,
                                   const char AP)
{
  DiagCollector& dc(*this);

  if (("Y" != dc.rootDiag()->diagParamMapItem("BETA_PARAMETER")) &&
      ("Y" != dc.rootDiag()->diagParamMapItem("ALL")))
  {
    return (*this);
  }

  dc << std::endl;
  dc << "ORIGIN: " << origin.loc() << std::endl;
  dc << "DESTINATION: " << destination.loc() << std::endl;
  dc << "TIME ZONE DIFFERENCE: " << utcoffset << " HOURS" << std::endl;
  dc << "DEPARTURE DATE: " << departureDate << std::endl;
  dc << "BOOKING DATE: " << bookingDate << std::endl;
  dc << "MILEAGE: " << mileage << std::endl;
  dc << "MILEAGE AFTER ROUNDING: " << roundMileage << std::endl;
  dc << "AP: " << AP << std::endl;
  dc << std::endl;

  flushMsg();

  return (*this);
}

Diag958Collector&
Diag958Collector::printParameterBeta(const int& timeDiff,
                                     const int& mileage,
                                     const char& direction,
                                     const char& APSatInd,
                                     const std::vector<double>* paramBeta)
{
  DiagCollector& dc(*this);

  if (("Y" != dc.rootDiag()->diagParamMapItem("BETA_PARAMETER")) &&
      ("Y" != dc.rootDiag()->diagParamMapItem("ALL")))
  {
    return (*this);
  }

  dc << std::endl;
  dc << "PARAMETER BETA KEY:" << std::endl << std::endl;

  dc << "TIME DIFFERENCE: " << timeDiff << std::endl;
  dc << "MILEAGE: " << mileage << std::endl;
  dc << "DIRECTION: " << direction << std::endl;
  dc << "AP: " << APSatInd << std::endl;

  dc << std::endl;
  dc << "PARAMETER BETA:" << std::endl << std::endl;

  if (paramBeta)
  {
    int id = 0;
    std::vector<double>::const_iterator paramBetaIter;

    for (paramBetaIter = paramBeta->begin(); paramBetaIter != paramBeta->end();
         ++paramBetaIter, ++id)
    {
      std::stringstream ssId;
      ssId << id;

      dc << "D" << ssId.str() << ": " << (*paramBetaIter) << std::endl;
    }
  }
  else
  {
    dc << "Beta is null\n";
  }

  dc << std::endl;

  flushMsg();

  return (*this);
}

Diag958Collector&
Diag958Collector::printESVPQItems(const std::vector<ESVPQItem*>& selectedFlights,
                                  const std::string descriptionStr)
{
  DiagCollector& dc(*this);

  if (("Y" != dc.rootDiag()->diagParamMapItem("LFS_SOLUTIONS")) &&
      ("Y" != dc.rootDiag()->diagParamMapItem("ALL")))
  {
    return (*this);
  }

  dc << std::endl;
  dc << descriptionStr << std::endl;
  dc << std::endl;

  int id = 1;
  std::vector<ESVPQItem*>::const_iterator flightsIter;

  for (flightsIter = selectedFlights.begin(); flightsIter != selectedFlights.end();
       ++flightsIter, ++id)
  {
    ESVPQItem* esvPQItem = (*flightsIter);

    std::stringstream ssId;
    ssId << id;

    dc << "SOLUTION NO: " << ssId.str() << std::endl;

    dc << "OUT ID: " << esvPQItem->outSopWrapper()->sop()->originalSopId();

    if (esvPQItem->inSopWrapper() != nullptr)
    {
      dc << ", IN ID: " << esvPQItem->inSopWrapper()->sop()->originalSopId();
    }

    dc << ", TOTAL AMOUNT: " << Money(esvPQItem->totalAmt(), "NUC");

    dc << ", ITIN UTILITY VALUE: " << esvPQItem->itinUtility();

    dc << ", OUT UTILITY VALUE: " << esvPQItem->getUtility(0);

    if (esvPQItem->inSopWrapper() != nullptr)
    {
      dc << ", IN UTILITY VALUE: " << esvPQItem->getUtility(1);
    }

    dc << std::endl;
  }

  flushMsg();

  return (*this);
}

Diag958Collector&
Diag958Collector::printVISOptions(VISOptions& visOptions)
{
  DiagCollector& dc(*this);

  if (("Y" != dc.rootDiag()->diagParamMapItem("INPUT_PARAMETERS")) &&
      ("Y" != dc.rootDiag()->diagParamMapItem("ALL")))
  {
    return (*this);
  }

  dc << std::endl;
  dc << "**********************************************************" << std::endl;
  dc << "VIS PARAMETERS: " << std::endl;
  dc << "**********************************************************" << std::endl;
  dc << std::endl;

  dc << std::endl;
  dc << "VALUE BASED ITINERARY SELECTION:" << std::endl;
  dc << std::endl;

  dc << "  VIS ENABLED (PVI): "
     << (visOptions.valueBasedItinSelection().enableVIS() ? "TRUE" : "FALSE") << std::endl;
  dc << "  TARGET NO. OF OUTBOUNDS RT (Q80): "
     << visOptions.valueBasedItinSelection().noOfOutboundsRT() << std::endl;
  dc << "  TARGET NO. OF INBOUNDS RT (Q81): "
     << visOptions.valueBasedItinSelection().noOfInboundsRT() << std::endl;
  dc << "  TARGET NO. OF OUTBOUNDS OW (QC3): "
     << visOptions.valueBasedItinSelection().noOfOutboundsOW() << std::endl;

  const char generateIVOption = generateIVOptionsCfg.getValue() ? 'Y' : 'N';

  dc << "  GENERATE INCREMENTAL VALUE OPTIONS: " << generateIVOption << std::endl;

  dc << std::endl;
  dc << "VIS OUTBOUND SELECTION (RT):" << std::endl;
  dc << std::endl;

  dc << "  PRIORITY OF SELECTION BY CARRIER (Q82): "
     << visOptions.visOutboundSelectionRT().carrierPriority() << std::endl;
  dc << "  PRIORITY OF SELECTION BY TIME-OF-DAY (Q83): "
     << visOptions.visOutboundSelectionRT().timeOfDayPriority() << std::endl;
  dc << "  PRIORITY OF SELECTION BY ELAPSED TIME (Q84): "
     << visOptions.visOutboundSelectionRT().elapsedTimePriority() << std::endl;
  dc << "  PRIORITY OF SELECTION BY UTILITY VALUE (Q85): "
     << visOptions.visOutboundSelectionRT().utilityValuePriority() << std::endl;
  dc << "  PRIORITY OF NON-STOP SELECTION (Q86): "
     << visOptions.visOutboundSelectionRT().nonStopPriority() << std::endl;
  dc << "  SELECTION BY CARRIER - NO. OF CARRIERS (Q87): "
     << visOptions.visOutboundSelectionRT().noOfCarriers() << std::endl;
  dc << "  SELECTION BY CARRIER - NO. OF OPTIONS PER CARRIER (Q88): "
     << visOptions.visOutboundSelectionRT().noOfOptionsPerCarrier() << std::endl;
  dc << "  SELECTION BY TIME-OF-DAY - BINS (SG1): ";

  for (const auto& elem : visOptions.visOutboundSelectionRT().timeOfDayBins())
  {
    dc << "(" << elem.beginTime() << "-" << elem.endTime() << ")"
       << ", ";
  }
  dc << std::endl;

  dc << "  SELECTION BY TIME-OF-DAY - NO. OF OPTIONS PER BIN (Q89): "
     << visOptions.visOutboundSelectionRT().noOfOptionsPerTimeBin() << std::endl;
  dc << "  SELECTION BY ELAPSED TIME - NO. OF OPTIONS (Q90): "
     << visOptions.visOutboundSelectionRT().noOfElapsedTimeOptions() << std::endl;
  dc << "  SELECTION BY UTILITY VALUE - NO. OF OPTIONS (Q91): "
     << visOptions.visOutboundSelectionRT().noOfUtilityValueOptions() << std::endl;
  dc << "  NON-STOP SELECTION - FARE MULTIPLIER (Q92): "
     << visOptions.visOutboundSelectionRT().nonStopFareMultiplier() << std::endl;
  dc << "  NON-STOP SELECTION - NO. OF OPTIONS (Q93): "
     << visOptions.visOutboundSelectionRT().noOfNonStopOptions() << std::endl;

  dc << std::endl;
  dc << "VIS INBOUND SELECTION (RT):" << std::endl;
  dc << std::endl;

  dc << "  PRIORITY OF SELECTION BY LOWEST FARE (Q94): "
     << visOptions.visInboundSelectionRT().lowestFarePriority() << std::endl;
  dc << "  PRIORITY OF SELECTION BY TIME-OF-DAY (Q95): "
     << visOptions.visInboundSelectionRT().timeOfDayPriority() << std::endl;
  dc << "  PRIORITY OF SELECTION BY ELAPSED TIME (Q96): "
     << visOptions.visInboundSelectionRT().elapsedTimePriority() << std::endl;
  dc << "  PRIORITY OF SELECTION BY UTILITY VALUE (Q97): "
     << visOptions.visInboundSelectionRT().utilityValuePriority() << std::endl;
  dc << "  PRIORITY OF NON-STOP SELECTION (Q98): "
     << visOptions.visInboundSelectionRT().nonStopPriority() << std::endl;
  dc << "  PRIORITY OF SIMPLE INTERLINE SELECTION (Q99): "
     << visOptions.visInboundSelectionRT().simpleInterlinePriority() << std::endl;
  dc << "  SELECTION BY LOWEST FARE - NO. OF OPTIONS (QA0): "
     << visOptions.visInboundSelectionRT().noOfLFSOptions() << std::endl;
  dc << "  SELECTION BY TIME-OF-DAY - BINS (SG2): ";

  for (const auto& elem : visOptions.visInboundSelectionRT().timeOfDayBins())
  {
    dc << "(" << elem.beginTime() << "-" << elem.endTime() << ")"
       << ", ";
  }
  dc << std::endl;

  dc << "  SELECTION BY TIME-OF-DAY - NO. OF OPTIONS PER BIN (QA1): "
     << visOptions.visInboundSelectionRT().noOfOptionsPerTimeBin() << std::endl;
  dc << "  SELECTION BY ELAPSED TIME - NO. OF OPTIONS (QA2): "
     << visOptions.visInboundSelectionRT().noOfElapsedTimeOptions() << std::endl;
  dc << "  SELECTION BY UTILITY VALUE - NO. OF OPTIONS (QA3): "
     << visOptions.visInboundSelectionRT().noOfUtilityValueOptions() << std::endl;
  dc << "  NON-STOP SELECTION - FARE MULTIPLIER (QA4): "
     << visOptions.visInboundSelectionRT().nonStopFareMultiplier() << std::endl;
  dc << "  NON-STOP SELECTION - NO. OF OPTIONS (QA5): "
     << visOptions.visInboundSelectionRT().noOfNonStopOptions() << std::endl;
  dc << "  SIMPLE INTERLINE SELECTION - NO. OF OPTIONS (QA6): "
     << visOptions.visInboundSelectionRT().noOfSimpleInterlineOptions() << std::endl;

  dc << std::endl;
  dc << "VIS OUTBOUND SELECTION (OW):" << std::endl;
  dc << std::endl;

  dc << "  PRIORITY OF SELECTION BY CARRIER (QA7): "
     << visOptions.visOutboundSelectionOW().carrierPriority() << std::endl;
  dc << "  PRIORITY OF SELECTION BY TIME-OF-DAY (QA8): "
     << visOptions.visOutboundSelectionOW().timeOfDayPriority() << std::endl;
  dc << "  PRIORITY OF SELECTION BY ELAPSED TIME (QA9): "
     << visOptions.visOutboundSelectionOW().elapsedTimePriority() << std::endl;
  dc << "  PRIORITY OF SELECTION BY UTILITY VALUE (QB0): "
     << visOptions.visOutboundSelectionOW().utilityValuePriority() << std::endl;
  dc << "  PRIORITY OF NON-STOP SELECTION (QB1): "
     << visOptions.visOutboundSelectionOW().nonStopPriority() << std::endl;
  dc << "  SELECTION BY CARRIER - NO. OF CARRIERS (QB2): "
     << visOptions.visOutboundSelectionOW().noOfCarriers() << std::endl;
  dc << "  SELECTION BY CARRIER - NO. OF OPTIONS PER CARRIER (QB3): "
     << visOptions.visOutboundSelectionOW().noOfOptionsPerCarrier() << std::endl;
  dc << "  SELECTION BY TIME-OF-DAY - BINS (SG3): ";

  for (const auto& elem : visOptions.visOutboundSelectionOW().timeOfDayBins())
  {
    dc << "(" << elem.beginTime() << "-" << elem.endTime() << ")"
       << ", ";
  }
  dc << std::endl;

  dc << "  SELECTION BY TIME-OF-DAY - NO. OF OPTIONS PER BIN (QB4): "
     << visOptions.visOutboundSelectionOW().noOfOptionsPerTimeBin() << std::endl;
  dc << "  SELECTION BY ELAPSED TIME - NO. OF OPTIONS (QB5): "
     << visOptions.visOutboundSelectionOW().noOfElapsedTimeOptions() << std::endl;
  dc << "  SELECTION BY UTILITY VALUE - NO. OF OPTIONS (QB6): "
     << visOptions.visOutboundSelectionOW().noOfUtilityValueOptions() << std::endl;
  dc << "  NON-STOP SELECTION - FARE MULTIPLIER (QB7): "
     << visOptions.visOutboundSelectionOW().nonStopFareMultiplier() << std::endl;
  dc << "  NON-STOP SELECTION - NO. OF OPTIONS (QB8): "
     << visOptions.visOutboundSelectionOW().noOfNonStopOptions() << std::endl;

  dc << std::endl;
  dc << "VIS LOW FARE SEARCH:" << std::endl;
  dc << std::endl;

  dc << "  NO. OF LFS ITINERARIES (QB9): " << visOptions.visLowFareSearch().noOfLFSItineraries()
     << std::endl;
  dc << "  NO. OF ADDITIONAL OUTBOUNDS RT (QC0): "
     << visOptions.visLowFareSearch().noOfAdditionalOutboundsRT() << std::endl;
  dc << "  NO. OF ADDITIONAL INBOUNDS RT (QC1): "
     << visOptions.visLowFareSearch().noOfAdditionalInboundsRT() << std::endl;
  dc << "  NO. OF ADDITIONAL OUTBOUNDS OW (QC2): "
     << visOptions.visLowFareSearch().noOfAdditionalOutboundsOW() << std::endl;

  dc << std::endl;
  dc << "**********************************************************" << std::endl;
  dc << std::endl;

  flushMsg();

  return (*this);
}

Diag958Collector&
Diag958Collector::printFlightsList(const ShoppingTrx& shoppingTrx)
{
  DiagCollector& dc(*this);

  if (("Y" != dc.rootDiag()->diagParamMapItem("DOMINATED_FLIGHTS")) &&
      ("Y" != dc.rootDiag()->diagParamMapItem("ALL")))
  {
    return (*this);
  }

  dc << std::endl;
  dc << "**********************************************************" << std::endl;
  dc << "FLIGHTS LIST: " << std::endl;
  dc << "**********************************************************" << std::endl;
  dc << std::endl;

  int allFlightsCount = 0;
  int dominatedFlightsCount = 0;

  // Go thorough all legs
  for (uint32_t legId = 0; legId != shoppingTrx.legs().size(); legId++)
  {
    const ShoppingTrx::Leg& leg = shoppingTrx.legs()[legId];

    // Go thorough all scheduling options
    for (uint32_t sopId = 0; sopId != leg.sop().size(); sopId++)
    {
      const ShoppingTrx::SchedulingOption& sop = leg.sop()[sopId];

      if (false == ((ShoppingTrx::SchedulingOption&)sop).getDummy())
      {
        const Itin* itin = sop.itin();

        dc << std::endl << "LEG: " << (legId + 1)
           << ", SCHEDULING OPTION ID: " << sop.originalSopId() << std::endl;

        // Display travel segments information
        dc << "TRAVEL SEGMENTS: " << std::endl;

        // Go thorough all travel segments
        std::vector<TravelSeg*>::const_iterator segIter;

        for (segIter = sop.itin()->travelSeg().begin(); segIter != sop.itin()->travelSeg().end();
             segIter++)
        {
          const TravelSeg* travelSeg = (*segIter);

          const AirSeg& airSegment = dynamic_cast<const AirSeg&>(*travelSeg);

          dc << "  " << travelSeg->origin()->loc() << "-" << travelSeg->destination()->loc() << " "
             << airSegment.carrier() << " " << airSegment.flightNumber() << " ("
             << airSegment.operatingCarrierCode() << ")" << std::endl;

          dc << "    DEPARTURE: " << airSegment.departureDT().dateToString(YYYYMmmDD, "-") << " "
             << airSegment.departureDT().timeToString(HHMM, ":") << std::endl;

          dc << "    ARRIVAL:   " << airSegment.arrivalDT().dateToString(YYYYMmmDD, "-") << " "
             << airSegment.arrivalDT().timeToString(HHMM, ":") << std::endl;
        }

        dc << "ELAPSED TIME: " << itin->getFlightTimeMinutes();
        dc << std::endl;

        std::vector<MoneyAmount> esvValues;
        ShoppingUtil::getCheapestESVValues(shoppingTrx, sop.itin(), esvValues);

        dc << "CHEAPEST ESV VALUES: " << std::endl;

        if (-1.0 == esvValues[0])
        {
          dc << "  OW ESV: " << std::setw(8) << "NA" << std::endl;
        }
        else
        {
          dc << "  OW ESV: " << std::setw(8) << Money(esvValues[0], "NUC") << std::endl;
        }

        if (-1.0 == esvValues[1])
        {
          dc << "  RT ESV: " << std::setw(8) << "NA" << std::endl;
        }
        else
        {
          dc << "  RT ESV: " << std::setw(8) << Money(esvValues[1], "NUC") << std::endl;
        }

        if (-1.0 == esvValues[2])
        {
          dc << "  CT ESV: " << std::setw(8) << "NA" << std::endl;
        }
        else
        {
          dc << "  CT ESV: " << std::setw(8) << Money(esvValues[2], "NUC") << std::endl;
        }

        if (-1.0 == esvValues[3])
        {
          dc << "  OJ ESV: " << std::setw(8) << "NA" << std::endl;
        }
        else
        {
          dc << "  OJ ESV: " << std::setw(8) << Money(esvValues[3], "NUC") << std::endl;
        }

        ++allFlightsCount;

        if (true == itin->dominatedFlight())
        {
          ++dominatedFlightsCount;
          dc << "DOMINATED FLIGHT: TRUE";
        }
        else
        {
          dc << "DOMINATED FLIGHT: FALSE";
        }

        dc << std::endl;
      }
    }
  }

  dc << std::endl;
  dc << "ALL FLIGHTS COUNT: " << allFlightsCount << std::endl;
  dc << "DOMINATED FLIGHTS COUNT: " << dominatedFlightsCount << std::endl;

  dc << std::endl;
  dc << "**********************************************************" << std::endl;
  dc << std::endl;

  flushMsg();

  return (*this);
}

void
Diag958Collector::logInfo(const std::vector<ESVPQItem*>& vec)
{
  if (!_active || (_displayLevel == -1))
    return;

  std::vector<ESVPQItem*>::const_iterator it = vec.begin();
  std::vector<ESVPQItem*>::const_iterator itEnd = vec.end();
  int i = 0;
  while (it != itEnd)
  {
    logInfo(i, *it);
    ++it;
    ++i;
  }

  flushMsg();
}

void
Diag958Collector::logInfo(const int i, const ESVPQItem* const pqItem)
{
  if (!_active || (_displayLevel == -1))
    return;
  DiagCollector& dc(*this);

  const DateFormat dFmt = DDMMM;
  const TimeFormat tFmt = HHMM;
  const bool rtTrip = (pqItem->inSopWrapper() != nullptr);
  dc << i << ".";
  dc << "(";
  dc << pqItem->outSopWrapper()->sop()->sopId();
  if (rtTrip)
    dc << "," << pqItem->inSopWrapper()->sop()->sopId();
  dc << ")";

  dc << ",PR:" << pqItem->priority() << ",SS:"
     << pqItem->selectionSource()
     //<< ",PT:" << pqItem->pruneType()
     << ",SC:" << pqItem->screenID() << ",AMT:" << pqItem->totalAmt() << ",NE:" << pqItem->nestID()
     << ",UT:" << pqItem->getUtility() << ",IV:"
     << pqItem->IV()
     //<< ",DV:" << pqItem->DV()
     << ",SO:" << pqItem->selectionOrder() << "," << (pqItem->isOnline() ? "ONLINE" : "INTRLN")
     << ",ET0:" << pqItem->getFlightTimeMinutes(0) << ",ET1:" << pqItem->getFlightTimeMinutes(1)
     << "," << pqItem->getNumStops(outLegIdx);
  if (rtTrip)
    dc << "," << pqItem->getNumStops(inLegIdx);
  dc << "," << pqItem->getFirstCarrier(outLegIdx) << " " << pqItem->getLastCarrier(outLegIdx)
     << ",dep:" << pqItem->getDepartTime(outLegIdx).dateToString(dFmt, "") << " "
     << pqItem->getDepartTime(outLegIdx).timeToString(tFmt, ":")
     << ",arr:" << pqItem->getArrivalTime(outLegIdx).dateToString(dFmt, "") << " "
     << pqItem->getArrivalTime(outLegIdx).timeToString(tFmt, ":");
  if (rtTrip)
    dc << "," << pqItem->getFirstCarrier(inLegIdx) << " " << pqItem->getLastCarrier(inLegIdx)
       << ",dep:" << pqItem->getDepartTime(inLegIdx).dateToString(dFmt, "") << " "
       << pqItem->getDepartTime(inLegIdx).timeToString(tFmt, ":")
       << ",arr:" << pqItem->getArrivalTime(inLegIdx).dateToString(dFmt, "") << " "
       << pqItem->getArrivalTime(inLegIdx).timeToString(tFmt, ":");
  dc << "\n";
}

void
Diag958Collector::logInfo(const std::string& s)
{
  if (!_active || (_displayLevel == -1))
    return;

  DiagCollector& dc(*this);
  dc << s;
  flushMsg();
}

Diag958Collector&
Diag958Collector::printSelectedFlights(const std::vector<ESVPQItem*>& selectedFlights,
                                       const bool& bOutbound)
{
  DiagCollector& dc(*this);

  if (bOutbound)
  {
    if (("Y" != dc.rootDiag()->diagParamMapItem("OUTBOUND_SELECTION")) &&
        ("Y" != dc.rootDiag()->diagParamMapItem("ALL")))
    {
      return (*this);
    }
  }
  else
  {
    if (("Y" != dc.rootDiag()->diagParamMapItem("INBOUND_SELECTION")) &&
        ("Y" != dc.rootDiag()->diagParamMapItem("ALL")))
    {
      return (*this);
    }
  }

  if (bOutbound)
  {
    dc << std::endl;
    dc << "**********************************************************" << std::endl;
    dc << "OUTBOUND FLIGHTS: " << std::endl;
    dc << "**********************************************************" << std::endl;
    dc << std::endl;
  }
  else
  {
    uint32_t outSopId = 0;

    if (!selectedFlights.empty())
    {
      outSopId = selectedFlights[0]->outSopWrapper()->sop()->originalSopId();
    }

    dc << std::endl;
    dc << "**********************************************************" << std::endl;
    dc << "INBOUND FLIGHTS FOR OUTBOUND FLIGHT ID: " << outSopId << std::endl;
    dc << "**********************************************************" << std::endl;
    dc << std::endl;
  }

  if (bOutbound)
  {
    dc << "CARRIER BUCKET: " << std::endl;
    dc << std::endl;
    printBucket(selectedFlights, bOutbound, "B-CR");
  }

  if (!bOutbound)
  {
    dc << "LFS BUCKET: " << std::endl;
    dc << std::endl;
    printBucket(selectedFlights, bOutbound, "B-LFS");
  }

  dc << std::endl;
  dc << "TIME OF DAY BUCKET: " << std::endl;
  dc << std::endl;
  printBucket(selectedFlights, bOutbound, "B-TB");

  dc << std::endl;
  dc << "ELAPSED TIME BUCKET: " << std::endl;
  dc << std::endl;
  printBucket(selectedFlights, bOutbound, "B-ET");

  dc << std::endl;
  dc << "UTILITY VALUE BUCKET: " << std::endl;
  dc << std::endl;
  printBucket(selectedFlights, bOutbound, "B-UV");

  dc << std::endl;
  dc << "NON-STOP BUCKET: " << std::endl;
  dc << std::endl;
  printBucket(selectedFlights, bOutbound, "B-NS");

  if (!bOutbound)
  {
    dc << std::endl;
    dc << "SIMPLE INTERLINE BUCKET: " << std::endl;
    dc << std::endl;
    printBucket(selectedFlights, bOutbound, "B-SI");
  }

  dc << std::endl;
  dc << "INCREMENTAL VALUE OPTIONS: " << std::endl;
  dc << std::endl;
  printBucket(selectedFlights, bOutbound, "O-IV");

  dc << std::endl;
  dc << "LFS OPTIONS: " << std::endl;
  dc << std::endl;
  printBucket(selectedFlights, bOutbound, "O-LFS");

  dc << std::endl;

  flushMsg();

  return (*this);
}

Diag958Collector&
Diag958Collector::printBucket(const std::vector<ESVPQItem*>& selectedFlights,
                              const bool& bOutbound,
                              const std::string& bucketName)
{
  DiagCollector& dc(*this);

  const DateFormat dFmt = DDMMM;
  const TimeFormat tFmt = HHMM;

  int legIndex = bOutbound ? 0 : 1;

  std::vector<ESVPQItem*>::const_iterator flightsIter;
  int id = 1;

  for (flightsIter = selectedFlights.begin(); flightsIter != selectedFlights.end(); ++flightsIter)
  {
    ESVPQItem* esvPQItem = (*flightsIter);

    if (std::string::npos == esvPQItem->selectionSource().find(bucketName, 0))
    {
      continue;
    }

    std::stringstream ssId;
    ssId << id;

    dc << "NO: " << ssId.str() << ", PRIORITY: " << esvPQItem->priority() << ", "
       << esvPQItem->selectionSource() << std::endl;
    id++;

    // Display PQ info
    dc << "  OUT ID: " << esvPQItem->outSopWrapper()->sop()->originalSopId();

    if (esvPQItem->inSopWrapper() != nullptr)
    {
      dc << ", IN ID: " << esvPQItem->inSopWrapper()->sop()->originalSopId();
    }

    dc << ", TOTAL AMOUNT: " << Money(esvPQItem->totalAmt(), "NUC");
    dc << std::endl;

    dc << "  UTILITY VALUE: " << esvPQItem->getUtility(legIndex);
    dc << ", INCREMENTAL VALUE: " << esvPQItem->IV();
    dc << std::endl;

    const ShoppingTrx::SchedulingOption* sop =
        (0 == legIndex) ? esvPQItem->outSopWrapper()->sop() : esvPQItem->inSopWrapper()->sop();
    const Itin* itin = sop->itin();

    dc << "  NEST ID: " << esvPQItem->nestID(legIndex)
       << ", ELAPSED TIME: " << itin->getFlightTimeMinutes() << std::endl;

    const AirSeg* as = dynamic_cast<const AirSeg*>(itin->firstTravelSeg());

    dc << "  FIRST CARRIER: " << as->carrier()
       << ", IS ONLINE: " << ((itin->onlineCarrier().empty()) ? "N" : "Y")
       << ", NUMBER OF STOPS: " << (itin->travelSeg().size() - 1) << std::endl;

    dc << "  DEP: " << itin->firstTravelSeg()->departureDT().dateToString(dFmt, "") << " "
       << itin->firstTravelSeg()->departureDT().timeToString(tFmt, ":")
       << ", ARR:" << itin->lastTravelSeg()->arrivalDT().dateToString(dFmt, "") << " "
       << itin->lastTravelSeg()->arrivalDT().timeToString(tFmt, ":") << std::endl;

    dc << std::endl;
  }

  flushMsg();

  return (*this);
}
}
