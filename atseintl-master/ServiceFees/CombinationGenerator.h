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

#include "Common/TseUtil.h"

#include <map>
#include <vector>

#include <time.h>

namespace tse
{
class FareMarket;

class CombinationGenerator
{
  friend class CombinationGeneratorTest;
  int _segmentsNo;
  int _seg1_1;
  int _seg1_2;
  int _seg1_3;
  int _seg1_4;
  int _seg2_2;
  int _seg2_3;
  int _seg2_4;
  int _seg3_3;
  int _seg3_4;
  int _seg4_4;
  std::map<int,
           std::pair<std::vector<TravelSeg*>::const_iterator,
                     std::vector<TravelSeg*>::const_iterator>*> _mappedRoute;
  clock_t _clockTime;
  uint64_t _combFilterMask;

public:
  CombinationGenerator(uint16_t segmentsNo,
                       std::vector<std::pair<std::vector<TravelSeg*>::const_iterator,
                                             std::vector<TravelSeg*>::const_iterator> >& route,
                       std::vector<TravelSeg*>::const_iterator& firstSegIt);
  void generate(TseUtil::SolutionSet& solutions, int timeout) const;
  void generate(TseUtil::SolutionSet& solutions, const uint16_t level, int timeout) const;
  void generate(TseUtil::SolutionSet& solutions,
                const uint16_t level,
                uint64_t combFilterMask,
                int timeout);

private:
  void generate(TseUtil::SolutionSet& solutions,
                uint64_t rot3n,
                uint64_t (CombinationGenerator::*interFun)(const uint64_t&) const,
                void (CombinationGenerator::*addSol)(TseUtil::SolutionSet&,
                                                     const uint64_t&,
                                                     const uint16_t) const,
                const uint16_t level,
                int timeout) const;
  uint64_t iterateWithoutMissingCities(const uint64_t& combNo) const;
  uint64_t iterateWithMissingCities(const uint64_t& combNo) const;
  void
  addSolution(TseUtil::SolutionSet& solutions, const uint64_t& combNo, const uint16_t level) const;
  int numberOfSetBits(uint64_t val) const;
  void addFilteredSolution(TseUtil::SolutionSet& solutions,
                           const uint64_t& combNo,
                           const uint16_t level) const;
  bool
  collectFareMarkets(int fms[],
                     std::vector<std::pair<std::vector<TravelSeg*>::const_iterator,
                                           std::vector<TravelSeg*>::const_iterator>*>& route) const;
  int getFareMarkets(const uint8_t combNo,
                     int startShift,
                     int endShift,
                     std::vector<std::pair<std::vector<TravelSeg*>::const_iterator,
                                           std::vector<TravelSeg*>::const_iterator>*>& route) const;
};

} // tse namespace

