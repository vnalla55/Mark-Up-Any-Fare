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
#include "ServiceFees/CombinationGenerator.h"

#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "DataModel/FareMarket.h"
#include "DataModel/TravelSeg.h"

namespace tse
{
namespace
{

class MapFM
{
  std::map<int,
           std::pair<std::vector<TravelSeg*>::const_iterator,
                     std::vector<TravelSeg*>::const_iterator>*>& _mappedRoute;
  uint16_t _segmentsNo;
  std::vector<TravelSeg*>::const_iterator& _firstSegIt;

public:
  MapFM(std::map<int,
                 std::pair<std::vector<TravelSeg*>::const_iterator,
                           std::vector<TravelSeg*>::const_iterator>*>& mappedRoute,
        int segmentsNo,
        std::vector<TravelSeg*>::const_iterator& firstSegIt)
    : _mappedRoute(mappedRoute), _segmentsNo(segmentsNo), _firstSegIt(firstSegIt)
  {
  }

  void operator()(std::pair<std::vector<TravelSeg*>::const_iterator,
                            std::vector<TravelSeg*>::const_iterator>& route)
  {
    _mappedRoute.insert(std::make_pair((std::distance(_firstSegIt, route.first) + 1) * _segmentsNo +
                                           std::distance(_firstSegIt, route.second),
                                       &route));
  }
};

struct UpdateFareMarkets
{
  UpdateFareMarkets(std::vector<std::pair<std::vector<TravelSeg*>::const_iterator,
                                          std::vector<TravelSeg*>::const_iterator>*>& routes)
    : _routes(routes)
  {
  }
  void operator()(TseUtil::Solution& sol) { sol._routes.swap(_routes); }

