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

#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Util/FlatMap.h"

#include <map>

namespace tse
{
class MergedFareMarket;
class PaxType;
class PricingTrx;
class PUPath;

class FareMarketPath
{
  enum class Thru : uint8_t
  {
    NOT_CALCULATED,
    YES,
    NO
  };

public:
  FareMarketPath() = default;

  FareMarketPath(const FareMarketPath&) = delete;
  void operator=(const FareMarketPath&) = delete;

  const std::vector<MergedFareMarket*>& fareMarketPath() const { return _fareMarketPath; }
  std::vector<MergedFareMarket*>& fareMarketPath() { return _fareMarketPath; }

  const std::map<MergedFareMarket*, std::vector<FareMarketPath*>>& sideTrips() const
  {
    return _sideTrips;
  }
  std::map<MergedFareMarket*, std::vector<FareMarketPath*>>& sideTrips() { return _sideTrips; }

  BrandCode getBrandCode() const { return _brandCode; }
  void setBrandCode(BrandCode value) { _brandCode = std::move(value); }

  std::vector<PUPath*>& puPaths() { return _puPaths; }
  const std::vector<PUPath*>& puPaths() const { return _puPaths; }

  uint16_t throughFarePrecedenceRank() const { return _throughFarePrecedenceRank; }

  MoneyAmount firstFareAmt(const PaxType* paxType) const
  {
    const auto i = _paxFirstFareAmt.find(paxType);
    if (UNLIKELY(i == _paxFirstFareAmt.end()))
      return std::numeric_limits<MoneyAmount>::max();

    return i->second;
  }

  bool thruPricing(PricingTrx* trx)
  {
    if (_thruPricing != Thru::NOT_CALCULATED)
      return _thruPricing == Thru::YES;

    return thruPricingCalculate(trx);
  }

  void calculateFirstFareAmt(const std::vector<PaxType*>& paxTypes);
  void calculateThroughFarePrecedenceRank();
  void calculateThroughFarePrecedenceRankOld();
  bool isValidForIbf() const;

private:
  std::vector<MergedFareMarket*> _fareMarketPath;

  // We may have vector of side trips per FareMarket [see FareMarket.h]
  // Each side trip can have multiple fareMarketPaths thats why data type
  // of the map is std:vector<std::vector<FareMarketPath*>>
  //
  // One FareMarket ===> vector of SideTrip, which is vector<TravelSeg*>
  // One SideTrip ===> vector of FareMarketPath
  //
  // key is the Index of the FareMarket in the _fareMarketPath to be able
  // to associate a sideTrip with a FareMarket of _fareMarketPath.

  std::map<MergedFareMarket*, std::vector<FareMarketPath*>> _sideTrips;

  std::vector<PUPath*> _puPaths;
  FlatMap<const PaxType*, MoneyAmount> _paxFirstFareAmt;

  uint16_t _throughFarePrecedenceRank = 0;
  Thru _thruPricing = Thru::NOT_CALCULATED;
  BrandCode _brandCode;

  bool thruPricingCalculate(PricingTrx* trx);
};

} // tse namespace

