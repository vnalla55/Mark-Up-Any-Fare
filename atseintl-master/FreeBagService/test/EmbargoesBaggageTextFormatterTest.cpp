// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/AirSeg.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/ServicesDescription.h"
#include "DBAccess/ServicesSubGroup.h"
#include "DBAccess/SubCodeInfo.h"
#include "FreeBagService/EmbargoesBaggageTextFormatter.h"
#include "FreeBagService/test/AirSegBuilder.h"
#include "FreeBagService/test/BaggageTravelBuilder.h"
#include "FreeBagService/test/S5Builder.h"
#include "FreeBagService/test/S7Builder.h"
#include "ServiceFees/OCFees.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class EmbargoesBaggageTextFormatterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(EmbargoesBaggageTextFormatterTest);
  CPPUNIT_TEST(testFormatEmbargoesText_S5_S7);
  CPPUNIT_TEST(testFormatEmbargoesText_S5_commercialName_s7);
  CPPUNIT_TEST(testFormatEmbargoesText_S5_commercialName_s7_occurrence_3);
  CPPUNIT_TEST(testFormatEmbargoesText_S5_Description1_commercialName);
  CPPUNIT_TEST(testFormatEmbargoesText_S5_Description1_subGroup_SP);
  CPPUNIT_TEST(testFormatEmbargoesText_No_S5_S7);
  CPPUNIT_TEST(testFormatEmbargoesText_S5_No_S7);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _trx = _memHandle.create<PricingTrx>();
    _mdh = _memHandle.create<MyDataHandle>();
    _formatter = _memHandle.create<EmbargoesBaggageTextFormatter>(*_trx);
  }

  void tearDown() { _memHandle.clear(); }

  AirSeg* createAirSeg(uint16_t order = 1)
  {
    return AirSegBuilder(&_memHandle)
        .withMulticity("PAR", "LON")
        .withAirports("ORY", "LHR")
        .withPnr(123)
        .withMarketingCarrier("LH")
        .withOperatingCarrier("LH")
        .withSegmentOrder(order)
        .build();
  }

  void testFormatEmbargoesText_S5_S7()
  {
    OptionalServicesInfo* s7 = S7Builder(&_memHandle).withFirstOccurrence(2).build();
    SubCodeInfo* s5 = S5Builder(&_memHandle)
                          .withGroup("BG")
                          .withSubCode("0GO")
                          .withDesc("45", "8Z")
                          .build();
    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle).addEmbargoOCFees(s5, s7).build();

    std::string expectedResult = "ONLY 1 EXCESS UP TO 100 POUNDS/45 KILOGRAMS AND UP TO 115 LINEAR "
                                 "INCHES/292 LINEAR CENTIMETERS PERMITTED\n";

    std::string result;
    for (OCFees* embargo : bgTravel->_embargoVector)
    {
      result += _formatter->formatEmbargoesText(embargo);
    }

    CPPUNIT_ASSERT_EQUAL(expectedResult, result);
  }

  void testFormatEmbargoesText_S5_commercialName_s7()
  {
    SubCodeInfo* s5first = S5Builder(&_memHandle)
                               .withGroup("BG")
                               .withSubCode("0GO")
                               .withDesc("23", "6U")
                               .build();
    SubCodeInfo* s5second = S5Builder(&_memHandle)
                                .withGroup("PT")
                                .withSubCode("0AZ")
                                .withCommercialName("MEDIUM PET IN HOLD")
                                .build();

    OptionalServicesInfo* s7first = S7Builder(&_memHandle).withFirstOccurrence(2).build();
    OptionalServicesInfo* s7second = S7Builder(&_memHandle).withFirstOccurrence(-1).build();

    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle)
                                  .addEmbargoOCFees(s5second, s7second)
                                  .addEmbargoOCFees(s5first, s7first)
                                  .build();

    std::string expectedResult = "MEDIUM PET IN HOLD NOT PERMITTED\n"
                                 "ONLY 1 EXCESS UP TO 50 POUNDS/23 KILOGRAMS AND UP TO 62 "
                                 "LINEAR INCHES/158 LINEAR CENTIMETERS PERMITTED\n";

    std::string result;
    for (OCFees* embargo : bgTravel->_embargoVector)
    {
      result += _formatter->formatEmbargoesText(embargo);
    }

    CPPUNIT_ASSERT_EQUAL(expectedResult, result);
  }

  void testFormatEmbargoesText_S5_commercialName_s7_occurrence_3()
  {
    SubCodeInfo* s5first = S5Builder(&_memHandle)
                               .withGroup("BG")
                               .withSubCode("0GO")
                               .withDesc("23", "6U")
                               .build();
    SubCodeInfo* s5second = S5Builder(&_memHandle)
                                .withGroup("PT")
                                .withSubCode("0AZ")
                                .withCommercialName("MEDIUM PET IN HOLD")
                                .build();

    OptionalServicesInfo* s7first = S7Builder(&_memHandle).withFirstOccurrence(3).build();
    OptionalServicesInfo* s7second = S7Builder(&_memHandle).withFirstOccurrence(-1).build();

    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle)
                                  .addEmbargoOCFees(s5second, s7second)
                                  .addEmbargoOCFees(s5first, s7first)
                                  .build();

    std::string expectedResult = "MEDIUM PET IN HOLD NOT PERMITTED\n"
                                 "ONLY 2 EXCESS UP TO 50 POUNDS/23 KILOGRAMS AND UP TO 62 "
                                 "LINEAR INCHES/158 LINEAR CENTIMETERS PERMITTED\n";

    std::string result;
    for (OCFees* embargo : bgTravel->_embargoVector)
    {
      result += _formatter->formatEmbargoesText(embargo);
    }

    CPPUNIT_ASSERT_EQUAL(expectedResult, result);
  }

  void testFormatEmbargoesText_S5_Description1_commercialName()
  {
    SubCodeInfo* s5 = S5Builder(&_memHandle)
                          .withGroup("BG")
                          .withSubCode("0IN")
                          .withDesc("BX", "")
                          .withCommercialName("BOX")
                          .build();

    OptionalServicesInfo* s7 = S7Builder(&_memHandle).withFirstOccurrence(-1).build();

    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle).addEmbargoOCFees(s5, s7).build();

    std::string expectedResult = "BOX NOT PERMITTED\n";

    std::string result;
    for (OCFees* embargo : bgTravel->_embargoVector)
    {
      result += _formatter->formatEmbargoesText(embargo);
    }

    CPPUNIT_ASSERT_EQUAL(expectedResult, result);
  }

  void testFormatEmbargoesText_S5_Description1_subGroup_SP()
  {
    SubCodeInfo* s5 = S5Builder(&_memHandle)
                          .withGroup("BG")
                          .withSubGroup("SP")
                          .withSubCode("0GA")
                          .withDesc("XS", "B6")
                          .withCommercialName("SPORT EQUIP LARGE DIMENSIONS")
                          .build();

    OptionalServicesInfo* s7 = S7Builder(&_memHandle).withFirstOccurrence(-1).build();

    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle).addEmbargoOCFees(s5, s7).build();

    std::string expectedResult = "SPORTING EQUIPMENT/BETWEEN 51 POUNDS/24 KILOGRAMS AND 70 "
                                 "POUNDS/32 KG SIXTH BAG NOT PERMITTED\n";

    std::string result;
    for (OCFees* embargo : bgTravel->_embargoVector)
    {
      result += _formatter->formatEmbargoesText(embargo);
    }

    CPPUNIT_ASSERT_EQUAL(expectedResult, result);
  }

  void testFormatEmbargoesText_No_S5_S7()
  {
    OptionalServicesInfo* s7 = S7Builder(&_memHandle).withFirstOccurrence(-1).build();

    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle).addEmbargoOCFees(0, s7).build();

    std::string expectedResult;
    std::string result;
    for (OCFees* embargo : bgTravel->_embargoVector)
    {
      result += _formatter->formatEmbargoesText(embargo);
    }

    CPPUNIT_ASSERT_EQUAL(expectedResult, result);
  }

  void testFormatEmbargoesText_S5_No_S7()
  {
    SubCodeInfo* s5 = S5Builder(&_memHandle)
                          .withGroup("BG")
                          .withSubGroup("SP")
                          .withSubCode("0GA")
                          .withDesc("XS", "B6")
                          .withCommercialName("SPORT EQUIP LARGE DIMENSIONS")
                          .build();

    BaggageTravel* bgTravel = BaggageTravelBuilder(&_memHandle).addEmbargoOCFees(s5, 0).build();

    std::string expectedResult;
    std::string result;
    for (OCFees* embargo : bgTravel->_embargoVector)
    {
      result += _formatter->formatEmbargoesText(embargo);
    }

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
};

CPPUNIT_TEST_SUITE_REGISTRATION(EmbargoesBaggageTextFormatterTest);
}
