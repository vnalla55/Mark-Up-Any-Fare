// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "DataModel/Services/CarrierApplication.h"
#include "DataModel/Services/CarrierFlight.h"
#include "DataModel/Services/PassengerTypeCode.h"
#include "DataModel/Services/RulesRecord.h"
#include "DataModel/Services/ServiceBaggage.h"
#include "DataModel/Services/ServiceFeeSecurity.h"
#include "ServiceInterfaces/LocService.h"
#include "TaxDisplay/Common/TaxDisplayRequest.h"
#include "TaxDisplay/Response/Line.h"
#include "TaxDisplay/Response/ResponseFormatter.h"
#include "TaxDisplay/ViewX1SequenceDetail.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/ReportingRecordServiceMock.h"
#include "test/TaxRoundingInfoServiceMock.h"

#include <gmock/gmock.h>

#include <ostream>

using testing::_;
using testing::Return;

namespace tax
{
namespace display
{
class LocServiceMock : public LocService
{
public:
  MOCK_CONST_METHOD1(getNation, type::Nation(const type::AirportCode&));
  MOCK_CONST_METHOD1(getNationByName, type::Nation(const type::NationName&));
  MOCK_CONST_METHOD1(getNationName, type::NationName(const type::Nation&));
  MOCK_CONST_METHOD1(getCityCode, type::CityCode(const type::AirportCode&));
  MOCK_CONST_METHOD1(getAlaskaZone, type::AlaskaZone(const type::AirportCode&));
  MOCK_CONST_METHOD1(getState, type::StateProvinceCode(const type::AirportCode&));
  MOCK_CONST_METHOD1(getCurrency, type::CurrencyCode(const type::AirportCode&));
  MOCK_CONST_METHOD3(isInLoc, bool(const type::AirportOrCityCode&,
                                   const LocZone&,
                                   const type::Vendor&));
  MOCK_CONST_METHOD3(matchPassengerLocation, bool(const tax::type::LocCode&,
                                                  const tax::LocZone&,
                                                  const tax::type::Vendor&));
};

std::ostream& operator<<(std::ostream& stream, const std::string& str)
{
  stream << "\"" << str << "\"";

  return stream;
}

class ViewX1SequenceDetailTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ViewX1SequenceDetailTest);

