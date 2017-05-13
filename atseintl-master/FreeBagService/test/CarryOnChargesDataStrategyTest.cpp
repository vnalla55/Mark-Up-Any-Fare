#include <functional>
#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "Common/MetricsMan.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "Diagnostic/Diag852Collector.h"
#include "Diagnostic/test/Diag852ParsedParamsTester.h"
#include "FreeBagService/CarryOnChargesDataStrategy.h"
#include "FreeBagService/BaggageTravelInfo.h"
#include "FreeBagService/test/BaggageTravelBuilder.h"
#include "FreeBagService/test/S5Builder.h"
#include "FreeBagService/test/S7Builder.h"
#include "ServiceFees/OCFees.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

using boost::assign::operator+=;

class CarryOnChargesDataStrategyTest : public CppUnit::TestFixture
{
  // MOCKS
  class MyDataHandle : public DataHandleMock
  {
  public:
    MyDataHandle() : _subCodes(0) {}

    const std::vector<SubCodeInfo*>& getSubCode(const VendorCode& vendor,
                                                const CarrierCode& carrier,
                                                const ServiceTypeCode& serviceTypeCode,
                                                const ServiceGroup& serviceGroup,
                                                const DateTime& /*date*/)
    {
      std::vector<SubCodeInfo*>* ret = _memHandle.create<std::vector<SubCodeInfo*> >();
      if (_subCodes)
      {
        for (SubCodeInfo* subCodeInfo : *_subCodes)
        {
          if ((subCodeInfo->serviceTypeCode() == serviceTypeCode) &&
              (subCodeInfo->serviceGroup() == serviceGroup) && (subCodeInfo->vendor() == vendor) &&
              (subCodeInfo->carrier() == carrier))
            *ret += subCodeInfo;
        }
      }
      return *ret;
    }

    void setSubCode(const std::vector<SubCodeInfo*>* subCodes) { _subCodes = subCodes; }

  private:
    TestMemHandle _memHandle;
    const std::vector<SubCodeInfo*>* _subCodes;
  };

  class TestDiagCollector : public Diag852Collector
  {
  public:
    TestDiagCollector() : _paramsTester(Diag852Collector::_params)
    {
      rootDiag() = _memHandle.create<Diagnostic>();
    }

    TestDiagCollector(Diag852Collector::DiagType diagType)
      : _paramsTester(Diag852Collector::_params)
    {
      _paramsTester.updateDiagType(diagType);
      TestDiagCollector();
    }

    void setDiagType(Diag852Collector::DiagType diagType)
    {
      _paramsTester.updateDiagType(diagType);
    }

    void addParam(const std::string& key, const std::string& value)
    {
      rootDiag()->diagParamMap().insert(std::make_pair(key, value));
      _paramsTester.forceInitialization();
    }

  private:
    Diag852ParsedParamsTester _paramsTester;
    TestMemHandle _memHandle;
  };

  CPPUNIT_TEST_SUITE(CarryOnChargesDataStrategyTest);

  CPPUNIT_TEST(testRetrieveS5Records_Positive_BG);
  CPPUNIT_TEST(testRetrieveS5Records_Positive_PT);
  CPPUNIT_TEST(testRetrieveS5Records_Negative_ServiceGroup);
  CPPUNIT_TEST(testRetrieveS5Records_Negative_ServiceType);
  CPPUNIT_TEST(testRetrieveS5Records_Negative_MerchInd);
  CPPUNIT_TEST(testRetrieveS5Records_Negative_SubGroup);
  CPPUNIT_TEST(testRetrieveS5Records_Negative_Carrier);
  CPPUNIT_TEST(testRetrieveS5Records_Positive_Multiple);

  CPPUNIT_TEST(testShouldDisplayChargesDiagnostic_True);
  CPPUNIT_TEST(testShouldDisplayChargesDiagnostic_FalseCC);
  CPPUNIT_TEST(testShouldDisplayChargesDiagnostic_FalseFL);

