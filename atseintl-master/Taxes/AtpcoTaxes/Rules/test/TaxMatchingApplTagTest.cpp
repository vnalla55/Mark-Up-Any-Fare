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

#include "Rules/TaxMatchingApplTagApplicator.h"
#include "Rules/TaxMatchingApplTagRule.h"

#include "test/include/CppUnitHelperMacros.h"

#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/GeoPathMapping.h"
#include "DomainDataObjects/Itin.h"
#include "Rules/PaymentDetail.h"
#include "Rules/PaymentRuleData.h"

#include "test/MileageServiceMock.h"

#include <memory>

namespace tax
{

class TaxMatchingApplTagTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxMatchingApplTagTest);

  CPPUNIT_TEST(testPassConditionsOk);
  CPPUNIT_TEST(testFailDifferentCountries);
  CPPUNIT_TEST(testFailNotFareBreak);
  CPPUNIT_TEST(testFailNotDomesticFare);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _taxMatchingApplTag = new type::TaxMatchingApplTag();
    *_taxMatchingApplTag = "01";
    _parent = new TaxMatchingApplTagRule(*_taxMatchingApplTag, false);
    _seqNo = new type::SeqNo();

    _geoPath = new GeoPath();
    _geoPath->geos().reserve(8);
    for (size_t i = 0; i <= 7; ++i)
    {
      _geoPath->geos().push_back(Geo());
      _geoPath->geos()[i].id() = i;
      if (i > 0)
      {
        _geoPath->geos()[i - 1].setNext(&_geoPath->geos()[i]);
        _geoPath->geos()[i].setPrev(&_geoPath->geos()[i - 1]);
      }
      _geoPath->geos()[i].loc().tag() =
        (i % 2) ? type::TaxPointTag::Arrival : type::TaxPointTag::Departure;
      _geoPath->geos()[i].loc().nation() = (i < 5) ? "PL" : "US";
    }
    _geoPath->geos()[0].loc().code() = "KRK";
    _geoPath->geos()[1].loc().code() = "KTW";
    _geoPath->geos()[2].loc().code() = "KTW";
    _geoPath->geos()[3].loc().code() = "WAW";
    _geoPath->geos()[4].loc().code() = "WAW";
    _geoPath->geos()[5].loc().code() = "NYC";
    _geoPath->geos()[6].loc().code() = "NYC";
    _geoPath->geos()[7].loc().code() = "DFW";
    for (size_t i = 0; i <= 7; ++i)
    {
      _geoPath->geos()[i].loc().cityCode().convertFrom( _geoPath->geos()[i].loc().code() );
    }

    _geoPathMapping = new GeoPathMapping();
    _geoPathMapping->mappings().push_back(Mapping());
    for (size_t i = 0; i <= 3; ++i)
    {
      _geoPathMapping->mappings()[0].maps().push_back(Map());
      _geoPathMapping->mappings()[0].maps().back().index() = i;
    }
    _geoPathMapping->mappings().push_back(Mapping());
    for (size_t i = 4; i <= 7; ++i)
    {
      _geoPathMapping->mappings()[1].maps().push_back(Map());
      _geoPathMapping->mappings()[1].maps().back().index() = i;
    }

    _ticketedPointTag =
      new type::TicketedPointTag(type::TicketedPointTag::MatchTicketedAndUnticketedPoints);
    _taxName = new TaxName();
    _paymentDetail = new PaymentDetail(
        PaymentRuleData(*_seqNo, *_ticketedPointTag, TaxableUnitTagSet::none(), 0,
            type::CurrencyCode(UninitializedCode),
            type::TaxAppliesToTagInd::Blank),
        _geoPath->geos()[0],
        _geoPath->geos()[3],
        *_taxName);
    _paymentDetail->getMutableTaxPointsProperties().resize(8);
    _paymentDetail->getMutableTaxPointsProperties()[0].isFirst = true;
    _paymentDetail->getMutableTaxPointsProperties()[0].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[3].isTimeStopover = true;
    _paymentDetail->getMutableTaxPointsProperties()[3].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[4].isTimeStopover = true;
    _paymentDetail->getMutableTaxPointsProperties()[4].isFareBreak = true;
    _paymentDetail->getMutableTaxPointsProperties()[7].isLast = true;
    _paymentDetail->getMutableTaxPointsProperties()[7].isFareBreak = true;

    _itin = new Itin();
    _itin->geoPath() = _geoPath;
    _itin->geoPathMapping() = _geoPathMapping;

    _mileageServiceMock.reset(new MileageServiceMock);
  }

  void tearDown()
  {
    delete _taxMatchingApplTag;
    delete _parent;
    delete _seqNo;
    delete _geoPath;
    delete _geoPathMapping;
    delete _ticketedPointTag;
    delete _paymentDetail;
    delete _taxName;
  }

  void testPassConditionsOk()
  {
    TaxMatchingApplTagApplicator applicator(_parent, *_itin, *_mileageServiceMock);
    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetail));
  }

  void testFailDifferentCountries()
  {
    _geoPath->geos()[3].loc().nation() = "BR";

    TaxMatchingApplTagApplicator applicator(_parent, *_itin, *_mileageServiceMock);
    CPPUNIT_ASSERT_EQUAL(false, applicator.apply(*_paymentDetail));
  }

  void testFailNotFareBreak()
  {
    _paymentDetail->getMutableTaxPointsProperties()[3].isFareBreak = false;

    TaxMatchingApplTagApplicator applicator(_parent, *_itin, *_mileageServiceMock);
    CPPUNIT_ASSERT_EQUAL(false, applicator.apply(*_paymentDetail));
  }

  void testFailNotDomesticFare()
  {
    _geoPath->geos()[1].loc().code() = "OHR";
    _geoPath->geos()[1].loc().nation() = "US";
    _geoPath->geos()[2].loc().code() = "OHR";
    _geoPath->geos()[2].loc().nation() = "US";

    TaxMatchingApplTagApplicator applicator(_parent, *_itin, *_mileageServiceMock);
    CPPUNIT_ASSERT_EQUAL(false, applicator.apply(*_paymentDetail));
  }

private:
  TaxMatchingApplTagRule* _parent;
  type::TaxMatchingApplTag* _taxMatchingApplTag;
  type::SeqNo* _seqNo;
  Itin* _itin;
  GeoPath* _geoPath;
  GeoPathMapping* _geoPathMapping;
  PaymentDetail* _paymentDetail;
  type::TicketedPointTag* _ticketedPointTag;
  TaxName* _taxName;
  std::unique_ptr<MileageServiceMock> _mileageServiceMock;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxMatchingApplTagTest);
} // namespace tax
