//----------------------------------------------------------------------------
//  File:        VISOptions.h
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

#pragma once

#include "Common/TseConsts.h"
#include "Common/TseStringTypes.h"

namespace tse
{

class VISTimeBin
{
public:
  VISTimeBin() : _beginTime(0), _endTime(0) {}

  VISTimeBin(int16_t begin, int16_t end) : _beginTime(begin), _endTime(end) {}

  short& beginTime() { return _beginTime; }
  const short& beginTime() const { return _beginTime; }

  short& endTime() { return _endTime; }
  const short& endTime() const { return _endTime; }

private:
  short _beginTime;
  short _endTime;
};

class ValueBasedItinSelection
{
public:
  ValueBasedItinSelection();

  ~ValueBasedItinSelection();

  bool& enableVIS() { return _enableVIS; }
  const bool& enableVIS() const { return _enableVIS; }

  short& noOfOutboundsRT() { return _noOfOutboundsRT; }
  const short& noOfOutboundsRT() const { return _noOfOutboundsRT; }

  short& noOfInboundsRT() { return _noOfInboundsRT; }
  const short& noOfInboundsRT() const { return _noOfInboundsRT; }

  short& noOfOutboundsOW() { return _noOfOutboundsOW; }
  const short& noOfOutboundsOW() const { return _noOfOutboundsOW; }

private:
  // PVI - Enable VIS
  bool _enableVIS;
  // Q80 - Target number of outbounds (RT)
  short _noOfOutboundsRT;
  // Q81 - Target number of inbounds per outbound (RT)
  short _noOfInboundsRT;
  // QC3 - Target number of outbounds (OW)
  short _noOfOutboundsOW;
};

class VISOutboundSelectionRT
{
public:
  VISOutboundSelectionRT();

  ~VISOutboundSelectionRT();

  short& carrierPriority() { return _carrierPriority; }
  const short& carrierPriority() const { return _carrierPriority; }

  short& timeOfDayPriority() { return _timeOfDayPriority; }
  const short& timeOfDayPriority() const { return _timeOfDayPriority; }

  short& elapsedTimePriority() { return _elapsedTimePriority; }
  const short& elapsedTimePriority() const { return _elapsedTimePriority; }

  short& utilityValuePriority() { return _utilityValuePriority; }
  const short& utilityValuePriority() const { return _utilityValuePriority; }

  short& nonStopPriority() { return _nonStopPriority; }
  const short& nonStopPriority() const { return _nonStopPriority; }

  short& noOfCarriers() { return _noOfCarriers; }
  const short& noOfCarriers() const { return _noOfCarriers; }

  short& noOfOptionsPerCarrier() { return _noOfOptionsPerCarrier; }
  const short& noOfOptionsPerCarrier() const { return _noOfOptionsPerCarrier; }

  std::vector<VISTimeBin>& timeOfDayBins() { return _timeOfDayBins; }
  const std::vector<VISTimeBin>& timeOfDayBins() const { return _timeOfDayBins; }

  short& noOfOptionsPerTimeBin() { return _noOfOptionsPerTimeBin; }
  const short& noOfOptionsPerTimeBin() const { return _noOfOptionsPerTimeBin; }

  short& noOfElapsedTimeOptions() { return _noOfElapsedTimeOptions; }
  const short& noOfElapsedTimeOptions() const { return _noOfElapsedTimeOptions; }

  short& noOfUtilityValueOptions() { return _noOfUtilityValueOptions; }
  const short& noOfUtilityValueOptions() const { return _noOfUtilityValueOptions; }

  double& nonStopFareMultiplier() { return _nonStopFareMultiplier; }
  const double& nonStopFareMultiplier() const { return _nonStopFareMultiplier; }

  short& noOfNonStopOptions() { return _noOfNonStopOptions; }
  const short& noOfNonStopOptions() const { return _noOfNonStopOptions; }

private:
  // Q82 - Priority of selection by carrier
  short _carrierPriority;
  // Q83 - Priority of selection by time of day
  short _timeOfDayPriority;
  // Q84 - Priority of selection by elapsed time
  short _elapsedTimePriority;
  // Q85 - Priority of selection by utility value
  short _utilityValuePriority;
  // Q86 - Priority of non-stop selection
  short _nonStopPriority;
  // Q87 - Selection by carrier - no. of carriers
  short _noOfCarriers;
  // Q88 - Selection by carrier - no. of options per carrier
  short _noOfOptionsPerCarrier;
  // SG1 - Selection by time-of-day binStart/end times
  std::vector<VISTimeBin> _timeOfDayBins;
  // Q89 - Selection by time-of-day no. of options per bin
  short _noOfOptionsPerTimeBin;
  // Q90 - Selection by elapsed time - no. of options
  short _noOfElapsedTimeOptions;
  // Q91 - Selection by utility value - no. of options
  short _noOfUtilityValueOptions;
  // Q92 - Non-stop selection - fareMultiplier
  double _nonStopFareMultiplier;
  // Q93 - Non-stop selection - no. of options
  short _noOfNonStopOptions;
};

class VISInboundSelectionRT
{
public:
  VISInboundSelectionRT();

