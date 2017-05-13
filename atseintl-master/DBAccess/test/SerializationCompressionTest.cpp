#include "test/include/CppUnitHelperMacros.h"
#include "DBAccess/AddonCombFareClassInfo.h"
#include "DBAccess/AddonZoneInfo.h"
#include "DBAccess/AdvResTktInfo.h"
#include "DBAccess/ATPResNationZones.h"
#include "DBAccess/BaggageSectorException.h"
#include "DBAccess/BankerSellRate.h"
#include "DBAccess/BaseFareRule.h"
#include "DBAccess/BlackoutInfo.h"
#include "DBAccess/BookingCodeConv.h"
#include "DBAccess/BookingCodeExceptionSequenceList.h"
#include "DBAccess/CarrierFlight.h"
#include "DBAccess/CircleTripProvision.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DBAccess/ContractPreference.h"
#include "DBAccess/Currency.h"
#include "DBAccess/Customer.h"
#include "DBAccess/CustomerSecurityHandshakeInfo.h"
#include "DBAccess/DiskCache.h"
#include "DBAccess/FareFocusFareClassInfo.h"
#include "DBAccess/FareFocusRuleInfo.h"
#include "DBAccess/FareProperties.h"
#include "DBAccess/SITAFareInfo.h"
#include "DBAccess/MarkupControl.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/FareClassAppInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/FootNoteCtrlInfo.h"
#include "DBAccess/SeatCabinCharacteristicInfo.h"
#include "DBAccess/GeneralRuleApp.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareByRuleCtrlInfo.h"
#include "DBAccess/FDAddOnFareInfo.h"
#include "DBAccess/EligibilityInfo.h"
#include "DBAccess/Mileage.h"
#include "DBAccess/NegFareSecurityInfo.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/Routing.h"
#include "DBAccess/PenaltyInfo.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/TaxNation.h"
#include "DBAccess/TaxReissue.h"
#include "DBAccess/SalesRestriction.h"
#include "DBAccess/TaxRestrictionLocationInfo.h"
#include "DBAccess/YQYRFees.h"
#include "DBAccess/TpdPsr.h"
#include "DBAccess/TaxRulesRecord.h"
#include "DBAccess/ZoneInfo.h"
#include "DBAccess/MinFareFareTypeGrp.h"
#include "DBAccess/SurchargesInfo.h"
#include "DBAccess/SeasonalAppl.h"
#include "DBAccess/FlightAppRule.h"
#include "DBAccess/TravelRestriction.h"
#include "DBAccess/VoluntaryChangesInfo.h"
#include "DBAccess/MileageSurchExcept.h"
#include "DBAccess/MarketCarrier.h"
#include "DBAccess/MarkupSecFilter.h"
#include "DBAccess/TariffInhibits.h"
#include "DBAccess/MinFareRuleLevelExcl.h"
#include "DBAccess/MinFareAppl.h"
#include "DBAccess/Loc.h"
#include "DBAccess/DateOverrideRuleItem.h"
#include "DBAccess/StopoversInfo.h"
#include "DBAccess/SvcFeesSecurityInfo.h"
#include "DBAccess/TransfersInfo1.h"
#include "DBAccess/MerchCarrierPreferenceInfo.h"
#include "DBAccess/MultiTransport.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/MinStayRestriction.h"
#include "DBAccess/MaxStayRestriction.h"
#include "DBAccess/GeoRuleItem.h"
#include "DBAccess/DayTimeAppInfo.h"
#include "DBAccess/MultiAirportCity.h"
#include "DBAccess/SvcFeesCurrencyInfo.h"
#include "DBAccess/SvcFeesCxrResultingFCLInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/TaxText.h"
#include "DBAccess/IndustryFareAppl.h"
#include "DBAccess/TicketEndorsementsInfo.h"
#include "DBAccess/PfcPFC.h"
#include "DBAccess/MiscFareTag.h"
#include "DBAccess/MileageSubstitution.h"
#include "DBAccess/PfcEssAirSvc.h"
#include "DBAccess/TariffCrossRefInfo.h"
#include "DBAccess/TaxCarrierAppl.h"
#include "DBAccess/TariffRuleRest.h"
#include "DBAccess/MerchActivationInfo.h"
#include "DBAccess/RoutingKeyInfo.h"
#include "DBAccess/NegFareRestExt.h"
#include "DBAccess/NegFareRestExtSeq.h"
#include "DBAccess/PfcAbsorb.h"
#include "DBAccess/SurfaceSectorExempt.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

