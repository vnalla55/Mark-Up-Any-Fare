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

#include <memory>
#include <stdexcept>

#include "test/include/CppUnitHelperMacros.h"
#include "Rules/AlternateRefAkHiFactorsApplicator.h"
#include "Rules/AlternateRefAkHiFactorsRule.h"
#include "Rules/GeoUtils.h"
#include "test/PaymentDetailMock.h"
#include "TestServer/Facades/LocServiceServer.h"
#include "TestServer/Facades/AKHIFactorServiceServer.h"

using namespace std;

namespace tax
{

namespace
{
  const type::AirportCode USA("USA");
  const type::AirportCode PL("PLL");
  const type::AirportCode UA("UAA");
}

class AlternateRefAkHiFactorsApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AlternateRefAkHiFactorsApplicatorTest);

  CPPUNIT_TEST(testNotAKHI);

  CPPUNIT_TEST(testHIBeginEnd);
  CPPUNIT_TEST(testHIBegin);
  CPPUNIT_TEST(testHIEnd);

  CPPUNIT_TEST(testAKBeginEnd);
  CPPUNIT_TEST(testAKBeginZoneA);
  CPPUNIT_TEST(testAKBeginZoneB);
  CPPUNIT_TEST(testAKEndZoneC);
  CPPUNIT_TEST(testAKEndZoneD);

  CPPUNIT_TEST(testMatchButNoFactor);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _egTaxAmt = type::MoneyAmount(20);

    _paymentDetail = new PaymentDetailMock();
    _paymentDetail->taxAmt() = _egTaxAmt;

    _locService.reset(new LocServiceServer);
    _locService->nations().push_back(new Nation);
    _locService->nations()[0].locCode() = PL;
    _locService->nations()[0].state() = "XX";
    _locService->nations()[0].alaskaZone() = type::AlaskaZone::Blank;

    _locService->nations().push_back(new Nation);
    _locService->nations()[1].locCode() = PL;
    _locService->nations()[1].state() = "XX";
    _locService->nations()[1].alaskaZone() = type::AlaskaZone::Blank;

    _AKHIFactorService.reset(new AKHIFactorServiceServer);
    _AKHIFactorService->aKHIFactor().push_back(new AKHIFactor);
    _AKHIFactorService->aKHIFactor()[0].locCode = PL;
    _AKHIFactorService->aKHIFactor()[0].hawaiiPercent = type::Percent(11);
    _AKHIFactorService->aKHIFactor()[0].zoneAPercent = type::Percent(12);
    _AKHIFactorService->aKHIFactor()[0].zoneBPercent = type::Percent(13);
    _AKHIFactorService->aKHIFactor()[0].zoneCPercent = type::Percent(14);
    _AKHIFactorService->aKHIFactor()[0].zoneDPercent = type::Percent(15);

    _taxPointBegin.reset(new Geo);
    _taxPointEnd.reset(new Geo);
  }

  void tearDown()
  {
    _egTaxAmt = type::MoneyAmount();

    _taxPointBegin.reset();
    _taxPointEnd.reset();
    _AKHIFactorService.reset();
    _locService.reset();
    _rule.reset();
    delete _paymentDetail;
  }

  void testNotAKHI()
  {
    prepareRule();
    AlternateRefAkHiFactorsApplicator applicator(*_rule, *_AKHIFactorService, *_locService);

    setLocBegin(PL);
    setLocEnd(UA);

    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(_egTaxAmt, _paymentDetail->taxAmt());
  }

  void testHIBeginEnd()
  {
    prepareRule();
    AlternateRefAkHiFactorsApplicator applicator(*_rule, *_AKHIFactorService, *_locService);

    setLocBegin(USA);
    setLocEnd(USA);

    _locService->nations()[0].locCode() = USA;
    _locService->nations()[0].state() = GeoUtils::HAWAII;
    _locService->nations()[1].locCode() = USA;
    _locService->nations()[1].state() = GeoUtils::HAWAII;

    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(_egTaxAmt, _paymentDetail->taxAmt());
  }

  void testHIBegin()
  {
    prepareRule();
    AlternateRefAkHiFactorsApplicator applicator(*_rule, *_AKHIFactorService, *_locService);

    setLocBegin(USA);
    setLocEnd(PL);

    _locService->nations()[0].locCode() = USA;
    _locService->nations()[0].state() = GeoUtils::HAWAII;
    _locService->nations()[1].locCode() = "AAA";
    _locService->nations()[1].state() = "BBB";

    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(type::Percent(11), _paymentDetail->taxAmt());
  }

  void testHIEnd()
  {
    prepareRule();
    AlternateRefAkHiFactorsApplicator applicator(*_rule, *_AKHIFactorService, *_locService);

    setLocBegin(PL);
    setLocEnd(USA);

    _locService->nations()[0].locCode() = USA;
    _locService->nations()[0].state() = GeoUtils::HAWAII;
    _locService->nations()[1].locCode() = "AAA";
    _locService->nations()[1].state() = "BBB";

    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(type::Percent(11), _paymentDetail->taxAmt());
  }

  void testAKBeginEnd()
  {
    prepareRule();
    AlternateRefAkHiFactorsApplicator applicator(*_rule, *_AKHIFactorService, *_locService);

    setLocBegin(USA);
    setLocEnd(USA);

    _locService->nations()[0].locCode() = USA;
    _locService->nations()[0].state() = GeoUtils::ALASKA;
    _locService->nations()[1].locCode() = USA;
    _locService->nations()[1].state() = GeoUtils::ALASKA;

    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(_egTaxAmt, _paymentDetail->taxAmt());
  }

  void testAKBeginZoneA()
  {
    prepareRule();
    AlternateRefAkHiFactorsApplicator applicator(*_rule, *_AKHIFactorService, *_locService);

    setLocBegin(USA);
    setLocEnd(PL);

    _locService->nations()[0].locCode() = USA;
    _locService->nations()[0].state() = GeoUtils::ALASKA;
    _locService->nations()[0].alaskaZone() = type::AlaskaZone::A;

    _locService->nations()[1].locCode() = PL;

    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(type::Percent(12), _paymentDetail->taxAmt());
  }

  void testAKBeginZoneB()
  {
    prepareRule();
    AlternateRefAkHiFactorsApplicator applicator(*_rule, *_AKHIFactorService, *_locService);

    setLocBegin(USA);
    setLocEnd(PL);

    _locService->nations()[0].locCode() = USA;
    _locService->nations()[0].state() = GeoUtils::ALASKA;
    _locService->nations()[0].alaskaZone() = type::AlaskaZone::B;

    _locService->nations()[1].locCode() = PL;

    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(type::Percent(13), _paymentDetail->taxAmt());
  }

  void testAKEndZoneC()
  {
    prepareRule();
    AlternateRefAkHiFactorsApplicator applicator(*_rule, *_AKHIFactorService, *_locService);

    setLocBegin(PL);
    setLocEnd(USA);

    _locService->nations()[0].locCode() = USA;
    _locService->nations()[0].state() = GeoUtils::ALASKA;
    _locService->nations()[0].alaskaZone() = type::AlaskaZone::C;

    _locService->nations()[1].locCode() = PL;

    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(type::Percent(14), _paymentDetail->taxAmt());
  }

  void testAKEndZoneD()
  {
    prepareRule();
    AlternateRefAkHiFactorsApplicator applicator(*_rule, *_AKHIFactorService, *_locService);

    setLocBegin(PL);
    setLocEnd(USA);

    _locService->nations()[0].locCode() = USA;
    _locService->nations()[0].state() = GeoUtils::ALASKA;
    _locService->nations()[0].alaskaZone() = type::AlaskaZone::D;

    _locService->nations()[1].locCode() = PL;

    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(type::Percent(15), _paymentDetail->taxAmt());
  }

  void testMatchButNoFactor()
  {
    prepareRule();
    AlternateRefAkHiFactorsApplicator applicator(*_rule, *_AKHIFactorService, *_locService);

    setLocBegin(PL);
    setLocEnd(USA);

    _locService->nations()[0].locCode() = USA;
    _locService->nations()[0].state() = GeoUtils::ALASKA;

    _locService->nations()[1].locCode() = "PL2";
    _locService->nations()[1].alaskaZone() = type::AlaskaZone::D;

    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(_egTaxAmt, _paymentDetail->taxAmt());
  }

private:
  type::MoneyAmount _egTaxAmt;
  PaymentDetailMock* _paymentDetail;
  std::unique_ptr<AlternateRefAkHiFactorsRule> _rule;
  std::unique_ptr<LocServiceServer> _locService;
  std::unique_ptr<AKHIFactorServiceServer> _AKHIFactorService;
  std::unique_ptr<Geo> _taxPointBegin;
  std::unique_ptr<Geo> _taxPointEnd;

  void prepareRule()
  {
    _rule.reset(new AlternateRefAkHiFactorsRule());
  }

  void setLocBegin(const type::AirportCode begin)
  {
    Loc locBegin;
    locBegin.code() = begin;

    _taxPointBegin->loc() = locBegin;
    _paymentDetail->setTaxPointBegin(*_taxPointBegin);
  }

  void setLocEnd(const type::AirportCode end)
  {
    Loc locEnd;
    locEnd.code() = end;

    _taxPointEnd->loc() = locEnd;
    _paymentDetail->setTaxPointEnd(*_taxPointEnd);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(AlternateRefAkHiFactorsApplicatorTest);
} // namespace tax
