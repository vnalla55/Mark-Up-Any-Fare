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
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/VecMap.h"

#include <algorithm>
#include <deque>
#include <map>
#include <vector>
#include <utility>

namespace tse
{
class FareMarket;
class Itin;
class PU;
class MergedFareMarket;
class FareMarketPath;

struct SpanishData
{
  using YFlexiMap = std::map<std::tuple<const LocCode, const LocCode, const CarrierCode, const CarrierCode>, MoneyAmount>;
  YFlexiMap spanishResidentAmount;
  std::set<CarrierCode> puPathValCrxList;
};

class PUPath
{
public:
  //   idx to _allPU,      fareUsageIdx,    for 2 ST of the FU: idx to _allPU
  typedef VecMap<uint8_t, std::vector<std::vector<uint8_t> > > FUToPULinkMap;
  typedef VecMap<uint8_t, FUToPULinkMap> MainTripSideTripLink;

  PUPath(const PUPath&) = delete;
  PUPath& operator=(const PUPath&) = delete;

  PUPath() = default;

  const std::vector<PU*>& puPath() const { return _puPath; }
  std::vector<PU*>& puPath() { return _puPath; }

  std::map<MergedFareMarket*, std::vector<PUPath*> >& sideTripPUPath() { return _sideTripPUPath; }
  const std::map<MergedFareMarket*, std::vector<PUPath*> >& sideTripPUPath() const
  {
    return _sideTripPUPath;
  }

  bool isMarketAssigned(const MergedFareMarket* mkt);

  void setTotalPU() { _totalPU = _allPU.size(); }
  void countTotalFC();
  uint16_t totalPU() const { return _totalPU; }
  uint16_t totalFC() const { return _totalFC; }

  const bool& itinWithinScandinavia() const { return _itinWithinScandinavia; }
  bool& itinWithinScandinavia() { return _itinWithinScandinavia; }

  std::vector<PU*>& allPU() { return _allPU; }
  const std::vector<PU*>& allPU() const { return _allPU; }

  std::deque<bool>& eoePUAvailable() { return _eoePUAvailable; }
  const std::deque<bool>& eoePUAvailable() const { return _eoePUAvailable; }

  MainTripSideTripLink& mainTripSideTripLink() { return _mainTripSideTripLink; }
  const MainTripSideTripLink& mainTripSideTripLink() const { return _mainTripSideTripLink; }

  Itin*& itin() { return _itin; }
  const Itin* itin() const { return _itin; }

  bool& shutdown() { return _shutdown; }
  const bool& shutdown() const { return _shutdown; }

  const bool& cxrFarePreferred() const { return _cxrFarePreferred; }
  bool& cxrFarePreferred() { return _cxrFarePreferred; }

  const FareMarketPath* fareMarketPath() const { return _fareMarketPath; }
  FareMarketPath*& fareMarketPath() { return _fareMarketPath; }

  const bool& isIntlCTJourneyWithOWPU() const { return _isIntlCTJourneyWithOWPU; }
  const BrandCode& getBrandCode() const { return _brandCode; }
  void setBrandCode(const BrandCode& value) { _brandCode = value; }

  bool& isIntlCTJourneyWithOWPU() { return _isIntlCTJourneyWithOWPU; }

  bool& abaTripWithOWPU() { return _abaTripWithOWPU; }
  const bool& abaTripWithOWPU() const { return _abaTripWithOWPU; }

  const uint16_t& getFlexFaresGroupId() const { return _flexFaresGroupId; }
  void setFlexFaresGroupId(const uint16_t& id) { _flexFaresGroupId = id; }

  void
  addSpanishResidentAmount(const LocCode& board, const LocCode& off, const CarrierCode& govCrx,
                           const CarrierCode& valCrx, MoneyAmount referenceFareAmount);

  MoneyAmount
  findSpanishResidentAmount(const FareMarket& fareMarket,
                            const CarrierCode& govCrx,
                            const CarrierCode& valCrx) const;

  const SpanishData::YFlexiMap&
  getSpanishResidentAmount() const;

  void setSRFApplicable(bool isApplicable) { _SRFApplicable = isApplicable; }
  bool isSRFApplicable() const { return _SRFApplicable; }

  const SpanishData* getSpanishData() const { return _spanishData; }
  void setSpanishData(SpanishData* spanishData) { _spanishData = spanishData; }

  bool operator<(const PUPath& otherPuPath) const;

private:
  std::vector<PU*> _puPath; // main trip
  bool _itinWithinScandinavia = false;
  bool _isIntlCTJourneyWithOWPU = false;
  bool _abaTripWithOWPU = false;

  bool _cxrFarePreferred = false;

  uint16_t _totalPU = 0;
  uint16_t _totalFC = 0;

  // One FareMarket might have multiple SideTrip
  // Each ST might form multiple FareMarketPath
  // Each FareMarketPath might form multiple PUPath

  std::map<MergedFareMarket*, std::vector<PUPath*> > _sideTripPUPath;

  std::deque<bool> _eoePUAvailable; // Since PU are reused in MT and ST, we need to mark each PU
  // after building FarePath using these bool flags that are
  // calculated only once

  std::vector<PU*> _allPU; // collection of ptr of main trip and side Trip PU

  MainTripSideTripLink _mainTripSideTripLink;

  bool _shutdown = false; // Short-Ckt logic of FarePathFactory sets this flag
  // to stop trying this PUPath any more

  FareMarketPath* _fareMarketPath = nullptr;
  BrandCode _brandCode;
  Itin* _itin = nullptr;

  uint16_t _flexFaresGroupId = 0;

  bool _SRFApplicable = false;
  SpanishData* _spanishData = 0;
};
} // tse namespace
