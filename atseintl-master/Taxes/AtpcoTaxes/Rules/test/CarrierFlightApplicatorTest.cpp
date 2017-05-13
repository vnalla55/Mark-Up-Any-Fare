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

#include <boost/assign/list_of.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/lexical_cast.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "Common/Consts.h"
#include "Common/TaxName.h"
#include "DataModel/Common/CodeIO.h"
#include "DataModel/Common/Types.h"
#include "DomainDataObjects/Flight.h"
#include "DomainDataObjects/Geo.h"
#include "DomainDataObjects/GeoPath.h"
#include "Rules/BusinessRule.h"
#include "Rules/PaymentDetail.h"
#include "Rules/PaymentRuleData.h"
#include "Rules/CarrierFlightApplicator.h"
#include "Rules/CarrierFlightRule.h"
#include "TestServer/Facades/CarrierFlightServiceServer.h"

#define PARAM_TESTS(_Inputs_, _Outputs_, _InitMethod_, _TestMethod_) \
  _InitMethod_(); \
  assert(_Inputs_.size() == _Outputs_.size()); \
  for (size_t i (0); i < _Inputs_.size(); ++i) \
  { \
    CPPUNIT_TEST_SUITE_ADD_TEST( \
    ( new CPPUNIT_NS::TestCaller<TestFixtureType>(context.getTestNameFor(#_TestMethod_ + boost::lexical_cast<std::string>(i)), \
                                                  &TestFixtureType::_TestMethod_, \
                                                  context.makeFixture() )) \
    ); \
  }

namespace tax
{

namespace
{
  type::CurrencyCode noCurrency (UninitializedCode);
}

class CarrierFlightApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CarrierFlightApplicatorTest);

  CPPUNIT_TEST(testNothingToValidate);

  PARAM_TESTS(_ruleDoesNotExistInputs,
              _ruleDoesNotExistOutputs,
              initRuleDoesNotExist,
              testRuleDoesNotExist);

  PARAM_TESTS(_carrierInputs,
              _carrierOutputs,
              initRuleMatchWithCarrier,
              testRuleMatchWithCarriers);

  CPPUNIT_TEST(testRuleMatchFailingOnStartOrEndPoint);

  PARAM_TESTS(_flightsInputs,
              _flightsOutputs,
              initRuleMatchWithFlightNumbers,
              testRuleMatchWithFlightNumbers);

  PARAM_TESTS(_beforeAndAfterInputs,
              _beforeAndAfterOutputs,
              initRuleMatchBeforeAndAfter,
              testRuleMatchBeforeAndAfter);

  PARAM_TESTS(_beforeAndAfterForArrivalInputs,
              _beforeAndAfterForArrivalOutputs,
              initRuleMatchBeforeAndAfterForArrival,
              testRuleMatchBeforeAndAfterForArrival);

  CPPUNIT_TEST(applicableWithOCFees0);
  CPPUNIT_TEST(applicableWithOCFees1);
  CPPUNIT_TEST(notApplicableWithOCFees);
  CPPUNIT_TEST(applicableWithBaggage0);
  CPPUNIT_TEST(applicableWithBaggage1);
  CPPUNIT_TEST(notApplicableWithBaggage);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _taxPointTag.reset(new type::TaxPointTag(type::TaxPointTag::Departure));
    _vendor.reset(new type::Vendor("ATP"));

    _carrierFlightRule.reset(new CarrierFlightRule(type::Index(0), type::Index(0), *_taxPointTag,
        *_vendor));

    _seqNo.reset(new type::SeqNo());
    _ticketedPointTag.reset(
      new type::TicketedPointTag(type::TicketedPointTag::MatchTicketedAndUnticketedPoints));
    _taxName = new TaxName();

    _geoPath.reset(new GeoPath());
    for (int i = 0; i < 8; ++i)
    {
      _geoPath->geos().push_back(Geo());
      _geoPath->geos().at(i).id() = i;
      _geoPath->geos().at(i).loc().tag() =
        (i % 2) ? type::TaxPointTag::Arrival : type::TaxPointTag::Departure;
      _geoPath->geos().at(i).loc().nation() = "US";
    }

    _properties.resize(8);
    _properties[0].isFirst = true;
    _properties[7].isLast = true;

    /* BA 1000 (AA)
     * BA 1001 (AA)
     * BA 1002 (AA)
     * BA 1003 (AA)
     */
    _flights.reset(new boost::ptr_vector<Flight>());
    _flightUsages.reset(new std::vector<FlightUsage>());
    for (int i = 0; i < 4; ++i)
    {
      _flights->push_back(new Flight());
      _flights->at(i).operatingCarrier() = "AA";
      _flights->at(i).marketingCarrier() = "BA";
      _flights->at(i).marketingCarrierFlightNumber() = type::FlightNumber(1000 + i);
      _flightUsages->push_back(FlightUsage());
      _flightUsages->at(i).flight() = &_flights->at(i);
    }

    _carrierFlightServiceServer.reset(new CarrierFlightServiceServer());
    for (int i = 0; i < 16; ++i)
    {
      _carrierFlightServiceServer->carrierFlights().push_back(new CarrierFlight());
      _carrierFlightServiceServer->carrierFlights()[i].itemNo = i + 1;
      _carrierFlightServiceServer->carrierFlights()[i].vendor = "ATP";
      for (int j = 0; j < 4; ++j)
      {
        _carrierFlightServiceServer->carrierFlights()[i]
          .segments.push_back(new CarrierFlightSegment());
      }
    }
  }

  void tearDown()
  {
    delete _taxName;
  }

  void testNothingToValidate()
  {
    TaxableUnitTagSet noUnits = TaxableUnitTagSet::none();
    _paymentDetail.reset(
        new PaymentDetail(PaymentRuleData(*_seqNo, *_ticketedPointTag, noUnits, 0, noCurrency,
                            type::TaxAppliesToTagInd::Blank),
                          _geoPath->geos().at(0),
                          _geoPath->geos().at(0),
                          *_taxName));
    _paymentDetail->getMutableTaxPointsProperties() = _properties;

    _carrierFlightRule.reset(new CarrierFlightRule(type::Index(0), type::Index(0), *_taxPointTag,
        *_vendor));
    initFligthsAndApplicator();
    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail));
  }

