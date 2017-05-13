//-------------------------------------------------------------------
//
//  File:        MergedFareMarket.h
//  Created:     Sep 23, 2004
//  Design:      Mohammad Hossan
//  Authors:
//
//  Description: List of FareMarkets per O&D
//
//  Updates:
//
//  Copyright Sabre 2004
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

#include "Common/SmallBitSet.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/IbfAvailabilityStatusAccumulators.h"

#include <vector>

namespace tse
{
class FareMarket;
class Loc;
class PaxType;
class PaxTypeFare;
class TravelSeg;

class MergedFareMarket final
{
  struct FirstFare
  {
    PaxTypeFare* ptf = nullptr;
    MoneyAmount totalNucAmount = 0.0;
  };

public:
  enum class Tag2FareIndicator : uint8_t
  {
    Absent, // For US/CA FareMarket we need to know
    Present, // whether we have Tag-2 Fare
    NonIssue // or not to build RT/CT/OJ PU
  };

  enum CxrFareType
  {
    UNKNOWN = 0x0000,
    OB_OW_TRIP_TYPE = 0x0001,
    OB_OJRC_TRIP_TYPE = 0x0002,
    OB_ALL_TRIP_TYPE = 0x0003,
    IB_OW_TRIP_TYPE = 0x0004,
    IB_OJRC_TRIP_TYPE = 0x0008,
    IB_ALL_TRIP_TYPE = 0x000C,
    BothDir_OW_TRIP_TYPE = 0x0005,
    BothDir_OJRC_TRIP_TYPE = 0x000A,
    BothDir_ALL_TRIP_TYPE = 0x000F,
    OW_TRIP_TYPE = 0x0004,
    OJRC_TRIP_TYPE = 0x0008,
    ALL_TRIP_TYPE = 0x000C
  };

  typedef SmallBitSet<uint16_t, CxrFareType> CxrFareTypeBitSet;

  MergedFareMarket() = default;
  MergedFareMarket(const MergedFareMarket&) = delete;
  MergedFareMarket& operator=(MergedFareMarket&) = delete;

  const Loc*& origin() { return _origin; }
  const Loc* origin() const { return _origin; }

  const Loc*& destination() { return _destination; }
  const Loc* destination() const { return _destination; }

  LocCode& boardMultiCity() { return _boardMultiCity; }
  const LocCode& boardMultiCity() const { return _boardMultiCity; }

  LocCode& offMultiCity() { return _offMultiCity; }
  const LocCode& offMultiCity() const { return _offMultiCity; }

  GlobalDirection& globalDirection() { return _globalDirection; }
  const GlobalDirection globalDirection() const { return _globalDirection; }

  std::vector<CarrierCode>& governingCarrier() { return _governingCarrier; }
  const std::vector<CarrierCode>& governingCarrier() const { return _governingCarrier; }

  CarrierCode& outboundGovCxr() { return _outboundGovCxr; }
  const CarrierCode outboundGovCxr() const { return _outboundGovCxr; }

  CarrierCode& inboundGovCxr() { return _inboundGovCxr; }
  const CarrierCode inboundGovCxr() const { return _inboundGovCxr; }

  GeoTravelType& geoTravelType() { return _geoTravelType; }
  const GeoTravelType geoTravelType() const { return _geoTravelType; }

  bool& cxrFarePreferred() { return _cxrFarePreferred; }
  const bool cxrFarePreferred() const { return _cxrFarePreferred; }

  bool& cxrFareExist() { return _cxrFareExist; }
  const bool& cxrFareExist() const { return _cxrFareExist; }

  BrandCode& brandCode() { return _brandCode; }
  const BrandCode& brandCode() const { return _brandCode; }

  bool& collectRec2Cat10() { return _collectRec2Cat10; }
  const bool& collectRec2Cat10() const { return _collectRec2Cat10; }

  std::vector<TravelSeg*>& travelSeg() { return _travelSeg; }

  const std::vector<TravelSeg*>& travelSeg() const { return _travelSeg; }

  std::vector<std::vector<TravelSeg*> >& sideTripTravelSeg() { return _sideTripTravelSeg; }
  const std::vector<std::vector<TravelSeg*> >& sideTripTravelSeg() const
  {
    return _sideTripTravelSeg;
  }

  std::vector<FareMarket*>& mergedFareMarket() { return _mergedFareMarket; }
  const std::vector<FareMarket*>& mergedFareMarket() const { return _mergedFareMarket; }

  SmallBitSet<uint8_t, FMTravelBoundary>& travelBoundary() { return _travelBoundary; }
  const SmallBitSet<uint8_t, FMTravelBoundary>& travelBoundary() const
  {
    return _travelBoundary;
  }

