#pragma once

#include "DataModel/Common/Types.h"
#include <vector>

namespace tax
{
class FlightUsage;
class Itin;
class Geo;

class SpecificTimeStopoverChecker
{
public:
  explicit SpecificTimeStopoverChecker(int minutes);
  bool isStopover(const FlightUsage& prev, const FlightUsage& next) const;

private:
  int _minutes;
};

class MonthsStopoverChecker
{
public:
  explicit MonthsStopoverChecker(int months) : _months(months) {};
  bool isStopover(const FlightUsage& prev, const FlightUsage& next) const;

private:
  int _months;
};

class SameDayStopoverChecker
{
public:
  explicit SameDayStopoverChecker(int hours);
  bool isStopover(const FlightUsage& prev, const FlightUsage& next) const;

private:
  int _minutes;
};

class DateStopoverChecker
{
public:
  explicit DateStopoverChecker(int days);
  bool isStopover(const FlightUsage& prev, const FlightUsage& next) const;

private:
  int _days;
};

class SpecialDomesticTimeStopoverChecker
{
public:
  explicit SpecialDomesticTimeStopoverChecker(const Itin& itin);
  bool isStopover(const FlightUsage& prev, const FlightUsage& next) const;

private:
  const std::vector<Geo>& _geos;
};

class DomesticTimeStopoverChecker
{
public:
  explicit DomesticTimeStopoverChecker(const Itin& itin);
  explicit DomesticTimeStopoverChecker(const Itin& itin, int minutes, type::Nation);
  bool isStopover(const FlightUsage& prev, const FlightUsage& next) const;

private:
  SpecificTimeStopoverChecker _byMinutes;
  bool _isDomestic;
};
}