private:
  struct CommonInput
  {
    CommonInput(size_t id, const type::FlightNumber& before, const type::FlightNumber& after)
      : _geoId(id), _before(before), _after(after)
    {
    }

    size_t _geoId;
    type::FlightNumber _before;
    type::FlightNumber _after;
  };
  static std::vector<CommonInput> _ruleDoesNotExistInputs;
  static std::vector<bool> _ruleDoesNotExistOutputs;
  static size_t _ruleDoesNotExistCounter;

  static void initRuleDoesNotExist()
  {
    using namespace boost::assign;

    _ruleDoesNotExistCounter = 0u;
    _ruleDoesNotExistInputs.clear();
    _ruleDoesNotExistInputs += CommonInput(1, 333, 0);
    _ruleDoesNotExistInputs += CommonInput(1, 0, 333);

    _ruleDoesNotExistOutputs.clear();
    _ruleDoesNotExistOutputs += false;
    _ruleDoesNotExistOutputs += false;
  }

public:
  void testRuleDoesNotExist()
  {
    const CommonInput& input = _ruleDoesNotExistInputs[_ruleDoesNotExistCounter];
    bool output = _ruleDoesNotExistOutputs[_ruleDoesNotExistCounter];
    _ruleDoesNotExistCounter++;

    TaxableUnitTagSet noUnits = TaxableUnitTagSet::none();
    _paymentDetail.reset(
        new PaymentDetail(PaymentRuleData(*_seqNo, *_ticketedPointTag, noUnits, 0, noCurrency,
                            type::TaxAppliesToTagInd::Blank),
                          _geoPath->geos().at(input._geoId),
                          _geoPath->geos().at(input._geoId),
                          *_taxName));
    _paymentDetail->getMutableTaxPointsProperties() = _properties;

    _carrierFlightRule.reset(new CarrierFlightRule(type::Index(input._before),
        type::Index(input._after), *_taxPointTag, *_vendor));
    initFligthsAndApplicator();
    CPPUNIT_ASSERT_EQUAL(output, _applicator->apply(*_paymentDetail));
  }

  void testRuleMatchFailingOnStartOrEndPoint()
  {
    TaxableUnitTagSet noUnits = TaxableUnitTagSet::none();
    _paymentDetail.reset(
        new PaymentDetail(PaymentRuleData(*_seqNo, *_ticketedPointTag, noUnits, 0, noCurrency,
                            type::TaxAppliesToTagInd::Blank),
                          _geoPath->geos().at(0),
                          _geoPath->geos().at(0),
                          *_taxName));
    _paymentDetail->getMutableTaxPointsProperties() = _properties;

    _carrierFlightRule.reset(new CarrierFlightRule(type::Index(1), type::Index(0), *_taxPointTag,
        *_vendor));
    initFligthsAndApplicator();
    CPPUNIT_ASSERT_EQUAL(false, _applicator->apply(*_paymentDetail));

    _paymentDetail.reset(
        new PaymentDetail(PaymentRuleData(*_seqNo, *_ticketedPointTag, noUnits, 0, noCurrency,
                            type::TaxAppliesToTagInd::Blank),
                          _geoPath->geos().at(7),
                          _geoPath->geos().at(7),
                          *_taxName));
    _paymentDetail->getMutableTaxPointsProperties() = _properties;

    _carrierFlightRule.reset(new CarrierFlightRule(type::Index(0), type::Index(1), *_taxPointTag,
        *_vendor));
    initFligthsAndApplicator();
    CPPUNIT_ASSERT_EQUAL(false, _applicator->apply(*_paymentDetail));
  }

