#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/ServicesDescription.h"
#include "DBAccess/ServicesSubGroup.h"
#include "DBAccess/SubCodeInfo.h"
#include "DataModel/BaggageCharge.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "FreeBagService/CarryOnBaggageTextFormatter.h"
#include "FreeBagService/test/S5Builder.h"
#include "FreeBagService/test/S7Builder.h"

namespace tse
{
using boost::assign::operator+=;

// MOCKS
namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

  const ServicesDescription* getServicesDescription(const ServiceGroupDescription& id)
  {
    if (id == "AA")
    {
      ServicesDescription* ret = _memHandle.create<ServicesDescription>();
      ret->description() = "AA DESCRIPTION";
      return ret;
    }
    else if (id == "FI")
    {
      ServicesDescription* ret = _memHandle.create<ServicesDescription>();
      ret->description() = "FISHING EQUIPMENT";
      return ret;
    }
    else if (id == "1Z")
    {
      ServicesDescription* ret = _memHandle.create<ServicesDescription>();
      ret->description() = "OVER 15 KILOGRAMS";
      return ret;
    }
    else if (id == "X5")
    {
      ServicesDescription* ret = _memHandle.create<ServicesDescription>();
      ret->description() = "EXCESS PIECE AND SIZE";
      return ret;
    }
    else if (id == "2D")
    {
      ServicesDescription* ret = _memHandle.create<ServicesDescription>();
      ret->description() = "OVER 79 INCHES/200 CENTIMETERS";
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
      servicesSubGroup->definition() = "SPORTING EQUIPMENT";
    }
    else if (serviceGroup == "PT")
    {
      if (serviceSubGroup == "PH")
        servicesSubGroup->definition() = "IN HOLD";
      else if (serviceSubGroup == "PC")
        servicesSubGroup->definition() = "IN CABIN";
    }

    return servicesSubGroup;
  }
};
}

class CarryOnBaggageTextFormatterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CarryOnBaggageTextFormatterTest);

  CPPUNIT_TEST(testFormatCarryOnChargeText);
  CPPUNIT_TEST(testFormatCarryOnChargeText_NoS7);

  CPPUNIT_TEST(testGetChargeOccurrenceText_None);
  CPPUNIT_TEST(testGetChargeOccurrenceText_First);
  CPPUNIT_TEST(testGetChargeOccurrenceText_FirstAndLast);
  CPPUNIT_TEST(testGetChargeOccurrenceText_FirstAndAbove);
  CPPUNIT_TEST(testGetChargeOccurrenceText_SingleS7Matched);
  CPPUNIT_TEST(testGetChargeOccurrenceText_SingleS7Matched_NotPermitted);
  CPPUNIT_TEST(testGetChargeOccurrenceText_Additional);

  CPPUNIT_TEST(testGetOrdinalSuffixText_0);
  CPPUNIT_TEST(testGetOrdinalSuffixText_1);
  CPPUNIT_TEST(testGetOrdinalSuffixText_2);
  CPPUNIT_TEST(testGetOrdinalSuffixText_3);
  CPPUNIT_TEST(testGetOrdinalSuffixText_4);
  CPPUNIT_TEST(testGetOrdinalSuffixText_11);
  CPPUNIT_TEST(testGetOrdinalSuffixText_21);
  CPPUNIT_TEST(testGetOrdinalSuffixText_99);

  CPPUNIT_TEST(testGetExcessBaggageWeightText_None);
  CPPUNIT_TEST(testGetExcessBaggageWeightText_Kilos);
  CPPUNIT_TEST(testGetExcessBaggageWeightText_Pounds);

  CPPUNIT_TEST(testGetCarryOnChargeDescriptionText_Description);
  CPPUNIT_TEST(testGetCarryOnChargeDescriptionText_ComercialName);

  CPPUNIT_TEST(testGetChargeApplicactionText_H);
  CPPUNIT_TEST(testGetChargeApplicactionText_C);
  CPPUNIT_TEST(testGetChargeApplicactionText_P);
  CPPUNIT_TEST(testGetChargeApplicactionText_K);
  CPPUNIT_TEST(testGetChargeApplicactionText_F);
  CPPUNIT_TEST(testGetChargeApplicactionText_X);

  CPPUNIT_TEST(testGetServiceSubGroup_group_BG_subGroup_SA);
  CPPUNIT_TEST(testGetServiceSubGroup_group_PT_subGroup_PH);
  CPPUNIT_TEST(testGetServiceSubGroup_group_PT_subGroup_PC);

  CPPUNIT_TEST(testGetS5Descriptions_group_PT_subGroup_PH_pieces_1);
  CPPUNIT_TEST(testGetS5Descriptions_group_PT_subGroup_PH_pieces_2);

  CPPUNIT_TEST(testGetS5Descriptions_subGroup_CY_pieces_1);
  CPPUNIT_TEST(testGetS5Descriptions_subGroup_CY_pieces_2);

  CPPUNIT_TEST(testGetS5Descriptions_NO_Description1);
  CPPUNIT_TEST(testGetS5Descriptions_NO_Description2);
  CPPUNIT_TEST(testGetS5Descriptions_NO_Description1_NO_Description2);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _mdh = _memHandle.create<MyDataHandle>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->setRequest(_memHandle.create<PricingRequest>());
    PricingOptions* opts = _memHandle.create<PricingOptions>();
    opts->currencyOverride() = _defaultCurrency;
    _trx->setOptions(opts);
    _formatter = _memHandle.create<CarryOnBaggageTextFormatter>(*_trx);
  }

  void tearDown() { _memHandle.clear(); }

