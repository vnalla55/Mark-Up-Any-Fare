// ----------------------------------------------------------------
//
//   Copyright Sabre 2015
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#pragma once
#include "Common/YQYR/YQYRCalculator.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/RexBaseTrx.h"

#include <map>
#include <utility>
#include <vector>

namespace tse
{
class DateTime;
class FareMarketPath;
class FarePath;
class Itin;
class PaxType;
class PricingTrx;

class YQYRCalculatorForREX : public YQYRCalculator
{
public:
  YQYRCalculatorForREX(PricingTrx& trx,
                       Itin& it,
                       const FareMarketPath* fmp,
                       const PaxType* paxType);

  virtual MoneyAmount chargeFarePath(const FarePath& fp, const CarrierCode valCxr) const override;
  virtual void process() override;
  virtual void findMatchingPaths(const FarePath* fp,
                                 const CarrierCode valCxr,
                                 std::vector<YQYRApplication>& yqyrApplications) override;

private:
  typedef std::pair<DateTime, bool> CalcKey;
  typedef std::map<CalcKey, YQYRCalculator*> CalcMap;
  CalcMap _calculators;
  RexBaseTrx& rexBaseTrx() const { return static_cast<RexBaseTrx&>(_trx); }

  YQYRCalculator& getDesignatedCalculator(const FarePath& fp) const;
};
} // tse namespace