private:
  static std::vector<CommonInput> _carrierInputs;
  static std::vector<bool> _carrierOutputs;
  static size_t _carrierCounter;

  static void initRuleMatchWithCarrier()
  {
    using namespace boost::assign;

    _carrierCounter = 0u;
    _carrierInputs.clear();
    _carrierInputs += CommonInput(0, 0, 1);
    _carrierInputs += CommonInput(0, 0, 2);
    _carrierInputs += CommonInput(0, 0, 3);
    _carrierInputs += CommonInput(0, 0, 4);
    _carrierInputs += CommonInput(0, 0, 5);

    _carrierInputs += CommonInput(7, 1, 0);
    _carrierInputs += CommonInput(7, 2, 0);
    _carrierInputs += CommonInput(7, 3, 0);
    _carrierInputs += CommonInput(7, 4, 0);
    _carrierInputs += CommonInput(7, 5, 0);

    _carrierOutputs.clear();
    _carrierOutputs += true;
    _carrierOutputs += true;
    _carrierOutputs += false;
    _carrierOutputs += false;
    _carrierOutputs += false;

    _carrierOutputs += true;
    _carrierOutputs += true;
    _carrierOutputs += false;
    _carrierOutputs += false;
    _carrierOutputs += false;
  }

public:
  void testRuleMatchWithCarriers()
  {
    for (int i = 0; i < 5; ++i)
    {
      _carrierFlightServiceServer->carrierFlights()[i].segments[0].flightFrom = -1;
      _carrierFlightServiceServer->carrierFlights()[i].segments[0].flightTo = MAX_FLIGHT_NUMBER;
    }
    _carrierFlightServiceServer->carrierFlights()[0].segments[0].marketingCarrier = "BA";
    _carrierFlightServiceServer->carrierFlights()[0].segments[0].operatingCarrier = type::CarrierCode(UninitializedCode);
    _carrierFlightServiceServer->carrierFlights()[1].segments[0].marketingCarrier = "BA";
    _carrierFlightServiceServer->carrierFlights()[1].segments[0].operatingCarrier = "AA";
    _carrierFlightServiceServer->carrierFlights()[2].segments[0].marketingCarrier = "XA";
    _carrierFlightServiceServer->carrierFlights()[2].segments[0].operatingCarrier = type::CarrierCode(UninitializedCode);
    _carrierFlightServiceServer->carrierFlights()[3].segments[0].marketingCarrier = "BA";
    _carrierFlightServiceServer->carrierFlights()[3].segments[0].operatingCarrier = "XA";
    _carrierFlightServiceServer->carrierFlights()[4].segments[0].marketingCarrier = "XA";
    _carrierFlightServiceServer->carrierFlights()[4].segments[0].operatingCarrier = "XB";

    const CommonInput& input = _carrierInputs[_carrierCounter];
    bool output = _carrierOutputs[_carrierCounter];
    _carrierCounter++;

    TaxableUnitTagSet noUnits = TaxableUnitTagSet::none();
    _paymentDetail.reset(
        new PaymentDetail(PaymentRuleData(*_seqNo, *_ticketedPointTag, noUnits, 0, noCurrency,
                            type::TaxAppliesToTagInd::Blank),
                          _geoPath->geos().at(input._geoId),
                          _geoPath->geos().at(input._geoId),
                          *_taxName));
    _paymentDetail->getMutableTaxPointsProperties() = _properties;

    _carrierFlightRule.reset(new CarrierFlightRule(type::Index(input._before),
        type::Index(input._after), *_taxPointTag, *_vendor));
    initFligthsAndApplicator();
    CPPUNIT_ASSERT_EQUAL(output, _applicator->apply(*_paymentDetail));
  }

