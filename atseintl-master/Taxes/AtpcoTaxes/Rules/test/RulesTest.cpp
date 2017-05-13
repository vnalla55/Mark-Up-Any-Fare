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
#include <stdexcept>
#include "test/include/CppUnitHelperMacros.h"

#include "Common/LocZone.h"
#include "DomainDataObjects/Request.h"
#include "ServiceInterfaces/DefaultServices.h"
#include "DataModel/Services/PassengerTypeCode.h"
#include "Rules/AlternateRefAkHiFactorsRule.h"
#include "Rules/BusinessRuleApplicator.h"
#include "Rules/CurrencyOfSaleRule.h"
#include "Rules/ExemptTagRule.h"
#include "Rules/FlatTaxRule.h"
#include "Rules/JourneyIncludesRule.h"
#include "Rules/JourneyLoc1AsOriginRule.h"
#include "Rules/TravelWhollyWithinRule.h"
#include "Rules/TaxPointLoc1StopoverTagRule.h"
#include "Rules/PassengerTypeCodeRule.h"
#include "Rules/ReturnToOriginRule.h"
#include "Rules/TaxMinMaxValueRule.h"
#include "Rules/TaxPointLoc1Rule.h"
#include "Rules/TaxPointLoc1TransferTypeRule.h"
#include "Rules/TaxPointLoc3AsPreviousPointRule.h"
#include "Rules/TicketedPointRule.h"
#include "TestServer/Facades/AKHIFactorServiceServer.h"
#include "TestServer/Facades/CurrencyServiceServer.h"
#include "TestServer/Facades/LocServiceServer.h"
#include "TestServer/Facades/PassengerTypesServiceServer.h"

