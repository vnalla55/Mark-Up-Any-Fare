#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <string>

#include "ATAE/ContentServices.h"
#include "Common/Config/ConfigMan.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/Cabin.h"
#include "DBAccess/Customer.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/MultiTransport.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/AltPricingDetailTrx.h"
#include "DataModel/BaseExchangeTrx.h"
#include "DataModel/CurrencyTrx.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/MileageTrx.h"
#include "DataModel/PaxDetail.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/Trx.h"
#include "Server/TseServer.h"
#include "Xform/XformClientXML.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/MockGlobal.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
using boost::assign::operator+=;
namespace
{
const std::string getStringOfCurrentYear()
{
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream stream;
    stream << std::put_time(&tm, "%Y");
    return stream.str();
}
const std::string currentYear = getStringOfCurrentYear();
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

class MockXformClientXML : public XformClientXML
{
public:
  MockXformClientXML(const std::string& name, tse::ConfigMan& config)
    : XformClientXML(name, config), _eraseInx(), _eraseXmlMsgElementCalled()
  {
  }


  bool mockLowerOptionNumber(const PaxDetail* item1, const PaxDetail* item2)
  {
    LowerOptionNumber lon;
    return lon(item1, item2);
  }

  MoneyAmount getTotalAmountPerPax(std::string& message, bool nDec)
  {
    return XformClientXML::getTotalAmountPerPax(message, nDec);
  }

  std::string eraseXmlElement(std::string& message, size_t eraseInx)
  {
    _eraseInx = eraseInx;
    return XformClientXML::eraseXmlElement(message, eraseInx);
  }

  std::string eraseXmlMsgElement(std::string& message)
  {
    _eraseXmlMsgElementCalled = true;
    return XformClientXML::eraseXmlMsgElement(message);
  }

  template <typename T>
  T* createAltPricingDetailTrx(PricingDetailTrx* pricingDetailTrx,
                               DataHandle& dh,
                               bool recordQuote,
                               bool rebook)
  {
    return XformClientXML::createAltPricingDetailTrx<T>(pricingDetailTrx, dh, recordQuote, rebook);
  }

  bool updateC56Tag(std::string& selXml, const MoneyAmount& totals, const CurrencyNoDec& nDec)
  {
    return XformClientXML::updateC56Tag(selXml, totals, nDec);
  }

  bool getSelectionXml(Trx*& trx,
                       const std::vector<PaxDetail*>& paxDetailsOrder,
                       AltPricingTrx::AccompRestrictionInfo* accompRestrictionInfo,
                       std::vector<std::string>& optionXmlVec,
                       PaxDetail* currentPaxDetail,
                       bool rebook,
                       MoneyAmount& totals)
  {
    return XformClientXML::getSelectionXml(trx,
                                           paxDetailsOrder,
                                           accompRestrictionInfo,
                                           optionXmlVec,
                                           currentPaxDetail,
                                           rebook,
                                           totals);
  }

  template <typename T>
  bool processWpaDetailRequest(const std::string& content,
                               DataHandle& dataHandle,
                               const std::vector<uint16_t>& selectionList,
                               Trx*& trx,
                               bool recordQuote,
                               bool rebook)
  {
    return XformClientXML::processWpaDetailRequest<T>(
        content, dataHandle, selectionList, trx, recordQuote, rebook);
  }

  void setDiagArguments(MileageTrx* mileageTrx) const
  {
    return XformClientXML::setDiagArguments(mileageTrx);
  }

  void setupDiag(tse::DataHandle& dataHandle, tse::MileageTrx* mTrx, tse::Trx*& trx) const
  {
    return XformClientXML::setupDiag(dataHandle, mTrx, trx);
  }


  bool findOBbfInContentHeader(const std::string& content)
  {
    return XformClientXML::findOBbfInContentHeader(content);
  }

  PricingDetailTrx*
  prepareDetailTrx(const std::string& content,
                   DataHandle& dataHandle,
                   const std::vector<uint16_t>& selectionList,
                   Trx*& trx,
                   PricingDetailModelMap::WpdfType wpdfType = PricingDetailModelMap::WPDF_NONE)
  {
    return XformClientXML::prepareDetailTrx(content, dataHandle, selectionList, trx, wpdfType);
  }

  size_t _eraseInx;
  bool _eraseXmlMsgElementCalled;
};
}

class XformClientXMLTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(XformClientXMLTest);
  CPPUNIT_TEST(testXformClientXML);
  CPPUNIT_TEST(testLowerOptionNumber);
  CPPUNIT_TEST(testGetTotalAmountPerPax);
  CPPUNIT_TEST(testEraseXmlElement);
  CPPUNIT_TEST(testEraseXmlMsgElement);
  CPPUNIT_TEST(testCreateAltPricingDetailTrx);
  CPPUNIT_TEST(testUpdateC56Tag);
  CPPUNIT_TEST(testGetSelectionXml);
  CPPUNIT_TEST(testProcessWpaDetailRequest);
  CPPUNIT_TEST(testSetDiagArguments_DiagArgDataEmpty);
  CPPUNIT_TEST(testSetDiagArguments_DiagArgDataNotEmpty);
  CPPUNIT_TEST(testSetupDiag_DiagnosticNone);
  CPPUNIT_TEST(testSetupDiag_DiagArgDataEmpty);
  CPPUNIT_TEST(testSetupDiag_DiagArgDataNotEmpty);
  CPPUNIT_TEST(testConvert_MileageTrx);
  CPPUNIT_TEST(testFareDisplayTrx);
  CPPUNIT_TEST(testCurrencyTrx);
  CPPUNIT_TEST(testNoPNRPricingTrx);
  CPPUNIT_TEST(testFormatResponse);
  CPPUNIT_TEST(testFormatBaggageResponse);
  CPPUNIT_TEST(testPBBFail);
  CPPUNIT_TEST(testPBBPass);
  CPPUNIT_TEST(testPBBBrandCodeValid);
  CPPUNIT_TEST(testPBBBrandCodeInvalidLength);
  CPPUNIT_TEST(testPBBBrandCodeNotAlphaNumeric);
  CPPUNIT_TEST(testParse_AncillaryPricingReq);
  CPPUNIT_TEST(testLegIdsOneSegment);
  CPPUNIT_TEST(testLegIdsThreeSegments);
  CPPUNIT_TEST(testLegIdsWithArunkSorted);
  CPPUNIT_TEST(testLegIdsWithArunkUnsorted);
  CPPUNIT_TEST(testLegIdsAllNotSet);
  CPPUNIT_TEST(testLegIdsUnsorted);
  CPPUNIT_TEST(testLegIdsOneNotSet);
  CPPUNIT_TEST(testLegIdsOneSet);
  CPPUNIT_TEST(testLegIdsAssignArunk);
  CPPUNIT_TEST(testLegIdsDiffMoreThenOne);
  CPPUNIT_TEST(testValidateBrandParityValidCase);
  CPPUNIT_TEST(testValidateBrandParityInvalidCase);
  CPPUNIT_TEST(testValidatePbbRequest);
  CPPUNIT_TEST(testValidatePbbRequestDiffFb);
  CPPUNIT_TEST(testValidatePbbRequestDontProcess);
  CPPUNIT_TEST(testValidatePbbRequestDontProcessEmpty);
  CPPUNIT_TEST(testSaveVclInformation);
  CPPUNIT_TEST(testEraseVclInformation);
  CPPUNIT_TEST(testSaveVclInformationMultipleSP);
  CPPUNIT_TEST(testEraseVclInformationMultipleSP);
  CPPUNIT_TEST(testAddVCLinPXI);

  CPPUNIT_TEST(testFindOBbfInContentHeaderObfInMainSection);
  CPPUNIT_TEST(testFindOBbfInContentHeaderMissingObfInMainSection);
  CPPUNIT_TEST(testFindOBbfInContentHeaderObfInSumSection);
  CPPUNIT_TEST_SUITE_END();

  std::string _cfgFileName;
  tse::ConfigMan _aConfig;
  XformClientXML* _testServer;
  TestMemHandle _memHandle;

  std::string getFile(const std::string& filename)
  {
    std::ifstream inFile(filename.c_str());
    std::string content;

    std::string line;

    while (std::getline(inFile, line))
      content += line;

    return content;
  }

  bool initializeConfig()
  {
    _cfgFileName = "xmlConfig.cfg";

    if (!_aConfig.read(_cfgFileName))
    {
      return false;
    }

    return true;
  }

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

  bool selXmlHelper(Trx*& trx,
                    const std::vector<PaxDetail*>& paxDetailsOrder,
                    AltPricingTrx::AccompRestrictionInfo* accompRestrictionInfo,
                    std::vector<std::string>& optionXmlVec,
                    PaxDetail* currentPaxDetail,
                    bool rebook,
                    MoneyAmount& totals)
  {
    ((MockXformClientXML*)_testServer)->_eraseInx = 9999;

    return _testServer->getSelectionXml(trx,
                                        paxDetailsOrder,
                                        accompRestrictionInfo,
                                        optionXmlVec,
                                        currentPaxDetail,
                                        rebook,
                                        totals);
  }

  std::string map2String(std::map<std::string, std::string> m)
  {
    std::string retVal = "{";

    std::map<std::string, std::string>::iterator i = m.begin();
    for (; i != m.end(); ++i)
      retVal += i->first + std::string(" => ") + i->second + ", ";
    retVal += "}";
    return retVal;
  }

  std::string getContent()
  {
    std::string content = "\
  <PricingResponse>\
  <AGI A10=\"MOW\" A20=\"M0XB\" A21=\"M0XB\" A90=\"VOO\" A80=\"M0XB\" C40=\"RUB\" Q01=\"39\" />\
  <BIL A20=\"M0XB\" AE0=\"AA\" AD0=\"448AE8\" C20=\"INTDWPI1\" A22=\"M0XB\" AA0=\"VOO\" A70=\"WPA$P\" />\
  <SUM PBV=\"T\" C56=\"90000\" C40=\"RUB\" D54=\"2\" S69=\"SITI\" AO0=\"MOW\" AF0=\"MOW\" B00=\"SU\" PBC=\"F\" D60=\"23:59\" PAR=\"F\">\
    <PXI B70=\"C09\" C43=\"RUB\" C5E=\"90000\" S85=\"ADT 1 ADT 0 2 1 1 2 JRTRU NMR1 327 J 2 2 2 JRTRU NMR1 327 J\" PBS=\"F\" B71=\"ADT\" Q4P=\"1\" S84=\"FARE\\n\" C40=\"RUB\" C5A=\"90000\" C54=\"23.69\" Q05=\"13\" C46=\"RUB\" C66=\"93323\" C65=\"3323\" C5D=\"0\" C5B=\"0\" C64=\"3323\" S66=\"KEJ SU MOW45000JRTRU SU KEJ45000JRTRU RUB90000END\" P27=\"F\" P26=\"F\">\
    <MSG N06=\"W\" Q0K=\"0\" S18=\"VALIDATING CARRIER - SU\" />\
    <TAX BC0=\"RU\" C6B=\"37\" C40=\"RUB\" S05=\"KEJ\" S04=\"SALES TAX\" C6A=\"1.00\" C41=\"EUR\" A40=\"RU\" />\
    <TBD BC0=\"RU\" C6B=\"37\" C41=\"EUR\" C6A=\"1.00\" A04=\"SU\" S05=\"KEJ\" C40=\"RUB\" A06=\"F\" A40=\"RU\" S04=\"SALES TAX\" />\
    <TBR><CCD C41=\"EUR\" C42=\"RUB\" C54=\"36.50000009855000\" Q05=\"13\" /></TBR>\
  </PXI> </SUM> </PricingResponse>";

    return content;
  }

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>(&_aConfig);
    _memHandle.create<MyDataHandle>();
    std::string serverName = "TestServer";

    CPPUNIT_ASSERT(initializeConfig() == true);

    _aConfig.setValue("DEFAULT_CONFIG", "../xmlConfig.cfg", "DATAHANDLER_CONFIGS");
    _aConfig.setValue("DETAIL_CONFIG", "../detailXmlConfig.cfg", "DATAHANDLER_CONFIGS");
    _aConfig.setValue("FARE_DISPLAY_CONFIG", "../fareDisplayXmlConfig.cfg", "DATAHANDLER_CONFIGS");
    _aConfig.setValue("TAX_CONFIG", "../taxRequest.cfg", "DATAHANDLER_CONFIGS");
    _aConfig.setValue("MILEAGE_CONFIG", "../mileageRequest.cfg", "DATAHANDLER_CONFIGS");
    _aConfig.setValue("CURRENCY_CONFIG", "../currencyRequest.cfg", "DATAHANDLER_CONFIGS");
    _aConfig.setValue(
        "PRICING_DISPLAY_CONFIG", "../pricingDisplayRequest.cfg", "DATAHANDLER_CONFIGS");
    _aConfig.setValue("PRICING_CONFIG", "../pricingRequest.cfg", "DATAHANDLER_CONFIGS");
    _aConfig.setValue(
        "PRICING_DETAIL_CONFIG", "../pricingDetailRequest.cfg", "DATAHANDLER_CONFIGS");

    _testServer = _memHandle.insert(new MockXformClientXML(serverName, _aConfig));
    CPPUNIT_ASSERT(_testServer != NULL);
    CPPUNIT_ASSERT(_testServer->initialize(0, (char**)0) == true);
  }

  void tearDown() { _memHandle.clear(); }

  void createSegments(PricingTrx& trx)
  {
    for (int nCnt = 0; nCnt < 3; ++nCnt)
    {
      AirSeg* seg = _memHandle.create<AirSeg>();
      seg->fareBasisCode() = "BFFWEB";
      seg->setBrandCode("FL");
      trx.travelSeg().push_back(seg);
    }
  }

  template <class Segment>
  void createSegment(PricingTrx& trx)
  {
    Segment* seg = _memHandle.create<Segment>();
    seg->fareBasisCode() = "BFFWEB";
    seg->setBrandCode("FL");
    trx.travelSeg().push_back(seg);
  }

  bool setSegmentsLegIds(TravelSegPtrVec& segments, std::vector<int16_t> ids)
  {
    if(segments.size() != ids.size())
      return false;

    for(size_t i = 0; i < segments.size(); ++i)
      segments[i]->legId() = ids[i];

    return true;
  }

  bool setSegmentsBrandCodes(TravelSegPtrVec& segments, std::vector<BrandCode> brands)
  {
    if(segments.size() != brands.size())
      return false;

    for(size_t i = 0; i < segments.size(); ++i)
      segments[i]->setBrandCode(brands[i]);

    return true;
  }

  //---------------------------------------------------------------------
  // tests
  //---------------------------------------------------------------------
  void testXformClientXML()
  {
    std::string aRequest = getFile("request02BD09.xml");
    CPPUNIT_ASSERT(!aRequest.empty());

    Trx* trx = nullptr;
    DataHandle dataHandle;
    CPPUNIT_ASSERT(_testServer->convert(dataHandle, aRequest, trx, false) == true);

    ContentServices* contentServices;
    try
    {
      PricingTrx& pricingTrx = dynamic_cast<PricingTrx&>(*trx);

      PricingRequest* request = pricingTrx.getRequest();
      request->globalDirection() = GlobalDirection::DO;

      contentServices = _memHandle.create<ContentServices>();
      CPPUNIT_ASSERT(contentServices != NULL);
    }
    catch (...)
    {
      CPPUNIT_ASSERT(false);
    }

    std::string aResponse;
    CPPUNIT_ASSERT(_testServer->convert(*trx, aResponse) == true);

    std::vector<std::string> dataFiles;

    dataFiles.push_back("pricing_req_test1.xml");
    dataFiles.push_back("pricing_req_test2.xml");

    int i = 0;
    for (const std::string& request : dataFiles)
    {
      std::string aRequest = getFile(request);
      CPPUNIT_ASSERT(!aRequest.empty());
      Trx* trx = nullptr;
      CPPUNIT_ASSERT(_testServer->convert(dataHandle, aRequest, trx, false) == true);
      std::string aResponse;
      CPPUNIT_ASSERT(_testServer->convert(*trx, aResponse) == true);
      ++i;
    }
  }

  void testLowerOptionNumber()
  {
    PaxDetail p1, p2;

    MockXformClientXML* server = (MockXformClientXML*)_testServer;

    p1.wpnOptionNumber() = 5;
    p2.wpnOptionNumber() = 3;

    CPPUNIT_ASSERT(server->mockLowerOptionNumber(&p2, &p1));
    CPPUNIT_ASSERT(!server->mockLowerOptionNumber(&p1, &p2));
    CPPUNIT_ASSERT(!server->mockLowerOptionNumber(NULL, &p2));
    CPPUNIT_ASSERT(!server->mockLowerOptionNumber(&p1, NULL));
    CPPUNIT_ASSERT(!server->mockLowerOptionNumber(NULL, NULL));
  }

  void testGetTotalAmountPerPax()
  {
    std::string message = "<SUM PBT=\"F\" PBV=\"F\" C56=\"909.56\" C40=\"EUR\"";

    CPPUNIT_ASSERT(_testServer->getTotalAmountPerPax(message, false) == 909.56);
    CPPUNIT_ASSERT(_testServer->getTotalAmountPerPax(message, true) == 2);

    message = "<SUM PBT=\"F\" PBV=\"F\" C56=\"12345.8765\" C40=\"EUR\"";
    CPPUNIT_ASSERT(_testServer->getTotalAmountPerPax(message, false) == 12345.8765);
    CPPUNIT_ASSERT(_testServer->getTotalAmountPerPax(message, true) == 4);

    message = "<SUM PBT=\"F\" PBV=\"F\" C56=\"1\" C40=\"EUR\"";
    CPPUNIT_ASSERT(_testServer->getTotalAmountPerPax(message, false) == 1);
    CPPUNIT_ASSERT(_testServer->getTotalAmountPerPax(message, true) == 0);

    message = "<SUM PBT=\"F\" PBV=\"F\" C57=\"12345.8765\" C40=\"EUR\"";
    CPPUNIT_ASSERT(_testServer->getTotalAmountPerPax(message, false) == 0);
    CPPUNIT_ASSERT(_testServer->getTotalAmountPerPax(message, true) == 0);

    message = "<SUM PBT=\"F\" PBV=\"F\" C56=\".99\" C40=\"EUR\"";
    CPPUNIT_ASSERT(_testServer->getTotalAmountPerPax(message, false) == 0.99);
    CPPUNIT_ASSERT(_testServer->getTotalAmountPerPax(message, true) == 2);
  }

  void testEraseXmlElement()
  {
    std::string message =
        "<SUM PBT=\"T\"><PXI B70=\"DIS\" S84=\"PSG TY\" P26=\"F\"><MSG N06=\"M\"></PXI></SUM>";

    std::string m = message;
    std::string expected = "<SUM PBT=\"T\"><PXI B70=\"DIS\"  P26=\"F\"><MSG N06=\"M\"></PXI></SUM>";
    CPPUNIT_ASSERT(_testServer->eraseXmlElement(m, 0) == expected);

    m = message;
    expected = "<SUM PBT=\"T\"><PXI B70=\"DIS\"  P26=\"F\"><MSG N06=\"M\"></PXI>";
    CPPUNIT_ASSERT(_testServer->eraseXmlElement(m, 1) == expected);

    m = message;
    expected = "<PXI B70=\"DIS\"  P26=\"F\"><MSG N06=\"M\"></PXI>";
    CPPUNIT_ASSERT(_testServer->eraseXmlElement(m, 2) == expected);

    m = message;
    expected = "<PXI B70=\"DIS\"  P26=\"F\"><MSG N06=\"M\"></PXI></SUM>";
    CPPUNIT_ASSERT(_testServer->eraseXmlElement(m, 3) == expected);
  }

  std::string getXmlMessage(std::vector<std::string>& xmlLines, std::string& msgToDelete, int pos)
  {
    std::string ret;

    for (unsigned int i = 0; i < xmlLines.size(); i++)
    {
      ret += xmlLines[i];
      if ((int)i == pos)
        ret += msgToDelete;
    }

    return ret;
  }

  void testEraseXmlMsgElement()
  {
    std::string msgToDelete = "<MSG N06=\"W\" Q0K=\"0\" S18=\"APPLICABLE BOOKING CLASS - QQQ\" />";
    msgToDelete +=
        "<MSG N06=\"W\" Q0K=\"0\" S18=\"REBOOK OPTION OF CHOICE BEFORE STORING FARE\" />";

    std::vector<std::string> xmlLines;
    xmlLines.push_back("<SUM PBV=\"F\" C56=\"13800\" D16=\"2008-08-27\" PBC=\"F\" D60=\"23:59\">");
    xmlLines.push_back(
        "<PXI B70=\"ADT\" C43=\"RUB\" C5E=\"13800\" SU KEJ6900VFAM RUB13800END\" P27=\"T\">");

    xmlLines.push_back("<MSG N06=\"N\" Q0K=\"0\" B00=\"SU\" S18=\"SU ONLY\" />");
    xmlLines.push_back("<MSG N06=\"W\" Q0K=\"0\" S18=\"VALIDATING CARRIER - SU\" />");
    xmlLines.push_back("</PXI></SUM>");

    std::string origXml = getXmlMessage(xmlLines, msgToDelete, -1);
    std::string ret = _testServer->eraseXmlMsgElement(origXml);
    CPPUNIT_ASSERT_EQUAL(std::string(""), ret);

    // try different locations for msgToDelete
    for (int i = 2; i < 5; i++)
    {
      std::string xml = getXmlMessage(xmlLines, msgToDelete, i);
      ret = _testServer->eraseXmlMsgElement(xml);
      CPPUNIT_ASSERT_EQUAL(origXml, ret);
    }
  }

  void testCreateAltPricingDetailTrx()
  {
    PricingDetailTrx trx;
    DataHandle dh;

    Agent agent;
    agent.agentDuty() = "testDuty";
    agent.vendorCrsCode() = "testCode";
    trx.ticketingAgent() = agent;

    Billing billing;
    billing.userStation() = "testStation";
    trx.billing() = &billing;

    AltPricingDetailTrx* altTrx =
        _testServer->createAltPricingDetailTrx<AltPricingDetailTrx>(&trx, dh, true, false);

    CPPUNIT_ASSERT_EQUAL(true, altTrx->getOptions()->isRecordQuote());
    CPPUNIT_ASSERT_EQUAL('T', altTrx->getOptions()->recordQuote());
    CPPUNIT_ASSERT_EQUAL(false, altTrx->rebook());
    CPPUNIT_ASSERT_EQUAL(std::string("testDuty"),
                         altTrx->getRequest()->ticketingAgent()->agentDuty());
    CPPUNIT_ASSERT_EQUAL(std::string("testCode"), altTrx->vendorCrsCode());
    CPPUNIT_ASSERT_EQUAL(std::string("testStation"), altTrx->billing()->userStation());

    altTrx = _testServer->createAltPricingDetailTrx<AltPricingDetailTrx>(&trx, dh, false, true);

    CPPUNIT_ASSERT_EQUAL(false, altTrx->getOptions()->isRecordQuote());
    CPPUNIT_ASSERT_EQUAL('F', altTrx->getOptions()->recordQuote());
    CPPUNIT_ASSERT_EQUAL(true, altTrx->rebook());
  }

  void testUpdateC56Tag()
  {
    std::string xml = "<SUM PBT=\"F\" PBV=\"F\" C56=\"909.56\" C40=\"EUR\">";
    MoneyAmount m(123.57);
    CurrencyNoDec c(3);

    CPPUNIT_ASSERT(_testServer->updateC56Tag(xml, m, c));
    CPPUNIT_ASSERT_EQUAL(std::string("<SUM PBT=\"F\" PBV=\"F\" C56=\"123.570\" C40=\"EUR\">"), xml);

    xml = "<SUM PBT=\"F\" PBV=\"F\" C56=\"1\" C40=\"EUR\">";
    CPPUNIT_ASSERT(_testServer->updateC56Tag(xml, m, c));
    CPPUNIT_ASSERT_EQUAL(std::string("<SUM PBT=\"F\" PBV=\"F\" C56=\"123.570\" C40=\"EUR\">"), xml);

    c = 2;
    CPPUNIT_ASSERT(_testServer->updateC56Tag(xml, m, c));
    CPPUNIT_ASSERT_EQUAL(std::string("<SUM PBT=\"F\" PBV=\"F\" C56=\"123.57\" C40=\"EUR\">"), xml);

    c = 1;
    CPPUNIT_ASSERT(_testServer->updateC56Tag(xml, m, c));
    CPPUNIT_ASSERT_EQUAL(std::string("<SUM PBT=\"F\" PBV=\"F\" C56=\"123.6\" C40=\"EUR\">"), xml);

    c = 0;
    CPPUNIT_ASSERT(_testServer->updateC56Tag(xml, m, c));
    CPPUNIT_ASSERT_EQUAL(std::string("<SUM PBT=\"F\" PBV=\"F\" C56=\"124\" C40=\"EUR\">"), xml);

    xml = "<SUM PBT=\"F\" PBV=\"F\" C5=\"909.56\" C40=\"EUR\">";
    CPPUNIT_ASSERT(!_testServer->updateC56Tag(xml, m, c));

    xml = "<SUM PBT=\"F\" PBV=\"F\" C56=\"909.56";
    CPPUNIT_ASSERT(!_testServer->updateC56Tag(xml, m, c));
  }

  void testGetSelectionXml()
  {
    std::vector<PaxDetail*> pdo;
    AltPricingTrx::AccompRestrictionInfo accInfo;
    std::vector<std::string> optionXmlVec;
    MoneyAmount m(0);
    PaxDetail p1;

    MockXformClientXML* server = (MockXformClientXML*)_testServer;

    pdo.push_back(&p1);
    optionXmlVec.push_back("test");
    Trx* trx = 0;

    CPPUNIT_ASSERT(!selXmlHelper(trx, pdo, &accInfo, optionXmlVec, &p1, false, m));

    p1.wpnOptionNumber() = 2;
    p1.accTvlData() = "testData";
    CPPUNIT_ASSERT(!selXmlHelper(trx, pdo, &accInfo, optionXmlVec, &p1, false, m));
    CPPUNIT_ASSERT_EQUAL(std::string("testData"), accInfo.validationStr());
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), accInfo.selectionNumber());

    PaxDetail p2;
    pdo.push_back(&p2);
    optionXmlVec.push_back("C56=\"111.11\"");
    CPPUNIT_ASSERT(selXmlHelper(trx, pdo, &accInfo, optionXmlVec, &p1, false, m));

    CPPUNIT_ASSERT_EQUAL(size_t(1), server->_eraseInx);
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(111.11), m);

    p2.wpnOptionNumber() = 3;
    optionXmlVec.push_back("C56=\"222.22\"");
    CPPUNIT_ASSERT(selXmlHelper(trx, pdo, &accInfo, optionXmlVec, &p2, false, m));

    CPPUNIT_ASSERT_EQUAL(size_t(3), server->_eraseInx);
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(111.11) + MoneyAmount(222.22), m);

    PaxDetail p3;
    pdo.push_back(&p3);
    CPPUNIT_ASSERT(selXmlHelper(trx, pdo, &accInfo, optionXmlVec, &p2, false, m));
    CPPUNIT_ASSERT_EQUAL(size_t(2), server->_eraseInx);

    server->_eraseXmlMsgElementCalled = false;
    bool rebook = true;
    CPPUNIT_ASSERT(selXmlHelper(trx, pdo, &accInfo, optionXmlVec, &p2, rebook, m));
    CPPUNIT_ASSERT_EQUAL(true, server->_eraseXmlMsgElementCalled);
  }

  void testProcessWpaDetailRequest()
  {
    std::string content;
    DataHandle dh;
    Trx* trx = 0;
    std::vector<uint16_t> selection;
    selection.push_back(1);

    PricingDetailTrx* pricingDetailTrx = _testServer->prepareDetailTrx(content, dh, selection, trx);
    CPPUNIT_ASSERT(!_testServer->processWpaDetailRequest<AltPricingDetailTrx>(
        pricingDetailTrx, content, dh, trx, false, false));

    content = getContent();
    pricingDetailTrx = _testServer->prepareDetailTrx(content, dh, selection, trx);
    CPPUNIT_ASSERT(_testServer->processWpaDetailRequest<AltPricingDetailTrx>(
        pricingDetailTrx, content, dh, trx, false, false));
    CPPUNIT_ASSERT(trx != 0);

    AltPricingDetailTrx* altTrx = dynamic_cast<AltPricingDetailTrx*>(trx);
    CPPUNIT_ASSERT(altTrx != 0);

    CPPUNIT_ASSERT_EQUAL(size_t(1), altTrx->accompRestrictionVec().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), altTrx->paxDetails().size());

    PaxDetail* paxDetail = altTrx->paxDetails().front();

    CPPUNIT_ASSERT(paxDetail->paxType() == "C09");
  }

  void testSetDiagArguments_DiagArgDataEmpty()
  {
    MileageTrx mTrx;
    PricingRequest request;
    mTrx.setRequest(&request);
    _testServer->setDiagArguments(&mTrx);
    CPPUNIT_ASSERT_EQUAL(size_t(0), mTrx.diagnostic().diagParamMap().size());
  }

  void testSetDiagArguments_DiagArgDataNotEmpty()
  {
    MileageTrx mTrx;
    PricingRequest request;
    request.diagArgData().push_back(std::string("AAA"));
    request.diagArgData().push_back(std::string("BB"));
    request.diagArgData().push_back(std::string("C"));
    request.diagArgData().push_back(std::string(""));
    mTrx.setRequest(&request);
    _testServer->setDiagArguments(&mTrx);
    CPPUNIT_ASSERT_EQUAL(std::string("{AA => A, BB => , }"),
                         map2String(mTrx.diagnostic().diagParamMap()));
  }

  void testSetupDiag_DiagnosticNone()
  {
    DataHandle dataHandle;
    MileageTrx mTrx;
    Trx* trx = nullptr;
    PricingRequest request;
    request.diagArgData().push_back(std::string("AAA"));
    request.diagArgData().push_back(std::string("BB"));
    request.diagArgData().push_back(std::string("C"));
    request.diagArgData().push_back(std::string(""));
    mTrx.setRequest(&request);
    _testServer->setupDiag(dataHandle, &mTrx, trx);
    CPPUNIT_ASSERT_EQUAL(size_t(0), mTrx.diagnostic().diagParamMap().size());
  }

  void testSetupDiag_DiagArgDataEmpty()
  {
    DataHandle dataHandle;
    MileageTrx mTrx;
    Trx* trx = nullptr;
    PricingRequest request;
    request.diagnosticNumber() = Diagnostic452;
    mTrx.setRequest(&request);
    _testServer->setupDiag(dataHandle, &mTrx, trx);
    CPPUNIT_ASSERT_EQUAL(size_t(0), mTrx.diagnostic().diagParamMap().size());
  }

  void testSetupDiag_DiagArgDataNotEmpty()
  {
    DataHandle dataHandle;
    MileageTrx mTrx;
    Trx* trx = nullptr;
    PricingRequest request;
    request.diagnosticNumber() = Diagnostic452;
    mTrx.setRequest(&request);
    request.diagArgData().push_back(std::string("AAA"));
    request.diagArgData().push_back(std::string("BB"));
    request.diagArgData().push_back(std::string("C"));
    request.diagArgData().push_back(std::string(""));
    _testServer->setupDiag(dataHandle, &mTrx, trx);
    CPPUNIT_ASSERT(mTrx.diagnostic().isActive());
    CPPUNIT_ASSERT_EQUAL(size_t(2), mTrx.diagnostic().diagParamMap().size());
  }

  void testConvert_MileageTrx()
  {
    DataHandle dataHandle;
    std::string requestXML(
        "XXXXXXXX<MileageRequest><AGI A10=\"DFW\" A20=\"B4T0\" A21=\"B4T0\" AB0=\"9999999\" "
        "AB1=\"9999999\" A90=\"-A8\" N0G=\"*\" A80=\"B4T0\" B00=\"1S\" C40=\"USD\" Q01=\"34\" "
        "/><BIL A20=\"B4T0\" Q03=\"5642\" Q02=\"6148\" AE0=\"AA\" AD0=\"02BD02\" A22=\"B4T0\" "
        "AA0=\"-A8\" C20=\"INTDWPI1\" A70=\"WN\" /><PRO C10=\"452\" /><DIG Q0A=\"31\" "
        "S01=\"SQ100\" /><OPT D01=\"09SEP09\" PBL=\"T\" /><WNI Q41=\"01\" A11=\"SAO\" B00=\"JJ\" "
        "Q0G=\"T\" /><WNI Q41=\"02\" A11=\"NYC\" B00=\"LH\" /><WNI Q41=\"03\" A11=\"FRA\" "
        "B00=\"LH\" Q0G=\"T\" /><WNI Q41=\"04\" A11=\"NYC\" B00=\"JJ\" /><WNI Q41=\"05\" "
        "A11=\"SAO\" B00=\"JJ\" Q0G=\"T\" /></MileageRequest>");
    Trx* trx = nullptr;
    _testServer->convert(dataHandle, requestXML, trx, false);
    MileageTrx* mTrx = dynamic_cast<MileageTrx*>(trx);
    CPPUNIT_ASSERT(mTrx->diagnostic().isActive());
  }
  void testFareDisplayTrx()
  {
    DataHandle dataHandle;
    std::string requestXML("XXXXXXXX<FAREDISPLAYREQUEST A01=\"DFW\" A02=\"LON\" D01=\"" + currentYear + "-01-10\" "
                           "D06=\"" + currentYear + "-01-10\" D54=\"0491\" S58=\"FQ\"><BIL A20=\"HDQ\" "
                           "A22=\"7TN8\" A70=\"FQDFW\" AA0=\"YIJ\" AD0=\"463629\" AE0=\"AA\" "
                           "C00=\"5786055900509132220\" C20=\"FAREDSP1\" S0R=\"PLIB\" Q02=\"3470\" "
                           "Q03=\"925\"/><PRO P80=\"T\" Q16=\"0\" PCF=\"F\" Q40=\"1\" "
                           "S15=\"CRSAGT\"/><PCL B01=\"AA\"/><AGI A10=\"PNQ\" A20=\"7TN8\" "
                           "A21=\"7TN8\" A80=\"7TN8\" A90=\"YIJ\" AB0=\"9999999\" AB1=\"9999999\" "
                           "B00=\"1S\" C40=\"USD\" N0G=\"*\" Q01=\"34\"/></FAREDISPLAYREQUEST>");
    Trx* trx = nullptr;
    _testServer->convert(dataHandle, requestXML, trx, false);
    FareDisplayTrx* fareDisplayTrx = dynamic_cast<FareDisplayTrx*>(trx);
    CPPUNIT_ASSERT(fareDisplayTrx->getRequest()->ticketingAgent()->tvlAgencyPCC() == "7TN8");
  }

  void testCurrencyTrx()
  {
    DataHandle dataHandle;
    std::string requestXML(
        "XXXXXXXX<CurrencyConversionRequest><FRT S91=\"ATSEDBLD06A\" S92=\"22222\" /><AGI "
        "A10=\"PNQ\" A20=\"7TN8\" A21=\"7TN8\" AB0=\"9999999\" AB1=\"9999999\" A90=\"YIJ\" "
        "N0G=\"*\" A80=\"7TN8\" B00=\"1S\" C40=\"USD\" Q01=\"34\" /><BIL S0R=\"PSS\" A20=\"7TN8\" "
        "Q03=\"925\" Q02=\"3470\" AE0=\"AA\" AD0=\"6B1CCD\" A22=\"7TN8\" AA0=\"YIJ\" "
        "C20=\"INTDWPI1\" A70=\"DC$CA\" /><OPT D01=\"" + currentYear + "-01-10\" N1V=\"DC\" N1F=\"C\" C46=\"CAD\" "
        "C52=\"200.50\" C42=\"USD\" /></CurrencyConversionRequest>");
    Trx* trx = nullptr;
    _testServer->convert(dataHandle, requestXML, trx, false);
    CurrencyTrx* currencyTrx = dynamic_cast<CurrencyTrx*>(trx);
    CPPUNIT_ASSERT(currencyTrx->getRequest()->ticketingAgent()->tvlAgencyPCC() == "7TN8");
  }
  void testNoPNRPricingTrx()
  {
    DataHandle dataHandle;
    std::string requestXML(
        "XXXXXXXX<NoPNRPricingRequest><AGI A10=\"PNQ\" A20=\"7TN8\" A21=\"7TN8\" A80=\"7TN8\" "
        "A90=\"YIJ\" AB0=\"9999999\" AB1=\"9999999\" B00=\"1S\" C40=\"USD\" N0G=\"*\" Q01=\"34\" "
        "/><BIL A20=\"HDQ\" A22=\"7TN8\" A70=\"WQDFW\" AA0=\"YIJ\" AD0=\"34E86E\" AE0=\"AA\" "
        "C00=\"8329911712615029039\" C20=\"INTLWQPR\" S0R=\"LBTY\" Q02=\"3470\" Q03=\"925\" /><PRO "
        "C45=\"USD\" D07=\"" + currentYear + "-01-10\" D54=\"1188\" PBK=\"T\" PBZ=\"F\" N08=\"B\" Q0P=\"1\" "
        "P0J=\"T\" S14=\"WQDFW/ADLPNQ$IPA-ATSEDBLD04A:22222\" S15=\"CRSAGT\" /><PXI B70=\"ADT\" "
        "Q0U=\"01\" /><SGI Q0C=\"01\" ><FLI Q0C=\"01\" N03=\"O\" B00=\"DL\" A01=\"DFW\" "
        "A02=\"PNQ\" D00=\"" + currentYear + "-01-10\" D30=\"1128\" /></SGI></NoPNRPricingRequest>");
    Trx* trx = nullptr;
    _testServer->convert(dataHandle, requestXML, trx, false);
    NoPNRPricingTrx* noPNRPricingTrx = dynamic_cast<NoPNRPricingTrx*>(trx);
    CPPUNIT_ASSERT(noPNRPricingTrx->getRequest()->ticketingAgent()->tvlAgencyPCC() == "7TN8");
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

    _testServer->formatResponse(tmpResponse, xmlResponse);

    CPPUNIT_ASSERT_EQUAL(expected, xmlResponse);
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

    CPPUNIT_ASSERT_EQUAL(expected, _testServer->formatBaggageResponse(baggageResponse, recNum));
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

  void testPBBFail()
  {
    DataHandle dataHandle;
    Trx* trx = 0;

    std::string requestXML(
        "XXXXXXXX<PricingRequest><AGI A10=\"PNQ\" A20=\"7TN8\" A21=\"7TN8\" AB0=\"1435761\" "
        "AB1=\"1435761\" A90=\"VOO\" N0G=\"*\" A80=\"7TN8\" B00=\"1B\" C40=\"INR\" Q01=\"16\" "
        "AE0=\"1B\" /><BIL A20=\"7TN8\" Q03=\"925\" Q02=\"2902\" AE0=\"AA\" AD0=\"46B1F9\" "
        "A22=\"7TN8\" AA0=\"VOO\" C20=\"INTLWPI1\" C01=\"423215464837755138\" A70=\"WP\" /><PRO "
        "PBB=\"T\" C45=\"INR\" D07=\"" + currentYear + "-08-21\" D54=\"0024\" PBZ=\"F\" N08=\"B\" Q0P=\"1\" "
        "P0J=\"T\" S14=\"WP+\" /><PXI B70=\"ADT\" Q0U=\"04\" /><SGI Q0C=\"01\" ><FLI Q0C=\"01\" "
        "N03=\"A\" B00=\"SQ\" B01=\"SQ\" A01=\"CCU\" A02=\"SIN\" B30=\"N\" D01=\"" + currentYear + "-09-29\" "
        "Q0B=\"517\" D02=\"" + currentYear + "-09-30\" D00=\"" + currentYear + "-08-21\" D31=\"1430\" D32=\"0390\" D30=\"0024\" "
        "BB2=\"SS\" BB0=\"OK\" /></SGI><SGI Q0C=\"02\" ><FLI Q0C=\"02\" N03=\"A\" B00=\"SQ\" "
        "B01=\"SQ\" A01=\"SIN\" A02=\"CCU\" B30=\"N\" D01=\"" + currentYear + "-10-15\" Q0B=\"516\" "
        "D02=\"" + currentYear + "-10-15\" D00=\"" + currentYear + "-08-21\" D31=\"1265\" D32=\"1355\" D30=\"0024\" BB2=\"GK\" "
        "BB0=\"OK\" /></SGI><RES ><IDS B00=\"SQ \" Q0B=\"517\" B30=\"N \" D01=\"" + currentYear + "-09-29\" "
        "D31=\"2350\" A01=\"CCU\" D02=\"" + currentYear + "-09-30\" D32=\"0630\" A02=\"SIN\" A70=\"SS\" "
        "Q0U=\"04\" N0V=\"J\" P2X=\"EN\" /><IDS B00=\"SQ \" Q0B=\"516\" B30=\"N \" "
        "D01=\"" + currentYear + "-10-15\" D31=\"2105\" A01=\"SIN\" D02=\"" + currentYear + "-10-15\" D32=\"2235\" A02=\"CCU\" "
        "A70=\"PK\" Q0U=\"04\" N0V=\"J\" P2X=\"ET\" /></RES></PricingRequest>");

    try
    {
      CPPUNIT_ASSERT(_testServer->convert(dataHandle, requestXML, trx, false));
    }
    catch (ErrorResponseException& ex)
    {
      CPPUNIT_ASSERT(ex.code() == ErrorResponseException::INVALID_INPUT);
      CPPUNIT_ASSERT(ex.message() == "MISSING BRAND CODE");
    }
  }

  void testPBBPass()
  {
    DataHandle dataHandle;
    Trx* trx = 0;

    std::string requestXML(
        "XXXXXXXX<PricingRequest><AGI A10=\"PNQ\" A20=\"7TN8\" A21=\"7TN8\" AB0=\"1435761\" "
        "AB1=\"1435761\" A90=\"VOO\" N0G=\"*\" A80=\"7TN8\" B00=\"1B\" C40=\"INR\" Q01=\"16\" "
        "AE0=\"1B\" /><BIL A20=\"7TN8\" Q03=\"925\" Q02=\"2902\" AE0=\"AA\" AD0=\"46B1F9\" "
        "A22=\"7TN8\" AA0=\"VOO\" C20=\"INTLWPI1\" C01=\"423215464837755138\" A70=\"WP\" /><PRO "
        "PBB=\"T\" C45=\"INR\" D07=\"" + currentYear + "-08-21\" D54=\"0024\" PBZ=\"F\" N08=\"B\" Q0P=\"1\" "
        "P0J=\"T\" S14=\"WP+\" /><PXI B70=\"ADT\" Q0U=\"04\" /><SGI Q0C=\"01\" SB2=\"Apple\">><FLI "
        "Q0C=\"01\" N03=\"A\" B00=\"SQ\" B01=\"SQ\" A01=\"CCU\" A02=\"SIN\" B30=\"N\" "
        "D01=\"" + currentYear + "-09-29\" Q0B=\"517\" D02=\"" + currentYear + "-09-30\" D00=\"" + currentYear + "-08-21\" D31=\"1430\" "
        "D32=\"0390\" D30=\"0024\" BB2=\"SS\" BB0=\"OK\" /></SGI><SGI Q0C=\"02\" ><FLI Q0C=\"02\" "
        "N03=\"A\" B00=\"SQ\" B01=\"SQ\" A01=\"SIN\" A02=\"CCU\" B30=\"N\" D01=\"" + currentYear + "-10-15\" "
        "Q0B=\"516\" D02=\"" + currentYear + "-10-15\" D00=\"" + currentYear + "-08-21\" D31=\"1265\" D32=\"1355\" D30=\"0024\" "
        "BB2=\"GK\" BB0=\"OK\" /></SGI><RES ><IDS B00=\"SQ \" Q0B=\"517\" B30=\"N \" "
        "D01=\"" + currentYear + "-09-29\" D31=\"2350\" A01=\"CCU\" D02=\"" + currentYear + "-09-30\" D32=\"0630\" A02=\"SIN\" "
        "A70=\"SS\" Q0U=\"04\" N0V=\"J\" P2X=\"EN\" /><IDS B00=\"SQ \" Q0B=\"516\" B30=\"N \" "
        "D01=\"" + currentYear + "-10-15\" D31=\"2105\" A01=\"SIN\" D02=\"" + currentYear + "-10-15\" D32=\"2235\" A02=\"CCU\" "
        "A70=\"PK\" Q0U=\"04\" N0V=\"J\" P2X=\"ET\" /></RES></PricingRequest>");

    CPPUNIT_ASSERT(_testServer->convert(dataHandle, requestXML, trx, false));
  }

  void testPBBBrandCodeValid()
  {
    DataHandle dataHandle;
    Trx* trx = 0;

    std::string requestXML(
        "XXXXXXXX<PricingRequest><AGI A10=\"PNQ\" A20=\"7TN8\" A21=\"7TN8\" AB0=\"1435761\" "
        "AB1=\"1435761\" A90=\"VOO\" N0G=\"*\" A80=\"7TN8\" B00=\"1B\" C40=\"INR\" Q01=\"16\" "
        "AE0=\"1B\" /><BIL A20=\"7TN8\" Q03=\"925\" Q02=\"2902\" AE0=\"AA\" AD0=\"46B1F9\" "
        "A22=\"7TN8\" AA0=\"VOO\" C20=\"INTLWPI1\" C01=\"423215464837755138\" A70=\"WP\" /><PRO "
        "PBB=\"T\" C45=\"INR\" D07=\"" + currentYear + "-08-21\" D54=\"0024\" PBZ=\"F\" N08=\"B\" Q0P=\"1\" "
        "P0J=\"T\" S14=\"WP+\" /><PXI B70=\"ADT\" Q0U=\"04\" /><SGI Q0C=\"01\" SB2=\"XX\">><FLI "
        "Q0C=\"01\" N03=\"A\" B00=\"SQ\" B01=\"SQ\" A01=\"CCU\" A02=\"SIN\" B30=\"N\" "
        "D01=\"" + currentYear + "-09-29\" Q0B=\"517\" D02=\"" + currentYear + "-09-30\" D00=\"" + currentYear + "-08-21\" D31=\"1430\" "
        "D32=\"0390\" D30=\"0024\" BB2=\"SS\" BB0=\"OK\" /></SGI><SGI Q0C=\"02\" ><FLI Q0C=\"02\" "
        "N03=\"A\" B00=\"SQ\" B01=\"SQ\" A01=\"SIN\" A02=\"CCU\" B30=\"N\" D01=\"" + currentYear + "-10-15\" "
        "Q0B=\"516\" D02=\"" + currentYear + "-10-15\" D00=\"" + currentYear + "-08-21\" D31=\"1265\" D32=\"1355\" D30=\"0024\" "
        "BB2=\"GK\" BB0=\"OK\" /></SGI><RES ><IDS B00=\"SQ \" Q0B=\"517\" B30=\"N \" "
        "D01=\"" + currentYear + "-09-29\" D31=\"2350\" A01=\"CCU\" D02=\"" + currentYear + "-09-30\" D32=\"0630\" A02=\"SIN\" "
        "A70=\"SS\" Q0U=\"04\" N0V=\"J\" P2X=\"EN\" /><IDS B00=\"SQ \" Q0B=\"516\" B30=\"N \" "
        "D01=\"" + currentYear + "-10-15\" D31=\"2105\" A01=\"SIN\" D02=\"" + currentYear + "-10-15\" D32=\"2235\" A02=\"CCU\" "
        "A70=\"PK\" Q0U=\"04\" N0V=\"J\" P2X=\"ET\" /></RES></PricingRequest>");

    CPPUNIT_ASSERT_NO_THROW(_testServer->convert(dataHandle, requestXML, trx, false));
  }

  void testPBBBrandCodeInvalidLength()
  {
    DataHandle dataHandle;
    Trx* trx = 0;

    std::string requestXML(
        "XXXXXXXX<PricingRequest><AGI A10=\"PNQ\" A20=\"7TN8\" A21=\"7TN8\" AB0=\"1435761\" "
        "AB1=\"1435761\" A90=\"VOO\" N0G=\"*\" A80=\"7TN8\" B00=\"1B\" C40=\"INR\" Q01=\"16\" "
        "AE0=\"1B\" /><BIL A20=\"7TN8\" Q03=\"925\" Q02=\"2902\" AE0=\"AA\" AD0=\"46B1F9\" "
        "A22=\"7TN8\" AA0=\"VOO\" C20=\"INTLWPI1\" C01=\"423215464837755138\" A70=\"WP\" /><PRO "
        "PBB=\"T\" C45=\"INR\" D07=\"" + currentYear + "-08-21\" D54=\"0024\" PBZ=\"F\" N08=\"B\" Q0P=\"1\" "
        "P0J=\"T\" S14=\"WP+\" /><PXI B70=\"ADT\" Q0U=\"04\" /><SGI Q0C=\"01\" SB2=\"A\">><FLI "
        "Q0C=\"01\" N03=\"A\" B00=\"SQ\" B01=\"SQ\" A01=\"CCU\" A02=\"SIN\" B30=\"N\" "
        "D01=\"" + currentYear + "-09-29\" Q0B=\"517\" D02=\"" + currentYear + "-09-30\" D00=\"" + currentYear + "-08-21\" D31=\"1430\" "
        "D32=\"0390\" D30=\"0024\" BB2=\"SS\" BB0=\"OK\" /></SGI><SGI Q0C=\"02\" ><FLI Q0C=\"02\" "
        "N03=\"A\" B00=\"SQ\" B01=\"SQ\" A01=\"SIN\" A02=\"CCU\" B30=\"N\" D01=\"" + currentYear + "-10-15\" "
        "Q0B=\"516\" D02=\"" + currentYear + "-10-15\" D00=\"" + currentYear + "-08-21\" D31=\"1265\" D32=\"1355\" D30=\"0024\" "
        "BB2=\"GK\" BB0=\"OK\" /></SGI><RES ><IDS B00=\"SQ \" Q0B=\"517\" B30=\"N \" "
        "D01=\"" + currentYear + "-09-29\" D31=\"2350\" A01=\"CCU\" D02=\"" + currentYear + "-09-30\" D32=\"0630\" A02=\"SIN\" "
        "A70=\"SS\" Q0U=\"04\" N0V=\"J\" P2X=\"EN\" /><IDS B00=\"SQ \" Q0B=\"516\" B30=\"N \" "
        "D01=\"" + currentYear + "-10-15\" D31=\"2105\" A01=\"SIN\" D02=\"" + currentYear + "-10-15\" D32=\"2235\" A02=\"CCU\" "
        "A70=\"PK\" Q0U=\"04\" N0V=\"J\" P2X=\"ET\" /></RES></PricingRequest>");

    CPPUNIT_ASSERT_THROW(_testServer->convert(dataHandle, requestXML, trx, false),
                         ErrorResponseException);
  }

  void testPBBBrandCodeNotAlphaNumeric()
  {
    DataHandle dataHandle;
    Trx* trx = 0;

    std::string requestXML(
        "XXXXXXXX<PricingRequest><AGI A10=\"PNQ\" A20=\"7TN8\" A21=\"7TN8\" AB0=\"1435761\" "
        "AB1=\"1435761\" A90=\"VOO\" N0G=\"*\" A80=\"7TN8\" B00=\"1B\" C40=\"INR\" Q01=\"16\" "
        "AE0=\"1B\" /><BIL A20=\"7TN8\" Q03=\"925\" Q02=\"2902\" AE0=\"AA\" AD0=\"46B1F9\" "
        "A22=\"7TN8\" AA0=\"VOO\" C20=\"INTLWPI1\" C01=\"423215464837755138\" A70=\"WP\" /><PRO "
        "PBB=\"T\" C45=\"INR\" D07=\"" + currentYear + "-08-21\" D54=\"0024\" PBZ=\"F\" N08=\"B\" Q0P=\"1\" "
        "P0J=\"T\" S14=\"WP+\" /><PXI B70=\"ADT\" Q0U=\"04\" /><SGI Q0C=\"01\" SB2=\"A-B\">><FLI "
        "Q0C=\"01\" N03=\"A\" B00=\"SQ\" B01=\"SQ\" A01=\"CCU\" A02=\"SIN\" B30=\"N\" "
        "D01=\"" + currentYear + "-09-29\" Q0B=\"517\" D02=\"" + currentYear + "-09-30\" D00=\"" + currentYear + "-08-21\" D31=\"1430\" "
        "D32=\"0390\" D30=\"0024\" BB2=\"SS\" BB0=\"OK\" /></SGI><SGI Q0C=\"02\" ><FLI Q0C=\"02\" "
        "N03=\"A\" B00=\"SQ\" B01=\"SQ\" A01=\"SIN\" A02=\"CCU\" B30=\"N\" D01=\"" + currentYear + "-10-15\" "
        "Q0B=\"516\" D02=\"" + currentYear + "-10-15\" D00=\"" + currentYear + "-08-21\" D31=\"1265\" D32=\"1355\" D30=\"0024\" "
        "BB2=\"GK\" BB0=\"OK\" /></SGI><RES ><IDS B00=\"SQ \" Q0B=\"517\" B30=\"N \" "
        "D01=\"" + currentYear + "-09-29\" D31=\"2350\" A01=\"CCU\" D02=\"" + currentYear + "-09-30\" D32=\"0630\" A02=\"SIN\" "
        "A70=\"SS\" Q0U=\"04\" N0V=\"J\" P2X=\"EN\" /><IDS B00=\"SQ \" Q0B=\"516\" B30=\"N \" "
        "D01=\"" + currentYear + "-10-15\" D31=\"2105\" A01=\"SIN\" D02=\"" + currentYear + "-10-15\" D32=\"2235\" A02=\"CCU\" "
        "A70=\"PK\" Q0U=\"04\" N0V=\"J\" P2X=\"ET\" /></RES></PricingRequest>");

    CPPUNIT_ASSERT_THROW(_testServer->convert(dataHandle, requestXML, trx, false),
                         ErrorResponseException);
  }

  void testParse_AncillaryPricingReq()
  {
    DataHandle dataHandle;
    Trx* trx = 0;

    const char* xmlRequest = "<AncillaryPricingReq></AncillaryPricingReq>";
    CPPUNIT_ASSERT_THROW(_testServer->parse(xmlRequest, dataHandle, trx), ErrorResponseException);
  }

  void testLegIdsOneSegment()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    std::string errorMessage;
    trx->setPbbRequest(PBB_RQ_PROCESS_BRANDS);
    createSegment<AirSeg>(*trx);
    CPPUNIT_ASSERT(setSegmentsLegIds(trx->travelSeg(), {0}));
    CPPUNIT_ASSERT(_testServer->validateLegIds(trx, errorMessage));
    CPPUNIT_ASSERT(errorMessage.empty());
  }

  void testLegIdsThreeSegments()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    std::string errorMessage;
    trx->setPbbRequest(PBB_RQ_PROCESS_BRANDS);
    createSegment<AirSeg>(*trx);
    createSegment<AirSeg>(*trx);
    createSegment<AirSeg>(*trx);
    CPPUNIT_ASSERT(setSegmentsLegIds(trx->travelSeg(), {0, 1, 2}));
    CPPUNIT_ASSERT(_testServer->validateLegIds(trx, errorMessage));
    CPPUNIT_ASSERT(errorMessage.empty());
  }

  void testLegIdsWithArunkSorted()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    std::string errorMessage;
    trx->setPbbRequest(PBB_RQ_PROCESS_BRANDS);
    createSegment<AirSeg>(*trx);
    createSegment<ArunkSeg>(*trx);
    createSegment<AirSeg>(*trx);
    CPPUNIT_ASSERT(setSegmentsLegIds(trx->travelSeg(), {0, 1, 1}));
    CPPUNIT_ASSERT(_testServer->validateLegIds(trx, errorMessage));
    CPPUNIT_ASSERT(errorMessage.empty());
  }

  void testLegIdsWithArunkUnsorted()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    std::string errorMessage;
    trx->setPbbRequest(PBB_RQ_PROCESS_BRANDS);
    createSegment<AirSeg>(*trx);
    createSegment<ArunkSeg>(*trx);
    createSegment<AirSeg>(*trx);
    CPPUNIT_ASSERT(setSegmentsLegIds(trx->travelSeg(), {1, 0, 2}));
    CPPUNIT_ASSERT(!_testServer->validateLegIds(trx, errorMessage));
    CPPUNIT_ASSERT_EQUAL(errorMessage, std::string("INVALID LEG ID: 1"));
  }

  void testLegIdsAllNotSet()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    std::string errorMessage;
    trx->setPbbRequest(PBB_RQ_PROCESS_BRANDS);
    createSegment<AirSeg>(*trx);
    createSegment<AirSeg>(*trx);
    createSegment<AirSeg>(*trx);
    CPPUNIT_ASSERT(_testServer->validateLegIds(trx, errorMessage));
    CPPUNIT_ASSERT(errorMessage.empty());
  }

  void testLegIdsUnsorted()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    std::string errorMessage;
    trx->setPbbRequest(PBB_RQ_PROCESS_BRANDS);
    createSegment<AirSeg>(*trx);
    createSegment<AirSeg>(*trx);
    createSegment<AirSeg>(*trx);
    CPPUNIT_ASSERT(setSegmentsLegIds(trx->travelSeg(), {1, 0, 1}));
    CPPUNIT_ASSERT(!_testServer->validateLegIds(trx, errorMessage));
    CPPUNIT_ASSERT_EQUAL(errorMessage, std::string("INVALID LEG ID: 1"));
  }

  void testLegIdsDiffMoreThenOne()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    std::string errorMessage;
    trx->setPbbRequest(PBB_RQ_PROCESS_BRANDS);
    createSegment<AirSeg>(*trx);
    createSegment<AirSeg>(*trx);
    createSegment<AirSeg>(*trx);

    CPPUNIT_ASSERT(setSegmentsLegIds(trx->travelSeg(), {1, 4, 4}));
    CPPUNIT_ASSERT(!_testServer->validateLegIds(trx, errorMessage));
    CPPUNIT_ASSERT_EQUAL(errorMessage, std::string("INVALID LEG ID: 5"));

    CPPUNIT_ASSERT(setSegmentsLegIds(trx->travelSeg(), {1, 1, 3}));
    CPPUNIT_ASSERT(!_testServer->validateLegIds(trx, errorMessage));
    CPPUNIT_ASSERT_EQUAL(errorMessage, std::string("INVALID LEG ID: 4"));
  }

  void testLegIdsOneNotSet()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    std::string errorMessage;
    trx->setPbbRequest(PBB_RQ_PROCESS_BRANDS);
    createSegment<AirSeg>(*trx);
    createSegment<AirSeg>(*trx);
    createSegment<AirSeg>(*trx);

    CPPUNIT_ASSERT(setSegmentsLegIds(trx->travelSeg(), {-1, 0, 1}));
    CPPUNIT_ASSERT(!_testServer->validateLegIds(trx, errorMessage));
    CPPUNIT_ASSERT_EQUAL(errorMessage, std::string("INCOMPLETE LEG INFO"));

    CPPUNIT_ASSERT(setSegmentsLegIds(trx->travelSeg(), {0, -1, 1}));
    CPPUNIT_ASSERT(!_testServer->validateLegIds(trx, errorMessage));
    CPPUNIT_ASSERT_EQUAL(errorMessage, std::string("INCOMPLETE LEG INFO"));

    CPPUNIT_ASSERT(setSegmentsLegIds(trx->travelSeg(), {0, 0, -1}));
    CPPUNIT_ASSERT(!_testServer->validateLegIds(trx, errorMessage));
    CPPUNIT_ASSERT_EQUAL(errorMessage, std::string("INCOMPLETE LEG INFO"));
  }

  void testLegIdsOneSet()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    std::string errorMessage;
    trx->setPbbRequest(PBB_RQ_PROCESS_BRANDS);
    createSegment<AirSeg>(*trx);
    createSegment<AirSeg>(*trx);
    createSegment<AirSeg>(*trx);

    CPPUNIT_ASSERT(setSegmentsLegIds(trx->travelSeg(), {1, -1, -1}));
    CPPUNIT_ASSERT(!_testServer->validateLegIds(trx, errorMessage));
    CPPUNIT_ASSERT_EQUAL(errorMessage, std::string("INCOMPLETE LEG INFO"));

    CPPUNIT_ASSERT(setSegmentsLegIds(trx->travelSeg(), {-1, 0, -1}));
    CPPUNIT_ASSERT(!_testServer->validateLegIds(trx, errorMessage));
    CPPUNIT_ASSERT_EQUAL(errorMessage, std::string("INCOMPLETE LEG INFO"));

    CPPUNIT_ASSERT(setSegmentsLegIds(trx->travelSeg(), {-1, -1, 0}));
    CPPUNIT_ASSERT(!_testServer->validateLegIds(trx, errorMessage));
    CPPUNIT_ASSERT_EQUAL(errorMessage, std::string("INCOMPLETE LEG INFO"));
  }

  void testLegIdsAssignArunk()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    std::string errorMessage;
    trx->setPbbRequest(PBB_RQ_PROCESS_BRANDS);
    createSegment<AirSeg>(*trx);
    createSegment<ArunkSeg>(*trx);
    createSegment<AirSeg>(*trx);
    createSegment<ArunkSeg>(*trx);
    createSegment<AirSeg>(*trx);
    CPPUNIT_ASSERT(setSegmentsLegIds(trx->travelSeg(), {0, -1, 1, -1, 2}));
    _testServer->initArunkSegLegIds(trx->travelSeg());
    CPPUNIT_ASSERT(trx->travelSeg().at(1)->legId() == 0);
    CPPUNIT_ASSERT(trx->travelSeg().at(3)->legId() == 1);
  }

  void testValidateBrandParityValidCase()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    std::string errorMessage;
    trx->setPbbRequest(PBB_RQ_PROCESS_BRANDS);
    createSegment<AirSeg>(*trx);
    createSegment<AirSeg>(*trx);
    createSegment<AirSeg>(*trx);
    CPPUNIT_ASSERT(setSegmentsLegIds(trx->travelSeg(), {0, 1, 1}));
    CPPUNIT_ASSERT(setSegmentsBrandCodes(trx->travelSeg(), {"FL", "BZ", "BZ"}));
    CPPUNIT_ASSERT(_testServer->validateBrandParity(trx->travelSeg()));
    CPPUNIT_ASSERT(_testServer->validateLegIds(trx, errorMessage));
    CPPUNIT_ASSERT(errorMessage.empty());
  }

  void testValidateBrandParityInvalidCase()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    std::string errorMessage;
    trx->setPbbRequest(PBB_RQ_PROCESS_BRANDS);
    createSegment<AirSeg>(*trx);
    createSegment<AirSeg>(*trx);
    createSegment<AirSeg>(*trx);
    CPPUNIT_ASSERT(setSegmentsLegIds(trx->travelSeg(), {0, 1, 1}));
    CPPUNIT_ASSERT(setSegmentsBrandCodes(trx->travelSeg(), {"FL", "BZ", "FL"}));
    CPPUNIT_ASSERT(!_testServer->validateBrandParity(trx->travelSeg()));
    CPPUNIT_ASSERT(!_testServer->validateLegIds(trx, errorMessage));
    CPPUNIT_ASSERT_EQUAL(errorMessage, std::string("ONLY ONE BRAND PER LEG ALLOWED"));
  }

  void testValidatePbbRequest()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setPbbRequest(PBB_RQ_PROCESS_BRANDS);
    trx->setRequest(_memHandle.create<PricingRequest>());
    trx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    createSegments(*trx);
    std::string str;

    CPPUNIT_ASSERT(_testServer->validatePbbRequest(trx, str));
    CPPUNIT_ASSERT(trx->isPbbRequest() == PBB_RQ_DONT_PROCESS_BRANDS);
  }

  void testValidatePbbRequestDiffFb()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setPbbRequest(PBB_RQ_PROCESS_BRANDS);
    trx->setRequest(_memHandle.create<PricingRequest>());
    trx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    createSegments(*trx);
    trx->travelSeg().back()->fareBasisCode() = "";
    std::string str;

    CPPUNIT_ASSERT(_testServer->validatePbbRequest(trx, str));
    CPPUNIT_ASSERT(trx->isPbbRequest() == PBB_RQ_DONT_PROCESS_BRANDS);
  }

  void testValidatePbbRequestDontProcess()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setPbbRequest(PBB_RQ_PROCESS_BRANDS);
    trx->setRequest(_memHandle.create<PricingRequest>());
    trx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    createSegments(*trx);
    trx->travelSeg().back()->setBrandCode("EC");
    std::string str;

    CPPUNIT_ASSERT(!_testServer->validatePbbRequest(trx, str));
    CPPUNIT_ASSERT(str == "INVALID INPUT FORMAT");
  }

  void testValidatePbbRequestDontProcessEmpty()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setPbbRequest(PBB_RQ_PROCESS_BRANDS);
    trx->setRequest(_memHandle.create<PricingRequest>());
    trx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    createSegments(*trx);
    trx->travelSeg().back()->setBrandCode("");
    std::string str;

    CPPUNIT_ASSERT(!_testServer->validatePbbRequest(trx, str));
    CPPUNIT_ASSERT(str == "INVALID INPUT FORMAT");
  }
  void testSaveVclInformation()
  {
    PaxDetail paxDetail;
    PaxDetail* p1 = &paxDetail;
    Trx* trx = _memHandle.create<PricingTrx>();
    std::string message = "<SUM PBV=\"T\" C56=\"3585.00\" C40=\"SGD\" D07=\"" + currentYear + "-10-23\" "
                          "D54=\"611\" S69=\"SOTO\" AO0=\"TYO\" AF0=\"TYO\" B00=\"BA\" "
                          "D00=\"" + currentYear + "-11-06\" D14=\"" + currentYear + "-11-06\" D16=\"" + currentYear + "-11-06\" D60=\"23:59\" "
                          "PBC=\"F\" PAR=\"F\" S79=\"atsedbld04b\"><VCL P3L=\"F\" SM0=\"BSP\" "
                          "VC0=\"T\"><DCX B00=\"9W\" TT0=\"ETKTREQ\"><PCX B00=\"BA\" "
                          "VC1=\"STD\"/></DCX></VCL><PXI B70=\"ADT\" C43=\"NUC\" C5E=\"2850.43\" ";

    _testServer->saveVclInformation(p1, message, trx );

    std::string expectResponse = "<VCL P3L=\"F\" SM0=\"BSP\" VC0=\"T\"><DCX B00=\"9W\" "
                                 "TT0=\"ETKTREQ\"><PCX B00=\"BA\" VC1=\"STD\"/></DCX></VCL>";
    CPPUNIT_ASSERT_EQUAL(expectResponse, p1->vclInfo());
  }

  void testEraseVclInformation()
  {
    Trx* trx = _memHandle.create<PricingTrx>();
    std::string selectionXml =
        "<SUM PBV=\"T\" C56=\"3585.00\" C40=\"SGD\" D07=\"" + currentYear + "-10-23\" D54=\"611\" S69=\"SOTO\" "
        "AO0=\"TYO\" AF0=\"TYO\" B00=\"BA\" D00=\"" + currentYear + "-11-06\" D14=\"" + currentYear + "-11-06\" "
        "D16=\"" + currentYear + "-11-06\" D60=\"23:59\" PBC=\"F\" PAR=\"F\" S79=\"atsedbld04b\"><VCL P3L=\"F\" "
        "SM0=\"BSP\" VC0=\"T\"><DCX B00=\"9W\" TT0=\"ETKTREQ\"><PCX B00=\"BA\" "
        "VC1=\"STD\"/></DCX></VCL><PXI B70=\"ADT\" C43=\"NUC\" C5E=\"2850.43\" ";

    _testServer->eraseVclInformation(selectionXml, trx);

    std::string expectResponse = "<SUM PBV=\"T\" C56=\"3585.00\" C40=\"SGD\" D07=\"" + currentYear + "-10-23\" "
                                 "D54=\"611\" S69=\"SOTO\" AO0=\"TYO\" AF0=\"TYO\" B00=\"BA\" "
                                 "D00=\"" + currentYear + "-11-06\" D14=\"" + currentYear + "-11-06\" D16=\"" + currentYear + "-11-06\" "
                                 "D60=\"23:59\" PBC=\"F\" PAR=\"F\" S79=\"atsedbld04b\"><PXI "
                                 "B70=\"ADT\" C43=\"NUC\" C5E=\"2850.43\" ";

    CPPUNIT_ASSERT_EQUAL(expectResponse, selectionXml);
  }