private:
  struct FlightNumberInput : public CommonInput
  {
    FlightNumberInput(size_t id,
                      const type::FlightNumber& before,
                      const type::FlightNumber& after,
                      const std::vector<type::FlightNumber>& flights)
      : CommonInput(id, before, after), _flights(flights)
    {
    }

    std::vector<type::FlightNumber> _flights;
  };
  static std::vector<FlightNumberInput> _flightsInputs;
  static std::vector<bool> _flightsOutputs;
  static size_t _flightsCounter;

  static void initRuleMatchWithFlightNumbers()
  {
    using namespace boost::assign;

    _flightsCounter = 0u;

    std::vector<type::FlightNumber> flt1000 = list_of(-1)(MAX_FLIGHT_NUMBER)
                                                     (1)(1000)
                                                     (1)(999)
                                                     (1000)(1000)
                                                     (990)(990)
                                                     (990)(1010)
                                                     (990)(999)
                                                     (1001)(1010);

    std::vector<type::FlightNumber> flt1003 = list_of(-1)(MAX_FLIGHT_NUMBER)
                                                     (1)(1003)
                                                     (1)(1002)
                                                     (1003)(1003)
                                                     (1000)(1000)
                                                     (990)(1010)
                                                     (990)(999)
                                                     (1004)(1010);

    _flightsInputs.clear();
    _flightsInputs += FlightNumberInput(0, 0, 1, flt1000);
    _flightsInputs += FlightNumberInput(0, 0, 2, flt1000);
    _flightsInputs += FlightNumberInput(0, 0, 3, flt1000);
    _flightsInputs += FlightNumberInput(0, 0, 4, flt1000);
    _flightsInputs += FlightNumberInput(0, 0, 5, flt1000);
    _flightsInputs += FlightNumberInput(0, 0, 6, flt1000);
    _flightsInputs += FlightNumberInput(0, 0, 7, flt1000);
    _flightsInputs += FlightNumberInput(0, 0, 8, flt1000);

    _flightsInputs += FlightNumberInput(7, 1, 0, flt1003);
    _flightsInputs += FlightNumberInput(7, 2, 0, flt1003);
    _flightsInputs += FlightNumberInput(7, 3, 0, flt1003);
    _flightsInputs += FlightNumberInput(7, 4, 0, flt1003);
    _flightsInputs += FlightNumberInput(7, 5, 0, flt1003);
    _flightsInputs += FlightNumberInput(7, 6, 0, flt1003);
    _flightsInputs += FlightNumberInput(7, 7, 0, flt1003);
    _flightsInputs += FlightNumberInput(7, 8, 0, flt1003);

    std::vector<bool> outs = list_of(true)(true)(false)(true)(false)(true)(false)(false)
                                    (true)(true)(false)(true)(false)(true)(false)(false);
    _flightsOutputs = outs;
  }

