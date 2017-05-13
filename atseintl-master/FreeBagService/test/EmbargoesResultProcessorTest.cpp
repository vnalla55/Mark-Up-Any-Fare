#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/AirSeg.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/Agent.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/ServicesDescription.h"
#include "DBAccess/ServicesSubGroup.h"
#include "DBAccess/SubCodeInfo.h"
#include "FreeBagService/EmbargoesBaggageTextFormatter.h"
#include "FreeBagService/EmbargoesResultProcessor.h"
#include "FreeBagService/test/AirSegBuilder.h"
#include "FreeBagService/test/BaggageTravelBuilder.h"
#include "FreeBagService/test/S5Builder.h"
#include "FreeBagService/test/S7Builder.h"
#include "ServiceFees/OCFees.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{

namespace
{
class FarePathBuilder
{
  TestMemHandle* _memHandle;
  FarePath* _farePath;

public:
  FarePathBuilder(TestMemHandle* memHandle) : _memHandle(memHandle)
  {
    _farePath = _memHandle->create<FarePath>();
  }

  FarePathBuilder& withBaggageTravels(std::vector<const BaggageTravel*>& bgTravels)
  {
    _farePath->baggageTravelsPerSector() = bgTravels;
    return *this;
  }

  FarePath* build() { return _farePath; }
};
}

class EmbargoesResultProcessorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(EmbargoesResultProcessorTest);
  CPPUNIT_TEST(testEmbargoesResultProcessorTest_mergeAll);
  CPPUNIT_TEST(testEmbargoesResultProcessorTest_mergeCityPair);
  CPPUNIT_TEST(testEmbargoesResultProcessorTest_differentCxr);
  CPPUNIT_TEST(testEmbargoesResultProcessorTest_differentEmbargoes);
  CPPUNIT_TEST(testEmbargoesResultProcessorTest_empty);
  CPPUNIT_TEST_SUITE_END();

