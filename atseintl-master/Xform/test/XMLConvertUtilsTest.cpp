#include "Xform/XMLConvertUtils.h"

#include "Common/TseConsts.h"
#include "DBAccess/MultiTransport.h"
#include "DBAccess/Cabin.h"
#include "DBAccess/Customer.h"
#include "DBAccess/NoPNROptions.h"
#include "DataModel/AltPricingTrx.h"
#include "DataModel/AltPricingDetailTrx.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/PricingDetailTrx.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"

#include "test/DBAccessMock/DataHandleMock.h"

#include "test/include/TestConfigInitializer.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include <boost/assign/std/vector.hpp>

namespace tse
{
using boost::assign::operator+=;

class XMLConvertUtils;

namespace
{
class PricingDetailTrxBuilder
{
private:
  TestMemHandle* _memHandle;
  PricingDetailTrx* _trx;
  Billing _billing;

public:
  PricingDetailTrxBuilder(TestMemHandle* memHandle) : _memHandle(memHandle)
  {
    _trx = _memHandle->create<PricingDetailTrx>();

    Agent agent;
    agent.agentDuty() = "testDuty";
    agent.vendorCrsCode() = "testCode";
    withAgent(agent);

    _billing.userStation() = "testStation";
    withBilling(_billing);
  }

  PricingDetailTrxBuilder& withAgent(Agent& agent)
  {
    _trx->ticketingAgent() = agent;
    return *this;
  }

  PricingDetailTrxBuilder& withBilling(Billing& billing)
  {
    _trx->billing() = &billing;
    return *this;
  }

  PricingDetailTrxBuilder& setAxess(bool isAxess)
  {
    _trx->ticketingAgent().vendorCrsCode() = isAxess ? AXESS_MULTIHOST_ID : SABRE_MULTIHOST_ID;
    return *this;
  }

  PricingDetailTrxBuilder& setTrailMessage(bool isTrailMessage)
  {
    if (isTrailMessage)
    {
      _trx->billing()->actionCode() = "WQ";
      _trx->ticketingAgent().agentCity() = "KRK";
    }
    else
    {
      _trx->billing()->actionCode() = "";
      _trx->ticketingAgent().agentCity() = "";
    }

    return *this;
  }