public:
  void testRuleMatchWithFlightNumbers()
  {
    const FlightNumberInput& input = _flightsInputs[_flightsCounter];
    bool output = _flightsOutputs[_flightsCounter];
    _flightsCounter++;

    for (size_t i = 0; i < input._flights.size()/2; ++i)
    {
      _carrierFlightServiceServer->carrierFlights()[i].segments[0].flightFrom = input._flights[2*i];
      _carrierFlightServiceServer->carrierFlights()[i].segments[0].flightTo = input._flights[2*i+1];
    }

    for (size_t i = 0; i < _carrierFlightServiceServer->carrierFlights().size(); ++i)
    {
      _carrierFlightServiceServer->carrierFlights()[i].segments[0].marketingCarrier = "BA";
      _carrierFlightServiceServer->carrierFlights()[i].segments[0].operatingCarrier = type::CarrierCode(UninitializedCode);
    }

    TaxableUnitTagSet noUnits = TaxableUnitTagSet::none();
    _paymentDetail.reset(
        new PaymentDetail(PaymentRuleData(*_seqNo, *_ticketedPointTag, noUnits, 0, noCurrency,
                            type::TaxAppliesToTagInd::Blank),
                          _geoPath->geos().at(input._geoId),
                          _geoPath->geos().at(input._geoId),
                          *_taxName));
    _paymentDetail->getMutableTaxPointsProperties() = _properties;

    _carrierFlightRule.reset(new CarrierFlightRule(type::Index(input._before),
        type::Index(input._after), *_taxPointTag, *_vendor));
    initFligthsAndApplicator();
    CPPUNIT_ASSERT_EQUAL(output, _applicator->apply(*_paymentDetail));
  }

private:
  static std::vector<FlightNumberInput> _beforeAndAfterInputs;
  static std::vector<bool> _beforeAndAfterOutputs;
  static size_t _beforeAndAfterCounter;

  static void initRuleMatchBeforeAndAfter()
  {
    using namespace boost::assign;

    _beforeAndAfterCounter = 0u;

    std::vector<type::FlightNumber> flt = list_of(1005)(1005)(1001)(1001)
                                                 (1006)(1006)(1002)(1002)
                                                 (1001)(1001)(1002)(1002)
                                                 (990)(990)(800)(800);

    _beforeAndAfterInputs.clear();
    _beforeAndAfterInputs += FlightNumberInput(3, 1, 2, flt);
    _beforeAndAfterInputs += FlightNumberInput(3, 2, 1, flt);
    _beforeAndAfterInputs += FlightNumberInput(3, 3, 3, flt);
    _beforeAndAfterInputs += FlightNumberInput(3, 3, 4, flt);
    _beforeAndAfterInputs += FlightNumberInput(3, 4, 3, flt);

    std::vector<bool> outs = list_of(true)(false)(true)(false)(false);
    _beforeAndAfterOutputs = outs;
  }