  CPPUNIT_TEST(tastShouldProcessCharges);
  CPPUNIT_TEST(tastShouldProcessCharges_nil);
  CPPUNIT_TEST(tastShouldProcessCharges_noS7);
  CPPUNIT_TEST(tastShouldProcessCharges_notFiled);
  CPPUNIT_TEST(tastShouldProcessCharges_toLarge);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _mdh = _memHandle.create<MyDataHandle>();
    _itin = _memHandle.create<Itin>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->itin() += _itin;
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    Loc* agentLoc = _memHandle.create<Loc>();
    agentLoc->loc() = "KRK";
    _trx->getRequest()->ticketingAgent()->agentLocation() = agentLoc;
    _strategy = _memHandle.insert(new CarryOnChargesDataStrategy(*_trx));
    _trx->billing() = _memHandle.create<Billing>();
  }

  void tearDown()
  {
    _memHandle.clear();
  }

protected:
  SubCodeInfo* createRecordS5(const ServiceTypeCode& typeCode = "OC",
                              const ServiceGroup& group = "BG",
                              const Indicator& merchInd = 'C',
                              const ServiceGroup& subGroup = "CY",
                              const VendorCode& vendor = ATPCO_VENDOR_CODE,
                              const CarrierCode& carrier = "AA",
                              const ServiceSubTypeCode& subTypeCode = "AAA")
  {
    return S5Builder(&_memHandle)
        .withTypeCode(typeCode)
        .withSubCode(subTypeCode)
        .withGroup(group)
        .withFltTktMerchInd(merchInd)
        .withSubGroup(subGroup)
        .withVendCarr(vendor, carrier)
        .build();
  }

  BaggageTravel* createBaggageTravel(const CarrierCode& carrier = "AA")
  {
    FarePath* farePath = _memHandle.create<FarePath>();
    _itin->farePath() += farePath;
    farePath->itin() = _itin;

    std::vector<TravelSeg*>* tSegs = _memHandle.create<std::vector<TravelSeg*> >();

    AirSeg* aSeg = _memHandle.create<AirSeg>();
    aSeg->setOperatingCarrierCode(carrier);
    tSegs->push_back(aSeg);

    return BaggageTravelBuilder(&_memHandle)
        .withCarrier(carrier)
        .withFarePath(farePath)
        .withMSS(tSegs->begin())
        .build();
  }

  std::vector<BaggageTravel*>* create1ItemBaggageTravelVector()
  {
    std::vector<BaggageTravel*>* bagTvls = _memHandle.create<std::vector<BaggageTravel*> >();
    *bagTvls += createBaggageTravel();
    return bagTvls;
  }

protected:
  MyDataHandle* _mdh;
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  Itin* _itin;
  CarryOnChargesDataStrategy* _strategy;

