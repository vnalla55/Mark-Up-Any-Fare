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

#include <cassert>
#include <memory>
#include <stdexcept>
#include "test/include/CppUnitHelperMacros.h"

#include "Common/TaxName.h"
#include "DataModel/Common/Types.h"
#include "DataModel/Services/RulesRecord.h"
#include "DomainDataObjects/Geo.h"
#include "DomainDataObjects/Flight.h"
#include "DomainDataObjects/GeoPath.h"
#include "Rules/BusinessRule.h"
#include "Rules/PaymentRuleData.h"
#include "Rules/ValidatingCarrierApplicator.h"
#include "Rules/ValidatingCarrierRule.h"
#include "TestServer/Facades/CarrierApplicationServiceServer.h"

using namespace std;

namespace tax
{
class ValidatingCarrierApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ValidatingCarrierApplicatorTest);

  CPPUNIT_TEST(testNoEntryInTable);
  CPPUNIT_TEST(testPositiveValidation);
  CPPUNIT_TEST(testNegativeValidation);
  CPPUNIT_TEST(testOptionalServiceSinglePositiveFareRelated);
  CPPUNIT_TEST(testOptionalServiceSinglePositiveFlightRelated);
  CPPUNIT_TEST(testOptionalServiceSingleNegativeFareRelated);
  CPPUNIT_TEST(testOptionalServiceSingleNegativeTicketRelated);
  CPPUNIT_TEST(testOptionalServiceMultiple);
  CPPUNIT_TEST(testBaggagePositive);
  CPPUNIT_TEST(testOptionalServicesAndBaggage);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _carrierApplicationItem.reset(new type::Index(0));
    _vendor.reset(new type::Vendor("ATP"));

    _validatingCarrierRule = new ValidatingCarrierRule(*_carrierApplicationItem, *_vendor);

    _seqNo.reset(new type::SeqNo());
    _ticketedPointTag.reset(
        new type::TicketedPointTag(type::TicketedPointTag::MatchTicketedAndUnticketedPoints));
    _taxName = new TaxName();

    _carrierCode.reset(new type::CarrierCode(UninitializedCode));

    _geoPath.reset(new GeoPath());
    for (int i = 0; i < 2; ++i)
    {
      _geoPath->geos().push_back(Geo());
      _geoPath->geos().at(i).id() = i;
      _geoPath->geos().at(i).loc().tag() =
          (i % 2) ? type::TaxPointTag::Arrival : type::TaxPointTag::Departure;
      _geoPath->geos().at(i).loc().nation() = "US";
    }

    _carrierApplicationServiceServer.reset(new CarrierApplicationServiceServer());
    _carrierApplicationServiceServer->carrierApplications().push_back(new CarrierApplication());
    _carrierApplicationServiceServer->carrierApplications()[0].itemNo = 1;
    _carrierApplicationServiceServer->carrierApplications()[0].vendor = "ATP";
    for (int j = 0; j < 3; ++j)
    {
      _carrierApplicationServiceServer->carrierApplications()[0].entries.push_back(
          new CarrierApplicationEntry());
    }

    _paymentDetail.reset(new PaymentDetail(
        PaymentRuleData(*_seqNo, *_ticketedPointTag, TaxableUnitTagSet::none(), 0,
            type::CurrencyCode(UninitializedCode),
            type::TaxAppliesToTagInd::Blank),
        _geoPath->geos().at(0),
        _geoPath->geos().at(0),
        *_taxName));
  }

  void tearDown()
  {
    delete _validatingCarrierRule;
    delete _taxName;
    _carrierApplication.reset();
  }

  void testNoEntryInTable()
  {
    *_carrierApplicationItem = 333;
    _carrierApplication =
        _carrierApplicationServiceServer->getCarrierApplication(*_vendor, *_carrierApplicationItem);
    ValidatingCarrierApplicator applicator(
        _validatingCarrierRule, *_carrierCode, _carrierApplication);

    *_carrierCode = "AA";
    CPPUNIT_ASSERT_EQUAL(false, applicator.apply(*_paymentDetail));
  }

  void testPositiveValidation()
  {
    setEntry(0, "AA", type::CarrierApplicationIndicator::Positive);
    setEntry(1, "AB", type::CarrierApplicationIndicator::Positive);
    setEntry(2, "AC", type::CarrierApplicationIndicator::Positive);

    *_carrierApplicationItem = 1;
    _carrierApplication =
        _carrierApplicationServiceServer->getCarrierApplication(*_vendor, *_carrierApplicationItem);
    ValidatingCarrierApplicator applicator(
        _validatingCarrierRule, *_carrierCode, _carrierApplication);

    *_carrierCode = "AA";
    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetail));

    *_carrierCode = "AC";
    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetail));

    *_carrierCode = "ZZ";
    CPPUNIT_ASSERT_EQUAL(false, applicator.apply(*_paymentDetail));
  }

  void testNegativeValidation()
  {
    setEntry(0, "AA", type::CarrierApplicationIndicator::Negative);
    setEntry(1, "$$", type::CarrierApplicationIndicator::Positive);
    setEntry(2, "AB", type::CarrierApplicationIndicator::Negative);

    *_carrierApplicationItem = 1;
    _carrierApplication =
        _carrierApplicationServiceServer->getCarrierApplication(*_vendor, *_carrierApplicationItem);
    ValidatingCarrierApplicator applicator(
        _validatingCarrierRule, *_carrierCode, _carrierApplication);

    *_carrierCode = "AA";
    CPPUNIT_ASSERT_EQUAL(false, applicator.apply(*_paymentDetail));

    *_carrierCode = "AB";
    CPPUNIT_ASSERT_EQUAL(false, applicator.apply(*_paymentDetail));

    *_carrierCode = "ZZ";
    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetail));
  }

  void testOptionalServiceSinglePositiveFareRelated()
  {
    setEntry(0, "AB", type::CarrierApplicationIndicator::Negative);
    setEntry(1, "AC", type::CarrierApplicationIndicator::Positive);
    setEntry(2, "$$", type::CarrierApplicationIndicator::Positive);

    addOptionalService("AB", type::OptionalServiceTag::FareRelated);
    *_carrierCode = "AC";

    *_carrierApplicationItem = 1;
    _carrierApplication =
        _carrierApplicationServiceServer->getCarrierApplication(*_vendor, *_carrierApplicationItem);
    ValidatingCarrierApplicator applicator(
        _validatingCarrierRule, *_carrierCode, _carrierApplication);

    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->optionalServiceItems()[0].isFailed());
  }

  void testOptionalServiceSinglePositiveFlightRelated()
  {
    setEntry(0, "AB", type::CarrierApplicationIndicator::Negative);
    setEntry(1, "AC", type::CarrierApplicationIndicator::Positive);
    setEntry(2, "$$", type::CarrierApplicationIndicator::Positive);

    addOptionalService("AC", type::OptionalServiceTag::FlightRelated);
    *_carrierCode = "AB";

    *_carrierApplicationItem = 1;
    _carrierApplication =
        _carrierApplicationServiceServer->getCarrierApplication(*_vendor, *_carrierApplicationItem);
    ValidatingCarrierApplicator applicator(
        _validatingCarrierRule, *_carrierCode, _carrierApplication);

    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->optionalServiceItems()[0].isFailed());
  }

  void testOptionalServiceSingleNegativeFareRelated()
  {
    setEntry(0, "AA", type::CarrierApplicationIndicator::Positive);
    setEntry(1, "AB", type::CarrierApplicationIndicator::Negative);
    setEntry(2, "$$", type::CarrierApplicationIndicator::Positive);

    addOptionalService("AA", type::OptionalServiceTag::FareRelated);
    *_carrierCode = "AB";

    *_carrierApplicationItem = 1;
    _carrierApplication =
        _carrierApplicationServiceServer->getCarrierApplication(*_vendor, *_carrierApplicationItem);
    ValidatingCarrierApplicator applicator(
        _validatingCarrierRule, *_carrierCode, _carrierApplication);

    CPPUNIT_ASSERT_EQUAL(false, applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->optionalServiceItems()[0].isFailed());
  }

  void testOptionalServiceSingleNegativeTicketRelated()
  {
    setEntry(0, "AA", type::CarrierApplicationIndicator::Positive);
    setEntry(1, "AB", type::CarrierApplicationIndicator::Negative);
    setEntry(2, "AC", type::CarrierApplicationIndicator::Positive);

    addOptionalService("AB", type::OptionalServiceTag::TicketRelated);
    *_carrierCode = "AA";

    *_carrierApplicationItem = 1;
    _carrierApplication =
        _carrierApplicationServiceServer->getCarrierApplication(*_vendor, *_carrierApplicationItem);
    ValidatingCarrierApplicator applicator(
        _validatingCarrierRule, *_carrierCode, _carrierApplication);

    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->optionalServiceItems()[0].isFailed());
  }

  void testOptionalServiceMultiple()
  {
    setEntry(0, "XX", type::CarrierApplicationIndicator::Negative);
    setEntry(1, "AB", type::CarrierApplicationIndicator::Negative);
    setEntry(2, "$$", type::CarrierApplicationIndicator::Positive);

    addOptionalService("AA", type::OptionalServiceTag::FareRelated);
    addOptionalService("AB", type::OptionalServiceTag::FareRelated);
    addOptionalService("AA", type::OptionalServiceTag::Merchandise);
    addOptionalService("AB", type::OptionalServiceTag::Merchandise);
    addOptionalService("XX", type::OptionalServiceTag::FareRelated);
    addOptionalService("XX", type::OptionalServiceTag::FlightRelated);

    *_carrierCode = "AA";

    *_carrierApplicationItem = 1;
    _carrierApplication =
        _carrierApplicationServiceServer->getCarrierApplication(*_vendor, *_carrierApplicationItem);
    ValidatingCarrierApplicator applicator(
        _validatingCarrierRule, *_carrierCode, _carrierApplication);

    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->optionalServiceItems()[0].isFailed());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->optionalServiceItems()[1].isFailed());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->optionalServiceItems()[2].isFailed());
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->optionalServiceItems()[3].isFailed());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->optionalServiceItems()[4].isFailed());
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->optionalServiceItems()[5].isFailed());
  }

  void testBaggagePositive()
  {
    setEntry(0, "AB", type::CarrierApplicationIndicator::Negative);
    setEntry(1, "AC", type::CarrierApplicationIndicator::Positive);
    setEntry(2, "$$", type::CarrierApplicationIndicator::Positive);

    addOptionalService("AC", type::OptionalServiceTag::BaggageCharge);
    *_carrierCode = "AB";

    *_carrierApplicationItem = 1;
    _carrierApplication =
        _carrierApplicationServiceServer->getCarrierApplication(*_vendor, *_carrierApplicationItem);
    ValidatingCarrierApplicator applicator(
        _validatingCarrierRule, *_carrierCode, _carrierApplication);

    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->optionalServiceItems()[0].isFailed());
  }

  void testOptionalServicesAndBaggage()
  {
    setEntry(0, "XX", type::CarrierApplicationIndicator::Negative);
    setEntry(1, "AB", type::CarrierApplicationIndicator::Negative);
    setEntry(2, "$$", type::CarrierApplicationIndicator::Positive);

    addOptionalService("AA", type::OptionalServiceTag::FareRelated);
    addOptionalService("AB", type::OptionalServiceTag::FareRelated);
    addOptionalService("AA", type::OptionalServiceTag::Merchandise);
    addOptionalService("AB", type::OptionalServiceTag::Merchandise);
    addOptionalService("XX", type::OptionalServiceTag::FareRelated);
    addOptionalService("XX", type::OptionalServiceTag::FlightRelated);
    addOptionalService("AA", type::OptionalServiceTag::BaggageCharge);
    addOptionalService("XX", type::OptionalServiceTag::BaggageCharge);

    *_carrierCode = "AA";

    *_carrierApplicationItem = 1;
    _carrierApplication =
        _carrierApplicationServiceServer->getCarrierApplication(*_vendor, *_carrierApplicationItem);
    ValidatingCarrierApplicator applicator(
        _validatingCarrierRule, *_carrierCode, _carrierApplication);

    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->optionalServiceItems()[0].isFailed());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->optionalServiceItems()[1].isFailed());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->optionalServiceItems()[2].isFailed());
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->optionalServiceItems()[3].isFailed());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->optionalServiceItems()[4].isFailed());
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->optionalServiceItems()[5].isFailed());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->optionalServiceItems()[6].isFailed());
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->optionalServiceItems()[7].isFailed());
  }