public:
  void testRuleMatchBeforeAndAfterCommon(const FlightNumberInput& input, bool output)
  {
    for (size_t i = 0u; i < _carrierFlightServiceServer->carrierFlights().size(); ++i)
    {
      _carrierFlightServiceServer->carrierFlights()[i].segments[0].marketingCarrier = "BA";
      _carrierFlightServiceServer->carrierFlights()[i].segments[0].operatingCarrier = type::CarrierCode(UninitializedCode);
      _carrierFlightServiceServer->carrierFlights()[i].segments[1].marketingCarrier = "BA";
      _carrierFlightServiceServer->carrierFlights()[i].segments[1].operatingCarrier = type::CarrierCode(UninitializedCode);
    }

    for (size_t i = 0; i < _carrierFlightServiceServer->carrierFlights().size()/4; ++i)
    {
      _carrierFlightServiceServer->carrierFlights()[i].segments[0].flightFrom = input._flights[4*i];
      _carrierFlightServiceServer->carrierFlights()[i].segments[0].flightTo = input._flights[4*i+1];
      _carrierFlightServiceServer->carrierFlights()[i].segments[1].flightFrom = input._flights[4*i+2];
      _carrierFlightServiceServer->carrierFlights()[i].segments[1].flightTo = input._flights[4*i+3];
    }

    TaxableUnitTagSet noUnits = TaxableUnitTagSet::none();
    _paymentDetail.reset(
        new PaymentDetail(PaymentRuleData(*_seqNo, *_ticketedPointTag, noUnits, 0, noCurrency,
                            type::TaxAppliesToTagInd::Blank),
                          _geoPath->geos().at(input._geoId),
                          _geoPath->geos().at(input._geoId),
                          *_taxName));
    _paymentDetail->getMutableTaxPointsProperties() = _properties;

    _carrierFlightRule.reset(new CarrierFlightRule(type::Index(input._before),
        type::Index(input._after), *_taxPointTag, *_vendor));
    initFligthsAndApplicator();
    CPPUNIT_ASSERT_EQUAL(output, _applicator->apply(*_paymentDetail));
  }

  void testRuleMatchBeforeAndAfter()
  {
    const FlightNumberInput& input = _beforeAndAfterInputs[_beforeAndAfterCounter];
    bool output = _beforeAndAfterOutputs[_beforeAndAfterCounter];
    _beforeAndAfterCounter++;

    testRuleMatchBeforeAndAfterCommon(input, output);
  }

private:
  static std::vector<FlightNumberInput> _beforeAndAfterForArrivalInputs;
  static std::vector<bool> _beforeAndAfterForArrivalOutputs;
  static size_t _beforeAndAfterCounterForArrival;

  static void initRuleMatchBeforeAndAfterForArrival()
  {
    using namespace boost::assign;

    _beforeAndAfterCounterForArrival = 0u;

    std::vector<type::FlightNumber> flt = list_of(1005)(1005)(1001)(1001)
                                                 (1006)(1006)(1002)(1002)
                                                 (1001)(1001)(1002)(1002)
                                                 (990)(990)(800)(800);

    _beforeAndAfterForArrivalInputs.clear();
    _beforeAndAfterForArrivalInputs += FlightNumberInput(3, 1, 2, flt);
    _beforeAndAfterForArrivalInputs += FlightNumberInput(3, 2, 1, flt);
    _beforeAndAfterForArrivalInputs += FlightNumberInput(3, 3, 3, flt);
    _beforeAndAfterForArrivalInputs += FlightNumberInput(3, 3, 4, flt);
    _beforeAndAfterForArrivalInputs += FlightNumberInput(3, 4, 3, flt);

    std::vector<bool> outs = list_of(false)(true)(true)(false)(false);
    _beforeAndAfterForArrivalOutputs = outs;
  }

public:

  void testRuleMatchBeforeAndAfterForArrival()
  {
    _taxPointTag.reset(new type::TaxPointTag(type::TaxPointTag::Arrival));

    const FlightNumberInput& input =
        _beforeAndAfterForArrivalInputs[_beforeAndAfterCounterForArrival];
    bool output = _beforeAndAfterForArrivalOutputs[_beforeAndAfterCounterForArrival];
    _beforeAndAfterCounterForArrival++;

    testRuleMatchBeforeAndAfterCommon(input, output);
  }