  std::vector<std::pair<std::vector<TravelSeg*>::const_iterator,
                        std::vector<TravelSeg*>::const_iterator>*>& _routes;
};
}

static Logger
logger("atseintl.ServiceFees.CombinationGenerator");

CombinationGenerator::CombinationGenerator(
    uint16_t segmentsNo,
    std::vector<std::pair<std::vector<TravelSeg*>::const_iterator,
                          std::vector<TravelSeg*>::const_iterator> >& route,
    std::vector<TravelSeg*>::const_iterator& firstSegIt)
  : _segmentsNo(segmentsNo), _clockTime(clock()), _combFilterMask(0)
{
  _seg1_1 = _segmentsNo + 1;
  _seg1_2 = _segmentsNo + 2;
  _seg1_3 = _segmentsNo + 3;
  _seg1_4 = _segmentsNo + 4;
  _seg2_2 = (_segmentsNo << 1) + 2;
  _seg2_3 = (_segmentsNo << 1) + 3;
  _seg2_4 = (_segmentsNo << 1) + 4;
  _seg3_3 = (_segmentsNo << 1) + _segmentsNo + 3;
  _seg3_4 = (_segmentsNo << 1) + _segmentsNo + 4;
  _seg4_4 = (_segmentsNo << 2) + 4;

  std::for_each(route.begin(), route.end(), MapFM(_mappedRoute, _segmentsNo, firstSegIt));
}

bool
CombinationGenerator::collectFareMarkets(
    int fms[],
    std::vector<std::pair<std::vector<TravelSeg*>::const_iterator,
                          std::vector<TravelSeg*>::const_iterator>*>& route) const
{
  std::map<int,
           std::pair<std::vector<TravelSeg*>::const_iterator,
                     std::vector<TravelSeg*>::const_iterator>*>::const_iterator pos =
      _mappedRoute.find(fms[0]);
  if (pos == _mappedRoute.end())
    return false;
  route.push_back(pos->second);
  if (!fms[1])
    return true;
  if ((pos = _mappedRoute.find(fms[1])) == _mappedRoute.end())
    return false;
  route.push_back(pos->second);
  if (!fms[2])
    return true;
  if ((pos = _mappedRoute.find(fms[2])) == _mappedRoute.end())
    return false;
  route.push_back(pos->second);
  if (!fms[3])
    return true;
  if ((pos = _mappedRoute.find(fms[3])) == _mappedRoute.end())
    return false;
  route.push_back(pos->second);
  return true;
}

int
CombinationGenerator::getFareMarkets(
    const uint8_t combNo,
    int startShift,
    int endShift,
    std::vector<std::pair<std::vector<TravelSeg*>::const_iterator,
                          std::vector<TravelSeg*>::const_iterator>*>& route) const
{
  if (!combNo)
    return startShift;
  int fms[4] = { 0 };

  int shift = _segmentsNo * startShift;
  int shiftE = _segmentsNo * endShift;

  switch (combNo)
  {
  case 3:
    return endShift + 1;
  case 15:
    return endShift + 2;
  case 63:
    return endShift + 3;
  case 255:
    return endShift + 4;
  case 2: // 1-1
  case 14:
  case 62:
  case 254:
  {
    fms[0] = shift + _seg1_1 + endShift;
  }
  break;
  case 8: // 1-2
  case 56:
  case 248:
  {
    fms[0] = shift + _seg1_2 + endShift;
  }
  break;
  case 10: // 1-1 2-2
  case 58:
  case 250:
  {
    fms[0] = shift + _seg1_1 + endShift;
    fms[1] = shiftE + _seg2_2 + endShift;
  }
  break;
  case 11: // 2-2
  case 59:
  case 251:
  {
    fms[0] = shift + _seg2_2 + endShift;
  }
  break;
  case 32: // 1-3
  case 224:
  {
    fms[0] = shift + _seg1_3 + endShift;
  }
  break;
  case 34: // 1-1 2-3
  case 226:
  {
    fms[0] = shift + _seg1_1 + endShift;
    fms[1] = shiftE + _seg2_3 + endShift;
  }
  break;
  case 35: // 2-3
  case 227:
  {
    fms[0] = shift + _seg2_3 + endShift;
  }
  break;
  case 40: // 1-2 3-3
  case 232:
  {
    fms[0] = shift + _seg1_2 + endShift;
    fms[1] = shiftE + _seg3_3 + endShift;
  }
  break;
  case 42: // 1-1 2-2 3-3
  case 234:
  {
    fms[0] = shift + _seg1_1 + endShift;
    fms[1] = shiftE + _seg2_2 + endShift;
    fms[2] = shiftE + _seg3_3 + endShift;
  }
  break;
  case 43: // 2-2 3-3
  case 235:
  {
    fms[0] = shift + _seg2_2 + endShift;
    fms[1] = shiftE + _seg3_3 + endShift;
  }
  break;
  case 46: // 1-1 3-3
  case 238:
  {
    fms[0] = shift + _seg1_1 + endShift;
    fms[1] = shiftE + _seg3_3 + endShift;
  }
  break;
  case 47: // 3-3
  case 239:
  {
    fms[0] = shift + _seg3_3 + endShift;
  }
  break;
  case 128: // 1-4
  {
    fms[0] = shift + _seg1_4 + endShift;
  }
  break;
  case 130: // 1-1 2-4
  {
    fms[0] = shift + _seg1_1 + endShift;
    fms[1] = shiftE + _seg2_4 + endShift;
  }
  break;
  case 131: // 2-4
  {
    fms[0] = shift + _seg2_4 + endShift;
  }
  break;
  case 136: // 1-2 3-4
  {
    fms[0] = shift + _seg1_2 + endShift;
    fms[1] = shiftE + _seg3_4 + endShift;
  }
  break;
  case 138: // 1-1 2-2 3-4
  {
    fms[0] = shift + _seg1_1 + endShift;
    fms[1] = shiftE + _seg2_2 + endShift;
    fms[2] = shiftE + _seg3_4 + endShift;
  }
  break;
  case 139: // 2-2 3-4
  {
    fms[0] = shift + _seg2_2 + endShift;
    fms[1] = shiftE + _seg3_4 + endShift;
  }
  break;
  case 142: // 1-1 3-4
  {
    fms[0] = shift + _seg1_1 + endShift;
    fms[1] = shiftE + _seg3_4 + endShift;
  }
  break;
  case 143: // 3-4
  {
    fms[0] = shift + _seg3_4 + endShift;
  }
  break;
  case 160: // 1-3 4-4
  {
    fms[0] = shift + _seg1_3 + endShift;
    fms[1] = shiftE + _seg4_4 + endShift;
  }
  break;
  case 162: // 1-1 2-3 4-4
  {
    fms[0] = shift + _seg1_1 + endShift;
    fms[1] = shiftE + _seg2_3 + endShift;
    fms[2] = shiftE + _seg4_4 + endShift;
  }
  break;
  case 163: // 2-3 4-4
  {
    fms[0] = shift + _seg2_3 + endShift;
    fms[1] = shiftE + _seg4_4 + endShift;
  }
  break;
  case 168: // 1-2 3-3 4-4
  {
    fms[0] = shift + _seg1_2 + endShift;
    fms[1] = shiftE + _seg3_3 + endShift;
    fms[2] = shiftE + _seg4_4 + endShift;
  }
  break;
  case 170: // 1-1 2-2 3-3 4-4
  {
    fms[0] = shift + _seg1_1 + endShift;
    fms[1] = shiftE + _seg2_2 + endShift;
    fms[2] = shiftE + _seg3_3 + endShift;
    fms[3] = shiftE + _seg4_4 + endShift;
  }
  break;
  case 171: // 2-2 3-3 4-4
  {
    fms[0] = shift + _seg2_2 + endShift;
    fms[1] = shiftE + _seg3_3 + endShift;
    fms[2] = shiftE + _seg4_4 + endShift;
  }
  break;
  case 174: // 1-1 3-3 4-4
  {
    fms[0] = shift + _seg1_1 + endShift;
    fms[1] = shiftE + _seg3_3 + endShift;
    fms[2] = shiftE + _seg4_4 + endShift;
  }
  break;
  case 175: // 3-3 4-4
  {
    fms[0] = shift + _seg3_3 + endShift;
    fms[1] = shiftE + _seg4_4 + endShift;
  }
  break;
  case 184: // 1-2 4-4
  {
    fms[0] = shift + _seg1_2 + endShift;
    fms[1] = shiftE + _seg4_4 + endShift;
  }
  break;
  case 186: // 1-1 2-2 4-4
  {
    fms[0] = shift + _seg1_1 + endShift;
    fms[1] = shiftE + _seg2_2 + endShift;
    fms[2] = shiftE + _seg4_4 + endShift;
  }
  break;
  case 187: // 2-2 4-4
  {
    fms[0] = shift + _seg2_2 + endShift;
    fms[1] = shiftE + _seg4_4 + endShift;
  }
  break;
  case 190: // 1-1 4-4
  {
    fms[0] = shift + _seg1_1 + endShift;
    fms[1] = shiftE + _seg4_4 + endShift;
  }
  break;
  case 191: // 4-4
  {
    fms[0] = shift + _seg4_4 + endShift;
  }
  break;
  default:
  {
    LOG4CXX_DEBUG(logger, "CombinationGenerator::getFareMarkets - unhandled bits combination");
    return -1;
  }
  }
  if (!collectFareMarkets(fms, route))
    return -1;
  if (combNo & 192)
    return endShift + 4;
  if (combNo & 48)
    return endShift + 3;
  if (combNo & 12)
    return endShift + 2;
  if (combNo & 3)
    return endShift + 1;
  return endShift;
}

void
CombinationGenerator::addSolution(TseUtil::SolutionSet& solutions,
                                  const uint64_t& combNo,
                                  const uint16_t level) const
{
  std::vector<std::pair<std::vector<TravelSeg*>::const_iterator,
                        std::vector<TravelSeg*>::const_iterator>*> routes;
  int startShift = 0;
  uint64_t combNoTmp = combNo;
  int segmentsNo = _segmentsNo;

  while (segmentsNo > 0)
  {
    if ((startShift =
             getFareMarkets(combNoTmp & 0xFF, startShift, _segmentsNo - segmentsNo, routes)) == -1)
      break;
    segmentsNo -= 4;
    combNoTmp >>= 8;
  }

  if (startShift != -1)
    solutions.modify(
        solutions.insert(TseUtil::Solution(
                             level,
                             std::vector<std::pair<std::vector<TravelSeg*>::const_iterator,
                                                   std::vector<TravelSeg*>::const_iterator>*>()))
            .first,
        UpdateFareMarkets(routes));
}

int
CombinationGenerator::numberOfSetBits(uint64_t val) const
{
  val = val - ((val >> 1) & 0x5555555555555555);
  val = (val & 0x3333333333333333) + ((val >> 2) & 0x3333333333333333);
  return (((val + (val >> 4)) & 0xF0F0F0F0F0F0F0F) * 0x101010101010101) >> 56;
}

void
CombinationGenerator::addFilteredSolution(TseUtil::SolutionSet& solutions,
                                          const uint64_t& combNo,
                                          const uint16_t level) const
{
  uint64_t combFiltered = combNo & _combFilterMask;
  if (combFiltered && numberOfSetBits(combFiltered) == level)
    CombinationGenerator::addSolution(solutions, combNo, level);
}

void
CombinationGenerator::generate(TseUtil::SolutionSet& solutions,
                               uint64_t rot3n,
                               uint64_t (CombinationGenerator::*interFun)(const uint64_t&) const,
                               void (CombinationGenerator::*addSol)(TseUtil::SolutionSet&,
                                                                    const uint64_t&,
                                                                    const uint16_t) const,
                               const uint16_t level,
                               int timeout) const
{
  if (_segmentsNo > 32)
    return;
  uint64_t maxBit = 1ULL << ((_segmentsNo << 1) - 1);
  uint64_t combNoEnd = maxBit + maxBit - 1;

  uint64_t combNo = maxBit + rot3n;
  int segWindow = _segmentsNo;
  int CLOCKS_PER_MSEC = CLOCKS_PER_SEC / 1000;

  while (combNo < combNoEnd && (clock() - _clockTime) / CLOCKS_PER_MSEC < timeout)
  {
    segWindow >>= 1;
    uint64_t maxBitWindow = 1ULL << (((_segmentsNo - segWindow) << 1) - 1);
    uint64_t combNoWindowEnd = maxBit + maxBitWindow - 1;

    while (combNo < combNoWindowEnd)
    {
      (this->*addSol)(solutions, combNo, level);

      combNo = (this->*interFun)(combNo);
    }
  }
}

uint64_t
CombinationGenerator::iterateWithoutMissingCities(const uint64_t& combNo) const
{
  switch (combNo & 3)
  {
  case 0:
    return combNo + 2;
  case 2:
  {
    uint64_t combTmp = combNo >> 2;
    int shift = 2;
    while (combTmp & 2)
    {
      shift += 2;
      combTmp >>= 2;
    }
    return (combTmp + 2) << shift;
  }
  default:
    LOG4CXX_DEBUG(logger, "CombinationGenerator::getFareMarkets - bit 01 is not handled");
    break;
  }
  return combNo + 2;
}

uint64_t
CombinationGenerator::iterateWithMissingCities(const uint64_t& combNo) const
{
  uint64_t combTmp = combNo >> 2;
  int bitShiftNo = 2;
  uint64_t rot3 = 3;
  uint64_t shift2 = 2;

  switch (combNo & 3)
  {
  case 0:
    return combNo + 2;
  case 2:
  {
    while ((combTmp & 3) == 2)
    {
      bitShiftNo += 2;
      combTmp >>= 2;
      shift2 <<= 2;
    }
    while ((combTmp & 3) == 3)
    {
      bitShiftNo += 2;
      combTmp >>= 2;
      shift2 <<= 2;
      rot3 = (rot3 << 2) + 3;
    }
    rot3 >>= 2;
    if (combTmp & 2)
      return ((combTmp + 1) << bitShiftNo) + shift2 + (rot3 >> 2);
    else
      return ((combTmp + 2) << bitShiftNo) + rot3;
  }
  case 3:
  {
    bitShiftNo = 2;
    while ((combTmp & 3) == 3)
    {
      bitShiftNo += 2;
      combTmp >>= 2;
      shift2 <<= 2;
      rot3 = (rot3 << 2) + 3;
    }
    if (combTmp & 2)
      return ((combTmp + 1) << bitShiftNo) + shift2 + (rot3 >> 2);
    else
      return ((combTmp + 2) << bitShiftNo) + rot3;
  }
  default:
    LOG4CXX_DEBUG(logger, "CombinationGenerator::getFareMarkets - bit 01 is not handled");
    break;
  }
  return combNo + 2;
}

void
CombinationGenerator::generate(TseUtil::SolutionSet& solutions, int timeout) const
{
  generate(solutions,
           2,
           &CombinationGenerator::iterateWithoutMissingCities,
           &CombinationGenerator::addSolution,
           0,
           timeout);
}

void
CombinationGenerator::generate(TseUtil::SolutionSet& solutions, const uint16_t level, int timeout)
    const
{
  uint16_t lvl = level - 1;
  uint64_t rot3n = 3;
  while (lvl)
  {
    rot3n = (rot3n << 2) + 3;
    --lvl;
  }

  generate(solutions,
           rot3n,
           &CombinationGenerator::iterateWithMissingCities,
           &CombinationGenerator::addSolution,
           level,
           timeout);
}

void
CombinationGenerator::generate(TseUtil::SolutionSet& solutions,
                               const uint16_t level,
                               uint64_t combFilterMask,
                               int timeout)
{
  _combFilterMask = combFilterMask;

  uint16_t lvl = level - 1;
  uint64_t rot3n = 3;
  while (lvl)
  {
    rot3n = (rot3n << 2) + 3;
    --lvl;
  }

  generate(solutions,
           rot3n,
           &CombinationGenerator::iterateWithMissingCities,
           &CombinationGenerator::addFilteredSolution,
           level,
           timeout);
}
}