  PricingDetailTrx* build() const { return _trx; }
};

class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;
  MultiTransport* getMT(LocCode city, LocCode loc)
  {
    MultiTransport* ret = _memHandle.create<MultiTransport>();
    ret->multitranscity() = city;
    ret->multitransLoc() = loc;
    return ret;
  }

public:
  const std::vector<Customer*>& getCustomer(const PseudoCityCode& key)
  {
    std::vector<Customer*>& ret = *_memHandle.create<std::vector<Customer*>>();
    if (key == "PNQ" || key == "DFW" || key == "HDQ" || key == "LHR" || key == "")
      return ret;
    else if (key == "7TN8" || key == "B4T0" || key == "DQ73")
    {
      Customer* c = _memHandle.create<Customer>();
      ret.push_back(c);
      if (key == "7TN8")
      {
        c->requestCity() = "PNQ";
        c->aaCity() = "PNQ";
        c->defaultCur() = "INR";
        c->lbtCustomerGroup() = "1435761";
        c->agencyName() = "AKBAR TRAVELS - POON";
        c->alternateHomePseudo() = "3OOD";
        c->branchAccInd() = 'N';
        c->curConvInd() = 'N';
        c->cadSubscriberInd() = 'N';
        c->channelId() = 'T';
        c->ssgGroupNo() = 16;
        c->crsCarrier() = "1B";
        c->hostName() = "ABAC";
        c->fareQuoteCur() = 'Y';
        c->eTicketCapable() = 'Y';
      }
      else if (key == "B4T0")
      {
        c->requestCity() = "DFW";
        c->aaCity() = "DFW";
        c->defaultCur() = "USD";
        c->lbtCustomerGroup() = "GRP02";
        c->agencyName() = "SABRE SOFTWARE";
        c->branchAccInd() = 'Y';
        c->curConvInd() = 'Y';
        c->cadSubscriberInd() = 'Y';
        c->channelId() = 'N';
        c->noRollupPfaBulkTkt() = 'N';
        c->ssgGroupNo() = 34;
        c->crsCarrier() = "1S";
        c->hostName() = "SABR";
        c->fareQuoteCur() = 'N';
        c->eTicketCapable() = 'Y';
      }
      else if (key == "DQ73")
      {
        c->requestCity() = "LHR";
        c->aaCity() = "LON";
        c->defaultCur() = "GBP";
        c->lbtCustomerGroup() = "9999999";
        c->agencyName() = "LON FARES HELPDESK";
        c->branchAccInd() = 'N';
        c->curConvInd() = 'N';
        c->cadSubscriberInd() = 'N';
        c->channelId() = 'N';
        c->ssgGroupNo() = 38;
        c->crsCarrier() = "1S";
        c->hostName() = "SABR";
        c->fareQuoteCur() = 'N';
        c->eTicketCapable() = 'N';
      }
      c->homePseudoCity() = key;
      c->webSubscriberInd() = 'N';
      c->btsSubscriberInd() = 'N';
      c->sellingFareInd() = 'N';
      c->tvlyInternetSubriber() = 'N';
      c->tvlyLocation() = 'N';
      c->availabilityIgRul2St() = 'Y';
      c->availabilityIgRul3St() = 'N';
      c->availIgRul2StWpnc() = 'Y';
      c->activateJourneyPricing() = 'Y';
      c->activateJourneyShopping() = 'Y';
      c->doNotApplySegmentFee() = 'N';
      c->optInAgency() = 'Y';
      c->privateFareInd() = 'Y';
      c->doNotApplyObTktFees() = 'N';
      c->defaultPassengerType() = "ADT";

      return ret;
    }
    return DataHandleMock::getCustomer(key);
  }
  const std::vector<NoPNROptions*>&
  getNoPNROptions(const Indicator& userApplType, const UserApplCode& userAppl)
  {
    std::vector<NoPNROptions*>& ret = *_memHandle.create<std::vector<NoPNROptions*>>();

    if (userAppl == "ABAC")
    {
      NoPNROptions* c = _memHandle.create<NoPNROptions>();
      ret.push_back(c);
      c->userApplType() = 'C';
      c->wqNotPermitted() = 'N';
      c->maxNoOptions() = 12;
      c->wqSort() = 'L';
      c->wqDuplicateAmounts() = 'N';
      c->fareLineHeaderFormat() = 'Y';
      c->passengerDetailLineFormat() = 3;
      c->fareLinePTC() = 1;
      c->primePTCRefNo() = 'N';
      c->secondaryPTCRefNo() = 'Y';
      c->fareLinePTCBreak() = 'Y';
      c->passengerDetailPTCBreak() = 'Y';
      c->negPassengerTypeMapping() = 1;
      c->noMatchOptionsDisplay() = 'N';
      c->allMatchTrailerMessage() = 1;
      c->matchIntegratedTrailer() = 1;
      c->accompaniedTvlTrailerMsg() = 3;
      c->rbdMatchTrailerMsg() = 1;
      c->rbdNoMatchTrailerMsg() = 1;
      c->rbdNoMatchTrailerMsg2() = 1;
      c->displayFareRuleWarningMsg() = 'Y';
      c->displayFareRuleWarningMsg2() = 'Y';
      c->displayFinalWarningMsg() = 'Y';
      c->displayFinalWarningMsg2() = 'Y';
      c->displayTaxWarningMsg() = 'N';
      c->displayTaxWarningMsg2() = 'N';
      c->displayPrivateFareInd() = 'N';
      c->displayNonCOCCurrencyInd() = 'Y';
      c->displayTruePTCInFareLine() = 'Y';
      c->applyROInFareDisplay() = 'Y';
      c->alwaysMapToADTPsgrType() = 'N';
      c->noMatchRBDMessage() = 1;
      c->noMatchNoFaresErrorMsg() = 2;
      c->totalNoOptions() = 14;
      return ret;
    }
    else if (userAppl == AXESS_USER || userAppl == SABRE_USER)
    {
      NoPNROptions* c = _memHandle.create<NoPNROptions>();
      ret.push_back(c);
      c->userApplType() = 'C';
      c->wqNotPermitted() = 'N';
      c->maxNoOptions() = 12;
      c->wqSort() = 'L';
      c->wqDuplicateAmounts() = 'N';
      c->fareLineHeaderFormat() = 'Y';
      c->passengerDetailLineFormat() = 3;
      c->fareLinePTC() = 1;
      c->primePTCRefNo() = 'N';
      c->secondaryPTCRefNo() = 'Y';
      c->fareLinePTCBreak() = 'Y';
      c->passengerDetailPTCBreak() = 'Y';
      c->negPassengerTypeMapping() = 1;
      c->noMatchOptionsDisplay() = 'N';
      c->allMatchTrailerMessage() = 1;
      c->matchIntegratedTrailer() = 1;
      c->accompaniedTvlTrailerMsg() = 3;
      c->rbdMatchTrailerMsg() = 1;
      c->rbdNoMatchTrailerMsg() = 1;
      c->rbdNoMatchTrailerMsg2() = 1;
      c->displayFareRuleWarningMsg() = 'Y';
      c->displayFareRuleWarningMsg2() = 'Y';
      c->displayFinalWarningMsg() = 'Y';
      c->displayFinalWarningMsg2() = 'Y';
      c->displayTaxWarningMsg() = 'N';
      c->displayTaxWarningMsg2() = 'N';
      c->displayPrivateFareInd() = 'N';
      c->displayNonCOCCurrencyInd() = 'Y';
      c->displayTruePTCInFareLine() = 'Y';
      c->applyROInFareDisplay() = 'Y';
      c->alwaysMapToADTPsgrType() = 'N';
      c->noMatchRBDMessage() = 1;
      c->noMatchNoFaresErrorMsg() = 2;
      c->totalNoOptions() = 14;
      return ret;
    }
    else if (userAppl == "")
    {
      return ret;
    }
    return DataHandleMock::getNoPNROptions(userApplType, userAppl);
  }
  const Loc* getLoc(const LocCode& locCode, const DateTime& date)
  {
    if (locCode == "")
      return 0;
    return DataHandleMock::getLoc(locCode, date);
  }
  const std::vector<const PaxTypeMatrix*>& getPaxTypeMatrix(const PaxTypeCode& paxTypeCode)
  {
    if (paxTypeCode == "ADT")
      return *_memHandle.create<std::vector<const PaxTypeMatrix*>>();
    return DataHandleMock::getPaxTypeMatrix(paxTypeCode);
  }
  const std::vector<FareCalcConfig*>& getFareCalcConfig(const Indicator& userApplType,
                                                        const UserApplCode& userAppl,
                                                        const PseudoCityCode& pseudoCity)
  {
    if (pseudoCity == "7TN8" || pseudoCity == "HDQ" || pseudoCity == "DQ73")
      return DataHandleMock::getFareCalcConfig(' ', "", "");
    return DataHandleMock::getFareCalcConfig(userApplType, userAppl, pseudoCity);
  }
  const Cabin*
  getCabin(const CarrierCode& carrier, const BookingCode& classOfService, const DateTime& date)
  {
    Cabin* ret = _memHandle.create<Cabin>();
    if (classOfService == "Y" || (carrier == "SQ" && classOfService == "N"))
    {
      ret->cabin().setEconomyClass();
      return ret;
    }
    else if (classOfService == "T" || classOfService == "Q")
    {
      ret->cabin().setEconomyClass();
      return ret;
    }
    else if (carrier == "DL")
    {
      ret->cabin().setEconomyClass();
      return ret;
    }
    return DataHandleMock::getCabin(carrier, classOfService, date);
  }
  const LocCode getMultiTransportCity(const LocCode& locCode)
  {
    if (locCode == "DFW")
      return "DFW";
    else if (locCode == "LHR")
      return "LON";
    else if (locCode == "PNQ")
      return "PNQ";
    else if (locCode == "7TN8")
      return "7TN8";

    return DataHandleMock::getMultiTransportCity(locCode);
  }
  const std::vector<MultiTransport*>& getMultiTransportCity(const LocCode& locCode,
                                                            const CarrierCode& carrierCode,
                                                            GeoTravelType tvlType,
                                                            const DateTime& tvlDate)
  {
    std::vector<MultiTransport*>& ret = *_memHandle.create<std::vector<MultiTransport*>>();
    if (locCode == "AKL" || locCode == "CCU" || locCode == "ICN" || locCode == "LAX" ||
        locCode == "NRT" || locCode == "PPT")
      return ret;
    else if (locCode == "BKK")
    {
      ret += getMT("BKK", "NBK"), getMT("BKK", "DMK");
      return ret;
    }
    else if (locCode == "DFW")
    {
      ret += getMT("DFW", "DAL"), getMT("DFW", "DFW"), getMT("DFW", "QDF");
      return ret;
    }
    else if (locCode == "NGO")
    {
      ret += getMT("NGO", "NGO"), getMT("NGO", "NKM");
      return ret;
    }
    else if (locCode == "SIN")
    {
      ret += getMT("SIN", "SIN"), getMT("SIN", "XSP");
      return ret;
    }
    else if (locCode == "SVO")
    {
      ret += getMT("SVO", "SVO"), getMT("SVO", "SVO");
      return ret;
    }
    else if (locCode == "JFK")
    {
      ret += getMT("JFK", "JFK"), getMT("JFK", "JFK");
      return ret;
    }
    else if (locCode == "PNQ")
    {
      ret += getMT("PNQ", "PNQ"), getMT("PNQ", "PNQ");
      return ret;
    }
    return DataHandleMock::getMultiTransportCity(locCode, carrierCode, tvlType, tvlDate);
  }
};
}

class XMLConvertUtilsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(XMLConvertUtilsTest);
  CPPUNIT_TEST(testFormatWpanDetailResponse);
  CPPUNIT_TEST(testGetWpanDetailPsgSum);
  CPPUNIT_TEST(testFormatResponse);
  CPPUNIT_TEST(testFormatWpnDetailsResponse);
  CPPUNIT_TEST(testFormatWpnDetailsResponseRemoveSft);
  CPPUNIT_TEST(testFormatBaggageResponse);
  CPPUNIT_TEST(testFormatWpnResponse_AXESS_trail_message);
  CPPUNIT_TEST(testFormatWpnResponse_AXESS_no_trail_message);
  CPPUNIT_TEST(testFormatWpnResponse_NOT_AXESS_trail_message);
  CPPUNIT_TEST(testFormatWpnResponse_NOT_AXESS_no_trail_message);
  CPPUNIT_TEST(testFormatWpnResponse_prepareResponseText);
  CPPUNIT_TEST(testFormatWpaWtfrResponse_0surface);
  CPPUNIT_TEST(testFormatWpaWtfrResponse_1surface);
  CPPUNIT_TEST(testFormatWpaWtfrResponse_multiSurface);
  CPPUNIT_TEST(testFormatWpanDetailResponse_0surface);
  CPPUNIT_TEST(testFormatWpanDetailResponse_1surface);
  CPPUNIT_TEST(testFormatWpanDetailResponse_multiSurface);

  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

  std::string getPBTFromResponse(std::string& response)
  {
    std::string result;

    size_t pos = response.find("PBT");
    if (pos != std::string::npos)
    {
      pos += 5;
      result = response.substr(pos, 1);
    }

    return result;
  }

  AltPricingDetailTrx* formatWpanWtfrSetup(int ariNumber, int ariWithSurface)
  {
    AltPricingTrx::AccompRestrictionInfo ari;
    ari.surfaceRestricted() = true;
    AltPricingDetailTrx* altPricing = _memHandle.create<AltPricingDetailTrx>();

    if (ariWithSurface > ariNumber)
      ariNumber = ariWithSurface;

    for (int i = 0; i < (ariNumber - ariWithSurface); i++)
      altPricing->accompRestrictionVec().push_back(AltPricingTrx::AccompRestrictionInfo());

    for (int i = 0; i < ariWithSurface; i++)
      altPricing->accompRestrictionVec().push_back(ari);

    return altPricing;
  }
  std::string formatWpanWtfrExpectedError()
  {
    std::string response =
        "<PricingResponse>"
        "<MSG N06=\"E\" Q0K=\"000002\" S18=\"ISSUE SEPARATE TICKETS-INTL SURFACE RESTRICTED\"/>"
        "<MSG N06=\"E\" Q0K=\"000003\" S18=\"TICKETING NOT PERMITTED\"/>"
        "<MSG N06=\"E\" Q0K=\"000004\" S18=\" \"/>"
        "</PricingResponse>";
    return response;
  }

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();
  }

  void tearDown() { _memHandle.clear(); }

  //---------------------------------------------------------------------
  // tests
  //---------------------------------------------------------------------

  void testFormatWpanDetailResponse()
  {
    AltPricingDetailTrx trx;
    trx.setRequest(_memHandle.create<PricingRequest>());
    trx.setOptions(_memHandle.create<PricingOptions>());
    trx.getRequest()->ticketingAgent() = _memHandle.create<Agent>();

    std::string response = XMLConvertUtils::formatWpanDetailResponse(&trx);
    CPPUNIT_ASSERT(response == "<PricingResponse></PricingResponse>");

    trx.getOptions()->recordQuote() = 'T';
    response = XMLConvertUtils::formatWpanDetailResponse(&trx);
    CPPUNIT_ASSERT(response.find("PRICE QUOTE RECORD RETAINED") == std::string::npos);

    trx.accompRestrictionVec().resize(1);
    trx.accompRestrictionVec()[0].selectionXml() = "S66=\"X\">";

    trx.getOptions()->recordQuote() = 'T';
    response = XMLConvertUtils::formatWpanDetailResponse(&trx);
    CPPUNIT_ASSERT(response.find("PRICE QUOTE RECORD RETAINED") != std::string::npos);

    trx.accompRestrictionVec().clear();
    trx.getOptions()->recordQuote() = 'F';
    response = XMLConvertUtils::formatWpanDetailResponse(&trx);
    CPPUNIT_ASSERT(response.find("PRICE QUOTE RECORD RETAINED") == std::string::npos);
  }

  void testGetWpanDetailPsgSum()
  {
    AltPricingDetailTrx trx;
    trx.setRequest(_memHandle.create<PricingRequest>());
    trx.setOptions(_memHandle.create<PricingOptions>());
    trx.getRequest()->ticketingAgent() = _memHandle.create<Agent>();

    trx.accompRestrictionVec().resize(1);
    trx.accompRestrictionVec()[0].selectionXml() = "<SUM PBV=\"F\">";
    trx.accompRestrictionVec()[0].guaranteed() = false;

    std::string response = XMLConvertUtils::getWpanDetailPsgSum(&trx);
    CPPUNIT_ASSERT(getPBTFromResponse(response) == "F");

    trx.accompRestrictionVec()[0].guaranteed() = true;
    response = XMLConvertUtils::getWpanDetailPsgSum(&trx);
    CPPUNIT_ASSERT(getPBTFromResponse(response) == "T");

    trx.accompRestrictionVec().resize(2);
    trx.accompRestrictionVec()[1].selectionXml() = "<SUM PBV=\"F\">";
    trx.accompRestrictionVec()[1].guaranteed() = false;

    response = XMLConvertUtils::getWpanDetailPsgSum(&trx);
    CPPUNIT_ASSERT(getPBTFromResponse(response) == "F");

    trx.accompRestrictionVec()[1].guaranteed() = true;
    response = XMLConvertUtils::getWpanDetailPsgSum(&trx);
    CPPUNIT_ASSERT(getPBTFromResponse(response) == "T");

    trx.accompRestrictionVec()[0].guaranteed() = false;
    response = XMLConvertUtils::getWpanDetailPsgSum(&trx);
    CPPUNIT_ASSERT(getPBTFromResponse(response) == "F");
  }

  void testFormatResponse()
  {
    std::string tmpResponse =
        "PSGR TYPE  ADT - 01\n     CXR RES DATE  FARE BASIS      "
        "NVB   NVA    BG\n WAW\nXMUC LH  Y   20AUG Y22OW                 "
        "20AUG 02P\n TYO LH  Y   20AUG Y22OW                 20AUG 02P\nFARE  "
        "PLN   6513.00 EQUIV HKD     15480\nTAX   HKD       143XW HKD         2ND HKD      2050XT\n"
        "TOTAL HKD     17675\nADT-01  Y22OW\n WAW LH X/MUC LH TYO M2007.97NUC2007.97END "
        "ROE3.24357\n"
        "XT HKD167RA HKD1883YQ\nRATE USED 1PLN-2.3764702279624HKD\nATTN*VALIDATING CARRIER - LH\n"
        "ATTN*APPLICABLE BOOKING CLASS -  1Y 2Y\nATTN*REBOOK OPTION OF CHOICE BEFORE STORING "
        "FARE\n";

    std::string xmlResponse;

    std::string expected =
        "<MSG N06=\"X\" Q0K=\"000002\" S18=\"PSGR TYPE  ADT - 01\"/>"
        "<MSG N06=\"X\" Q0K=\"000003\" S18=\"     CXR RES DATE  FARE BASIS      NVB   NVA    BG\"/>"
        "<MSG N06=\"X\" Q0K=\"000004\" S18=\" WAW\"/>"
        "<MSG N06=\"X\" Q0K=\"000005\" S18=\"XMUC LH  Y   20AUG Y22OW                 20AUG 02P\"/>"
        "<MSG N06=\"X\" Q0K=\"000006\" S18=\" TYO LH  Y   20AUG Y22OW                 20AUG 02P\"/>"
        "<MSG N06=\"X\" Q0K=\"000007\" S18=\"FARE  PLN   6513.00 EQUIV HKD     15480\"/>"
        "<MSG N06=\"X\" Q0K=\"000008\" S18=\"TAX   HKD       143XW HKD         2ND HKD      "
        "2050XT\"/>"
        "<MSG N06=\"X\" Q0K=\"000009\" S18=\"TOTAL HKD     17675\"/>"
        "<MSG N06=\"X\" Q0K=\"000010\" S18=\"ADT-01  Y22OW\"/>"
        "<MSG N06=\"X\" Q0K=\"000011\" S18=\" WAW LH X/MUC LH TYO M2007.97NUC2007.97END "
        "ROE3.24357\"/>"
        "<MSG N06=\"X\" Q0K=\"000012\" S18=\"XT HKD167RA HKD1883YQ\"/>"
        "<MSG N06=\"X\" Q0K=\"000013\" S18=\"RATE USED 1PLN-2.3764702279624HKD\"/>"
        "<MSG N06=\"X\" Q0K=\"000014\" S18=\"ATTN*VALIDATING CARRIER - LH\"/>"
        "<MSG N06=\"X\" Q0K=\"000015\" S18=\"ATTN*APPLICABLE BOOKING CLASS -  1Y 2Y\"/>"
        "<MSG N06=\"X\" Q0K=\"000016\" S18=\"ATTN*REBOOK OPTION OF CHOICE BEFORE STORING FARE\"/>";

    XMLConvertUtils::formatResponse(tmpResponse, xmlResponse);

    CPPUNIT_ASSERT_EQUAL(expected, xmlResponse);
  }

  void testFormatWpnDetailsResponse()
  {
    PricingDetailTrxBuilder obj(&_memHandle);

    std::string wpnDetails =
        "PSGR TYPE  ADT - 01\\n     CXR RES DATE  FARE BASIS      "
        "NVB   NVA    BG\\n WAW\nXMUC LH  Y   20AUG Y22OW                 "
        "20AUG 02P\\n TYO LH  Y   20AUG Y22OW                 20AUG 02P\\nFARE  "
        "PLN   6513.00 EQUIV HKD     15480\\nTAX   HKD       143XW HKD         2ND HKD      "
        "2050XT\\n"
        "TOTAL HKD     17675\\nADT-01  Y22OW\n WAW LH X/MUC LH TYO M2007.97NUC2007.97END "
        "ROE3.24357\\n"
        "XT HKD167RA HKD1883YQ\\nRATE USED 1PLN-2.3764702279624HKD\\nATTN*VALIDATING CARRIER - "
        "LH\\n"
        "ATTN*APPLICABLE BOOKING CLASS -  1Y 2Y\\nATTN*REBOOK OPTION OF CHOICE BEFORE STORING "
        "FARE\\n";
    bool isDetailedWQ = false;
    bool isFrist = false;
    std::string wqApplicableBookingCodesLine;
    NoPNROptions* noPnrOptions = 0;
    int recNum = 2;

    std::string expected =
        "<MSG N06=\"X\" Q0K=\"000002\" S18=\"PSGR TYPE  ADT - 01\"/>"
        "<MSG N06=\"X\" Q0K=\"000003\" S18=\"     CXR RES DATE  FARE BASIS      NVB   NVA    BG\"/>"
        "<MSG N06=\"X\" Q0K=\"000004\" S18=\" WAW\"/>"
        "<MSG N06=\"X\" Q0K=\"000005\" S18=\"XMUC LH  Y   20AUG Y22OW                 20AUG 02P\"/>"
        "<MSG N06=\"X\" Q0K=\"000006\" S18=\" TYO LH  Y   20AUG Y22OW                 20AUG 02P\"/>"
        "<MSG N06=\"X\" Q0K=\"000007\" S18=\"FARE  PLN   6513.00 EQUIV HKD     15480\"/>"
        "<MSG N06=\"X\" Q0K=\"000008\" S18=\"TAX   HKD       143XW HKD         2ND HKD      "
        "2050XT\"/>"
        "<MSG N06=\"X\" Q0K=\"000009\" S18=\"TOTAL HKD     17675\"/>"
        "<MSG N06=\"X\" Q0K=\"000010\" S18=\"ADT-01  Y22OW\"/>"
        "<MSG N06=\"X\" Q0K=\"000011\" S18=\" WAW LH X/MUC LH TYO M2007.97NUC2007.97END "
        "ROE3.24357\"/>"
        "<MSG N06=\"X\" Q0K=\"000012\" S18=\"XT HKD167RA HKD1883YQ\"/>"
        "<MSG N06=\"X\" Q0K=\"000013\" S18=\"RATE USED 1PLN-2.3764702279624HKD\"/>"
        "<MSG N06=\"X\" Q0K=\"000014\" S18=\"ATTN*VALIDATING CARRIER - LH\"/>"
        "<MSG N06=\"X\" Q0K=\"000015\" S18=\"ATTN*APPLICABLE BOOKING CLASS -  1Y 2Y\"/>"
        "<MSG N06=\"X\" Q0K=\"000016\" S18=\"ATTN*REBOOK OPTION OF CHOICE BEFORE STORING FARE\"/>"
        "<MSG N06=\"X\" Q0K=\"000017\" S18=\" \"/>";

    PricingDetailTrx* trx = obj.build();
    trx->wpnTrx() = true;

    /*  CPPUNIT_ASSERT_EQUAL(expected,
                           XMLConvertUtils::formatWpnDetailsResponse(wpnDetails,
                                                                     isDetailedWQ,
                                                                     isFrist,
                                                                     wqApplicableBookingCodesLine,
                                                                     noPnrOptions,
                                                                     recNum,
                                                                     *trx));*/
  }

  void testFormatWpnDetailsResponseRemoveSft()
  {
    PricingDetailTrxBuilder obj(&_memHandle);

    std::string wpnDetails =
        "PSGR TYPE  ADT - 01\\n     CXR RES DATE  FARE BASIS      "
        "NVB   NVA    BG\\n WAW\nXMUC LH  Y   20AUG Y22OW                 "
        "20AUG 02P\\n TYO LH  Y   20AUG Y22OW                 20AUG 02P\\nFARE  "
        "PLN   6513.00 EQUIV HKD     15480\\nTAX   HKD       143XW HKD         2ND HKD      "
        "2050XT\\n"
        "TOTAL HKD     17675\\nADT-01  Y22OW\n WAW LH X/MUC LH TYO M2007.97NUC2007.97END "
        "ROE3.24357\\n"
        "XT HKD167RA HKD1883YQ\\nRATE USED 1PLN-2.3764702279624HKD\\nATTN*VALIDATING CARRIER - "
        "LH\\n"
        "SERVICE FEE _PDC_SF_AMOUNT_ SERVICE FEE TAX _PDC_SF_TAX_AMOUNT_\\n"
        " _PDC_TOTAL_WITH_SF_AMOUNT_\\n"
        "ATTN*APPLICABLE BOOKING CLASS -  1Y 2Y\\nATTN*REBOOK OPTION OF CHOICE BEFORE STORING "
        "FARE\\n";
    bool isDetailedWQ = false;
    bool isFrist = false;
    std::string wqApplicableBookingCodesLine;
    NoPNROptions* noPnrOptions = 0;
    int recNum = 2;

    std::string expected =
        "<MSG N06=\"X\" Q0K=\"000002\" S18=\"PSGR TYPE  ADT - 01\"/>"
        "<MSG N06=\"X\" Q0K=\"000003\" S18=\"     CXR RES DATE  FARE BASIS      NVB   NVA    BG\"/>"
        "<MSG N06=\"X\" Q0K=\"000004\" S18=\" WAW\"/>"
        "<MSG N06=\"X\" Q0K=\"000005\" S18=\"XMUC LH  Y   20AUG Y22OW                 20AUG 02P\"/>"
        "<MSG N06=\"X\" Q0K=\"000006\" S18=\" TYO LH  Y   20AUG Y22OW                 20AUG 02P\"/>"
        "<MSG N06=\"X\" Q0K=\"000007\" S18=\"FARE  PLN   6513.00 EQUIV HKD     15480\"/>"
        "<MSG N06=\"X\" Q0K=\"000008\" S18=\"TAX   HKD       143XW HKD         2ND HKD      "
        "2050XT\"/>"
        "<MSG N06=\"X\" Q0K=\"000009\" S18=\"TOTAL HKD     17675\"/>"
        "<MSG N06=\"X\" Q0K=\"000010\" S18=\"ADT-01  Y22OW\"/>"
        "<MSG N06=\"X\" Q0K=\"000011\" S18=\" WAW LH X/MUC LH TYO M2007.97NUC2007.97END "
        "ROE3.24357\"/>"
        "<MSG N06=\"X\" Q0K=\"000012\" S18=\"XT HKD167RA HKD1883YQ\"/>"
        "<MSG N06=\"X\" Q0K=\"000013\" S18=\"RATE USED 1PLN-2.3764702279624HKD\"/>"
        "<MSG N06=\"X\" Q0K=\"000014\" S18=\"ATTN*VALIDATING CARRIER - LH\"/>"
        "<MSG N06=\"X\" Q0K=\"000015\" S18=\"ATTN*APPLICABLE BOOKING CLASS -  1Y 2Y\"/>"
        "<MSG N06=\"X\" Q0K=\"000016\" S18=\"ATTN*REBOOK OPTION OF CHOICE BEFORE STORING FARE\"/>"
        "<MSG N06=\"X\" Q0K=\"000017\" S18=\" \"/>";

    PricingDetailTrx* trx = obj.build();
    trx->wpnTrx() = true;

    /* CPPUNIT_ASSERT_EQUAL(expected,
                          XMLConvertUtils::formatWpnDetailsResponse(wpnDetails,
                                                                    isDetailedWQ,
                                                                    isFrist,
                                                                    wqApplicableBookingCodesLine,
                                                                    noPnrOptions,
                                                                    recNum,
                                                                    *trx));*/
  }

  void testFormatBaggageResponse()
  {
    std::string baggageResponse =
        "ATTN*BAG ALLOWANCE     -WAWNRT-02P/LH/EACH PIECE UP TO 50 POUND\n"
        "ATTN*S/23 KILOGRAMS AND UP TO 62 LINEAR INCHES/158 LINEAR CENTI\n"
        "ATTN*METERS\nATTN*CARRY ON ALLOWANCE\n"
        "ATTN*WAWMUC MUCNRT-01P/LH\n"
        "ATTN*01/UP TO 18 POUNDS/8 KILOGRAMS AND UP TO 46 LINEAR INCHES/\n"
        "ATTN*118 LINEAR CENTIMETERS\nATTN*CARRY ON CHARGES\n"
        "ATTN*WAWMUC MUCNRT-LH\nATTN*1ST CARRY ON PERSONAL ITEMS-HKD0\n"
        "ATTN*ADDITIONAL ALLOWANCES AND/OR DISCOUNTS MAY APPLY\n";

    int recNum = 55;
    std::string expected =
        "<MSG N06=\"Y\" Q0K=\"000055\" S18=\"ATTN*BAG ALLOWANCE     -WAWNRT-02P/LH/EACH PIECE UP "
        "TO 50 POUND\"/>"
        "<MSG N06=\"Y\" Q0K=\"000056\" S18=\"ATTN*S/23 KILOGRAMS AND UP TO 62 LINEAR INCHES/158 "
        "LINEAR CENTI\"/>"
        "<MSG N06=\"Y\" Q0K=\"000057\" S18=\"ATTN*METERS\"/>"
        "<MSG N06=\"Y\" Q0K=\"000058\" S18=\"ATTN*CARRY ON ALLOWANCE\"/>"
        "<MSG N06=\"Y\" Q0K=\"000059\" S18=\"ATTN*WAWMUC MUCNRT-01P/LH\"/>"
        "<MSG N06=\"Y\" Q0K=\"000060\" S18=\"ATTN*01/UP TO 18 POUNDS/8 KILOGRAMS AND UP TO 46 "
        "LINEAR INCHES/\"/>"
        "<MSG N06=\"Y\" Q0K=\"000061\" S18=\"ATTN*118 LINEAR CENTIMETERS\"/>"
        "<MSG N06=\"Y\" Q0K=\"000062\" S18=\"ATTN*CARRY ON CHARGES\"/>"
        "<MSG N06=\"Y\" Q0K=\"000063\" S18=\"ATTN*WAWMUC MUCNRT-LH\"/>"
        "<MSG N06=\"Y\" Q0K=\"000064\" S18=\"ATTN*1ST CARRY ON PERSONAL ITEMS-HKD0\"/>"
        "<MSG N06=\"Y\" Q0K=\"000065\" S18=\"ATTN*ADDITIONAL ALLOWANCES AND/OR DISCOUNTS MAY "
        "APPLY\"/>";

    CPPUNIT_ASSERT_EQUAL(expected, XMLConvertUtils::formatBaggageResponse(baggageResponse, recNum));
  }

  void testFormatWpnResponse_AXESS_trail_message()
  {
    PricingDetailTrxBuilder obj(&_memHandle);
    obj.setAxess(true);
    obj.setTrailMessage(true);

    PricingDetailTrx* trx = obj.build();
    PricingRequest req;
    req.collectOBFee() = 'F';
    trx->setRequest(&req);

    const std::string expected =
        "<PricingResponse>"
        "<MSG N06=\"X\" Q0K=\"000002\" S18=\"VT \"/>"
        "<MSG N06=\"X\" Q0K=\"000003\" S18=\"** TOTALS INCLUDE KNOWN TAXES AND FEES **\"/>"
        "<MSG N06=\"X\" Q0K=\"000004\" S18=\"** TOTAL FARE, TAXES AND FEES MAY CHANGE ONCE FLIGHTS "
        "ARE \"/>"
        "<MSG N06=\"X\" Q0K=\"000005\" S18=\"   CONFIRMED **\"/>"
        "</PricingResponse>";

    CPPUNIT_ASSERT_EQUAL(expected, XMLConvertUtils::formatWpnResponse(*trx));
  }

  void testFormatWpnResponse_AXESS_no_trail_message()
  {
    PricingDetailTrxBuilder obj(&_memHandle);
    obj.setAxess(true);
    obj.setTrailMessage(false);

    PricingDetailTrx* trx = obj.build();
    PricingRequest req;
    req.collectOBFee() = 'F';
    trx->setRequest(&req);

    const std::string expected = "<PricingResponse>"
                                 "<MSG N06=\"X\" Q0K=\"000002\" S18=\"VT \"/>"
                                 "</PricingResponse>";

    CPPUNIT_ASSERT_EQUAL(expected, XMLConvertUtils::formatWpnResponse(*trx));
  }

  void testFormatWpnResponse_NOT_AXESS_trail_message()
  {
    PricingDetailTrxBuilder obj(&_memHandle);
    obj.setAxess(false);
    obj.setTrailMessage(true);

    PricingDetailTrx* trx = obj.build();
    PricingRequest req;
    req.collectOBFee() = 'F';
    trx->setRequest(&req);

    const std::string expected =
        "<PricingResponse>"
        "<MSG N06=\"X\" Q0K=\"000002\" S18=\"** TOTALS INCLUDE KNOWN TAXES AND FEES **\"/>"
        "<MSG N06=\"X\" Q0K=\"000003\" S18=\"** TOTAL FARE, TAXES AND FEES MAY CHANGE ONCE FLIGHTS "
        "ARE \"/>"
        "<MSG N06=\"X\" Q0K=\"000004\" S18=\"   CONFIRMED **\"/>"
        "</PricingResponse>";

    CPPUNIT_ASSERT_EQUAL(expected, XMLConvertUtils::formatWpnResponse(*trx));
  }

  void testFormatWpnResponse_NOT_AXESS_no_trail_message()
  {
    PricingDetailTrxBuilder obj(&_memHandle);
    obj.setAxess(false);
    obj.setTrailMessage(false);

    PricingDetailTrx* trx = obj.build();
    PricingRequest req;
    req.collectOBFee() = 'F';
    trx->setRequest(&req);

    const std::string expected = "<PricingResponse></PricingResponse>";

    CPPUNIT_ASSERT_EQUAL(expected, XMLConvertUtils::formatWpnResponse(*trx));
  }

  void testFormatWpnResponse_prepareResponseText()
  {
    std::string rowDiag = "******************* SERVICE FEE DIAGNOSTIC ********************\n"
                          "********************** OB FEES ANALYSIS ***********************\n"
                          "VALIDATING CXR : SK                  REQUESTED PAX TYPE : ADT\n"
                          "TRAVEL DATE    : 2015-04-25\n"
                          "TKT DATE       : 2015-04-13\n"
                          "JOURNEY TYPE   : OW INTL   JOURNEY DEST/TURNAROUND: STO\n"
                          "--------------------------------------------------------------\n"
                          "                  S4 RECORDS DATA PROCESSING\n"
                          "                  FOP BIN NUMBER -\n"
                          "                  CHARGE AMOUNT  -        0 DKK\n"
                          "                  RESIDUAL IND   - TRUE\n"
                          "  SVC    SEQ NUM   PAX    AMOUNT      FOP BIN    STATUS\n"
                          " OBFCA      1000   INF        0                  FAIL PAX TYPE\n"
                          " OBFCA      1050   ITF        0                  FAIL PAX TYPE\n"
                          " OBFCA      1100   JNF        0                  FAIL PAX TYPE\n"
                          " OBFCA      1150              0                  FAIL FARE BASIS\n"
                          " OBFCA      1650              0                  FAIL FARE BASIS\n"
                          " OBFCA      2650              0       670510     FAIL SECUR T183\n"
                          " OBFCA      2700              0       677262     FAIL SECUR T183\n"
                          " OBFCA      2950         2.2000PCT               FAIL SECUR T183\n"
                          " OBFDA    100000   INF   1.0000PCT               FAIL PAX TYPE\n"
                          " OBFDA    150000   CNN   2.0000PCT               FAIL PAX TYPE\n"
                          " OBFDA    200000         3.0000PCT               PASS\n";

    std::string expected =
        "<MSG N06=\"X\" Q0K=\"3\" S18=\"******************* SERVICE FEE DIAGNOSTIC "
        "********************\"/>"
        "<MSG N06=\"X\" Q0K=\"4\" S18=\"********************** OB FEES ANALYSIS "
        "***********************\"/>"
        "<MSG N06=\"X\" Q0K=\"5\" S18=\"VALIDATING CXR : SK                  REQUESTED PAX TYPE : "
        "ADT\"/>"
        "<MSG N06=\"X\" Q0K=\"6\" S18=\"TRAVEL DATE    : 2015-04-25\"/>"
        "<MSG N06=\"X\" Q0K=\"7\" S18=\"TKT DATE       : 2015-04-13\"/>"
        "<MSG N06=\"X\" Q0K=\"8\" S18=\"JOURNEY TYPE   : OW INTL   JOURNEY DEST/TURNAROUND: STO\"/>"
        "<MSG N06=\"X\" Q0K=\"9\" "
        "S18=\"--------------------------------------------------------------\"/>"
        "<MSG N06=\"X\" Q0K=\"10\" S18=\"                  S4 RECORDS DATA PROCESSING\"/>"
        "<MSG N06=\"X\" Q0K=\"11\" S18=\"                  FOP BIN NUMBER -\"/>"
        "<MSG N06=\"X\" Q0K=\"12\" S18=\"                  CHARGE AMOUNT  -        0 DKK\"/>"
        "<MSG N06=\"X\" Q0K=\"13\" S18=\"                  RESIDUAL IND   - TRUE\"/>"
        "<MSG N06=\"X\" Q0K=\"14\" S18=\"  SVC    SEQ NUM   PAX    AMOUNT      FOP BIN    "
        "STATUS\"/>"
        "<MSG N06=\"X\" Q0K=\"15\" S18=\" OBFCA      1000   INF        0                  FAIL PAX "
        "TYPE\"/>"
        "<MSG N06=\"X\" Q0K=\"16\" S18=\" OBFCA      1050   ITF        0                  FAIL PAX "
        "TYPE\"/>"
        "<MSG N06=\"X\" Q0K=\"17\" S18=\" OBFCA      1100   JNF        0                  FAIL PAX "
        "TYPE\"/>"
        "<MSG N06=\"X\" Q0K=\"18\" S18=\" OBFCA      1150              0                  FAIL "
        "FARE BASIS\"/>"
        "<MSG N06=\"X\" Q0K=\"19\" S18=\" OBFCA      1650              0                  FAIL "
        "FARE BASIS\"/>"
        "<MSG N06=\"X\" Q0K=\"20\" S18=\" OBFCA      2650              0       670510     FAIL "
        "SECUR T183\"/>"
        "<MSG N06=\"X\" Q0K=\"21\" S18=\" OBFCA      2700              0       677262     FAIL "
        "SECUR T183\"/>"
        "<MSG N06=\"X\" Q0K=\"22\" S18=\" OBFCA      2950         2.2000PCT               FAIL "
        "SECUR T183\"/>"
        "<MSG N06=\"X\" Q0K=\"23\" S18=\" OBFDA    100000   INF   1.0000PCT               FAIL PAX "
        "TYPE\"/>"
        "<MSG N06=\"X\" Q0K=\"24\" S18=\" OBFDA    150000   CNN   2.0000PCT               FAIL PAX "
        "TYPE\"/>"
        "<MSG N06=\"X\" Q0K=\"25\" S18=\" OBFDA    200000         3.0000PCT               PASS\"/>";

    CPPUNIT_ASSERT_EQUAL(expected, XMLConvertUtils::prepareResponseText(rowDiag));
  }

  void testFormatWpaWtfrResponse_0surface()
  {
    AltPricingTrx* altPricing = formatWpanWtfrSetup(3, 0);
    std::string response = XMLConvertUtils::formatWpaWtfrResponse(altPricing);

    std::string expectResponse = "<MultiPricingResponse></MultiPricingResponse>";
    CPPUNIT_ASSERT_EQUAL(expectResponse, response);
  }

  void testFormatWpaWtfrResponse_1surface()
  {
    AltPricingTrx* altPricing = formatWpanWtfrSetup(3, 1);
    std::string response = XMLConvertUtils::formatWpaWtfrResponse(altPricing);

    CPPUNIT_ASSERT_EQUAL(formatWpanWtfrExpectedError(), response);
  }

  void testFormatWpaWtfrResponse_multiSurface()
  {
    AltPricingTrx* altPricing = formatWpanWtfrSetup(4, 2);
    std::string response = XMLConvertUtils::formatWpaWtfrResponse(altPricing);

    CPPUNIT_ASSERT_EQUAL(formatWpanWtfrExpectedError(), response);
  }

  void testFormatWpanDetailResponse_0surface()
  {
    AltPricingDetailTrx* altPricing = formatWpanWtfrSetup(4, 0);
    std::string response = XMLConvertUtils::formatWpanDetailResponse(altPricing);

    std::string expectedResponse = "<PricingResponse></PricingResponse>";
    CPPUNIT_ASSERT_EQUAL(expectedResponse, response);
  }

  void testFormatWpanDetailResponse_1surface()
  {
    AltPricingDetailTrx* altPricing = formatWpanWtfrSetup(5, 1);
    std::string response = XMLConvertUtils::formatWpanDetailResponse(altPricing);

    CPPUNIT_ASSERT_EQUAL(formatWpanWtfrExpectedError(), response);
  }

  void testFormatWpanDetailResponse_multiSurface()
  {
    AltPricingDetailTrx* altPricing = formatWpanWtfrSetup(4, 3);
    std::string response = XMLConvertUtils::formatWpanDetailResponse(altPricing);

    CPPUNIT_ASSERT_EQUAL(formatWpanWtfrExpectedError(), response);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(XMLConvertUtilsTest);
}
