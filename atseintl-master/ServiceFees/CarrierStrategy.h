//-------------------------------------------------------------------
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
//-------------------------------------------------------------------
#pragma once

#include "Common/TsePrimitiveTypes.h"

#include <string>

namespace tse
{

struct CarrierStrategy
{
  virtual bool checkIndCrxInd(const Indicator& indCrxInd) const = 0;
  virtual bool getOperatingMarketingInd() const { return false; }
  bool isIndInd(const Indicator& indCrxInd) const;
  char getCarrierStrategyTypeShort() const { return _carrierStrategyTypeShort; }
  const std::string& getCarrierStrategyTypeFull() const { return _carrierStrategyTypeFull; }
  virtual ~CarrierStrategy() {}

protected:
  CarrierStrategy(char carrierStrategyTypeShort, const std::string& carrierStrategyTypeFull) :
    _carrierStrategyTypeShort(carrierStrategyTypeShort), _carrierStrategyTypeFull(carrierStrategyTypeFull) {}

  const char _carrierStrategyTypeShort;
  const std::string _carrierStrategyTypeFull;
};

struct MarketingCarrierStrategy : public CarrierStrategy
{
  bool checkIndCrxInd(const Indicator& indCrxInd) const override;
  bool getOperatingMarketingInd() const override { return true; }

  MarketingCarrierStrategy() : CarrierStrategy('M', "MARKETING") {}
  virtual ~MarketingCarrierStrategy() {}
};

struct OperatingCarrierStrategy : public CarrierStrategy
{
  bool checkIndCrxInd(const Indicator& indCrxInd) const override;

  OperatingCarrierStrategy() : CarrierStrategy('O', "OPERATING") {}
  virtual ~OperatingCarrierStrategy() {}
};

struct PartitionCarrierStrategy : public CarrierStrategy
{
  bool checkIndCrxInd(const Indicator& indCrxInd) const override { return true; }

  PartitionCarrierStrategy() : CarrierStrategy('P', "PARTITION") {}
  virtual ~PartitionCarrierStrategy() {}
};

struct MultipleOperatingCarrierStrategy : public CarrierStrategy
{
  bool checkIndCrxInd(const Indicator& indCrxInd) const override;

  MultipleOperatingCarrierStrategy() : CarrierStrategy('O', "OPERATING") {}
  virtual ~MultipleOperatingCarrierStrategy() {}
};

} // tse namespace

