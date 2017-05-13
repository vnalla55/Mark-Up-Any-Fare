// ----------------------------------------------------------------
//
//   Copyright Sabre 2011
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

#include "Common/ShpqTypes.h"
#include "DataModel/ItinIndex.h"
#include "DataModel/OwrtFareMarket.h"

#include <map>
#include <memory>
#include <vector>

namespace tse
{

class FareMarket;
class Loc;
class ShoppingTrx;

namespace shpq
{

class CxrFareMarkets;
class DirFMPathListCollector;

class CxrFMCollector
{

  friend class CxrFMCollectorTest;

  typedef std::vector<FareMarket*> FmVector;
  typedef std::shared_ptr<CxrFareMarkets> CxrFareMarketsPtr;

  struct TwoSegmentLeg;
  typedef std::map<const MultiAirportLocationPair, TwoSegmentLeg> CxrFMMap;

  class BaseParamsInfo
  {
  public:
    BaseParamsInfo(ShoppingTrx&, FareMarket*, const ItinIndex::Key&);

    bool isOwEnabled() const { return _owType; }
    bool isHrtEnabled() const { return _hrtType; }

    SolutionType owType() const { return _owType ? OW : NONE; }
    SolutionType hrtType() const { return _hrtType ? HRT : NONE; }

    ShoppingTrx& getTrx() const { return _trx; }
    FareMarket* getFareMarket() const { return _fm; }

    const ItinIndex::Key getCxrIndexKey() const { return _cxrIndexKey; }

  private:
    ShoppingTrx& _trx;
    FareMarket* _fm;
    bool _owType;
    bool _hrtType;
    const ItinIndex::Key _cxrIndexKey;
  };

  class OwHrtFMPair
  {
  public:
    void insertFM(const BaseParamsInfo&);
    CxrFareMarketsPtr getCxrFM(const SolutionType sType) const;

    bool isValid() const { return _cxrFM[0] || _cxrFM[1]; }

  private:
    void insertFM(const BaseParamsInfo&, const SolutionType);
    bool isTypeValid(const SolutionType sType) const;
    size_t getIndex(const SolutionType sType) const;

    CxrFareMarketsPtr _cxrFM[2]; // 0 - OW, 1 - HRT
  };

  struct TwoSegmentLeg
  {
    enum
    {
      ORIGIN_SEG, // index of first seegment of leg (starts in _origin)
      DEST_SEG // index of second segment of leg (end in _destination)
    };

    OwHrtFMPair _segments[2];
  };

public:
  typedef std::shared_ptr<DirFMPathListCollector> DirFMPathListCollectorPtr;
  typedef std::set<LocCode> CitySet;

  CxrFMCollector(ShoppingTrx& trx,
                 const TravelSeg* origin,
                 const TravelSeg* dest,
                 const CitySet& citySet);

  bool addFareMarket(FareMarket*, const ItinIndex::Key cxrIndexKey);
  void collectDirFMPathList(DirFMPathListCollectorPtr);

private:
  void addToDirFMPathList(DirFMPathListCollectorPtr, OwHrtFMPair&);
  void addToDirFMPathList(DirFMPathListCollectorPtr, const TwoSegmentLeg&);

  void insertThruFM(const BaseParamsInfo&);
  void insertTwoSegmentsFM(uint16_t segmentIndex,
                           const BaseParamsInfo& paramsInfo,
                           const MultiAirportLocationPair breakPointLocationKey);

private:
  ShoppingTrx& _trx;

  OwHrtFMPair _thruFM; // pair of OW and HRT thru CxrFareMarkets
  const CitySet& _citySet;
  CxrFMMap _twoSegmentCxrFM; // two segments combination of CxrFareMarkets
  FmVector _addedFareMarkets;

  const TravelSeg* _origin;
  const TravelSeg* _destination;
};

typedef std::shared_ptr<CxrFMCollector> CxrFMCollectorPtr;
}
} // namespace tse::shpq
