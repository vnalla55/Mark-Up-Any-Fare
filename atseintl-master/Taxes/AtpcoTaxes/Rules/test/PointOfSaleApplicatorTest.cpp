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
#include <memory>
#include <set>

#include "test/include/CppUnitHelperMacros.h"
#include "test/MakePaymentDetail.h"

#include "Rules/PointOfSaleApplicator.h"
#include "Rules/PointOfSaleRule.h"
#include "ServiceInterfaces/LocService.h"

namespace
{

  class LocServiceMock : public tax::LocService
  {
  public:
    tax::type::Nation getNation(const tax::type::AirportCode& /* loc */) const
    {
      return tax::type::Nation(tax::UninitializedCode);
    }
    tax::type::Nation getNationByName(const tax::type::NationName& /* nationName */) const
    {
      return tax::type::Nation(tax::UninitializedCode);
    }
    tax::type::NationName getNationName(const tax::type::Nation& /* nationCode */) const
    {
      return tax::type::NationName();
    }
    tax::type::CityCode getCityCode(const tax::type::AirportCode& /* loc */) const
    {
      return tax::type::CityCode(tax::UninitializedCode);
    };
    tax::type::AlaskaZone getAlaskaZone(const tax::type::AirportCode& /* loc */) const
    {
      return tax::type::AlaskaZone::A;
    };
    tax::type::StateProvinceCode getState(const tax::type::AirportCode& /* loc */) const
    {
      return "";
    };
    tax::type::CurrencyCode getCurrency(const tax::type::AirportCode& /* loc */) const
    {
      return tax::type::CurrencyCode(tax::UninitializedCode);
    };

    bool matchPassengerLocation(const tax::type::LocCode&,
                                const tax::LocZone&,
                                const tax::type::Vendor&) const
    {
      return false;
    }
  };

  class LocServiceTrueMock : public LocServiceMock
  {
  public:
    bool isInLoc(const tax::type::AirportOrCityCode& /* airportCode */,
                 const tax::LocZone& /* jrnyLoc1LocZone */,
                 const tax::type::Vendor& /* vendor */) const
    {
      return true;
    }
  };

  class LocServiceFalseMock : public LocServiceMock
  {
    bool isInLoc(const tax::type::AirportOrCityCode& /* airportCode */,
                 const tax::LocZone& /* jrnyLoc1LocZone */,
                 const tax::type::Vendor& /* vendor */) const
    {
      return false;
    }
  };

  class PointOfSaleApplicatorFactory
  {
    std::unique_ptr<tax::LocService> _locService;
    std::unique_ptr<tax::PointOfSaleRule> _rule;

  public:
    PointOfSaleApplicatorFactory(tax::LocService* locService,
                                 const tax::LocZone locZone,
                                 const tax::type::Vendor vendor)
    {
      _locService.reset(locService);
      _rule.reset(new tax::PointOfSaleRule(locZone, vendor));
    }

    tax::PointOfSaleApplicator* createApplicator(const tax::type::AirportCode& salePoint)
    {
      return new tax::PointOfSaleApplicator(*_rule, salePoint, (*_locService.get()));
    }
  };
}

namespace tax
{
class PointOfSaleApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PointOfSaleApplicatorTest);
  CPPUNIT_TEST(testApply_SalePoint_Empty);
  CPPUNIT_TEST(testApply_inLoc);
  CPPUNIT_TEST(testApply_NotInLoc);
  CPPUNIT_TEST_SUITE_END();

public:

  void testApply_SalePoint_Empty()
  {
    tax::type::AirportCode pointOfSale = "KRK";
    PaymentDetail payDetail = MakePaymentDetail();
    PointOfSaleApplicatorFactory factory(new LocServiceTrueMock(), tax::LocZone(), "LH");
    std::unique_ptr<PointOfSaleApplicator> applicator(factory.createApplicator(pointOfSale));
    CPPUNIT_ASSERT(applicator);
    CPPUNIT_ASSERT(applicator.get()->apply(payDetail));
  }

  void testApply_inLoc()
  {
    tax::type::AirportCode pointOfSale = "KRK";
    PaymentDetail payDetail = MakePaymentDetail();
    PointOfSaleApplicatorFactory factory(new LocServiceTrueMock(), tax::LocZone(), "LH");
    std::unique_ptr<PointOfSaleApplicator> applicator(factory.createApplicator(pointOfSale));
    CPPUNIT_ASSERT(applicator);
    CPPUNIT_ASSERT(applicator.get()->apply(payDetail));
  }

  void testApply_NotInLoc()
  {
    tax::type::AirportCode pointOfSale = "KRK";
    PaymentDetail payDetail = MakePaymentDetail();
    PointOfSaleApplicatorFactory factory(new LocServiceFalseMock(), tax::LocZone(), "LH");
    std::unique_ptr<PointOfSaleApplicator> applicator(factory.createApplicator(pointOfSale));
    CPPUNIT_ASSERT(applicator);
    CPPUNIT_ASSERT(!applicator.get()->apply(payDetail));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(PointOfSaleApplicatorTest);
}
