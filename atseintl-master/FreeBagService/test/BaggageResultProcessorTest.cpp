#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "Common/Config/ConfigMan.h"
#include "Common/MetricsMan.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/BaggageCharge.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/Billing.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "FreeBagService/BaggageResultProcessor.h"
#include "FreeBagService/BaggageTextFormatter.h"
#include "FreeBagService/test/AirSegBuilder.h"
#include "FreeBagService/test/BaggageTravelBuilder.h"
#include "FreeBagService/test/S5Builder.h"
#include "FreeBagService/test/S7Builder.h"
#include "ServiceFees/OCFees.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/MockGlobal.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{
using boost::assign::operator+=;

// MOCKS
namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

  const std::vector<SubCodeInfo*>& getSubCode(const VendorCode& vendor,
                                              const CarrierCode& carrier,
                                              const ServiceTypeCode& serviceTypeCode,
                                              const ServiceGroup& serviceGroup,
                                              const DateTime& date)
  {
    std::vector<SubCodeInfo*>* ret = _memHandle.create<std::vector<SubCodeInfo*> >();
    if (serviceTypeCode == "OC" && serviceGroup == "BG")
    {
      if (vendor == "ATP")
      {
        ret->push_back(S5Builder(&_memHandle)
                           .withSubCode("0F2")
                           .withDesc("12")
                           .withFltTktMerchInd(BAGGAGE_CHARGE)
                           .build());
        return *ret;
      }
      else if (vendor == "MMGR")
      {
        return *ret;
      }
    }
    return DataHandleMock::getSubCode(vendor, carrier, serviceTypeCode, serviceGroup, date);
  }
};
}

class BaggageResultProcessorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BaggageResultProcessorTest);

  CPPUNIT_TEST(testProcess);

  CPPUNIT_TEST(testBuildBaggageResponse_NoAllowance);
  CPPUNIT_TEST(testBuildBaggageResponse_NoAllowance_FirstCheckInCxr);
  CPPUNIT_TEST(testBuildBaggageResponse_OnlyAllowance);
  CPPUNIT_TEST(testBuildBaggageResponse_NoAvailNoCharges_X);

  CPPUNIT_TEST(testProcessAdditionalAllowances);
  CPPUNIT_TEST(testProcessFeesAtEachCheck);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    MockGlobal::setMetricsMan(_memHandle.create<tse::MetricsMan>());
    _memHandle.create<TestConfigInitializer>();

    _mdh = _memHandle.create<MyDataHandle>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    _trx->ticketingDate() = DateTime(9999, 1, 1);
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->billing() = _memHandle.create<Billing>();
    _processor = _memHandle.create<BaggageResultProcessor>(*_trx);
  }

  void tearDown()
  {
    _memHandle.clear();
    MockGlobal::clear();
  }

protected:
  TestMemHandle _memHandle;
  DataHandleMock* _mdh;
  PricingTrx* _trx;
  BaggageResultProcessor* _processor;