namespace tse
{

template <typename T>
static const std::vector<T*>*
createVector()
{
  typedef typename boost::remove_const<T>::type NONCONST;
  std::vector<T*>* vect(new std::vector<T*>);
  NONCONST* item1(new NONCONST);
  T::dummyData(*item1);
  vect->push_back(item1);
  NONCONST* item2(new NONCONST);
  T::dummyData(*item2);
  vect->push_back(item2);
  return vect;
}

template <>
const std::vector<AddonFareInfo*>*
createVector<AddonFareInfo>()
{
  std::vector<AddonFareInfo*>* vect(new std::vector<AddonFareInfo*>);
  AddonFareInfo* fare(new AddonFareInfo);
  AddonFareInfo::dummyData(*fare);
  vect->push_back(fare);
  SITAAddonFareInfo* sitaFare(new SITAAddonFareInfo);
  SITAAddonFareInfo::dummyData(*sitaFare);
  vect->push_back(sitaFare);
  FDAddOnFareInfo* fdFare(new FDAddOnFareInfo);
  FDAddOnFareInfo::dummyData(*fdFare);
  vect->push_back(fdFare);
  return vect;
}

// existing dummyData is populating _pAdditionalInfoContainer, but we don't serialize it
static void
FareInfoDummyData(FareInfo* obj)
{
  FareInfo::dummyData(*obj);
  delete obj->_pAdditionalInfoContainer;
  obj->_pAdditionalInfoContainer = 0;
}

static void
SITAFareInfoDummyData(SITAFareInfo* obj)
{
  SITAFareInfo::dummyData(*obj);
  delete obj->_pAdditionalInfoContainer;
  obj->_pAdditionalInfoContainer = 0;
}

template <>
const std::vector<const FareInfo*>*
createVector<const FareInfo>()
{
  std::vector<const FareInfo*>* vect(new std::vector<const FareInfo*>);
  FareInfo* fare(new FareInfo);
  FareInfoDummyData(fare);
  vect->push_back(fare);
  SITAFareInfo* sitaFare(new SITAFareInfo);
  SITAFareInfoDummyData(sitaFare);
  vect->push_back(sitaFare);
  return vect;
}

class SerializationCompressionTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SerializationCompressionTest);
  CPPUNIT_TEST(testFareClassAppInfo);
  CPPUNIT_TEST(testFare);
  CPPUNIT_TEST(testMarkupControl);
  CPPUNIT_TEST(testNegFareRest);
  CPPUNIT_TEST(testOptionalServices);
  CPPUNIT_TEST(testFareByRuleItemInfo);
  CPPUNIT_TEST(testGeneralFareRule);
  CPPUNIT_TEST(testFootnoteCtrl);
  CPPUNIT_TEST(testAddonCombFareClassInfo);
  CPPUNIT_TEST(testAddonCombFareClassInfoMap);
  CPPUNIT_TEST(testBaseFareRule);
  CPPUNIT_TEST(testSeatCabinCharacteristicRule);
  CPPUNIT_TEST(testGeneralRuleApp);
  CPPUNIT_TEST(testFareByRuleApp);
  CPPUNIT_TEST(testFareByRuleCtrl);
  CPPUNIT_TEST(testAddonFare);
  CPPUNIT_TEST(testCombinabilityRuleInfo);
  CPPUNIT_TEST(testEligibilityInfo);
  CPPUNIT_TEST(testATPResNationZones);
  CPPUNIT_TEST(testMileage);
  CPPUNIT_TEST(testCircleTripProvision);
  CPPUNIT_TEST(testNegFareSecurityInfo);
  CPPUNIT_TEST(testTaxCodeReg);
  CPPUNIT_TEST(testRouting);
  CPPUNIT_TEST(testPenalty);
  CPPUNIT_TEST(testFareCalcConfig);
  CPPUNIT_TEST(testTaxNation);
  CPPUNIT_TEST(testTaxReissue);
  CPPUNIT_TEST(testAddonZoneInfo);
  CPPUNIT_TEST(testSalesRestriction);
  CPPUNIT_TEST(testTaxRestrictionLocationInfo);
  CPPUNIT_TEST(testYQYRFees);
  CPPUNIT_TEST(testTpdPsr);
  CPPUNIT_TEST(testTaxRulesRecord);
  CPPUNIT_TEST(testZoneInfo);
  CPPUNIT_TEST(testBookingCodeExceptionSequence);
  CPPUNIT_TEST(testMinFareFareTypeGrp);
  CPPUNIT_TEST(testSurchargesInfo);
  CPPUNIT_TEST(testSeasonalAppl);
  CPPUNIT_TEST(testFlightAppRule);
  CPPUNIT_TEST(testBookingCodeConv);
  CPPUNIT_TEST(testContractPreference);
  CPPUNIT_TEST(testAdvResTktInfo);
  CPPUNIT_TEST(testTravelRestriction);
  CPPUNIT_TEST(testVoluntaryChangesInfo);
  CPPUNIT_TEST(testMileageSurchExcept);
  CPPUNIT_TEST(testCarrierFlight);
  CPPUNIT_TEST(testMarketCarrier);
  CPPUNIT_TEST(testMarkupSecFilter);
  CPPUNIT_TEST(testTariffInhibits);
  CPPUNIT_TEST(testMinFareRuleLevelExcl);
  CPPUNIT_TEST(testMinFareAppl);
  CPPUNIT_TEST(testLoc);
  CPPUNIT_TEST(testBlackoutInfo);
  CPPUNIT_TEST(testDateOverrideRuleItem);
  CPPUNIT_TEST(testStopoversInfo);
  CPPUNIT_TEST(testSvcFeesSecurityInfo);
  CPPUNIT_TEST(testTransfersInfo1);
  CPPUNIT_TEST(testMerchCarrierPreferenceInfo);
  CPPUNIT_TEST(testMultiTransport);
  CPPUNIT_TEST(testDiscountInfo);
  CPPUNIT_TEST(testMinStayRestriction);
  CPPUNIT_TEST(testMaxStayRestriction);
  CPPUNIT_TEST(testGeoRuleItem);
  CPPUNIT_TEST(testDayTimeAppInfo);
  CPPUNIT_TEST(testMultiAirportCity);
  CPPUNIT_TEST(testSvcFeesCurrencyInfo);
  CPPUNIT_TEST(testSvcFeesCxrResultingFCLInfo);
  CPPUNIT_TEST(testSubCodeInfo);
  CPPUNIT_TEST(testTaxText);
  CPPUNIT_TEST(testIndustryFareAppl);
  CPPUNIT_TEST(testTicketEndorsementsInfo);
  CPPUNIT_TEST(testPfcPFC);
  CPPUNIT_TEST(testMiscFareTag);
  CPPUNIT_TEST(testMileageSubstitution);
  CPPUNIT_TEST(testPfcEssAirSvc);
  CPPUNIT_TEST(testTariffCrossRefInfo);
  CPPUNIT_TEST(testTaxCarrierAppl);
  CPPUNIT_TEST(testTariffRuleRest);
  CPPUNIT_TEST(testMerchActivationInfo);
  CPPUNIT_TEST(testRoutingKeyInfo);
  CPPUNIT_TEST(testCustomerSecurityHandshakeInfo);
  CPPUNIT_TEST(testNegFareRestExt);
  CPPUNIT_TEST(testNegFareRestExtSeq);
  CPPUNIT_TEST(testPfcAbsorb);
  CPPUNIT_TEST(testSurfaceSectorExempt);
  CPPUNIT_TEST(testCurrency);
  CPPUNIT_TEST(testFareFocusFareClass);
  CPPUNIT_TEST(testFareFocusRule);
  CPPUNIT_TEST(testBankerSellRate);
  CPPUNIT_TEST(testCustomer);
  CPPUNIT_TEST(testBaggageSectorException);
  CPPUNIT_TEST(testFareProperties);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

  class SpecificTestConfigInitializer : public TestConfigInitializer
  {
  public:
    SpecificTestConfigInitializer()
    {
      DiskCache::initialize(_config);
      _memHandle.create<MockDataManager>();
    }

    ~SpecificTestConfigInitializer() { _memHandle.clear(); }

  private:
    TestMemHandle _memHandle;
  };

