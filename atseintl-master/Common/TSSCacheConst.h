#pragma once

#include <string>

namespace tse
{
namespace tsscache
{
enum TSSCacheEnum
{
  IsInLocInd
  , IsAnActualPaxInTrxInd
  , DateInd
  , RuleItemInd
  , GFRInd
  , FNInd
  , CorpIdInd
  , GeneralRuleAppInd
  , GeneralRuleAppTariffRuleInd
  , CurrencyInd
  , CarrierPreferenceInd
  , LocInd
  , FareClassAppInd
  , PaxTypeInd
  , FareTypeMatrixInd
  , MarkupBySecondSellerIdInd
  , MarkupBySecurityItemNoInd
  , MarkupByPccInd
  , GeoRuleItemInd
  , MultiTransportCityCodeInd
  , SamePointInd
  , DSTInd
  , TariffInhibitInd
  , CarrierCombinationInd
  , TaxCarrierApplInd
  , EndOnEndInd
  , TariffRuleRestInd
  , TariffCrossRefInd
  , FareByRuleItemInd
  , RoutingInd
  , DateOverrideRuleItemInd
  , NUCFirstInd
  , ZoneInd
  , BaseFareRuleInd
  , SectorSurchargeInd
  , ATPResNationZonesInd
  , MultiAirportCityInd
  , FCLimitationInd
  , GlobalDirInd
  , GlobalDirSegInd
  , FareByRuleAppInd
  , FareByRuleCtrlInd
  , NegFareSecurityInd
  , VendorTypeInd
  , TSIInd
  , MileageInd
  , CarrierFlightInd
  , BookingCodeExceptionInd
  , BookingCodeException2Ind
  , CabinInd
  , AddOnCombFareClassInd
  , AddonFareInd
  , CombinabilityRuleInfoInd
  , CarrierApplicationInfoInd
  , MarkupSecFilterInd
  , MarriedCabinInd
  , UtcOffsetDifferenceInd
  , SvcFeesSecurityInd
  , TaxSpecConfigInd
  , MileageSurchExceptInd
  , PaxTypeMatrixInd
  , MultiCityAirportInd
  , OpenJawRuleInd
  , CircleTripRuleItemInd
  , OpenJawRestrictionInd
  , DBEGlobalClassInd
  , FareClassRestRuleInd
  , IndustryFareBasisModInd
  , TaxCodeRegInd
  , SvcFeesCxrResultingFCLInfoInd
  , NationInd
  , AllNationsInd
  , MultiTransportCityLocCodeInd
  , MultiTransportCityVectorInd
  , SalesRestrictionInd
  , CurrencyConversionRoundInd
  , CurrencyConverterRoundByRuleInd
  , IsInZoneInd
  , FareFocusSecurityInd
  , FareFocusRuleInd
  , IsInLocObjInd
  , FareClassAppByTravelDateInd
  , RBDByCabinInd
  , GeneralRuleAppByTvlDateInd
  , GeneralRuleAppTariffRuleByTvlDateInd
  , NUMBER_OF_CACHES
};

inline const std::string& cacheNameByIndex(TSSCacheEnum index)
{
  static const std::string TSSCacheNames[] = {"IsInLoc",
                                              "IsAnActualPaxInTrx",
                                              "Date",
                                              "RuleItem",
                                              "GFR",
                                              "FN",
                                              "CorpId",
                                              "GeneralRuleApp",
                                              "GeneralRuleAppTariffRule",
                                              "Currency",
                                              "CarrierPreference",
                                              "Loc",
                                              "FareClassApp",
                                              "PaxType",
                                              "FareTypeMatrix",
                                              "MarkupBySecondSellerId",
                                              "MarkupBySecurityItemNo",
                                              "MarkupByPcc",
                                              "GeoRuleItem",
                                              "MultiTransportCityCode",
                                              "SamePoint",
                                              "DST",
                                              "TariffInhibit",
                                              "CarrierCombination",
                                              "TaxCarrierAppl",
                                              "EndOnEnd",
                                              "TariffRuleRest",
                                              "TariffCrossRef",
                                              "FareByRuleItem",
                                              "Routing",
                                              "DateOverrideRuleItem",
                                              "NUCFirst",
                                              "Zone",
                                              "BaseFareRule",
                                              "SectorSurcharge",
                                              "ATPResNationZones",
                                              "MultiAirportCity",
                                              "FCLimitation",
                                              "GlobalDir",
                                              "GlobalDirSeg",
                                              "FareByRuleApp",
                                              "FareByRuleCtrl",
                                              "NegFareSecurity",
                                              "VendorType",
                                              "TSI",
                                              "Mileage",
                                              "CarrierFlight",
                                              "BookingCodeException",
                                              "BookingCodeException2",
                                              "Cabin",
                                              "AddOnCombFareClass",
                                              "AddonFare",
                                              "CombinabilityRuleInfo",
                                              "CarrierApplicationInfo",
                                              "MarkupSecFilter",
                                              "MarriedCabin",
                                              "UtcOffsetDifference",
                                              "SvcFeesSecurity",
                                              "TaxSpecConfig",
                                              "MileageSurchExcept",
                                              "PaxTypeMatrix",
                                              "MultiCityAirport",
                                              "OpenJawRule",
                                              "CircleTripRuleItem",
                                              "OpenJawRestriction",
                                              "DBEGlobalClass",
                                              "FareClassRestRule",
                                              "IndustryFareBasisMod",
                                              "TaxCodeReg",
                                              "SvcFeesCxrResultingFCLInfo",
                                              "Nation",
                                              "AllNations",
                                              "MultiTransportCityLocCode",
                                              "MultiTransportCityVector",
                                              "SalesRestriction",
                                              "CurrencyConversionRound",
                                              "CurrencyConverterRoundByRule",
                                              "IsInZone",
                                              "FareFocusSecurity",
                                              "FareFocusRule",
                                              "IsInLocObj",
                                              "FareClassAppByTravelDate",
                                              "RBDByCabin",
                                              "GeneralRuleAppByTvlDate",
                                              "GeneralRuleAppTariffRuleByTvlDate",
                                              "This name should not appear"};

  return TSSCacheNames[index];
}
}// tsscache
}// tse