  CPPUNIT_TEST(testCategoryTravelApplication);
  CPPUNIT_TEST(testCategoryTaxPointSpecification);
  CPPUNIT_TEST(testCategoryTravelCarrierQualifyingTags);
  CPPUNIT_TEST(testCategoryServiceBaggageTax);
  CPPUNIT_TEST(testCategoryTravelSectorDetails);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _rulesRecord.reset(new RulesRecord());
    _locService.reset(new LocServiceMock());
    _roundingService.reset(new TaxRoundingInfoServiceMock());
    _request.reset(new TaxDisplayRequest());
    _formatter.reset(new ResponseFormatter());
    _view.reset(new ViewX1SequenceDetail(*_rulesRecord, *_locService, *_roundingService, *_request, *_formatter));
  }

  void tearDown() {}

  void testCategoryTravelApplication()
  {
    const std::vector<std::string> res =
        {
          "7-TRAVEL APPLICATION DETAILS:",
          "JOURNEY DEFINITIONS:",
          "JOURNEY-ENTIRE SELECTED ITINERARY",
          "JOURNEY ORIGIN-FIRST TICKETED POINT ON ITINEARY",
          "JOURNEY DESTINATION-LAST TIKCETED POINT ON ITINERARY",
          "JOURNEY TURNAROUND- (NOT APPLICABLE TO ONE-WAY JOURNEY)",
          "FURTHEST STOP OR FARE BREAK FROM JOURNEY ORIGIN",
          "RETURN TO ORIGIN:",
          "N-TRAVEL JOURNEY MUST NOT RETURN TO JOURNEY ORIGIN",
          ".",
          ".",
          "JOURNEY ORIGIN LOCATION 1:",
          "N-NATION: GALACTIC EMPIRE",
          ".",
          ".",
          "JOURNEY DESTINATION/TURNAROUND LOCATION 2:",
          "P-AIRPORT: COR",
          ".",
          ".",
          "JOURNEY LOCATIONS INCLUDE:",
          "A-IATA AREA: 1,2,3",
          ".",
          ".",
          "TRAVEL WHOLLY WITHIN LOCATIONS:",
          "S-STATE/PROVINCE: STATE",
          ".",
          ".",
          "JOURNEY INDICATOR:",
          "A-JOURNEY LOC2 IS DESTINATION POINT FOR ONE-WAY OR ROUND-TRIP OR OPEN-JAW JOURNEY",
          ".",
          ".",
          "."
        };

    _rulesRecord->rtnToOrig = type::RtnToOrig::NotReturnToOrigin;
    _rulesRecord->jrnyLocZone1 = LocZone(type::LocType::Nation, "GE");
    _rulesRecord->jrnyLocZone2 = LocZone(type::LocType::Airport, "COR");
    _rulesRecord->jrnyIncludes = LocZone(type::LocType::Area, "1,2,3");
    _rulesRecord->trvlWhollyWithin = LocZone(type::LocType::StateProvince, "STATE");
    _rulesRecord->jrnyInd = type::JrnyInd::JnyLoc2DestPointForOWOrRTOrOJ;

    EXPECT_CALL(*_locService, getNationName(_)).WillOnce(Return(type::NationName("GALACTIC EMPIRE")));

    _view->categoryTravelApplication();

    CPPUNIT_ASSERT_EQUAL(res.size(), _formatter->linesList().size());
    std::size_t i = 0;
    for (const Line& line : _formatter->linesList())
      CPPUNIT_ASSERT_EQUAL(res[i++], line.str());
  }

  void testCategoryTaxPointSpecification()
  {
    const std::vector<std::string> res =
      {"8-TAX POINT SPECIFICATION DETAILS:",
       "TAX POINT LOCATION 1 ADJACENT INTERNATIONAL/DOMESTIC INDICATOR:",
       "D-DOMESTIC STOPOVER. TRAVEL BETWEEN LOC1 AND STOPOVER POINT "
       "PRIOR IF DEPARTURE TAX/AFTER IF ARRIVAL TAX MUST BE DOMESTIC",
       ".",
       ".",
       "TAX POINT LOCATION 1 TRANSFER TYPE:",
       "C-ONLINE, SAME CARRIER WITH EQUIPMENT CHANGE, SAME FLIGHT NBR",
       ".",
       ".",
       "TAX POINT LOCATION 1 STOPOVER TAG:",
       "N-NOT FARE BREAK",
       ".",
       ".",
       "TAX POINT LOCATION 1:",
       "S-STATE/PROVINCE: STATE",
       ".",
       ".",
       "TAX POINT LOCATION 2 INTERNATIONAL/DOMESTIC INDICATOR:",
       "I-INTERNATIONAL. LOC2 MUST NOT BE IN SAME COUNTRY AS LOC1",
       ".",
       ".",
       "TAX POINT LOCATION 2 COMPARISON:",
       "X-LOC2 MUST BE A DIFFERENT CITY FROM THE POINT PRIOR TO LOC1 "
                        "FOR DEPARTURE TAX/AFTER LOC1 FOR ARRIVAL TAX",
       ".",
       ".",
       "TAX POINT LOCATION 2 STOPOVER TAG:",
       "S-LOC2 MUST BE FIRST STOPOVER POINT AFTER LOC1 FOR DEPARTURE "
                        "TAX/PRIOR TO LOC1 FOR ARRIVAL TAX",
       ".",
       ".",
       "TAX POINT LOCATION 2:",
       "P-AIRPORT: XYZ",
       ".",
       ".",
       "TAX POINT LOCATION 3 APPLICATION TAG:",
       "N-LOC3 MUST BE FIRST STOPOVER POINT AFTER LOC1 FOR DEPARTURE "
                        "TAX/PRIOR TO LOC1 FOR ARRIVAL TAX",
       ".",
       ".",
       "TAX POINT LOCATION 3:",
       "A-IATA AREA: 1,2,3",
       ".",
       ".",
       "."};

    _rulesRecord->taxPointLoc1IntlDomInd = type::AdjacentIntlDomInd::AdjacentStopoverDomestic;
    _rulesRecord->taxPointLoc1TransferType = type::TransferTypeTag::OnlineWithChangeOfGauge;
    _rulesRecord->taxPointLoc1StopoverTag = type::StopoverTag::NotFareBreak;
    _rulesRecord->taxPointLocZone1 = LocZone(type::LocType::StateProvince, "STATE");
    _rulesRecord->taxPointLoc2IntlDomInd = type::IntlDomInd::International;
    _rulesRecord->taxPointLoc2Compare = type::TaxPointLoc2Compare::Point;
    _rulesRecord->taxPointLoc2StopoverTag = type::Loc2StopoverTag::Stopover;
    _rulesRecord->taxPointLocZone2 = LocZone(type::LocType::Airport, "XYZ");
    _rulesRecord->taxPointLoc3GeoType = type::TaxPointLoc3GeoType::Stopover;
    _rulesRecord->taxPointLocZone3 = LocZone(type::LocType::Area, "1,2,3");

    _view->categoryTaxPointSpecification();

    CPPUNIT_ASSERT_EQUAL(res.size(), _formatter->linesList().size());
    std::size_t i = 0;
    for (const Line& line : _formatter->linesList())
      CPPUNIT_ASSERT_EQUAL(res[i++], line.str());
  }

  void testCategoryTravelCarrierQualifyingTags()
  {
    _rulesRecord->stopoverTimeTag = "12";
    _rulesRecord->stopoverTimeUnit = type::StopoverTimeUnit::Hours;

    CarrierApplicationEntry* applicationEntry1 = new CarrierApplicationEntry();
    applicationEntry1->carrier = "CA";
    applicationEntry1->applind = type::CarrierApplicationIndicator::Negative;
    CarrierApplicationEntry* applicationEntry2 = new CarrierApplicationEntry();
    applicationEntry2->carrier = "CB";
    applicationEntry2->applind = type::CarrierApplicationIndicator::Positive;
    CarrierApplicationEntry* applicationEntry3 = new CarrierApplicationEntry();
    applicationEntry3->carrier = "CC";
    applicationEntry3->applind = type::CarrierApplicationIndicator::Positive;
    CarrierApplicationEntry* applicationEntry4 = new CarrierApplicationEntry();
    applicationEntry4->carrier = "$$"; // all cxrs
    applicationEntry4->applind = type::CarrierApplicationIndicator::Negative;
    CarrierApplicationEntry* applicationEntry5 = new CarrierApplicationEntry();
    applicationEntry5->carrier = "CF";
    applicationEntry5->applind = type::CarrierApplicationIndicator::Negative;
    std::shared_ptr<CarrierApplication> carrierApplication = std::make_shared<CarrierApplication>();
    carrierApplication->entries.push_back(applicationEntry1);
    carrierApplication->entries.push_back(applicationEntry2);
    carrierApplication->entries.push_back(applicationEntry3);
    carrierApplication->entries.push_back(applicationEntry4);
    carrierApplication->entries.push_back(applicationEntry5);

    CarrierFlightSegment* flightSegment1 = new CarrierFlightSegment();
    flightSegment1->marketingCarrier = "M5";
    flightSegment1->operatingCarrier = "M5";
    flightSegment1->flightFrom = 100;
    flightSegment1->flightTo = 499;
    CarrierFlightSegment* flightSegment2 = new CarrierFlightSegment();
    flightSegment2->marketingCarrier = "M5";
    flightSegment2->operatingCarrier = "AA";
    flightSegment2->flightFrom = 700;
    flightSegment2->flightTo = 700;
    CarrierFlightSegment* flightSegment3 = new CarrierFlightSegment();
    flightSegment3->marketingCarrier = "M5";
    // flightSegment3->operatingCarrier = blank
    flightSegment3->flightFrom = 800;
    flightSegment3->flightTo = 999;
    CarrierFlightSegment* flightSegment4 = new CarrierFlightSegment();
    flightSegment4->marketingCarrier = "M6";
    flightSegment4->operatingCarrier = "M6";
    flightSegment4->flightFrom = -1;
    flightSegment4->flightTo = tax::MAX_FLIGHT_NUMBER;
    std::shared_ptr<CarrierFlight> carrierFlight = std::make_shared<CarrierFlight>();
    carrierFlight->segments.push_back(flightSegment1);
    carrierFlight->segments.push_back(flightSegment2);
    carrierFlight->segments.push_back(flightSegment3);
    carrierFlight->segments.push_back(flightSegment4);

    PassengerTypeCodeItem* passengerItem1 = new PassengerTypeCodeItem();
    passengerItem1->applTag = type::PtcApplTag::NotPermitted;
    passengerItem1->passengerType = "VAC";
    passengerItem1->minimumAge = -1;
    passengerItem1->maximumAge = -1;
    passengerItem1->statusTag = type::PassengerStatusTag::Blank;
    passengerItem1->location = LocZone();
    passengerItem1->matchIndicator = type::PtcMatchIndicator::Input;
    PassengerTypeCodeItem* passengerItem2 = new PassengerTypeCodeItem();
    passengerItem2->applTag = type::PtcApplTag::NotPermitted;
    passengerItem2->passengerType = "VAC";
    passengerItem2->minimumAge = -1;
    passengerItem2->maximumAge = -1;
    passengerItem2->statusTag = type::PassengerStatusTag::Blank;
    passengerItem2->location = LocZone();
    passengerItem2->matchIndicator = type::PtcMatchIndicator::Output;
    PassengerTypeCodeItem* passengerItem3 = new PassengerTypeCodeItem();
    passengerItem3->applTag = type::PtcApplTag::Permitted;
    passengerItem3->passengerType = "INF";
    passengerItem3->minimumAge = -1;
    passengerItem3->maximumAge = 1;
    passengerItem3->statusTag = type::PassengerStatusTag::Blank;
    passengerItem3->location = LocZone(type::LocType::Area, "2");
    passengerItem3->matchIndicator = type::PtcMatchIndicator::Input;
    PassengerTypeCodeItem* passengerItem4 = new PassengerTypeCodeItem();
    passengerItem4->applTag = type::PtcApplTag::Permitted;
    passengerItem4->passengerType = "FNF";
    passengerItem4->minimumAge = -1;
    passengerItem4->maximumAge = 1;
    passengerItem4->statusTag = type::PassengerStatusTag::Blank;
    passengerItem4->location = LocZone(type::LocType::City, "KRK");
    passengerItem4->matchIndicator = type::PtcMatchIndicator::Input;
    PassengerTypeCodeItem* passengerItem5 = new PassengerTypeCodeItem();
    passengerItem5->applTag = type::PtcApplTag::Permitted;
    passengerItem5->passengerType = "GIF";
    passengerItem5->minimumAge = -1;
    passengerItem5->maximumAge = 1;
    passengerItem5->statusTag = type::PassengerStatusTag::Blank;
    passengerItem5->location = LocZone();
    passengerItem5->matchIndicator = type::PtcMatchIndicator::Input;
    PassengerTypeCodeItem* passengerItem6 = new PassengerTypeCodeItem();
    passengerItem6->applTag = type::PtcApplTag::Permitted;
    // passengerItem6->passengerType = blank
    passengerItem6->minimumAge = 3;
    passengerItem6->maximumAge = -1;
    passengerItem6->statusTag = type::PassengerStatusTag::Resident;
    passengerItem6->location = LocZone();
    passengerItem6->matchIndicator = type::PtcMatchIndicator::Input;
    std::shared_ptr<PassengerTypeCodeItems> passengerTypeCodeItems = std::make_shared<PassengerTypeCodeItems>();
    passengerTypeCodeItems->push_back(passengerItem1);
    passengerTypeCodeItems->push_back(passengerItem2);
    passengerTypeCodeItems->push_back(passengerItem3);
    passengerTypeCodeItems->push_back(passengerItem4);
    passengerTypeCodeItems->push_back(passengerItem5);
    passengerTypeCodeItems->push_back(passengerItem6);

    _request->userType = TaxDisplayRequest::UserType::SABRE;

    const std::vector<std::string> res =
      {
        "9-TRAVEL/CARRIER QUALIFYING TAG DETAILS:",
        "",
        "STOPOVER TIME/UNIT:",
        "012 HOURS (H)",
        ".",
        ".",
        "VALIDATING CARRIER APPLICATION (TABLE 190):",
        "PERMITTED ON: CB, CC",
        "EXCEPT CARRIERS: ALL CARRIERS",
        ".",
        ".",
        "CARRIER FLIGHT APPLICATION 1 (TABLE 186):",
        "FLIGHT INTO LOC1 FOR DEPARTURE/FLIGHT OUT OF LOC1 FOR ARRIVAL",
        "MARKETING/OPERATING FLIGHT 1-THROUGH-FLIGHT 2",
        "CARRIER  /CARRIER",
        "M5       /M5        0100             0499",
        "M5       /AA        0700                 ",
        "M5       /          0800             0999",
        "M6       /M6                             ",
        ".",
        ".",
        "CARRIER FLIGHT APPLICATION 2 (TABLE 186):",
        "FLIGHT INTO LOC1 FOR DEPARTURE/FLIGHT OUT OF LOC1 FOR ARRIVAL",
        "MARKETING/OPERATING FLIGHT 1-THROUGH-FLIGHT 2",
        "CARRIER  /CARRIER",
        "M5       /M5        0100             0499",
        "M5       /AA        0700                 ",
        "M5       /          0800             0999",
        "M6       /M6                             ",
        ".",
        ".",
        "PASSENGER TYPE APPLICATION (TABLE 169):",
        "PERMIT - TYPE - MIN - MAX - PTC MATCH   -    PASSENGER",
        "Y/N      CODE   AGE   AGE   TYPE             STATUS",
        "NO     - VAC  -     -     - I-INPUT PTC -    ",
        "NO     - VAC  -     -     - O-FARE PTC  -    ",
        "YES    - INF  -     -  1  - I-INPUT PTC -    ",
        "LOCATION: A-IATA AREA: 2",
        "YES    - FNF  -     -  1  - I-INPUT PTC -    ",
        "LOCATION: C-CITY: KRK",
        "YES    - GIF  -     -  1  - I-INPUT PTC -    ",
        "YES    -      -  3  -     - I-INPUT PTC -    R-RESIDENT",
        ".",
        ".",
        "."
      };

    _view->setTravelCarrierQualifyingTagsData(carrierApplication, carrierFlight, carrierFlight, passengerTypeCodeItems);
    _view->categoryTravelCarrierQualifyingTags();

    CPPUNIT_ASSERT_EQUAL(res.size(), _formatter->linesList().size());
    std::size_t i = 0;
    for (const Line& line : _formatter->linesList())
      CPPUNIT_ASSERT_EQUAL(res[i++], line.str());
  }

  void testCategoryServiceBaggageTax()
  {
    _request->userType = TaxDisplayRequest::UserType::SABRE;
    ServiceFeeSecurityItem* securityItem1 = new ServiceFeeSecurityItem();
    securityItem1->travelAgencyIndicator = type::TravelAgencyIndicator::Blank;
    securityItem1->carrierGdsCode = "AA";
    securityItem1->dutyFunctionCode = "GN";
    securityItem1->codeType = type::CodeType::AirlineSpec;
    securityItem1->code = "RES";
    securityItem1->viewBookTktInd = type::ViewBookTktInd::ViewBookTkt;
    securityItem1->location = LocZone(type::LocType::City, "TUL");
    ServiceFeeSecurityItem* securityItem2 = new ServiceFeeSecurityItem();
    securityItem2->travelAgencyIndicator = type::TravelAgencyIndicator::Agency;
    // securityItem2->carrierGdsCode = blank
    securityItem2->dutyFunctionCode = "BG";
    securityItem2->codeType = type::CodeType::ElecResServProvider;
    securityItem2->code = "12345678";
    securityItem2->viewBookTktInd = type::ViewBookTktInd::ViewOnly;
    ServiceFeeSecurityItem* securityItem3 = new ServiceFeeSecurityItem();
    securityItem3->travelAgencyIndicator = type::TravelAgencyIndicator::Agency;
    securityItem3->carrierGdsCode = "1S";
    securityItem3->dutyFunctionCode = "KB";
    securityItem3->codeType = type::CodeType::AgencyNumber;
    securityItem3->code = "87654321";
    securityItem3->viewBookTktInd = type::ViewBookTktInd::ViewBookTkt;
    ServiceFeeSecurityItem* securityItem4 = new ServiceFeeSecurityItem();
    securityItem4->travelAgencyIndicator = type::TravelAgencyIndicator::Agency;
    securityItem4->carrierGdsCode = "1S";
    securityItem4->dutyFunctionCode = "LS";
    securityItem4->codeType = type::CodeType::LNIATA;
    securityItem4->code = "123456";
    securityItem4->viewBookTktInd = type::ViewBookTktInd::ViewOnly;
    ServiceFeeSecurityItem* securityItem5 = new ServiceFeeSecurityItem();
    securityItem5->travelAgencyIndicator = type::TravelAgencyIndicator::Agency;
    securityItem5->carrierGdsCode = "1S";
    securityItem5->dutyFunctionCode = "BH";
    securityItem5->codeType = type::CodeType::AgencyPCC;
    securityItem5->code = "A0A0";
    securityItem5->viewBookTktInd = type::ViewBookTktInd::ViewBookTkt;
    std::shared_ptr<ServiceFeeSecurityItems> securityItems = std::make_shared<ServiceFeeSecurityItems>();
    securityItems->push_back(securityItem1);
    securityItems->push_back(securityItem2);
    securityItems->push_back(securityItem3);
    securityItems->push_back(securityItem4);
    securityItems->push_back(securityItem5);

    _rulesRecord->serviceBaggageApplTag = type::ServiceBaggageApplTag::E;


    ServiceBaggageEntry* baggageEntry1 = new ServiceBaggageEntry();
    baggageEntry1->applTag = type::ServiceBaggageAppl::Positive;
    baggageEntry1->taxCode = "YQ";
    ServiceBaggageEntry* baggageEntry2 = new ServiceBaggageEntry();
    baggageEntry2->applTag = type::ServiceBaggageAppl::Negative;
    baggageEntry2->taxCode = "YR";
    ServiceBaggageEntry* baggageEntry3 = new ServiceBaggageEntry();
    baggageEntry3->applTag = type::ServiceBaggageAppl::Positive;
    baggageEntry3->feeOwnerCarrier = "SK";
    baggageEntry3->taxCode = "OC";
    baggageEntry3->taxTypeSubcode = "OK9";
    baggageEntry3->optionalServiceTag = type::OptionalServiceTag::FlightRelated;
    baggageEntry3->group = "BG";
    baggageEntry3->subGroup = "SP";
    std::shared_ptr<ServiceBaggage> serviceBaggage = std::make_shared<ServiceBaggage>();
    serviceBaggage->entries.push_back(baggageEntry1);
    serviceBaggage->entries.push_back(baggageEntry2);
    serviceBaggage->entries.push_back(baggageEntry3);

    const std::vector<std::string> res =
      {
        "10-SERVICE/BAGGAGE TAX DETAILS:",
        "SERVICE FEE SECURITY APPLICATION (TABLE 183):",
        "SEQ TVL CXR DUTY CODE                  CODE     VIEW/BOOK/TKT",
        "NO. AGT GDS FUNC TYPE",
        "001     AA   GN  A-AIRLINE             RES      1-VIEW/BOOK/TKT",
        "LOCATION",
        "C-CITY-TUL",
        "002  X       BG  E-ERSP NO.            12345678 2-VIEW ONLY",
        "003  X  1S   KB  I-IATA TVL AGENCY     87654321 1-VIEW/BOOK/TKT",
        "004  X  1S   LS  L-LNIATA-CRT NO.      123456   2-VIEW ONLY",
        "005  X  1S   BH  T-PSEUDO/TVL AGT      A0A0     1-VIEW/BOOK/TKT",
        ".",
        ".",
        "SERVICE/BAGGAGE APPLICATION TAG:",
        "E-MAY PROCESS AGAINST DATA IN TAXABLE UNIT TAGS AND SERVICE/BAGGAGE APPLICATION TABLE",
        ".",
        ".",
        "SERVICE/BAGGAGE APPLICATION (TABLE 168):",
        "SEQ APPLY  FEE TAX  TAX/TYPE SERVICE TYPE        ATTRIBUTES",
        "NBR Y/N    CXR CODE SUB/CODE                     GRP - SUB",
        "001 Y-YES      YQ                                      ",
        "002 N-NO       YR                                      ",
        "003 Y-YES  SK  OC   OK9      F-FLIGHT RELATED    BG    SP",
        ".",
        ".",
        "."
      };

    _view->setServiceBaggageData(securityItems, serviceBaggage);
    _view->categoryServiceBaggageTax();

    CPPUNIT_ASSERT_EQUAL(res.size(), _formatter->linesList().size());
    std::size_t i = 0;
    for (const Line& line : _formatter->linesList())
      CPPUNIT_ASSERT_EQUAL(res[i++], line.str());
  }

  void testCategoryTravelSectorDetails()
  {
    const std::vector<std::string> res =
      {
          "11-SECTOR DETAILS:",
          "SECTOR DETAIL APPLICATION TAG:",
          "B-MATCH ANY SECTOR BETWEEN JOUNEY ORIGIN AND DESTINATION",
          ".",
          ".",
          "SECTOR DETAIL APPLICATION (TABLE 167):",
          "SEQ  APPLY  FIELD: VALUE",
          "001  X-NO   EQUIP: BUS",
          "002  X-NO   EQUIP: TRN",
          "003  X-NO   RBD: R/U, ASD",
                      "FARE OWNING CARRIER: JT",
                      "RULE TARIFF: 002",
          "004  Y-YES  CABIN CODE: Y-ECONOMY",
                      "FARE TYPE: ER",
                      "TARIFF TYPE: PUBLIC",
                      "RULE NBR: 1000",
                      "TICKET CODE: CD-SENIOR CITIZEN",
          ".",
          ".",
          "."
      };

    _rulesRecord->sectorDetailApplTag = type::SectorDetailApplTag::AnySectorOrginDestination;
    SectorDetailEntry* entry1 = new SectorDetailEntry();
    entry1->applTag = type::SectorDetailAppl::Negative;
    entry1->equipmentCode = "BUS";
    SectorDetailEntry* entry2 = new SectorDetailEntry();
    entry2->applTag = type::SectorDetailAppl::Negative;
    entry2->equipmentCode = "TRN";
    SectorDetailEntry* entry3 = new SectorDetailEntry();
    entry3->applTag = type::SectorDetailAppl::Negative;
    entry3->reservationCodes.push_back("R/U");
    entry3->reservationCodes.push_back("ASD");
    entry3->fareOwnerCarrier = "JT";
    entry3->tariff = Tariff(2, type::FareTariffInd());
    SectorDetailEntry* entry4 = new SectorDetailEntry();
    entry4->applTag = type::SectorDetailAppl::Positive;
    entry4->cabinCode = type::CabinCode::Economy;
    entry4->fareType = "ER";
    entry4->tariff = Tariff(-1, "PUB");
    entry4->fareOwnerCarrier = "JT";
    entry4->rule = "1000";
    entry4->ticketCode = "TICKET CODE: CD-SENIOR CITIZEN";
    auto sectorDetail = std::make_shared<SectorDetail>();
    sectorDetail->entries.push_back(entry1);
    sectorDetail->entries.push_back(entry2);
    sectorDetail->entries.push_back(entry3);
    sectorDetail->entries.push_back(entry4);

    //    CPPUNIT_ASSERT_EQUAL(res.size(), _formatter->linesList().size());
    std::size_t i = 0;
    for (const Line& line : _formatter->linesList())
      CPPUNIT_ASSERT_EQUAL(res[i++], line.str());
  }

private:
  std::unique_ptr<RulesRecord> _rulesRecord;
  std::unique_ptr<LocServiceMock> _locService;
  std::unique_ptr<TaxRoundingInfoServiceMock> _roundingService;
  std::unique_ptr<TaxDisplayRequest> _request;
  std::unique_ptr<ResponseFormatter> _formatter;
  std::unique_ptr<ViewX1SequenceDetail> _view;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ViewX1SequenceDetailTest);
} /* namespace display */
} /* namespace tax */