public:
  AirSeg* createAirSeg(LocCode origin, LocCode dest, CarrierCode operatingCarrier)
  {
    return AirSegBuilder(&_memHandle)
        .withMulticity("PAR", "LON")
        .withAirports(origin, dest)
        .withLocations(origin, dest)
        .withPnr(123)
        .withMarketingCarrier("LH")
        .withOperatingCarrier(operatingCarrier)
        .build();
  }

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _trx = _memHandle.create<PricingTrx>();
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();

    _mdh = _memHandle.create<MyDataHandle>();
    _formatter = _memHandle.create<EmbargoesBaggageTextFormatter>(*_trx);
    _resultProcessor = _memHandle.create<EmbargoesResultProcessor>(*_trx);
  }

  void tearDown() { _memHandle.clear(); }

  BaggageTravel*
  buildBaggageTravel_1EmbT1(LocCode origin, LocCode dest, CarrierCode operatingCarrier)
  {
    OptionalServicesInfo* s7 = S7Builder(&_memHandle).withFirstOccurrence(2).build();
    SubCodeInfo* s5 = S5Builder(&_memHandle)
                          .withGroup("BG")
                          .withSubCode("0GO")
                          .withDesc("45", "8Z")
                          .build();
    BaggageTravel* bgTravel =
        BaggageTravelBuilder(&_memHandle).addEmbargoOCFees(s5, s7).withTrx(_trx).build();

    std::vector<TravelSeg*>* travelSegVector = _memHandle.create<std::vector<TravelSeg*> >();
    travelSegVector->push_back(createAirSeg(origin, dest, operatingCarrier));
    bgTravel->updateSegmentsRange(travelSegVector->begin(), travelSegVector->end());
    bgTravel->_MSS = travelSegVector->begin();

    return bgTravel;
  }

  BaggageTravel*
  buildBaggageTravel_1EmbT2(LocCode origin, LocCode dest, CarrierCode operatingCarrier)
  {
    OptionalServicesInfo* s7 = S7Builder(&_memHandle).withFirstOccurrence(-1).build();
    SubCodeInfo* s5 = S5Builder(&_memHandle)
                          .withGroup("PT")
                          .withSubCode("0AZ")
                          .withCommercialName("MEDIUM PET IN HOLD")
                          .build();

    BaggageTravel* bgTravel =
        BaggageTravelBuilder(&_memHandle).addEmbargoOCFees(s5, s7).withTrx(_trx).build();

    std::vector<TravelSeg*>* travelSegVector = _memHandle.create<std::vector<TravelSeg*> >();
    travelSegVector->push_back(createAirSeg(origin, dest, operatingCarrier));
    bgTravel->updateSegmentsRange(travelSegVector->begin(), travelSegVector->end());
    bgTravel->_MSS = travelSegVector->begin();

    return bgTravel;
  }

  void testEmbargoesResultProcessorTest_mergeAll()
  {
    std::vector<const BaggageTravel*> baggageTravels;
    baggageTravels.push_back(buildBaggageTravel_1EmbT1("ORY", "LHR", "AA"));
    baggageTravels.push_back(buildBaggageTravel_1EmbT1("ORY", "LHR", "AA"));

    std::string result = _resultProcessor->formatEmbargoesText(baggageTravels);

    std::string expectedResult = "  \nEMBARGOES-APPLY TO EACH PASSENGER\n"
                                 "ORYLHR ORYLHR-AA\n"
                                 "ONLY 1 EXCESS UP TO 100 POUNDS/45 KILOGRAMS AND UP TO 115 LINEAR "
                                 "INCHES/292 LINEAR CENTIMETERS PERMITTED\n";

    CPPUNIT_ASSERT_EQUAL(expectedResult, result);
  }

  void testEmbargoesResultProcessorTest_mergeCityPair()
  {
    std::vector<const BaggageTravel*> baggageTravels;
    baggageTravels.push_back(buildBaggageTravel_1EmbT1("ORY", "LHR", "AA"));
    baggageTravels.push_back(buildBaggageTravel_1EmbT1("LHR", "ORD", "AA"));

    std::string result = _resultProcessor->formatEmbargoesText(baggageTravels);

    std::string expectedResult = "  \nEMBARGOES-APPLY TO EACH PASSENGER\n"
                                 "ORYLHR LHRORD-AA\n"
                                 "ONLY 1 EXCESS UP TO 100 POUNDS/45 KILOGRAMS AND UP TO 115 LINEAR "
                                 "INCHES/292 LINEAR CENTIMETERS PERMITTED\n";

    CPPUNIT_ASSERT_EQUAL(expectedResult, result);
  }

  void testEmbargoesResultProcessorTest_differentCxr()
  {
    std::vector<const BaggageTravel*> baggageTravels;
    baggageTravels.push_back(buildBaggageTravel_1EmbT1("ORY", "LHR", "AA"));
    baggageTravels.push_back(buildBaggageTravel_1EmbT1("ORY", "LHR", "LH"));

    std::string result = _resultProcessor->formatEmbargoesText(baggageTravels);

    std::string expectedResult = "  \nEMBARGOES-APPLY TO EACH PASSENGER\n"
                                 "ORYLHR-AA\n"
                                 "ONLY 1 EXCESS UP TO 100 POUNDS/45 KILOGRAMS AND UP TO 115 LINEAR "
                                 "INCHES/292 LINEAR CENTIMETERS PERMITTED\n"
                                 "ORYLHR-LH\n"
                                 "ONLY 1 EXCESS UP TO 100 POUNDS/45 KILOGRAMS AND UP TO 115 LINEAR "
                                 "INCHES/292 LINEAR CENTIMETERS PERMITTED\n";

    CPPUNIT_ASSERT_EQUAL(expectedResult, result);
  }

  void testEmbargoesResultProcessorTest_differentEmbargoes()
  {
    std::vector<const BaggageTravel*> baggageTravels;
    baggageTravels.push_back(buildBaggageTravel_1EmbT1("ORY", "LHR", "AA"));
    baggageTravels.push_back(buildBaggageTravel_1EmbT2("LHR", "ORD", "AA"));

    std::string result = _resultProcessor->formatEmbargoesText(baggageTravels);

    std::string expectedResult = "  \nEMBARGOES-APPLY TO EACH PASSENGER\n"
                                 "ORYLHR-AA\n"
                                 "ONLY 1 EXCESS UP TO 100 POUNDS/45 KILOGRAMS AND UP TO 115 LINEAR "
                                 "INCHES/292 LINEAR CENTIMETERS PERMITTED\n"
                                 "LHRORD-AA\n"
                                 "MEDIUM PET IN HOLD NOT PERMITTED\n";

    CPPUNIT_ASSERT_EQUAL(expectedResult, result);
  }

  void testEmbargoesResultProcessorTest_empty()
  {
    std::vector<const BaggageTravel*> baggageTravels;
    std::string result = _resultProcessor->formatEmbargoesText(baggageTravels);

    std::string expectedResult;
    CPPUNIT_ASSERT_EQUAL(expectedResult, result);
  }

