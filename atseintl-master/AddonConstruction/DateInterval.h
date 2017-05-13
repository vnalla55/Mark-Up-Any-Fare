//-------------------------------------------------------------------
//
//  File:        DateInterval.h
//  Created:     Feb 25, 2005
//
//  Description:
//
//  Copyright Sabre 2005
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/TSEDateInterval.h"

namespace tse
{

class InhibitedDateInterval : public TSEDateInterval
{
public:
  Indicator& inhibit() { return _inhibit; }
  const Indicator inhibit() const { return _inhibit; }

protected:
  Indicator _inhibit;
};

class DateIntervalBase
{

public:
  DateIntervalBase() : _splittedPart('0') {};
  virtual ~DateIntervalBase() {};

  virtual void clone(const DateIntervalBase& cloneFrom) { _effInterval = cloneFrom.effInterval(); }

  // accessors
  // =========

  TSEDateInterval& effInterval()
  {
    return _effInterval;
  };
  const TSEDateInterval& effInterval() const
  {
    return _effInterval;
  };

  const Indicator splittedPart() const
  {
    return _splittedPart;
  };
  Indicator& splittedPart()
  {
    return _splittedPart;
  };

protected:
  TSEDateInterval _effInterval;
  Indicator _splittedPart;

  bool setInterval(TSEDateInterval& intervalToChange, const TSEDateInterval& newInterval)
  {
    intervalToChange = newInterval;

    return _effInterval.defineIntersection(_effInterval, newInterval);
  };

  bool
  setInterval(InhibitedDateInterval& intervalToChange, const InhibitedDateInterval& newInterval)
  {
    intervalToChange = newInterval;

    return _effInterval.defineIntersection(_effInterval, newInterval);
  };

}; // End of class DateIntervalBase

class AddonCortegeDateInterval : public DateIntervalBase
{

public:
  AddonCortegeDateInterval() : DateIntervalBase() {};
  virtual ~AddonCortegeDateInterval() {};

  // accessors
  // =========

  const TSEDateInterval& addonZoneInterval() const
  {
    return _addonZoneInterval;
  };
  bool setAddonZoneInterval(const TSEDateInterval& newInterval)
  {
    return setInterval(_addonZoneInterval, newInterval);
  };

protected:
  TSEDateInterval _addonZoneInterval;

}; // End of class AddonCortegeDateInterval

class AtpcoFareDateInterval : public DateIntervalBase
{
public:
  AtpcoFareDateInterval()
    : DateIntervalBase(),
      _showOrigCombFareClassInterval(false),
      _showDestCombFareClassInterval(false) {};
  virtual ~AtpcoFareDateInterval() {};

  // accessors
  // =========

  const InhibitedDateInterval& trfXRefInterval() const
  {
    return _trfXRefInterval;
  };
  bool setTrfXRefInterval(const InhibitedDateInterval& newInterval)
  {
    return setInterval(_trfXRefInterval, newInterval);
  };

  const TSEDateInterval& combFareClassInterval(const bool isOriginInterval) const
  {
    return (isOriginInterval ? _origCombFareClassInterval : _destCombFareClassInterval);
  };

  bool setCombFareClassInterval(const TSEDateInterval& newInterval, const bool isOriginInterval)
  {
    if (isOriginInterval)
      return setInterval(_origCombFareClassInterval, newInterval);

    return setInterval(_destCombFareClassInterval, newInterval);
  };

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
  bool setCombFareClassInterval(const DateTime& createDate,
                                const DateTime& expireDate,
                                const bool isOriginInterval)
  {
    TSEDateInterval interval;
    interval.createDate() = createDate;
    interval.effDate() = createDate;
    interval.discDate() = DateTime(max_date_time);
    interval.expireDate() = expireDate;
    return setCombFareClassInterval(interval, isOriginInterval);
  };
#endif

  const bool showCombFareClassInterval(const bool isOriginInterval) const
  {
    return (isOriginInterval ? _showOrigCombFareClassInterval : _showDestCombFareClassInterval);
  };

  bool& showCombFareClassInterval(const bool isOriginInterval)
  {
    return (isOriginInterval ? _showOrigCombFareClassInterval : _showDestCombFareClassInterval);
  };

protected:
  InhibitedDateInterval _trfXRefInterval;
  TSEDateInterval _origCombFareClassInterval;
  TSEDateInterval _destCombFareClassInterval;

  bool _showOrigCombFareClassInterval;
  bool _showDestCombFareClassInterval;

}; // End of class AddonCortegeDateInterval

class SitaFareDateInterval : public DateIntervalBase
{
public:
  SitaFareDateInterval() : DateIntervalBase() {};
  virtual ~SitaFareDateInterval() {};

protected:
}; // End of class SitaFareDateInterval

} // End namespace tse

