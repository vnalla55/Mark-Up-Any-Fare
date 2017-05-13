#include <utility>
#include <functional>
#include <algorithm>

#include <boost/assign/std/vector.hpp>
#include <boost/range/as_array.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "Common/Config/ConfigMan.h"
#include "Common/Global.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AncRequest.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "Diagnostic/Diag852Collector.h"
#include "Diagnostic/test/Diag852ParsedParamsTester.h"
#include "FreeBagService/AllowanceDataStrategy.h"
#include "FreeBagService/test/AirSegBuilder.h"
#include "FreeBagService/test/BaggageTravelBuilder.h"
#include "FreeBagService/test/S5Builder.h"
#include "FreeBagService/test/S7Builder.h"
#include "FreeBagService/BaggageTravelInfo.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/OptionalServicesValidator.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/MockGlobal.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

using boost::assign::operator+=;

// MOCKS
namespace
{
class CheckS5 : std::unary_function<const SubCodeInfo*, bool>
{
  CarrierCode _carrier;

public:
  CheckS5(const CarrierCode& carrier) : _carrier(carrier) {}
  bool operator()(const SubCodeInfo* s5) const { return s5->carrier() != _carrier; }
};

class CheckS7 : std::unary_function<const OptionalServicesInfo*, bool>
{
  CarrierCode _carrier;

public:
  CheckS7(const CarrierCode& carrier) : _carrier(carrier) {}
  bool operator()(const OptionalServicesInfo* s7) { return s7->carrier() != _carrier; }
};

class TestDiagCollector : public Diag852Collector
{
public:
  TestDiagCollector() : _paramsTester(Diag852Collector::_params)
  {
    rootDiag() = _memHandle.create<Diagnostic>();
  }

  void addParam(const std::string& key, const std::string& value)
  {
    rootDiag()->diagParamMap().insert(std::make_pair(key, value));
    _paramsTester.forceInitialization();
  }

private:
  TestMemHandle _memHandle;
  Diag852ParsedParamsTester _paramsTester;
};

class FarePathBuilder
{
  TestMemHandle* _memHandle;
  FarePath* _farePath;

public:
  FarePathBuilder(TestMemHandle* memHandle) : _memHandle(memHandle)
  {
    _farePath = _memHandle->create<FarePath>();
    _farePath->itin() = _memHandle->create<Itin>();
  }

  FarePathBuilder& withTravelSeg(const std::vector<TravelSeg*>& segments)
  {
    _farePath->itin()->travelSeg() = segments;
    return *this;
  }

  FarePath* build() { return _farePath; }
};

class CheckedPointBuilder
{
  TestMemHandle* _memHandle;
  CheckedPoint* _checkedPoint;

public:
  CheckedPointBuilder(TestMemHandle* memHandle) : _memHandle(memHandle)
  {
    _checkedPoint = _memHandle->create<CheckedPoint>();
  }

  CheckedPoint& build() { return *_checkedPoint; }
};
}

class AllowanceDataStrategyTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AllowanceDataStrategyTest);

  CPPUNIT_TEST(testRetrieveS5Record_AtpcoUsDot);
  CPPUNIT_TEST(testRetrieveS5Record_AtpcoNonUsDot);
  CPPUNIT_TEST(testRetrieveS5Record_MmUsDot);
  CPPUNIT_TEST(testRetrieveS5Record_MmNonUsDot);
  CPPUNIT_TEST(testRetrieveS5Record_FcicAtpcoUsDot);
  CPPUNIT_TEST(testRetrieveS5Record_FcicAtpcoNonUsDot);
  CPPUNIT_TEST(testRetrieveS5Record_FcicMmUsDot);
  CPPUNIT_TEST(testRetrieveS5Record_FcicMmNonUsDot);
  CPPUNIT_TEST(testRetrieveS5Record_MssSameAsFcicUsDot);
  CPPUNIT_TEST(testRetrieveS5Record_MssSameAsFcicNonUsDot);
  CPPUNIT_TEST(testRetrieveS5Record_NoS5DataUsDot);
  CPPUNIT_TEST(testRetrieveS5Record_NoS5DataNonUsDot);

  CPPUNIT_TEST(testCheckServiceType_PassWhenCodeOdfAndTypeA);
  CPPUNIT_TEST(testCheckServiceType_FailWhenCodeOdfAndTypeX);
  CPPUNIT_TEST(testCheckServiceType_FailWhenCodeOdfAndTypeI);
  CPPUNIT_TEST(testCheckServiceType_FailWhenCodeWwwAndTypeA);

  CPPUNIT_TEST(testCheckServiceType_FailWhenConcurNotX);
  CPPUNIT_TEST(testCheckServiceType_FailWhenRfiCodeNotCAndNotBlank);
  CPPUNIT_TEST(testCheckServiceType_FailWhenSsimCodeBlank);
  CPPUNIT_TEST(testCheckServiceType_FailWhenSsrCodeNotBlank);
  CPPUNIT_TEST(testCheckServiceType_FailWhenEmdTypeNot4);
  CPPUNIT_TEST(testCheckServiceType_FailWhenBookingIndNotBlank);

  CPPUNIT_TEST(testRetrieveS5Record_ReturnNullWhenNoData);
  CPPUNIT_TEST(testRetrieveS5Record_ReturnValidRecord);

  CPPUNIT_TEST(testShouldDisplayDiagnostic_True);
  CPPUNIT_TEST(testShouldDisplayDiagnostic_False_Fl);
  CPPUNIT_TEST(testShouldDisplayDiagnostic_False_Dc);

  CPPUNIT_TEST(testRetrieveS5Record_Atp);
  CPPUNIT_TEST(testRetrieveS5Record_Mmgr);
  CPPUNIT_TEST(testRetrieveS5Record_Empty);

  CPPUNIT_TEST(test_isAllowanceCarrierOverridden);
  CPPUNIT_TEST(test_isChargesCarrierOverridden);
  CPPUNIT_TEST(test_getAllowanceCarrierOverridden);

  CPPUNIT_TEST(test_processBaggageTravelWhollyWithinUs_No_S5);
  CPPUNIT_TEST(test_processBaggageTravelWhollyWithinUs_No_S5_AllowanceCarrierOverride);
  CPPUNIT_TEST(test_processBaggageTravelWhollyWithinUs_No_S5_ChargesCarrierOverride);
  CPPUNIT_TEST(test_processBaggageTravelWhollyWithinUs_S5_No_S7);
  CPPUNIT_TEST(test_processBaggageTravelWhollyWithinUs_S5_S7);

  CPPUNIT_TEST(test_processBaggageTravelNoDotTableOld_No_UsDot_No_S5);
  CPPUNIT_TEST(test_processBaggageTravelNoDotTableOld_No_UsDot_S5_No_S7);
  CPPUNIT_TEST(test_processBaggageTravelNoDotTableOld_No_UsDot_S5_S7_No_DEFER);
  CPPUNIT_TEST(test_processBaggageTravelNoDotTableOld_No_UsDot_S5_S7_DEFER);
  CPPUNIT_TEST(test_processBaggageTravelNoDotTableOld_No_UsDot_S5_S7_DEFER_S5_No_S7);
  CPPUNIT_TEST(test_processBaggageTravelNoDotTableOld_No_UsDot_S5_S7_DEFER_S5_S7);
  CPPUNIT_TEST(test_processBaggageTravelNoDotTableOld_No_UsDot_S5_S7_DEFER_S5_S7_DEFER_S5_S7);

  CPPUNIT_TEST(test_processBaggageTravelNoDotTable_NoS5);
  CPPUNIT_TEST(test_processBaggageTravelNoDotTable_MsS5_NoS7);
  CPPUNIT_TEST(test_processBaggageTravelNoDotTable_MsS5_S7);
  CPPUNIT_TEST(test_processBaggageTravelNoDotTable_MsS5_Defer_NoS5);
  CPPUNIT_TEST(test_processBaggageTravelNoDotTable_MsS5_Defer_NoS7);
  CPPUNIT_TEST(test_processBaggageTravelNoDotTable_MsS5_Defer_S7Defer);
  CPPUNIT_TEST(test_processBaggageTravelNoDotTable_MsS5_Defer_S7);
  CPPUNIT_TEST(test_processBaggageTravelNoDotTable_MsS5_Defer_NoS5_FcS5);
  CPPUNIT_TEST(test_processBaggageTravelNoDotTable_NoMsS5_FcS5_NoS7);
  CPPUNIT_TEST(test_processBaggageTravelNoDotTable_NoMsS5_FcS5_S7);
  CPPUNIT_TEST(test_processBaggageTravelNoDotTable_NoMsS5_FcS5_Defer_NoS5);
  CPPUNIT_TEST(test_processBaggageTravelNoDotTable_NoMsS5_FcS5_Defer_NoS7);
  CPPUNIT_TEST(test_processBaggageTravelNoDotTable_NoMsS5_FcS5_Defer_S7Defer);
  CPPUNIT_TEST(test_processBaggageTravelNoDotTable_NoMsS5_FcS5_Defer_S7);

  CPPUNIT_TEST(test_processBaggageTravelDotTable_No_CarrierUsDot);
  CPPUNIT_TEST(test_processBaggageTravelDotTable_No_CarrierUsDot_override_allowance);
  CPPUNIT_TEST(test_processBaggageTravelDotTable_No_CarrierUsDot_override_charges);
  CPPUNIT_TEST(test_processBaggageTravelDotTable_CarrierUsDot_No_S5);
  CPPUNIT_TEST(test_processBaggageTravelDotTable_CarrierUsDot_S5_No_S7);
  CPPUNIT_TEST(test_processBaggageTravelDotTable_CarrierUsDot_S5_S7_No_Defer);
  CPPUNIT_TEST(test_processBaggageTravelDotTable_CarrierUsDot_S5_S7_Defer_MSSJourney_No_DotTable);
  CPPUNIT_TEST(
      test_processBaggageTravelDotTable_CarrierUsDot_S5_S7_Defer_MSSJourney_DotTable_No_S5);
  CPPUNIT_TEST(
      test_processBaggageTravelDotTable_CarrierUsDot_S5_S7_Defer_MSSJourney_DotTable_S5_No_S7);
  CPPUNIT_TEST(
      test_processBaggageTravelDotTable_CarrierUsDot_S5_S7_Defer_MSSJourney_DotTable_S5_No_S7_NextS7);
  CPPUNIT_TEST(
      test_processBaggageTravelDotTable_CarrierUsDot_S5_S7_Defer_MSSJourney_DotTable_S5_S7);

  CPPUNIT_TEST(test_processBaggageTravelDotTable_No_CarrierCta);
  CPPUNIT_TEST(test_processBaggageTravelDotTable_No_CarrierUsDotCta);
  CPPUNIT_TEST(test_processBaggageTravelDotTable_CarrierCta_S5_S7_No_Defer);
  CPPUNIT_TEST(test_processBaggageTravelDotTable_CarrierUsDotCta_S5_S7_No_Defer);

  CPPUNIT_TEST(test_processBaggageTravelForAllowanceCarrierOverridden_no_S5);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _mdh = _memHandle.create<MyDataHandle>();
    _itin = _memHandle.create<Itin>();
    _farePath = _memHandle.create<FarePath>();
    _farePath->itin() = _itin;
    _itin->farePath() += _farePath;
    _trx = _memHandle.create<PricingTrx>();
    _trx->setExcTrxType(PricingTrx::NOT_EXC_TRX);
    _trx->itin() += _itin;
    _trx->setRequest(_memHandle.create<AncRequest>());
    _strategy = _memHandle.insert(new AllowanceDataStrategy(*_trx));
    _furthestCheckedPoint = CheckedPointBuilder(&_memHandle).build();
    _bgTravels = _memHandle.create<std::vector<BaggageTravel*> >();

    PricingOptions* pricingOptions = _memHandle.create<PricingOptions>();
    _trx->setOptions(pricingOptions);
    _trx->ticketingDate() = DateTime(9999,1,1);
  }

  void tearDown() { _memHandle.clear(); }

