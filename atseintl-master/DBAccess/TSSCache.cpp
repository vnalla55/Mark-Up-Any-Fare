#include "DBAccess/TSSCache.h"

namespace tse
{

namespace tsscache
{

TSSCacheEnum RuleItemEntry::_cacheIndex(RuleItemInd);
TSSCacheEnum GFREntry::_cacheIndex(GFRInd);
TSSCacheEnum FNEntry::_cacheIndex(FNInd);
TSSCacheEnum CorpIdEntry::_cacheIndex(CorpIdInd);
TSSCacheEnum GeneralRuleAppEntry::_cacheIndex(GeneralRuleAppInd);
TSSCacheEnum GeneralRuleAppTariffRuleEntry::_cacheIndex(GeneralRuleAppTariffRuleInd);
TSSCacheEnum CurrencyEntry::_cacheIndex(CurrencyInd);
TSSCacheEnum CarrierPreferenceEntry::_cacheIndex(CarrierPreferenceInd);
TSSCacheEnum LocEntry::_cacheIndex(LocInd);
TSSCacheEnum FareClassAppEntry::_cacheIndex(FareClassAppInd);
TSSCacheEnum PaxTypeEntry::_cacheIndex(PaxTypeInd);
TSSCacheEnum FareTypeMatrixEntry::_cacheIndex(FareTypeMatrixInd);
TSSCacheEnum MarkupBySecondSellerIdEntry::_cacheIndex(MarkupBySecondSellerIdInd);
TSSCacheEnum MarkupBySecurityItemNoEntry::_cacheIndex(MarkupBySecurityItemNoInd);
TSSCacheEnum MarkupByPccEntry::_cacheIndex(MarkupByPccInd);
TSSCacheEnum GeoRuleItemEntry::_cacheIndex(GeoRuleItemInd);
TSSCacheEnum MultiTransportCityCodeEntry::_cacheIndex(MultiTransportCityCodeInd);
TSSCacheEnum SamePointEntry::_cacheIndex(SamePointInd);
TSSCacheEnum DSTEntry::_cacheIndex(DSTInd);
TSSCacheEnum TariffInhibitEntry::_cacheIndex(TariffInhibitInd);
TSSCacheEnum CarrierCombinationEntry::_cacheIndex(CarrierCombinationInd);
TSSCacheEnum TaxCarrierApplEntry::_cacheIndex(TaxCarrierApplInd);
TSSCacheEnum EndOnEndEntry::_cacheIndex(EndOnEndInd);
TSSCacheEnum TariffRuleRestEntry::_cacheIndex(TariffRuleRestInd);
TSSCacheEnum TariffCrossRefEntry::_cacheIndex(TariffCrossRefInd);
TSSCacheEnum FareByRuleItemEntry::_cacheIndex(FareByRuleItemInd);
TSSCacheEnum RoutingEntry::_cacheIndex(RoutingInd);
TSSCacheEnum DateOverrideRuleItemEntry::_cacheIndex(DateOverrideRuleItemInd);
TSSCacheEnum NUCFirstEntry::_cacheIndex(NUCFirstInd);
TSSCacheEnum ZoneEntry::_cacheIndex(ZoneInd);
TSSCacheEnum BaseFareRuleEntry::_cacheIndex(BaseFareRuleInd);
TSSCacheEnum SectorSurchargeEntry::_cacheIndex(SectorSurchargeInd);
TSSCacheEnum ATPResNationZonesEntry::_cacheIndex(ATPResNationZonesInd);
TSSCacheEnum MultiAirportCityEntry::_cacheIndex(MultiAirportCityInd);
TSSCacheEnum FCLimitationEntry::_cacheIndex(FCLimitationInd);
TSSCacheEnum GlobalDirEntry::_cacheIndex(GlobalDirInd);
TSSCacheEnum GlobalDirSegEntry::_cacheIndex(GlobalDirSegInd);
TSSCacheEnum FareByRuleAppEntry::_cacheIndex(FareByRuleAppInd);
TSSCacheEnum FareByRuleCtrlEntry::_cacheIndex(FareByRuleCtrlInd);
TSSCacheEnum NegFareSecurityEntry::_cacheIndex(NegFareSecurityInd);
TSSCacheEnum VendorTypeEntry::_cacheIndex(VendorTypeInd);
TSSCacheEnum TSIEntry::_cacheIndex(TSIInd);
TSSCacheEnum MileageEntry::_cacheIndex(MileageInd);
TSSCacheEnum CarrierFlightEntry::_cacheIndex(CarrierFlightInd);
TSSCacheEnum BookingCodeExceptionEntry::_cacheIndex(BookingCodeExceptionInd);
TSSCacheEnum BookingCodeException2Entry::_cacheIndex(BookingCodeException2Ind);
TSSCacheEnum CabinEntry::_cacheIndex(CabinInd);
TSSCacheEnum AddonCombFareClassEntry::_cacheIndex(AddOnCombFareClassInd);
TSSCacheEnum AddonFareEntry::_cacheIndex(AddonFareInd);
TSSCacheEnum CombinabilityRuleInfoEntry::_cacheIndex(CombinabilityRuleInfoInd);
TSSCacheEnum CarrierApplicationInfoEntry::_cacheIndex(CarrierApplicationInfoInd);
TSSCacheEnum MarkupSecFilterEntry::_cacheIndex(MarkupSecFilterInd);
TSSCacheEnum MarriedCabinEntry::_cacheIndex(MarriedCabinInd);
TSSCacheEnum UtcOffsetDifferenceEntry::_cacheIndex(UtcOffsetDifferenceInd);
TSSCacheEnum SvcFeesSecurityEntry::_cacheIndex(SvcFeesSecurityInd);
TSSCacheEnum TaxSpecConfigEntry::_cacheIndex(TaxSpecConfigInd);
TSSCacheEnum MileageSurchExceptEntry::_cacheIndex(MileageSurchExceptInd);
TSSCacheEnum PaxTypeMatrixEntry::_cacheIndex(PaxTypeMatrixInd);
TSSCacheEnum MultiCityAirportEntry::_cacheIndex(MultiCityAirportInd);
TSSCacheEnum OpenJawRuleEntry::_cacheIndex(OpenJawRuleInd);
TSSCacheEnum CircleTripRuleItemEntry::_cacheIndex(CircleTripRuleItemInd);
TSSCacheEnum OpenJawRestrictionEntry::_cacheIndex(OpenJawRestrictionInd);
TSSCacheEnum DBEGlobalClassEntry::_cacheIndex(DBEGlobalClassInd);
TSSCacheEnum FareClassRestRuleEntry::_cacheIndex(FareClassRestRuleInd);
TSSCacheEnum IndustryFareBasisModEntry::_cacheIndex(IndustryFareBasisModInd);
TSSCacheEnum TaxCodeRegEntry::_cacheIndex(TaxCodeRegInd);
TSSCacheEnum SvcFeesCxrResultingFCLInfoEntry::_cacheIndex(SvcFeesCxrResultingFCLInfoInd);
TSSCacheEnum NationEntry::_cacheIndex(NationInd);
TSSCacheEnum AllNationsEntry::_cacheIndex(AllNationsInd);
TSSCacheEnum MultiTransportCityLocCodeEntry::_cacheIndex(MultiTransportCityLocCodeInd);
TSSCacheEnum MultiTransportCityVectorEntry::_cacheIndex(MultiTransportCityVectorInd);
TSSCacheEnum SalesRestrictionEntry::_cacheIndex(SalesRestrictionInd);
TSSCacheEnum FareFocusSecurityEntry::_cacheIndex(FareFocusSecurityInd);
TSSCacheEnum FareFocusRuleEntry::_cacheIndex(FareFocusRuleInd);
TSSCacheEnum FareClassAppByTravelDateEntry::_cacheIndex(FareClassAppByTravelDateInd);
TSSCacheEnum RBDByCabinEntry::_cacheIndex(RBDByCabinInd);
TSSCacheEnum GeneralRuleAppByTvlDateEntry::_cacheIndex(GeneralRuleAppByTvlDateInd);
TSSCacheEnum GeneralRuleAppTariffRuleByTvlDateEntry::_cacheIndex(GeneralRuleAppTariffRuleByTvlDateInd);

} // tsscache

} // tse
