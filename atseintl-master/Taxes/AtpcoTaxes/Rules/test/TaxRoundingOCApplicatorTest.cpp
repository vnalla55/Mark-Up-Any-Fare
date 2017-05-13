// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"
#include "test/PaymentDetailMock.h"
#include "Rules/TaxRoundingOCApplicator.h"
#include "ServiceInterfaces/TaxRoundingInfoService.h"

#include <memory>

namespace tax
{
class TaxRoundingInfoServiceMock : public tax::TaxRoundingInfoService
{
public:
  void getTrxRoundingInfo(const tax::type::Nation& nation,
                          tax::type::MoneyAmount& unit,
                          tax::type::TaxRoundingDir& dir) const override
  {
    if (nation == "UK")
    {
      unit = tax::type::MoneyAmount(1, 10);
      dir = tax::type::TaxRoundingDir::NoRounding;
    }
    else if (nation == "US")
    {
      unit = tax::type::MoneyAmount(1);
      dir = tax::type::TaxRoundingDir::RoundDown;
    }
  }

  void getFareRoundingInfo(const type::CurrencyCode&,
                           type::MoneyAmount&,
                           type::TaxRoundingDir&) const override {}

  void getNationRoundingInfo(const type::Nation& /*nation*/,
                            type::MoneyAmount& /*unit*/,
                            type::TaxRoundingDir& /*dir*/) const override {}

  void doStandardRound(tax::type::MoneyAmount& /* amount */,
                       tax::type::MoneyAmount& /* unit */,
                       tax::type::TaxRoundingDir& /* dir */,
                       tax::type::MoneyAmount /* currencyUnit */,
                       bool /* isOcFee */) const override
  {
    // no rounding
  }

  static tax::TaxRoundingInfoService* create() { return new TaxRoundingInfoServiceMock; }
};

class TaxRoundingOCApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxRoundingOCApplicatorTest);
  CPPUNIT_TEST(testIncludeTax_P_0);
  CPPUNIT_TEST(testIncludeTax_P_15);
  CPPUNIT_TEST(testIncludeTax_P_15_P_20);
  CPPUNIT_TEST(testIncludeTax_P_15_P_20_tax);
  CPPUNIT_TEST(testIncludeTax_P_15_P_20_Nearest_001_RoundUp_01);
  CPPUNIT_TEST(testIncludeTax_P_15_P_20_tax_Nearest_001_RoundUp_01);
  CPPUNIT_TEST(testIncludeTax_F_0);
  CPPUNIT_TEST(testIncludeTax_F_10);
  CPPUNIT_TEST(testIncludeTax_F_10_P15);
  CPPUNIT_TEST(testIncludeTax_P15_F2);
  CPPUNIT_TEST_SUITE_END();

  std::unique_ptr<TaxRoundingInfoService> _taxRoundingInfoService;

