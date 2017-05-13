// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include "Rules/ReturnToOriginApplicator.h"
#include "DataModel/Common/Types.h"
#include "test/GeoPathMock.h"
#include "test/PaymentDetailMock.h"

#include <memory>

namespace tax
{

class ReturnToOriginApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ReturnToOriginApplicatorTest);

  CPPUNIT_TEST(testReturnToOrigin_DomesticSameCities);
  CPPUNIT_TEST(testReturnToOrigin_DomesticDifferentCities);
  CPPUNIT_TEST(testReturnToOrigin_InternationalSameNations);
  CPPUNIT_TEST(testReturnToOrigin_InternationalDifferentNations);

  CPPUNIT_TEST(testNotReturnToOrigin_DomesticSameCities);
  CPPUNIT_TEST(testNotReturnToOrigin_DomesticDifferentCities);
  CPPUNIT_TEST(testNotReturnToOrigin_InternationalSameNations);
  CPPUNIT_TEST(testNotReturnToOrigin_InternationalDifferentNations);

  CPPUNIT_TEST_SUITE_END();

public:
  void createApplicator(type::RtnToOrig const& rtnToOrig)
  {
    _rtnToOrig.reset(new tax::type::RtnToOrig(rtnToOrig));
    _applicator.reset(new tax::ReturnToOriginApplicator(0, *_geoPathMock, *_rtnToOrig));
  }

  void setUp()
  {
    _geoPathMock.reset(new tax::GeoPathMock());
    _paymentDetailMock.reset(new tax::PaymentDetailMock());
  }

  void tearDown()
  {
    _rtnToOrig.reset();
    _applicator.reset();
  }

  void testReturnToOrigin_DomesticSameCities()
  {
    _geoPathMock->setIsJourneyDomestic(true);
    _geoPathMock->setOriginCity("KRK");
    _geoPathMock->setDestinationCity("KRK");
    createApplicator(tax::type::RtnToOrig::ReturnToOrigin);
    CPPUNIT_ASSERT(_applicator->apply(*_paymentDetailMock));
  }

  void testReturnToOrigin_DomesticDifferentCities()
  {
    _geoPathMock->setIsJourneyDomestic(true);
    _geoPathMock->setOriginCity("KRK");
    _geoPathMock->setDestinationCity("DFW");
    createApplicator(type::RtnToOrig::ReturnToOrigin);
    CPPUNIT_ASSERT(!_applicator->apply(*_paymentDetailMock));
  }

  void testReturnToOrigin_InternationalSameNations()
  {
    _geoPathMock->setIsJourneyDomestic(false);
    _geoPathMock->setOriginNation("PL");
    _geoPathMock->setDestinationNation("PL");
    createApplicator(type::RtnToOrig::ReturnToOrigin);
    CPPUNIT_ASSERT(_applicator->apply(*_paymentDetailMock));
  }

  void testReturnToOrigin_InternationalDifferentNations()
  {
    _geoPathMock->setIsJourneyDomestic(false);
    _geoPathMock->setOriginNation("PL");
    _geoPathMock->setDestinationNation("US");
    createApplicator(type::RtnToOrig::ReturnToOrigin);
    CPPUNIT_ASSERT(!_applicator->apply(*_paymentDetailMock));
  }

  void testNotReturnToOrigin_DomesticSameCities()
  {
    _geoPathMock->setIsJourneyDomestic(true);
    _geoPathMock->setOriginCity("KRK");
    _geoPathMock->setDestinationCity("KRK");
    createApplicator(type::RtnToOrig::NotReturnToOrigin);
    CPPUNIT_ASSERT(!_applicator->apply(*_paymentDetailMock));
  }

  void testNotReturnToOrigin_DomesticDifferentCities()
  {
    _geoPathMock->setIsJourneyDomestic(true);
    _geoPathMock->setOriginCity("KRK");
    _geoPathMock->setDestinationCity("DFW");
    createApplicator(type::RtnToOrig::NotReturnToOrigin);
    CPPUNIT_ASSERT(_applicator->apply(*_paymentDetailMock));
  }

  void testNotReturnToOrigin_InternationalSameNations()
  {
    _geoPathMock->setIsJourneyDomestic(false);
    _geoPathMock->setOriginNation("PL");
    _geoPathMock->setDestinationNation("PL");
    createApplicator(type::RtnToOrig::NotReturnToOrigin);
    CPPUNIT_ASSERT(!_applicator->apply(*_paymentDetailMock));
  }

  void testNotReturnToOrigin_InternationalDifferentNations()
  {
    _geoPathMock->setIsJourneyDomestic(false);
    _geoPathMock->setOriginNation("PL");
    _geoPathMock->setDestinationNation("US");
    createApplicator(type::RtnToOrig::NotReturnToOrigin);
    CPPUNIT_ASSERT(_applicator->apply(*_paymentDetailMock));
  }

private:
  std::unique_ptr<tax::GeoPathMock> _geoPathMock;
  std::unique_ptr<tax::PaymentDetailMock> _paymentDetailMock;
  std::unique_ptr<tax::type::RtnToOrig> _rtnToOrig;
  std::unique_ptr<tax::ReturnToOriginApplicator> _applicator;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ReturnToOriginApplicatorTest);

} // namespace tax