private:
  class MyDataHandle : public DataHandleMock
  {
    TestMemHandle _memHandle;

    const ServicesDescription* getServicesDescription(const ServiceGroupDescription& id)
    {
      if (id == "45")
      {
        ServicesDescription* ret = _memHandle.create<ServicesDescription>();
        ret->description() = "UP TO 100 POUNDS/45 KILOGRAMS";
        return ret;
      }
      else if (id == "8Z")
      {
        ServicesDescription* ret = _memHandle.create<ServicesDescription>();
        ret->description() = "UP TO 115 LINEAR INCHES/292 LINEAR CENTIMETERS";
        return ret;
      }
      else if (id == "23")
      {
        ServicesDescription* ret = _memHandle.create<ServicesDescription>();
        ret->description() = "UP TO 50 POUNDS/23 KILOGRAMS";
        return ret;
      }
      else if (id == "6U")
      {
        ServicesDescription* ret = _memHandle.create<ServicesDescription>();
        ret->description() = "UP TO 62 LINEAR INCHES/158 LINEAR CENTIMETERS";
        return ret;
      }
      else if (id == "BX")
      {
        ServicesDescription* ret = _memHandle.create<ServicesDescription>();
        ret->description() = "BOX";
        return ret;
      }
      else if (id == "B6")
      {
        ServicesDescription* ret = _memHandle.create<ServicesDescription>();
        ret->description() = "SIXTH BAG";
        return ret;
      }
      else if (id == "XS")
      {
        ServicesDescription* ret = _memHandle.create<ServicesDescription>();
        ret->description() = "BETWEEN 51 POUNDS/24 KILOGRAMS AND 70 POUNDS/32 KG";
        return ret;
      }

      return 0;
    }

    const ServicesSubGroup*
    getServicesSubGroup(const ServiceGroup& serviceGroup, const ServiceGroup& serviceSubGroup)
    {
      ServicesSubGroup* servicesSubGroup = _memHandle.create<ServicesSubGroup>();
      servicesSubGroup->serviceGroup() = serviceGroup;
      servicesSubGroup->serviceSubGroup() = serviceSubGroup;

      if (serviceGroup == "BG")
      {
        if (serviceSubGroup == "SP")
          servicesSubGroup->definition() = "SPORTING EQUIPMENT";
        else if (serviceSubGroup == "SR")
          servicesSubGroup->definition() = "SKIM BOARD";
        else if (serviceSubGroup == "XS")
          servicesSubGroup->definition() = "BAGGAGE EXCESS";
      }
      else if (serviceGroup == "PT")
      {
        if (serviceSubGroup == "PC")
          servicesSubGroup->definition() = "IN CABIN";
        else if (serviceSubGroup == "PH")
          servicesSubGroup->definition() = "IN HOLD";
      }
      return servicesSubGroup;
    }
  };

  TestMemHandle _memHandle;
  PricingTrx* _trx;
  MyDataHandle* _mdh;
  EmbargoesBaggageTextFormatter* _formatter;
  EmbargoesResultProcessor* _resultProcessor;
};

CPPUNIT_TEST_SUITE_REGISTRATION(EmbargoesResultProcessorTest);

} // tse