protected:

  SubCodeInfo* createRecordS5(const ServiceSubTypeCode& subCode = "0DF",
                              const Indicator& srvType = 'A',
                              const Indicator& concur = 'X',
                              const Indicator& rfiCode = 'C',
                              const Indicator& ssimCode = ' ',
                              const SubCodeSSR& ssrCode = "",
                              const Indicator& emdType = '4',
                              const ServiceBookingInd& bookingInd = "")
  {
    return S5Builder(&_memHandle)
        .withSubCode(subCode)
        .withFltTktMerchInd(srvType)
        .withCodes(concur, ssimCode, rfiCode, emdType, bookingInd, ssrCode)
        .build();
  }

  void addAirSegToItin(const CarrierCode& marketingCxr, const CarrierCode& operatingCxr)
  {
    _itin->travelSeg() +=
        AirSegBuilder(&_memHandle).withCarriers(marketingCxr, operatingCxr).build();
  }

  BaggageTravel* createBaggageTravel()
  {
    return BaggageTravelBuilder(&_memHandle).withFarePath(_farePath).build();
  }

  std::vector<BaggageTravel*>* create1ItemBaggageTravelVector()
  {
    std::vector<BaggageTravel*>* bagTvls = _memHandle.create<std::vector<BaggageTravel*> >();
    *bagTvls += createBaggageTravel();
    return bagTvls;
  }

  AirSeg* createAirSeg(const NationCode& origin, const NationCode& destination)
  {
    return AirSegBuilder(&_memHandle).withNations(origin, destination).build();
  }

  ExchangePricingTrx* buildExchangeTrx(bool unflown)
  {
    ExchangePricingTrx* exchangeTrx = _memHandle.create<ExchangePricingTrx>();
    exchangeTrx->setRequest(_trx->getRequest());
    exchangeTrx->ticketingDate() = DateTime(2026, 1, 1);
    exchangeTrx->currentTicketingDT() = DateTime(2022, 2, 2);
    exchangeTrx->setOriginalTktIssueDT() = DateTime(2033, 3, 3);
    exchangeTrx->travelSeg().push_back(AirSegBuilder(&_memHandle).withUnflown(unflown).build());
    return exchangeTrx;
  }

  void overrideAllowanceCarrier(const CarrierCode& carrier)
  {
    AncRequest* req = static_cast<AncRequest*>(_trx->getRequest());
    req->majorSchemaVersion() = 2;
    req->carrierOverriddenForBaggageAllowance() = carrier;
  }

  void overrideChargesCarrier(const CarrierCode& carrier)
  {
    AncRequest* req = static_cast<AncRequest*>(_trx->getRequest());
    req->majorSchemaVersion() = 2;
    req->carrierOverriddenForBaggageCharges() = carrier;
  }