//-------------------------

  void testSaveVclInformationMultipleSP()
  {
    PaxDetail paxDetail;
    PaxDetail* p1 = &paxDetail;
    Trx* trx = _memHandle.create<PricingTrx>();
    std::string message = "<SUM PBV=\"T\" C56=\"3585.00\" C40=\"SGD\" D07=\"" + currentYear + "-10-23\" "
                          "D54=\"611\" S69=\"SOTO\" AO0=\"TYO\" AF0=\"TYO\" B00=\"BA\" "
                          "D00=\"" + currentYear + "-11-06\" D14=\"" + currentYear + "-11-06\" D16=\"" + currentYear + "-11-06\" D60=\"23:59\" "
                          "PBC=\"F\" PAR=\"F\" S79=\"atsedbld04b\">"
                          "<VCL P3L=\"F\" SM0=\"ARC\" VC0=\"T\"><DCX B00=\"9W\" TT0=\"ETKTPREF\"><PCX B00=\"BA\" VC1=\"STD\"/></DCX></VCL>"
                          "<VCL P3L=\"F\" SM0=\"SAT\" VC0=\"T\"><DCX B00=\"9W\" TT0=\"ETKTREQ\"><PCX B00=\"BA\" VC1=\"STD\"/></DCX></VCL>"
                          "<PXI B70=\"ADT\" C43=\"NUC\" C5E=\"2850.43\" ";

    _testServer->saveVclInformation(p1, message, trx );

    std::string expectResponse = "<VCL P3L=\"F\" SM0=\"ARC\" VC0=\"T\"><DCX B00=\"9W\" TT0=\"ETKTPREF\"><PCX B00=\"BA\" VC1=\"STD\"/></DCX></VCL>"
                                 "<VCL P3L=\"F\" SM0=\"SAT\" VC0=\"T\"><DCX B00=\"9W\" TT0=\"ETKTREQ\"><PCX B00=\"BA\" VC1=\"STD\"/></DCX></VCL>";
    CPPUNIT_ASSERT_EQUAL(expectResponse, p1->vclInfo());
  }

  void testEraseVclInformationMultipleSP()
  {
    Trx* trx = _memHandle.create<PricingTrx>();
    std::string selectionXml =
        "<SUM PBV=\"T\" C56=\"3585.00\" C40=\"SGD\" D07=\"" + currentYear + "-10-23\" D54=\"611\" S69=\"SOTO\" "
        "AO0=\"TYO\" AF0=\"TYO\" B00=\"BA\" D00=\"" + currentYear + "-11-06\" D14=\"" + currentYear + "-11-06\" "
        "D16=\"" + currentYear + "-11-06\" D60=\"23:59\" PBC=\"F\" PAR=\"F\" S79=\"atsedbld04b\">"
        "<VCL P3L=\"F\" SM0=\"ARC\" VC0=\"T\"><DCX B00=\"9W\" TT0=\"ETKTPREF\"><PCX B00=\"BA\" VC1=\"STD\"/></DCX></VCL>"
        "<VCL P3L=\"F\" SM0=\"SAT\" VC0=\"T\"><DCX B00=\"9W\" TT0=\"ETKTREQ\"><PCX B00=\"BA\" VC1=\"STD\"/></DCX></VCL>"
        "<PXI B70=\"ADT\" C43=\"NUC\" C5E=\"2850.43\" ";

    _testServer->eraseVclInformation(selectionXml, trx);

    std::string expectResponse = "<SUM PBV=\"T\" C56=\"3585.00\" C40=\"SGD\" D07=\"" + currentYear + "-10-23\" "
                                 "D54=\"611\" S69=\"SOTO\" AO0=\"TYO\" AF0=\"TYO\" B00=\"BA\" "
                                 "D00=\"" + currentYear + "-11-06\" D14=\"" + currentYear + "-11-06\" D16=\"" + currentYear + "-11-06\" "
                                 "D60=\"23:59\" PBC=\"F\" PAR=\"F\" S79=\"atsedbld04b\"><PXI "
                                 "B70=\"ADT\" C43=\"NUC\" C5E=\"2850.43\" ";

    CPPUNIT_ASSERT_EQUAL(expectResponse, selectionXml);
  }
  void testAddVCLinPXI()
  {
    std::string vclInfo = "<VCL P3L=\"F\" SM0=\"BSP\" VC0=\"T\"><DCX B00=\"9W\" "
                          "TT0=\"ETKTREQ\"><PCX B00=\"BA\" VC1=\"STD\"/></DCX></VCL>";
    std::string selectionXml = "<SUM PBV=\"T\" C56=\"3585.00\" C40=\"SGD\" D07=\"" + currentYear + "-10-23\" "
                               "D54=\"611\" S69=\"SOTO\" AO0=\"TYO\" AF0=\"TYO\" B00=\"BA\" "
                               "D00=\"" + currentYear + "-11-06\" D14=\"" + currentYear + "-11-06\" D16=\"" + currentYear + "-11-06\" "
                               "D60=\"23:59\" PBC=\"F\" PAR=\"F\" S79=\"atsedbld04b\"><PXI "
                               "B70=\"ADT\" </PXI>";
    ;
    _testServer->addVCLinPXI(selectionXml, vclInfo);
    std::string expectResponse = "<SUM PBV=\"T\" C56=\"3585.00\" C40=\"SGD\" D07=\"" + currentYear + "-10-23\" "
                                 "D54=\"611\" S69=\"SOTO\" AO0=\"TYO\" AF0=\"TYO\" B00=\"BA\" "
                                 "D00=\"" + currentYear + "-11-06\" D14=\"" + currentYear + "-11-06\" D16=\"" + currentYear + "-11-06\" "
                                 "D60=\"23:59\" PBC=\"F\" PAR=\"F\" S79=\"atsedbld04b\"><PXI "
                                 "B70=\"ADT\" <VCL P3L=\"F\" SM0=\"BSP\" VC0=\"T\"><DCX B00=\"9W\" "
                                 "TT0=\"ETKTREQ\"><PCX B00=\"BA\" VC1=\"STD\"/></DCX></VCL></PXI>";

    CPPUNIT_ASSERT_EQUAL(expectResponse, selectionXml);
  }

  void testFindOBbfInContentHeaderObfInMainSection()
  {
    const std::string request = getFile("selectionReqObfInMainSection.xml");

    CPPUNIT_ASSERT(_testServer->findOBbfInContentHeader(request));
  }

  void testFindOBbfInContentHeaderMissingObfInMainSection()
  {
    const std::string request = getFile("selectionReqMissingObfInMainSection.xml");

    CPPUNIT_ASSERT(!_testServer->findOBbfInContentHeader(request));
  }

  void testFindOBbfInContentHeaderObfInSumSection()
  {
    const std::string request = getFile("selectionReqObfInSumSection.xml");

    CPPUNIT_ASSERT(!_testServer->findOBbfInContentHeader(request));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(XformClientXMLTest);
}
