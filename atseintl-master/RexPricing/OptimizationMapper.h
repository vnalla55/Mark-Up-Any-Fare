#pragma once

#include "DBAccess/Loc.h"

#include <vector>
#include <map>

namespace tse
{
class FareCompInfo;
class TravelSeg;
class DiagCollector;
class ExcItin;
class Itin;
class FareMarket;
class PaxTypeFare;

class OptimizationMapper
{
  friend class OptimizationMapperTest;
public:
  void processMapping(const ExcItin& excItin,
                      const Itin& newItin,
                      DiagCollector* dc);

  enum TariffRestr : int
  {
    PUBLIC = 0,
    PRIVATE = 1,
    UNRESTRICTED = 2
  };

  void addCarrierRestriction(const FareCompInfo* fci, std::set<CarrierCode> carriers);
  void addTariffRestriction(const FareCompInfo* fci, const PaxTypeFare* ptf);

  bool isFareMarketAllowed(const FareMarket* fm) const;
  TariffRestr isFMRestrToPrivOrPub(const FareMarket* fm) const;



private:
  class MapperBase
  {
  public:
    virtual bool operator()(const Loc* fciLoc, const Loc* segLoc) const = 0;
  };

  class CityMapper : public MapperBase
  {
  public:
    bool operator()(const Loc* fciLoc, const Loc* segLoc) const
    {
      return fciLoc->loc() == segLoc->loc();
    }
  };

  class CountryMapper : public MapperBase
  {
  public:
    bool operator()(const Loc* fciLoc, const Loc* segLoc) const
    {
      return fciLoc->nation() == segLoc->nation();
    }
  };

  class SubAreaMapper : public MapperBase
  {
  public:
    bool operator()(const Loc* fciLoc, const Loc* segLoc) const
    {
      return fciLoc->subarea() == segLoc->subarea();
    }
  };

  enum Direction
  {
    INBOUND,
    OUTBOUND,
    BIDIRECTIONAL
  };

  Direction getDirection(uint16_t furthestPoint, uint16_t curPos) const;
  Direction getDirection(uint16_t furthestPoint, const FareCompInfo* fci) const;

  bool isDirectionMatched(Direction fcDirection, Direction segDirection) const;

  void mapWith(const MapperBase& mapper,
               const ExcItin& excItin,
               const Itin& newItin);

  void printMap(std::string header) const;
  void printUnmapped(const std::vector<FareCompInfo*>& fcInfos,
                     const std::vector<TravelSeg*>& newSegs) const;

  std::map<const FareCompInfo*, const std::vector<TravelSeg*>> _fcToSegs;

  std::vector<bool> _matchedFCs;
  std::vector<bool> _matchedTvlSegs;
  std::map<const TravelSeg*, uint16_t> _tvlPtrToId;
  DiagCollector* _dc = nullptr;

  std::map<const TravelSeg*, std::set<CarrierCode>> _newTvlSegToCxr;
  std::map<const TravelSeg*, bool> _newTvlSegToTariffType;

};

} //tse
