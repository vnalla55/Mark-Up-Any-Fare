#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "DataModel/PricingOptions.h"

namespace tse
{

class NoPNRPricingOptions : public PricingOptions
{
private:
  Indicator _sortAscending = 0; // P88
  Indicator _sortDescending = 0; // P89
  Indicator _noMatch = 0; // AXM

protected:
  NoPNRPricingOptions(const NoPNRPricingOptions&) = delete;
  NoPNRPricingOptions& operator=(const NoPNRPricingOptions&) = delete;

public:
  NoPNRPricingOptions() = default;

  char& sortAscending() { return _sortAscending; }
  char sortAscending() const { return _sortAscending; }

  char& sortDescending() { return _sortDescending; }
  char sortDescending() const { return _sortDescending; }
  bool isSortDescending() const { return TypeConvert::pssCharToBool(_sortDescending); }

  char& noMatch() { return _noMatch; }
  char noMatch() const { return _noMatch; }
  bool isNoMatch() const { return TypeConvert::pssCharToBool(_noMatch); }
};
}