  uint16_t getStartSegNum() const { return _startSeg; }
  uint16_t getEndSegNum() const { return _endSeg; }
  void setSegNums(uint16_t start, uint16_t end)
  {
    _startSeg = start;
    _endSeg = end;
  }

  FMDirection& direction() { return _direction; }
  const FMDirection& direction() const { return _direction; }

  Tag2FareIndicator& tag2FareIndicator() { return _tag2FareIndicator; }
  const Tag2FareIndicator tag2FareIndicator() const { return _tag2FareIndicator; }

  std::map<const PaxType*, std::map<FareType, CxrFareTypeBitSet> >& allCxrFareTypes_old()
  {
    return _allCxrFareTypes_old;
  }
  const std::map<const PaxType*, std::map<FareType, CxrFareTypeBitSet> >& allCxrFareTypes_old() const
  {
    return _allCxrFareTypes_old;
  }

  std::map<const PaxType*, std::map<FareType, std::map<CarrierCode, CxrFareTypeBitSet> > >& allCxrFareTypes()
  {
    return _allCxrFareTypes;
  }
  const std::map<const PaxType*, std::map<FareType, std::map<CarrierCode, CxrFareTypeBitSet> > >& allCxrFareTypes() const
  {
    return _allCxrFareTypes;
  }

  const bool hardPassExists() const { return _hardPassExists; }
  bool& hardPassExists() { return _hardPassExists; }

  IbfErrorMessage getIbfErrorMessage() const { return _IbfErrorMessageChoiceAcc.getStatus(); }

  void updateIbfErrorMessage(IbfErrorMessage status)
  {
    _IbfErrorMessageChoiceAcc.updateStatus(status);
  }

  // We need to use the above one to pass a SmallBitSet if we use more than one criteria. The one
  // below is only for
  // directionality
  //

  bool cxrFareTypeExists(const PaxType* paxType,
                         const FareType& fareType,
                         const CxrFareTypeBitSet& cxrFareTypeBitSet,
                         const CarrierCode & valCxr) const;


  bool cxrFareTypeExists_old(const PaxType* paxType,
                             const FareType& fareType,
                             const CxrFareTypeBitSet& cxrFareTypeBitSet) const;

  const FirstFare* firstFare(const PaxType* paxType) const
  {
    const auto it = _paxFirstFare.find(paxType);
    if (UNLIKELY(it == _paxFirstFare.end()))
      return nullptr;

    return &it->second;
  }

  void findFirstFare(const std::vector<PaxType*>& paxTypes);

private:
  const Loc* _origin = nullptr;
  const Loc* _destination = nullptr;

  LocCode _boardMultiCity;
  LocCode _offMultiCity;

  GlobalDirection _globalDirection = GlobalDirection::XX;
  GeoTravelType _geoTravelType = GeoTravelType::UnknownGeoTravelType;

  Tag2FareIndicator _tag2FareIndicator = Tag2FareIndicator::NonIssue;

  std::vector<CarrierCode> _governingCarrier;

  CarrierCode _inboundGovCxr;
  CarrierCode _outboundGovCxr;

  bool _cxrFarePreferred = false;
  bool _cxrFareExist = false;
  std::map<const PaxType*, std::map<FareType, CxrFareTypeBitSet> > _allCxrFareTypes_old; // will be removed with fallback
  std::map<const PaxType*, std::map<FareType, std::map<CarrierCode, CxrFareTypeBitSet> > > _allCxrFareTypes;

  BrandCode _brandCode;
  bool _hardPassExists = false;

  std::vector<TravelSeg*> _travelSeg;
  std::vector<std::vector<TravelSeg*>> _sideTripTravelSeg;

  // Each elem could be a pointer to the result of merge of FareMarkets
  // or just the pointer to the original FareMarket if there is only one
  // of that priority
  std::vector<FareMarket*> _mergedFareMarket;

  SmallBitSet<uint8_t, FMTravelBoundary> _travelBoundary;

  FMDirection _direction = FMDirection::UNKNOWN;

  uint16_t _startSeg = 0;
  uint16_t _endSeg = 0;

  // no need to collect Rec2Cat10 info of the PaxTypeFare of the
  // FareMarket which will not be used in building FareMarketPath
  //
  bool _collectRec2Cat10 = false;

  std::map<const PaxType*, FirstFare> _paxFirstFare;

  IbfErrorMessageChoiceAcc _IbfErrorMessageChoiceAcc;
};
} // tse namespace