protected:
  TestMemHandle _memHandle;
  DataHandleMock* _mdh;
  PricingTrx* _trx;
  CarryOnBaggageTextFormatter* _formatter;

  static const CurrencyCode _defaultCurrency;

public:
  // TESTS
  void testFormatCarryOnChargeText()
  {
    BaggageCharge charge;
    charge.subCodeInfo() = S5Builder(&_memHandle).withCommercialName("COMMERCIAL NAME").build();
    charge.optFee() = S7Builder(&_memHandle)
                          .withBaggageOccurrence(1, 2)
                          .withBaggageWeight(10, 'L')
                          .withApplication('H')
                          .build();
    charge.feeCurrency() = _defaultCurrency;
    charge.feeAmount() = 123.45;

    CPPUNIT_ASSERT_EQUAL(std::string("1ST-2ND OVER 10LB COMMERCIAL NAME-PLN123.45 PER KILO"),
                         _formatter->formatCarryOnChargeText(&charge));
  }

  void testFormatCarryOnChargeText_NoS7()
  {
    BaggageCharge charge;
    charge.subCodeInfo() = S5Builder(&_memHandle).withCommercialName("COMMERCIAL NAME").build();
    charge.optFee() = 0;
    charge.feeCurrency() = _defaultCurrency;
    charge.feeAmount() = 123.45;

    CPPUNIT_ASSERT_EQUAL(std::string("COMMERCIAL NAME-CARRY ON FEES UNKNOWN-CONTACT CARRIER"),
                         _formatter->formatCarryOnChargeText(&charge));
  }

  void testGetChargeOccurrenceText_None()
  {

    CPPUNIT_ASSERT(
        _formatter->getChargeOccurrenceText(
                        S7Builder(&_memHandle).withBaggageOccurrence(0, 1).build()).empty());
  }

  void testGetChargeOccurrenceText_First()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("1ST "),
                         _formatter->getChargeOccurrenceText(
                             S7Builder(&_memHandle).withBaggageOccurrence(1, 1).build()));
  }

  void testGetChargeOccurrenceText_FirstAndLast()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("1ST-2ND "),
                         _formatter->getChargeOccurrenceText(
                             S7Builder(&_memHandle).withBaggageOccurrence(1, 2).build()));
  }

  void testGetChargeOccurrenceText_FirstAndAbove()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("1ST AND EACH ABOVE 1ST "),
                         _formatter->getChargeOccurrenceText(
                             S7Builder(&_memHandle).withBaggageOccurrence(1, 0).build()));
  }

  void testGetChargeOccurrenceText_SingleS7Matched()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("1ST-2ND "),
                         _formatter->getChargeOccurrenceText(
                             S7Builder(&_memHandle).withBaggageOccurrence(1, 2).build(), true));
  }

  void testGetChargeOccurrenceText_SingleS7Matched_NotPermitted()
  {
    CPPUNIT_ASSERT(
        _formatter->getChargeOccurrenceText(
                        S7Builder(&_memHandle)
                            .withBaggageOccurrence(1, 2)
                            .withNotAvailNoCharge(BaggageTextFormatter::NOT_PERMITTED_IND)
                            .build(),
                        true).empty());
  }

  void testGetChargeOccurrenceText_Additional()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("ADDITIONAL "),
                         _formatter->getChargeOccurrenceText(
                             S7Builder(&_memHandle)
                                 .withFirstOccurrence(2)
                                 .withNotAvailNoCharge(BaggageTextFormatter::NOT_PERMITTED_IND)
                                 .build(),
                             false));
  }

  void testGetOrdinalSuffixText_0()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("TH"), _formatter->getOrdinalSuffixText(0));
  }

  void testGetOrdinalSuffixText_1()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("ST"), _formatter->getOrdinalSuffixText(1));
  }

  void testGetOrdinalSuffixText_2()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("ND"), _formatter->getOrdinalSuffixText(2));
  }

  void testGetOrdinalSuffixText_3()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("RD"), _formatter->getOrdinalSuffixText(3));
  }

  void testGetOrdinalSuffixText_4()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("TH"), _formatter->getOrdinalSuffixText(4));
  }

  void testGetOrdinalSuffixText_11()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("TH"), _formatter->getOrdinalSuffixText(11));
  }

  void testGetOrdinalSuffixText_21()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("ST"), _formatter->getOrdinalSuffixText(21));
  }

  void testGetOrdinalSuffixText_99()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("TH"), _formatter->getOrdinalSuffixText(99));
  }

  void testGetExcessBaggageWeightText_None()
  {
    CPPUNIT_ASSERT(_formatter->getExcessBaggageWeightText(
                                   S7Builder(&_memHandle).withBaggageWeight(0).build()).empty());
  }

  void testGetExcessBaggageWeightText_Pounds()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("OVER 5KG "),
                         _formatter->getExcessBaggageWeightText(
                             S7Builder(&_memHandle).withBaggageWeight(5, 'K').build()));
  }

  void testGetExcessBaggageWeightText_Kilos()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("OVER 10LB "),
                         _formatter->getExcessBaggageWeightText(
                             S7Builder(&_memHandle).withBaggageWeight(10, 'L').build()));
  }

  void testGetCarryOnChargeDescriptionText_Description()
  {
    CPPUNIT_ASSERT_EQUAL(
        std::string("AA DESCRIPTION"),
        _formatter->getCarryOnChargeDescriptionText(S5Builder(&_memHandle)
                                                        .withDesc("AA")
                                                        .withCommercialName("COMMERCIAL NAME")
                                                        .build()));
  }

  void testGetCarryOnChargeDescriptionText_ComercialName()
  {
    CPPUNIT_ASSERT_EQUAL(
        std::string("COMMERCIAL NAME"),
        _formatter->getCarryOnChargeDescriptionText(S5Builder(&_memHandle)
                                                        .withDesc("")
                                                        .withCommercialName("COMMERCIAL NAME")
                                                        .build()));
  }

  void testGetChargeApplicactionText_H()
  {
    CPPUNIT_ASSERT_EQUAL(
        CarryOnBaggageTextFormatter::PER_KILO,
        _formatter->getChargeApplicactionText(S7Builder(&_memHandle).withApplication('H').build()));
  }

  void testGetChargeApplicactionText_C()
  {
    CPPUNIT_ASSERT_EQUAL(
        CarryOnBaggageTextFormatter::PER_KILO,
        _formatter->getChargeApplicactionText(S7Builder(&_memHandle).withApplication('C').build()));
  }

  void testGetChargeApplicactionText_P()
  {
    CPPUNIT_ASSERT_EQUAL(
        CarryOnBaggageTextFormatter::PER_KILO,
        _formatter->getChargeApplicactionText(S7Builder(&_memHandle).withApplication('P').build()));
  }

  void testGetChargeApplicactionText_K()
  {
    CPPUNIT_ASSERT_EQUAL(
        CarryOnBaggageTextFormatter::PER_KILO,
        _formatter->getChargeApplicactionText(S7Builder(&_memHandle).withApplication('K').build()));
  }

  void testGetChargeApplicactionText_F()
  {
    CPPUNIT_ASSERT_EQUAL(
        CarryOnBaggageTextFormatter::PER_5_KILOS,
        _formatter->getChargeApplicactionText(S7Builder(&_memHandle).withApplication('F').build()));
  }

  void testGetChargeApplicactionText_X()
  {
    CPPUNIT_ASSERT(_formatter->getChargeApplicactionText(
                                   S7Builder(&_memHandle).withApplication('X').build()).empty());
  }

  void testGetServiceSubGroup_group_BG_subGroup_SA()
  {
    CPPUNIT_ASSERT_EQUAL(
        std::string("SPORTING EQUIPMENT/"),
        _formatter->getServiceSubGroupText(S5Builder(&_memHandle).withGroup("BG", "SP").build()));
  }

  void testGetServiceSubGroup_group_PT_subGroup_PH()
  {
    CPPUNIT_ASSERT_EQUAL(
        std::string("PET IN HOLD/"),
        _formatter->getServiceSubGroupText(S5Builder(&_memHandle).withGroup("PT", "PH").build()));
  }

  void testGetServiceSubGroup_group_PT_subGroup_PC()
  {
    CPPUNIT_ASSERT_EQUAL(
        std::string("PET IN CABIN/"),
        _formatter->getServiceSubGroupText(S5Builder(&_memHandle).withGroup("PT", "PC").build()));
  }

  void testGetS5Descriptions_group_PT_subGroup_PH_pieces_1()
  {
    std::string expectedString =
        "01/PET IN HOLD/OVER 15 KILOGRAMS AND OVER 79 INCHES/200 CENTIMETERS\n";

    CPPUNIT_ASSERT_EQUAL(
        expectedString,
        _formatter->getS5DescriptionsText(
            S5Builder(&_memHandle).withDesc("1Z", "2D").withGroup("PT", "PH").build(), 1));
  }

  void testGetS5Descriptions_group_PT_subGroup_PH_pieces_2()
  {
    std::string expectedString =
        "02/EACH PIECE PET IN HOLD/OVER 15 KILOGRAMS AND OVER 79 INCHES/200 CENTIMETERS\n";

    CPPUNIT_ASSERT_EQUAL(
        expectedString,
        _formatter->getS5DescriptionsText(
            S5Builder(&_memHandle).withDesc("1Z", "2D").withGroup("PT", "PH").build(), 2));
  }

  void testGetS5Descriptions_subGroup_CY_pieces_1()
  {
    std::string expectedString = "01/OVER 15 KILOGRAMS AND OVER 79 INCHES/200 CENTIMETERS\n";

    CPPUNIT_ASSERT_EQUAL(
        expectedString,
        _formatter->getS5DescriptionsText(
            S5Builder(&_memHandle).withDesc("1Z", "2D").withSubGroup("CY").build(), 1));
  }

  void testGetS5Descriptions_subGroup_CY_pieces_2()
  {
    std::string expectedString =
        "02/EACH PIECE OVER 15 KILOGRAMS AND OVER 79 INCHES/200 CENTIMETERS\n";

    CPPUNIT_ASSERT_EQUAL(
        expectedString,
        _formatter->getS5DescriptionsText(
            S5Builder(&_memHandle).withDesc("1Z", "2D").withSubGroup("CY").build(), 2));
  }

  void testGetS5Descriptions_NO_Description1()
  {
    std::string expectedString = "02/EACH PIECE PREMIUM SEAT\n";

    CPPUNIT_ASSERT_EQUAL(expectedString,
                         _formatter->getS5DescriptionsText(S5Builder(&_memHandle)
                                                               .withDesc("1D", "2D")
                                                               .withSubGroup("CY")
                                                               .withCommercialName("PREMIUM SEAT")
                                                               .build(),
                                                           2));
  }

  void testGetS5Descriptions_NO_Description2()
  {
    std::string expectedString = "02/EACH PIECE PREMIUM SEAT\n";

    CPPUNIT_ASSERT_EQUAL(expectedString,
                         _formatter->getS5DescriptionsText(S5Builder(&_memHandle)
                                                               .withDesc("1Z", "1D")
                                                               .withSubGroup("CY")
                                                               .withCommercialName("PREMIUM SEAT")
                                                               .build(),
                                                           2));
  }

  void testGetS5Descriptions_NO_Description1_NO_Description2()
  {
    std::string expectedString = "02/EACH PIECE PREMIUM SEAT\n";

    CPPUNIT_ASSERT_EQUAL(expectedString,
                         _formatter->getS5DescriptionsText(S5Builder(&_memHandle)
                                                               .withDesc("3D", "2D")
                                                               .withSubGroup("CY")
                                                               .withCommercialName("PREMIUM SEAT")
                                                               .build(),
                                                           2));
  }
};
const CurrencyCode CarryOnBaggageTextFormatterTest::_defaultCurrency = "PLN";
CPPUNIT_TEST_SUITE_REGISTRATION(CarryOnBaggageTextFormatterTest);
} // tse
