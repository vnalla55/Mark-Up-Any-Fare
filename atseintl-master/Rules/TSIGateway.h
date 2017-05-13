//-------------------------------------------------------------------
////
////  Copyright Sabre 2009
////
////          The copyright to the computer program(s) herein
////          is the property of Sabre.
////          The program(s) may be used and/or copied only with
////          the written permission of Sabre or in accordance
////          with the terms and conditions stipulated in the
////          agreement/contract under which the program(s)
////          have been supplied.
////
////-------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"

#include <vector>

namespace tse
{

class TravelSeg;

class TSIGateway
{
  friend class TSIGatewayTest;

public:
  TSIGateway() : _markGWType(MARK_NONE), _tvlSegs(nullptr) {}

  enum MarkGWType
  {
    MARK_NONE,
    MARK_ALL_GATEWAY,
    MARK_ORIG_GATEWAY,
    MARK_DEST_GATEWAY
  };

  bool isDepartureFromGW(const TravelSeg*) const;
  bool isArrivalOnGW(const TravelSeg*) const;

  bool markGW(TSIGateway::MarkGWType markType, const std::vector<TravelSeg*>& tvlSegs);

  bool markGW(TSIGateway::MarkGWType markType,
              const std::vector<TravelSeg*>& outTvlSegs,
              const std::vector<TravelSeg*>& inTvlSegs);

  bool markGwRtw(MarkGWType markType, const std::vector<TravelSeg*>& segs);

private:
  void markGWBtwAreas(const IATAAreaCode& origArea, const IATAAreaCode& destArea);
  void markGWBtwZones(const TravelSeg& orig, const TravelSeg& dest);
  void markGWBtwNations(const NationCode& origNation, const NationCode& destNation);
  template<class GwPred>
  bool markGwByPred(const GwPred& gwPred);
  bool combineInboundGW(const tse::TSIGateway&);
  bool foundGW() const;
  TravelSeg* tsDepartFromOrigGW() const;
  TravelSeg* tsArriveOnDestGW() const;

  MarkGWType _markGWType;
  const std::vector<TravelSeg*>* _tvlSegs;
  std::vector<TravelSeg*> _savedTvlSegs;
  std::set<uint16_t> _gwLocIndex;
};

} // namespace tse