private:
  ValidatingCarrierRule* _validatingCarrierRule;
  TaxName* _taxName;

  std::unique_ptr<type::Index> _carrierApplicationItem;
  std::unique_ptr<type::Vendor> _vendor;
  std::unique_ptr<type::TaxPointTag> _taxPointTag;
  std::unique_ptr<CarrierApplicationServiceServer> _carrierApplicationServiceServer;
  std::unique_ptr<type::SeqNo> _seqNo;
  std::unique_ptr<GeoPath> _geoPath;
  std::unique_ptr<type::TicketedPointTag> _ticketedPointTag;
  std::unique_ptr<PaymentDetail> _paymentDetail;
  std::unique_ptr<type::CarrierCode> _carrierCode;
  std::shared_ptr<const CarrierApplication> _carrierApplication;

  void setEntry(size_t const& index,
                type::CarrierCode const& carrierCode,
                type::CarrierApplicationIndicator const& indicator)
  {
    CarrierApplicationEntry& entry =
        _carrierApplicationServiceServer->carrierApplications()[0].entries[index];

    entry.carrier = carrierCode;
    entry.applind = indicator;
  }

  void
  addOptionalService(type::CarrierCode const& carrierCode,
                     type::OptionalServiceTag const& type = type::OptionalServiceTag::FareRelated)
  {
    OptionalService* optionalService = new OptionalService();
    optionalService->ownerCarrier() = carrierCode;
    optionalService->type() = type;

    _paymentDetail->optionalServiceItems().push_back(optionalService);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ValidatingCarrierApplicatorTest);
} // namespace tax