public:

  void setup()
  {
    _taxRoundingInfoService.reset(TaxRoundingInfoServiceMock::create());
  }

  void testIncludeTax_P_0()
  {
    RoundingInfo roundingInfo(type::MoneyAmount(1, 100), type::TaxRoundingDir::Nearest, false);
    TaxAndRounding tar(PERCENTAGE, type::MoneyAmount(0, 1), roundingInfo, true);

    OptionalService oc;
    oc.taxInclInd() = true;
    oc.amount() = 55;
    oc.taxAmount() = 0;
    oc.taxAndRounding() = tar;
    oc.includeTax(tar, *(_taxRoundingInfoService.get()));

    CPPUNIT_ASSERT_EQUAL(oc.amount(), type::MoneyAmount(55, 1));
    CPPUNIT_ASSERT_EQUAL(oc.getTaxEquivalentAmount(), type::MoneyAmount(0, 1));
  }

  void testIncludeTax_P_15()
  {
    RoundingInfo roundingInfo(type::MoneyAmount(1, 100), type::TaxRoundingDir::Nearest, false);
    TaxAndRounding tar(PERCENTAGE, type::MoneyAmount(15, 100), roundingInfo, true);

    OptionalService oc;
    oc.taxInclInd() = true;
    oc.amount() = 55;
    oc.taxAmount() = type::MoneyAmount(33, 4);
    oc.taxAndRounding() = tar;
    oc.includeTax(tar, *(_taxRoundingInfoService.get()));

    CPPUNIT_ASSERT_EQUAL(oc.amount(), type::MoneyAmount(4783, 100));
    CPPUNIT_ASSERT_EQUAL(oc.getTaxEquivalentAmount(), type::MoneyAmount(717, 100));
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(oc.amount() + oc.taxAmount()), type::MoneyAmount(55, 1));
  }

  void testIncludeTax_P_15_P_20()
  {
    RoundingInfo roundingInfo(type::MoneyAmount(1, 100), type::TaxRoundingDir::Nearest, false);
    TaxAndRounding tar(PERCENTAGE, type::MoneyAmount(15, 100), roundingInfo, true);

    OptionalService oc;
    oc.taxInclInd() = true;
    oc.amount() = 55;
    oc.taxAmount() = 0;
    oc.includeTax(tar, *(_taxRoundingInfoService.get()));

    CPPUNIT_ASSERT_EQUAL(oc.amount(), type::MoneyAmount(4783, 100));
    CPPUNIT_ASSERT_EQUAL(oc.getTaxEquivalentAmount(), type::MoneyAmount(0, 1));

    TaxAndRounding tar2(PERCENTAGE, type::MoneyAmount(20, 100), roundingInfo, true);
    oc.includeTax(tar2, *(_taxRoundingInfoService.get()));

    CPPUNIT_ASSERT_EQUAL(oc.amount(), type::MoneyAmount(2037, 50));
    CPPUNIT_ASSERT_EQUAL(oc.getTaxEquivalentAmount(), type::MoneyAmount(0, 1));
  }

  void testIncludeTax_P_15_P_20_tax()
  {
    RoundingInfo roundingInfo(type::MoneyAmount(1, 100), type::TaxRoundingDir::Nearest, false);
    TaxAndRounding tar(PERCENTAGE, type::MoneyAmount(15, 100), roundingInfo, true);

    OptionalService oc;
    oc.taxInclInd() = true;
    oc.amount() = 55;
    oc.taxAmount() = type::MoneyAmount(33, 4);
    oc.taxAndRounding() = tar;
    oc.includeTax(tar, *(_taxRoundingInfoService.get()));

    CPPUNIT_ASSERT_EQUAL(oc.amount(), type::MoneyAmount(4783, 100));
    CPPUNIT_ASSERT_EQUAL(oc.getTaxEquivalentAmount(), type::MoneyAmount(717, 100));

    TaxAndRounding tar2(PERCENTAGE, type::MoneyAmount(20, 100), roundingInfo, true);
    oc.includeTax(tar2, *(_taxRoundingInfoService.get()));

    CPPUNIT_ASSERT_EQUAL(oc.amount(), type::MoneyAmount(2037, 50));
    CPPUNIT_ASSERT_EQUAL(oc.getTaxEquivalentAmount(), type::MoneyAmount(611, 100));
  }

  void testIncludeTax_P_15_P_20_Nearest_001_RoundUp_01()
  {
    RoundingInfo roundingInfo(type::MoneyAmount(1, 100), type::TaxRoundingDir::Nearest, false);
    TaxAndRounding tar(PERCENTAGE, type::MoneyAmount(0, 1), roundingInfo, true);

    OptionalService oc;
    oc.taxInclInd() = true;
    oc.amount() = 55;
    oc.taxAndRounding() = tar;

    TaxAndRounding tar1(PERCENTAGE, type::MoneyAmount(15, 100), roundingInfo, true);
    oc.includeTax(tar1, *(_taxRoundingInfoService.get()));

    CPPUNIT_ASSERT_EQUAL(oc.amount(), type::MoneyAmount(4783, 100));
    CPPUNIT_ASSERT_EQUAL(oc.getTaxEquivalentAmount(), type::MoneyAmount(0, 1));

    RoundingInfo roundingInfo2(type::MoneyAmount(1, 10), type::TaxRoundingDir::RoundUp, false);
    TaxAndRounding tar2(PERCENTAGE, type::MoneyAmount(20, 100), roundingInfo2, true);
    oc.includeTax(tar2, *(_taxRoundingInfoService.get()));

    CPPUNIT_ASSERT_EQUAL(oc.amount(), type::MoneyAmount(4069, 100));
    CPPUNIT_ASSERT_EQUAL(oc.getTaxEquivalentAmount(), type::MoneyAmount(0, 1));
  }

  void testIncludeTax_P_15_P_20_tax_Nearest_001_RoundUp_01()
  {
    RoundingInfo roundingInfo(type::MoneyAmount(1, 100), type::TaxRoundingDir::Nearest, false);
    TaxAndRounding tar(PERCENTAGE, type::MoneyAmount(15, 100), roundingInfo, true);

    OptionalService oc;
    oc.taxInclInd() = true;
    oc.amount() = 55;
    oc.taxAndRounding() = tar;

    oc.includeTax(tar, *(_taxRoundingInfoService.get()));

    CPPUNIT_ASSERT_EQUAL(oc.amount(), type::MoneyAmount(4783, 100));
    CPPUNIT_ASSERT_EQUAL(oc.getTaxEquivalentAmount(), type::MoneyAmount(717, 100));

    RoundingInfo roundingInfo2(type::MoneyAmount(1, 10), type::TaxRoundingDir::RoundUp, false);
    TaxAndRounding tar2(PERCENTAGE, type::MoneyAmount(20, 100), roundingInfo2, true);
    oc.includeTax(tar2, *(_taxRoundingInfoService.get()));

    CPPUNIT_ASSERT_EQUAL(oc.amount(), type::MoneyAmount(4069, 100));
    CPPUNIT_ASSERT_EQUAL(oc.getTaxEquivalentAmount(), type::MoneyAmount(611, 100));
  }

  void testIncludeTax_F_0()
  {
    RoundingInfo roundingInfo(type::MoneyAmount(1, 100), type::TaxRoundingDir::Nearest, false);
    TaxAndRounding tar(FLAT, type::MoneyAmount(0, 1), roundingInfo, true);

    OptionalService oc;
    oc.taxInclInd() = true;
    oc.amount() = 55;
    oc.taxAndRounding() = tar;
    oc.includeTax(tar, *(_taxRoundingInfoService.get()));

    CPPUNIT_ASSERT_EQUAL(oc.amount(), type::MoneyAmount(55, 1));
    CPPUNIT_ASSERT_EQUAL(oc.getTaxEquivalentAmount(), type::MoneyAmount(0, 1));
  }

  void testIncludeTax_F_10()
  {
    RoundingInfo roundingInfo(type::MoneyAmount(1, 100), type::TaxRoundingDir::Nearest, false);
    TaxAndRounding tar(FLAT, type::MoneyAmount(10, 1), roundingInfo, true);

    OptionalService oc;
    oc.taxInclInd() = true;
    oc.amount() = 55;
    oc.taxAndRounding() = tar;
    oc.includeTax(tar, *(_taxRoundingInfoService.get()));

    CPPUNIT_ASSERT_EQUAL(oc.amount(), type::MoneyAmount(45, 1));
    CPPUNIT_ASSERT_EQUAL(oc.getTaxEquivalentAmount(), type::MoneyAmount(10, 1));
  }

  void testIncludeTax_F_10_P15()
  {
    RoundingInfo roundingInfo(type::MoneyAmount(1, 100), type::TaxRoundingDir::Nearest, false);
    TaxAndRounding tar(FLAT, type::MoneyAmount(10, 1), roundingInfo, true);

    OptionalService oc;
    oc.taxInclInd() = true;
    oc.amount() = 55;
    oc.taxAndRounding() = tar;
    oc.includeTax(tar, *(_taxRoundingInfoService.get()));

    CPPUNIT_ASSERT_EQUAL(oc.amount(), type::MoneyAmount(45, 1));
    CPPUNIT_ASSERT_EQUAL(oc.getTaxEquivalentAmount(), type::MoneyAmount(10, 1));

    TaxAndRounding tar2(PERCENTAGE, type::MoneyAmount(15, 100), roundingInfo, true);
    oc.includeTax(tar2, *(_taxRoundingInfoService.get()));

    CPPUNIT_ASSERT_EQUAL(oc.amount(), type::MoneyAmount(3913, 100));
    CPPUNIT_ASSERT_EQUAL(oc.getTaxEquivalentAmount(), type::MoneyAmount(10, 1));
  }

  void testIncludeTax_P15_F2()
  {
    RoundingInfo roundingInfo(type::MoneyAmount(1, 100), type::TaxRoundingDir::Nearest, false);
    TaxAndRounding tar_1(FLAT, type::MoneyAmount(2, 1), roundingInfo, true);

    OptionalService oc_1, oc_2;
    oc_1.taxInclInd() = true;
    oc_1.amount() = 55;
    oc_1.taxAndRounding() = tar_1;

    TaxAndRounding tar_2(PERCENTAGE, type::MoneyAmount(15, 100), roundingInfo, true);

    oc_2.taxInclInd() = true;
    oc_2.amount() = 55;
    oc_2.taxAndRounding() = tar_2;

    oc_1.includeTax(tar_2, *(_taxRoundingInfoService.get()));
    oc_2.includeTax(tar_2, *(_taxRoundingInfoService.get()));

    CPPUNIT_ASSERT_EQUAL(oc_1.amount(), type::MoneyAmount(4783, 100));
    CPPUNIT_ASSERT_EQUAL(oc_1.getTaxEquivalentAmount(), type::MoneyAmount(2, 1));

    CPPUNIT_ASSERT_EQUAL(oc_2.amount(), type::MoneyAmount(4783, 100));
    CPPUNIT_ASSERT_EQUAL(oc_2.getTaxEquivalentAmount(), type::MoneyAmount(717, 100));

    oc_1.includeTax(tar_1, *(_taxRoundingInfoService.get()));
    oc_2.includeTax(tar_1, *(_taxRoundingInfoService.get()));

    CPPUNIT_ASSERT_EQUAL(oc_1.amount(), type::MoneyAmount(4609, 100));
    CPPUNIT_ASSERT_EQUAL(oc_1.getTaxEquivalentAmount(), type::MoneyAmount(2, 1));

    CPPUNIT_ASSERT_EQUAL(oc_2.amount(), type::MoneyAmount(4609, 100));
    CPPUNIT_ASSERT_EQUAL(oc_2.getTaxEquivalentAmount(), type::MoneyAmount(691, 100));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxRoundingOCApplicatorTest);
} // namespace tax
