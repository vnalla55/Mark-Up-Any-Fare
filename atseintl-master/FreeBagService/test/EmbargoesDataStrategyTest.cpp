#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/SubCodeInfo.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/Billing.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "Diagnostic/Diag852Collector.h"
#include "FreeBagService/EmbargoesDataStrategy.h"
#include "FreeBagService/test/AirSegBuilder.h"
#include "FreeBagService/test/BaggageTravelBuilder.h"
#include "FreeBagService/test/S5Builder.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

using boost::assign::operator+=;

class EmbargoesDataStrategyTest : public CppUnit::TestFixture
{
  class MyDataHandle : public DataHandleMock
  {
  public:
    MyDataHandle() : _subCodes(0) {}

    const std::vector<SubCodeInfo*>& getSubCode(const VendorCode& vendor,
                                                const CarrierCode& carrier,
                                                const ServiceTypeCode& /*serviceTypeCode*/,
                                                const ServiceGroup& /*serviceGroup*/,
                                                const DateTime& /*date*/)
    {
      std::vector<SubCodeInfo*>* ret = _memHandle.create<std::vector<SubCodeInfo*> >();

      if (_subCodes)
      {
        for (SubCodeInfo* subCodeInfo : *_subCodes)
        {
          if ((subCodeInfo->vendor() == vendor) && (subCodeInfo->carrier() == carrier))
            ret->push_back(subCodeInfo);
        }
      }
      return *ret;
    }

    void setSubCodes(const std::vector<SubCodeInfo*>* subCodes) { _subCodes = subCodes; }

  private:
    TestMemHandle _memHandle;
    const std::vector<SubCodeInfo*>* _subCodes;
  };

  CPPUNIT_TEST_SUITE(EmbargoesDataStrategyTest);
  CPPUNIT_TEST(testRetrieveS5Records_SrvTypeMatch_CarrierMatch);
  CPPUNIT_TEST(testRetrieveS5Records_SrvTypeMatch_CarrierNotMatch);
  CPPUNIT_TEST(testRetrieveS5Records_SrvTypeNotMatch_CarrierMatch);
  CPPUNIT_TEST(testRetrieveS5Records_SrvTypeNotMatch_CarrierNotMatch);
  CPPUNIT_TEST(testRetrieveS5Records_SrvTypeMatch_CarrierMatch_ConcurX);
  CPPUNIT_TEST(testRetrieveS5Records_SrvTypeMatch_CarrierMatch_ConcurOther);
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
    _trx->billing() = _memHandle.create<Billing>();
    _strategy = _memHandle.insert(new EmbargoesDataStrategy(*_trx));
  }

  void tearDown()
  {
    _memHandle.clear();
  }

protected:
  SubCodeInfo* createS5Record(const Indicator& srvType = 'E',
                              const Indicator& concur = 'X',
                              const VendorCode& vendor = ATPCO_VENDOR_CODE,
                              const CarrierCode& carrier = "LO")
  {
    return S5Builder(&_memHandle)
        .withVendCarr(vendor, carrier)
        .withFltTktMerchInd(srvType)
        .withConcur(concur)
        .build();
  }

  BaggageTravel* createBaggageTravel()
  {
    FarePath* farePath = _memHandle.create<FarePath>();
    _itin->farePath() += farePath;
    farePath->itin() = _itin;
    return BaggageTravelBuilder(&_memHandle)
        .withFarePath(farePath)
        .withMSS(_itin->travelSeg().begin())
        .build();
  }

  void addAirSegToItin(const CarrierCode& marketingCxr, const CarrierCode& operatingCxr)
  {
    _itin->travelSeg() +=
        AirSegBuilder(&_memHandle).withCarriers(marketingCxr, operatingCxr).build();
  }

  MyDataHandle* _mdh;
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  Itin* _itin;
  EmbargoesDataStrategy* _strategy;

public:
  void testRetrieveS5Records_SrvTypeMatch_CarrierMatch()
  {
    std::vector<SubCodeInfo*> subCodesIn;
    subCodesIn += createS5Record();

    _mdh->setSubCodes(&subCodesIn);
    addAirSegToItin("LO", "LO");

    const BaggageTravel* baggageTravel = createBaggageTravel();

    std::vector<const SubCodeInfo*> subCodesOut;
    _strategy->retrieveS5Records(baggageTravel, subCodesOut);

    CPPUNIT_ASSERT_EQUAL((size_t)1, subCodesOut.size());
  }

  void testRetrieveS5Records_SrvTypeMatch_CarrierNotMatch()
  {
    std::vector<SubCodeInfo*> subCodesIn;
    subCodesIn += createS5Record();

    _mdh->setSubCodes(&subCodesIn);
    addAirSegToItin("LO", "AA");

    const BaggageTravel* baggageTravel = createBaggageTravel();

    std::vector<const SubCodeInfo*> subCodesOut;
    _strategy->retrieveS5Records(baggageTravel, subCodesOut);

    CPPUNIT_ASSERT_EQUAL((size_t)0, subCodesOut.size());
  }

  void testRetrieveS5Records_SrvTypeNotMatch_CarrierMatch()
  {
    std::vector<SubCodeInfo*> subCodesIn;
    subCodesIn += createS5Record('C');

    _mdh->setSubCodes(&subCodesIn);
    addAirSegToItin("LO", "LO");

    const BaggageTravel* baggageTravel = createBaggageTravel();

    std::vector<const SubCodeInfo*> subCodesOut;
    _strategy->retrieveS5Records(baggageTravel, subCodesOut);

    CPPUNIT_ASSERT_EQUAL((size_t)0, subCodesOut.size());
  }

  void testRetrieveS5Records_SrvTypeNotMatch_CarrierNotMatch()
  {
    std::vector<SubCodeInfo*> subCodesIn;
    subCodesIn += createS5Record('C');

    _mdh->setSubCodes(&subCodesIn);
    addAirSegToItin("LO", "AA");

    const BaggageTravel* baggageTravel = createBaggageTravel();

    std::vector<const SubCodeInfo*> subCodesOut;
    _strategy->retrieveS5Records(baggageTravel, subCodesOut);

    CPPUNIT_ASSERT_EQUAL((size_t)0, subCodesOut.size());
  }

  void testRetrieveS5Records_SrvTypeMatch_CarrierMatch_ConcurX()
  {
    std::vector<SubCodeInfo*> subCodesIn;
    subCodesIn += createS5Record('E', 'X');

    _mdh->setSubCodes(&subCodesIn);
    addAirSegToItin("LO", "LO");

    const BaggageTravel* baggageTravel = createBaggageTravel();

    std::vector<const SubCodeInfo*> subCodesOut;
    _strategy->retrieveS5Records(baggageTravel, subCodesOut);

    CPPUNIT_ASSERT_EQUAL((size_t)1, subCodesOut.size());
  }

  void testRetrieveS5Records_SrvTypeMatch_CarrierMatch_ConcurOther()
  {
    std::vector<SubCodeInfo*> subCodesIn;
    subCodesIn += createS5Record('E', '1');

    _mdh->setSubCodes(&subCodesIn);
    addAirSegToItin("LO", "LO");

    const BaggageTravel* baggageTravel = createBaggageTravel();

    std::vector<const SubCodeInfo*> subCodesOut;

    _strategy->retrieveS5Records(baggageTravel, subCodesOut);
    CPPUNIT_ASSERT_EQUAL((size_t)0, subCodesOut.size());
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(EmbargoesDataStrategyTest);
} // tse