private:
  void initOCFees(type::FlightNumber fltFrom, type::FlightNumber fltTo,
		  type::FlightNumber before, type::FlightNumber after)
  {
    _carrierFlightServiceServer->carrierFlights()[0].segments[0].marketingCarrier = "BA";
    _carrierFlightServiceServer->carrierFlights()[0].segments[0].operatingCarrier = type::CarrierCode(UninitializedCode);
    _carrierFlightServiceServer->carrierFlights()[0].segments[0].flightFrom = fltFrom;
    _carrierFlightServiceServer->carrierFlights()[0].segments[0].flightTo = fltTo;

    _taxPointTag.reset(new type::TaxPointTag(type::TaxPointTag::Arrival));
    _carrierFlightRule.reset(new CarrierFlightRule(type::Index(before),
        type::Index(after), *_taxPointTag, *_vendor));

    initFligthsAndApplicator();

    TaxableUnitTagSet noUnits = TaxableUnitTagSet::none();
    _paymentDetail.reset(
        new PaymentDetail(PaymentRuleData(*_seqNo, *_ticketedPointTag, noUnits, 0, noCurrency,
                            type::TaxAppliesToTagInd::Blank),
                          _geoPath->geos().at(3),
                          _geoPath->geos().at(3),
                          *_taxName));
    _paymentDetail->getMutableTaxPointsProperties() = _properties;
  }
public:
  void applicableWithOCFees0()
  {
    initOCFees(1000, 1000, 0, 1);

    addOptionalService(type::OptionalServiceTag::TicketRelated, 3);
    addOptionalService(type::OptionalServiceTag::FlightRelated, 3);

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail)); // 1 OC matched
    CPPUNIT_ASSERT(!_paymentDetail->optionalServiceItems().at(0).isFailed());
    CPPUNIT_ASSERT(_paymentDetail->optionalServiceItems().at(1).isFailed());
    CPPUNIT_ASSERT(_paymentDetail->getItineraryDetail().isFailedRule());
    CPPUNIT_ASSERT(!_paymentDetail->isFailed());
  }

  void applicableWithOCFees1()
  {
    initOCFees(1000, 1999, 0, 1);

    addOptionalService(type::OptionalServiceTag::TicketRelated, 3);
    addOptionalService(type::OptionalServiceTag::FlightRelated, 3);

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail));
    CPPUNIT_ASSERT(!_paymentDetail->optionalServiceItems().at(0).isFailed());
    CPPUNIT_ASSERT(!_paymentDetail->optionalServiceItems().at(1).isFailed());
    CPPUNIT_ASSERT(!_paymentDetail->getItineraryDetail().isFailedRule());
    CPPUNIT_ASSERT(!_paymentDetail->isFailed());
  }

  void notApplicableWithOCFees()
  {
    initOCFees(1000, 1000, 0, 1);

    addOptionalService(type::OptionalServiceTag::FlightRelated, 1);
    addOptionalService(type::OptionalServiceTag::FlightRelated, 3);

    CPPUNIT_ASSERT_EQUAL(false, _applicator->apply(*_paymentDetail));
  }

  void applicableWithBaggage0()
  {
    initOCFees(1000, 1000, 0, 1);

    addOptionalService(type::OptionalServiceTag::TicketRelated, 3);
    addOptionalService(type::OptionalServiceTag::BaggageCharge, 3);

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail)); // 1 OC matched
    CPPUNIT_ASSERT(!_paymentDetail->optionalServiceItems().at(0).isFailed());
    CPPUNIT_ASSERT(_paymentDetail->optionalServiceItems().at(1).isFailed());
    CPPUNIT_ASSERT(_paymentDetail->getItineraryDetail().isFailedRule());
    CPPUNIT_ASSERT(!_paymentDetail->isFailed());
  }

  void applicableWithBaggage1()
  {
    initOCFees(1000, 1999, 0, 1);

    addOptionalService(type::OptionalServiceTag::TicketRelated, 3);
    addOptionalService(type::OptionalServiceTag::BaggageCharge, 3);

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail));
    CPPUNIT_ASSERT(!_paymentDetail->optionalServiceItems().at(0).isFailed());
    CPPUNIT_ASSERT(!_paymentDetail->optionalServiceItems().at(1).isFailed());
    CPPUNIT_ASSERT(!_paymentDetail->getItineraryDetail().isFailedRule());
    CPPUNIT_ASSERT(!_paymentDetail->isFailed());
  }

  void notApplicableWithBaggage()
  {
    initOCFees(1000, 1000, 0, 1);

    addOptionalService(type::OptionalServiceTag::BaggageCharge, 1);
    addOptionalService(type::OptionalServiceTag::BaggageCharge, 3);

    CPPUNIT_ASSERT_EQUAL(false, _applicator->apply(*_paymentDetail));
  }