  ~VISInboundSelectionRT();

  short& lowestFarePriority() { return _lowestFarePriority; }
  const short& lowestFarePriority() const { return _lowestFarePriority; }

  short& timeOfDayPriority() { return _timeOfDayPriority; }
  const short& timeOfDayPriority() const { return _timeOfDayPriority; }

  short& elapsedTimePriority() { return _elapsedTimePriority; }
  const short& elapsedTimePriority() const { return _elapsedTimePriority; }

  short& utilityValuePriority() { return _utilityValuePriority; }
  const short& utilityValuePriority() const { return _utilityValuePriority; }

  short& nonStopPriority() { return _nonStopPriority; }
  const short& nonStopPriority() const { return _nonStopPriority; }

  short& simpleInterlinePriority() { return _simpleInterlinePriority; }
  const short& simpleInterlinePriority() const { return _simpleInterlinePriority; }

  short& noOfLFSOptions() { return _noOfLFSOptions; }
  const short& noOfLFSOptions() const { return _noOfLFSOptions; }

  std::vector<VISTimeBin>& timeOfDayBins() { return _timeOfDayBins; }
  const std::vector<VISTimeBin>& timeOfDayBins() const { return _timeOfDayBins; }

  short& noOfOptionsPerTimeBin() { return _noOfOptionsPerTimeBin; }
  const short& noOfOptionsPerTimeBin() const { return _noOfOptionsPerTimeBin; }

  short& noOfElapsedTimeOptions() { return _noOfElapsedTimeOptions; }
  const short& noOfElapsedTimeOptions() const { return _noOfElapsedTimeOptions; }

  short& noOfUtilityValueOptions() { return _noOfUtilityValueOptions; }
  const short& noOfUtilityValueOptions() const { return _noOfUtilityValueOptions; }

  double& nonStopFareMultiplier() { return _nonStopFareMultiplier; }
  const double& nonStopFareMultiplier() const { return _nonStopFareMultiplier; }

  short& noOfNonStopOptions() { return _noOfNonStopOptions; }
  const short& noOfNonStopOptions() const { return _noOfNonStopOptions; }

  short& noOfSimpleInterlineOptions() { return _noOfSimpleInterlineOptions; }
  const short& noOfSimpleInterlineOptions() const { return _noOfSimpleInterlineOptions; }

private:
  // Q94 - Priority of selection by lowest fare
  short _lowestFarePriority;
  // Q95 - Priority of selection by time of day
  short _timeOfDayPriority;
  // Q96 - Priority of selection by elapsed time
  short _elapsedTimePriority;
  // Q97 - Priority of selection by utility value
  short _utilityValuePriority;
  // Q98 - Priority of non-stop selection
  short _nonStopPriority;
  // Q99 - Priority of simple interline selection
  short _simpleInterlinePriority;
  // QA0 - Selection by lowest fare - no. of options
  short _noOfLFSOptions;
  // SG2 - Selection by time-of-day binStart/end times
  std::vector<VISTimeBin> _timeOfDayBins;
  // QA1 - Selection by time-of-day no. of options per bin
  short _noOfOptionsPerTimeBin;
  // QA2 - Selection by elapsed time - no. of options
  short _noOfElapsedTimeOptions;
  // QA3 - Selection by utility value - no. of options
  short _noOfUtilityValueOptions;
  // QA4 - Non-stop selection - fareMultiplier
  double _nonStopFareMultiplier;
  // QA5 - Non-stop selection - no. of options
  short _noOfNonStopOptions;
  // QA6 - Simple interline selection - no. of options
  short _noOfSimpleInterlineOptions;
};

class VISOutboundSelectionOW
{
public:
  VISOutboundSelectionOW();

  ~VISOutboundSelectionOW();

  short& carrierPriority() { return _carrierPriority; }
  const short& carrierPriority() const { return _carrierPriority; }

  short& timeOfDayPriority() { return _timeOfDayPriority; }
  const short& timeOfDayPriority() const { return _timeOfDayPriority; }

  short& elapsedTimePriority() { return _elapsedTimePriority; }
  const short& elapsedTimePriority() const { return _elapsedTimePriority; }

  short& utilityValuePriority() { return _utilityValuePriority; }
  const short& utilityValuePriority() const { return _utilityValuePriority; }

  short& nonStopPriority() { return _nonStopPriority; }
  const short& nonStopPriority() const { return _nonStopPriority; }

  short& noOfCarriers() { return _noOfCarriers; }
  const short& noOfCarriers() const { return _noOfCarriers; }

  short& noOfOptionsPerCarrier() { return _noOfOptionsPerCarrier; }
  const short& noOfOptionsPerCarrier() const { return _noOfOptionsPerCarrier; }