namespace tax
{

class BusinessRuleApplicator;

class RulesTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RulesTest);

  CPPUNIT_TEST(testTaxPointLoc1Rule);
  CPPUNIT_TEST(testExemptTagRule);
  CPPUNIT_TEST(testJourneyLoc1AsOriginRule);
  CPPUNIT_TEST(testTravelWhollyWithinRule);
  CPPUNIT_TEST(testJourneyIncludesRule);
  CPPUNIT_TEST(testReturnToOriginRule);
  CPPUNIT_TEST(testTicketedPointRule);
  CPPUNIT_TEST(testTaxPointLoc1TransferTypeRule);
  CPPUNIT_TEST(testTaxPointLoc1StopoverTagRule);
  CPPUNIT_TEST(testTaxPointLoc3AsPreviousPointRule);
  CPPUNIT_TEST(testCurrencyOfSaleRule);
  CPPUNIT_TEST(testFlatTaxRule);
  CPPUNIT_TEST(testTaxMinMaxValueRule);
  CPPUNIT_TEST(testPassengerTypeCodeRule);
  CPPUNIT_TEST(testAlternateRefAkHiFactorsRule);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    type::Index index(0);
    _request.allItins().push_back(Itin());
    _request.itins().push_back(&_request.allItins().back());
    _request.allItins()[index].geoPathRefId() = index;
    _request.geoPaths().push_back(GeoPath());
  }

  void tearDown()
  {
  }

  void testTaxPointLoc1Rule()
  {
    const LocZone locZone;
    const type::Vendor vendor;

    TaxPointLoc1Rule taxPointLoc1Rule(locZone, vendor);
    testBRGetDescription(taxPointLoc1Rule);
  }

  void testExemptTagRule()
  {
    ExemptTagRule exemptTagRuleRule;
    testBRGetDescription(exemptTagRuleRule);
  }

  void testJourneyLoc1AsOriginRule()
  {
    LocZone locZone;
    locZone.type() = type::LocType::Area;
    type::Vendor vendor;

    JourneyLoc1AsOriginRule journeyRule(locZone, vendor);
    testBRGetDescription(journeyRule);
  }

  void testTravelWhollyWithinRule()
  {
    type::TicketedPointTag ticketedPoint(type::TicketedPointTag::MatchTicketedAndUnticketedPoints);
    LocZone locZone;
    locZone.type() = type::LocType::Area;
    type::Vendor vendor;

    TravelWhollyWithinRule journeyRule(ticketedPoint, locZone, vendor);
    testBRGetDescription(journeyRule);
  }

  void testJourneyIncludesRule()
  {
    type::TicketedPointTag ticketedPoint(type::TicketedPointTag::MatchTicketedAndUnticketedPoints);
    LocZone locZone;
    locZone.type() = type::LocType::Area;
    type::Vendor vendor;

    JourneyIncludesRule journeyRule(ticketedPoint, locZone, vendor, false);
    testBRGetDescription(journeyRule);
  }

  void testReturnToOriginRule()
  {
    ReturnToOriginRule returnToOriginRule(type::RtnToOrig::ReturnToOrigin);
    testBRGetDescription(returnToOriginRule);
  }

  void testTicketedPointRule()
  {
    TicketedPointRule ticketedPointRule;
    testBRGetDescription(ticketedPointRule);
  }

  void testTaxPointLoc1TransferTypeRule()
  {
    type::TransferTypeTag transferTypeTag = type::TransferTypeTag::Interline;
    TaxPointLoc1TransferTypeRule taxPointLoc1TransferTypeRule(transferTypeTag);
    testBRGetDescription(taxPointLoc1TransferTypeRule);
  }

  void testTaxPointLoc1StopoverTagRule()
  {
    type::StopoverTag stopoverTag = type::StopoverTag::Connection;
    type::TicketedPointTag ticketedPoint = type::TicketedPointTag::MatchTicketedAndUnticketedPoints;

    TaxPointLoc1StopoverTagRule loc1StopoverTagRule(stopoverTag, ticketedPoint, false);
    testBRGetDescription(loc1StopoverTagRule);
  }

  void testTaxPointLoc3AsPreviousPointRule()
  {
    LocZone locZone;
    type::Vendor vendor;
    TaxPointLoc3AsPreviousPointRule taxPointLoc3AsPreviousPointRule(locZone, vendor);
    testBRGetDescription(taxPointLoc3AsPreviousPointRule);
  }

  void testCurrencyOfSaleRule()
  {
    const type::CurrencyCode currencyCode;
    CurrencyOfSaleRule currencyOfSaleRule(currencyCode);
    testBRGetDescription(currencyOfSaleRule);
  }

  void testFlatTaxRule()
  {
    TaxableUnitTagSet taxableUnitTags = TaxableUnitTagSet::none();
    FlatTaxRule flatTaxRule(taxableUnitTags);

    testBRGetDescription(flatTaxRule);
  }

  void testTaxMinMaxValueRule()
  {
    TaxMinMaxValueRule taxMinMaxValueRule(type::CurrencyCode(), int16_t(0), int16_t(0),
                                          type::CurDecimals());

    testBRGetDescription(taxMinMaxValueRule);
  }

  void testAlternateRefAkHiFactorsRule()
  {
    AlternateRefAkHiFactorsRule alternateRefAkHiFactorsRule;

    testBRGetDescription(alternateRefAkHiFactorsRule);
  }

  void testPassengerTypeCodeRule()
  {
    PassengerTypeCodeRule passengerTypeCodeRule(type::Vendor("ATP"), type::Index(0),
                                                type::TaxRemittanceId(type::TaxRemittanceId::Sale));

    testBRGetDescription(passengerTypeCodeRule);
  }

private:
  Request _request;
  DefaultServices _services;

  void testBRGetDescription(BusinessRule& businessRule)
  {
    DefaultServices services;
    boost::ptr_vector<PassengerTypeCode> ptcCache;
    services.setPassengerTypesService(new PassengerTypesServiceServer(ptcCache));
    const std::string description = businessRule.getDescription(services);
    CPPUNIT_ASSERT_MESSAGE("Description is empty", !description.empty());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(RulesTest);

} // namespace tax
