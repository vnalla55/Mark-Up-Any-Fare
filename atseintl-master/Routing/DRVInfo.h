#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Routing/RoutingInfo.h"

#include <vector>

namespace tse
{
using DRVCityCarrier = std::vector<TravelRoute::CityCarrier>;

class DRVInfo : public RoutingInfo
{
public:
  bool drvInfoStatus() const { return _drvInfoStatus; }
  bool& drvInfoStatus() { return _drvInfoStatus; }
  bool getRoutingFailed() const { return _getRoutingFailed; }
  bool& getRoutingFailed() { return _getRoutingFailed; }
  bool missingCityIndexInvalid() const { return _missingCityIndexInvalid; }
  bool& missingCityIndexInvalid() { return _missingCityIndexInvalid; }
  bool notSameCarrier() const { return _notSameCarrier; }
  bool& notSameCarrier() { return _notSameCarrier; }
  bool notSameCountry() const { return _notSameCountry; }
  bool& notSameCountry() { return _notSameCountry; }
  bool missingCityOrigDest() const { return _missingCityOrigDest; }
  bool& missingCityOrigDest() { return _missingCityOrigDest; }
  bool notQualifiedForDRV() const { return _notQualifiedForDRV; }
  bool& notQualifiedForDRV() { return _notQualifiedForDRV; }
  bool noFareMarket() const { return _noFareMarket; }
  bool& noFareMarket() { return _noFareMarket; }
  bool flightStopMarket() const { return _flightStopMarket; }
  bool& flightStopMarket() { return _flightStopMarket; }
  bool noPaxTypeFares() const { return _noPaxTypeFares; }
  bool& noPaxTypeFares() { return _noPaxTypeFares; }
  const FareClassCode& fareClass() const { return _fareClass; }
  FareClassCode& fareClass() { return _fareClass; }
  const CurrencyCode& currency() const { return _currency; }
  CurrencyCode& currency() { return _currency; }
  const MoneyAmount& fareAmount() const { return _fareAmount; }
  MoneyAmount& fareAmount() { return _fareAmount; }
  const VendorCode& vendor() const { return _vendor; }
  VendorCode& vendor() { return _vendor; }
  const GlobalDirection& global() const { return _global; }
  GlobalDirection& global() { return _global; }
  const CarrierCode& localGovCxr() const { return _localGovCxr; }
  CarrierCode& localGovCxr() { return _localGovCxr; }
  const RoutingNumber& routingNumber() const { return _routingNumber; }
  RoutingNumber& routingNumber() { return _routingNumber; }
  TariffNumber& routingTariff1() { return _routingTariff1; }
  const TariffCode& tariffCode1() const { return _tariffCode1; }
  TariffCode& tariffCode1() { return _tariffCode1; }
  const TariffNumber& routingTariff2() const { return _routingTariff2; }
  TariffNumber& routingTariff2() { return _routingTariff2; }
  const TariffCode& tariffCode2() const { return _tariffCode2; }
  TariffCode& tariffCode2() { return _tariffCode2; }

  const DRVCityCarrier& localCityCarrier() const { return _localCityCarrier; }
  DRVCityCarrier& localCityCarrier() { return _localCityCarrier; }

  DRVCityCarrier& intlCityCarrier() { return _intlCityCarrier; }

private:
  bool _drvInfoStatus = false;
  bool _getRoutingFailed = false;
  bool _missingCityIndexInvalid = false;
  bool _notSameCarrier = false;
  bool _notSameCountry = false;
  bool _missingCityOrigDest = false;
  bool _notQualifiedForDRV = false;
  bool _noFareMarket = false;
  bool _noPaxTypeFares = false;
  bool _flightStopMarket = false;
  FareClassCode _fareClass;
  CurrencyCode _currency;
  MoneyAmount _fareAmount = 0;
  VendorCode _vendor;
  GlobalDirection _global = GlobalDirection::NO_DIR;
  CarrierCode _localGovCxr;
  RoutingNumber _routingNumber;
  TariffNumber _routingTariff1 = 0;
  TariffCode _tariffCode1;
  TariffNumber _routingTariff2 = 0;
  TariffCode _tariffCode2;
  DRVCityCarrier _localCityCarrier;
  DRVCityCarrier _intlCityCarrier;
};
}
