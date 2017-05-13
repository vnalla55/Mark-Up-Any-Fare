//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include <string>
#include <algorithm>

#include "Common/Config/ConfigurableValuesPool.h"
#include "Common/MetricsUtil.h"
#include "Common/StopWatch.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/Vendor.h"
#include "DataModel/Fare.h"
#include "DataModel/Trx.h"
#include "DBAccess/AddonZoneInfo.h"
#include "DBAccess/AirlineCountrySettlementPlanInfo.h"
#include "DBAccess/AirlineInterlineAgreementInfo.h"
#include "DBAccess/BookingCodeExceptionSequence.h"
#include "DBAccess/BookingCodeExceptionSequenceList.h"
#include "DBAccess/Cabin.h"
#include "DBAccess/CarrierMixedClass.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/Record2Types.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DBAccess/CountrySettlementPlanInfo.h"
#include "DBAccess/Customer.h"
#include "DBAccess/CustomerActivationControl.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Differentials.h"
#include "DBAccess/DST.h"
#include "DBAccess/EndOnEnd.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/FareClassAppInfo.h"
#include "DBAccess/FareDisplaySort.h"
#include "DBAccess/FareDispRec1PsgType.h"
#include "DBAccess/FareDispRec8PsgType.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/FDSFareBasisComb.h"
#include "DBAccess/FDSGlobalDir.h"
#include "DBAccess/FDSPsgType.h"
#include "DBAccess/FDSSorting.h"
#include "DBAccess/FltTrkCntryGrp.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/GenSalesAgentInfo.h"
#include "DBAccess/HipMileageExceptInfo.h"
#include "DBAccess/LimitationCmn.h"
#include "DBAccess/LimitationFare.h"
#include "DBAccess/LimitationJrny.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MultiAirportCity.h"
#include "DBAccess/Nation.h"
#include "DBAccess/NeutralValidatingAirlineInfo.h"
#include "DBAccess/OpenJawRule.h"
#include "DBAccess/PassengerAirlineInfo.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/ReissueSequence.h"
#include "DBAccess/RuleCatAlphaCode.h"
#include "DBAccess/SalesRestriction.h"
#include "DBAccess/SurfaceSectorExemptionInfo.h"
#include "DBAccess/SvcFeesAccCodeInfo.h"
#include "DBAccess/SvcFeesSecurityInfo.h"
#include "DBAccess/SvcFeesTktDesignatorInfo.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TicketingFeesInfo.h"
#include "DBAccess/TSIInfo.h"
#include "DBAccess/VoluntaryChangesInfo.h"
#include "DBAccess/Waiver.h"
#include "DBAccess/ZoneInfo.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/MockDataManager.h"
#include "test/include/TestLogger.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{
// test class that counts the number of instances
class Foo
{
public:
  static int count;
  Trx* _trx;
  Foo() { count++; }
  Foo(const Foo&) { count++; }
  virtual ~Foo() { count--; }
  Trx& trx() { return *_trx; }
};

int Foo::count = 0;

class DataHandleTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DataHandleTest);
  CPPUNIT_TEST(testGetGroups);
  CPPUNIT_TEST(testGetTours);
  CPPUNIT_TEST(testGetZone);
  CPPUNIT_SKIP_TEST(testGetDayTimeAppInfo);
  CPPUNIT_TEST(testGetGeoRule);
  CPPUNIT_TEST(testGetOpenJawRule);
  CPPUNIT_TEST(testGetDifferentials);
  CPPUNIT_TEST(testGetSeasonalAppl);
  CPPUNIT_TEST(testGetCopMinimum);
  CPPUNIT_TEST(testGetCopParticipatingNation);
  CPPUNIT_TEST(testGetMinFareAppl);
  CPPUNIT_TEST(testGetIndustryPricingAppl);
  CPPUNIT_TEST(testGetIndustryFareAppl);
  CPPUNIT_TEST(testGetIndustryFareBasisMod);
  CPPUNIT_TEST(testGetSalesNationRestr);
  CPPUNIT_TEST(testGetMinFareFareTypeGrp);
  CPPUNIT_TEST(testGetPfcPFC);
  CPPUNIT_TEST(testGetPfcMultiAirport);
  CPPUNIT_TEST(testGetPfcEssAirSvc);
  CPPUNIT_TEST(testGetPfcCollectMeth);
  CPPUNIT_TEST(testGetPfcAbsorb);
  CPPUNIT_TEST(testGetPfcEquipTypeExempt);
  CPPUNIT_TEST(testGetTariffMileageAddon);
  CPPUNIT_TEST(testGetMileageSubstitution);
  CPPUNIT_TEST(testGetSurfaceSectorExempt);
  CPPUNIT_TEST(testGetTpdPsr);
  CPPUNIT_TEST(testGetFCLimitation);
  CPPUNIT_TEST(testGetJLimitation);
  CPPUNIT_TEST(testGetCabin);
  CPPUNIT_TEST(testGetSalesRestriction);
  CPPUNIT_TEST(testGetFareCalcConfig);
  CPPUNIT_SKIP_TEST(testGetBankerSellRate);
  CPPUNIT_SKIP_TEST(testGetBankerSellRateRange);
  CPPUNIT_SKIP_TEST(testGetBankerSellRateRangeVersionEquivalent);
  CPPUNIT_TEST(testGetTariffXRefByFareTariff);
  CPPUNIT_TEST(testGetTariffXRefByRuleTariff);
  CPPUNIT_TEST(testGetMileage);
  CPPUNIT_TEST(testGetCurrencySelection);
  CPPUNIT_TEST(testGetEndOnEnd);
  CPPUNIT_SKIP_TEST(testGetCarrierCombination);
  CPPUNIT_SKIP_TEST(testGetCarrierPreference); // missing carrier AA in database:
  // ORACLE-INT-HIST(only on RH5)
  CPPUNIT_TEST(testGetGeneralFareRule);
  CPPUNIT_TEST(testGetFareByRuleItem);
  CPPUNIT_TEST(testGetAllFareTypeMatrix);
  CPPUNIT_TEST(testGetTSI);
  CPPUNIT_TEST(testGetLoc);
  CPPUNIT_SKIP_TEST(testGetFareClassApp);
  CPPUNIT_TEST(testGetFareByRuleApp);
  CPPUNIT_TEST(testGetFareByRuleAppWhenCorpIdExists);
  CPPUNIT_TEST(testGetFareByRuleAppWhenCorpIdNotExist);
  CPPUNIT_SKIP_TEST(testGetTaxCode);
  CPPUNIT_TEST(testGetObject);
  CPPUNIT_TEST(testGetFltTrkCntryGrp);
  CPPUNIT_TEST(testGetCarrierMixedClass);
  CPPUNIT_TEST(testGetMultiAirportCity);
  CPPUNIT_TEST(testGetTaxNation);
  CPPUNIT_TEST(testGetNation);
  CPPUNIT_TEST(testGetPaxType);
  CPPUNIT_TEST(testGetPaxTypeMatrix);
  CPPUNIT_TEST(testGetCombinability);
  CPPUNIT_TEST(testGetGlobalDirSeg);
  CPPUNIT_TEST(testGetRouting);
  CPPUNIT_TEST(testGetFareDisplayInclCd);
  CPPUNIT_TEST(testGetFareDispInclRuleTrf);
  CPPUNIT_TEST(testGetFareDispInclFareType);
  CPPUNIT_TEST(testGetFareDispInclDsplType);
  CPPUNIT_TEST(testGetRuleCatAlphaCode);
  CPPUNIT_TEST(testGetFareDispRec1PsgType);
  CPPUNIT_TEST(testGetFareDispRec8PsgType);
  CPPUNIT_TEST(testGetWaiver);
  CPPUNIT_TEST(testGetVoluntaryChangesConfigPass);
  CPPUNIT_TEST(testGetVoluntaryChangesConfigDefault);
  CPPUNIT_SKIP_TEST(testGetCountrySettlementPlans_NonHistorical);
  CPPUNIT_SKIP_TEST(testGetCountrySettlementPlans_Historical);
  CPPUNIT_SKIP_TEST(testGetAirlineInterlineAgreements);
  CPPUNIT_SKIP_TEST(testGetAirlineInterlineAgreements_Historical);
  CPPUNIT_SKIP_TEST(testGetAirlineCountrySettlementPlans);
  CPPUNIT_SKIP_TEST(testGetAirlineCountrySettlementPlans_ByCountryAndPlan);
  CPPUNIT_SKIP_TEST(testGetAirlineCountrySettlementPlans_ByCountryAndAirline);
  CPPUNIT_SKIP_TEST(testGetNeutralValidatingAirlines);
  CPPUNIT_SKIP_TEST(testGetNeutralValidatingAirlines_Historical);
  CPPUNIT_SKIP_TEST(testGetGenSalesAgents);
  CPPUNIT_SKIP_TEST(testGetGenSalesAgents_Preload);
  CPPUNIT_SKIP_TEST(testGetGenSalesAgents_Historical);
  CPPUNIT_SKIP_TEST(testGetCustomerActivationControl);
  CPPUNIT_SKIP_TEST(testGetCustomer);
  CPPUNIT_TEST(testGetBookingCodeException);
  CPPUNIT_TEST_SUITE_END();