private:
  std::unique_ptr<CarrierFlightRule> _carrierFlightRule;
  TaxName* _taxName;

  std::unique_ptr<type::Vendor> _vendor;
  std::unique_ptr<type::TaxPointTag> _taxPointTag;
  std::unique_ptr<CarrierFlightServiceServer> _carrierFlightServiceServer;
  std::unique_ptr<type::SeqNo> _seqNo;
  std::unique_ptr<type::TicketedPointTag> _ticketedPointTag;
  std::unique_ptr<boost::ptr_vector<Flight>> _flights;
  std::unique_ptr<std::vector<FlightUsage>> _flightUsages;
  std::unique_ptr<PaymentDetail> _paymentDetail;
  std::unique_ptr<GeoPath> _geoPath;
  std::unique_ptr<CarrierFlightApplicator> _applicator;
  std::shared_ptr<const CarrierFlight> _flightBefore;
  std::shared_ptr<const CarrierFlight> _flightAfter;
  std::vector<TaxPointProperties> _properties;

  void initFligthsAndApplicator()
  {
    _flightBefore = _carrierFlightServiceServer->getCarrierFlight(
      *_vendor, _carrierFlightRule->carrierFlightItemBefore());
    _flightAfter = _carrierFlightServiceServer->getCarrierFlight(
      *_vendor, _carrierFlightRule->carrierFlightItemAfter());
    _applicator.reset(new CarrierFlightApplicator(*_carrierFlightRule, *_geoPath, *_flightUsages,
                                                  _flightBefore, _flightAfter));
  }

  void addOptionalService(const type::OptionalServiceTag& ocType, size_t geoId)
  {
    OptionalService* oc = new OptionalService;
    oc->type() = ocType;
    oc->setTaxPointBegin(_geoPath->geos().at(geoId));
    _paymentDetail->optionalServiceItems().push_back(oc);
  }
};

std::vector<CarrierFlightApplicatorTest::CommonInput>
CarrierFlightApplicatorTest::_ruleDoesNotExistInputs;

std::vector<bool> CarrierFlightApplicatorTest::_ruleDoesNotExistOutputs;
size_t CarrierFlightApplicatorTest::_ruleDoesNotExistCounter;


std::vector<CarrierFlightApplicatorTest::CommonInput>
CarrierFlightApplicatorTest::_carrierInputs;

std::vector<bool> CarrierFlightApplicatorTest::_carrierOutputs;
size_t CarrierFlightApplicatorTest::_carrierCounter;


std::vector<CarrierFlightApplicatorTest::FlightNumberInput>
CarrierFlightApplicatorTest::_flightsInputs;

std::vector<bool> CarrierFlightApplicatorTest::_flightsOutputs;
size_t CarrierFlightApplicatorTest::_flightsCounter;


std::vector<CarrierFlightApplicatorTest::FlightNumberInput>
CarrierFlightApplicatorTest::_beforeAndAfterInputs;

std::vector<bool> CarrierFlightApplicatorTest::_beforeAndAfterOutputs;
size_t CarrierFlightApplicatorTest::_beforeAndAfterCounter;


std::vector<CarrierFlightApplicatorTest::FlightNumberInput>
CarrierFlightApplicatorTest::_beforeAndAfterForArrivalInputs;

std::vector<bool> CarrierFlightApplicatorTest::_beforeAndAfterForArrivalOutputs;
size_t CarrierFlightApplicatorTest::_beforeAndAfterCounterForArrival;

CPPUNIT_TEST_SUITE_REGISTRATION(CarrierFlightApplicatorTest);
} // namespace tax