public:
  // TESTS
  void testRetrieveS5Records_Positive_BG()
  {
    std::vector<SubCodeInfo*> subCodesIn;
    subCodesIn += createRecordS5("OC", "BG", 'C', "CY", ATPCO_VENDOR_CODE, "AA");
    _mdh->setSubCode(&subCodesIn);

    std::vector<const SubCodeInfo*> subCodesOut;

    _strategy->retrieveS5Records(createBaggageTravel("AA"), subCodesOut);

    CPPUNIT_ASSERT_EQUAL((size_t)1, subCodesOut.size());
    CPPUNIT_ASSERT(subCodesIn.front() == subCodesOut.front());
  }

  void testRetrieveS5Records_Positive_PT()
  {
    std::vector<SubCodeInfo*> subCodesIn;
    subCodesIn += createRecordS5("OC", "PT", 'C', "CY", ATPCO_VENDOR_CODE, "AA");
    _mdh->setSubCode(&subCodesIn);

    std::vector<const SubCodeInfo*> subCodesOut;

    _strategy->retrieveS5Records(createBaggageTravel("AA"), subCodesOut);

    CPPUNIT_ASSERT_EQUAL((size_t)1, subCodesOut.size());
    CPPUNIT_ASSERT(subCodesIn.front() == subCodesOut.front());
  }

  void testRetrieveS5Records_Negative_ServiceGroup()
  {
    std::vector<SubCodeInfo*> subCodesIn;
    subCodesIn += createRecordS5("OC", "XX", 'C', "CY", ATPCO_VENDOR_CODE, "AA");
    _mdh->setSubCode(&subCodesIn);

    std::vector<const SubCodeInfo*> subCodesOut;

    _strategy->retrieveS5Records(createBaggageTravel("AA"), subCodesOut);

    CPPUNIT_ASSERT(subCodesOut.empty());
  }

  void testRetrieveS5Records_Negative_ServiceType()
  {
    std::vector<SubCodeInfo*> subCodesIn;
    subCodesIn += createRecordS5("XX", "BG", 'C', "CY", ATPCO_VENDOR_CODE, "AA");
    _mdh->setSubCode(&subCodesIn);

    std::vector<const SubCodeInfo*> subCodesOut;

    _strategy->retrieveS5Records(createBaggageTravel("AA"), subCodesOut);

    CPPUNIT_ASSERT(subCodesOut.empty());
  }

  void testRetrieveS5Records_Negative_MerchInd()
  {
    std::vector<SubCodeInfo*> subCodesIn;
    subCodesIn += createRecordS5("OC", "BG", 'X', "CY", ATPCO_VENDOR_CODE, "AA");
    _mdh->setSubCode(&subCodesIn);

    std::vector<const SubCodeInfo*> subCodesOut;

    _strategy->retrieveS5Records(createBaggageTravel("AA"), subCodesOut);

    CPPUNIT_ASSERT(subCodesOut.empty());
  }

  void testRetrieveS5Records_Negative_SubGroup()
  {
    std::vector<SubCodeInfo*> subCodesIn;
    subCodesIn += createRecordS5("OC", "BG", 'C', "XX", ATPCO_VENDOR_CODE, "AA");
    _mdh->setSubCode(&subCodesIn);

    std::vector<const SubCodeInfo*> subCodesOut;

    _strategy->retrieveS5Records(createBaggageTravel("AA"), subCodesOut);

    CPPUNIT_ASSERT(subCodesOut.empty());
  }

  void testRetrieveS5Records_Negative_Carrier()
  {
    std::vector<SubCodeInfo*> subCodesIn;
    subCodesIn += createRecordS5("OC", "BG", 'C', "CY", ATPCO_VENDOR_CODE, "XX");
    _mdh->setSubCode(&subCodesIn);

    std::vector<const SubCodeInfo*> subCodesOut;

    _strategy->retrieveS5Records(createBaggageTravel("AA"), subCodesOut);

    CPPUNIT_ASSERT(subCodesOut.empty());
  }

  void testRetrieveS5Records_Positive_Multiple()
  {
    std::vector<SubCodeInfo*> subCodesIn;
    subCodesIn += createRecordS5("XX", "BG", 'C', "CY", ATPCO_VENDOR_CODE, "AA", "AAA");
    subCodesIn += createRecordS5("OC", "XX", 'C', "CY", ATPCO_VENDOR_CODE, "AA", "BBB");

    subCodesIn += createRecordS5("OC", "BG", 'C', "CY", ATPCO_VENDOR_CODE, "AA", "CCC");
    subCodesIn += createRecordS5("OC", "BG", 'C', "CY", ATPCO_VENDOR_CODE, "AA", "DDD");

    subCodesIn += createRecordS5("OC", "BG", 'X', "CY", ATPCO_VENDOR_CODE, "AA", "EEE");
    subCodesIn += createRecordS5("OC", "BG", 'C', "XX", ATPCO_VENDOR_CODE, "AA", "FFF");

    subCodesIn += createRecordS5("OC", "PT", 'C', "CY", ATPCO_VENDOR_CODE, "AA", "GGG");
    subCodesIn += createRecordS5("OC", "PT", 'C', "CY", ATPCO_VENDOR_CODE, "AA", "HHH");

    subCodesIn += createRecordS5("OC", "PT", 'X', "CY", ATPCO_VENDOR_CODE, "AA", "III");
    subCodesIn += createRecordS5("OC", "PT", 'C', "XX", ATPCO_VENDOR_CODE, "AA", "JJJ");
    _mdh->setSubCode(&subCodesIn);

    std::vector<const SubCodeInfo*> subCodesOut;

    _strategy->retrieveS5Records(createBaggageTravel("AA"), subCodesOut);

    CPPUNIT_ASSERT_EQUAL((size_t)4, subCodesOut.size());
    CPPUNIT_ASSERT_EQUAL(ServiceGroup("BG"), subCodesOut[0]->serviceGroup());
    CPPUNIT_ASSERT_EQUAL(ServiceGroup("BG"), subCodesOut[1]->serviceGroup());
    CPPUNIT_ASSERT_EQUAL(ServiceGroup("PT"), subCodesOut[2]->serviceGroup());
    CPPUNIT_ASSERT_EQUAL(ServiceGroup("PT"), subCodesOut[3]->serviceGroup());
  }

  void testShouldDisplayChargesDiagnostic_True()
  {
    TestDiagCollector diag;
    diag.addParam("FL", "01");
    diag.addParam("CP", "01");
    diag.addParam(CARRY_ON_CHARGE, "");
    std::vector<BaggageTravel*>* baggageTravels = create1ItemBaggageTravelVector();

    CPPUNIT_ASSERT(_strategy->shouldDisplayChargesDiagnostic(
        (*baggageTravels)[0], BaggageTravelInfo(0, 0), &diag));
  }

  void testShouldDisplayChargesDiagnostic_FalseCC()
  {
    TestDiagCollector diag;
    diag.addParam("FL", "01");
    diag.addParam("CP", "01");
    std::vector<BaggageTravel*>* baggageTravels = create1ItemBaggageTravelVector();

    CPPUNIT_ASSERT(!_strategy->shouldDisplayChargesDiagnostic(
        (*baggageTravels)[0], BaggageTravelInfo(0, 0), &diag));
  }

  void testShouldDisplayChargesDiagnostic_FalseFL()
  {
    TestDiagCollector diag;
    diag.addParam("FL", "02");
    diag.addParam("CP", "01");
    diag.addParam(CARRY_ON_CHARGE, "");
    std::vector<BaggageTravel*>* baggageTravels = create1ItemBaggageTravelVector();

    CPPUNIT_ASSERT(!_strategy->shouldDisplayChargesDiagnostic(
        (*baggageTravels)[0], BaggageTravelInfo(0, 0), &diag));
  }

  void tastShouldProcessCharges()
  {
    CPPUNIT_ASSERT(
        _strategy->shouldProcessCharges(S7Builder(&_memHandle).withBaggagePcs(1).build()));
  }

  void tastShouldProcessCharges_nil()
  {
    CPPUNIT_ASSERT(
        _strategy->shouldProcessCharges(S7Builder(&_memHandle).withBaggagePcs(0).build()));
  }

  void tastShouldProcessCharges_noS7() { CPPUNIT_ASSERT(_strategy->shouldProcessCharges(0)); }

  void tastShouldProcessCharges_notFiled()
  {
    CPPUNIT_ASSERT(
        !_strategy->shouldProcessCharges(S7Builder(&_memHandle).withBaggagePcs(-1).build()));
  }

  void tastShouldProcessCharges_toLarge()
  {
    CPPUNIT_ASSERT(
        !_strategy->shouldProcessCharges(S7Builder(&_memHandle).withBaggagePcs(2).build()));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(CarryOnChargesDataStrategyTest);
} // tse
