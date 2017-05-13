#pragma once

#include "Diagnostic/Diag942Collector.h"
#include "Pricing/Shopping/PQ/SoloItinGenerator.h"

#include <vector>

namespace tse
{
class FarePath;
class ShoppingTrx;
}

namespace tse
{
namespace shpq
{

class SoloFamilyGrouping
{
public:
  enum GroupingOption
  {
    DISABLE_GROUPING,
    GROUP_ALL_SOLUTIONS,
    GROUP_SOL_FAREPATHS_ONLY
  };

  SoloFamilyGrouping(ShoppingTrx& trx,
                     const GroupingOption groupingOption,
                     const FarePath* const farePath)
    : _trx(trx), _groupingOption(groupingOption)
  {
    setFarePathInfo(farePath);
  }

  void process(SoloItinGenerator::SolutionVector& solutionsVector);

  static GroupingOption getGroupingOption(uint32_t configValue);

private:
  struct FarePathInfo
  {
    FarePathInfo() : _thruFarePath(true), _dualFMFlag(false), _industryFare(false) {}

    bool _thruFarePath;
    bool _dualFMFlag;
    bool _industryFare;
  };

  void setFarePathInfo(const FarePath* const farePath);
  void printSopIdVec(const SopIdVec& sopIdVec);
  void printMothersAndChildern();
  void printHeader(const FarePath* const farePath);

  bool isThruFarePath() const { return _farePathInfo._thruFarePath; }
  bool isDualFMFlag() const { return _farePathInfo._dualFMFlag; }
  bool groupAll() const { return _groupingOption == GROUP_ALL_SOLUTIONS; }
  bool compareSops() const { return (groupAll() ? true : !isThruFarePath()); }
  bool isIndustryFareUsed() const { return _farePathInfo._industryFare; }
  void processGroupSolFarePathsOnly(SoloItinGenerator::SolutionVector& solutionsVector);
  void decideSopRelationship(SoloItinGenerator::VectorOfSopVec& sopsAddedToFlightMatrix,
                             SoloItinGenerator::VectorOfSopVec::iterator& similarSop,
                             const SoloItinGenerator::SolutionItem& sol);

private:
  ShoppingTrx& _trx;
  GroupingOption _groupingOption;
  FarePathInfo _farePathInfo;
  Diag942Collector* _diagPtr = nullptr;
};
}
} // namespace tse::shpq

