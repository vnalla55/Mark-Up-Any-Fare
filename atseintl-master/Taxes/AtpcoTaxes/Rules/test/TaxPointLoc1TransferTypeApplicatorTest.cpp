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

#include "Common/Consts.h"
#include "Common/TaxName.h"
#include "DataModel/Common/Types.h"
#include "DomainDataObjects/Flight.h"
#include "DomainDataObjects/FlightUsage.h"
#include "DomainDataObjects/Geo.h"
#include "Rules/PaymentDetail.h"
#include "Rules/PaymentRuleData.h"
#include "Rules/TaxPointLoc1TransferTypeApplicator.h"
#include "Rules/TaxPointLoc1TransferTypeRule.h"

#include <memory>

namespace tax
{

class TaxPointLoc1TransferTypeApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxPointLoc1TransferTypeApplicatorTest);

  CPPUNIT_TEST(testTagBlank_Apply);
  CPPUNIT_TEST(testFirstTaxPoint_NoApply);
  CPPUNIT_TEST(testLastTaxPoint_NoApply);

  CPPUNIT_TEST(testInterlineSameCarrier_NoApply);
  CPPUNIT_TEST(testInterlineBlank_Apply);
  CPPUNIT_TEST(testInterline_Apply);

  CPPUNIT_TEST(testOnlineWithChangeFlight_DifferentCarrierNoApply);
  CPPUNIT_TEST(testOnlineWithChangeFlight_SameFlightNoApply);
  CPPUNIT_TEST(testOnlineWithChangeFlightBlank_Apply);
  CPPUNIT_TEST(testOnlineWithChangeFlight_Apply);
  CPPUNIT_TEST(testOnlineWithChangeFlight_Apply);

  CPPUNIT_TEST(testOnlineCHG_DifferentCarrierNoApply);
  CPPUNIT_TEST(testOnlineCHG_DifferentFlightNoApply);
  CPPUNIT_TEST(testOnlineCHG_SameEquipmentNoApply);
  CPPUNIT_TEST(testOnlineCHGBlank_Apply);
  CPPUNIT_TEST(testOnlineCHG_Apply);
  CPPUNIT_TEST(testOnlineCHG_BothCHGApply);

  CPPUNIT_TEST(testOnlineNoCHG_DifferentCarrierNoApply);
  CPPUNIT_TEST(testOnlineNoCHG_DifferentFlightNoApply);
  CPPUNIT_TEST(testOnlineNoCHG_DifferentEquipmentNoApply);
  CPPUNIT_TEST(testOnlineNoCHGBlank_Apply);
  CPPUNIT_TEST(testOnlineNoCHG_Apply);
  CPPUNIT_TEST(testOnlineNoCHGBothCHG_NoApply);

  CPPUNIT_TEST_SUITE_END();