public:
  void setUp() { _memHandle.create<SpecificTestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  template <typename T>
  static void destroyVector(const std::vector<T*>* vect)
  {
    for (typename std::vector<T*>::const_iterator it(vect->begin()), itend(vect->end());
         it != itend;
         ++it)
    {
      delete *it;
    }
    delete vect;
  }

  template <typename T>
  static void testCompressionSerialization()
  {
    const std::vector<T*>* orig(createVector<T>());
    sfc::CompressedData* compressed(compressEntry(orig));
    const std::vector<T*>* restored(uncompressEntry(*compressed, orig));
    delete compressed;
    std::string msg;
    CPPUNIT_ASSERT(equalPtrContainer(*orig, *restored, msg));
    destroyVector(restored);
    destroyVector(orig);
  }

  void testFare() { testCompressionSerialization<const FareInfo>(); }

  void testMarkupControl() { testCompressionSerialization<MarkupControl>(); }

  void testOptionalServices() { testCompressionSerialization<OptionalServicesInfo>(); }

  void testFareByRuleItemInfo() { testCompressionSerialization<FareByRuleItemInfo>(); }

  void testFareClassAppInfo() { testCompressionSerialization<const FareClassAppInfo>(); }

  void testNegFareRest() { testCompressionSerialization<NegFareRest>(); }

  void testGeneralFareRule() { testCompressionSerialization<GeneralFareRuleInfo>(); }

  void testFootnoteCtrl() { testCompressionSerialization<FootNoteCtrlInfo>(); }

  void testAddonCombFareClassInfo() { testCompressionSerialization<AddonCombFareClassInfo>(); }

  void testAddonCombFareClassInfoMap()
  {
    AddonFareClassCombMultiMap* orig(new AddonFareClassCombMultiMap);
    dummyData(*orig);
    sfc::CompressedData* compressed(compressEntry(orig));
    if (compressed)
    {
      const AddonFareClassCombMultiMap* restored(uncompressEntry(*compressed, orig));
      delete compressed;
      if (restored)
      {
        CPPUNIT_ASSERT(*orig == *restored);
        delete restored;
      }
      else
      {
        CPPUNIT_ASSERT(false);
      }
    }
    else
    {
      CPPUNIT_ASSERT(false);
    }
    delete orig;
  }

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

  void dummyData(AddonFareClassCombMultiMap& map)
  {
    AddonCombFareClassInfo* addon1(new AddonCombFareClassInfo);
    AddonCombFareClassInfo::dummyData(*addon1);
    std::vector<AddonCombFareClassInfo*> addons1(1, addon1);
    AddonCombFareClassSpecifiedKey key1(addon1->fareClass(), addon1->owrt());
    map.emplace(key1, std::move(addons1));

    AddonCombFareClassInfo* addon2(new AddonCombFareClassInfo);
    AddonCombFareClassInfo::dummyData2(*addon2);
    std::vector<AddonCombFareClassInfo*> addons2(1, addon2);
    AddonCombFareClassSpecifiedKey key2(addon2->fareClass(), addon2->owrt());
    map.emplace(key2, std::move(addons2));
  }

#else

  void dummyData(AddonFareClassCombMultiMap& map)
  {
    AddonCombFareClassInfo* addon1(new AddonCombFareClassInfo);
    AddonCombFareClassInfo::dummyData(*addon1);
    AddonCombFareClassInfoKey key1(
        addon1->addonFareClass(), addon1->geoAppl(), addon1->owrt(), addon1->fareClass());
    map.insert(AddonFareClassCombMultiMap::value_type(key1, addon1));

    AddonCombFareClassInfo* addon2(new AddonCombFareClassInfo);
    AddonCombFareClassInfo::dummyData2(*addon2);
    AddonCombFareClassInfoKey key2(
        addon2->addonFareClass(), addon2->geoAppl(), addon2->owrt(), addon2->fareClass());
    map.insert(AddonFareClassCombMultiMap::value_type(key2, addon2));
  }

#endif

  void testBaseFareRule() { testCompressionSerialization<const BaseFareRule>(); }

  void testSeatCabinCharacteristicRule()
  {
    testCompressionSerialization<SeatCabinCharacteristicInfo>();
  }

  void testGeneralRuleApp() { testCompressionSerialization<GeneralRuleApp>(); }

  void testFareByRuleApp() { testCompressionSerialization<FareByRuleApp>(); }

  void testFareByRuleCtrl() { testCompressionSerialization<FareByRuleCtrlInfo>(); }

  void testAddonFare() { testCompressionSerialization<AddonFareInfo>(); }

  void testCombinabilityRuleInfo() { testCompressionSerialization<CombinabilityRuleInfo>(); }

  void testEligibilityInfo() { testCompressionSerialization<const EligibilityInfo>(); }

  void testATPResNationZones() { testCompressionSerialization<ATPResNationZones>(); }

  void testMileage() { testCompressionSerialization<Mileage>(); }

  void testCircleTripProvision() { testCompressionSerialization<CircleTripProvision>(); }

  void testNegFareSecurityInfo() { testCompressionSerialization<NegFareSecurityInfo>(); }

  void testTaxCodeReg() { testCompressionSerialization<TaxCodeReg>(); }

  void testRouting() { testCompressionSerialization<Routing>(); }

  void testPenalty() { testCompressionSerialization<PenaltyInfo>(); }

  void testFareCalcConfig() { testCompressionSerialization<FareCalcConfig>(); }

  void testTaxNation() { testCompressionSerialization<TaxNation>(); }

  void testTaxReissue() { testCompressionSerialization<TaxReissue>(); }

  void testAddonZoneInfo() { testCompressionSerialization<AddonZoneInfo>(); }

  void testSalesRestriction() { testCompressionSerialization<SalesRestriction>(); }

  void testTaxRestrictionLocationInfo()
  {
    testCompressionSerialization<TaxRestrictionLocationInfo>();
  }

  void testYQYRFees() { testCompressionSerialization<YQYRFees>(); }

  void testTpdPsr() { testCompressionSerialization<TpdPsr>(); }

  void testTaxRulesRecord() { testCompressionSerialization<TaxRulesRecord>(); }

  void testZoneInfo() { testCompressionSerialization<const ZoneInfo>(); }

  void testBookingCodeExceptionSequence()
  {
    testCompressionSerialization<BookingCodeExceptionSequence>();
  }

  void testMinFareFareTypeGrp()
  {
    testCompressionSerialization<MinFareFareTypeGrp>();
  }

  void testSurchargesInfo()
  {
    testCompressionSerialization<SurchargesInfo>();
  }

  void testSeasonalAppl()
  {
    testCompressionSerialization<SeasonalAppl>();
  }

  void testFlightAppRule()
  {
    testCompressionSerialization<FlightAppRule>();
  }

  void testBookingCodeConv()
  {
    testCompressionSerialization<BookingCodeConv>();
  }

  void testContractPreference()
  {
    testCompressionSerialization<ContractPreference>();
  }

  void testAdvResTktInfo()
  {
    testCompressionSerialization<AdvResTktInfo>();
  }

  void testTravelRestriction()
  {
    testCompressionSerialization<TravelRestriction>();
  }

  void testVoluntaryChangesInfo()
  {
    testCompressionSerialization<VoluntaryChangesInfo>();
  }

  void testMileageSurchExcept()
  {
    testCompressionSerialization<MileageSurchExcept>();
  }

  void testCarrierFlight()
  {
    testCompressionSerialization<CarrierFlight>();
  }

  void testMarketCarrier()
  {
    testCompressionSerialization<MarketCarrier>();
  }

  void testMarkupSecFilter()
  {
    testCompressionSerialization<MarkupSecFilter>();
  }

  void testTariffInhibits()
  {
    testCompressionSerialization<TariffInhibits>();
  }

  void testMinFareRuleLevelExcl()
  {
    testCompressionSerialization<MinFareRuleLevelExcl>();
  }

  void testMinFareAppl()
  {
    testCompressionSerialization<MinFareAppl>();
  }

  void testLoc()
  {
    testCompressionSerialization<Loc>();
  }

  void testBlackoutInfo()
  {
    testCompressionSerialization<BlackoutInfo>();
  }

  void testDateOverrideRuleItem()
  {
    testCompressionSerialization<DateOverrideRuleItem>();
  }

  void testStopoversInfo()
  {
    testCompressionSerialization<StopoversInfo>();
  }

  void testSvcFeesSecurityInfo()
  {
    testCompressionSerialization<SvcFeesSecurityInfo>();
  }

  void testTransfersInfo1()
  {
    testCompressionSerialization<TransfersInfo1>();
  }

  void testMerchCarrierPreferenceInfo()
  {
    testCompressionSerialization<MerchCarrierPreferenceInfo>();
  }

  void testMultiTransport()
  {
    testCompressionSerialization<MultiTransport>();
  }

  void testDiscountInfo()
  {
    testCompressionSerialization<DiscountInfo>();
  }

  void testMinStayRestriction()
  {
    testCompressionSerialization<MinStayRestriction>();
  }

  void testMaxStayRestriction()
  {
    testCompressionSerialization<MaxStayRestriction>();
  }

  void testGeoRuleItem()
  {
    testCompressionSerialization<GeoRuleItem>();
  }

  void testDayTimeAppInfo()
  {
    testCompressionSerialization<DayTimeAppInfo>();
  }

  void testMultiAirportCity()
  {
    testCompressionSerialization<MultiAirportCity>();
  }

  void testSvcFeesCurrencyInfo()
  {
    testCompressionSerialization<SvcFeesCurrencyInfo>();
  }

  void testSvcFeesCxrResultingFCLInfo()
  {
    testCompressionSerialization<SvcFeesCxrResultingFCLInfo>();
  }

  void testSubCodeInfo()
  {
    testCompressionSerialization<SubCodeInfo>();
  }

  void testTaxText()
  {
    testCompressionSerialization<TaxText>();
  }

  void testIndustryFareAppl()
  {
    testCompressionSerialization<IndustryFareAppl>();
  }

  void testTicketEndorsementsInfo()
  {
    testCompressionSerialization<TicketEndorsementsInfo>();
  }

  void testPfcPFC()
  {
    testCompressionSerialization<PfcPFC>();
  }

  void testMiscFareTag()
  {
    testCompressionSerialization<MiscFareTag>();
  }

  void testMileageSubstitution()
  {
    testCompressionSerialization<MileageSubstitution>();
  }

  void testPfcEssAirSvc()
  {
    testCompressionSerialization<PfcEssAirSvc>();
  }

  void testTariffCrossRefInfo()
  {
    testCompressionSerialization<TariffCrossRefInfo>();
  }

  void testTaxCarrierAppl()
  {
    testCompressionSerialization<TaxCarrierAppl>();
  }

  void testTariffRuleRest()
  {
    testCompressionSerialization<TariffRuleRest>();
  }

  void testMerchActivationInfo()
  {
    testCompressionSerialization<MerchActivationInfo>();
  }

  void testRoutingKeyInfo()
  {
    testCompressionSerialization<RoutingKeyInfo>();
  }

  void testCustomerSecurityHandshakeInfo()
  {
    testCompressionSerialization<CustomerSecurityHandshakeInfo>();
  }

  void testNegFareRestExt()
  {
    testCompressionSerialization<NegFareRestExt>();
  }


  void testNegFareRestExtSeq()
  {
    testCompressionSerialization<NegFareRestExtSeq>();
  }

  void testPfcAbsorb()
  {
    testCompressionSerialization<PfcAbsorb>();
  }

  void testSurfaceSectorExempt()
  {
    testCompressionSerialization<SurfaceSectorExempt>();
  }

  void testCurrency()
  {
    testCompressionSerialization<Currency>();
  }

  void testFareFocusFareClass()
  {
    testCompressionSerialization<FareFocusFareClassInfo>();
  }

  void testFareFocusRule()
  {
    testCompressionSerialization<FareFocusRuleInfo>();
  }

  void testBankerSellRate()
  {
    testCompressionSerialization<BankerSellRate>();
  }

  void testCustomer()
  {
    testCompressionSerialization<Customer>();
  }

  void testBaggageSectorException()
  {
    testCompressionSerialization<BaggageSectorException>();
  }

  void testFareProperties()
  {
    testCompressionSerialization<FareProperties>();
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(SerializationCompressionTest);

} // tse
