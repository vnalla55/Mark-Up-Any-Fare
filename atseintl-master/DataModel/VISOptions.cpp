//----------------------------------------------------------------------------
//  File:        VISOptions.cpp
//  Created:     2009-04-06
//
//  Description: Class used to keep VIS configuration
//
//  Updates:
//
//  Copyright Sabre 2009
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

#include "DataModel/VISOptions.h"

#include <vector>

using namespace tse;

ValueBasedItinSelection::ValueBasedItinSelection()
  : _enableVIS(false), _noOfOutboundsRT(35), _noOfInboundsRT(20), _noOfOutboundsOW(75)
{
}

ValueBasedItinSelection::~ValueBasedItinSelection() {}

VISOutboundSelectionRT::VISOutboundSelectionRT()
  : _carrierPriority(1),
    _timeOfDayPriority(2),
    _elapsedTimePriority(3),
    _utilityValuePriority(4),
    _nonStopPriority(5),
    _noOfCarriers(5),
    _noOfOptionsPerCarrier(1),
    _noOfOptionsPerTimeBin(1),
    _noOfElapsedTimeOptions(1),
    _noOfUtilityValueOptions(5),
    _nonStopFareMultiplier(2.0),
    _noOfNonStopOptions(200)
{
  VISTimeBin tb1(0, 600);
  VISTimeBin tb2(601, 1200);
  VISTimeBin tb3(1201, 1800);
  VISTimeBin tb4(1801, 2359);

  _timeOfDayBins.push_back(tb1);
  _timeOfDayBins.push_back(tb2);
  _timeOfDayBins.push_back(tb3);
  _timeOfDayBins.push_back(tb4);
}

VISOutboundSelectionRT::~VISOutboundSelectionRT() {}

VISInboundSelectionRT::VISInboundSelectionRT()
  : _lowestFarePriority(1),
    _timeOfDayPriority(2),
    _elapsedTimePriority(3),
    _utilityValuePriority(4),
    _nonStopPriority(6),
    _simpleInterlinePriority(5),
    _noOfLFSOptions(2),
    _noOfOptionsPerTimeBin(1),
    _noOfElapsedTimeOptions(1),
    _noOfUtilityValueOptions(5),
    _nonStopFareMultiplier(2.0),
    _noOfNonStopOptions(200),
    _noOfSimpleInterlineOptions(2)
{
  VISTimeBin tb1(0, 600);
  VISTimeBin tb2(601, 1200);
  VISTimeBin tb3(1201, 1800);
  VISTimeBin tb4(1801, 2359);

  _timeOfDayBins.push_back(tb1);
  _timeOfDayBins.push_back(tb2);
  _timeOfDayBins.push_back(tb3);
  _timeOfDayBins.push_back(tb4);
}

VISInboundSelectionRT::~VISInboundSelectionRT() {}

VISOutboundSelectionOW::VISOutboundSelectionOW()
  : _carrierPriority(1),
    _timeOfDayPriority(2),
    _elapsedTimePriority(3),
    _utilityValuePriority(4),
    _nonStopPriority(5),
    _noOfCarriers(5),
    _noOfOptionsPerCarrier(1),
    _noOfOptionsPerTimeBin(1),
    _noOfElapsedTimeOptions(1),
    _noOfUtilityValueOptions(5),
    _nonStopFareMultiplier(2.0),
    _noOfNonStopOptions(200)
{
  VISTimeBin tb1(0, 600);
  VISTimeBin tb2(601, 1200);
  VISTimeBin tb3(1201, 1800);
  VISTimeBin tb4(1801, 2359);

  _timeOfDayBins.push_back(tb1);
  _timeOfDayBins.push_back(tb2);
  _timeOfDayBins.push_back(tb3);
  _timeOfDayBins.push_back(tb4);
}

VISOutboundSelectionOW::~VISOutboundSelectionOW() {}

VISLowFareSearch::VISLowFareSearch()
  : _noOfLFSItineraries(50),
    _noOfAdditionalOutboundsRT(1),
    _noOfAdditionalInboundsRT(1),
    _noOfAdditionalOutboundsOW(1)
{
}

VISLowFareSearch::~VISLowFareSearch() {}
