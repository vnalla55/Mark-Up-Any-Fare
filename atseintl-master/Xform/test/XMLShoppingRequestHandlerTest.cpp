#include <type_traits>
#include <vector>

#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "Common/ErrorResponseException.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/ClassOfService.h"
#include "Common/Thread/PriorityQueueTimerTaskExecutor.h"
#include "DataModel/Billing.h"
#include "DataModel/FlexFares/GroupsData.h"
#include "DataModel/FlexFares/Types.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Cabin.h"
#include "DBAccess/Customer.h"
#include "DBAccess/MultiTransport.h"
#include "DBAccess/RBDByCabinInfo.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestPricingTrxFactory.h"
#include "test/testdata/TestShoppingTrxFactory.h"
#include "Xform/CustomXMLParser/IParser.h"
#include "Xform/CustomXMLParser/IXMLSchema.h"
#include "Xform/CustomXMLParser/IXMLUtils.h"
#include "Xform/ShoppingSchemaNames.h"
#include "Xform/XMLShoppingHandler.h"
#include "Diagnostic/DiagnosticUtil.h"

const bool
_bCheckWellFormedness(false);

namespace tse
{
FALLBACKVALUE_DECL(fallbackVITA4NonBSP);

using namespace boost::assign;
using namespace shopping;

namespace
{
ILookupMap _elemLookupMap, _attrLookupMap;
bool
bInit(IXMLUtils::initLookupMaps(shoppingElementNames,
                                _NumberElementNames_,
                                _elemLookupMap,
                                shoppingAttributeNames,
                                _NumberAttributeNames_,
                                _attrLookupMap));

using TnShoppingBrandingModeUT = std::underlying_type<TnShoppingBrandingMode>::type;
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
  const std::vector<FareCalcConfig*>& getFareCalcConfig(const Indicator& userApplType,
                                                        const UserApplCode& userAppl,
                                                        const PseudoCityCode& pseudoCity)
  {
    if (userAppl == "SABR" || pseudoCity == "W0H3")
      return DataHandleMock::getFareCalcConfig(' ', "", "");
    return DataHandleMock::getFareCalcConfig(userApplType, userAppl, pseudoCity);
  }
  const std::vector<const PaxTypeMatrix*>& getPaxTypeMatrix(const PaxTypeCode& paxTypeCode)
  {
    if (paxTypeCode == "ADT")
      return *_memHandle.create<std::vector<const PaxTypeMatrix*> >();
    return DataHandleMock::getPaxTypeMatrix(paxTypeCode);
  }
  const Loc* getLoc(const LocCode& locCode, const DateTime& date)
  {
    if (locCode == "")
      return 0;
    return DataHandleMock::getLoc(locCode, date);
  }
  const std::vector<Customer*>& getCustomer(const PseudoCityCode& key)
  {
    if (key == "DFW")
    {
      return *_memHandle.create<std::vector<Customer*> >();
    }
    else if (key == "W0H3")
    {
      std::vector<Customer*>* ret = _memHandle.create<std::vector<Customer*> >();
      Customer* c = _memHandle.create<Customer>();
      ret->push_back(c);
      c->pseudoCity() = key;
      c->arcNo() = "9999999";
      c->lbtCustomerGroup() = "9999999";
      c->branchAccInd() = 'N';
      c->curConvInd() = 'N';
      c->cadSubscriberInd() = 'N';
      c->webSubscriberInd() = 'N';
      c->btsSubscriberInd() = 'N';
      c->sellingFareInd() = 'N';
      c->tvlyInternetSubriber() = 'N';
      c->availabilityIgRul2St() = 'Y';
      c->availabilityIgRul3St() = 'Y';
      c->availIgRul2StWpnc() = 'Y';
      c->activateJourneyPricing() = 'Y';
      c->activateJourneyShopping() = 'Y';
      c->doNotApplySegmentFee() = 'N';
      c->optInAgency() = 'Y';
      c->privateFareInd() = 'Y';
      c->doNotApplyObTktFees() = 'N';
      c->homePseudoCity() = "W0H3";
      c->homeArcNo() = "4553728";
      c->requestCity() = "DFW";
      c->aaCity() = "SAT";
      c->defaultCur() = "USD";
      c->agencyName() = "TRAVELOCITY.COM";
      c->channelId() = 'N';
      c->crsCarrier() = "1S";
      c->ssgGroupNo() = 9;
      c->eTicketCapable() = 'Y';
      c->hostName() = "SABR";
      c->fareQuoteCur() = 'N';
      return *ret;
    }
    return DataHandleMock::getCustomer(key);
  }
  const Cabin* getCabin(const CarrierCode& carrier, const BookingCode& cos, const DateTime& date)
  {
    if (carrier == "AA")
    {
      Cabin* ret = _memHandle.create<Cabin>();
      if (cos == "F")
        ret->cabin().setFirstClass();
      else if (cos == "C")
        ret->cabin().setBusinessClass();
      else if (cos == "B" || cos == "H" || cos == "K" || cos == "M" || cos == "Q" || cos == "V" ||
               cos == "Y")
        ret->cabin().setEconomyClass();
      return ret;
    }
    return DataHandleMock::getCabin(carrier, cos, date);
  }
  const std::vector<RBDByCabinInfo*>& getRBDByCabin(const VendorCode& vendor,
                                                    const CarrierCode& carrier,
                                                    DateTime tvlDate)
    {
      if (carrier == "AA")
      {
        LocKey lk1, lk2;
        lk1.loc() = "MIA";
        lk1.locType() = 'C';
        lk2.loc() = "DFW";
        lk2.locType() = 'C';
        std::vector<RBDByCabinInfo*>* ret = _memHandle.create<std::vector<RBDByCabinInfo*> >();
        RBDByCabinInfo* info = _memHandle.create<RBDByCabinInfo>();
        info->vendor() = "ATP";
        info->carrier() = "AA";
        info->sequenceNo() = 111233;
        info->globalDir() = "";
        info->effDate() = DateTime(2010, 1, 1);
        info->discDate() = DateTime(2025, 12, 31);
        info->createDate() = DateTime(2010, 1, 1);
        info->expireDate() = DateTime(2025, 12, 31);
        info->locKey1() = lk1;
        info->locKey2() = lk2;
        info->firstTicketDate() = DateTime(2010, 1, 2);
        info->lastTicketDate() = DateTime(2025, 12, 31);
        info->flightNo1() = 0;
        info->flightNo2() = 0;
        info->equipmentType() = "";
        info->bookingCodeCabinMap().insert(std::make_pair("F",'F'));
        info->bookingCodeCabinMap().insert(std::make_pair("A",'F'));
        info->bookingCodeCabinMap().insert(std::make_pair("J",'C'));
        info->bookingCodeCabinMap().insert(std::make_pair("C",'C'));
        info->bookingCodeCabinMap().insert(std::make_pair("I",'C'));
        info->bookingCodeCabinMap().insert(std::make_pair("W",'Y'));
        info->bookingCodeCabinMap().insert(std::make_pair("T",'Y'));
        info->bookingCodeCabinMap().insert(std::make_pair("Y",'Y'));
        ret->push_back(info);
        return *ret;
      }
      return DataHandleMock::getRBDByCabin(vendor, carrier, DateTime(2010, 1, 2));
    }
  const std::vector<MultiTransport*>& getMultiTransportCity(const LocCode& locCode,
                                                            const CarrierCode& carrierCode,
                                                            GeoTravelType tvlType,
                                                            const DateTime& tvlDate)
  {
    std::vector<MultiTransport*>& ret = *_memHandle.create<std::vector<MultiTransport*> >();
    if (locCode == "DFW")
    {
      ret += getMT(locCode, "DAL"), getMT(locCode, "DFW"), getMT(locCode, "QDF");
      return ret;
    }
    else if (locCode == "MIA")
      return ret;
    return DataHandleMock::getMultiTransportCity(locCode, carrierCode, tvlType, tvlDate);
  }
  const LocCode getMultiTransportCity(const LocCode& locCode)
  {
    if (locCode == "DFW")
      return "DFW";
    return DataHandleMock::getMultiTransportCity(locCode);
  }
};
}

