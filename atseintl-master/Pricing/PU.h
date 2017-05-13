//-------------------------------------------------------------------
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

#include "Common/TseCodeTypes.h"
#include "DataModel/PricingUnit.h"

#include <algorithm>
#include <set>
#include <vector>

namespace tse
{

class MergedFareMarket;

class PU
{

public:
  enum GovCarrier
  {
    UNKNOWN, // may have to wait until the fare-combination is built
    // specially for Multi-Gov-Carrier FareMakret
    ALL_AA_CARRIER,
    SAME_CARRIER,
    MULTI_CARRIER
  };

  PU(const PU&) = delete;
  PU& operator=(const PU&) = delete;

  PU() = default;
  // virtual ~PU() {} //@TODO clean up the Vector

  bool isMarketAssigned(const MergedFareMarket* fmkt) const
  {
    return std::any_of(_fareMarket.begin(),
                       _fareMarket.end(),
                       [fmkt](MergedFareMarket* mfm)
                       { return mfm == fmkt; });
  }

  const std::vector<MergedFareMarket*>& fareMarket() const { return _fareMarket; }
  std::vector<MergedFareMarket*>& fareMarket() { return _fareMarket; }

  const uint16_t ojLeg1FCCount() const { return _ojLeg1FCCount; }
  uint16_t& ojLeg1FCCount() { return _ojLeg1FCCount; }

  PricingUnit::Type& puType() { return _puType; }
  PricingUnit::Type puType() const { return _puType; }

  PricingUnit::PUSubType& puSubType() { return _puSubType; }
  const PricingUnit::PUSubType& puSubType() const { return _puSubType; }

  bool& sameNationOJ() { return _sameNationOJ; }
  const bool& sameNationOJ() const { return _sameNationOJ; }

  bool& sameNationOrigSurfaceOJ() { return _sameNationOrigSurfaceOJ; }
  const bool& sameNationOrigSurfaceOJ() const { return _sameNationOrigSurfaceOJ; }

  bool& allowNOJInZone210() { return _allowNOJInZone210; }
  const bool& allowNOJInZone210() const { return _allowNOJInZone210; }

  GovCarrier& puGovCarrier() { return _puGovCarrier; }
  const GovCarrier& puGovCarrier() const { return _puGovCarrier; }

  PricingUnit::OJSurfaceStatus& ojSurfaceStatus() { return _ojSurfaceStatus; }
  const PricingUnit::OJSurfaceStatus& ojSurfaceStatus() const { return _ojSurfaceStatus; }

  GeoTravelType& geoTravelType() { return _geoTravelType; }
  const GeoTravelType& geoTravelType() const { return _geoTravelType; }

  const TravelSeg* turnAroundPoint() const { return _turnAroundPoint; }
  const TravelSeg*& turnAroundPoint() { return _turnAroundPoint; }

  const std::vector<Directionality>& fareDirectionality() const { return _fareDirectionality; }
  std::vector<Directionality>& fareDirectionality() { return _fareDirectionality; }

  const bool& hasSideTrip() const { return _hasSideTrip; }
  bool& hasSideTrip() { return _hasSideTrip; }

  const bool& isCompleteJourney() const { return _isCompleteJourney; }
  bool& isCompleteJourney() { return _isCompleteJourney; }

  const bool& noPUToEOE() const { return _noPUToEOE; }
  bool& noPUToEOE() { return _noPUToEOE; }

  const bool& possibleSideTripPU() const { return _possibleSideTripPU; }
  bool& possibleSideTripPU() { return _possibleSideTripPU; }

  const bool& itinWithinScandinavia() const { return _itinWithinScandinavia; }
  bool& itinWithinScandinavia() { return _itinWithinScandinavia; }

  const GeoTravelType& itinGeoTvlType() const { return _itinGeoTvlType; }
  GeoTravelType& itinGeoTvlType() { return _itinGeoTvlType; }

  void setFCCount() { _fcCount = static_cast<uint16_t>(_fareMarket.size()); }
  const uint16_t& fcCount() const { return _fcCount; }
  uint16_t& fcCount() { return _fcCount; }

