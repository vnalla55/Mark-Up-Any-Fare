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

#include "Common/LocZone.h"
#include "Common/TaxName.h"
#include "DataModel/Common/Types.h"
#include "DomainDataObjects/FlightUsage.h"
#include "DomainDataObjects/Geo.h"
#include "DomainDataObjects/GeoPath.h"
#include "Rules/BusinessRule.h"
#include "Rules/PaymentDetail.h"
#include "Rules/PaymentRuleData.h"
#include "Rules/TaxPointLoc2Applicator.h"
#include "Rules/TaxPointLoc2Rule.h"
#include "test/LocServiceMock.h"
#include "test/MileageServiceMock.h"

#include <memory>

namespace tax
{
class TaxPointLoc2ApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxPointLoc2ApplicatorTest);

  CPPUNIT_TEST(testApply_match);
  CPPUNIT_TEST(testApply_noMatch);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _locZone.reset(new LocZone());
    _vendor.reset(new type::Vendor());

    _locServiceMock.reset(new LocServiceMock());
    _seqNo.reset(new type::SeqNo());
    _ticketedPointTag.reset(
        new type::TicketedPointTag(type::TicketedPointTag::MatchTicketedAndUnticketedPoints));
    _geoPath.reset(new GeoPath());
    _geoPath->id() = 0;
    _taxName = new TaxName();

    for (int i = 0; i < 8; ++i)
    {
      _geoPath->geos().push_back(Geo());
      _geoPath->geos().at(i).id() = i;
      _geoPath->geos().at(i).loc().tag() =
          (i % 2) ? type::TaxPointTag::Arrival : type::TaxPointTag::Departure;
    }
    _geoPath->geos().at(0).loc().nation() = "PL";
    _geoPath->geos().at(1).loc().nation() = "US";
    _geoPath->geos().at(2).loc().nation() = "US";
    _geoPath->geos().at(3).loc().nation() = "MX";
    _geoPath->geos().at(4).loc().nation() = "MX";
    _geoPath->geos().at(5).loc().nation() = "US";
    _geoPath->geos().at(6).loc().nation() = "US";
    _geoPath->geos().at(7).loc().nation() = "PL";
  }

  void tearDown() { delete _taxName; }

  TaxPointLoc2Applicator getApplicator(const TaxPointLoc2Rule& rule)
  {
    TaxPointLoc2Applicator applicator(rule, *_locServiceMock);

    return applicator;
  }

  void setUpPaymentDetail(type::Index beginId, type::Index endId)
  {
    _paymentDetail.reset(new PaymentDetail(
        PaymentRuleData(*_seqNo, *_ticketedPointTag, TaxableUnitTagSet::none(), 0,
            type::CurrencyCode(UninitializedCode),
            type::TaxAppliesToTagInd::Blank),
        _geoPath->geos().at(beginId),
        _geoPath->geos().at(endId),
        *_taxName));
  }

  void addOc(type::OptionalServiceTag ocType, type::Index beginId, type::Index endId)
  {
    _paymentDetail->optionalServiceItems().push_back(new OptionalService());
    _paymentDetail->optionalServiceItems().back().type() = ocType;
    _paymentDetail->optionalServiceItems().back().setTaxPointBegin(_geoPath->geos().at(beginId));
    _paymentDetail->optionalServiceItems().back().setTaxPointLoc2(_geoPath->geos().at(endId));
    _paymentDetail->optionalServiceItems().back().setTaxPointEnd(_geoPath->geos().at(endId));
  }

  void addYqYr(type::Index begin,
               type::Index end,
               type::TaxCode code,
               type::YqYrType type,
               type::MoneyAmount amount,
               bool taxIncluded)
  {
    TaxableYqYrs& taxableYqYrs = _paymentDetail->getMutableYqYrDetails();
    taxableYqYrs._subject.push_back(TaxableYqYr(code, type, taxIncluded, amount));
    std::vector<type::Index> ids = taxableYqYrs._ids;
    ids.push_back(ids.size());
    type::Index i = ids.back();
    taxableYqYrs.init(taxableYqYrs._subject, ids);
    taxableYqYrs._ranges.push_back(std::make_pair(begin, end));
    taxableYqYrs._data[i]._taxPointLoc2 = &_geoPath->geos()[end];
    taxableYqYrs._data[i]._taxPointEnd = &_geoPath->geos()[end];
  }

  void setUpRule()
  {
    _taxPointLoc2Rule.reset(new TaxPointLoc2Rule(*_locZone, *_vendor));
  }

  void testApply_match()
  {
    _locServiceMock->clear().add(true, 3);
    _locServiceMock->add(false);
    _locServiceMock->add(true, 2);
    setUpPaymentDetail(0, 5);
    addOc(type::OptionalServiceTag::FlightRelated, 0, 5);
    addOc(type::OptionalServiceTag::BaggageCharge, 0, 5);
    addYqYr(0, 1, "YQ", 'F', 10, false);
    addYqYr(0, 3, "YQ", 'F', 10, false);
    addYqYr(0, 5, "YQ", 'F', 10, false);

    _taxPointLoc2Rule.reset(new TaxPointLoc2Rule(*_locZone, *_vendor));
    TaxPointLoc2Applicator applicator = getApplicator(*_taxPointLoc2Rule);

    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(type::Index(6), _locServiceMock->counter());
    CPPUNIT_ASSERT(_locServiceMock->empty());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->getItineraryDetail().isFailedRule());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->optionalServiceItems()[0].isFailed());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->optionalServiceItems()[1].isFailed());
    CPPUNIT_ASSERT_EQUAL(static_cast<const BusinessRule*>(0),
        _paymentDetail->getMutableYqYrDetails().getFailedRule(0));
    CPPUNIT_ASSERT_EQUAL(static_cast<const BusinessRule*>(_taxPointLoc2Rule.get()),
        _paymentDetail->getMutableYqYrDetails().getFailedRule(1));
    CPPUNIT_ASSERT_EQUAL(static_cast<const BusinessRule*>(0),
        _paymentDetail->getMutableYqYrDetails().getFailedRule(2));
  }

  void testApply_noMatch()
  {
    _locServiceMock->clear().add(false, 6);
    setUpPaymentDetail(0, 5);
    addOc(type::OptionalServiceTag::FlightRelated, 0, 5);
    addOc(type::OptionalServiceTag::BaggageCharge, 0, 5);
    addYqYr(0, 1, "YQ", 'F', 10, false);
    addYqYr(0, 3, "YQ", 'F', 10, false);
    addYqYr(0, 5, "YQ", 'F', 10, false);

    _taxPointLoc2Rule.reset(new TaxPointLoc2Rule(*_locZone, *_vendor));
    TaxPointLoc2Applicator applicator = getApplicator(*_taxPointLoc2Rule);

    CPPUNIT_ASSERT_EQUAL(false, applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(type::Index(6), _locServiceMock->counter());
    CPPUNIT_ASSERT(_locServiceMock->empty());
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->getItineraryDetail().isFailedRule());
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->areAllOptionalServicesFailed());
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->getMutableYqYrDetails().areAllFailed());
  }

private:
  std::unique_ptr<TaxPointLoc2Rule> _taxPointLoc2Rule;
  TaxName* _taxName;

  std::unique_ptr<LocZone> _locZone;
  std::unique_ptr<type::Vendor> _vendor;
  std::unique_ptr<LocServiceMock> _locServiceMock;
  std::unique_ptr<type::SeqNo> _seqNo;
  std::unique_ptr<type::TicketedPointTag> _ticketedPointTag;
  std::unique_ptr<PaymentDetail> _paymentDetail;
  std::unique_ptr<GeoPath> _geoPath;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxPointLoc2ApplicatorTest);
} // namespace tax