public:
  // TESTS
  void testRetrieveS5Record_AtpcoUsDot()
  {
    addAirSegToItin("AM", "AO");

    const SubCodeInfo* codeInfo = _strategy->retrieveS5Record(
        BaggageTravelBuilder(&_memHandle).withMSSJourney(_itin->travelSeg().begin()).build(), true);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AM"), codeInfo->carrier());
    CPPUNIT_ASSERT_EQUAL(VendorCode(ATPCO_VENDOR_CODE), codeInfo->vendor());
  }

  void testRetrieveS5Record_AtpcoNonUsDot()
  {
    addAirSegToItin("AM", "AO");

    const SubCodeInfo* codeInfo = _strategy->retrieveS5Record(
        BaggageTravelBuilder(&_memHandle).withMSS(_itin->travelSeg().begin()).build(), false);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AO"), codeInfo->carrier());
    CPPUNIT_ASSERT_EQUAL(VendorCode(ATPCO_VENDOR_CODE), codeInfo->vendor());
  }

  void testRetrieveS5Record_MmUsDot()
  {
    addAirSegToItin("MM", "MO");

    const SubCodeInfo* codeInfo = _strategy->retrieveS5Record(
        BaggageTravelBuilder(&_memHandle).withMSSJourney(_itin->travelSeg().begin()).build(), true);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("MM"), codeInfo->carrier());
    CPPUNIT_ASSERT_EQUAL(VendorCode(MERCH_MANAGER_VENDOR_CODE), codeInfo->vendor());
  }

  void testRetrieveS5Record_MmNonUsDot()
  {
    addAirSegToItin("MM", "MO");

    const SubCodeInfo* codeInfo = _strategy->retrieveS5Record(
        BaggageTravelBuilder(&_memHandle).withMSS(_itin->travelSeg().begin()).build(), false);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("MO"), codeInfo->carrier());
    CPPUNIT_ASSERT_EQUAL(VendorCode(MERCH_MANAGER_VENDOR_CODE), codeInfo->vendor());
  }

  void testRetrieveS5Record_FcicAtpcoUsDot()
  {
    addAirSegToItin("AM", "AO");
    addAirSegToItin("XM", "XO");

    const SubCodeInfo* codeInfo =
        _strategy->retrieveS5Record(BaggageTravelBuilder(&_memHandle)
                                        .withMSSJourney(_itin->travelSeg().begin() + 1)
                                        .withFarePath(_farePath)
                                        .build(),
                                    true);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AM"), codeInfo->carrier());
    CPPUNIT_ASSERT_EQUAL(VendorCode(ATPCO_VENDOR_CODE), codeInfo->vendor());
  }

  void testRetrieveS5Record_FcicAtpcoNonUsDot()
  {
    addAirSegToItin("AM", "AO");
    addAirSegToItin("XM", "XO");

    const SubCodeInfo* codeInfo = _strategy->retrieveS5Record(
        BaggageTravelBuilder(&_memHandle)
            .withMSS(_itin->travelSeg().begin() + 1)
            .withTravelSeg(_itin->travelSeg().begin(), _itin->travelSeg().end())
            .build(),
        false);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AO"), codeInfo->carrier());
    CPPUNIT_ASSERT_EQUAL(VendorCode(ATPCO_VENDOR_CODE), codeInfo->vendor());
  }

  void testRetrieveS5Record_FcicMmUsDot()
  {
    addAirSegToItin("MM", "MO");
    addAirSegToItin("XM", "XO");

    const SubCodeInfo* codeInfo =
        _strategy->retrieveS5Record(BaggageTravelBuilder(&_memHandle)
                                        .withMSSJourney(_itin->travelSeg().begin() + 1)
                                        .withFarePath(_farePath)
                                        .build(),
                                    true);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("MM"), codeInfo->carrier());
    CPPUNIT_ASSERT_EQUAL(VendorCode(MERCH_MANAGER_VENDOR_CODE), codeInfo->vendor());
  }

  void testRetrieveS5Record_FcicMmNonUsDot()
  {
    addAirSegToItin("MM", "MO");
    addAirSegToItin("XM", "XO");

    const SubCodeInfo* codeInfo = _strategy->retrieveS5Record(
        BaggageTravelBuilder(&_memHandle)
            .withMSS(_itin->travelSeg().begin() + 1)
            .withTravelSeg(_itin->travelSeg().begin(), _itin->travelSeg().end())
            .build(),
        false);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("MO"), codeInfo->carrier());
    CPPUNIT_ASSERT_EQUAL(VendorCode(MERCH_MANAGER_VENDOR_CODE), codeInfo->vendor());
  }

  void testRetrieveS5Record_MssSameAsFcicUsDot()
  {
    addAirSegToItin("XM", "XO");
    addAirSegToItin("XM", "XO");

    const SubCodeInfo* codeInfo =
        _strategy->retrieveS5Record(BaggageTravelBuilder(&_memHandle)
                                        .withMSSJourney(_itin->travelSeg().begin() + 1)
                                        .withFarePath(_farePath)
                                        .build(),
                                    true);
    CPPUNIT_ASSERT(!codeInfo);
  }

  void testRetrieveS5Record_MssSameAsFcicNonUsDot()
  {
    addAirSegToItin("XM", "XO");
    addAirSegToItin("XM", "XO");

    const SubCodeInfo* codeInfo = _strategy->retrieveS5Record(
        BaggageTravelBuilder(&_memHandle)
            .withMSS(_itin->travelSeg().begin() + 1)
            .withTravelSeg(_itin->travelSeg().begin(), _itin->travelSeg().end())
            .build(),
        false);
    CPPUNIT_ASSERT(!codeInfo);
  }

  void testRetrieveS5Record_NoS5DataUsDot()
  {
    addAirSegToItin("XM", "XO");
    addAirSegToItin("YM", "YO");

    const SubCodeInfo* codeInfo =
        _strategy->retrieveS5Record(BaggageTravelBuilder(&_memHandle)
                                        .withMSSJourney(_itin->travelSeg().begin() + 1)
                                        .withFarePath(_farePath)
                                        .build(),
                                    true);
    CPPUNIT_ASSERT(!codeInfo);
  }

  void testRetrieveS5Record_NoS5DataNonUsDot()
  {
    addAirSegToItin("XM", "XO");
    addAirSegToItin("YM", "YO");

    const SubCodeInfo* codeInfo = _strategy->retrieveS5Record(
        BaggageTravelBuilder(&_memHandle)
            .withMSS(_itin->travelSeg().begin() + 1)
            .withTravelSeg(_itin->travelSeg().begin(), _itin->travelSeg().end())
            .build(),
        false);
    CPPUNIT_ASSERT(!codeInfo);
  }

  void testCheckServiceType_PassWhenCodeOdfAndTypeA()
  {
    const SubCodeInfo* codeInfo = createRecordS5("0DF", 'A');
    CPPUNIT_ASSERT(_strategy->checkServiceType(codeInfo));
  }

  void testCheckServiceType_FailWhenCodeOdfAndTypeX()
  {
    const SubCodeInfo* codeInfo = createRecordS5("0DF", 'X');
    CPPUNIT_ASSERT(!_strategy->checkServiceType(codeInfo));
  }

  void testCheckServiceType_FailWhenCodeOdfAndTypeI()
  {
    const SubCodeInfo* codeInfo = createRecordS5("0DF", 'I');
    CPPUNIT_ASSERT(!_strategy->checkServiceType(codeInfo));
  }

  void testCheckServiceType_FailWhenCodeWwwAndTypeA()
  {
    const SubCodeInfo* codeInfo = createRecordS5("WWW", 'A');
    CPPUNIT_ASSERT(!_strategy->checkServiceType(codeInfo));
  }

  void testCheckServiceType_FailWhenConcurNotX()
  {
    Indicator concurTable[] = { "12" };

    for (const Indicator& concur : boost::as_array(concurTable))
    {
      const SubCodeInfo* codeInfo = createRecordS5("0DF", 'A', concur);
      CPPUNIT_ASSERT(!_strategy->checkServiceType(codeInfo));
    }
  }

  void testCheckServiceType_FailWhenRfiCodeNotCAndNotBlank()
  {
    Indicator rfiCodeTable[] = { "IDBAGEF" };

    for (const Indicator& rfiCode : boost::as_array(rfiCodeTable))
    {
      const SubCodeInfo* codeInfo = createRecordS5("0DF", 'A', 'X', rfiCode);
      CPPUNIT_ASSERT(!_strategy->checkServiceType(codeInfo));
    }
  }

  void testCheckServiceType_FailWhenSsimCodeBlank()
  {
    Indicator ssimCodeTable[] = { "RHKDMGFL" };

    for (const Indicator& ssimCode : boost::as_array(ssimCodeTable))
    {
      const SubCodeInfo* codeInfo = createRecordS5("0DF", 'A', 'X', 'C', ssimCode);
      CPPUNIT_ASSERT(!_strategy->checkServiceType(codeInfo));
    }
  }

  void testCheckServiceType_FailWhenSsrCodeNotBlank()
  {
    const SubCodeInfo* codeInfo = createRecordS5("0DF", 'A', 'X', 'C', ' ', "ASCV");
    CPPUNIT_ASSERT(!_strategy->checkServiceType(codeInfo));
  }

  void testCheckServiceType_FailWhenEmdTypeNot4()
  {
    Indicator emdTypeTable[] = { "1235" };

    for (const Indicator& emdType : boost::as_array(emdTypeTable))
    {
      const SubCodeInfo* codeInfo = createRecordS5("0DF", 'A', 'X', 'C', ' ', "VGML", emdType);
      CPPUNIT_ASSERT(!_strategy->checkServiceType(codeInfo));
    }
  }

  void testCheckServiceType_FailWhenBookingIndNotBlank()
  {
    ServiceBookingInd bookingIndTable[] = { "01", "02", "03", "04" };

    for (const ServiceBookingInd& bookingInd : boost::as_array(bookingIndTable))
    {
      const SubCodeInfo* codeInfo =
          createRecordS5("0DF", 'A', 'X', 'C', ' ', "VGML", '4', bookingInd);
      CPPUNIT_ASSERT(!_strategy->checkServiceType(codeInfo));
    }
  }

  void testRetrieveS5Record_ReturnNullWhenNoData()
  {
    CPPUNIT_ASSERT(!_strategy->retrieveS5Record("XXX", "XX"));
  }

  void testRetrieveS5Record_ReturnValidRecord()
  {
    CPPUNIT_ASSERT(_strategy->retrieveS5Record(ATPCO_VENDOR_CODE, "AA"));
  }

  void testShouldDisplayDiagnostic_True()
  {
    TestDiagCollector diag;
    diag.addParam("FL", "01");
    diag.addParam("CP", "01");
    std::vector<BaggageTravel*>* baggageTravels = create1ItemBaggageTravelVector();

    CPPUNIT_ASSERT(
        _strategy->shouldDisplayDiagnostic((*baggageTravels)[0], BaggageTravelInfo(0, 0), &diag));
  }

  void testShouldDisplayDiagnostic_False_Fl()
  {
    TestDiagCollector diag;
    diag.addParam("FL", "02");
    diag.addParam("CP", "01");
    std::vector<BaggageTravel*>* baggageTravels = create1ItemBaggageTravelVector();

    CPPUNIT_ASSERT(
        !_strategy->shouldDisplayDiagnostic((*baggageTravels)[0], BaggageTravelInfo(0, 0), &diag));
  }

  void testShouldDisplayDiagnostic_False_Dc()
  {
    TestDiagCollector diag;
    diag.addParam("FL", "01");
    diag.addParam("CP", "01");
    diag.addParam("DC", "");
    std::vector<BaggageTravel*>* baggageTravels = create1ItemBaggageTravelVector();

    CPPUNIT_ASSERT(
        !_strategy->shouldDisplayDiagnostic((*baggageTravels)[0], BaggageTravelInfo(0, 0), &diag));
  }

  void testRetrieveS5Record_Atp()
  {
    const SubCodeInfo* s5 = _strategy->retrieveS5Record("AA");
    CPPUNIT_ASSERT(s5);
    CPPUNIT_ASSERT_EQUAL(std::string(ATPCO_VENDOR_CODE), std::string(s5->vendor()));
  }

  void testRetrieveS5Record_Mmgr()
  {
    const SubCodeInfo* s5 = _strategy->retrieveS5Record("MU");
    CPPUNIT_ASSERT(s5);
    CPPUNIT_ASSERT_EQUAL(std::string(MERCH_MANAGER_VENDOR_CODE), std::string(s5->vendor()));
  }

  void testRetrieveS5Record_Empty()
  {
    const SubCodeInfo* s5 = _strategy->retrieveS5Record("LH");
    CPPUNIT_ASSERT(!s5);
  }

  OptionalServicesInfo* getS7(VendorCode vendor,
                              CarrierCode carrier,
                              ServiceSubTypeCode serviceSubCode,
                              Indicator fltTkt,
                              uint32_t taxTblItemNo)
  {
    return S7Builder(&_memHandle)
        .withVendCarr(vendor, carrier)
        .withSubTypeCode(serviceSubCode)
        .withFltTktMerchInd(fltTkt)
        .withTaxTblItemNo(taxTblItemNo)
        .build();
  }

  void test_isAllowanceCarrierOverridden()
  {
    _trx->setRequest(_memHandle.create<AncRequest>());
    AncRequest* req = static_cast<AncRequest*>(_trx->getRequest());
    req->majorSchemaVersion() = 2;
    req->carrierOverriddenForBaggageAllowance() = "AA";
    CPPUNIT_ASSERT(_strategy->isAllowanceCarrierOverridden());
  }

  void test_isChargesCarrierOverridden()
  {
    _trx->setRequest(_memHandle.create<AncRequest>());
    AncRequest* req = static_cast<AncRequest*>(_trx->getRequest());
    req->majorSchemaVersion() = 2;
    req->carrierOverriddenForBaggageCharges() = "AA";
    CPPUNIT_ASSERT(_strategy->isChargesCarrierOverridden());
  }

  void test_getAllowanceCarrierOverridden()
  {
    _trx->setRequest(_memHandle.create<AncRequest>());
    AncRequest* req = static_cast<AncRequest*>(_trx->getRequest());
    req->majorSchemaVersion() = 2;
    req->carrierOverriddenForBaggageAllowance() = "AA";
    CPPUNIT_ASSERT_EQUAL(std::string("AA"),
                         std::string(_strategy->getAllowanceCarrierOverridden()));
  }

  void test_processBaggageTravelWhollyWithinUs_No_S5()
  {
    std::vector<TravelSeg*> segments;

    segments.push_back(
        AirSegBuilder(&_memHandle).withMarketingCarrier("LH").withOperatingCarrier("AA").build());

    FarePath* farePath = FarePathBuilder(&_memHandle).withTravelSeg(segments).build();

    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle)
                                  .withTrx(_trx)
                                  .withFarePath(farePath)
                                  .withTravelSeg(segments)
                                  .build();

    _bgTravels->push_back(bgTravel);
    _strategy->processBaggageTravelWhollyWithinUsOrCa(
        (*_bgTravels)[0], BaggageTravelInfo(0, 0), _furthestCheckedPoint, 0);

    CPPUNIT_ASSERT(!bgTravel->_processCharges);
    CPPUNIT_ASSERT(!bgTravel->_allowance);
  }

  void test_processBaggageTravelWhollyWithinUs_No_S5_AllowanceCarrierOverride()
  {
    overrideAllowanceCarrier("KL");

    std::vector<TravelSeg*> segments;
    segments.push_back(
        AirSegBuilder(&_memHandle).withMarketingCarrier("LH").withOperatingCarrier("AA").build());

    FarePath* farePath = FarePathBuilder(&_memHandle).withTravelSeg(segments).build();

    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle)
                                  .withTrx(_trx)
                                  .withFarePath(farePath)
                                  .withTravelSeg(segments)
                                  .build();

    _bgTravels->push_back(bgTravel);
    _strategy->processBaggageTravelWhollyWithinUsOrCa(
        (*_bgTravels)[0], BaggageTravelInfo(0, 0), _furthestCheckedPoint, 0);

    CPPUNIT_ASSERT(!bgTravel->_processCharges);
    CPPUNIT_ASSERT(!bgTravel->_allowance);
  }

  void test_processBaggageTravelWhollyWithinUs_No_S5_ChargesCarrierOverride()
  {
    overrideChargesCarrier("KL");

    std::vector<TravelSeg*> segments;
    segments.push_back(
        AirSegBuilder(&_memHandle).withMarketingCarrier("LH").withOperatingCarrier("AA").build());

    FarePath* farePath = FarePathBuilder(&_memHandle).withTravelSeg(segments).build();

    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle)
                                  .withTrx(_trx)
                                  .withFarePath(farePath)
                                  .withTravelSeg(segments)
                                  .build();

    _bgTravels->push_back(bgTravel);
    _strategy->processBaggageTravelWhollyWithinUsOrCa(
        (*_bgTravels)[0], BaggageTravelInfo(0, 0), _furthestCheckedPoint, 0);

    CPPUNIT_ASSERT(bgTravel->_processCharges);
    CPPUNIT_ASSERT(!bgTravel->_allowance);
  }

  void test_processBaggageTravelWhollyWithinUs_S5_No_S7()
  {
    _mdh->addS5("LH");

    std::vector<TravelSeg*> segments;
    segments.push_back(
        AirSegBuilder(&_memHandle).withMarketingCarrier("LH").withOperatingCarrier("AA").build());

    FarePath* farePath = FarePathBuilder(&_memHandle).withTravelSeg(segments).build();

    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle)
                                  .withTrx(_trx)
                                  .withFarePath(farePath)
                                  .withTravelSeg(segments)
                                  .build();

    _bgTravels->push_back(bgTravel);
    _strategy->processBaggageTravelWhollyWithinUsOrCa(
        (*_bgTravels)[0], BaggageTravelInfo(0, 0), _furthestCheckedPoint, 0);

    CPPUNIT_ASSERT(bgTravel->_processCharges);
    CPPUNIT_ASSERT(!bgTravel->_allowance);
  }

  void test_processBaggageTravelWhollyWithinUs_S5_S7()
  {
    _mdh->addS5("LH");
    _mdh->addS7("LH", 'F');

    std::vector<TravelSeg*> segments;
    segments.push_back(
        AirSegBuilder(&_memHandle).withMarketingCarrier("LH").withOperatingCarrier("AA").build());

    FarePath* farePath = FarePathBuilder(&_memHandle).withTravelSeg(segments).build();

    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle)
                                  .withTrx(_trx)
                                  .withFarePath(farePath)
                                  .withTravelSeg(segments)
                                  .build();

    _bgTravels->push_back(bgTravel);
    _strategy->processBaggageTravelWhollyWithinUsOrCa(
        (*_bgTravels)[0], BaggageTravelInfo(0, 0), _furthestCheckedPoint, 0);

    CPPUNIT_ASSERT(bgTravel->_processCharges);
    CPPUNIT_ASSERT(bgTravel->_allowance);
    CPPUNIT_ASSERT_EQUAL(bgTravel->_allowance->carrierCode(), CarrierCode("LH"));
  }

  void test_processBaggageTravelNoDotTableOld_No_UsDot_No_S5()
  {
    _trx->ticketingDate() = DateTime(2015,1,1); // before IATA Reso 302
    std::vector<TravelSeg*> segments;
    segments.push_back(
        AirSegBuilder(&_memHandle).withMarketingCarrier("LH").withOperatingCarrier("AA").build());

    FarePath* farePath = FarePathBuilder(&_memHandle).withTravelSeg(segments).build();

    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle)
                                  .withTrx(_trx)
                                  .withFarePath(farePath)
                                  .withTravelSeg(segments)
                                  .build();

    _bgTravels->push_back(bgTravel);
    _strategy->processBaggageTravelNoDotTableOld(
        (*_bgTravels)[0], BaggageTravelInfo(0, 0), _furthestCheckedPoint, false, 0);

    CPPUNIT_ASSERT(bgTravel->_processCharges);
    CPPUNIT_ASSERT(!bgTravel->_allowance);
  }

  void test_processBaggageTravelNoDotTableOld_No_UsDot_S5_No_S7()
  {
    _trx->ticketingDate() = DateTime(2015,1,1); // before IATA Reso 302
    _mdh->addS5("LH");

    std::vector<TravelSeg*> segments;
    segments.push_back(
        AirSegBuilder(&_memHandle).withMarketingCarrier("LH").withOperatingCarrier("AA").build());

    FarePath* farePath = FarePathBuilder(&_memHandle).withTravelSeg(segments).build();

    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle)
                                  .withTrx(_trx)
                                  .withFarePath(farePath)
                                  .withTravelSeg(segments)
                                  .build();

    _bgTravels->push_back(bgTravel);
    _strategy->processBaggageTravelNoDotTableOld(
        (*_bgTravels)[0], BaggageTravelInfo(0, 0), _furthestCheckedPoint, false, 0);

    CPPUNIT_ASSERT(!bgTravel->_processCharges);
    CPPUNIT_ASSERT(!bgTravel->_allowance);
  }

  void test_processBaggageTravelNoDotTableOld_No_UsDot_S5_S7_No_DEFER()
  {
    _trx->ticketingDate() = DateTime(2015,1,1); // before IATA Reso 302
    _mdh->addS5("AA");
    _mdh->addS7("AA", 'F');

    std::vector<TravelSeg*> segments;
    segments.push_back(
        AirSegBuilder(&_memHandle).withMarketingCarrier("LH").withOperatingCarrier("AA").build());

    FarePath* farePath = FarePathBuilder(&_memHandle).withTravelSeg(segments).build();

    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle)
                                  .withTrx(_trx)
                                  .withFarePath(farePath)
                                  .withTravelSeg(segments)
                                  .build();

    _bgTravels->push_back(bgTravel);
    _strategy->processBaggageTravelNoDotTableOld(
        (*_bgTravels)[0], BaggageTravelInfo(0, 0), _furthestCheckedPoint, false, 0);

    CPPUNIT_ASSERT_EQUAL(bgTravel->_allowanceCxr, CarrierCode("AA"));
    CPPUNIT_ASSERT(bgTravel->_processCharges);
    CPPUNIT_ASSERT(bgTravel->_allowance);
  }

  void test_processBaggageTravelNoDotTableOld_No_UsDot_S5_S7_DEFER()
  {
    _trx->ticketingDate() = DateTime(2015,1,1); // before IATA Reso 302
    _mdh->addS5("AA");
    _mdh->addS7("AA", 'D');

    std::vector<TravelSeg*> segments;

    segments.push_back(
        AirSegBuilder(&_memHandle).withMarketingCarrier("LH").withOperatingCarrier("AA").build());

    segments.push_back(
        AirSegBuilder(&_memHandle).withMarketingCarrier("LT").withOperatingCarrier("BA").build());

    FarePath* farePath = FarePathBuilder(&_memHandle).withTravelSeg(segments).build();

    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle)
                                  .withTrx(_trx)
                                  .withFarePath(farePath)
                                  .withTravelSeg(segments)
                                  .build();

    _bgTravels->push_back(bgTravel);
    _strategy->processBaggageTravelNoDotTableOld(
        (*_bgTravels)[0], BaggageTravelInfo(0, 0), _furthestCheckedPoint, false, 0);

    CPPUNIT_ASSERT_EQUAL(bgTravel->_allowanceCxr, CarrierCode("AA"));
    CPPUNIT_ASSERT(!bgTravel->_processCharges);
    CPPUNIT_ASSERT(!bgTravel->_allowance);
  }

  void test_processBaggageTravelNoDotTableOld_No_UsDot_S5_S7_DEFER_S5_No_S7()
  {
    _trx->ticketingDate() = DateTime(2015,1,1); // before IATA Reso 302
    _mdh->addS5("AA");
    _mdh->addS5("LH");
    _mdh->addS7("AA", 'D');

    std::vector<TravelSeg*> segments;

    segments.push_back(
        AirSegBuilder(&_memHandle).withMarketingCarrier("LH").withOperatingCarrier("AA").build());

    segments.push_back(
        AirSegBuilder(&_memHandle).withMarketingCarrier("LT").withOperatingCarrier("BA").build());

    FarePath* farePath = FarePathBuilder(&_memHandle).withTravelSeg(segments).build();

    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle)
                                  .withTrx(_trx)
                                  .withFarePath(farePath)
                                  .withTravelSeg(segments)
                                  .build();

    _bgTravels->push_back(bgTravel);
    _strategy->processBaggageTravelNoDotTableOld(
        (*_bgTravels)[0], BaggageTravelInfo(0, 0), _furthestCheckedPoint, false, 0);

    CPPUNIT_ASSERT_EQUAL(bgTravel->_allowanceCxr, CarrierCode("LH"));
    CPPUNIT_ASSERT(bgTravel->_processCharges);
    CPPUNIT_ASSERT(!bgTravel->_allowance);
  }

  void test_processBaggageTravelNoDotTableOld_No_UsDot_S5_S7_DEFER_S5_S7()
  {
    _trx->ticketingDate() = DateTime(2015,1,1); // before IATA Reso 302
    _mdh->addS5("AA");
    _mdh->addS5("LH");
    _mdh->addS7("AA", 'D');
    _mdh->addS7("LH", 'F');

    std::vector<TravelSeg*> segments;

    segments.push_back(
        AirSegBuilder(&_memHandle).withMarketingCarrier("LH").withOperatingCarrier("AA").build());

    segments.push_back(
        AirSegBuilder(&_memHandle).withMarketingCarrier("LT").withOperatingCarrier("BA").build());

    FarePath* farePath = FarePathBuilder(&_memHandle).withTravelSeg(segments).build();

    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle)
                                  .withTrx(_trx)
                                  .withFarePath(farePath)
                                  .withTravelSeg(segments)
                                  .build();

    _bgTravels->push_back(bgTravel);
    _strategy->processBaggageTravelNoDotTableOld(
        (*_bgTravels)[0], BaggageTravelInfo(0, 0), _furthestCheckedPoint, false, 0);

    CPPUNIT_ASSERT_EQUAL(bgTravel->_allowanceCxr, CarrierCode("LH"));
    CPPUNIT_ASSERT(bgTravel->_processCharges);
    CPPUNIT_ASSERT(bgTravel->_allowance);
  }

  void test_processBaggageTravelNoDotTableOld_No_UsDot_S5_S7_DEFER_S5_S7_DEFER_S5_S7()
  {
    _trx->ticketingDate() = DateTime(2015,1,1); // before IATA Reso 302
    _mdh->addS5("BA");
    _mdh->addS5("AA");
    _mdh->addS5("LH");
    _mdh->addS7("BA", 'D');
    _mdh->addS7("AA", 'D');
    _mdh->addS7("LH", 'F');

    std::vector<TravelSeg*> segments;

    segments.push_back(
        AirSegBuilder(&_memHandle).withMarketingCarrier("LH").withOperatingCarrier("AA").build());

    segments.push_back(
        AirSegBuilder(&_memHandle).withMarketingCarrier("LT").withOperatingCarrier("BA").build());

    FarePath* farePath = FarePathBuilder(&_memHandle).withTravelSeg(segments).build();

    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle)
                                  .withTrx(_trx)
                                  .withFarePath(farePath)
                                  .withTravelSeg(segments)
                                  .build();

    _bgTravels->push_back(bgTravel);
    _strategy->processBaggageTravelNoDotTableOld(
        (*_bgTravels)[0], BaggageTravelInfo(0, 0), _furthestCheckedPoint, false, 0);

    CPPUNIT_ASSERT_EQUAL(bgTravel->_allowanceCxr, CarrierCode("LH"));
    CPPUNIT_ASSERT(bgTravel->_processCharges);
    CPPUNIT_ASSERT(bgTravel->_allowance);
  }

  void initProcessBtNoDotTable()
  {
    std::vector<TravelSeg*>& segs = *_memHandle.create<std::vector<TravelSeg*>>();
    segs +=
        AirSegBuilder(&_memHandle).withMarketingCarrier("FM").withOperatingCarrier("FO").build(),
        AirSegBuilder(&_memHandle).withMarketingCarrier("MM").withOperatingCarrier("MO").build();

    FarePath* fp = FarePathBuilder(&_memHandle).withTravelSeg(segs).build();
    BaggageTravel* bt = BaggageTravelBuilder(&_memHandle)
                            .withTrx(_trx)
                            .withFarePath(fp)
                            .withTravelSeg(segs)
                            .withMSS(segs.begin() + 1)
                            .build();

    _bgTravels->push_back(bt);
  }

  void test_processBaggageTravelNoDotTable_NoS5()
  {
    _mdh->addS5("XX");
    initProcessBtNoDotTable();

    BaggageTravel& bt = *_bgTravels->front();
    AllowanceDataStrategy::MatchS7Context cxt(
        BaggageTravelInfo(0, 0), _furthestCheckedPoint, BaggageTripType::OTHER, 0);
    _strategy->processBaggageTravelNoDotTable(bt, cxt);

    CPPUNIT_ASSERT(!bt._processCharges);
    CPPUNIT_ASSERT(!bt._defer);
    CPPUNIT_ASSERT(!bt._allowance);
  }

  void test_processBaggageTravelNoDotTable_MsS5_NoS7()
  {
    _mdh->addS5("MM");
    initProcessBtNoDotTable();

    BaggageTravel& bt = *_bgTravels->front();
    AllowanceDataStrategy::MatchS7Context cxt(
        BaggageTravelInfo(0, 0), _furthestCheckedPoint, BaggageTripType::OTHER, 0);
    _strategy->processBaggageTravelNoDotTable(bt, cxt);

    CPPUNIT_ASSERT(bt._processCharges);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("MM"), bt._allowanceCxr);
    CPPUNIT_ASSERT(!bt._defer);
    CPPUNIT_ASSERT(!bt._allowance);
  }

  void test_processBaggageTravelNoDotTable_MsS5_S7()
  {
    _mdh->addS5("MM");
    _mdh->addS7("MM", 'F');
    initProcessBtNoDotTable();

    BaggageTravel& bt = *_bgTravels->front();
    AllowanceDataStrategy::MatchS7Context cxt(
        BaggageTravelInfo(0, 0), _furthestCheckedPoint, BaggageTripType::OTHER, 0);
    _strategy->processBaggageTravelNoDotTable(bt, cxt);

    CPPUNIT_ASSERT(bt._processCharges);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("MM"), bt._allowanceCxr);
    CPPUNIT_ASSERT(!bt._defer);
    CPPUNIT_ASSERT(bt._allowance != 0);
  }

  void test_processBaggageTravelNoDotTable_MsS5_Defer_NoS5()
  {
    _mdh->addS5("MM");
    _mdh->addS7("MM", 'O');
    initProcessBtNoDotTable();

    BaggageTravel& bt = *_bgTravels->front();
    AllowanceDataStrategy::MatchS7Context cxt(
        BaggageTravelInfo(0, 0), _furthestCheckedPoint, BaggageTripType::OTHER, 0);
    _strategy->processBaggageTravelNoDotTable(bt, cxt);

    CPPUNIT_ASSERT(!bt._processCharges);
    CPPUNIT_ASSERT(!bt._defer);
    CPPUNIT_ASSERT(!bt._allowance);
  }

  void test_processBaggageTravelNoDotTable_MsS5_Defer_NoS7()
  {
    _mdh->addS5("MM");
    _mdh->addS5("MO");
    _mdh->addS7("MM", 'O');
    initProcessBtNoDotTable();

    BaggageTravel& bt = *_bgTravels->front();
    AllowanceDataStrategy::MatchS7Context cxt(
        BaggageTravelInfo(0, 0), _furthestCheckedPoint, BaggageTripType::OTHER, 0);
    _strategy->processBaggageTravelNoDotTable(bt, cxt);

    CPPUNIT_ASSERT(bt._processCharges);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("MO"), bt._allowanceCxr);
    CPPUNIT_ASSERT(bt._defer);
    CPPUNIT_ASSERT(!bt._allowance);
  }

  void test_processBaggageTravelNoDotTable_MsS5_Defer_S7Defer()
  {
    _mdh->addS5("MM");
    _mdh->addS5("MO");
    _mdh->addS7("MM", 'O');
    _mdh->addS7("MO", 'O');
    initProcessBtNoDotTable();

    BaggageTravel& bt = *_bgTravels->front();
    AllowanceDataStrategy::MatchS7Context cxt(
        BaggageTravelInfo(0, 0), _furthestCheckedPoint, BaggageTripType::OTHER, 0);
    _strategy->processBaggageTravelNoDotTable(bt, cxt);

    CPPUNIT_ASSERT(bt._processCharges);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("MO"), bt._allowanceCxr);
    CPPUNIT_ASSERT(bt._defer);
    CPPUNIT_ASSERT(!bt._allowance);
  }

  void test_processBaggageTravelNoDotTable_MsS5_Defer_S7()
  {
    _mdh->addS5("MM");
    _mdh->addS5("MO");
    _mdh->addS7("MM", 'O');
    _mdh->addS7("MO", 'F');
    initProcessBtNoDotTable();

    BaggageTravel& bt = *_bgTravels->front();
    AllowanceDataStrategy::MatchS7Context cxt(
        BaggageTravelInfo(0, 0), _furthestCheckedPoint, BaggageTripType::OTHER, 0);
    _strategy->processBaggageTravelNoDotTable(bt, cxt);

    CPPUNIT_ASSERT(bt._processCharges);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("MO"), bt._allowanceCxr);
    CPPUNIT_ASSERT(bt._defer);
    CPPUNIT_ASSERT(bt._allowance);
  }

  void test_processBaggageTravelNoDotTable_MsS5_Defer_NoS5_FcS5()
  {
    _mdh->addS5("MM");
    _mdh->addS5("FM");
    _mdh->addS7("MM", 'O');
    initProcessBtNoDotTable();

    BaggageTravel& bt = *_bgTravels->front();
    AllowanceDataStrategy::MatchS7Context cxt(
        BaggageTravelInfo(0, 0), _furthestCheckedPoint, BaggageTripType::OTHER, 0);
    _strategy->processBaggageTravelNoDotTable(bt, cxt);

    CPPUNIT_ASSERT(bt._processCharges);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("FM"), bt._allowanceCxr);
    CPPUNIT_ASSERT(!bt._defer);
    CPPUNIT_ASSERT(!bt._allowance);
  }

  void test_processBaggageTravelNoDotTable_NoMsS5_FcS5_NoS7()
  {
    _mdh->addS5("FM");
    initProcessBtNoDotTable();

    BaggageTravel& bt = *_bgTravels->front();
    AllowanceDataStrategy::MatchS7Context cxt(
        BaggageTravelInfo(0, 0), _furthestCheckedPoint, BaggageTripType::OTHER, 0);
    _strategy->processBaggageTravelNoDotTable(bt, cxt);

    CPPUNIT_ASSERT(bt._processCharges);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("FM"), bt._allowanceCxr);
    CPPUNIT_ASSERT(!bt._defer);
    CPPUNIT_ASSERT(!bt._allowance);
  }

  void test_processBaggageTravelNoDotTable_NoMsS5_FcS5_S7()
  {
    _mdh->addS5("FM");
    _mdh->addS7("FM", 'F');
    initProcessBtNoDotTable();

    BaggageTravel& bt = *_bgTravels->front();
    AllowanceDataStrategy::MatchS7Context cxt(
        BaggageTravelInfo(0, 0), _furthestCheckedPoint, BaggageTripType::OTHER, 0);
    _strategy->processBaggageTravelNoDotTable(bt, cxt);

    CPPUNIT_ASSERT(bt._processCharges);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("FM"), bt._allowanceCxr);
    CPPUNIT_ASSERT(!bt._defer);
    CPPUNIT_ASSERT(bt._allowance);
  }

  void test_processBaggageTravelNoDotTable_NoMsS5_FcS5_Defer_NoS5()
  {
    _mdh->addS5("FM");
    _mdh->addS7("FM", 'O');
    initProcessBtNoDotTable();

    BaggageTravel& bt = *_bgTravels->front();
    AllowanceDataStrategy::MatchS7Context cxt(
        BaggageTravelInfo(0, 0), _furthestCheckedPoint, BaggageTripType::OTHER, 0);
    _strategy->processBaggageTravelNoDotTable(bt, cxt);

    CPPUNIT_ASSERT(!bt._processCharges);
    CPPUNIT_ASSERT(!bt._defer);
    CPPUNIT_ASSERT(!bt._allowance);
  }

  void test_processBaggageTravelNoDotTable_NoMsS5_FcS5_Defer_NoS7()
  {
    _mdh->addS5("FM");
    _mdh->addS7("FM", 'O');
    _mdh->addS5("FO");
    initProcessBtNoDotTable();

    BaggageTravel& bt = *_bgTravels->front();
    AllowanceDataStrategy::MatchS7Context cxt(
        BaggageTravelInfo(0, 0), _furthestCheckedPoint, BaggageTripType::OTHER, 0);
    _strategy->processBaggageTravelNoDotTable(bt, cxt);

    CPPUNIT_ASSERT(bt._processCharges);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("FO"), bt._allowanceCxr);
    CPPUNIT_ASSERT(bt._defer);
    CPPUNIT_ASSERT(!bt._allowance);
  }

  void test_processBaggageTravelNoDotTable_NoMsS5_FcS5_Defer_S7Defer()
  {
    _mdh->addS5("FM");
    _mdh->addS7("FM", 'O');
    _mdh->addS5("FO");
    _mdh->addS7("FO", 'O');
    initProcessBtNoDotTable();

    BaggageTravel& bt = *_bgTravels->front();
    AllowanceDataStrategy::MatchS7Context cxt(
        BaggageTravelInfo(0, 0), _furthestCheckedPoint, BaggageTripType::OTHER, 0);
    _strategy->processBaggageTravelNoDotTable(bt, cxt);

    CPPUNIT_ASSERT(bt._processCharges);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("FO"), bt._allowanceCxr);
    CPPUNIT_ASSERT(bt._defer);
    CPPUNIT_ASSERT(!bt._allowance);
  }

  void test_processBaggageTravelNoDotTable_NoMsS5_FcS5_Defer_S7()
  {
    _mdh->addS5("FM");
    _mdh->addS7("FM", 'O');
    _mdh->addS5("FO");
    _mdh->addS7("FO", 'F');
    initProcessBtNoDotTable();

    BaggageTravel& bt = *_bgTravels->front();
    AllowanceDataStrategy::MatchS7Context cxt(
        BaggageTravelInfo(0, 0), _furthestCheckedPoint, BaggageTripType::OTHER, 0);
    _strategy->processBaggageTravelNoDotTable(bt, cxt);

    CPPUNIT_ASSERT(bt._processCharges);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("FO"), bt._allowanceCxr);
    CPPUNIT_ASSERT(bt._defer);
    CPPUNIT_ASSERT(bt._allowance);
  }

  void test_processBaggageTravelDotTable_No_CarrierUsDot()
  {
    overrideAllowanceCarrier(CarrierCode());
    overrideChargesCarrier(CarrierCode());

    std::vector<TravelSeg*> segments;
    segments += AirSegBuilder(&_memHandle).withCarriers("LH", "AA").build();
    segments += AirSegBuilder(&_memHandle).withCarriers("LT", "BA").build();

    FarePath* farePath = FarePathBuilder(&_memHandle).withTravelSeg(segments).build();
    farePath->itin()->setBaggageTripType(BaggageTripType::TO_FROM_US);
    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle)
                                  .withTrx(_trx)
                                  .withFarePath(farePath)
                                  .withTravelSeg(segments)
                                  .build();

    _strategy->processBaggageTravelDotTable(
        bgTravel, BaggageTravelInfo(0, 0), _furthestCheckedPoint, BaggageTripType::TO_FROM_US, 0);

    CPPUNIT_ASSERT(!bgTravel->_processCharges);
    CPPUNIT_ASSERT(!bgTravel->_allowance);
  }

  void test_processBaggageTravelDotTable_No_CarrierUsDot_override_allowance()
  {
    overrideAllowanceCarrier(CarrierCode("KL"));
    overrideChargesCarrier(CarrierCode());

    std::vector<TravelSeg*> segments;
    segments += AirSegBuilder(&_memHandle).withCarriers("LH", "AA").build();
    segments += AirSegBuilder(&_memHandle).withCarriers("LT", "BA").build();

    FarePath* farePath = FarePathBuilder(&_memHandle).withTravelSeg(segments).build();
    farePath->itin()->setBaggageTripType(BaggageTripType::TO_FROM_US);
    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle)
                                  .withTrx(_trx)
                                  .withFarePath(farePath)
                                  .withTravelSeg(segments)
                                  .build();

    _strategy->processBaggageTravelDotTable(
        bgTravel, BaggageTravelInfo(0, 0), _furthestCheckedPoint, BaggageTripType::TO_FROM_US, 0);

    CPPUNIT_ASSERT(!bgTravel->_processCharges);
    CPPUNIT_ASSERT(!bgTravel->_allowance);
  }

  void test_processBaggageTravelDotTable_No_CarrierUsDot_override_charges()
  {
    overrideAllowanceCarrier(CarrierCode());
    overrideChargesCarrier(CarrierCode("KL"));

    std::vector<TravelSeg*> segments;
    segments += AirSegBuilder(&_memHandle).withCarriers("LH", "AA").build();
    segments += AirSegBuilder(&_memHandle).withCarriers("LT", "BA").build();

    FarePath* farePath = FarePathBuilder(&_memHandle).withTravelSeg(segments).build();
    farePath->itin()->setBaggageTripType(BaggageTripType::TO_FROM_US);
    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle)
                                  .withTrx(_trx)
                                  .withFarePath(farePath)
                                  .withTravelSeg(segments)
                                  .build();

    _strategy->processBaggageTravelDotTable(
        bgTravel, BaggageTravelInfo(0, 0), _furthestCheckedPoint, BaggageTripType::TO_FROM_US, 0);

    CPPUNIT_ASSERT(bgTravel->_processCharges);
    CPPUNIT_ASSERT(!bgTravel->_allowance);
  }

  void test_processBaggageTravelDotTable_CarrierUsDot_No_S5()
  {
    _mdh->addCarrierUsDot("LH");

    std::vector<TravelSeg*> segments;
    segments += AirSegBuilder(&_memHandle).withCarriers("LH", "AA").build();
    segments += AirSegBuilder(&_memHandle).withCarriers("LT", "BA").build();

    FarePath* farePath = FarePathBuilder(&_memHandle).withTravelSeg(segments).build();
    farePath->itin()->setBaggageTripType(BaggageTripType::TO_FROM_US);
    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle)
                                  .withTrx(_trx)
                                  .withFarePath(farePath)
                                  .withTravelSeg(segments)
                                  .build();

    _strategy->processBaggageTravelDotTable(
        bgTravel, BaggageTravelInfo(0, 0), _furthestCheckedPoint, BaggageTripType::TO_FROM_US, 0);

    CPPUNIT_ASSERT(!bgTravel->_processCharges);
    CPPUNIT_ASSERT(!bgTravel->_allowance);
  }

  void test_processBaggageTravelDotTable_CarrierUsDot_S5_No_S7()
  {
    _mdh->addCarrierUsDot("LH");
    _mdh->addS5("LH");

    std::vector<TravelSeg*> segments;
    segments += AirSegBuilder(&_memHandle).withCarriers("LH", "AA").build();
    segments += AirSegBuilder(&_memHandle).withCarriers("LT", "BA").build();

    FarePath* farePath = FarePathBuilder(&_memHandle).withTravelSeg(segments).build();
    farePath->itin()->setBaggageTripType(BaggageTripType::TO_FROM_US);
    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle)
                                  .withTrx(_trx)
                                  .withFarePath(farePath)
                                  .withTravelSeg(segments)
                                  .build();

    _strategy->processBaggageTravelDotTable(
        bgTravel, BaggageTravelInfo(0, 0), _furthestCheckedPoint, BaggageTripType::TO_FROM_US, 0);

    CPPUNIT_ASSERT(bgTravel->_processCharges);
    CPPUNIT_ASSERT(bgTravel->_allowance);
    CPPUNIT_ASSERT(!bgTravel->_allowance->optFee());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("LH"), bgTravel->_allowanceCxr);
  }

  void test_processBaggageTravelDotTable_CarrierUsDot_S5_S7_No_Defer()
  {
    _mdh->addCarrierUsDot("LH");
    _mdh->addS5("LH");
    _mdh->addS7("LH", 'F');

    std::vector<TravelSeg*> segments;
    segments += AirSegBuilder(&_memHandle).withCarriers("LH", "AA").build();
    segments += AirSegBuilder(&_memHandle).withCarriers("LT", "BA").build();

    FarePath* farePath = FarePathBuilder(&_memHandle).withTravelSeg(segments).build();
    farePath->itin()->setBaggageTripType(BaggageTripType::TO_FROM_US);
    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle)
                                  .withTrx(_trx)
                                  .withFarePath(farePath)
                                  .withTravelSeg(segments)
                                  .build();

    _strategy->processBaggageTravelDotTable(
        bgTravel, BaggageTravelInfo(0, 0), _furthestCheckedPoint, BaggageTripType::TO_FROM_US, 0);

    CPPUNIT_ASSERT(bgTravel->_processCharges);
    CPPUNIT_ASSERT(bgTravel->_allowance);
    CPPUNIT_ASSERT(bgTravel->_allowance->optFee());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("LH"), bgTravel->_allowance->carrierCode());
    CPPUNIT_ASSERT_EQUAL(CarrierCode(), bgTravel->_allowanceCxr);
  }

  void test_processBaggageTravelDotTable_CarrierUsDot_S5_S7_Defer_MSSJourney_No_DotTable()
  {
    _mdh->addCarrierUsDot("LH");
    _mdh->addS5("LH");
    _mdh->addS7("LH", 'D');

    std::vector<TravelSeg*> segments;
    segments += AirSegBuilder(&_memHandle).withCarriers("LH", "AA").build();
    segments += AirSegBuilder(&_memHandle).withCarriers("LT", "BA").build();

    FarePath* farePath = FarePathBuilder(&_memHandle).withTravelSeg(segments).build();
    farePath->itin()->setBaggageTripType(BaggageTripType::TO_FROM_US);
    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle)
                                  .withTrx(_trx)
                                  .withFarePath(farePath)
                                  .withTravelSeg(segments)
                                  .build();

    _strategy->processBaggageTravelDotTable(
        bgTravel, BaggageTravelInfo(0, 0), _furthestCheckedPoint, BaggageTripType::TO_FROM_US, 0);

    CPPUNIT_ASSERT(bgTravel->_processCharges);
    CPPUNIT_ASSERT(bgTravel->_allowance);
    CPPUNIT_ASSERT(!bgTravel->_allowance->optFee());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("LH"), bgTravel->_allowance->carrierCode());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("LH"), bgTravel->_allowanceCxr);
  }

  void test_processBaggageTravelDotTable_CarrierUsDot_S5_S7_Defer_MSSJourney_DotTable_No_S5()
  {
    _mdh->addCarrierUsDot("LH");
    _mdh->addCarrierUsDot("LT");
    _mdh->addS5("LH");
    _mdh->addS7("LH", 'D');

    std::vector<TravelSeg*> segments;
    segments += AirSegBuilder(&_memHandle).withCarriers("LH", "AA").build();
    segments += AirSegBuilder(&_memHandle).withCarriers("LT", "BA").build();

    FarePath* farePath = FarePathBuilder(&_memHandle).withTravelSeg(segments).build();
    farePath->itin()->setBaggageTripType(BaggageTripType::TO_FROM_US);
    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle)
                                  .withTrx(_trx)
                                  .withFarePath(farePath)
                                  .withTravelSeg(segments)
                                  .build();

    _strategy->processBaggageTravelDotTable(
        bgTravel, BaggageTravelInfo(0, 0), _furthestCheckedPoint, BaggageTripType::TO_FROM_US, 0);

    CPPUNIT_ASSERT(bgTravel->_processCharges);
    CPPUNIT_ASSERT(bgTravel->_allowance);
    CPPUNIT_ASSERT(!bgTravel->_allowance->optFee());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("LH"), bgTravel->_allowance->carrierCode());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("LH"), bgTravel->_allowanceCxr);
  }

  void test_processBaggageTravelDotTable_CarrierUsDot_S5_S7_Defer_MSSJourney_DotTable_S5_No_S7()
  {
    _mdh->addCarrierUsDot("LH");
    _mdh->addCarrierUsDot("LT");
    _mdh->addS5("LH");
    _mdh->addS5("LT");
    _mdh->addS7("LH", 'D');

    std::vector<TravelSeg*> segments;
    segments += AirSegBuilder(&_memHandle).withCarriers("LH", "AA").build();
    segments += AirSegBuilder(&_memHandle).withCarriers("LT", "BA").build();

    FarePath* farePath = FarePathBuilder(&_memHandle).withTravelSeg(segments).build();
    farePath->itin()->setBaggageTripType(BaggageTripType::TO_FROM_US);
    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle)
                                  .withTrx(_trx)
                                  .withFarePath(farePath)
                                  .withTravelSeg(segments)
                                  .build();

    _bgTravels->push_back(bgTravel);
    _strategy->processBaggageTravelDotTable(
        bgTravel, BaggageTravelInfo(0, 0), _furthestCheckedPoint, BaggageTripType::TO_FROM_US, 0);

    CPPUNIT_ASSERT(bgTravel->_processCharges);
    CPPUNIT_ASSERT(bgTravel->_allowance);
    CPPUNIT_ASSERT(!bgTravel->_allowance->optFee());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("LH"), bgTravel->_allowance->carrierCode());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("LH"), bgTravel->_allowanceCxr);
  }

  void test_processBaggageTravelDotTable_CarrierUsDot_S5_S7_Defer_MSSJourney_DotTable_S5_No_S7_NextS7()
  {
    _mdh->addCarrierUsDot("LH");
    _mdh->addCarrierUsDot("LT");
    _mdh->addS5("LH");
    _mdh->addS5("LT");
    _mdh->addS7("LH", 'D');
    _mdh->addS7("LH", 'F');

    std::vector<TravelSeg*> segments;
    segments += AirSegBuilder(&_memHandle).withCarriers("LH", "AA").build();
    segments += AirSegBuilder(&_memHandle).withCarriers("LT", "BA").build();

    FarePath* farePath = FarePathBuilder(&_memHandle).withTravelSeg(segments).build();
    farePath->itin()->setBaggageTripType(BaggageTripType::TO_FROM_US);
    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle)
                                  .withTrx(_trx)
                                  .withFarePath(farePath)
                                  .withTravelSeg(segments)
                                  .build();

    _bgTravels->push_back(bgTravel);
    _strategy->processBaggageTravelDotTable(
        bgTravel, BaggageTravelInfo(0, 0), _furthestCheckedPoint, BaggageTripType::TO_FROM_US, 0);

    CPPUNIT_ASSERT(bgTravel->_processCharges);
    CPPUNIT_ASSERT(bgTravel->_allowance);
    CPPUNIT_ASSERT(bgTravel->_allowance->optFee());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("LH"), bgTravel->_allowance->carrierCode());
    CPPUNIT_ASSERT_EQUAL(CarrierCode(), bgTravel->_allowanceCxr);
  }

  void test_processBaggageTravelDotTable_CarrierUsDot_S5_S7_Defer_MSSJourney_DotTable_S5_S7()
  {
    _mdh->addCarrierUsDot("LH");
    _mdh->addCarrierUsDot("LT");
    _mdh->addS5("LH");
    _mdh->addS5("LT");
    _mdh->addS7("LH", 'D');
    _mdh->addS7("LT", 'F');

    std::vector<TravelSeg*> segments;
    segments += AirSegBuilder(&_memHandle).withCarriers("LH", "AA").build();
    segments += AirSegBuilder(&_memHandle).withCarriers("LT", "BA").build();

    FarePath* farePath = FarePathBuilder(&_memHandle).withTravelSeg(segments).build();
    farePath->itin()->setBaggageTripType(BaggageTripType::TO_FROM_US);
    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle)
                                  .withTrx(_trx)
                                  .withFarePath(farePath)
                                  .withTravelSeg(segments)
                                  .build();

    _strategy->processBaggageTravelDotTable(
        bgTravel, BaggageTravelInfo(0, 0), _furthestCheckedPoint, BaggageTripType::TO_FROM_US, 0);

    CPPUNIT_ASSERT(bgTravel->_processCharges);
    CPPUNIT_ASSERT(bgTravel->_allowance);
    CPPUNIT_ASSERT(bgTravel->_allowance->optFee());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("LT"), bgTravel->_allowance->carrierCode());
    CPPUNIT_ASSERT_EQUAL(CarrierCode(), bgTravel->_allowanceCxr);
  }

  void test_processBaggageTravelDotTable_No_CarrierCta()
  {
    overrideAllowanceCarrier(CarrierCode());
    overrideChargesCarrier(CarrierCode());
    _mdh->addCarrierUsDot("LH");
    _mdh->addS5("LH");

    std::vector<TravelSeg*> segments;
    segments += AirSegBuilder(&_memHandle).withCarriers("LH", "AA").build();
    segments += AirSegBuilder(&_memHandle).withCarriers("LT", "BA").build();

    FarePath* farePath = FarePathBuilder(&_memHandle).withTravelSeg(segments).build();
    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle)
                                  .withTrx(_trx)
                                  .withFarePath(farePath)
                                  .withTravelSeg(segments)
                                  .build();

    _strategy->processBaggageTravelDotTable(
        bgTravel, BaggageTravelInfo(0, 0), _furthestCheckedPoint, BaggageTripType::TO_FROM_CA, 0);

    CPPUNIT_ASSERT(!bgTravel->_processCharges);
    CPPUNIT_ASSERT(!bgTravel->_allowance);
  }

  void test_processBaggageTravelDotTable_No_CarrierUsDotCta()
  {
    overrideAllowanceCarrier(CarrierCode());
    overrideChargesCarrier(CarrierCode());
    _mdh->addCarrierUsDot("LH");
    _mdh->addCarrierCta("LT");
    _mdh->addS5("LH");
    _mdh->addS5("LT");

    std::vector<TravelSeg*> segments;
    segments += AirSegBuilder(&_memHandle).withCarriers("LH", "AA").build();
    segments += AirSegBuilder(&_memHandle).withCarriers("LT", "BA").build();

    FarePath* farePath = FarePathBuilder(&_memHandle).withTravelSeg(segments).build();
    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle)
                                  .withTrx(_trx)
                                  .withFarePath(farePath)
                                  .withTravelSeg(segments)
                                  .build();

    _strategy->processBaggageTravelDotTable(bgTravel,
                                            BaggageTravelInfo(0, 0),
                                            _furthestCheckedPoint,
                                            BaggageTripType::BETWEEN_US_CA,
                                            0);

    CPPUNIT_ASSERT(!bgTravel->_processCharges);
    CPPUNIT_ASSERT(!bgTravel->_allowance);
  }

  void test_processBaggageTravelDotTable_CarrierCta_S5_S7_No_Defer()
  {
    _mdh->addCarrierUsDot("LH");
    _mdh->addCarrierCta("LT");
    _mdh->addS5("LH");
    _mdh->addS7("LH", 'F');
    _mdh->addS5("LT");
    _mdh->addS7("LT", 'F');

    std::vector<TravelSeg*> segments;
    segments += AirSegBuilder(&_memHandle).withCarriers("LH", "AA").build();
    segments += AirSegBuilder(&_memHandle).withCarriers("LT", "BA").build();

    FarePath* farePath = FarePathBuilder(&_memHandle).withTravelSeg(segments).build();
    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle)
                                  .withTrx(_trx)
                                  .withFarePath(farePath)
                                  .withTravelSeg(segments)
                                  .build();

    _strategy->processBaggageTravelDotTable(
        bgTravel, BaggageTravelInfo(0, 0), _furthestCheckedPoint, BaggageTripType::TO_FROM_CA, 0);

    CPPUNIT_ASSERT(bgTravel->_processCharges);
    CPPUNIT_ASSERT(bgTravel->_allowance);
    CPPUNIT_ASSERT(bgTravel->_allowance->optFee());
    CPPUNIT_ASSERT_EQUAL(CarrierCode(), bgTravel->_allowanceCxr);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("LT"), bgTravel->_allowance->carrierCode());
  }

  void test_processBaggageTravelDotTable_CarrierUsDotCta_S5_S7_No_Defer()
  {
    _mdh->addCarrierUsDot("LH");
    _mdh->addCarrierUsDot("AC");
    _mdh->addCarrierCta("LT");
    _mdh->addCarrierCta("AC");
    _mdh->addS5("LH");
    _mdh->addS7("LH", 'F');
    _mdh->addS5("LT");
    _mdh->addS7("LT", 'F');
    _mdh->addS5("AC");
    _mdh->addS7("AC", 'F');

    std::vector<TravelSeg*> segments;
    segments += AirSegBuilder(&_memHandle).withCarriers("LH", "AA").build();
    segments += AirSegBuilder(&_memHandle).withCarriers("LT", "BA").build();
    segments += AirSegBuilder(&_memHandle).withCarriers("AC", "BA").build();

    FarePath* farePath = FarePathBuilder(&_memHandle).withTravelSeg(segments).build();
    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle)
                                  .withTrx(_trx)
                                  .withFarePath(farePath)
                                  .withTravelSeg(segments)
                                  .build();

    _strategy->processBaggageTravelDotTable(
        bgTravel, BaggageTravelInfo(0, 0), _furthestCheckedPoint, BaggageTripType::BETWEEN_US_CA, 0);

    CPPUNIT_ASSERT(bgTravel->_processCharges);
    CPPUNIT_ASSERT(bgTravel->_allowance);
    CPPUNIT_ASSERT(bgTravel->_allowance->optFee());
    CPPUNIT_ASSERT_EQUAL(CarrierCode(), bgTravel->_allowanceCxr);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AC"), bgTravel->_allowance->carrierCode());
  }

  void test_processBaggageTravelForAllowanceCarrierOverridden_no_S5()
  {
    SubCodeInfo* s5 = S5Builder(&_memHandle)
                          .withVendor(ATPCO_VENDOR_CODE)
                          .withCarrier("LH")
                          .withAllowanceMatched()
                          .build();

    OptionalServicesInfo* s7 = S7Builder(&_memHandle).withAllowanceMatched("LH", 'F').build();

    std::vector<TravelSeg*> segments;
    segments.push_back(
        AirSegBuilder(&_memHandle).withMarketingCarrier("LH").withOperatingCarrier("AA").build());

    FarePath* farePath = FarePathBuilder(&_memHandle).withTravelSeg(segments).build();

    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle)
                                  .withTrx(_trx)
                                  .withFarePath(farePath)
                                  .withTravelSeg(segments)
                                  .withAllowance(s5, s7)
                                  .build();

    _bgTravels->push_back(bgTravel);
    _strategy->processBaggageTravelForAllowanceCarrierOverridden(
        (*_bgTravels)[0], BaggageTravelInfo(0, 0), _furthestCheckedPoint, true, 0);

    CPPUNIT_ASSERT(!bgTravel->_allowance);
    CPPUNIT_ASSERT_EQUAL(bgTravel->_allowanceCxr, CarrierCode("LH"));
  }