public:
  // TESTS
  void testProcess()
  {
    FarePath firstFarePath;
    FarePath secondFarePath;

    AirSeg firstAirSeg;
    AirSeg secondAirSeg;
    std::vector<TravelSeg*> travelSegs;
    travelSegs += &firstAirSeg;
    travelSegs += &secondAirSeg;

    std::vector<BaggageTravel*> baggageTravels;
    baggageTravels +=
        BaggageTravelBuilder(&_memHandle)
            .withFarePath(&firstFarePath)
            .withAllowanceS7(S7Builder(&_memHandle).withBaggagePcs(3).withBaggageWeight(-1).build())
            .withTravelSegMore(travelSegs)
            .withTrx(_trx)
            .build();

    baggageTravels +=
        BaggageTravelBuilder(&_memHandle)
            .withFarePath(&secondFarePath)
            .withAllowanceS7(
                 S7Builder(&_memHandle).withBaggagePcs(-1).withBaggageWeight(30, 'K').build())
            .withTravelSegMore(travelSegs)
            .withTrx(_trx)
            .build();

    _processor->buildAllowanceText(baggageTravels, false);

    std::string expected;
    expected = "03";
    expected += BaggageTextFormatter::PIECES_UNIT_CODE;
    CPPUNIT_ASSERT_EQUAL(expected, firstFarePath.baggageAllowance().find(&firstAirSeg)->second);
    CPPUNIT_ASSERT_EQUAL(expected, firstFarePath.baggageAllowance().find(&secondAirSeg)->second);

    expected = "30K";
    CPPUNIT_ASSERT_EQUAL(expected, secondFarePath.baggageAllowance().find(&firstAirSeg)->second);
    CPPUNIT_ASSERT_EQUAL(expected, secondFarePath.baggageAllowance().find(&secondAirSeg)->second);
  }

  void testBuildBaggageResponse_NoAllowance()
  {
    AirSeg* carrierTravelSeg =
        AirSegBuilder(&_memHandle).withLocations("KRK", "WAW").withMarketingCarrier("LO").build();

    AirSeg* btSeg1 = AirSegBuilder(&_memHandle).withLocations("KRK", "WAW").build();
    AirSeg* btSeg2 = AirSegBuilder(&_memHandle).withLocations("WAW", "GDA").build();

    std::vector<TravelSeg*> btSegs;
    btSegs += btSeg1;
    btSegs += btSeg2;

    FarePath farePath;

    BaggageTravel* bt = BaggageTravelBuilder(&_memHandle)
                            .withAllowanceS7(0)
                            .withCarrierTravelSeg(carrierTravelSeg)
                            .withTravelSegMore(btSegs)
                            .withFarePath(&farePath)
                            .withTrx(_trx)
                            .build();

    Itin itin;
    itin.setBaggageTripType(BaggageTripType::TO_FROM_US);
    farePath.baggageTravels() += bt;
    farePath.itin() = &itin;

    std::string unknown = BaggageTextFormatter::UNKNOWN_INDICATOR +
                          BaggageTextFormatter::UNKNOWN_FEE_MSG +
                          carrierTravelSeg->marketingCarrierCode();

    std::string expected = "BAG ALLOWANCE     -KRKGDA-*/LO\n" + unknown + "\n" +
                           "1STCHECKED BAG FEE-KRKGDA-*/LO\n" + unknown + "\n" +
                           "2NDCHECKED BAG FEE-KRKGDA-*/LO\n" + unknown + "\n";
    std::string result = _processor->buildBaggageResponse(&farePath);

    CPPUNIT_ASSERT_EQUAL(expected, result);
    CPPUNIT_ASSERT(!_processor->_additionalAllowancesApply[&farePath]);
  }

  void testBuildBaggageResponse_NoAllowance_FirstCheckInCxr()
  {
    AirSeg* btSeg1 =
        AirSegBuilder(&_memHandle).withLocations("KRK", "WAW").withMarketingCarrier("AA").build();

    AirSeg* btSeg2 = AirSegBuilder(&_memHandle).withLocations("WAW", "GDA").build();

    std::vector<TravelSeg*> btSegs;
    btSegs += btSeg1;
    btSegs += btSeg2;

    BaggageTravel* bt = BaggageTravelBuilder(&_memHandle)
                            .withAllowanceS7(0)
                            .withCarrierTravelSeg(0)
                            .withTravelSegMore(btSegs)
                            .withTrx(_trx)
                            .build();

    Itin itin;
    itin.setBaggageTripType(BaggageTripType::TO_FROM_US);
    FarePath farePath;
    farePath.baggageTravels() += bt;
    farePath.itin() = &itin;

    std::string unknown = BaggageTextFormatter::UNKNOWN_INDICATOR +
                          BaggageTextFormatter::UNKNOWN_FEE_MSG + btSeg1->marketingCarrierCode();

    std::string expected = "BAG ALLOWANCE     -KRKGDA-*/AA\n" + unknown + "\n" +
                           "1STCHECKED BAG FEE-KRKGDA-*/AA\n" + unknown + "\n" +
                           "2NDCHECKED BAG FEE-KRKGDA-*/AA\n" + unknown + "\n";
    std::string result = _processor->buildBaggageResponse(&farePath);

    CPPUNIT_ASSERT_EQUAL(expected, result);
    CPPUNIT_ASSERT(!_processor->_additionalAllowancesApply[&farePath]);
  }

  void testBuildBaggageResponse_OnlyAllowance()
  {
    AirSeg* carrierTravelSeg =
        AirSegBuilder(&_memHandle).withLocations("KRK", "WAW").withMarketingCarrier("LO").build();

    AirSeg* btSeg1 = AirSegBuilder(&_memHandle).withLocations("KRK", "WAW").build();
    AirSeg* btSeg2 = AirSegBuilder(&_memHandle).withLocations("WAW", "GDA").build();

    std::vector<TravelSeg*> btSegs;
    btSegs += btSeg1;
    btSegs += btSeg2;

    BaggageTravel* bt =
        BaggageTravelBuilder(&_memHandle)
            .withAllowanceS7(S7Builder(&_memHandle)
                                 .withBaggagePcs(-1)
                                 .withBaggageWeight(30, BaggageTextFormatter::POUNDS_UNIT)
                                 .build())
            .withCarrierTravelSeg(carrierTravelSeg)
            .withTravelSegMore(btSegs)
            .withTrx(_trx)
            .build();

    Itin itin;
    itin.setBaggageTripType(BaggageTripType::TO_FROM_US);
    FarePath farePath;
    farePath.baggageTravels() += bt;
    farePath.itin() = &itin;

    std::string expected = "BAG ALLOWANCE     -KRKGDA-30LB/LO\n";
    std::string result = _processor->buildBaggageResponse(&farePath);

    CPPUNIT_ASSERT_EQUAL(expected, result);
    CPPUNIT_ASSERT(_processor->_additionalAllowancesApply[&farePath]);
  }

  void testBuildBaggageResponse_NoAvailNoCharges_X()
  {
    AirSeg* carrierTravelSeg =
        AirSegBuilder(&_memHandle).withLocations("KRK", "WAW").withMarketingCarrier("LO").build();

    AirSeg* btSeg1 = AirSegBuilder(&_memHandle).withLocations("KRK", "WAW").build();
    AirSeg* btSeg2 = AirSegBuilder(&_memHandle).withLocations("WAW", "GDA").build();

    std::vector<TravelSeg*> btSegs;
    btSegs += btSeg1;
    btSegs += btSeg2;

    Itin itin;
    itin.setBaggageTripType(BaggageTripType::TO_FROM_US);
    FarePath farePath;
    farePath.itin() = &itin;

    BaggageTravel* bt =
        BaggageTravelBuilder(&_memHandle)
            .withAllowanceS7(S7Builder(&_memHandle)
                                 .withBaggagePcs(0)
                                 .withBaggageWeight(0, BaggageTextFormatter::POUNDS_UNIT)
                                 .build())
            .withCarrierTravelSeg(carrierTravelSeg)
            .withTravelSegMore(btSegs)
            .with1stBagS7(S7Builder(&_memHandle).withNotAvailNoCharge('X').build(), &farePath)
            .with2ndBagS7(S7Builder(&_memHandle).withNotAvailNoCharge('X').build(), &farePath)
            .withTrx(_trx)
            .build();

    farePath.baggageTravels() += bt;

    std::string expected = "BAG ALLOWANCE     -KRKGDA-NIL/LO\n"
                           "1STCHECKED BAG FEE-KRKGDA-NOT PERMITTED/LO\n"
                           "2NDCHECKED BAG FEE-KRKGDA-NOT PERMITTED/LO\n";
    std::string result = _processor->buildBaggageResponse(&farePath);

    CPPUNIT_ASSERT_EQUAL(expected, result);
    CPPUNIT_ASSERT(_processor->_additionalAllowancesApply[&farePath]);
  }

  void testProcessAdditionalAllowances()
  {
    FarePath fp1;
    FarePath fp2;
    FarePath fp3;

    _processor->_additionalAllowancesApply[&fp1] = true;
    _processor->_additionalAllowancesApply[&fp2] = false;
    _processor->_additionalAllowancesApply[&fp3] = true;

    _processor->processAdditionalAllowances();

    std::string expected = "<ADD>\n";

    CPPUNIT_ASSERT_EQUAL(expected, fp1.baggageResponse());
    CPPUNIT_ASSERT(fp2.baggageResponse().empty());
    CPPUNIT_ASSERT_EQUAL(expected, fp3.baggageResponse());
  }

  void testProcessFeesAtEachCheck()
  {
    FarePath fp1;
    FarePath fp2;
    FarePath fp3;

    _processor->_feesAtEachCheckApply[&fp1] = true;
    _processor->_feesAtEachCheckApply[&fp2] = false;
    _processor->_feesAtEachCheckApply[&fp3] = true;

    _processor->processFeesAtEachCheck();

    std::string expected = "**BAG FEES APPLY AT EACH CHECK IN LOCATION\n";

    CPPUNIT_ASSERT_EQUAL(expected, fp1.baggageResponse());
    CPPUNIT_ASSERT(fp2.baggageResponse().empty());
    CPPUNIT_ASSERT_EQUAL(expected, fp3.baggageResponse());
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(BaggageResultProcessorTest);
} // tse