  const bool& cxrFarePreferred() const { return _cxrFarePreferred; }
  bool& cxrFarePreferred() { return _cxrFarePreferred; }

  const bool& invalidateYYForTOJ() const { return _invalidateYYForTOJ; }
  bool& invalidateYYForTOJ() { return _invalidateYYForTOJ; }

  const std::vector<CarrierCode>& invalidCxrForOJ() const { return _invalidCxrForOJ; }
  std::vector<CarrierCode>& invalidCxrForOJ() { return _invalidCxrForOJ; }

  const bool& inDiffCntrySameSubareaForOOJ() const { return _inDiffCntrySameSubareaForOOJ; }
  bool& inDiffCntrySameSubareaForOOJ() { return _inDiffCntrySameSubareaForOOJ; }

  const bool& specialEuropeanDoubleOJ() const { return _specialEuropeanDoubleOJ; }
  bool& specialEuropeanDoubleOJ() { return _specialEuropeanDoubleOJ; }

  const uint16_t& getFlexFaresGroupId() const { return _flexFaresGroupId; }
  void setFlexFaresGroupId(const uint16_t& id) { _flexFaresGroupId = id; }

  const bool isSpecialOpenJaw() const { return _specialOpenJaw; }
  void setSpecialOpenJaw(bool value) { _specialOpenJaw = value; }
  
  const std::set<PU*>& intlOJToOW() const { return _intlOJToOW; }
  std::set<PU*>& intlOJToOW() { return _intlOJToOW; }

  bool operator<(const PU& rhs) const
  {
    if (_fareMarket < rhs._fareMarket)
      return true;

    if (_fareMarket == rhs._fareMarket)
    {
      if (_puType < rhs._puType)
        return true;

      if (_puType == rhs._puType)
      {
        if (UNLIKELY(_puSubType < rhs._puSubType))
          return true;

        if (LIKELY(_puSubType == rhs._puSubType))
        {
          if (_fareDirectionality < rhs._fareDirectionality)
            return true;
        }
      }
    }

    return false;
  }

  struct PUPtrCmp
  {
    bool operator()(const PU* p1, const PU* p2) const { return *p1 < *p2; }
  };

private:
  std::vector<MergedFareMarket*> _fareMarket;
  std::vector<Directionality> _fareDirectionality;

  PricingUnit::Type _puType = PricingUnit::Type::UNKNOWN;
  PricingUnit::PUSubType _puSubType = PricingUnit::UNKNOWN_SUBTYPE;

  bool _sameNationOJ = false;
  bool _sameNationOrigSurfaceOJ = false;
  bool _allowNOJInZone210 = false;
  PricingUnit::OJSurfaceStatus _ojSurfaceStatus = PricingUnit::NOT_CHECKED;
  GeoTravelType _geoTravelType = GeoTravelType::UnknownGeoTravelType;

  const TravelSeg* _turnAroundPoint = nullptr;

  GovCarrier _puGovCarrier = UNKNOWN; // only within PU

  bool _hasSideTrip = false;
  bool _isCompleteJourney = false;
  bool _noPUToEOE = false; // to facilitate comb-validation
  bool _possibleSideTripPU = false;
  uint16_t _fcCount = 0;

  uint16_t _ojLeg1FCCount = 0;

  GeoTravelType _itinGeoTvlType = GeoTravelType::UnknownGeoTravelType; // All the Itins (MIP) will have same value
  bool _itinWithinScandinavia = false; // All the Itins (MIP) will have same value

  bool _cxrFarePreferred = false;
  bool _invalidateYYForTOJ = false; // to invalidate YY fares for TOJ (across 2 areas)
  std::vector<CarrierCode> _invalidCxrForOJ; // invalid carrierCodes for TOJ (multi
                                             // GoverningCarriers)
  bool _inDiffCntrySameSubareaForOOJ = false; // allow NL fare for international
  // OOJ (in diff countries, same subArea

  bool _specialEuropeanDoubleOJ = false; // allow SP fare for international European DoubleOJ.

  uint16_t _flexFaresGroupId = 0;
  bool _specialOpenJaw = false;
  std::set<PU*> _intlOJToOW;
};
} // tse namespace