  std::vector<VISTimeBin>& timeOfDayBins() { return _timeOfDayBins; }
  const std::vector<VISTimeBin>& timeOfDayBins() const { return _timeOfDayBins; }

  short& noOfOptionsPerTimeBin() { return _noOfOptionsPerTimeBin; }
  const short& noOfOptionsPerTimeBin() const { return _noOfOptionsPerTimeBin; }

  short& noOfElapsedTimeOptions() { return _noOfElapsedTimeOptions; }
  const short& noOfElapsedTimeOptions() const { return _noOfElapsedTimeOptions; }

  short& noOfUtilityValueOptions() { return _noOfUtilityValueOptions; }
  const short& noOfUtilityValueOptions() const { return _noOfUtilityValueOptions; }

  double& nonStopFareMultiplier() { return _nonStopFareMultiplier; }
  const double& nonStopFareMultiplier() const { return _nonStopFareMultiplier; }

  short& noOfNonStopOptions() { return _noOfNonStopOptions; }
  const short& noOfNonStopOptions() const { return _noOfNonStopOptions; }

private:
  // QA7 - Priority of selection by carrier
  short _carrierPriority;
  // QA8 - Priority of selection by time of day
  short _timeOfDayPriority;
  // QA9 - Priority of selection by elapsed time
  short _elapsedTimePriority;
  // QB0 - Priority of selection by utility value
  short _utilityValuePriority;
  // QB1 - Priority of non-stop selection
  short _nonStopPriority;
  // QB2 - Selection by carrier - no. of carriers
  short _noOfCarriers;
  // QB3 - Selection by carrier - no. of options per carrier
  short _noOfOptionsPerCarrier;
  // SG3 - Selection by time-of-day binStart/end times
  std::vector<VISTimeBin> _timeOfDayBins;
  // QB4 - Selection by time-of-day no. of options per bin
  short _noOfOptionsPerTimeBin;
  // QB5 - Selection by elapsed time - no. of options
  short _noOfElapsedTimeOptions;
  // QB6 - Selection by utility value - no. of options
  short _noOfUtilityValueOptions;
  // QB7 - Non-stop selection - fareMultiplier
  double _nonStopFareMultiplier;
  // QB8 - Non-stop selection - no. of options
  short _noOfNonStopOptions;
};

class VISLowFareSearch
{
public:
  VISLowFareSearch();

  ~VISLowFareSearch();

  short& noOfLFSItineraries() { return _noOfLFSItineraries; }
  const short& noOfLFSItineraries() const { return _noOfLFSItineraries; }

  short& noOfAdditionalOutboundsRT() { return _noOfAdditionalOutboundsRT; }
  const short& noOfAdditionalOutboundsRT() const { return _noOfAdditionalOutboundsRT; }

  short& noOfAdditionalInboundsRT() { return _noOfAdditionalInboundsRT; }
  const short& noOfAdditionalInboundsRT() const { return _noOfAdditionalInboundsRT; }

  short& noOfAdditionalOutboundsOW() { return _noOfAdditionalOutboundsOW; }
  const short& noOfAdditionalOutboundsOW() const { return _noOfAdditionalOutboundsOW; }

private:
  // QB9 - No. of LFS itineraries
  short _noOfLFSItineraries;
  // QC0 - No. of additional outbounds (RT)
  short _noOfAdditionalOutboundsRT;
  // QC1 - No. of additional inbounds (RT)
  short _noOfAdditionalInboundsRT;
  // QC3 - No. of additional outbounds (OW)
  short _noOfAdditionalOutboundsOW;
};

class VISOptions
{
public:
  VISOptions() {};

  ~VISOptions() {};

  ValueBasedItinSelection& valueBasedItinSelection() { return _valueBasedItinSelection; }
  const ValueBasedItinSelection& valueBasedItinSelection() const
  {
    return _valueBasedItinSelection;
  }

  VISOutboundSelectionRT& visOutboundSelectionRT() { return _visOutboundSelectionRT; }
  const VISOutboundSelectionRT& visOutboundSelectionRT() const { return _visOutboundSelectionRT; }

  VISInboundSelectionRT& visInboundSelectionRT() { return _visInboundSelectionRT; }
  const VISInboundSelectionRT& visInboundSelectionRT() const { return _visInboundSelectionRT; }

  VISOutboundSelectionOW& visOutboundSelectionOW() { return _visOutboundSelectionOW; }
  const VISOutboundSelectionOW& visOutboundSelectionOW() const { return _visOutboundSelectionOW; }

  VISLowFareSearch& visLowFareSearch() { return _visLowFareSearch; }
  const VISLowFareSearch& visLowFareSearch() const { return _visLowFareSearch; }

private:
  ValueBasedItinSelection _valueBasedItinSelection;
  VISOutboundSelectionRT _visOutboundSelectionRT;
  VISInboundSelectionRT _visInboundSelectionRT;
  VISOutboundSelectionOW _visOutboundSelectionOW;
  VISLowFareSearch _visLowFareSearch;
};

} // End namespace tse