private:
  static const char* cities[];

  DataHandle* _dataHandle;
  TestMemHandle _memHandle;
  RootLoggerGetOff _logger;

  static const DateTime TICKETDATE;
  static const DateTime TODAY;

  class SpecificConfigInitializer
  {
  protected:
    class ConfigBinder
    {
    public:
      ConfigBinder()
      {
        GlobalImpl::_allowHistorical = true;
        GlobalImpl::_configMan = &MockDataManagerImpl::_mainConfig;
        DiskCache::initialize(MockDataManagerImpl::_mainConfig);
      }
      ~ConfigBinder() { GlobalImpl::_configMan = 0; }

    protected:
      struct GlobalImpl : public Global
      {
        using Global::_configMan;
        using Global::_allowHistorical;
      };

      struct MockDataManagerImpl : public MockDataManager
      {
        using MockDataManager::_mainConfig;
      };

    } _configBinder;
    MockDataManager _mockDataManager;
  };

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<SpecificConfigInitializer>();
    _dataHandle = _memHandle.create<DataHandle>(TICKETDATE);
  }

  void tearDown() { _memHandle.clear(); }

  void testGetFareClassApp()
  {
    const std::vector<const tse::FareClassAppInfo*>& result =
        _dataHandle->getFareClassApp("ATP", "AA", 1, "2001", "B11");
    CPPUNIT_ASSERT(result.size() != 0);
    CPPUNIT_ASSERT(result.front()->_segs.size() == 1);
  }

  // Retrieve fare tariff
  void testGetTariffXRefByFareTariff()
  {
    const std::vector<tse::TariffCrossRefInfo*>& result = _dataHandle->getTariffXRefByFareTariff(
        "ATP", "AA", INTERNATIONAL, 1, DateTime::localTime());
    CPPUNIT_ASSERT(result.size() != 0);
  }

  // Retrieve rule tariff
  void testGetTariffXRefByRuleTariff()
  {
    const std::vector<tse::TariffCrossRefInfo*>& result = _dataHandle->getTariffXRefByRuleTariff(
        "ATP", "RG", INTERNATIONAL, 753, DateTime::localTime());
    CPPUNIT_ASSERT(result.size() != 0);
  }

  void testGetObject()
  {
    Foo* foo;
    int numFoo = Foo::count;
    {
      DataHandle dataHandle;
      dataHandle.get(foo);
      CPPUNIT_ASSERT(Foo::count == numFoo + 1);
    }

    CPPUNIT_ASSERT(Foo::count == numFoo);
    {
      DataHandle dataHandle;
      {
        DataHandle dataHandle2;
        dataHandle2.get(foo);
        dataHandle.import(dataHandle2);
      }
      CPPUNIT_ASSERT(Foo::count == numFoo + 1);
    }
    CPPUNIT_ASSERT(Foo::count == numFoo);

    tse::StopWatch stopWatch;
    stopWatch.start();
    DataHandle dh;
    Fare* fare;
    for (int i = 0; i < 1000000; i++)
    {
      fare = 0;
      dh.get(fare);
      if (fare == 0)
        CPPUNIT_FAIL("Unable to allocate fare");
    }
    stopWatch.stop();
  }

  void testGetTaxNation()
  {
    const tse::TaxNation* taxNation = _dataHandle->getTaxNation("PE", TICKETDATE);
    CPPUNIT_ASSERT(taxNation != 0);
  }

  void testGetNation()
  {
    const Nation* nation = _dataHandle->getNation("KR", TICKETDATE);
    CPPUNIT_ASSERT(nation != 0);
  }

  void testGetLoc()
  {
    DateTime dt(2005, 1, 1);
    const Loc* loc = _dataHandle->getLoc("FRA", dt);
    CPPUNIT_ASSERT(loc != 0);
    loc = _dataHandle->getLoc("EWR", dt);
    CPPUNIT_ASSERT(loc != 0);
  }

  void testGetTaxCode()
  {
    const std::vector<TaxCodeReg*> taxes = _dataHandle->getTaxCode("AA", TICKETDATE);
    CPPUNIT_ASSERT(taxes.size() != 0);
  }

  void testGetPaxType()
  {
    const PaxTypeInfo* paxType = _dataHandle->getPaxType("ADT", Vendor::ATPCO);
    CPPUNIT_ASSERT(paxType != 0);
    CPPUNIT_ASSERT(paxType->adultInd() == 'Y');
  }

  void testGetPaxTypeMatrix()
  {
    const std::vector<const PaxTypeMatrix*>& paxType = _dataHandle->getPaxTypeMatrix("TV1");
    CPPUNIT_ASSERT(paxType.size() != 0);
  }

  void testGetCombinability()
  {
    DateTime now = TICKETDATE;
    const std::vector<CombinabilityRuleInfo*>& rules =
        _dataHandle->getCombinabilityRule("ATP", "CO", 1, "2020", now);
    if (!rules.empty())
    {
      // CombinabilityRuleInfo& rule = *(rules.front());
      CPPUNIT_ASSERT((rules.front())->effDate() <= now);
      CPPUNIT_ASSERT((rules.front())->discDate() >= now);
    }
  }

  void testGetBookingCodeException()
  {
    bool isRuleZero = false;
    const BookingCodeExceptionSequenceList& conv2 = _dataHandle->getBookingCodeExceptionSequence(
        "ATP", "AA", 21, "0000", '2', TICKETDATE, isRuleZero, false);
    CPPUNIT_ASSERT(conv2.size() != 0);
    const BookingCodeExceptionSequenceList& conv1 = _dataHandle->getBookingCodeExceptionSequence(
        "ATP", "F9", 0, "0000", '1', TICKETDATE, isRuleZero, false);
    CPPUNIT_ASSERT(conv1.size() != 0);
  }

  void testGetGlobalDirSeg()
  {
    const std::vector<GlobalDirSeg*>& gd = _dataHandle->getGlobalDirSeg(TICKETDATE);
    CPPUNIT_ASSERT(gd.size() != 0);
  }

  void testGetRouting()
  {
    const std::vector<Routing*> r = _dataHandle->getRouting("ATP", "BA", 1, "0074", TICKETDATE);
    CPPUNIT_ASSERT(!r.empty());
  }

  void testGetMultiAirportCity()
  {
    const std::vector<MultiAirportCity*> r = _dataHandle->getMultiAirportCity("EWR");
    CPPUNIT_ASSERT(!r.empty());
    CPPUNIT_ASSERT(r.front()->city() == "NYC");
  }

  void testGetCarrierMixedClass()
  {
    const std::vector<CarrierMixedClass*>& r = _dataHandle->getCarrierMixedClass("AA", TICKETDATE);
    CPPUNIT_ASSERT(!r.empty());
    CPPUNIT_ASSERT(r.front()->carrier() == "AA");
  }

  void testGetMileage()
  {
    const Mileage* r = _dataHandle->getMileage("MIA", "LAX", TPM, GlobalDirection::WH, TICKETDATE);
    CPPUNIT_ASSERT(r != 0);
    r = _dataHandle->getMileage("MIA", "ORL", TPM, GlobalDirection::WH, TICKETDATE);
    CPPUNIT_ASSERT(r != 0);
    r = _dataHandle->getMileage("LAX", "ORL", TPM, GlobalDirection::WH, TICKETDATE);
    CPPUNIT_ASSERT(r != 0);
    const std::vector<Mileage*>& l1 = _dataHandle->getMileage("MIA", "LAX", TICKETDATE);
    CPPUNIT_ASSERT(!l1.empty());
    const std::vector<Mileage*>& l2 = _dataHandle->getMileage("MIA", "LAX", TICKETDATE, TPM);
    CPPUNIT_ASSERT(!l2.empty());
  }

  void testGetFltTrkCntryGrp()
  {
    const FltTrkCntryGrp* r = _dataHandle->getFltTrkCntryGrp("AA", TICKETDATE);
    CPPUNIT_ASSERT(r != 0);
    CPPUNIT_ASSERT(!(r->nations().empty()));
  }

  void testGetFareByRuleApp()
  {
    CarrierCode carrier = "AA";
    std::string corpid = "";
    AccountCode account = "BKN01";
    DateTime t = TICKETDATE;
    TktDesignator tktDesignator = "CORP";
    std::vector<PaxTypeCode> paxTypes;
    paxTypes.push_back("ADT");
    const std::vector<FareByRuleApp*>& r =
        _dataHandle->getFareByRuleApp(carrier, corpid, account, tktDesignator, t, paxTypes);
    CPPUNIT_ASSERT(!(r.empty()));
  }

  void testGetFareByRuleAppWhenCorpIdExists()
  {
    CarrierCode carrier = "AA";
    std::string corpid = "BEA01";
    AccountCode account = "";
    DateTime t = TICKETDATE;
    TktDesignator tktDesignator = "CORP";
    std::vector<PaxTypeCode> paxTypes;
    paxTypes.push_back("ADT");
    const std::vector<FareByRuleApp*>& r =
        _dataHandle->getFareByRuleApp(carrier, corpid, account, tktDesignator, t, paxTypes);
    CPPUNIT_ASSERT(!(r.empty()));
  }

  void testGetFareByRuleAppWhenCorpIdNotExist()
  {
    CarrierCode carrier = "NW";
    std::string corpid = "BKN01";
    AccountCode account = "";
    DateTime t = TICKETDATE;
    TktDesignator tktDesignator = "ROBB";
    std::vector<PaxTypeCode> paxTypes;
    paxTypes.push_back("ADT");
    const std::vector<FareByRuleApp*>& r =
        _dataHandle->getFareByRuleApp(carrier, corpid, account, tktDesignator, t, paxTypes);
    CPPUNIT_ASSERT(!(r.empty()));
  }

  void testGetTSI()
  {
    // from cfg file
    /*
      DataHandle dataHandle;
      const TSIInfo* r =
      _dataHandle->getTSI(84);
      CPPUNIT_ASSERT(r != 0);
      CPPUNIT_ASSERT(r->tsi() == 84);
      CPPUNIT_ASSERT(r->geoRequired() == ' ');
      CPPUNIT_ASSERT(r->geoNotType() == ' ');
      CPPUNIT_ASSERT(r->geoOut() == ' ');
      CPPUNIT_ASSERT(r->geoItinPart() == ' ');
      CPPUNIT_ASSERT(r->geoCheck() == 'D');
      CPPUNIT_ASSERT(r->loopDirection() == 'F');
      CPPUNIT_ASSERT(r->loopOffset() == 0);
      CPPUNIT_ASSERT(r->loopToSet() == 0);
      CPPUNIT_ASSERT(r->loopMatch() == 'F');
      CPPUNIT_ASSERT(r->scope() == 'A');
      CPPUNIT_ASSERT(r->type() == 'D');
      CPPUNIT_ASSERT(r->matchCriteria().size() == 2);
      CPPUNIT_ASSERT(r->matchCriteria()[0] == 'S');
      CPPUNIT_ASSERT(r->matchCriteria()[1] == 'N');
    */
  }

  void testGetAllFareTypeMatrix()
  {
    const std::vector<FareTypeMatrix*>& r = _dataHandle->getAllFareTypeMatrix(TICKETDATE);
    CPPUNIT_ASSERT(!(r.empty()));
  }

  void testGetFareByRuleItem()
  {
    const FareByRuleItemInfo* r = _dataHandle->getFareByRuleItem("ATP", 12428);
    if (r != 0)
    {
      CPPUNIT_ASSERT(r->vendor() == "ATP");
      CPPUNIT_ASSERT(r->itemNo() == 12428);
    }
  }

  void testGetGeneralFareRule()
  {
    DateTime now = TICKETDATE;
    const std::vector<GeneralFareRuleInfo*>& rules =
        _dataHandle->getGeneralFareRule("ATP", "AA", 1, "2051", 4, now);
    if (!rules.empty())
    {
      CPPUNIT_ASSERT((rules.front())->effDate() <= now);
      CPPUNIT_ASSERT((rules.front())->discDate() >= now);
    }
  }

  // missing carrier AA in database: ORACLE-INT-HIST(only on RH5)
  void testGetCarrierPreference()
  {
    DateTime now = TICKETDATE;
    const CarrierPreference* r = _dataHandle->getCarrierPreference("AA", now);
    CPPUNIT_ASSERT(r != 0);
  }

  void testGetCarrierCombination()
  {
    const std::vector<CarrierCombination*>& r = _dataHandle->getCarrierCombination("ATP", 16551);
    CPPUNIT_ASSERT(!r.empty());
  }

  void testGetEndOnEnd()
  {
    const EndOnEnd* r = _dataHandle->getEndOnEnd("ATP", 21773);
    CPPUNIT_ASSERT(r != 0);
    CPPUNIT_ASSERT(r->segs().empty());
  }

  void testGetCurrencySelection()
  {
    DateTime now = TICKETDATE;
    const std::vector<CurrencySelection*>& r = _dataHandle->getCurrencySelection("HK", now);
    CPPUNIT_ASSERT(!r.empty());
  }

  void testGetFareCalcConfig()
  {
    const std::vector<FareCalcConfig*>& r = _dataHandle->getFareCalcConfig('Z', "ZZZZ", "ZZZZ");
    CPPUNIT_ASSERT(!r.empty());
    FareCalcConfig* x = r.front();
    CPPUNIT_ASSERT(x->userApplType() == ' ');
    CPPUNIT_ASSERT(x->userAppl() == "");
    CPPUNIT_ASSERT(x->pseudoCity() == "");
  }

  void testGetBankerSellRate()
  {
    const std::vector<BankerSellRate*>& r2 = _dataHandle->getBankerSellRate("IQD", "USD", TODAY);
    CPPUNIT_ASSERT(!r2.empty());
  }

  void testGetBankerSellRateRange()
  {
    const std::vector<BankerSellRate*>& r2 = _dataHandle->getBankerSellRate("IQD", "USD", TODAY);
    CPPUNIT_ASSERT(!r2.empty());
  }

  void testGetBankerSellRateRangeVersionEquivalent()
  {
    const std::vector<BankerSellRate*>& vector =
        _dataHandle->getBankerSellRate("IQD", "USD", TODAY);
    auto range = _dataHandle->getBankerSellRateRange("IQD", "USD", TODAY);
    std::vector<BankerSellRate*> vectorFromRange;
    for (auto ptr : range)
      vectorFromRange.push_back(ptr);

    CPPUNIT_ASSERT(vectorFromRange == vector);
  }
  void testGetSalesRestriction()
  {
    const SalesRestriction* r = _dataHandle->getSalesRestriction("ATP", 1964);
    CPPUNIT_ASSERT(r != 0);
    CPPUNIT_ASSERT(r->vendor() == "ATP");
    CPPUNIT_ASSERT(r->itemNo() == 1964);
  }

  void testGetCabin()
  {
    std::string carrier("AA");
    std::string clazz("Y");
    const Cabin* r = _dataHandle->getCabin(carrier, clazz, TICKETDATE);
    CPPUNIT_ASSERT(r != 0);
    CPPUNIT_ASSERT_EQUAL(carrier, (std::string)r->carrier());
    CPPUNIT_ASSERT_EQUAL(clazz, (std::string)r->classOfService());
    CPPUNIT_ASSERT(r->cabin().isEconomyClass());
  }

  void testGetFCLimitation()
  {
    const std::vector<LimitationFare*>& r = _dataHandle->getFCLimitation(TICKETDATE);
    CPPUNIT_ASSERT(!r.empty());
    std::vector<LimitationFare*>::const_iterator i = r.begin();
    for (; i != r.end(); i++)
    {
      CPPUNIT_ASSERT((*i)->intlDepartMaxNo() < 0 ||
                     ((*i)->intlDepartMaxNo() >= 0 && (*i)->intlDepartMaxNo() <= 9));
      CPPUNIT_ASSERT((*i)->intlArrivalMaxNo() < 0 ||
                     ((*i)->intlArrivalMaxNo() >= 0 && (*i)->intlArrivalMaxNo() <= 9));
      CPPUNIT_ASSERT((*i)->maxRetransitAllowed() < 0 ||
                     ((*i)->maxRetransitAllowed() >= 0 && (*i)->maxRetransitAllowed() <= 9));
      CPPUNIT_ASSERT((*i)->maxStopsAtRetransit() < 0 ||
                     ((*i)->maxStopsAtRetransit() >= 0 && (*i)->maxStopsAtRetransit() <= 9));
    }
  }

  void testGetJLimitation()
  {
    const std::vector<LimitationJrny*>& r = _dataHandle->getJLimitation(TICKETDATE);
    CPPUNIT_ASSERT(!r.empty());
    std::vector<LimitationJrny*>::const_iterator i = r.begin();
    for (; i != r.end(); i++)
    {
      CPPUNIT_ASSERT((*i)->intlDepartMaxNo() < 0 ||
                     ((*i)->intlDepartMaxNo() >= 0 && (*i)->intlDepartMaxNo() <= 9));
      CPPUNIT_ASSERT((*i)->intlArrivalMaxNo() < 0 ||
                     ((*i)->intlArrivalMaxNo() >= 0 && (*i)->intlArrivalMaxNo() <= 9));
      CPPUNIT_ASSERT((*i)->maxRetransitAllowed() < 0 ||
                     ((*i)->maxRetransitAllowed() >= 0 && (*i)->maxRetransitAllowed() <= 9));
      CPPUNIT_ASSERT((*i)->maxStopsAtRetransit() < 0 ||
                     ((*i)->maxStopsAtRetransit() >= 0 && (*i)->maxStopsAtRetransit() <= 9));
    }
  }

  void testGetTpdPsr()
  {
    const std::vector<TpdPsr*>& r1 = _dataHandle->getTpdPsr('T', "AA", '1', '2', TICKETDATE);
    CPPUNIT_ASSERT(!r1.empty());
    const std::vector<TpdPsr*>& r2 = _dataHandle->getTpdPsr('P', "AA", '1', '2', TICKETDATE);
    CPPUNIT_ASSERT(!r2.empty());
  }

  void testGetSurfaceSectorExempt()
  {
    // TODO: Use correct data from DB
    //    const SurfaceSectorExempt* r = _dataHandle->getSurfaceSectorExempt(
    //                                    "LGW", "LHR", TICKETDATE);
    // CPPUNIT_ASSERT(!r != 0);
  }

  void testGetMileageSubstitution()
  {
    // TODO: Use correct data from DB
    const MileageSubstitution* r = _dataHandle->getMileageSubstitution("XVX", TICKETDATE);
    CPPUNIT_ASSERT(r != 0);
  }
  void testGetTariffMileageAddon()
  {
    const TariffMileageAddon* r1 =
        _dataHandle->getTariffMileageAddon("", "IZT", GlobalDirection::AT, TICKETDATE);
    CPPUNIT_ASSERT(r1);
  }

  void testGetPfcPFC()
  {
    const PfcPFC* r = _dataHandle->getPfcPFC("DFW", TICKETDATE);
    CPPUNIT_ASSERT(r != 0);
  }

  void testGetPfcMultiAirport()
  {
    const PfcMultiAirport* r = _dataHandle->getPfcMultiAirport("DFW", TICKETDATE);
    CPPUNIT_ASSERT(r != 0);
  }

  void testGetPfcEssAirSvc()
  {
    const std::vector<PfcEssAirSvc*>& r = _dataHandle->getPfcEssAirSvc("DFW", "HOT", TICKETDATE);
    CPPUNIT_ASSERT(!r.empty());
  }

  void testGetPfcCollectMeth()
  {
    const std::vector<PfcCollectMeth*> r = _dataHandle->getPfcCollectMeth("QF", TICKETDATE);
    CPPUNIT_ASSERT(!r.empty());
  }

  void testGetPfcAbsorb()
  {
    const std::vector<PfcAbsorb*>& r = _dataHandle->getPfcAbsorb("LAX", "US", TICKETDATE);
    CPPUNIT_ASSERT(!r.empty());
  }

  void testGetPfcEquipTypeExempt()
  {
    const PfcEquipTypeExempt* r = _dataHandle->getPfcEquipTypeExempt("S76", "", TICKETDATE);
    CPPUNIT_ASSERT(r != 0);
  }

  void testGetMinFareFareTypeGrp()
  {
    const MinFareFareTypeGrp* r = _dataHandle->getMinFareFareTypeGrp("INDUSTRY", TICKETDATE);
    CPPUNIT_ASSERT(r != 0);
  }

  void testGetSalesNationRestr()
  {
    const std::vector<SalesNationRestr*>& r = _dataHandle->getSalesNationRestr("XX", TICKETDATE);
    CPPUNIT_ASSERT(r.empty());
  }

  void testGetIndustryPricingAppl()
  {
    const std::vector<const IndustryPricingAppl*>& r1 =
        _dataHandle->getIndustryPricingAppl("", GlobalDirection::ZZ, TICKETDATE);
    CPPUNIT_ASSERT(!r1.empty());
    const std::vector<const IndustryPricingAppl*>& r2 =
        _dataHandle->getIndustryPricingAppl("BR", GlobalDirection::ZZ, TICKETDATE);
    CPPUNIT_ASSERT(!r2.empty());
  }

  void testGetIndustryFareAppl()
  {
    const std::vector<const IndustryFareAppl*>& r =
        _dataHandle->getIndustryFareAppl('Y', "AA", TICKETDATE);
    CPPUNIT_ASSERT(!r.empty());
  }

  void testGetIndustryFareBasisMod()
  {
    /** TODO : use real data when it's available */
    const std::vector<const IndustryFareBasisMod*>& r =
        _dataHandle->getIndustryFareBasisMod("AA", 'X', "XXXX", TICKETDATE);
    CPPUNIT_ASSERT(r.empty());
  }

  void testGetMinFareAppl()
  {
    const std::vector<MinFareAppl*>& r = _dataHandle->getMinFareAppl("ATP", 770, "AA", TICKETDATE);
    CPPUNIT_ASSERT(!r.empty());
  }

  void testGetCopMinimum()
  {
    const std::vector<CopMinimum*>& r = _dataHandle->getCopMinimum("FR", TICKETDATE);
    CPPUNIT_ASSERT(!r.empty());
  }

  void testGetCopParticipatingNation()
  {
    // Uzbekistan is in Russian nation...
    const CopParticipatingNation* r =
        _dataHandle->getCopParticipatingNation("UZ", "RU", TICKETDATE);
    CPPUNIT_ASSERT(r != 0);
    // ... but Latvia is not
    r = _dataHandle->getCopParticipatingNation("LV", "RU", TICKETDATE);
    CPPUNIT_ASSERT(r == 0);
  }

  void testGetSeasonalAppl()
  {
    const SeasonalAppl* r = _dataHandle->getSeasonalAppl("ATP", 100);
    CPPUNIT_ASSERT(r != 0);
  }

  void testGetCircleTripProvision()
  {
    const CircleTripProvision* r = _dataHandle->getCircleTripProvision("SDR", "BIO", TICKETDATE);
    CPPUNIT_ASSERT(r != 0);
  }

  void testGetDifferentials()
  {
    const CarrierCode cxr("AA");
    const std::vector<Differentials*>& r = _dataHandle->getDifferentials(cxr, TICKETDATE);
    CPPUNIT_ASSERT(!r.empty());
    std::vector<Differentials*>::const_iterator i = r.begin();
    bool p2 = false, p3 = false, p4 = false;
    for (; i != r.end(); i++)
    {
      Differentials& d = **i;
      if (d.carrier() == cxr)
      {
        if (d.calculationInd() == 'N')
        {
          CPPUNIT_ASSERT(!p2 && !p3 && !p4);
        }
        else
        {
          p3 = true;
          CPPUNIT_ASSERT(!p4);
        }
      }
      else
      {
        CPPUNIT_ASSERT(d.carrier() == "");
        if (d.calculationInd() == 'N')
        {
          p2 = true;
          CPPUNIT_ASSERT(!p3 && !p4);
        }
        else
          p4 = true;
      }
    }
  }

  void testGetOpenJawRule()
  {
    const OpenJawRule* r = _dataHandle->getOpenJawRule("ATP", 3);
    CPPUNIT_ASSERT(r != 0);
  }

  void testGetGeoRule()
  {
    const std::vector<GeoRuleItem*>& r = _dataHandle->getGeoRuleItem("ATP", 5);
    CPPUNIT_ASSERT(!r.empty());
  }

  void testGetDayTimeAppInfo()
  {
    const DayTimeAppInfo* r = _dataHandle->getDayTimeAppInfo("ATP", 47195);
    CPPUNIT_ASSERT(r != 0);
  }

  void testGetFareDisplayInclCd()
  {
    const std::vector<FareDisplayInclCd*>& r =
        _dataHandle->getFareDisplayInclCd(' ', "", ' ', "", "MIL");
    CPPUNIT_ASSERT(!r.empty());
  }

  void testGetFareDispInclRuleTrf()
  {
    const std::vector<FareDispInclRuleTrf*>& r =
        _dataHandle->getFareDispInclRuleTrf(' ', "", ' ', "", "CT");
    CPPUNIT_ASSERT(!r.empty());
  }

  void testGetFareDispInclFareType()
  {
    const std::vector<FareDispInclFareType*>& r =
        _dataHandle->getFareDispInclFareType(' ', "", ' ', "", "NLP");
    CPPUNIT_ASSERT(!r.empty());
  }

  void testGetFareDispInclDsplType()
  {
    const std::vector<FareDispInclDsplType*>& r =
        _dataHandle->getFareDispInclDsplType(' ', "", ' ', "", "NET");
    CPPUNIT_ASSERT(!r.empty());
  }

  void testGetZone()
  {
    const ZoneInfo* r = _dataHandle->getZone("SABR", "0001847", 'M', TICKETDATE);
    CPPUNIT_ASSERT(r != 0);
  }

  void testGetRuleCatAlphaCode()
  {
    const std::vector<RuleCatAlphaCode*>& r = _dataHandle->getRuleCatAlphaCode("TR");
    CPPUNIT_ASSERT(!r.empty());
  }

  void testGetFareDispRec1PsgType()
  {
    const std::vector<FareDispRec1PsgType*>& r =
        _dataHandle->getFareDispRec1PsgType(' ', "", ' ', "", "NL");
    CPPUNIT_ASSERT(!r.empty());
  }

  void testGetFareDispRec8PsgType()
  {
    const std::vector<FareDispRec8PsgType*>& r =
        _dataHandle->getFareDispRec8PsgType(' ', "", ' ', "", "MIL");
    CPPUNIT_ASSERT(!r.empty());
  }

  void testGetGroups() { CPPUNIT_ASSERT(_dataHandle->getGroups("ATP", 80) != 0); }

  void testGetTours() { CPPUNIT_ASSERT(_dataHandle->getTours("ATP", 310) != 0); }

  void testGetWaiver()
  {
    _dataHandle->refreshHist(DateTime::localTime());

    const std::vector<Waiver*>& waiver = _dataHandle->getWaiver("ATP", 10, DateTime::localTime());

    CPPUNIT_ASSERT(waiver.size() == 6);
    CPPUNIT_ASSERT(_dataHandle->getWaiver("ATP", 10, DateTime::localTime()).size() == 6);
    CPPUNIT_ASSERT(!_dataHandle->getWaiver("ATP", 10, DateTime::emptyDate()).empty());
    CPPUNIT_ASSERT(_dataHandle->getWaiver("XXX", 10, DateTime::localTime()).empty());
    CPPUNIT_ASSERT(_dataHandle->getWaiver("ATP", 137, DateTime::localTime()).empty());
  }

  void testGetVoluntaryChangesConfigPass()
  {
    DateTime currentTktDate(2008, 11, 1), originalTktIssueDate(2008, 2, 1);
    CPPUNIT_ASSERT_EQUAL(
        DateTime(2009, 3, 30),
        _dataHandle->getVoluntaryChangesConfig("JT", currentTktDate, originalTktIssueDate));
  }

  void testGetVoluntaryChangesConfigDefault()
  {
    DateTime currentTktDate(2008, 11, 1), originalTktIssueDate(2008, 2, 1);

    CPPUNIT_ASSERT_EQUAL(
        DateTime::emptyDate(),
        _dataHandle->getVoluntaryChangesConfig("AA", currentTktDate, originalTktIssueDate));
  }

  void testGetCountrySettlementPlans_NonHistorical()
  {
    const NationCode countryJp = "US";
    const DateTime today = DateTime::localTime();
    _dataHandle->setTicketDate(today); // For non-historical

    const std::vector<CountrySettlementPlanInfo*>& cspList =
        _dataHandle->getCountrySettlementPlans(countryJp);
    CPPUNIT_ASSERT(!cspList.empty());
  }

  void testGetCountrySettlementPlans_Historical()
  {
    const NationCode countryJp = "US";
    const DateTime today = DateTime::localTime();
    const DateTime reasonableHistoricalDate = today.subtractDays(3);
    _dataHandle->setTicketDate(reasonableHistoricalDate);

    const std::vector<CountrySettlementPlanInfo*>& cspList =
        _dataHandle->getCountrySettlementPlans(countryJp);
    CPPUNIT_ASSERT(!cspList.empty());
  }

  void testGetAirlineInterlineAgreements()
  {
    const NationCode country = "JP";
    const CrsCode gds = "1S";
    const CarrierCode validatingCarrier = "AA";
    const DateTime today = DateTime::localTime();
    _dataHandle->setTicketDate(today); // non historical

    const std::vector<AirlineInterlineAgreementInfo*>& aiaList =
        _dataHandle->getAirlineInterlineAgreements(country, gds, validatingCarrier);

    CPPUNIT_ASSERT(!aiaList.empty());
  }

  void testGetAirlineInterlineAgreements_Historical()
  {
    const NationCode country = "JP";
    const CrsCode gds = "1S";
    const CarrierCode validatingCarrier = "AA";
    const DateTime today = DateTime::localTime();
    const DateTime historicalDate = today.subtractDays(3);
    _dataHandle->setTicketDate(historicalDate);

    const std::vector<AirlineInterlineAgreementInfo*>& aiaList =
        _dataHandle->getAirlineInterlineAgreements(country, gds, validatingCarrier);

    CPPUNIT_ASSERT(!aiaList.empty());
  }

  void testGetAirlineCountrySettlementPlans()
  {
    const NationCode countryUs = "US";
    const CrsCode gds1s = "1S";
    const CarrierCode airlineAa = "AA";
    const SettlementPlanType spTypeArc = "ARC";
    const DateTime today = DateTime::localTime();
    _dataHandle->setTicketDate(today); // For non historical

    const std::vector<AirlineCountrySettlementPlanInfo*>& acspList =
        _dataHandle->getAirlineCountrySettlementPlans(countryUs, gds1s, airlineAa, spTypeArc);

    CPPUNIT_ASSERT(!acspList.empty());
  }

  void testGetAirlineCountrySettlementPlans_ByCountryAndPlan()
  {
    const NationCode countryUs = "US";
    const CrsCode gds1s = "1S";
    const SettlementPlanType spTypeArc = "ARC";

    const std::vector<AirlineCountrySettlementPlanInfo*>& acspList =
        _dataHandle->getAirlineCountrySettlementPlans(countryUs, gds1s, spTypeArc);

    CPPUNIT_ASSERT(!acspList.empty());
  }

  void testGetAirlineCountrySettlementPlans_ByCountryAndAirline()
  {
    const NationCode country = "RU";
    const CrsCode gds = "1S";
    const CarrierCode airline = "SU";

    const std::vector<AirlineCountrySettlementPlanInfo*>& acspList =
        _dataHandle->getAirlineCountrySettlementPlans(gds, country, airline);

    CPPUNIT_ASSERT(!acspList.empty());
  }

  void testGetNeutralValidatingAirlines()
  {
    const NationCode country = "AE";
    const CrsCode gds = "1S";
    const SettlementPlanType spType = "BSP";
    const DateTime today = DateTime::localTime();
    _dataHandle->setTicketDate(today); // For non historical

    const std::vector<NeutralValidatingAirlineInfo*>& nvaList =
        _dataHandle->getNeutralValidatingAirlines(country, gds, spType);

    CPPUNIT_ASSERT(!nvaList.empty());
  }

  void testGetNeutralValidatingAirlines_Historical()
  {
    const NationCode country = "AE";
    const CrsCode gds = "1S";
    const SettlementPlanType spType = "BSP";
    const DateTime today = DateTime::localTime();
    _dataHandle->setTicketDate(today.subtractDays(3)); // Historical date

    const std::vector<NeutralValidatingAirlineInfo*>& nvaList =
        _dataHandle->getNeutralValidatingAirlines(country, gds, spType);

    CPPUNIT_ASSERT(!nvaList.empty());
  }

  void testGetGenSalesAgents()
  {
    const CrsCode gds = "1S";
    const NationCode nation = "BM";
    const CarrierCode nonParticipatingCarrier = "KL";
    const SettlementPlanType settlementPlan = "BSP";
    const DateTime today = DateTime::localTime();

    _dataHandle->setTicketDate(today); // For non-historical
    const std::vector<GenSalesAgentInfo*>& gsaList =
        _dataHandle->getGenSalesAgents(gds, nation, settlementPlan, nonParticipatingCarrier);

    CPPUNIT_ASSERT(!gsaList.empty());
    const size_t resultSetSize = 1;
    CPPUNIT_ASSERT_EQUAL(resultSetSize, gsaList.size());
    CPPUNIT_ASSERT_EQUAL(gds, gsaList[0]->getGDSCode());
    CPPUNIT_ASSERT_EQUAL(nation, gsaList[0]->getCountryCode());
    CPPUNIT_ASSERT_EQUAL(settlementPlan, gsaList[0]->getSettlementPlanCode());
    CPPUNIT_ASSERT_EQUAL(nonParticipatingCarrier, gsaList[0]->getNonParticipatingCxr());
    const CarrierCode validatingCarrier = "DL";
    CPPUNIT_ASSERT_EQUAL(validatingCarrier, gsaList[0]->getCxrCode());
  }

  void testGetGenSalesAgents_Preload()
  {
    const CrsCode gds = "1S";
    const NationCode nation = "BM";
    const SettlementPlanType settlementPlan = "BSP";

    const std::vector<GenSalesAgentInfo*>& gsaList =
        _dataHandle->getGenSalesAgents(gds, nation, settlementPlan);

    CPPUNIT_ASSERT(!gsaList.empty());
    const size_t resultSetSize = 62;
    CPPUNIT_ASSERT_EQUAL(resultSetSize, gsaList.size());
    CPPUNIT_ASSERT_EQUAL(gds, gsaList[0]->getGDSCode());
    CPPUNIT_ASSERT_EQUAL(nation, gsaList[0]->getCountryCode());
    CPPUNIT_ASSERT_EQUAL(settlementPlan, gsaList[0]->getSettlementPlanCode());
  }

  void testGetGenSalesAgents_Historical()
  {
    const CrsCode gds = "1S";
    const NationCode nation = "US";
    const SettlementPlanType settlementPlan = "ARC";
    const CarrierCode nonParticipatingCarrier = "1X";
    const DateTime today = DateTime::localTime();
    const DateTime reasonableHistoricalDate = today.subtractDays(3);
    _dataHandle->setTicketDate(reasonableHistoricalDate);

    const std::vector<GenSalesAgentInfo*>& gsaList =
        _dataHandle->getGenSalesAgents(gds, nation, settlementPlan, nonParticipatingCarrier);

    CPPUNIT_ASSERT(!gsaList.empty());
  }

  void testGetCustomerActivationControl()
  {
    const std::string projectCode = "GSA";
    const std::vector<CustomerActivationControl*>& cacList =
        _dataHandle->getCustomerActivationControl(projectCode);
    CPPUNIT_ASSERT(!cacList.empty());
  }

  void testGetCustomer()
  {
    const PseudoCityCode pcc = "9YFG"; // Multi-settlement plan user
    const std::vector<Customer*>& customerList = _dataHandle->getCustomer(pcc);
    CPPUNIT_ASSERT(!customerList.empty());
    CPPUNIT_ASSERT(!customerList[0]->settlementPlans().empty());
    CPPUNIT_ASSERT("BSPRUTSTU" == customerList[0]->settlementPlans());
  }
};

const char* DataHandleTest::cities[] = {"FRA", "HKG", "LON", "NYC", "PAR", "SIN", "SYD"};
const DateTime
DataHandleTest::TICKETDATE(2016, 3, 3);
const DateTime
DataHandleTest::TODAY(DateTime::localTime());

CPPUNIT_TEST_SUITE_REGISTRATION(DataHandleTest);

} // tse