public:
  void createApplicator(const type::TransferTypeTag type)
  {
    _rule.reset(new TaxPointLoc1TransferTypeRule(type));
    _applicator.reset(new TaxPointLoc1TransferTypeApplicator(*_rule, _flightUsages, _flights));
  }

  const std::shared_ptr<PaymentDetail> createPaymentDetail(const type::Index taxPointLoc1Id = 1)
  {
    _taxPoint->id() = taxPointLoc1Id;
    std::shared_ptr<PaymentDetail> paymentDetail =
        std::make_shared<PaymentDetail>(PaymentRuleData(type::SeqNo(),
                                                        *_ticketedPointTag,
                                                        TaxableUnitTagSet::none(),
                                                        0,
                                                        type::CurrencyCode(UninitializedCode),
                                                        type::TaxAppliesToTagInd::Blank),
                                        *_taxPoint,
                                        *_taxPoint,
                                        *_taxName);
    paymentDetail->getMutableTaxPointsProperties().resize(_flightUsages.size() * 2);
    return paymentDetail;
  }

  const std::shared_ptr<PaymentDetail> createOpenPaymentDetail(const type::Index taxPointLoc1Id = 1)
  {
    std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail(taxPointLoc1Id);
    paymentDetail->getMutableTaxPointsProperties()[1].isOpen = true;
    paymentDetail->getMutableTaxPointsProperties()[2].isOpen = true;
    return paymentDetail;
  }

  void setUp()
  {
    _flightUsages.push_back(FlightUsage());
    _flightUsages.push_back(FlightUsage());
    _flightUsages.at(0).flightRefId() = 0;
    _flightUsages.at(1).flightRefId() = 1;

    _flights.push_back(Flight());
    _flights.push_back(Flight());
    _flights.at(0).marketingCarrier() = type::CarrierCode("AA");
    _flights.at(1).marketingCarrier() = type::CarrierCode("AA");
    _flights.at(0).marketingCarrierFlightNumber() = type::FlightNumber(34);
    _flights.at(1).marketingCarrierFlightNumber() = type::FlightNumber(34);
    _flights.at(0).equipment() = type::EquipmentCode("ABC");
    _flights.at(1).equipment() = type::EquipmentCode("ABC");

    _ticketedPointTag =
      new type::TicketedPointTag(type::TicketedPointTag::MatchTicketedAndUnticketedPoints);

    _taxPoint = new Geo();
    _taxName = new TaxName();
  }

  void tearDown()
  {
    _rule.reset();
    _applicator.reset();
    _flightUsages.clear();
    _flights.clear();
    delete _ticketedPointTag;
    delete _taxPoint;
    delete _taxName;
  }

  void testTagBlank_Apply()
  {
    createApplicator(type::TransferTypeTag::Blank);
    CPPUNIT_ASSERT(_applicator->apply(*createPaymentDetail()));
  }

  void testFirstTaxPoint_NoApply()
  {
    createApplicator(type::TransferTypeTag::Interline);

    const type::Index firstTaxPoint(0);
    CPPUNIT_ASSERT(!_applicator->apply(*createPaymentDetail(firstTaxPoint)));
  }

  void testLastTaxPoint_NoApply()
  {
    createApplicator(type::TransferTypeTag::Interline);

    const type::Index lastTaxPoint(type::Index((_flightUsages.size() * 2) - 1));
    CPPUNIT_ASSERT(!_applicator->apply(*createPaymentDetail(lastTaxPoint)));
  }

  void testInterlineSameCarrier_NoApply()
  {
    _flights.at(0).marketingCarrier() = type::CarrierCode("AA");
    _flights.at(1).marketingCarrier() = type::CarrierCode("AA");

    createApplicator(type::TransferTypeTag::Interline);

    CPPUNIT_ASSERT(!_applicator->apply(*createPaymentDetail()));
  }

  void testInterlineBlank_Apply()
  {
    _flights.at(0).marketingCarrier() = type::CarrierCode("AA");
    _flights.at(1).marketingCarrier() = type::CarrierCode(UninitializedCode);

    createApplicator(type::TransferTypeTag::Interline);

    CPPUNIT_ASSERT(_applicator->apply(*createPaymentDetail()));
    CPPUNIT_ASSERT(_applicator->apply(*createOpenPaymentDetail()));
  }

  void testInterline_Apply()
  {
    _flights.at(0).marketingCarrier() = type::CarrierCode("AA");
    _flights.at(1).marketingCarrier() = type::CarrierCode("BB");

    createApplicator(type::TransferTypeTag::Interline);

    CPPUNIT_ASSERT(_applicator->apply(*createPaymentDetail()));
    CPPUNIT_ASSERT(_applicator->apply(*createOpenPaymentDetail()));
  }

  void testOnlineWithChangeFlight_DifferentCarrierNoApply()
  {
    _flights.at(0).marketingCarrier() = type::CarrierCode("AA");
    _flights.at(1).marketingCarrier() = type::CarrierCode("BB");

    createApplicator(type::TransferTypeTag::OnlineWithChangeOfFlightNumber);

    CPPUNIT_ASSERT(!_applicator->apply(*createPaymentDetail()));
  }

  void testOnlineWithChangeFlight_SameFlightNoApply()
  {
    _flights.at(0).marketingCarrierFlightNumber() = type::FlightNumber(34);
    _flights.at(1).marketingCarrierFlightNumber() = type::FlightNumber(34);

    createApplicator(type::TransferTypeTag::OnlineWithChangeOfFlightNumber);

    CPPUNIT_ASSERT(!_applicator->apply(*createPaymentDetail()));
  }

  void testOnlineWithChangeFlightBlank_Apply()
  {
    _flights.at(0).marketingCarrier() = type::CarrierCode("AA");
    _flights.at(1).marketingCarrier() = type::CarrierCode(UninitializedCode);
    _flights.at(0).marketingCarrierFlightNumber() = type::FlightNumber(34);
    _flights.at(1).marketingCarrierFlightNumber() = type::FlightNumber(0);

    createApplicator(type::TransferTypeTag::OnlineWithChangeOfFlightNumber);

    CPPUNIT_ASSERT(_applicator->apply(*createPaymentDetail()));
    CPPUNIT_ASSERT(_applicator->apply(*createOpenPaymentDetail()));
  }

  void testOnlineWithChangeFlight_Apply()
  {
    _flights.at(0).marketingCarrierFlightNumber() = type::FlightNumber(34);
    _flights.at(1).marketingCarrierFlightNumber() = type::FlightNumber(35);

    createApplicator(type::TransferTypeTag::OnlineWithChangeOfFlightNumber);

    CPPUNIT_ASSERT(_applicator->apply(*createPaymentDetail()));
    CPPUNIT_ASSERT(_applicator->apply(*createOpenPaymentDetail()));
  }

  void testOnlineCHG_DifferentCarrierNoApply()
  {
    _flights.at(0).marketingCarrier() = type::CarrierCode("AA");
    _flights.at(1).marketingCarrier() = type::CarrierCode("BB");

    createApplicator(type::TransferTypeTag::OnlineWithChangeOfGauge);

    CPPUNIT_ASSERT(!_applicator->apply(*createPaymentDetail()));
  }

  void testOnlineCHG_DifferentFlightNoApply()
  {
    _flights.at(0).marketingCarrierFlightNumber() = type::FlightNumber(45);
    _flights.at(1).marketingCarrierFlightNumber() = type::FlightNumber(46);

    createApplicator(type::TransferTypeTag::OnlineWithChangeOfGauge);

    CPPUNIT_ASSERT(!_applicator->apply(*createPaymentDetail()));
  }

  void testOnlineCHG_SameEquipmentNoApply()
  {
    _flights.at(0).equipment() = type::EquipmentCode("ABC");
    _flights.at(1).equipment() = type::EquipmentCode("ABC");

    createApplicator(type::TransferTypeTag::OnlineWithChangeOfGauge);

    CPPUNIT_ASSERT(!_applicator->apply(*createPaymentDetail()));
  }

  void testOnlineCHGBlank_Apply()
  {
    _flights.at(0).marketingCarrier() = type::CarrierCode("AA");
    _flights.at(1).marketingCarrier() = type::CarrierCode(UninitializedCode);
    _flights.at(0).marketingCarrierFlightNumber() = type::FlightNumber(34);
    _flights.at(1).marketingCarrierFlightNumber() = type::FlightNumber(0);
    _flights.at(0).equipment() = type::EquipmentCode("ABC");
    _flights.at(1).equipment() = type::EquipmentCode(BLANK);

    createApplicator(type::TransferTypeTag::OnlineWithChangeOfGauge);

    CPPUNIT_ASSERT(_applicator->apply(*createPaymentDetail()));
    CPPUNIT_ASSERT(_applicator->apply(*createOpenPaymentDetail()));
  }

  void testOnlineCHG_Apply()
  {
    _flights.at(0).equipment() = type::EquipmentCode("ABC");
    _flights.at(1).equipment() = type::EquipmentCode("DEF");

    createApplicator(type::TransferTypeTag::OnlineWithChangeOfGauge);

    CPPUNIT_ASSERT(_applicator->apply(*createPaymentDetail()));
    CPPUNIT_ASSERT(_applicator->apply(*createOpenPaymentDetail()));
  }

  void testOnlineCHG_BothCHGApply()
  {
    _flights.at(0).equipment() = type::EquipmentCode("CHG");
    _flights.at(1).equipment() = type::EquipmentCode("CHG");

    createApplicator(type::TransferTypeTag::OnlineWithChangeOfGauge);

    CPPUNIT_ASSERT(_applicator->apply(*createPaymentDetail()));
  }

  void testOnlineNoCHG_DifferentCarrierNoApply()
  {
    _flights.at(0).marketingCarrier() = type::CarrierCode("AA");
    _flights.at(1).marketingCarrier() = type::CarrierCode("BB");

    createApplicator(type::TransferTypeTag::OnlineWithNoChangeOfGauge);

    CPPUNIT_ASSERT(!_applicator->apply(*createPaymentDetail()));
  }

  void testOnlineNoCHG_DifferentFlightNoApply()
  {
    _flights.at(0).marketingCarrierFlightNumber() = type::FlightNumber(45);
    _flights.at(1).marketingCarrierFlightNumber() = type::FlightNumber(46);

    createApplicator(type::TransferTypeTag::OnlineWithNoChangeOfGauge);

    CPPUNIT_ASSERT(!_applicator->apply(*createPaymentDetail()));
  }

  void testOnlineNoCHG_DifferentEquipmentNoApply()
  {
    _flights.at(0).equipment() = type::EquipmentCode("ABC");
    _flights.at(1).equipment() = type::EquipmentCode("DEF");

    createApplicator(type::TransferTypeTag::OnlineWithNoChangeOfGauge);

    CPPUNIT_ASSERT(!_applicator->apply(*createPaymentDetail()));
  }

  void testOnlineNoCHGBlank_Apply()
  {
    _flights.at(0).marketingCarrier() = type::CarrierCode("AA");
    _flights.at(1).marketingCarrier() = type::CarrierCode(UninitializedCode);
    _flights.at(0).marketingCarrierFlightNumber() = type::FlightNumber(34);
    _flights.at(1).marketingCarrierFlightNumber() = type::FlightNumber(0);
    _flights.at(0).equipment() = type::EquipmentCode("ABC");
    _flights.at(1).equipment() = type::EquipmentCode(BLANK);

    createApplicator(type::TransferTypeTag::OnlineWithNoChangeOfGauge);

    CPPUNIT_ASSERT(_applicator->apply(*createPaymentDetail()));
    CPPUNIT_ASSERT(_applicator->apply(*createOpenPaymentDetail()));
  }

  void testOnlineNoCHG_Apply()
  {
    _flights.at(0).equipment() = type::EquipmentCode("ABC");
    _flights.at(1).equipment() = type::EquipmentCode("ABC");

    createApplicator(type::TransferTypeTag::OnlineWithNoChangeOfGauge);

    CPPUNIT_ASSERT(_applicator->apply(*createPaymentDetail()));
    CPPUNIT_ASSERT(_applicator->apply(*createOpenPaymentDetail()));
  }

  void testOnlineNoCHGBothCHG_NoApply()
  {
    _flights.at(0).equipment() = type::EquipmentCode("CHG");
    _flights.at(1).equipment() = type::EquipmentCode("CHG");

    createApplicator(type::TransferTypeTag::OnlineWithNoChangeOfGauge);

    CPPUNIT_ASSERT(!_applicator->apply(*createPaymentDetail()));
  }

private:
  std::unique_ptr<TaxPointLoc1TransferTypeRule> _rule;
  std::unique_ptr<TaxPointLoc1TransferTypeApplicator> _applicator;

  std::vector<FlightUsage> _flightUsages;
  std::vector<Flight> _flights;

  type::TicketedPointTag* _ticketedPointTag;
  Geo* _taxPoint;
  TaxName* _taxName;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxPointLoc1TransferTypeApplicatorTest);
} // namespace tax