class MockXMLShoppingHandler : public XMLShoppingHandler
{
public:
  MockXMLShoppingHandler(DataHandle& dataHandle) : XMLShoppingHandler(dataHandle) {}
};

class XMLShoppingHandlerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(XMLShoppingHandlerTest);
  CPPUNIT_TEST(testNoOCFeesRequest);
  CPPUNIT_TEST(testNoOCFeesRequestParse);
  CPPUNIT_TEST(testOCFeesRequestParse);
  CPPUNIT_TEST(testCarnivalSumOfLocalFlag);
  CPPUNIT_TEST(testNumberOfAdditionalItinerariesRequestedBasedOnOWFares);
  CPPUNIT_TEST(testMaxAllowedUsesOfFareCombination);
  CPPUNIT_TEST(testDIVType);
  CPPUNIT_TEST(testBrandedFaresFlagNoset);
  CPPUNIT_TEST(testBrandedFaresFlagFalse);
  CPPUNIT_TEST(testBrandedFaresFlagTrue);
  CPPUNIT_TEST(testValidatingCarrierFlag);
  CPPUNIT_TEST(testValidatingCarrierFlagTrue);
  CPPUNIT_TEST(testValidatingCarrierFlagFalse);
  CPPUNIT_TEST(testAlternateValidatingCarrierFlagTrue);
  CPPUNIT_TEST(testAlternateValidatingCarrierFlagFalse);
  CPPUNIT_TEST(testAlternateValidatingCarrierFlagDefault);
  CPPUNIT_TEST(testFlexFareGroup);
  CPPUNIT_TEST(testParseS15XFFOptionsForFareFocus_S15_Ffocus_False);
  CPPUNIT_TEST(testParseS15XFFOptionsForFareFocus_S15_Ffocus_True);
  CPPUNIT_TEST(testParseS15XFFOptionsForFareFocus_XFF_False);
  CPPUNIT_TEST(testParseS15XFFOptionsForFareFocus_XFF_S15_True);
  CPPUNIT_TEST(testRestrictExcludeFareFocusRule_XFF_NOEPR_Throw);
  CPPUNIT_TEST(testRestrictExcludeFareFocusRule_XFF_EPR_No_Throw);
  CPPUNIT_TEST(testParseTNBrandOptions_BFA);
  CPPUNIT_TEST(testParseTNBrandOptions_BFA_noMIP);
  CPPUNIT_TEST(testParseTNBrandOptions_bothTrueInMIP);
  CPPUNIT_TEST(testParseTNBrandOptions_BFANon);
  CPPUNIT_TEST(testParseTNBrandOptions_BFA_zero);
  CPPUNIT_TEST(testParseTNBrandOptions_BFA_one);
  CPPUNIT_TEST(testParseTNBrandOptions_BFS);
  CPPUNIT_TEST(testParseTNBrandOptions_BFSNon);
  CPPUNIT_TEST(testNewAtpcoRbdByCabinAnswerTable);
  CPPUNIT_TEST(testRBDDiagnosticConversionfromIntellisell);
  CPPUNIT_TEST(testparseS15PDOOptions_PDO_False);
  CPPUNIT_TEST(testparseS15PDOOptions_S15_ORGFQD_True);
  CPPUNIT_TEST(testparseS15PDOOptions_PDO_S15_True);
  CPPUNIT_TEST(testparseS15PDOOptions_PDO_NOEPR_Throw);
  CPPUNIT_TEST(testparseS15PDOOptions_PDO_EPR_No_Throw);
  CPPUNIT_TEST(testparseS15PDROptions_S15_AGYRET_True);
  CPPUNIT_TEST(testparseS15PDROptions_PDR_False);
  CPPUNIT_TEST(testparseS15PDROptions_PDR_S15_True);
  CPPUNIT_TEST(testparseS15PDROptions_PDR_NOEPR_Throw);
  CPPUNIT_TEST(testparseS15PDROptions_PDR_EPR_No_Throw);
  CPPUNIT_TEST(testparseS15XRSOptions_S15_ORGFQD_True);
  CPPUNIT_TEST(testparseS15XRSOptions_XRS_False);
  CPPUNIT_TEST(testparseS15XRSOptions_XRS_S15_True);
  CPPUNIT_TEST(testparseS15XRSOptions_XRS_NOEPR_Throw);
  CPPUNIT_TEST(testparseS15XRSOptions_XRS_EPR_No_Throw);
  CPPUNIT_TEST(testCheckOptionalParametersXRSorPDOorPDR_Error1);
  CPPUNIT_TEST(testCheckOptionalParametersXRSorPDOorPDR_Error2);
  CPPUNIT_TEST(testparseB12B13Attribute_B12EmptyValue);
  CPPUNIT_TEST(testparseB12B13Attribute_B12SingleValue);
  CPPUNIT_TEST(testparseB12B13Attribute_B12MultiValue);
  CPPUNIT_TEST(testparseB12B13Attribute_B13EmptyValue);
  CPPUNIT_TEST(testparseB12B13Attribute_B13SingleValue);
  CPPUNIT_TEST(testparseB12B13Attribute_B13MultiValue);
  CPPUNIT_TEST(testparseB12B13Attribute_B12B13MultiValue);
  CPPUNIT_TEST(testparseXRAType);
  CPPUNIT_TEST(testStoreProcOptsInformation_BI0_CabinRequest);
  CPPUNIT_TEST(testStoreProcOptsInformation_BI0_CabinRequestThrowException);
  CPPUNIT_TEST(testSPV_Valid_IND);
  CPPUNIT_TEST(testSPV_Valid_IND1);
  CPPUNIT_TEST(testSPV_Valid_IND2);
  CPPUNIT_TEST(testSPV_Valid_IND3);
  CPPUNIT_TEST(testSPV_Invalid_IND);
  CPPUNIT_TEST(testSPV_IND_CRC);
  CPPUNIT_TEST(testSPV_IND_CRC_CTC);

  CPPUNIT_TEST_SUITE_END();

  void parse(const std::string& request)
  {
    IValueString attrValueArray[_NumberAttributeNames_];
    int attrRefArray[_NumberAttributeNames_];
    IXMLSchema schema(_elemLookupMap,
                      _attrLookupMap,
                      _NumberAttributeNames_,
                      attrValueArray,
                      attrRefArray,
                      _bCheckWellFormedness);
    IParser parser(request, *_handler, schema);
    parser.parse();
  }

  void parseBuffer(const char* buf, size_t bufSize)
  {
    IValueString attrValueArray[_NumberAttributeNames_];
    int attrRefArray[_NumberAttributeNames_];
    IXMLSchema schema(_elemLookupMap,
                      _attrLookupMap,
                      _NumberAttributeNames_,
                      attrValueArray,
                      attrRefArray,
                      _bCheckWellFormedness);
    IParser parser(buf, bufSize, *_handler, schema);
    parser.parse();
  }

  void parseString(const std::string& request) { parseBuffer(request.c_str(), request.size()); }

  void parseFile(const std::string& request)
  {
    std::ifstream input(request.c_str());
    assert(input);
    input.seekg(0, std::ios::end);
    int bufSize = input.tellg();
    char buf[bufSize + 1];

    input.seekg(0, std::ios::beg);
    input.read(buf, bufSize);
    input.close();
    buf[bufSize] = 0;

    parseBuffer(buf, bufSize);
  }

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();
    _trx = TestShoppingTrxFactory::create("/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    _handler = _memHandle.insert(new XMLShoppingHandler(_dataHandle));
    _handler->_shoppingTrx = _trx;
    _handler->_pricingTrx = _trx;
    _handler->_trxResult = _trx;
    _handler->initTrx();
    _handler->_shoppingTrx = NULL;
    const std::string activationDate = "2014-02-01";
    TestConfigInitializer::setValue(
               "ATPCO_RBDBYCABIN_ANSWER_TABLE_ACTIVATION_DATE", activationDate, "PRICING_SVC");
    DateTime dt(activationDate, 0);
    _trx->ticketingDate() = dt.subtractDays(2);
    Billing * billing = _memHandle.create<Billing>();
    billing->actionCode() = "WPNI.C";
    _trx->billing() = billing;
    TimerTaskExecutor::setInstance(new PriorityQueueTimerTaskExecutor);
  }

  void tearDown()
  {
    _dataHandle.clear();
    _memHandle.clear();
    TimerTaskExecutor::destroyInstance();
  }

  void testNoOCFeesRequest()
  {
    CPPUNIT_ASSERT_EQUAL(false, _trx->getRequest()->isCollectOCFees());
    CPPUNIT_ASSERT_EQUAL((uint16_t)0, (uint16_t)_trx->getOptions()->serviceGroupsVec().size());
  }

  void testNoOCFeesRequestParse()
  {
    parseString("<ShoppingRequest><PRO/></ShoppingRequest>");

    CPPUNIT_ASSERT_EQUAL(false, _trx->getRequest()->isCollectOCFees());
    CPPUNIT_ASSERT_EQUAL((uint16_t)0, (uint16_t)_trx->getOptions()->serviceGroupsVec().size());
  }

  void testOCFeesRequestParse()
  {
    parseString("<ShoppingRequest N06=\"M\"><PRO SEY=\"T\"/><RFG S01=\"AE\"/></ShoppingRequest>");

    CPPUNIT_ASSERT_EQUAL(true, _trx->getRequest()->isCollectOCFees());
    CPPUNIT_ASSERT_EQUAL(true, _trx->getOptions()->isProcessAllGroups());
  }

  void testCarnivalSumOfLocalFlag()
  {
    CPPUNIT_ASSERT_EQUAL(false, _trx->getOptions()->isCarnivalSumOfLocal());

    parseString("<ShoppingRequest><PRO Q0S=\"1\" SLC=\"T\"/></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(true, _trx->getOptions()->isCarnivalSumOfLocal());

    parseString("<ShoppingRequest><PRO Q0S=\"1\" SLC=\"F\"/></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(false, _trx->getOptions()->isCarnivalSumOfLocal());
  }

  void testNumberOfAdditionalItinerariesRequestedBasedOnOWFares()
  {
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(0),
                         _trx->getOptions()->getAdditionalItinsRequestedForOWFares());

    parseString("<ShoppingRequest><PRO Q0S=\"1\" QD1=\"34\"/></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(34),
                         _trx->getOptions()->getAdditionalItinsRequestedForOWFares());
  }

  void testMaxAllowedUsesOfFareCombination()
  {
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(0),
                         _trx->getOptions()->getMaxAllowedUsesOfFareCombination());

    parseString("<ShoppingRequest><PRO Q0S=\"1\" QD2=\"87\"/></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(87),
                         _trx->getOptions()->getMaxAllowedUsesOfFareCombination());
  }

  void testDIVType()
  {
    TestConfigInitializer::setValue(
        "ADDITIONAL_NS_PERCENTAGE_UPPER_LIMIT", "1.0", "SHOPPING_DIVERSITY", true);

    _handler->_shoppingTrx = _trx;
    CPPUNIT_ASSERT_THROW(
        parseString("<ShoppingRequest><DIV OPC=\"1\" PCP=\"0.1\"/></ShoppingRequest>"),
        ErrorResponseException);
    while (!_handler->_parsers.empty())
      _handler->_parsers.pop();
    CPPUNIT_ASSERT_THROW(parseString("<ShoppingRequest><DIV TOD=\"0-0\" /></ShoppingRequest>"),
                         ErrorResponseException);
    while (!_handler->_parsers.empty())
      _handler->_parsers.pop();
    CPPUNIT_ASSERT_THROW(
        parseString("<ShoppingRequest><DIV TOD=\"0-719|720-1440\" /></ShoppingRequest>"),
        ErrorResponseException);
    while (!_handler->_parsers.empty())
      _handler->_parsers.pop();
    CPPUNIT_ASSERT_THROW(parseString("<ShoppingRequest><DIV OPC=\"-5\" /></ShoppingRequest>"),
                         ErrorResponseException);
    while (!_handler->_parsers.empty())
      _handler->_parsers.pop();
    CPPUNIT_ASSERT_THROW(parseString("<ShoppingRequest><DIV OPC=\"0\" /></ShoppingRequest>"),
                         ErrorResponseException);
    while (!_handler->_parsers.empty())
      _handler->_parsers.pop();
    CPPUNIT_ASSERT_THROW(parseString("<ShoppingRequest><DIV FAC=\"0.9\" /></ShoppingRequest>"),
                         ErrorResponseException);
    while (!_handler->_parsers.empty())
      _handler->_parsers.pop();
    CPPUNIT_ASSERT_THROW(parseString("<ShoppingRequest><DIV NSO=\"1.3\" /></ShoppingRequest>"),
                         ErrorResponseException);
    while (!_handler->_parsers.empty())
      _handler->_parsers.pop();
    CPPUNIT_ASSERT_THROW(parseString("<ShoppingRequest><DIV NSO=\"-0.3\" /></ShoppingRequest>"),
                         ErrorResponseException);
    while (!_handler->_parsers.empty())
      _handler->_parsers.pop();
    CPPUNIT_ASSERT_THROW(parseString("<ShoppingRequest><DIV IOP=\"-5\" /></ShoppingRequest>"),
                         ErrorResponseException);
    while (!_handler->_parsers.empty())
      _handler->_parsers.pop();
    CPPUNIT_ASSERT_THROW(parseString("<ShoppingRequest><DIV IOP=\"0\" /></ShoppingRequest>"),
                         ErrorResponseException);
    while (!_handler->_parsers.empty())
      _handler->_parsers.pop();
    CPPUNIT_ASSERT_THROW(parseString("<ShoppingRequest><DIV FLN=\"-5\" /></ShoppingRequest>"),
                         ErrorResponseException);
    while (!_handler->_parsers.empty())
      _handler->_parsers.pop();
    CPPUNIT_ASSERT_THROW(parseString("<ShoppingRequest><DIV FLN=\"0\" /></ShoppingRequest>"),
                         ErrorResponseException);
    while (!_handler->_parsers.empty())
      _handler->_parsers.pop();
    CPPUNIT_ASSERT_THROW(parseString("<ShoppingRequest><DIV FAS=\"0.3\" /></ShoppingRequest>"),
                         ErrorResponseException);
    while (!_handler->_parsers.empty())
      _handler->_parsers.pop();
    CPPUNIT_ASSERT_THROW(parseString("<ShoppingRequest><DIV TTS=\"0.3\" /></ShoppingRequest>"),
                         ErrorResponseException);
    while (!_handler->_parsers.empty())
      _handler->_parsers.pop();
    CPPUNIT_ASSERT_THROW(
        parseString(
            "<ShoppingRequest><DIV TOD=\"0-719|720-1439\" TTD=\"0.1\" /></ShoppingRequest>"),
        ErrorResponseException);
    while (!_handler->_parsers.empty())
      _handler->_parsers.pop();
    CPPUNIT_ASSERT_THROW(
        parseString(
            "<ShoppingRequest><DIV TOD=\"0-719|720-1439\" TTD=\"1.1|0.9\" /></ShoppingRequest>"),
        ErrorResponseException);
    while (!_handler->_parsers.empty())
      _handler->_parsers.pop();
    CPPUNIT_ASSERT_THROW(
        parseString(
            "<ShoppingRequest><DIV TOD=\"0-719|720-1439\" TTD=\"0.1|-0.9\" /></ShoppingRequest>"),
        ErrorResponseException);
    while (!_handler->_parsers.empty())
      _handler->_parsers.pop();
    CPPUNIT_ASSERT_THROW(
        parseString(
            "<ShoppingRequest><DIV TOD=\"0-719|720-1439\" TTD=\"0.1|0.8\" /></ShoppingRequest>"),
        ErrorResponseException);
    while (!_handler->_parsers.empty())
      _handler->_parsers.pop();
    CPPUNIT_ASSERT_THROW(parseString("<ShoppingRequest><DIV PCP=\"1.3\" /></ShoppingRequest>"),
                         ErrorResponseException);
    while (!_handler->_parsers.empty())
      _handler->_parsers.pop();
    CPPUNIT_ASSERT_THROW(parseString("<ShoppingRequest><DIV PCP=\"-0.3\" /></ShoppingRequest>"),
                         ErrorResponseException);
    while (!_handler->_parsers.empty())
      _handler->_parsers.pop();
    CPPUNIT_ASSERT_THROW(parseString("<ShoppingRequest><DIV BKD=\"0.1\" /></ShoppingRequest>"),
                         ErrorResponseException);
    while (!_handler->_parsers.empty())
      _handler->_parsers.pop();
    CPPUNIT_ASSERT_THROW(
        parseString("<ShoppingRequest><DIV BKD=\"1.1|0.2|0.3|0.4\" /></ShoppingRequest>"),
        ErrorResponseException);
    while (!_handler->_parsers.empty())
      _handler->_parsers.pop();
    CPPUNIT_ASSERT_THROW(
        parseString("<ShoppingRequest><DIV BKD=\"0.1|-0.2|0.3|0.4\" /></ShoppingRequest>"),
        ErrorResponseException);
    while (!_handler->_parsers.empty())
      _handler->_parsers.pop();
    CPPUNIT_ASSERT_THROW(
        parseString("<ShoppingRequest><DIV BKD=\"0.2|0.2|0.3|0.4\" /></ShoppingRequest>"),
        ErrorResponseException);
    while (!_handler->_parsers.empty())
      _handler->_parsers.pop();

    CPPUNIT_ASSERT_THROW(parseString("<ShoppingRequest><DIV FRL=\"-3\" /></ShoppingRequest>"),
                         ErrorResponseException);
    while (!_handler->_parsers.empty())
      _handler->_parsers.pop();
    CPPUNIT_ASSERT_THROW(
        parseString("<ShoppingRequest><DIV><CXP PCP=\"0.3\"/></DIV></ShoppingRequest>"),
        ErrorResponseException);
    while (!_handler->_parsers.empty())
      _handler->_parsers.pop();
    CPPUNIT_ASSERT_THROW(parseString("<ShoppingRequest><DIV><CXP B00=\"AA\" PCP=\"0.3\" "
                                     "OPC=\"3\"/></DIV></ShoppingRequest>"),
                         ErrorResponseException);
    while (!_handler->_parsers.empty())
      _handler->_parsers.pop();
    CPPUNIT_ASSERT_THROW(parseString("<ShoppingRequest><DIV><CXP B00=\"AA\" PCP=\"0.3\"/><CXP "
                                     "B00=\"AA\" OPC=\"3\"/></DIV></ShoppingRequest>"),
                         ErrorResponseException);
    while (!_handler->_parsers.empty())
      _handler->_parsers.pop();

    _handler->_shoppingTrx = NULL;
  }

  void testBrandedFaresFlagNoset()
  {
    parseString("<ShoppingRequest><PRO/></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(false, _trx->getRequest()->isBrandedFaresRequest());
  }

  void testBrandedFaresFlagFalse()
  {
    parseString("<ShoppingRequest><PRO BFR=\"F\"/></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(false, _trx->getRequest()->isBrandedFaresRequest());
  }

  void testBrandedFaresFlagTrue()
  {
    parseString("<ShoppingRequest><PRO BFR=\"T\"/></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(true, _trx->getRequest()->isBrandedFaresRequest());
  }

  void testValidatingCarrierFlag()
  {
    CPPUNIT_ASSERT_EQUAL(true, _trx->getRequest()->isValidatingCarrierRequest());
    CPPUNIT_ASSERT_EQUAL(true, _trx->getRequest()->isAlternateValidatingCarrierRequest());
  }

  void testValidatingCarrierFlagTrue()
  {
    parseString("<ShoppingRequest><PRO VCX=\"T\"/></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(true, _trx->getRequest()->isValidatingCarrierRequest());
  }

  void testValidatingCarrierFlagFalse()
  {
    parseString("<ShoppingRequest><PRO VCX=\"F\"/></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(false, _trx->getRequest()->isValidatingCarrierRequest());
  }

  void testAlternateValidatingCarrierFlagTrue()
  {
    parseString("<ShoppingRequest><PRO VCX=\"T\" DVL=\"T\"/></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(true, _trx->getRequest()->isAlternateValidatingCarrierRequest());
  }

  void testAlternateValidatingCarrierFlagFalse()
  {
    parseString("<ShoppingRequest><PRO VCX=\"T\" DVL=\"F\"/></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(false, _trx->getRequest()->isAlternateValidatingCarrierRequest());
  }

  void testAlternateValidatingCarrierFlagDefault()
  {
    parseString("<ShoppingRequest><PRO VCX=\"T\"/></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(true, _trx->getRequest()->isAlternateValidatingCarrierRequest());
  }

  void testParseTNBrandOptions_BFA()
  {
    parseString("<ShoppingRequest N06=\"M\"><PRO BFA=\"10\" BFS=\"T\"/></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(true, _trx->isBRAll());
    CPPUNIT_ASSERT_EQUAL(
        static_cast<TnShoppingBrandingModeUT>(TnShoppingBrandingMode::MULTIPLE_BRANDS),
        static_cast<TnShoppingBrandingModeUT>(_trx->getTnShoppingBrandingMode()));
    CPPUNIT_ASSERT_EQUAL((size_t)10, _trx->getNumberOfBrands());
  }

  void testParseTNBrandOptions_BFA_zero()
  {
    parseString("<ShoppingRequest N06=\"M\"><PRO BFA=\"0\" BFS=\"T\"/></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(true, _trx->isBRAll());
    CPPUNIT_ASSERT_EQUAL(
        static_cast<TnShoppingBrandingModeUT>(TnShoppingBrandingMode::MULTIPLE_BRANDS),
        static_cast<TnShoppingBrandingModeUT>(_trx->getTnShoppingBrandingMode()));
  }

  void testParseTNBrandOptions_BFA_one()
  {
    parseString("<ShoppingRequest N06=\"M\"><PRO BFA=\"1\" BFS=\"T\"/></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(false, _trx->isBRAll());
    CPPUNIT_ASSERT_EQUAL(
        static_cast<TnShoppingBrandingModeUT>(TnShoppingBrandingMode::SINGLE_BRAND),
        static_cast<TnShoppingBrandingModeUT>(_trx->getTnShoppingBrandingMode()));
  }

  void testParseTNBrandOptions_BFA_noMIP()
  {
    parseString("<ShoppingRequest><PRO BFA=\"T\" BFS=\"T\"/></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(false, _trx->isBRAll());
    CPPUNIT_ASSERT_EQUAL(static_cast<TnShoppingBrandingModeUT>(TnShoppingBrandingMode::NO_BRANDS),
                         static_cast<TnShoppingBrandingModeUT>(_trx->getTnShoppingBrandingMode()));
  }

  void testParseTNBrandOptions_bothTrueInMIP()
  {
    parseString("<ShoppingRequest N06=\"M\"><PRO BFA=\"T\" BFS=\"T\"/></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(true, _trx->isBRAll());
    CPPUNIT_ASSERT_EQUAL(static_cast<std::underlying_type<TnShoppingBrandingMode>::type>(
                             TnShoppingBrandingMode::MULTIPLE_BRANDS),
                         static_cast<uint8_t>(_trx->getTnShoppingBrandingMode()));
    CPPUNIT_ASSERT_EQUAL((size_t)TnShoppingBrandsLimit::UNLIMITED_BRANDS,
                         _trx->getNumberOfBrands());
  }

  void testParseTNBrandOptions_BFANon()
  {
    parseString("<ShoppingRequest N06=\"M\"><PRO /></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(false, _trx->isBRAll());
    CPPUNIT_ASSERT_EQUAL(static_cast<TnShoppingBrandingModeUT>(TnShoppingBrandingMode::NO_BRANDS),
                         static_cast<TnShoppingBrandingModeUT>(_trx->getTnShoppingBrandingMode()));
  }

  void testParseTNBrandOptions_BFS()
  {
    parseString("<ShoppingRequest N06=\"M\"><PRO BFS=\"T\"/></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(
        static_cast<TnShoppingBrandingModeUT>(TnShoppingBrandingMode::SINGLE_BRAND),
        static_cast<TnShoppingBrandingModeUT>(_trx->getTnShoppingBrandingMode()));
  }

  void testParseTNBrandOptions_BFSNon()
  {
    parseString("<ShoppingRequest N06=\"M\"><PRO /></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(false, _trx->isBRAll());
    CPPUNIT_ASSERT_EQUAL(static_cast<TnShoppingBrandingModeUT>(TnShoppingBrandingMode::NO_BRANDS),
                         static_cast<TnShoppingBrandingModeUT>(_trx->getTnShoppingBrandingMode()));
  }


  void testFlexFareGroup()
  {
    // Test for group 0
    parseString(
        "<ShoppingRequest N06=\"M\"><PRO PFF=\"T\" Q17=\"0\" P49=\"T\"/></ShoppingRequest>");
    testFFG(0, 0, false, false, false, false, false, false, false, false, flexFares::JumpCabinLogic::ENABLED, false);

    parseString(
        "<ShoppingRequest N06=\"M\"><PRO PFF=\"T\" Q17=\"1\" P49=\"T\"/></ShoppingRequest>");
    testFFG(1, 1, false, true, false, false, false, false, false, false, flexFares::JumpCabinLogic::ENABLED, true);

    // Test for advanced purchase, size should be 2 now, and set ffg Id=Q17=2.
    parseString(initFFGString("2", "FFR=\"XA\""));
    testFFG(2, 2, false, false, true, false, false, false, false, false, flexFares::JumpCabinLogic::ENABLED, true);

    // Test for No Penalties Required, size should be 3 now, and set ffg Id=Q17=3.
    parseString(initFFGString("3", "FFR=\"XP\""));
    testFFG(3, 3, true, false, false, false, false, false, false, false, flexFares::JumpCabinLogic::ENABLED, true);

    // Test for No Min/Max Stay, size should be 4 now, and set ffg Id=Q17=8.
    parseString(initFFGString("8", "FFR=\"XS\""));
    testFFG(4, 8, false, true, false, false, false, false, false, false, flexFares::JumpCabinLogic::ENABLED, true);

    // Test for No Restriction, size should be 5 now, and set ffg Id=Q17=9.
    parseString(initFFGString("9", "FFR=\"XR\""));
    testFFG(5, 9, true, true, true, false, false, false, false, false, flexFares::JumpCabinLogic::ENABLED, true);

    // Test for business cabin type, size should be 6 now, and set ffg Id=Q17=10.
    parseString(initFFGString("10", "B31=\"C\""));
    testFFG(6, 10, false, false, false, true, false, false, false, false, flexFares::JumpCabinLogic::ENABLED, true);

    // Test for pax type code for CHILD, size should be 7 now, and set ffg Id=Q17=20.
    parseString(initFFGString("20", "PTC=\"CNN\""));
    testFFG(7, 20, false, false, false, false, false, false, false, false, flexFares::JumpCabinLogic::ENABLED, true, CHILD);

    // Test for public fares, reuse ffg ID/Q17 which is 20, so the size should still be 7
    parseString(initFFGString("20", "P1Y=\"Y\""));
    testFFG(7, 20, false, false, false, false, true, false, false, false, flexFares::JumpCabinLogic::ENABLED, true, CHILD);

    // Test for private fares, size should be 8 now, and set ffg Id=Q17=30
    parseString(initFFGString("30", "P1Z=\"Y\""));
    testFFG(8, 30, false, false, false, false, false, true, false, false, flexFares::JumpCabinLogic::ENABLED, true);

    // Test for XC Indicator, size should be 9 now and set ffg Id=Q17=31
    parseString(initFFGString("31", "PXC=\"T\""));
    testFFG(9, 31, false, false, false, false, false, false, true, false, flexFares::JumpCabinLogic::ENABLED, true);

    // Test for P20/XC Indicator, size should be 10 now and set ffg Id=Q17=32
    parseString(initFFGString("32", "P20=\"T\""));
    testFFG(10, 32, false, false, false, false, false, false, false, true, flexFares::JumpCabinLogic::ENABLED, true);

    // Test for PXS Indicator, size should be 11 now and set ffg Id=Q17=33
    parseString(initFFGString("33", "PXS=\"T\""));
    testFFG(11, 33, false, false, false, false, false, false, false, false, flexFares::JumpCabinLogic::ONLY_MIXED, true);

    // Test for PXU Indicator, size should be 12 now and set ffg Id=Q17=34
    parseString(initFFGString("34", "PXU=\"T\""));
    testFFG(12, 34, false, false, false, false, false, false, false, false, flexFares::JumpCabinLogic::DISABLED, true);

    // Test for PXS and PXU Indicator, size should be 13 now and set ffg Id=Q17=35
    parseString(initFFGString("35", "PXS=\"T\" PXU=\"T\""));
    testFFG(13, 35, false, false, false, false, false, false, false, false, flexFares::JumpCabinLogic::DISABLED, true);

    // Test for PXS and PXU Indicator, size should be 14 now and set ffg Id=Q17=36
    parseString(initFFGString("36", "PXS=\"T\" PXU=\"F\""));
    testFFG(14, 36, false, false, false, false, false, false, false, false, flexFares::JumpCabinLogic::ENABLED, true);

    // Test for PXS and PXU Indicator, size should be 15 now and set ffg Id=Q17=37
    parseString(initFFGString("37", "PXS=\"F\" PXU=\"T\""));
    testFFG(15, 37, false, false, false, false, false, false, false, false, flexFares::JumpCabinLogic::DISABLED, true);

    // Test for PXS and PXU Indicator, size should be 16 now and set ffg Id=Q17=38
    parseString(initFFGString("38", "PXS=\"F\" PXU=\"F\""));
    testFFG(16, 38, false, false, false, false, false, false, false, false, flexFares::JumpCabinLogic::ENABLED, true);
  }

  std::string initFFGString(const std::string& q17String, const std::string& attrString)
  {
    return "<ShoppingRequest N06=\"M\"><PRO PFF=\"T\" Q17=\"0\"/><FFG Q17=\"" + q17String + "\" " +
           attrString + "/></ShoppingRequest>";
  }

  void testFFG(uint16_t ffgSize,
               uint16_t ffgId,
               bool isNoPenaltiesRequired,
               bool isNoMinMaxStayRequired,
               bool isNoAdvancedPurchaseRequired,
               bool testCabinForBusinessClass,
               bool arePublicFaresRequired,
               bool arePrivateFaresRequired,
               bool flexFareXCIndicator,
               bool flexFareXOIndicator,
               flexFares::JumpCabinLogic flexFareJumpCabinLogic,
               bool isFlexFareGroup,
               PaxTypeCode paxTypeCode = ADULT)
  {
    flexFares::GroupsData ffgData = _trx->getRequest()->getFlexFaresGroupsData();

    CPPUNIT_ASSERT_EQUAL(ffgSize, ffgData.getSize());
    CPPUNIT_ASSERT_EQUAL(arePublicFaresRequired, ffgData.arePublicFaresRequired(ffgId));
    CPPUNIT_ASSERT_EQUAL(arePrivateFaresRequired, ffgData.arePrivateFaresRequired(ffgId));
    CPPUNIT_ASSERT_EQUAL(isNoPenaltiesRequired, ffgData.isNoPenaltiesRequired(ffgId));
    CPPUNIT_ASSERT_EQUAL(isNoMinMaxStayRequired, ffgData.isNoMinMaxStayRequired(ffgId));
    CPPUNIT_ASSERT_EQUAL(isNoAdvancedPurchaseRequired, ffgData.isNoAdvancePurchaseRequired(ffgId));
    if (testCabinForBusinessClass)
    {
      CPPUNIT_ASSERT_EQUAL(true, ffgData.getRequestedCabin(ffgId).isBusinessClass());
    }
    else
    {
      CPPUNIT_ASSERT_EQUAL(true, ffgData.getRequestedCabin(ffgId).isUnknownClass());
    }
    CPPUNIT_ASSERT_EQUAL(flexFareXCIndicator, ffgData.isFlexFareXCIndicatorON(ffgId));
    CPPUNIT_ASSERT_EQUAL(flexFareXOIndicator, TypeConvert::pssCharToBool(ffgData.getFlexFareXOFares(ffgId)));
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(flexFareJumpCabinLogic), static_cast<int>(ffgData.getFFGJumpCabinLogic(ffgId)));
    CPPUNIT_ASSERT_EQUAL(isFlexFareGroup, ffgData.isFlexFareGroup(ffgId));
    CPPUNIT_ASSERT_EQUAL(paxTypeCode, ffgData.getPaxTypeCode(ffgId));
  }

  void testParseS15XFFOptionsForFareFocus_S15_Ffocus_False()
  {
    static std::string EPR_FFOCUS = "FFOCUS";
    parseString("<ShoppingRequest><PRO S15=\"TEST,TEST1\" /></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(false, _trx->getOptions()->isKeywordPresent(EPR_FFOCUS));
  }

  void testParseS15XFFOptionsForFareFocus_S15_Ffocus_True()
  {
    static std::string EPR_FFOCUS = "FFOCUS";
    parseString("<ShoppingRequest><PRO S15=\"TEST,FFOCUS\" /></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(true, _trx->getOptions()->isKeywordPresent(EPR_FFOCUS));
  }

  void testParseS15XFFOptionsForFareFocus_XFF_False()
  {
    parseString("<ShoppingRequest><PRO XFF=\"F\" /></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(false, _trx->getOptions()->isExcludeFareFocusRule());
  }

  void testParseS15XFFOptionsForFareFocus_XFF_S15_True()
  {
    parseString("<ShoppingRequest><PRO XFF=\"T\" S15=\"TEST,FFOCUS\" /></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(true, _trx->getOptions()->isExcludeFareFocusRule());
  }

  void testRestrictExcludeFareFocusRule_XFF_NOEPR_Throw()
  {
    CPPUNIT_ASSERT_THROW(parseString("<ShoppingRequest><PRO XFF=\"T\" //S15=\"TEST,NOFFOCUS\" /></ShoppingRequest>"),ErrorResponseException);
  }

  void testRestrictExcludeFareFocusRule_XFF_EPR_No_Throw()
  {
    CPPUNIT_ASSERT_NO_THROW(parseString("<ShoppingRequest><PRO XFF=\"T\" S15=\"TEST,FFOCUS\" /></ShoppingRequest>"));
  }
//*******************************

  void testparseS15PDOOptions_PDO_False()
  {
    static std::string EPR_ORGFQD = "ORGFQD";
    parseString("<ShoppingRequest><PRO PDO=\"F\" /></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(false, _trx->getOptions()->isKeywordPresent(EPR_ORGFQD));
  }

  void testparseS15PDOOptions_S15_ORGFQD_True()
  {
    static std::string EPR_ORGFQD = "ORGFQD";
    parseString("<ShoppingRequest><PRO S15=\"TEST,ORGFQD\" /></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(true, _trx->getOptions()->isKeywordPresent(EPR_ORGFQD));
  }

  void testparseS15PDOOptions_PDO_S15_True()
  {
    parseString("<ShoppingRequest><PRO PDO=\"T\" S15=\"TEST,ORGFQD\" /></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(true, _trx->getOptions()->isPDOForFRRule());
  }

  void testparseS15PDOOptions_PDO_NOEPR_Throw()
  {
    CPPUNIT_ASSERT_THROW(parseString("<ShoppingRequest><PRO PDO=\"T\" S15=\"TEST,NOORGFQD\" /></ShoppingRequest>"),ErrorResponseException);
  }

  void testparseS15PDOOptions_PDO_EPR_No_Throw()
  {
    CPPUNIT_ASSERT_NO_THROW(parseString("<ShoppingRequest><PRO PDO=\"T\" S15=\"TEST,ORGFQD\" /></ShoppingRequest>"));
  }

  void testCheckOptionalParametersXRSorPDOorPDR_Error1()
  {
    CPPUNIT_ASSERT_THROW(parseString("<ShoppingRequest><PRO PDO=\"T\" XRS=\"T\" /></ShoppingRequest>"),ErrorResponseException);
  }

  void testCheckOptionalParametersXRSorPDOorPDR_Error2()
  {
    CPPUNIT_ASSERT_THROW(parseString("<ShoppingRequest><PRO PDR=\"T\" XRS=\"T\" /></ShoppingRequest>"),ErrorResponseException);
  }

  void testparseS15PDROptions_S15_AGYRET_True()
  {
    static std::string EPR_AGYRET = "AGYRET";
    parseString("<ShoppingRequest><PRO S15=\"TEST,AGYRET\" /></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(true, _trx->getOptions()->isKeywordPresent(EPR_AGYRET));
  }

  void testparseS15PDROptions_PDR_False()
  {
    parseString("<ShoppingRequest><PRO PDO=\"F\" /></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(false, _trx->getOptions()->isPDRForFRRule());
  }

  void testparseS15PDROptions_PDR_S15_True()
  {
    parseString("<ShoppingRequest><PRO PDR=\"T\" S15=\"TEST,AGYRET\" /></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(true, _trx->getOptions()->isPDRForFRRule());
  }

  void testparseS15PDROptions_PDR_NOEPR_Throw()
  {
    CPPUNIT_ASSERT_THROW(parseString("<ShoppingRequest><PRO PDR=\"T\" S15=\"TEST,NOAGYRET\" /></ShoppingRequest>"),ErrorResponseException);
  }

  void testparseS15PDROptions_PDR_EPR_No_Throw()
  {
    CPPUNIT_ASSERT_NO_THROW(parseString("<ShoppingRequest><PRO PDR=\"T\" S15=\"TEST,AGYRET\" /></ShoppingRequest>"));
  }

  void testparseS15XRSOptions_S15_ORGFQD_True()
  {
    static std::string EPR_ORGFQD = "ORGFQD";
    parseString("<ShoppingRequest><PRO S15=\"TEST,ORGFQD\" /></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(true, _trx->getOptions()->isKeywordPresent(EPR_ORGFQD));
  }

  void testparseS15XRSOptions_XRS_False()
  {
    parseString("<ShoppingRequest><PRO PDO=\"F\" /></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(false, _trx->getOptions()->isXRSForFRRule());
  }

  void testparseS15XRSOptions_XRS_S15_True()
  {
    parseString("<ShoppingRequest><PRO XRS=\"T\" S15=\"TEST,ORGFQD\" /></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(true, _trx->getOptions()->isXRSForFRRule());
  }

  void testparseS15XRSOptions_XRS_NOEPR_Throw()
  {
    CPPUNIT_ASSERT_THROW(parseString("<ShoppingRequest><PRO XRS=\"T\" S15=\"TEST,NOEPR\" /></ShoppingRequest>"),ErrorResponseException);
  }

  void testparseS15XRSOptions_XRS_EPR_No_Throw()
  {
    CPPUNIT_ASSERT_NO_THROW(parseString("<ShoppingRequest><PRO XRS=\"T\" S15=\"TEST,ORGFQD\" /></ShoppingRequest>"));
  }

  void testRBDDiagnosticConversionfromIntellisell()
  {
    parseString("<ShoppingRequest D70=\"50\" ><DIA Q0A=\"927\"><PRO TEP=\"2\" /><ARG NAM=\"BK\" VAL=\"H\" /></DIA></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(int16_t(187), _trx->getRequest()->diagnosticNumber());
  }

  void testNewAtpcoRbdByCabinAnswerTable()
  {
    parseString("<ShoppingRequest N06=\"M\"><PRO D54=\"46\" P0J=\"T\" SEZ=\"T\" SEV=\"T\" VCX=\"T\" DVL=\"T\"/> "
                "<AAF Q1K=\"0\" B00=\"AA\" B01=\"AA\" Q0B=\"2155\" S05=\"777\" P0Z=\"T\"> "
                "<BRD A01=\"MIA\" D01=\"2015-06-01\" D31=\"1120\"/><OFF A02=\"DFW\" D02=\"2015-06-01\" D32=\"1455\"/></AAF> "
                "<AVL Q16=\"0\" A01=\"MIA\" A02=\"DFW\"><FBK Q1K=\"0\" Q17=\"F9|J7|C7|I9|W7|T9|Y7\"/></AVL>"
                "<LEG Q14=\"0\" B31=\"Y\"><SOP Q15=\"0\"><FID Q1K=\"0\" /><AID Q16=\"0\" /></SOP></LEG> "
                "<ITN NUM=\"0\" C65=\"0\"><SID Q14=\"0\" Q15=\"0\"><BKK B30=\"Y\" /></SID></ITN> "
                "</ShoppingRequest>");
    std::vector<ClassOfService*>& cosVec = _handler->_avlParser.value()->back();
    CPPUNIT_ASSERT_EQUAL((uint16_t)9, cosVec[0]->numSeats());
    CPPUNIT_ASSERT(cosVec[0]->cabin().isFirstClass());
    CPPUNIT_ASSERT(!cosVec[0]->cabin().isEconomyClass());
    CPPUNIT_ASSERT_EQUAL(BookingCode("F"), cosVec[0]->bookingCode());
  }

  void testparseB12B13Attribute_B12EmptyValue()
  {
    parseString("<ShoppingRequest><PRO B12=\"\"/></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL((uint16_t)0, (uint16_t)_trx->getRequest()->nonPreferredVCs().size());
  }

  void testparseB12B13Attribute_B12SingleValue()
  {
    parseString("<ShoppingRequest><PRO B12=\"AA\"/></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(std::string("AA"), DiagnosticUtil::containerToString(_trx->getRequest()->nonPreferredVCs()));
  }

  void testparseB12B13Attribute_B12MultiValue()
  {
    parseString("<ShoppingRequest><PRO B12=\"BA|LH|AA\"/></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(std::string("BA,LH,AA"), DiagnosticUtil::containerToString(_trx->getRequest()->nonPreferredVCs()));
  }

  void testparseB12B13Attribute_B13EmptyValue()
  {
    parseString("<ShoppingRequest><PRO B13=\"\"/></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL((uint16_t)0, (uint16_t)_trx->getRequest()->preferredVCs().size());
  }

  void testparseB12B13Attribute_B13SingleValue()
  {
    parseString("<ShoppingRequest><PRO B13=\"AA\"/></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(std::string("AA"), DiagnosticUtil::containerToString(_trx->getRequest()->preferredVCs()));
  }

  void testparseB12B13Attribute_B13MultiValue()
  {
    parseString("<ShoppingRequest><PRO B13=\"BA|LH|AA\"/></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(std::string("BA,LH,AA"), DiagnosticUtil::containerToString(_trx->getRequest()->preferredVCs()));
  }

  void testparseB12B13Attribute_B12B13MultiValue()
  {
    parseString("<ShoppingRequest><PRO B12=\"AF|KL\" B13=\"BA|LH|AA\"/></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(std::string("BA,LH,AA"), DiagnosticUtil::containerToString(_trx->getRequest()->preferredVCs()));
    CPPUNIT_ASSERT_EQUAL(std::string("AF,KL"), DiagnosticUtil::containerToString(_trx->getRequest()->nonPreferredVCs()));
  }

  void testparseXRAType()
  {
    TestConfigInitializer::setValue("FEATURE_ENABLE", "Y", "XRAY");

    parseString("<ShoppingRequest><XRA MID=\"mid\" CID=\"cid\"/><PRO /></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(std::string("mid"), _trx->getXrayJsonMessage()->getMid());
    CPPUNIT_ASSERT_EQUAL(std::string("cid"), _trx->getXrayJsonMessage()->getCid());
  }

  void testStoreProcOptsInformation_BI0_CabinRequest()
  {

    _trx->billing()->actionCode() = "WPNI";
    parseString("<ShoppingRequest N06=\"M\"><PRO BI0=\"AB\" /></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(true, _trx->getRequest()->isjumpUpCabinAllowed());

    _trx->billing()->actionCode() = "WPNI";
    parseString("<ShoppingRequest N06=\"M\"><PRO BI0=\"YB\" /></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(false, _trx->getRequest()->isjumpUpCabinAllowed());

    parseString("<ShoppingRequest N06=\"M\"><PRO BI0=\"PB\" /></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(Indicator('1'), _trx->getOptions()->cabin().getCabinIndicator());

    parseString("<ShoppingRequest N06=\"M\"><PRO BI0=\"YB\" /></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL(Indicator('8'), _trx->getOptions()->cabin().getCabinIndicator());

  }

  void testStoreProcOptsInformation_BI0_CabinRequestThrowException()
  {
    CPPUNIT_ASSERT_THROW(parseString("<ShoppingRequest N06=\"M\"><PRO BI0=\"ZZ\" /></ShoppingRequest>"), ErrorResponseException);
  }

  void testStoreProcOptsInformation_BI0_CabinRequestThrowExceptionMultiCabin()
  {
    CPPUNIT_ASSERT_THROW(parseString("<ShoppingRequest N06=\"M\"><PRO BI0=\"JBYB\" /></ShoppingRequest>"), ErrorResponseException);
  }

  void testSPV_Valid_IND()
  {
    fallback::value::fallbackVITA4NonBSP.set(true);
    parseString("<ShoppingRequest N06=\"M\"><SPV SMV=\"F\" IEV=\"F\" /></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL((uint16_t)tse::spValidator::noSMV_noIEV, (uint16_t)_trx->getRequest()->spvInd());
  }

  void testSPV_Valid_IND1()
  {
    fallback::value::fallbackVITA4NonBSP.set(true);
    parseString("<ShoppingRequest N06=\"M\"><SPV SMV=\"F\" /></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL((uint16_t)tse::spValidator::noSMV_IEV, (uint16_t)_trx->getRequest()->spvInd());
  }

  void testSPV_Valid_IND2()
  {
    fallback::value::fallbackVITA4NonBSP.set(true);
    CPPUNIT_ASSERT_THROW(parseString("<ShoppingRequest N06=\"M\"><SPV IEV=\"F\" /></ShoppingRequest>"),  ErrorResponseException);
  }

  void testSPV_Valid_IND3()
  {
    fallback::value::fallbackVITA4NonBSP.set(true);
    parseString("<ShoppingRequest N06=\"M\"><SPV  /></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL((uint16_t)tse::spValidator::SMV_IEV, (uint16_t)_trx->getRequest()->spvInd());
  }

  void testSPV_Invalid_IND()
  {
    fallback::value::fallbackVITA4NonBSP.set(true);
    CPPUNIT_ASSERT_THROW(parseString("<ShoppingRequest N06=\"M\"><SPV SMV=\"T\" IEV=\"F\" /></ShoppingRequest>"),  ErrorResponseException);
  }

  void testSPV_IND_CRC()
  {
    fallback::value::fallbackVITA4NonBSP.set(true);
    parseString("<ShoppingRequest N06=\"M\"><SPV SMV=\"F\" IEV=\"F\" CRC=\"AA|BA|LH\" /></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL((uint16_t)tse::spValidator::noSMV_noIEV, (uint16_t)_trx->getRequest()->spvInd());
    CPPUNIT_ASSERT_EQUAL(std::string("AA,BA,LH"), DiagnosticUtil::containerToString(_trx->getRequest()->spvCxrsCode()));
    CPPUNIT_ASSERT_EQUAL((uint16_t)0, (uint16_t)_trx->getRequest()->spvCntyCode().size());
  }

  void testSPV_IND_CRC_CTC()
  {
    fallback::value::fallbackVITA4NonBSP.set(true);
    parseString("<ShoppingRequest N06=\"M\"><SPV SMV=\"T\" IEV=\"T\" CRC=\"AA|BA|LH\" CTC=\"US|CA|IT\" /></ShoppingRequest>");
    CPPUNIT_ASSERT_EQUAL((uint16_t)tse::spValidator::SMV_IEV,  (uint16_t)_trx->getRequest()->spvInd());
    CPPUNIT_ASSERT_EQUAL(std::string("AA,BA,LH"), DiagnosticUtil::containerToString(_trx->getRequest()->spvCxrsCode()));
    CPPUNIT_ASSERT_EQUAL((uint16_t)0, (uint16_t)_trx->getRequest()->spvCntyCode().size());
  }

private:
  ShoppingTrx* _trx;
  XMLShoppingHandler* _handler;
  DataHandle _dataHandle;
  std::vector<size_t> _resources;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(XMLShoppingHandlerTest);
}
