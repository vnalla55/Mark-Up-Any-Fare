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

#include "DataModel/Common/Types.h"
#include "DomainDataObjects/FarePath.h"
#include "DomainDataObjects/GeoPathMapping.h"
#include "DomainDataObjects/PointOfSale.h"
#include "Rules/ServiceFeeSecurityApplicator.h"
#include "Rules/ServiceFeeSecurityRule.h"
#include "TestServer/Facades/ServiceFeeSecurityServiceServer.h"
#include "test/PaymentDetailMock.h"
#include "test/LocServiceMock.h"

#include <memory>
#include <set>

namespace tax
{

namespace
{
  const type::LocZoneText noLoc (UninitializedCode);
  const type::PseudoCityCode nullPCC (UninitializedCode);
}

class ServiceFeeSecurityApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ServiceFeeSecurityApplicatorTest);
  CPPUNIT_TEST(dontMatchIfNoRowMatches);
  CPPUNIT_TEST(matchIfFirstRowMatches);
  CPPUNIT_TEST(matchIfMiddleRowMatches);
  CPPUNIT_TEST(matchIfLastRowMatches);

  CPPUNIT_TEST(matchAgencyTagTest);

  CPPUNIT_TEST(testCarrierMatches_CSR);
  CPPUNIT_TEST(testCarrierMatches_Carrier);
  CPPUNIT_TEST(testIsCrsCodeMatches);

  CPPUNIT_TEST(matchIfDutyEmpty);
  CPPUNIT_TEST(dontMatchIfNoAgentDutyInfo);
  CPPUNIT_TEST(matchIfDutyCodesAgree);
  CPPUNIT_TEST(matchIfDutyCodesAgreeWithMapping);
  CPPUNIT_TEST(dontMatchIfNoAgentFunctionInfo);
  CPPUNIT_TEST(matchIfFunctionCodesAgree);

  CPPUNIT_TEST(matchIfLocationEmpty);
  CPPUNIT_TEST(dontMatchIfAgentLocationEmpty);
  CPPUNIT_TEST(matchIfLocationMatches);

  CPPUNIT_TEST(matchCodeTypeBlank);
  CPPUNIT_TEST(dontMatchCodeTypesXVLA);
  CPPUNIT_TEST(testIsAirlineDepartmentMatches);
  CPPUNIT_TEST(matchCodeTypeT);
  CPPUNIT_TEST(matchCodeTypeI);
  CPPUNIT_TEST(matchCodeTypeE);

  CPPUNIT_TEST(matchIfViewBookTkt);
  CPPUNIT_TEST(dontMatchIfViewOnly);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _rule.reset(new ServiceFeeSecurityRule("ATP", 17));
    _items.reset(new ServiceFeeSecurityItems());
    _applicator.reset(new ServiceFeeSecurityApplicator(_rule.get(), _pointOfSale, _locServiceMock, _items));
    _pointOfSale.agentPcc() = tax::type::PseudoCityCode(tax::UninitializedCode);
    _pointOfSale.vendorCrsCode() = "1S"; // Sabre
    _pointOfSale.carrierCode() = "AA";
    _pointOfSale.agentDuty() = "0";
    _pointOfSale.agentFunction() = "H7N";
  }

  void tearDown()
  {
  }

  void addServiceFeeSecurityEntry(type::TravelAgencyIndicator tai,
                                  const type::CarrierGdsCode& carrierGdsCode,
                                  const std::string& dutyFunctionCode, const type::LocType& loctype,
                                  const type::LocZoneText& loc, const type::CodeType& codeType,
                                  const std::string& code,
                                  type::ViewBookTktInd viewBookTktInd = type::ViewBookTktInd::ViewBookTkt)
  {
    std::unique_ptr<ServiceFeeSecurityItem> item(new ServiceFeeSecurityItem);
    item->travelAgencyIndicator = tai;
    item->carrierGdsCode = carrierGdsCode;
    item->dutyFunctionCode = dutyFunctionCode;
    item->location.type() = loctype;
    item->location.code() = loc;
    item->codeType = codeType;
    item->code = code;
    item->viewBookTktInd = viewBookTktInd;
    _items->push_back(item.release());
  }

  void dontMatchIfNoRowMatches()
  {
    addServiceFeeSecurityEntry(type::TravelAgencyIndicator::Agency, "LH", "", type::LocType::Area,
                               noLoc, type::CodeType::Blank, "");
    addServiceFeeSecurityEntry(type::TravelAgencyIndicator::Agency, "AA", "", type::LocType::Area,
                               noLoc, type::CodeType::Blank, "");
    addServiceFeeSecurityEntry(type::TravelAgencyIndicator::Agency, "VA", "", type::LocType::Area,
                               noLoc, type::CodeType::Blank, "");
    addServiceFeeSecurityEntry(type::TravelAgencyIndicator::Agency, "BA", "", type::LocType::Area,
                               noLoc, type::CodeType::Blank, "");

    CPPUNIT_ASSERT(!_applicator->apply(_paymentDetail));
  }

  void matchIfFirstRowMatches()
  {
    addServiceFeeSecurityEntry(type::TravelAgencyIndicator::Agency, "1S", "", type::LocType::Blank,
                               noLoc, type::CodeType::Blank, "");
    addServiceFeeSecurityEntry(type::TravelAgencyIndicator::Agency, "AA", "", type::LocType::Area,
                               noLoc, type::CodeType::Blank, "");
    addServiceFeeSecurityEntry(type::TravelAgencyIndicator::Agency, "VA", "", type::LocType::Area,
                               noLoc, type::CodeType::Blank, "");
    addServiceFeeSecurityEntry(type::TravelAgencyIndicator::Agency, "BA", "", type::LocType::Area,
                               noLoc, type::CodeType::Blank, "");
    _pointOfSale.agentPcc() = "B4T0";
    _pointOfSale.loc() = "KRK";

    CPPUNIT_ASSERT(_applicator->apply(_paymentDetail));
  }

  void matchIfMiddleRowMatches()
  {
    addServiceFeeSecurityEntry(type::TravelAgencyIndicator::Agency, "AA", "", type::LocType::Area,
                               noLoc, type::CodeType::Blank, "");
    addServiceFeeSecurityEntry(type::TravelAgencyIndicator::Blank, "1S", "", type::LocType::Blank,
                               noLoc, type::CodeType::Blank, "");
    addServiceFeeSecurityEntry(type::TravelAgencyIndicator::Agency, "VA", "", type::LocType::Area,
                               noLoc, type::CodeType::Blank, "");

    CPPUNIT_ASSERT(_applicator->apply(_paymentDetail));
  }

  void matchIfLastRowMatches()
  {
    addServiceFeeSecurityEntry(type::TravelAgencyIndicator::Agency, "UA", "", type::LocType::Area,
                               noLoc, type::CodeType::Blank, "");
    addServiceFeeSecurityEntry(type::TravelAgencyIndicator::Agency, "AA", "", type::LocType::Area,
                               noLoc, type::CodeType::Blank, "");
    addServiceFeeSecurityEntry(type::TravelAgencyIndicator::Blank, "1S", "", type::LocType::Blank,
                               noLoc, type::CodeType::Blank, "");

    CPPUNIT_ASSERT(_applicator->apply(_paymentDetail));
  }

  void matchAgencyTagTest()
  {
    CPPUNIT_ASSERT(_applicator->agencyMatches(type::TravelAgencyIndicator::Blank, nullPCC));
    CPPUNIT_ASSERT(_applicator->agencyMatches(type::TravelAgencyIndicator::Blank, "W0H3"));
    CPPUNIT_ASSERT(!_applicator->agencyMatches(type::TravelAgencyIndicator::Agency, nullPCC));
    CPPUNIT_ASSERT(_applicator->agencyMatches(type::TravelAgencyIndicator::Agency, "W0H3"));
  }

  void testCarrierMatches_CSR()
  {
    CPPUNIT_ASSERT(_applicator->carrierMatches("1S", "1S", "AA"));
    CPPUNIT_ASSERT(!_applicator->carrierMatches("1S", "LH", "1S"));
  }

  void testCarrierMatches_Carrier()
  {
    CPPUNIT_ASSERT(_applicator->carrierMatches("AA", "1S", "AA"));
    CPPUNIT_ASSERT(!_applicator->carrierMatches("LH", "LH", "1S"));
  }

  void testIsCrsCodeMatches()
  {
    CPPUNIT_ASSERT(_applicator->isCrsCodeMatches("1S", "1B"));
    CPPUNIT_ASSERT(_applicator->isCrsCodeMatches("1S", "1J"));
    CPPUNIT_ASSERT(_applicator->isCrsCodeMatches("1S", "1F"));
    CPPUNIT_ASSERT(_applicator->isCrsCodeMatches("1B", "1B"));
    CPPUNIT_ASSERT(!_applicator->isCrsCodeMatches("1B", "1J"));
    CPPUNIT_ASSERT(!_applicator->isCrsCodeMatches("2S", "1J"));
  }

  void matchIfDutyEmpty()
  {
    CPPUNIT_ASSERT(_applicator->dutyFunctionMatches("", "A", "*"));
    CPPUNIT_ASSERT(_applicator->dutyFunctionMatches("", "", ""));
    CPPUNIT_ASSERT(_applicator->dutyFunctionMatches("", "A", ""));
    CPPUNIT_ASSERT(_applicator->dutyFunctionMatches("", "X", "7*H"));
  }

  void dontMatchIfNoAgentDutyInfo()
  {
    CPPUNIT_ASSERT(!_applicator->dutyFunctionMatches("A", "", "*"));
    CPPUNIT_ASSERT(!_applicator->dutyFunctionMatches("A8", "", ""));
    CPPUNIT_ASSERT(!_applicator->dutyFunctionMatches("E", "", ""));
    CPPUNIT_ASSERT(!_applicator->dutyFunctionMatches("E3", "", "7*H"));
  }

  void matchIfDutyCodesAgree()
  {
    CPPUNIT_ASSERT(_applicator->dutyFunctionMatches("1", "1", "*"));
    CPPUNIT_ASSERT(_applicator->dutyFunctionMatches("2", "2", ""));
    CPPUNIT_ASSERT(_applicator->dutyFunctionMatches("3", "3", ""));
    CPPUNIT_ASSERT(_applicator->dutyFunctionMatches("4", "4", "7*H"));
    CPPUNIT_ASSERT(!_applicator->dutyFunctionMatches("1", "2", "*"));
    CPPUNIT_ASSERT(!_applicator->dutyFunctionMatches("2", "1", ""));
    CPPUNIT_ASSERT(!_applicator->dutyFunctionMatches("3", "4", ""));
    CPPUNIT_ASSERT(!_applicator->dutyFunctionMatches("4", "3", "7*H"));
  }

  void matchIfDutyCodesAgreeWithMapping()
  { // ABCDE in the rule match to $@*-/
    CPPUNIT_ASSERT(_applicator->dutyFunctionMatches("A", "$", "*"));
    CPPUNIT_ASSERT(_applicator->dutyFunctionMatches("B", "@", ""));
    CPPUNIT_ASSERT(_applicator->dutyFunctionMatches("C", "*", ""));
    CPPUNIT_ASSERT(_applicator->dutyFunctionMatches("D", "-", "7*H"));
    CPPUNIT_ASSERT(_applicator->dutyFunctionMatches("E", "/", "7*H"));
    CPPUNIT_ASSERT(!_applicator->dutyFunctionMatches("$", "A", "*"));
    CPPUNIT_ASSERT(!_applicator->dutyFunctionMatches("@", "B", "*"));
    CPPUNIT_ASSERT(!_applicator->dutyFunctionMatches("*", "C", ""));
    CPPUNIT_ASSERT(!_applicator->dutyFunctionMatches("-", "D", ""));
    CPPUNIT_ASSERT(!_applicator->dutyFunctionMatches("/", "E", "7*H"));
  }

  void dontMatchIfNoAgentFunctionInfo()
  {
    CPPUNIT_ASSERT(!_applicator->dutyFunctionMatches("1X", "1", ""));
    CPPUNIT_ASSERT(!_applicator->dutyFunctionMatches("2E", "2", ""));
    CPPUNIT_ASSERT(!_applicator->dutyFunctionMatches("3A", "3", ""));
    CPPUNIT_ASSERT(!_applicator->dutyFunctionMatches("47", "4", ""));
  }

  void matchIfFunctionCodesAgree()
  {
    CPPUNIT_ASSERT(_applicator->dutyFunctionMatches("11", "1", "1A7"));
    CPPUNIT_ASSERT(_applicator->dutyFunctionMatches("2U", "2", "U"));
    CPPUNIT_ASSERT(_applicator->dutyFunctionMatches("3Z", "3", "Z"));
    CPPUNIT_ASSERT(_applicator->dutyFunctionMatches("4D", "4", "D*H"));
    CPPUNIT_ASSERT(!_applicator->dutyFunctionMatches("15", "1", "*"));
    CPPUNIT_ASSERT(!_applicator->dutyFunctionMatches("2X", "2", "Y"));
    CPPUNIT_ASSERT(!_applicator->dutyFunctionMatches("3%", "3", "&&&"));
    CPPUNIT_ASSERT(!_applicator->dutyFunctionMatches("4#", "4", "7*H"));

    // with duty mapping:
    CPPUNIT_ASSERT(_applicator->dutyFunctionMatches("A*", "$", "*"));
    CPPUNIT_ASSERT(_applicator->dutyFunctionMatches("B0", "@", "0X"));
    CPPUNIT_ASSERT(_applicator->dutyFunctionMatches("CX", "*", "X0"));
    CPPUNIT_ASSERT(_applicator->dutyFunctionMatches("D$", "-", "$*H"));
    CPPUNIT_ASSERT(_applicator->dutyFunctionMatches("E:", "/", ":*H"));
    CPPUNIT_ASSERT(!_applicator->dutyFunctionMatches("$*", "A", "*"));
    CPPUNIT_ASSERT(!_applicator->dutyFunctionMatches("@0", "B", "0"));
    CPPUNIT_ASSERT(!_applicator->dutyFunctionMatches("*X", "C", "X"));
    CPPUNIT_ASSERT(!_applicator->dutyFunctionMatches("-$", "D", "$"));
    CPPUNIT_ASSERT(!_applicator->dutyFunctionMatches("/:", "E", ":*H"));
  }

  void matchIfLocationEmpty()
  {
    LocZone ruleLoc;
    ruleLoc.type() = type::LocType::Blank;
    CPPUNIT_ASSERT(_applicator->locationMatches (ruleLoc, "KRK"));
    CPPUNIT_ASSERT(_applicator->locationMatches (ruleLoc, type::AirportOrCityCode(UninitializedCode)));
  }

  void dontMatchIfAgentLocationEmpty()
  {
    LocZone ruleLoc;
    ruleLoc.type() = type::LocType::Area;
    ruleLoc.code() = "ASIA";
    CPPUNIT_ASSERT(!_applicator->locationMatches (ruleLoc, type::AirportOrCityCode(UninitializedCode)));
    CPPUNIT_ASSERT(!_applicator->locationMatches (ruleLoc, type::AirportOrCityCode(UninitializedCode)));
  }

  void matchIfLocationMatches()
  {
    LocZone ruleLoc;
    ruleLoc.type() = type::LocType::Area;
    ruleLoc.code() = "ASIA";
    _locServiceMock.clear().add(true);
    CPPUNIT_ASSERT(_applicator->locationMatches (ruleLoc, "KRK"));
    CPPUNIT_ASSERT(_locServiceMock.empty());
    _locServiceMock.clear().add(false);
    CPPUNIT_ASSERT(!_applicator->locationMatches (ruleLoc, "KRK"));
    CPPUNIT_ASSERT(_locServiceMock.empty());
  }

  void matchCodeTypeBlank()
  { // blank always matches
    CPPUNIT_ASSERT(_applicator->codeMatches (type::CodeType::Blank, "", _pointOfSale));
  }

  void dontMatchCodeTypesXVLA()
  { // X, V, L, A never match
    _pointOfSale.agentAirlineDept() = "ABC";
    _pointOfSale.agentOfficeDesignator() = "DEF";

    CPPUNIT_ASSERT(!_applicator->codeMatches (type::CodeType::Department, "", _pointOfSale));
    CPPUNIT_ASSERT(!_applicator->codeMatches (type::CodeType::LNIATA, "", _pointOfSale));
    CPPUNIT_ASSERT(!_applicator->codeMatches (type::CodeType::AirlineSpec, "", _pointOfSale));
    CPPUNIT_ASSERT(!_applicator->codeMatches (type::CodeType::CarrierDept, "", _pointOfSale));

  }

  void testIsAirlineDepartmentMatches()
  {
    _pointOfSale.agentAirlineDept() = "ABC";
    _pointOfSale.agentOfficeDesignator() = "DEF";

    CPPUNIT_ASSERT(!_applicator->isAirlineDepartmentMatches("", _pointOfSale));
    CPPUNIT_ASSERT(!_applicator->isAirlineDepartmentMatches("ABB", _pointOfSale));
    CPPUNIT_ASSERT(_applicator->isAirlineDepartmentMatches("ABC", _pointOfSale));
    CPPUNIT_ASSERT(_applicator->isAirlineDepartmentMatches("DEF", _pointOfSale));
  }

  void matchCodeTypeT()
  {
    _pointOfSale.agentPcc() = "7DH0";
    CPPUNIT_ASSERT(!_applicator->codeMatches (type::CodeType::AgencyPCC, "", _pointOfSale));
    CPPUNIT_ASSERT(!_applicator->codeMatches (type::CodeType::AgencyPCC, "", _pointOfSale));
    CPPUNIT_ASSERT(!_applicator->codeMatches (type::CodeType::AgencyPCC, "7DH8", _pointOfSale));
    CPPUNIT_ASSERT(_applicator->codeMatches (type::CodeType::AgencyPCC, "7DH0", _pointOfSale));
    _pointOfSale.agentPcc() = nullPCC;
    CPPUNIT_ASSERT(!_applicator->codeMatches (type::CodeType::AgencyPCC, "7DH0", _pointOfSale));
  }

  void matchCodeTypeI()
  {
    _pointOfSale.iataNumber() = "749245";
    CPPUNIT_ASSERT(!_applicator->codeMatches (type::CodeType::AgencyNumber, "", _pointOfSale));
    CPPUNIT_ASSERT(!_applicator->codeMatches (type::CodeType::AgencyNumber, "", _pointOfSale));
    CPPUNIT_ASSERT(!_applicator->codeMatches (type::CodeType::AgencyNumber, "749244", _pointOfSale));
    CPPUNIT_ASSERT(_applicator->codeMatches (type::CodeType::AgencyNumber, "749245", _pointOfSale));
    _pointOfSale.iataNumber() = "";
    CPPUNIT_ASSERT(!_applicator->codeMatches (type::CodeType::AgencyNumber, "749245", _pointOfSale));
  }

  void matchCodeTypeE()
  {
    _pointOfSale.ersp() = "749245";
    CPPUNIT_ASSERT(!_applicator->codeMatches (type::CodeType::ElecResServProvider, "", _pointOfSale));
    CPPUNIT_ASSERT(!_applicator->codeMatches (type::CodeType::ElecResServProvider, "", _pointOfSale));
    CPPUNIT_ASSERT(!_applicator->codeMatches (type::CodeType::ElecResServProvider, "749244", _pointOfSale));
    CPPUNIT_ASSERT(_applicator->codeMatches (type::CodeType::ElecResServProvider, "749245", _pointOfSale));
    _pointOfSale.ersp() = "";
    CPPUNIT_ASSERT(!_applicator->codeMatches (type::CodeType::ElecResServProvider, "749245", _pointOfSale));
  }

  void matchIfViewBookTkt()
  {
    addServiceFeeSecurityEntry(type::TravelAgencyIndicator::Agency, "1S", "", type::LocType::Blank,
                               noLoc, type::CodeType::Blank, "", type::ViewBookTktInd::ViewBookTkt);

    _pointOfSale.agentPcc() = "B4T0";
    _pointOfSale.loc() = "KRK";

    CPPUNIT_ASSERT(_applicator->apply(_paymentDetail));
  }

  void dontMatchIfViewOnly()
  {
    addServiceFeeSecurityEntry(type::TravelAgencyIndicator::Agency, "1S", "", type::LocType::Blank,
                               noLoc, type::CodeType::Blank, "", type::ViewBookTktInd::ViewOnly);

    _pointOfSale.agentPcc() = "B4T0";
    _pointOfSale.loc() = "KRK";

    CPPUNIT_ASSERT(!_applicator->apply(_paymentDetail));
  }

  void verify()
  {
    _locServiceMock.clear().add(true).add(false).add(true);
  }

private:
  std::unique_ptr<ServiceFeeSecurityRule> _rule;
  std::unique_ptr<ServiceFeeSecurityApplicator> _applicator;
  PointOfSale _pointOfSale;
  PaymentDetailMock _paymentDetail;
  LocServiceMock _locServiceMock;
  std::shared_ptr<ServiceFeeSecurityItems> _items;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ServiceFeeSecurityApplicatorTest);
} // namespace tax