private:
  class MyDataHandle : public DataHandleMock
  {
    std::vector<SubCodeInfo*>* _s5s;
    std::vector<OptionalServicesInfo*>* _s7s;
    std::set<CarrierCode> _carrierUsDot;
    std::set<CarrierCode> _carrierCta;

  public:
    MyDataHandle()
    {
      _s5s = _memHandle.create<std::vector<SubCodeInfo*> >();
      _s7s = _memHandle.create<std::vector<OptionalServicesInfo*> >();
    }

    void addS5(const CarrierCode& carrier)
    {
      *_s5s += S5Builder(&_memHandle)
                   .withVendor(ATPCO_VENDOR_CODE)
                   .withCarrier(carrier)
                   .withAllowanceMatched()
                   .build();
    }

    void addS7(const CarrierCode& carrier, const Indicator& notAvailNoChargeInd)
    {
      *_s7s += S7Builder(&_memHandle).withAllowanceMatched(carrier, notAvailNoChargeInd).build();
    }

    void addCarrierUsDot(const CarrierCode& carrierUsDot) { _carrierUsDot.insert(carrierUsDot); }
    void addCarrierCta(const CarrierCode& carrierCta) { _carrierCta.insert(carrierCta); }

    const std::vector<SubCodeInfo*>& getSubCode(const VendorCode& vendor,
                                                const CarrierCode& carrier,
                                                const ServiceTypeCode& serviceTypeCode,
                                                const ServiceGroup& serviceGroup,
                                                const DateTime& date)
    {
      std::vector<SubCodeInfo*>* subCodes = _memHandle.create<std::vector<SubCodeInfo*> >();

      if (_s5s->empty())
      {
        if ((vendor == ATPCO_VENDOR_CODE && carrier[0] == 'A') ||
            (vendor == MERCH_MANAGER_VENDOR_CODE && carrier[0] == 'M'))
        {
          *subCodes += S5Builder(&_memHandle)
                           .withVendCarr(vendor, carrier)
                           .withFltTktMerchInd(BAGGAGE_ALLOWANCE)
                           .withSubCode("0DF")
                           .withCodes('X', ' ', 'C', '4', "", "")
                           .build();
        }
      }
      else
      {
        std::vector<SubCodeInfo*>* result = _memHandle.create<std::vector<SubCodeInfo*> >();
        std::remove_copy_if(
            _s5s->begin(), _s5s->end(), std::back_inserter(*result), CheckS5(carrier));
        return *result;
      }

      return *subCodes;
    }

    const std::vector<OptionalServicesInfo*>&
    getOptionalServicesInfo(const VendorCode& vendor,
                            const CarrierCode& carrier,
                            const ServiceTypeCode& serviceTypeCode,
                            const ServiceSubTypeCode& serviceSubTypeCode,
                            Indicator fltTktMerchInd,
                            const DateTime& date)
    {
      std::vector<OptionalServicesInfo*>* result =
          _memHandle.create<std::vector<OptionalServicesInfo*> >();
      std::remove_copy_if(
          _s7s->begin(), _s7s->end(), std::back_inserter(*result), CheckS7(carrier));

      for (size_t i = 0; i < result->size(); ++i)
      {
        OptionalServicesInfo* s7 = result->at(i);
        s7->seqNo() = static_cast<uint32_t>(i + 1);
      }

      return *result;
    }

    bool isCarrierUSDot(const CarrierCode& carrier)
    {
      return _carrierUsDot.find(carrier) != _carrierUsDot.end();
    }

    bool isCarrierCTA(const CarrierCode& carrier)
    {
      return _carrierCta.find(carrier) != _carrierCta.end();
    }

    TestMemHandle _memHandle;
  };

  MyDataHandle* _mdh;
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  Itin* _itin;
  FarePath* _farePath;
  AllowanceDataStrategy* _strategy;
  std::vector<BaggageTravel*>* _bgTravels;
  CheckedPoint _furthestCheckedPoint;
};

CPPUNIT_TEST_SUITE_REGISTRATION(AllowanceDataStrategyTest);

} // tse
