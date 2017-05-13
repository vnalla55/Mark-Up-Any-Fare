// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
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

#include "DataModel/ItinIndex.h"
#include "Fares/RoutingController.h"

#include <map>

namespace tse
{
class BCETuning;
class FareMarket;
class MileageInfo;

class ShpBitValidationCollector
{
public:
  class SharedElements
  {
  public:
    SharedElements() : _mileageInfoOutbound(nullptr), _mileageInfoInbound(nullptr) {}
    MileageInfo* getMileageInfOutbound() { return _mileageInfoOutbound; }
    MileageInfo* getMileageInfInbound() { return _mileageInfoInbound; }
    void setMileageInfOutbound(MileageInfo* info) { _mileageInfoOutbound = info; }
    void setMileageInfInbound(MileageInfo* info) { _mileageInfoInbound = info; }
    ShoppingRtgMap& getRoutingMap() { return _routingMap; }
    std::vector<BCETuning>& getBCEData() { return _bceData; }

  private:
    MileageInfo* _mileageInfoOutbound;
    MileageInfo* _mileageInfoInbound;
    ShoppingRtgMap _routingMap;
    std::vector<BCETuning> _bceData;
  };

  typedef std::map<uint32_t, SharedElements> BitValidationShareMap;

  class FMValidationSharedData
  {
  public:
    void updateFareMarketData(FareMarket* fareMarket, const uint32_t bit);
    void collectFareMarketData(FareMarket* fareMarket, const uint32_t bit);
    ShoppingRtgMap* getRoutingMapForBit(const uint32_t bit);
    std::vector<BCETuning>* getBCEData(const uint32_t bit);

  protected:
    SharedElements& getBitSharedData(uint32_t bit) { return _sharedData[bit]; }
    BitValidationShareMap& getSharedData() { return _sharedData; }

  private:
    BitValidationShareMap _sharedData;
  };

  typedef std::map<const FareMarket*, FMValidationSharedData> FMValidationShareMap;
  typedef std::map<ItinIndex::Key, FMValidationShareMap> CxrBitValidationShareMap;

  ShpBitValidationCollector() {};
  virtual ~ShpBitValidationCollector() {};

  FMValidationSharedData*
  getFMSharedData(const ItinIndex::Key carrierKey, const FareMarket* fareMarket);

protected:
  CxrBitValidationShareMap& getCxrValidationShareMap() { return _cxrValidationShareMap; }

private:
  CxrBitValidationShareMap _cxrValidationShareMap;
};

} /* namespace tse */
