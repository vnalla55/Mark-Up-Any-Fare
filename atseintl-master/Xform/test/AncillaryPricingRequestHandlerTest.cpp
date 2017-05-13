#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/Billing.h"
#include "DataModel/Itin.h"
#include "DataModel/SurfaceSeg.h"
#include "DBAccess/Cabin.h"
#include "DBAccess/Currency.h"
#include "DBAccess/Customer.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/Loc.h"
#include "Xform/AncillaryPricingRequestHandler.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>

#include <algorithm>
#include <fstream>

namespace tse
{
namespace
{
class MyDataHandle : public DataHandleMock
{
public:
  MyDataHandle(TestMemHandle& memHandle) : _memHandle(memHandle) {}
  const std::vector<Customer*>& getCustomer(const PseudoCityCode& key)
  {
    if (key == "VIE" || key == "")
      return *_memHandle.create<std::vector<Customer*> >();
    else if (key == "K7RF")
    {
      std::vector<Customer*>* ret = _memHandle.create<std::vector<Customer*> >();
      Customer* c = _memHandle.create<Customer>();
      c->pseudoCity() = "K7RF";
      c->homePseudoCity() = "K7RF";
      c->requestCity() = "BNE";
      c->aaCity() = "VIE";
      c->defaultCur() = "CUD";
      ret->push_back(c);
      return *ret;
    }
    return DataHandleMock::getCustomer(key);
  }
  const LocCode getMultiTransportCity(const LocCode& locCode)
  {
    if (locCode == "VIE")
      return "VIE";
    return DataHandleMock::getMultiTransportCity(locCode);
  }
  const std::vector<FareCalcConfig*>& getFareCalcConfig(const Indicator& userApplType,
                                                        const UserApplCode& userAppl,
                                                        const PseudoCityCode& pseudoCity)
  {
    std::vector<FareCalcConfig*>& ret = *_memHandle.create<std::vector<FareCalcConfig*> >();
    ret.push_back(_memHandle.create<FareCalcConfig>());
    return ret;
  }
  const std::vector<const PaxTypeMatrix*>& getPaxTypeMatrix(const PaxTypeCode& paxTypeCode)
  {
    if (paxTypeCode == "ADT" || paxTypeCode == "CNN" || paxTypeCode == "NEG")
      return *_memHandle.create<std::vector<const PaxTypeMatrix*> >();
    return DataHandleMock::getPaxTypeMatrix(paxTypeCode);
  }
  bool corpIdExists(const std::string& corpId, const DateTime& tvlDate)
  {
    if (corpId == "VALID" || corpId == "VALID2" || corpId == "VALID3" || corpId == "COR01")
      return true;
    else if (corpId == "INVALID" || corpId == "INVALID2" || corpId == "INVALID3" ||
             corpId == "COR02")
      return false;
    return DataHandleMock::corpIdExists(corpId, tvlDate);
  }
  const Loc* getLoc(const LocCode& locCode, const DateTime& date)
  {
    if (locCode == "LO1" || locCode == "LO2" || locCode == "LO3" || locCode == "LO4" || locCode == "LGA")
      return _memHandle.create<Loc>();
    return DataHandleMock::getLoc(locCode, date);
  }
  const Cabin*
  getCabin(const CarrierCode& carrier, const BookingCode& classOfService, const DateTime& date)
  {
    if ((carrier == "BA" || carrier == "LT") && (classOfService == "Y" || classOfService == "C"))
    {
      Cabin* c = _memHandle.create<Cabin>();
      if (classOfService == "Y")
        c->cabin().setEconomyClass();
      else
        c->cabin().setBusinessClass();
      return c;
    }
    return DataHandleMock::getCabin(carrier, classOfService, date);
  }
  const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                          const CarrierCode& carrierCode,
                                          GeoTravelType tvlType,
                                          const DateTime& tvlDate)
  {
    if (locCode == "LO1" || locCode == "LO2" || locCode == "LO3" || locCode == "LO4")
      return "";
    else if (locCode == "LGA" || locCode == "JFK")
      return "NYC";
    else if (locCode == "ORD")
      return "CHI";
    else
    return DataHandleMock::getMultiTransportCityCode(locCode, carrierCode, tvlType, tvlDate);
  }

  const Currency* getCurrency(const CurrencyCode& currency)
  {
    if (currency == "GBP" || currency == "AUD")
      return _memHandle.create<Currency>();
    else if (currency == "XXX")
      return 0;
    else
      return DataHandleMock::getCurrency(currency);
  }

  bool getUtcOffsetDifference(const DSTGrpCode& dstgrp1,
                              const DSTGrpCode& dstgrp2,
                              short& utcOffset,
                              const DateTime& dateTime1,
                              const DateTime& dateTime2)
  {
    utcOffset = 0;
    return true;
  }

private:
  TestMemHandle& _memHandle;
};
}

namespace
{
class AncillaryPricingRequestHandlerMock : public AncillaryPricingRequestHandler
{
public:
  AncillaryPricingRequestHandlerMock(Trx*& trx) : AncillaryPricingRequestHandler(trx) {}

  bool startElement(int idx, const IAttributes& attrs) override
  {
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    trx->modifiableActivationFlags().setMonetaryDiscount(true);
    trx->fareCalcConfig() = _memHandle.create<FareCalcConfig>();
    return AncillaryPricingRequestHandler::startElement(idx, attrs);
  }

protected:
  const Cabin* getCabin()
  {
    Cabin* c = _memHandle.create<Cabin>();
    c->cabin().setEconomyClass();
    return c;
  }
 TestMemHandle _memHandle;
};
}

class AncillaryPricingRequestHandlerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AncillaryPricingRequestHandlerTest);

  CPPUNIT_TEST(testParseAGI);
  CPPUNIT_TEST(testParseAGI_NoCurrencyCode);
  CPPUNIT_TEST(testParseTAG);
  CPPUNIT_TEST(testParseTAG_withTKN);
  CPPUNIT_TEST(testParseTAG_NoCurrencyCode);
  CPPUNIT_TEST(testParse_M70_BIL_A70Empty);
  CPPUNIT_TEST(testParse_M70_BIL_A70_fromOTA);
  CPPUNIT_TEST(testParseBIL_A70_WPAE);
  CPPUNIT_TEST(testParseBIL_A70_WPAE_fromOTA);
  CPPUNIT_TEST(testParseBIL_A70_WPAE_T);
  CPPUNIT_TEST(testParseBIL_A70_WPAE_T_fromOTA);
  CPPUNIT_TEST(testParseBIL_A70_Invalid_Value_Z);
  CPPUNIT_TEST(testParseBIL_A70_Invalid_Value_WP);
  CPPUNIT_TEST(testParseBIL_A70_Invalid_Value_WPP);
  CPPUNIT_TEST(testParseBIL_A70_Invalid_Value_WPAE);
  CPPUNIT_TEST(testParseBIL_A70_Invalid_Value_WPBET);
  CPPUNIT_TEST(testParseBIL_B05);
  CPPUNIT_TEST(testParseBIL_No_B05);
  CPPUNIT_TEST(testParsePRO);
  CPPUNIT_TEST(testPasrePRO_BlankS15);
  CPPUNIT_TEST(testParsePRO_NoS15);
  CPPUNIT_TEST(testParsePRO_TKI);
  CPPUNIT_TEST(testParsePRO_NoTKI);
  CPPUNIT_TEST(testParseRFG);
  CPPUNIT_TEST(testParseFFY);
  CPPUNIT_TEST(testParseFFY_Itin);
  CPPUNIT_TEST(testParseFFY_Dups);
  CPPUNIT_TEST(testParseDIG);
  CPPUNIT_TEST(testParseFLI);
  CPPUNIT_TEST(testParseFLI_Fail_ClassOfService);
  CPPUNIT_TEST(testParseFLI_Fail_NoMarketingCarrier);
  CPPUNIT_TEST(testParseFLI_NoOperatingCarrier);
  CPPUNIT_TEST(testParseFLI_Fail_NoOrigAirport);
  CPPUNIT_TEST(testParseFLI_Fail_NoDestAirport);
  CPPUNIT_TEST(testParseFBA);
  CPPUNIT_TEST(testParsePXI);
  CPPUNIT_TEST(testParsePXI_C07);
  CPPUNIT_TEST(testParseACI);
  CPPUNIT_TEST(testParseCII_Valid);
  CPPUNIT_TEST(testParseCII_Invalid);
  CPPUNIT_TEST(testParseFBI);
  CPPUNIT_TEST(testParseIRO_NoPaxType);
  CPPUNIT_TEST(testParseIRO_FreqFlyerStatus);
  CPPUNIT_TEST(testParseIRO_MaxAccontCodes);
  CPPUNIT_TEST(testParseIRO_MaxCorpId);
  CPPUNIT_TEST(testParseIRO_TicketIssueDate);
  CPPUNIT_TEST(testParseIRO_TicketIssueDate_Empty);
  CPPUNIT_TEST(testParseOSC_givenOscIsNotInItn_shouldNotStoreOscDataInItin);
  CPPUNIT_TEST(testParseOSC_givenOscIsInItn_whenOscHasAidAttribute_shouldStoreAidInItin);
  CPPUNIT_TEST(testParseOSC_givenOscIsInItn_whenOscDoesNotHaveAidAttribute_shouldThrow);
  CPPUNIT_TEST(testParseOSC_givenOscIsInItn_whenOscHasPmiAttribute_shouldStorePmiInItin);
  CPPUNIT_TEST(testParseOSC_givenOscIsInItn_whenOscDoesNotHavePmiAttribute_shouldStorePmiInItinWithNoValue);
  CPPUNIT_TEST(testParseOSC_givenTwoOscInItn_whenTwoPmiAttributesHaveTheSameValue_shouldThrow);
  CPPUNIT_TEST(testParseOSC_givenOscIsInItn_whenOscHasQtyAttribute_shouldStoreQtyInItin);
  CPPUNIT_TEST(testParseOSC_givenOscIsInItn_whenOscDoesNotHaveAnyAttributesBesidesAid_shouldCreateDefaultAncPriceModifier);
  CPPUNIT_TEST(testParseOSC_givenOscIsInItn_whenOscHasQtyAttributeThatIsNotAnInteger_shouldThrow);
  CPPUNIT_TEST(testParseOSC_givenMultipleItnWithMultipleOsc_whenEveryOscIsValid_shouldStoreAllOscInItin);
  CPPUNIT_TEST(testParseOSC_givenOscIsInItn_whenOscHasDrpAttribute_shouldStoreDrpInItin);
  CPPUNIT_TEST(testParseOSC_givenOscIsInItn_whenOscHasDrpAttributeThatIsNotAnInteger_shouldThrow);
  CPPUNIT_TEST(testParseOSC_givenOscIsInItn_whenOscHasDrpDiscountAttributeThatIsGreaterThan100_shouldThrow);
  CPPUNIT_TEST(testParseOSC_givenOscIsInItn_whenOscHasDrpMarkupAttributeThatIsGreaterThan100_shouldNotThrow);
  CPPUNIT_TEST(testParseOSC_givenOscIsInItn_whenOscHasDrpAttributeThatIsLessThan0_shouldThrow);
  CPPUNIT_TEST(testParseOSC_givenOscIsInItn_whenOscHasDrpAndDrm_shouldThrow);
  CPPUNIT_TEST(testParseOSC_givenOscIsInItn_whenOscHasDrmAttributeThatIsLessThan0_shouldThrow);
  CPPUNIT_TEST(testParseOSC_givenOscIsInItn_whenOscHasDrmAttribute_shouldStoreDrmInItin);
  CPPUNIT_TEST(testParseITN_AddArunk);
  CPPUNIT_TEST(testParseITN_SuccessHistorical);
  CPPUNIT_TEST(testParseITN_FailDateOrder);
  CPPUNIT_TEST(testParseITN_WPAENoRFG);

  CPPUNIT_TEST(testConvertDate_Match);
  CPPUNIT_TEST(testConvertDate_Fail_letter);
  CPPUNIT_TEST(testConvertDate_Fail_format);

  CPPUNIT_TEST(testCheckForDateConsistency_First_NoDate);
  CPPUNIT_TEST(testCheckForDateConsistency_First_WithDate);
  CPPUNIT_TEST(testCheckForDateConsistency_WasFirstDate_NoDate);
  CPPUNIT_TEST(testCheckForDateConsistency_WasFirstDate_WithDate);
  CPPUNIT_TEST(testCheckForDateConsistency_WasDate_NoDate);
  CPPUNIT_TEST(testCheckForDateConsistency_WasDate_WithDate);
  CPPUNIT_TEST(testCheckForDateConsistency_WasNoDate_NoDate);
  CPPUNIT_TEST(testCheckForDateConsistency_WasNoDate_WithDate);

  CPPUNIT_TEST(testCheckForTimeConsistency_DepTime_NoArrTime);
  CPPUNIT_TEST(testCheckForTimeConsistency_NoDepTime_ArrTime);
  CPPUNIT_TEST(testCheckForTimeConsistency_First_NoDate);
  CPPUNIT_TEST(testCheckForTimeConsistency_First_WithDate);
  CPPUNIT_TEST(testCheckForTimeConsistency_WasDate_NoDate);
  CPPUNIT_TEST(testCheckForTimeConsistency_WasDate_WithDate);
  CPPUNIT_TEST(testCheckForTimeConsistency_WasNoDate_NoDate);
  CPPUNIT_TEST(testCheckForTimeConsistency_WasNoDate_WithDate);

  CPPUNIT_TEST(testSetCabin_R);
  CPPUNIT_TEST(testSetCabin_F);
  CPPUNIT_TEST(testSetCabin_J);
  CPPUNIT_TEST(testSetCabin_C);
  CPPUNIT_TEST(testSetCabin_P);
  CPPUNIT_TEST(testSetCabin_Y);
  CPPUNIT_TEST(testSetCabin_X);
  CPPUNIT_TEST(testSetCabin_R_WithCOS);
  CPPUNIT_TEST(testSetCabin_Blank_WithCOS);

  CPPUNIT_TEST(testSetTime_Empty);
  CPPUNIT_TEST(testSetTime_1028);

  CPPUNIT_TEST(testPssTime_Empty);
  CPPUNIT_TEST(testPssTime_0);
  CPPUNIT_TEST(testPssTime_2359);

  CPPUNIT_TEST(testSetSectors);

  CPPUNIT_TEST(testCheckSideTrip_First);
  CPPUNIT_TEST(testCheckSideTrip_Last);
  CPPUNIT_TEST(testCheckSideTrip_Second);

  CPPUNIT_TEST(testPXI_ADT_ADT);
  CPPUNIT_TEST(testPXI_ADT_BLANK);
  CPPUNIT_TEST(testPXI_BLANK_ADT);
  CPPUNIT_TEST(testPXI_C06_C07);
  CPPUNIT_TEST(testPXI_C00);
  CPPUNIT_TEST(testPXI_Y01);

  /*
  CPPUNIT_TEST(testCheckCurrency_BLANK);
  CPPUNIT_TEST(testCheckCurrency_CurrencyExists);
  CPPUNIT_TEST(testCheckCurrency_CurrencyDontExists);
  */
  CPPUNIT_TEST(testParsePNMWhenHasDataForLiberty);
  CPPUNIT_TEST(testParsePNMWhenHasDataForPSS);
  CPPUNIT_TEST(testParsePNMWhenHasDataForSabreWebService);
  CPPUNIT_TEST(testParsePNMWhenEmptyPsgNameNoForLiberty);
  CPPUNIT_TEST(testParsePNMWhen2xADTForLibertyAndAccumulatedNumberOfPsg);

  CPPUNIT_TEST(testCheckRequiredDataForWPAE_Pass);
  CPPUNIT_TEST(testCheckRequiredDataForWPAE_NoMatchFBA);
  CPPUNIT_TEST(testCheckRequiredDataForWPAE_NoMatchFBI);
  CPPUNIT_TEST(testCheckFLIDataForWPAE_Pass);
  CPPUNIT_TEST(testCheckFLIDataForWPAE_FailB01);
  CPPUNIT_TEST(testCheckFLIDataForWPAE_OpenSeg);
  CPPUNIT_TEST(testCheckFBADataForWPAE_Pass);
  CPPUNIT_TEST(testCheckFBADataForWPAE_FailQ6D);
  CPPUNIT_TEST(testCheckFBIDataForWPAE_Pass);
  CPPUNIT_TEST(testCheckFBIDataForWPAE_FailC50);

  /*
  CPPUNIT_TEST(testCheckIfR7Request_PostTkt_PSS);
  CPPUNIT_TEST(testCheckIfR7Request_M70_PSS);
  CPPUNIT_TEST(testCheckIfR7Request_PostTkt_Unknown);
  */

  CPPUNIT_TEST(testParseOriginalPricingCommand_AccountCode);
  CPPUNIT_TEST(testParseOriginalPricingCommand_CorpId);
  CPPUNIT_TEST(testParseOriginalPricingCommand_BuyingDate);
  CPPUNIT_TEST(testParseOriginalPricingCommand_TktDesignator_SkipWhenExist);
  CPPUNIT_TEST(testParseOriginalPricingCommand_TktDesignator);
  CPPUNIT_TEST(testParseOriginalPricingCommand_SegSelTktDesignator_SkipWhenExist);
  CPPUNIT_TEST(testParseOriginalPricingCommand_SegSelTktDesignator);
  CPPUNIT_TEST(testParseOriginalPricingCommand_RangeSegSelTktDesignator_SkipWhenExist);
  CPPUNIT_TEST(testParseOriginalPricingCommand_RangeSegSelTktDesignator);
  CPPUNIT_TEST(testParseOriginalPricingCommand_WPNCS);
  CPPUNIT_TEST(testParseOriginalPricingCommand_WPNCB);
  CPPUNIT_TEST(testParseOriginalPricingCommand_WPNC);
  CPPUNIT_TEST(testParseOriginalPricingCommand_SalePointOverride);
  CPPUNIT_TEST(testParseOriginalPricingCommand_TicketPoinOverride);
  CPPUNIT_TEST(testParseOriginalPricingCommand_CurrencyOverride);
  CPPUNIT_TEST(testParseOriginalPricingCommand_FFS);

  CPPUNIT_TEST(testCleanOriginalPricingCommandPostTkt);

  CPPUNIT_TEST(testParseSchemaVersion_Empty);
  CPPUNIT_TEST(testParseSchemaVersion_Valid);
  CPPUNIT_TEST(testParseSchemaVersion_Invalid);

  CPPUNIT_TEST(test_parseAST);
  CPPUNIT_TEST(test_parseRFG_AST);

  CPPUNIT_TEST(testCheckFlights_Non_Air);
  CPPUNIT_TEST(testCheckFlights_Mixed);

  CPPUNIT_TEST(testParseITN_PassItn1);
  CPPUNIT_TEST(testParseITN_FailItn1_Origin);
  CPPUNIT_TEST(testParseITN_FailItn1_Destination);
  CPPUNIT_TEST(testParseITN_FailItn1_DepartureDate);
  CPPUNIT_TEST(testParseITN_FailItn1_DepartureTime);
  CPPUNIT_TEST(testParseITN_FailItn1_ArrivalDate);
  CPPUNIT_TEST(testParseITN_FailItn1_ArrivalTime);
  CPPUNIT_TEST(testParseITN_FailItn1_MarketingCxr);
  CPPUNIT_TEST(testParseITN_FailItn1_OperatingCxr);
  CPPUNIT_TEST(testParseITN_FailItn1_BookingCode);
  CPPUNIT_TEST(testParseITN_FailItn1_FlightNo);
  CPPUNIT_TEST(testParseITN_FailItn1_Equipment);
  CPPUNIT_TEST(testCheckFlights_diffSize);
  CPPUNIT_TEST(testParseBTS);
  CPPUNIT_TEST(testMultiAirportCityNoArunk);
  CPPUNIT_TEST(testNonMultiAirportCityAddArunk);
  CPPUNIT_TEST(testIsRtwA);
  CPPUNIT_TEST(testIsRtwWPAE);
  CPPUNIT_TEST(testIsNotRtwFcNo);
  CPPUNIT_TEST(testIsNotRtwNoActionCode);
  CPPUNIT_TEST(testIsNotRtwWrongActionCode);
  CPPUNIT_TEST(testParseOfficeStationCodePresent);
  CPPUNIT_TEST(testParseOfficeStationCodeNotPresent);

  CPPUNIT_TEST(testParseGBA_DXT_empty);
  CPPUNIT_TEST(testParseGBA_DXT_empty_withPNM);

  CPPUNIT_TEST(testParseGBA_DXT_BGonly);
  CPPUNIT_TEST(testParseGBA_DXT_MLonly);
  CPPUNIT_TEST(testParseGBA_DXT_BGandML);
  CPPUNIT_TEST(testParseGBA_CAT_empty);
  CPPUNIT_TEST(testParseGBA_CAT_empty_noPNM);
  CPPUNIT_TEST(testParseGBA_CAT_BGonly);
  CPPUNIT_TEST(testParseGBA_CAT_MLonly);
  CPPUNIT_TEST(testParseGBA_CAT_BGandML);
  CPPUNIT_TEST(testParseGBA_OCF_empty);
  CPPUNIT_TEST(testParseGBA_OCF_1group);
  CPPUNIT_TEST(testParseGBA_OCF_2groups);
  CPPUNIT_TEST(testParseGBA_Incorrect);
  CPPUNIT_TEST(testParseGBA_IncorrectS01);
  CPPUNIT_TEST(testParseGBA_DXT_RouteIndicator_Pass);
  CPPUNIT_TEST(testParseGBA_DXT_RouteIndicator_Fail);

  CPPUNIT_TEST(testParseRCP_RES);
  CPPUNIT_TEST(testParseRCP_CKI);
  CPPUNIT_TEST(testParseRCP_empty);
  CPPUNIT_TEST(testParseRCP_incorrect);

  CPPUNIT_TEST(test_ItinNoFBAFail);
  CPPUNIT_TEST(test_ItinNoFBIFail);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = 0;
    _handler = _memHandle.insert(new AncillaryPricingRequestHandler(_trx));
    _handlerM = _memHandle.insert(new AncillaryPricingRequestHandlerMock(_trx));
    _memHandle.insert(new MyDataHandle(_memHandle));
    const std::string activationDateEnabled = "2000-01-01";
    const std::string activationDateDisabled = "2050-01-01";
    TestConfigInitializer::setValue("EMD_VALIDATION_FLIGHT_RELATED_SERVICE_AND_PREPAID_BAGGAGE",
                                    activationDateEnabled,
                                    "EMD_ACTIVATION");
    TestConfigInitializer::setValue("EMD_VALIDATION_ON_RESERVATION_PATH_FOR_AB240",
                                    activationDateDisabled,
                                    "EMD_ACTIVATION");
    TestConfigInitializer::setValue("EMD_VALIDATION_FLIGHT_RELATED_ON_CHECKIN_PATH_FOR_AB240",
                                    activationDateDisabled,
                                    "EMD_ACTIVATION");
  }
  void tearDown() { _memHandle.clear(); }
  void testParseAGI()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(
        _dataHandle, "<AGI A10=\"VIE\" A20=\"K7RF\" A21=\"K8RF\" A80=\"K9RF\" A90=\"-A8\" "
                     "AB0=\"9999999\" AB1=\"8888888\" AE0=\"AA\" B00=\"1S\" C40=\"AUD\" C6C=\"12\" "
                     "Q01=\"04\" N0G=\"*\" N0L=\"BB\" />"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(LocCode("VIE"), trx->getRequest()->ticketingAgent()->agentCity());
    CPPUNIT_ASSERT_EQUAL(PseudoCityCode("K7RF"),
                         trx->getRequest()->ticketingAgent()->tvlAgencyPCC());
    CPPUNIT_ASSERT_EQUAL(PseudoCityCode("K8RF"),
                         trx->getRequest()->ticketingAgent()->mainTvlAgencyPCC());
    CPPUNIT_ASSERT_EQUAL(std::string("K9RF"), trx->getRequest()->ticketingAgent()->airlineDept());
    CPPUNIT_ASSERT_EQUAL(std::string("-A8"), trx->getRequest()->ticketingAgent()->agentFunctions());
    CPPUNIT_ASSERT_EQUAL(std::string("9999999"),
                         trx->getRequest()->ticketingAgent()->tvlAgencyIATA());
    CPPUNIT_ASSERT_EQUAL(std::string("8888888"),
                         trx->getRequest()->ticketingAgent()->homeAgencyIATA());
    CPPUNIT_ASSERT_EQUAL(std::string("AA"), trx->getRequest()->ticketingAgent()->vendorCrsCode());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("1S"), trx->getRequest()->ticketingAgent()->cxrCode());
    CPPUNIT_ASSERT_EQUAL(std::string("AUD"),
                         trx->getRequest()->ticketingAgent()->currencyCodeAgent());
    CPPUNIT_ASSERT_EQUAL(uint32_t(12),
                         trx->getRequest()->ticketingAgent()->agentCommissionAmount());
    CPPUNIT_ASSERT_EQUAL(std::string("*"), trx->getRequest()->ticketingAgent()->agentDuty());
    CPPUNIT_ASSERT_EQUAL(std::string("BB"),
                         trx->getRequest()->ticketingAgent()->agentCommissionType());
    CPPUNIT_ASSERT_EQUAL(int16_t(4), trx->getRequest()->ticketingAgent()->coHostID());
  }
  void testParseAGI_NoCurrencyCode()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle, "<AGI A10=\"VIE\" A20=\"K7RF\"/>"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    // from DataHandle::getCustomer
    CPPUNIT_ASSERT_EQUAL(std::string("CUD"),
                         trx->getRequest()->ticketingAgent()->currencyCodeAgent());
  }
  void testParseTAG()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(
        _dataHandle, "<TAG A10=\"VIE\" A20=\"K7RF\" A21=\"K8RF\" A80=\"K9RF\" A90=\"-A8\" "
                     "AB0=\"9999999\" AB1=\"8888888\" AE0=\"AA\" B00=\"1S\" C40=\"AUD\" C6C=\"12\" "
                     "Q01=\"04\" N0G=\"*\" N0L=\"BB\" />"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    AncRequest* ancRq = static_cast<AncRequest*>(trx->getRequest());
    assert(ancRq->isTicketNumberValid(0) == true);
    assert(ancRq->isTicketNumberValid(1) == false);
    assert(ancRq->isTicketNumberValid(2) == false);
    assert(ancRq->isTicketNumberValid(3) == false);
    ancRq->setActiveAgent(AncRequest::TicketingAgent);
    CPPUNIT_ASSERT_EQUAL(LocCode("VIE"), trx->getRequest()->ticketingAgent()->agentCity());
    CPPUNIT_ASSERT_EQUAL(PseudoCityCode("K7RF"),
                         trx->getRequest()->ticketingAgent()->tvlAgencyPCC());
    CPPUNIT_ASSERT_EQUAL(PseudoCityCode("K8RF"),
                         trx->getRequest()->ticketingAgent()->mainTvlAgencyPCC());
    CPPUNIT_ASSERT_EQUAL(std::string("K9RF"), trx->getRequest()->ticketingAgent()->airlineDept());
    CPPUNIT_ASSERT_EQUAL(std::string("-A8"), trx->getRequest()->ticketingAgent()->agentFunctions());
    CPPUNIT_ASSERT_EQUAL(std::string("9999999"),
                         trx->getRequest()->ticketingAgent()->tvlAgencyIATA());
    CPPUNIT_ASSERT_EQUAL(std::string("8888888"),
                         trx->getRequest()->ticketingAgent()->homeAgencyIATA());
    CPPUNIT_ASSERT_EQUAL(std::string("AA"), trx->getRequest()->ticketingAgent()->vendorCrsCode());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("1S"), trx->getRequest()->ticketingAgent()->cxrCode());
    CPPUNIT_ASSERT_EQUAL(std::string("AUD"),
                         trx->getRequest()->ticketingAgent()->currencyCodeAgent());
    CPPUNIT_ASSERT_EQUAL(uint32_t(12),
                         trx->getRequest()->ticketingAgent()->agentCommissionAmount());
    CPPUNIT_ASSERT_EQUAL(std::string("*"), trx->getRequest()->ticketingAgent()->agentDuty());
    CPPUNIT_ASSERT_EQUAL(std::string("BB"),
                         trx->getRequest()->ticketingAgent()->agentCommissionType());
    CPPUNIT_ASSERT_EQUAL(int16_t(4), trx->getRequest()->ticketingAgent()->coHostID());
    static_cast<AncRequest*>(trx->getRequest())->setActiveAgent(AncRequest::CheckInAgent);
  }
  void testParseTAG_withTKN()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(
        _dataHandle, "<TAG A10=\"VIE\" A20=\"K7RF\" A21=\"K8RF\" A80=\"K9RF\" A90=\"-A8\" "
                     "AB0=\"9999999\" AB1=\"8888888\" AE0=\"AA\" B00=\"1S\" C40=\"AUD\" C6C=\"12\" "
                     "Q01=\"04\" N0G=\"*\" N0L=\"BB\" TKN=\"7\"/>"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    AncRequest* ancRq = static_cast<AncRequest*>(trx->getRequest());
    assert(ancRq->isTicketNumberValid(0) == false);
    assert(ancRq->isTicketNumberValid(7) == true);
    ancRq->setActiveAgent(AncRequest::TicketingAgent, 7);
    CPPUNIT_ASSERT_EQUAL(LocCode("VIE"), trx->getRequest()->ticketingAgent()->agentCity());
    CPPUNIT_ASSERT_EQUAL(PseudoCityCode("K7RF"),
                         trx->getRequest()->ticketingAgent()->tvlAgencyPCC());
    CPPUNIT_ASSERT_EQUAL(PseudoCityCode("K8RF"),
                         trx->getRequest()->ticketingAgent()->mainTvlAgencyPCC());
    CPPUNIT_ASSERT_EQUAL(std::string("K9RF"), trx->getRequest()->ticketingAgent()->airlineDept());
    CPPUNIT_ASSERT_EQUAL(std::string("-A8"), trx->getRequest()->ticketingAgent()->agentFunctions());
    CPPUNIT_ASSERT_EQUAL(std::string("9999999"),
                         trx->getRequest()->ticketingAgent()->tvlAgencyIATA());
    CPPUNIT_ASSERT_EQUAL(std::string("8888888"),
                         trx->getRequest()->ticketingAgent()->homeAgencyIATA());
    CPPUNIT_ASSERT_EQUAL(std::string("AA"), trx->getRequest()->ticketingAgent()->vendorCrsCode());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("1S"), trx->getRequest()->ticketingAgent()->cxrCode());
    CPPUNIT_ASSERT_EQUAL(std::string("AUD"),
                         trx->getRequest()->ticketingAgent()->currencyCodeAgent());
    CPPUNIT_ASSERT_EQUAL(uint32_t(12),
                         trx->getRequest()->ticketingAgent()->agentCommissionAmount());
    CPPUNIT_ASSERT_EQUAL(std::string("*"), trx->getRequest()->ticketingAgent()->agentDuty());
    CPPUNIT_ASSERT_EQUAL(std::string("BB"),
                         trx->getRequest()->ticketingAgent()->agentCommissionType());
    CPPUNIT_ASSERT_EQUAL(int16_t(4), trx->getRequest()->ticketingAgent()->coHostID());
    static_cast<AncRequest*>(trx->getRequest())->setActiveAgent(AncRequest::CheckInAgent);
  }
  void testParseTAG_NoCurrencyCode()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle, "<TAG A10=\"VIE\" A20=\"K7RF\"/>"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    static_cast<AncRequest*>(trx->getRequest())->setActiveAgent(AncRequest::TicketingAgent);
    // from DataHandle::getCustomer
    CPPUNIT_ASSERT_EQUAL(std::string("CUD"),
                         trx->getRequest()->ticketingAgent()->currencyCodeAgent());
    static_cast<AncRequest*>(trx->getRequest())->setActiveAgent(AncRequest::CheckInAgent);
  }
  void testParse_M70_BIL_A70Empty()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(
        _dataHandle, "<BIL A20=\"HDQ\" A22=\"K7RF\" A70=\"\" AA0=\"-A8\" AD0=\"61B670\" AE0=\"AA\" "
                     "C00=\"123456789012345678\" C01=\"189123788797592066\" C20=\"INTDWPI1\" "
                     "Q03=\"5642\" Q02=\"6148\" />"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(std::string("HDQ"), trx->billing()->userPseudoCityCode());
    CPPUNIT_ASSERT_EQUAL(std::string("K7RF"), trx->billing()->aaaCity());
    CPPUNIT_ASSERT_EQUAL(std::string(""), trx->billing()->actionCode());
    CPPUNIT_ASSERT_EQUAL(std::string("-A8"), trx->billing()->aaaSine());
    CPPUNIT_ASSERT_EQUAL(std::string("61B670"), trx->billing()->userSetAddress());
    CPPUNIT_ASSERT_EQUAL(std::string("AA"), trx->billing()->partitionID());
    CPPUNIT_ASSERT_EQUAL(uint64_t(123456789012345678), trx->billing()->parentTransactionID());
    CPPUNIT_ASSERT_EQUAL(uint64_t(189123788797592066), trx->billing()->clientTransactionID());
    CPPUNIT_ASSERT_EQUAL(std::string("INTDWPI1"), trx->billing()->parentServiceName());
    CPPUNIT_ASSERT_EQUAL(std::string("6148"), trx->billing()->userBranch());
    CPPUNIT_ASSERT_EQUAL(std::string("5642"), trx->billing()->userStation());
    CPPUNIT_ASSERT_EQUAL(AncRequest::M70Request,
                         static_cast<AncRequest*>(trx->getRequest())->ancRequestType());
  }
  void testParse_M70_BIL_A70_fromOTA()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(
        _dataHandle, "<BIL A20=\"HDQ\" A22=\"K7RF\" A70=\"A\" AA0=\"-A8\" AD0=\"61B670\" "
                     "AE0=\"AA\" C00=\"123456789012345678\" C01=\"189123788797592066\" "
                     "C20=\"INTDWPI1\" Q03=\"5642\" Q02=\"6148\" />"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(std::string("HDQ"), trx->billing()->userPseudoCityCode());
    CPPUNIT_ASSERT_EQUAL(std::string("K7RF"), trx->billing()->aaaCity());
    CPPUNIT_ASSERT_EQUAL(std::string("A"), trx->billing()->actionCode());
    CPPUNIT_ASSERT_EQUAL(std::string("-A8"), trx->billing()->aaaSine());
    CPPUNIT_ASSERT_EQUAL(std::string("61B670"), trx->billing()->userSetAddress());
    CPPUNIT_ASSERT_EQUAL(std::string("AA"), trx->billing()->partitionID());
    CPPUNIT_ASSERT_EQUAL(uint64_t(123456789012345678), trx->billing()->parentTransactionID());
    CPPUNIT_ASSERT_EQUAL(uint64_t(189123788797592066), trx->billing()->clientTransactionID());
    CPPUNIT_ASSERT_EQUAL(std::string("INTDWPI1"), trx->billing()->parentServiceName());
    CPPUNIT_ASSERT_EQUAL(std::string("6148"), trx->billing()->userBranch());
    CPPUNIT_ASSERT_EQUAL(std::string("5642"), trx->billing()->userStation());
    CPPUNIT_ASSERT_EQUAL(AncRequest::M70Request,
                         static_cast<AncRequest*>(trx->getRequest())->ancRequestType());
  }
  void testParseBIL_A70_WPAE()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle, "<BIL A70=\"WP*AE\" />"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(AncRequest::WPAERequest,
                         static_cast<AncRequest*>(trx->getRequest())->ancRequestType());
  }
  void testParseBIL_A70_WPAE_fromOTA()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle, "<BIL A70=\"I\" />"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(AncRequest::WPAERequest,
                         static_cast<AncRequest*>(trx->getRequest())->ancRequestType());
  }
  void testParseBIL_A70_WPAE_T()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle, "<BIL A70=\"WPAE*\" />"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(AncRequest::PostTktRequest,
                         static_cast<AncRequest*>(trx->getRequest())->ancRequestType());
  }
  void testParseBIL_A70_WPAE_T_fromOTA()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle, "<BIL A70=\"T\" />"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(AncRequest::PostTktRequest,
                         static_cast<AncRequest*>(trx->getRequest())->ancRequestType());
  }

  void testParseBIL_A70_Invalid_Value_Z()
  {
    CPPUNIT_ASSERT_THROW(_handler->parse(_dataHandle, "<BIL A70=\"Z\" />"), ErrorResponseException);
  }
  void testParseBIL_A70_Invalid_Value_WP()
  {
    CPPUNIT_ASSERT_THROW(_handler->parse(_dataHandle, "<BIL A70=\"WP\" />"),
                         ErrorResponseException);
  }
  void testParseBIL_A70_Invalid_Value_WPP()
  {
    CPPUNIT_ASSERT_THROW(_handler->parse(_dataHandle, "<BIL A70=\"WPP\" />"),
                         ErrorResponseException);
  }
  void testParseBIL_A70_Invalid_Value_WPAE()
  {
    CPPUNIT_ASSERT_THROW(_handler->parse(_dataHandle, "<BIL A70=\"WPAE\" />"),
                         ErrorResponseException);
  }
  void testParseBIL_A70_Invalid_Value_WPBET()
  {
    CPPUNIT_ASSERT_THROW(_handler->parse(_dataHandle, "<BIL A70=\"WPBET\" />"),
                         ErrorResponseException);
  }
  void testParseBIL_B05()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle, "<BIL B05=\"JJ\" />"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(std::string("JJ"),
                         trx->billing()->validatingCarrier());
  }
  void testParseBIL_No_B05()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle, "<BIL />"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(std::string(""),
                         trx->billing()->validatingCarrier());
  }

  void testParsePRO()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(
        _dataHandle,
        "<PRO AF0=\"LON\" AG0=\"LHR\" C45=\"GBP\" C10=\"198\" S15=\"TMPCRS,CRSAGT,MUL375\" />"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(LocCode("LON"), trx->getRequest()->ticketPointOverride());
    CPPUNIT_ASSERT_EQUAL(LocCode("LHR"), trx->getRequest()->salePointOverride());
    CPPUNIT_ASSERT_EQUAL(CurrencyCode("GBP"), trx->getOptions()->currencyOverride());
    CPPUNIT_ASSERT_EQUAL(short(198), trx->getRequest()->diagnosticNumber());
    CPPUNIT_ASSERT(trx->getOptions()->isMOverride());

    CPPUNIT_ASSERT_EQUAL(size_t(3), trx->getOptions()->eprKeywords().size());
    CPPUNIT_ASSERT(trx->getOptions()->eprKeywords().find("CRSAGT") !=
                   trx->getOptions()->eprKeywords().end());
    CPPUNIT_ASSERT(trx->getOptions()->eprKeywords().find("TMPCRS") !=
                   trx->getOptions()->eprKeywords().end());
    CPPUNIT_ASSERT(trx->getOptions()->eprKeywords().find("MUL375") !=
                   trx->getOptions()->eprKeywords().end());
  }
  void testPasrePRO_BlankS15()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(
        _dataHandle, "<PRO AF0=\"LON\" AG0=\"LHR\" C45=\"GBP\" C10=\"198\" S15=\"\" />"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(size_t(0), trx->getOptions()->eprKeywords().size());
  }
  void testParsePRO_NoS15()
  {
    CPPUNIT_ASSERT_NO_THROW(
        _handler->parse(_dataHandle, "<PRO AF0=\"LON\" AG0=\"LHR\" C45=\"GBP\" C10=\"198\" />"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(size_t(0), trx->getOptions()->eprKeywords().size());
  }
  void testParsePRO_TKI()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(
        _dataHandle, "<PRO AF0=\"LON\" AG0=\"LHR\" C45=\"GBP\" C10=\"198\" TKI=\"T\" />"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT(trx->getOptions()->isTicketingInd());
  }
  void testParsePRO_NoTKI()
  {
    CPPUNIT_ASSERT_NO_THROW(
        _handler->parse(_dataHandle, "<PRO AF0=\"LON\" AG0=\"LHR\" C45=\"GBP\" C10=\"198\" />"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT(!trx->getOptions()->isTicketingInd());
  }
  void testParseRFG()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle, "<RFG S01=\"XXX\" Q0A=\"1\" />"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(size_t(1), trx->getOptions()->serviceGroupsVec().size());
    CPPUNIT_ASSERT_EQUAL(ServiceGroup("XXX"),
                         trx->getOptions()->serviceGroupsVec().front().groupCode());
    CPPUNIT_ASSERT_EQUAL(int(1), trx->getOptions()->serviceGroupsVec().front().numberOfItems());
  }
  void testParseFFY()
  {
    CPPUNIT_ASSERT_NO_THROW(
        _handler->parse(_dataHandle, "<FFY Q7D=\"1\" B70=\"ADT\" B00=\"AA\" />"));
    CPPUNIT_ASSERT_EQUAL(size_t(1), _handler->_ffData.count("ADT"));
    CPPUNIT_ASSERT_EQUAL(uint16_t(1),
                         _handler->_ffData.lower_bound("ADT")->second->freqFlyerTierLevel());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), _handler->_ffData.lower_bound("ADT")->second->cxr());
  }
  void testParseFFY_Itin()
  {
    CPPUNIT_ASSERT_NO_THROW(_handlerM->parse(_dataHandle,
                                            "<AncillaryPricingRequest Version=\"3.0.0\">"
                                            "<AGI A10=\"VIE\" />"
                                            "<BIL S0R=\"PANC\" A70=\"A\"/>"
                                            "<PRO><RFG GBA=\"DXT\"></RFG></PRO>"
                                            + _itinFfyInAncPricingReq));

    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(size_t(1), trx->itin().size());
    AncRequest* req = dynamic_cast<AncRequest*>(trx->getRequest());
    Itin* itin = trx->itin().front();
    CPPUNIT_ASSERT_EQUAL(size_t(1), req->paxTypesPerItin()[itin].size());
    PaxType* paxType = req->paxTypesPerItin()[itin].front();
    CPPUNIT_ASSERT_EQUAL(size_t(1), paxType->freqFlyerTierWithCarrier().size());
    PaxType::FreqFlyerTierWithCarrier* ffy = paxType->freqFlyerTierWithCarrier().front();
    CPPUNIT_ASSERT_EQUAL(CarrierCode("BA"), ffy->cxr());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), ffy->freqFlyerTierLevel());
  }
  void testParseFFY_MixFails()
  {
    CPPUNIT_ASSERT_THROW(_handler->parse(_dataHandle,
                                         "<AncillaryPricingRequest Version=\"3.0.0\">"
                                         "<AGI A10=\"VIE\" />"
                                         "<BIL S0R=\"PANC\" A70=\"PRET\"/>"
                                         "<PRO><RFG GBA=\"DXT\"></RFG>"
                                         "<FFY Q7D=\"1\" B70=\"ADT\" B00=\"AA\" />"
                                         "</PRO>"
                                         + _itinFfyInAncPricingReq),
                         ErrorResponseException);
  }
  void testParseFFY_Ver3OutsideItinFails()
  {
    CPPUNIT_ASSERT_THROW(_handler->parse(_dataHandle,
                                         "<AncillaryPricingRequest Version=\"3.0.0\">"
                                         "<AGI A10=\"VIE\" />"
                                         "<BIL S0R=\"PANC\" A70=\"PRET\"/>"
                                         "<PRO><RFG GBA=\"DXT\"></RFG>"
                                         "<FFY Q7D=\"1\" B70=\"ADT\" B00=\"AA\" />"
                                         "</PRO>"
                                         + _itinInAncPricingReq),
                         ErrorResponseException);
  }
  void testParseFFY_Dups()
  {
    CPPUNIT_ASSERT_THROW(_handler->parse(_dataHandle, "<AN><FFY Q7D=\"1\" B70=\"ADT\" B00=\"AA\" "
                                                      "/><FFY Q7D=\"2\" B70=\"ADT\" B00=\"AA\" "
                                                      "/></AN>"),
                         ErrorResponseException);
  }
  void testParseDIG()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle, "<DIG Q0A=\"198\" S01=\"DDALL\" />"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(size_t(1), trx->getRequest()->diagArgType().size());
    CPPUNIT_ASSERT_EQUAL(std::string("198"), trx->getRequest()->diagArgType().front());
    CPPUNIT_ASSERT_EQUAL(size_t(1), trx->getRequest()->diagArgData().size());
    CPPUNIT_ASSERT_EQUAL(std::string("DDALL"), trx->getRequest()->diagArgData().front());
    CPPUNIT_ASSERT_EQUAL(std::string("ALL"), trx->diagnostic().diagParamMap()["DD"]);
  }
  void testParseFLI()
  {
    _handler->_currentTvlSeg = _memHandle.create<AirSeg>();
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(
        _dataHandle, "<FLI A01=\"LHR\" A02=\"BOS\" D01=\"2010-12-01\" D31=\"420\" "
                     "D02=\"2010-12-02\" D32=\"515\" B00=\"BA\" B01=\"AA\" B30=\"Y\" Q0B=\"412\" "
                     "S95=\"FLT\" N0E=\"Y\" BB0=\"QF\" C7A=\"12\" C7B=\"T\" />"));
    CPPUNIT_ASSERT_EQUAL(LocCode("LHR"), _handler->_currentTvlSeg->origAirport());
    CPPUNIT_ASSERT_EQUAL(LocCode("BOS"), _handler->_currentTvlSeg->destAirport());
    CPPUNIT_ASSERT_EQUAL(DateTime(2010, 12, 01, 4, 20, 0), _handler->_currentTvlSeg->departureDT());
    CPPUNIT_ASSERT_EQUAL(DateTime(2010, 12, 02, 5, 15, 0), _handler->_currentTvlSeg->arrivalDT());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("BA"), _handler->_currentTvlSeg->marketingCarrierCode());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), _handler->_currentTvlSeg->operatingCarrierCode());
    CPPUNIT_ASSERT_EQUAL(BookingCode("Y"), _handler->_currentTvlSeg->getBookingCode());
    CPPUNIT_ASSERT_EQUAL(FlightNumber(412), _handler->_currentTvlSeg->marketingFlightNumber());
    CPPUNIT_ASSERT_EQUAL(EquipmentType("FLT"), _handler->_currentTvlSeg->equipmentType());
    CPPUNIT_ASSERT_EQUAL(std::string("QF"), _handler->_currentTvlSeg->resStatus());
    CPPUNIT_ASSERT_EQUAL('T', _handler->_currentTvlSeg->checkedPortionOfTravelInd());
  }
  void testParseFLI_Fail_ClassOfService()
  {
    _handler->_currentTvlSeg = _memHandle.create<AirSeg>();
    CPPUNIT_ASSERT_THROW(
        _handler->parse(_dataHandle, "<AncillaryPricingRequest Version=\"2.0.0\"><FLI A01=\"LHR\" "
                                     "A02=\"BOS\" D01=\"2010-12-01\" D31=\"420\" "
                                     "D02=\"2010-12-02\" D32=\"515\" B00=\"BA\" B01=\"AA\" "
                                     "Q0B=\"412\" S95=\"FLT\" N0E=\"Y\" BB0=\"QF\" C7A=\"12\" "
                                     "C7B=\"T\" /></AncillaryPricingRequest>"),
        ErrorResponseException);
  }
  void testParseFLI_Fail_NoMarketingCarrier()
  {
    _handler->_currentTvlSeg = _memHandle.create<AirSeg>();
    CPPUNIT_ASSERT_THROW(_handler->parse(_dataHandle, "<FLI A01=\"LHR\" A02=\"BOS\" B01=\"BA\" />"),
                         ErrorResponseException);
  }
  void testParseFLI_NoOperatingCarrier()
  {
    _handler->_currentTvlSeg = _memHandle.create<AirSeg>();
    CPPUNIT_ASSERT_NO_THROW(
        _handler->parse(_dataHandle, "<FLI A01=\"LHR\" A02=\"BOS\" B00=\"BA\" />"));
    CPPUNIT_ASSERT_EQUAL(CarrierCode("BA"), _handler->_currentTvlSeg->marketingCarrierCode());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("BA"), _handler->_currentTvlSeg->operatingCarrierCode());
  }
  void testParseFLI_Fail_NoOrigAirport()
  {
    _handler->_currentTvlSeg = _memHandle.create<AirSeg>();
    CPPUNIT_ASSERT_THROW(_handler->parse(_dataHandle, "<FLI A02=\"BOS\" B00=\"BA\" B01=\"BA\" />"),
                         ErrorResponseException);
  }
  void testParseFLI_Fail_NoDestAirport()
  {
    _handler->_currentTvlSeg = _memHandle.create<AirSeg>();
    CPPUNIT_ASSERT_THROW(_handler->parse(_dataHandle, "<FLI A01=\"LHR\" B00=\"BA\" B01=\"BA\" />"),
                         ErrorResponseException);
  }
  void testParseFBA()
  {
    _handler->_currentTvlSeg = _memHandle.create<AirSeg>();
    _handler->_itin = _memHandle.create<Itin>();
    CPPUNIT_ASSERT_NO_THROW(
        _handler->parse(_dataHandle, "<FBA Q6D=\"1\" Q6E=\"2\" S07=\"T\" S08=\"F\" />"));
    AncRequest* req =
        dynamic_cast<AncRequest*>(dynamic_cast<AncillaryPricingTrx*>(_trx)->getRequest());
    CPPUNIT_ASSERT(req != 0);
    CPPUNIT_ASSERT_EQUAL(size_t(1), req->fareBreakAssociationPerItin()[_handler->_itin].size());
    CPPUNIT_ASSERT_EQUAL(SequenceNumber(0),
                         req->fareBreakAssociationPerItin()[_handler->_itin].front()->segmentID());
    CPPUNIT_ASSERT_EQUAL(
        SequenceNumber(1),
        req->fareBreakAssociationPerItin()[_handler->_itin].front()->fareComponentID());
    CPPUNIT_ASSERT_EQUAL(SequenceNumber(2),
                         req->fareBreakAssociationPerItin()[_handler->_itin].front()->sideTripID());
    CPPUNIT_ASSERT_EQUAL(
        true, req->fareBreakAssociationPerItin()[_handler->_itin].front()->sideTripStart());
    CPPUNIT_ASSERT_EQUAL(
        false, req->fareBreakAssociationPerItin()[_handler->_itin].front()->sideTripEnd());
  }
  void testParsePXI()
  {
    _handler->_currentTvlSeg = _memHandle.create<AirSeg>();
    // need ticketing agent for GetFareCalcConfig
    CPPUNIT_ASSERT_NO_THROW(
        _handler->parse(_dataHandle, "<AN><AGI A10=\"VIE\" /><PXI B70=\"ADT\" Q0U=\"1\" /></AN>"));
    AncRequest* req =
        dynamic_cast<AncRequest*>(dynamic_cast<AncillaryPricingTrx*>(_trx)->getRequest());
    CPPUNIT_ASSERT(req != 0);
    CPPUNIT_ASSERT_EQUAL(size_t(1), req->paxTypesPerItin()[_handler->_itin].size());
    CPPUNIT_ASSERT_EQUAL(PaxTypeCode("ADT"),
                         req->paxTypesPerItin()[_handler->_itin].front()->paxType());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), req->paxTypesPerItin()[_handler->_itin].front()->number());
  }
  void testParsePXI_C07()
  {
    _handler->_currentTvlSeg = _memHandle.create<AirSeg>();
    // need ticketing agent for GetFareCalcConfig
    CPPUNIT_ASSERT_NO_THROW(
        _handler->parse(_dataHandle, "<AN><AGI A10=\"VIE\" /><PXI B70=\"C07\" Q0U=\"1\" /></AN>"));
    AncRequest* req =
        dynamic_cast<AncRequest*>(dynamic_cast<AncillaryPricingTrx*>(_trx)->getRequest());
    CPPUNIT_ASSERT(req != 0);
    CPPUNIT_ASSERT_EQUAL(size_t(1), req->paxTypesPerItin()[_handler->_itin].size());
    CPPUNIT_ASSERT_EQUAL(PaxTypeCode("CNN"),
                         req->paxTypesPerItin()[_handler->_itin].front()->paxType());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), req->paxTypesPerItin()[_handler->_itin].front()->number());
    CPPUNIT_ASSERT_EQUAL(uint16_t(7), req->paxTypesPerItin()[_handler->_itin].front()->age());
  }
  void testParseACI()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle, "<ACI ACC=\"AC01\" />"));
    AncRequest* req =
        dynamic_cast<AncRequest*>(dynamic_cast<AncillaryPricingTrx*>(_trx)->getRequest());
    CPPUNIT_ASSERT(req != 0);
    CPPUNIT_ASSERT_EQUAL(size_t(1), req->accountCodeIdPerItin()[_handler->_itin].size());
    CPPUNIT_ASSERT_EQUAL(std::string("AC01"), req->accountCodeIdPerItin()[_handler->_itin].front());
  }
  void testParseCII_Valid()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle, "<CII CID=\"VALID\" />"));
    AncRequest* req =
        dynamic_cast<AncRequest*>(dynamic_cast<AncillaryPricingTrx*>(_trx)->getRequest());
    CPPUNIT_ASSERT(req != 0);
    CPPUNIT_ASSERT_EQUAL(size_t(1), req->corpIdPerItin()[_handler->_itin].size());
    CPPUNIT_ASSERT_EQUAL(std::string("VALID"), req->corpIdPerItin()[_handler->_itin].front());
  }
  void testParseCII_Invalid()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle, "<CII CID=\"INVALID\" />"));
    AncRequest* req =
        dynamic_cast<AncRequest*>(dynamic_cast<AncillaryPricingTrx*>(_trx)->getRequest());
    CPPUNIT_ASSERT(req != 0);
    CPPUNIT_ASSERT_EQUAL(size_t(1), req->invalidCorpIdPerItin()[_handler->_itin].size());
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID"),
                         req->invalidCorpIdPerItin()[_handler->_itin].front());
  }
  void testParseFBI()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle, "<FBI Q00=\"1\" B02=\"BA\" C50=\"123.45\" "
                                                         "B50=\"Y3\" S53=\"XEX\" Q3W=\"123\" "
                                                         "S90=\"0001\" Q3V=\"25\" P1Z=\"T\" />"));
    AncRequest* req =
        dynamic_cast<AncRequest*>(dynamic_cast<AncillaryPricingTrx*>(_trx)->getRequest());
    CPPUNIT_ASSERT(req != 0);
    CPPUNIT_ASSERT_EQUAL(size_t(1), req->fareBreakPerItin()[_handler->_itin].size());
    CPPUNIT_ASSERT_EQUAL(SequenceNumber(1),
                         req->fareBreakPerItin()[_handler->_itin].front()->fareComponentID());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("BA"),
                         req->fareBreakPerItin()[_handler->_itin].front()->governingCarrier());
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(123.45),
                         req->fareBreakPerItin()[_handler->_itin].front()->fareAmount());
    CPPUNIT_ASSERT_EQUAL(FareClassCode("Y3"),
                         req->fareBreakPerItin()[_handler->_itin].front()->fareBasis());
    CPPUNIT_ASSERT_EQUAL(FareType("XEX"),
                         req->fareBreakPerItin()[_handler->_itin].front()->fareType());
    CPPUNIT_ASSERT_EQUAL(TariffNumber(123),
                         req->fareBreakPerItin()[_handler->_itin].front()->fareTariff());
    CPPUNIT_ASSERT_EQUAL(RuleNumber("0001"),
                         req->fareBreakPerItin()[_handler->_itin].front()->fareRule());
    CPPUNIT_ASSERT_EQUAL(uint16_t(25),
                         req->fareBreakPerItin()[_handler->_itin].front()->fareIndicator());
    CPPUNIT_ASSERT_EQUAL(true,
                         req->fareBreakPerItin()[_handler->_itin].front()->privateIndicator());
  }
  void testParseIRO_NoPaxType()
  {
    // if no pax type, it will add ADT
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(
        _dataHandle, "<AN><AGI A10=\"VIE\" /><ITN Q00=\"1\" ><IRO SHC=\"TOUR\" /><SGI "
                     "Q00=\"1\"><FLI A01=\"LHR\" A02=\"VIE\" D01=\"2050-01-01\" D31=\"420\" "
                     "D02=\"2050-01-01\" D32=\"515\" B00=\"AA\" /></SGI></ITN></AN>"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    AncRequest* req = dynamic_cast<AncRequest*>(trx->getRequest());
    CPPUNIT_ASSERT(req != 0);
    CPPUNIT_ASSERT_EQUAL(size_t(1), trx->itin().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), req->paxTypesPerItin()[trx->itin()[0]].size());
    CPPUNIT_ASSERT_EQUAL(PaxTypeCode("ADT"),
                         req->paxTypesPerItin()[trx->itin()[0]].front()->paxType());
  }
  void testParseIRO_FreqFlyerStatus()
  {
    // will populate FrequentFlyerStatus
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(
        _dataHandle, "<AN><AGI A10=\"VIE\" /><FFY Q7D=\"5\" B70=\"ADT\" B00=\"AA\" /><IRO "
                     "SHC=\"TOUR\" ANG=\"T\"><PXI B70=\"ADT\" Q0U=\"2\" /></IRO></AN>"));
    AncRequest* req =
        dynamic_cast<AncRequest*>(dynamic_cast<AncillaryPricingTrx*>(_trx)->getRequest());
    CPPUNIT_ASSERT(req != 0);
    // check tour code
    CPPUNIT_ASSERT_EQUAL(std::string("TOUR"), req->tourCodePerItin()[_handler->_itin]);
    // check ancillaries non guarantee indicator
    CPPUNIT_ASSERT_EQUAL(true, req->ancillNonGuaranteePerItin()[_handler->_itin]);
    // check Freq if fereq flyer is set
    CPPUNIT_ASSERT_EQUAL(size_t(1), req->paxTypesPerItin()[_handler->_itin].size());
    CPPUNIT_ASSERT_EQUAL(
        size_t(1),
        req->paxTypesPerItin()[_handler->_itin].front()->freqFlyerTierWithCarrier().size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5),
                         req->paxTypesPerItin()[_handler->_itin]
                             .front()
                             ->freqFlyerTierWithCarrier()
                             .front()
                             ->freqFlyerTierLevel());
    CPPUNIT_ASSERT_EQUAL(
        CarrierCode("AA"),
        req->paxTypesPerItin()[_handler->_itin].front()->freqFlyerTierWithCarrier().front()->cxr());
  }
  void testParseIRO_MaxAccontCodes()
  {
    // max number of ACI is 4
    CPPUNIT_ASSERT_THROW(_handler->parse(_dataHandle, "<AN><AGI A10=\"VIE\" /><IRO><ACI "
                                                      "ACC=\"AC01\" /><ACI ACC=\"AC02\" /><ACI "
                                                      "ACC=\"AC03\" /><ACI ACC=\"AC04\" /><ACI "
                                                      "ACC=\"AC05\" /></IRO></AN>"),
                         ErrorResponseException);
  }
  void testParseIRO_MaxCorpId()
  {
    // max number of CII is 4
    CPPUNIT_ASSERT_THROW(_handler->parse(_dataHandle, "<AN><AGI A10=\"VIE\" /><IRO><CII "
                                                      "CID=\"VALID\" /><CII CID=\"VALID2\" /><CII "
                                                      "CID=\"VALID3\" /><CII CID=\"INVALID\" "
                                                      "/><CII CID=\"INVALID2\" /></IRO></AN>"),
                         ErrorResponseException);
  }
  void testParseIRO_TicketIssueDate()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(
        _dataHandle, "<AN><AGI A10=\"LHR\" /><ITN Q00=\"1\" ><IRO D07=\"2012-12-21\" /><SGI "
                     "Q00=\"1\"><FLI A01=\"LHR\" A02=\"VIE\" D01=\"2050-01-01\" D31=\"420\" "
                     "D02=\"2050-01-01\" D32=\"515\" B00=\"AA\" /></SGI></ITN></AN>"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    AncRequest* req = dynamic_cast<AncRequest*>(trx->getRequest());
    CPPUNIT_ASSERT(req);
    CPPUNIT_ASSERT(req->ticketingDatesPerItin().find(trx->itin().front()) !=
                   req->ticketingDatesPerItin().end());
    CPPUNIT_ASSERT_EQUAL(
        DateTime(2012, 12, 21).toSimpleString(),
        req->ticketingDatesPerItin().find(trx->itin().front())->second.toSimpleString());
  }
  void testParseIRO_TicketIssueDate_Empty()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(
        _dataHandle, "<AN><AGI A10=\"LHR\" /><ITN Q00=\"1\" ><IRO /><SGI Q00=\"1\"><FLI "
                     "A01=\"LHR\" A02=\"VIE\" D01=\"2050-01-01\" D31=\"420\" D02=\"2050-01-01\" "
                     "D32=\"515\" B00=\"AA\" /></SGI></ITN></AN>"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    AncRequest* req = dynamic_cast<AncRequest*>(trx->getRequest());
    CPPUNIT_ASSERT(req);
    CPPUNIT_ASSERT(req->ticketingDatesPerItin().find(trx->itin().front()) ==
                   req->ticketingDatesPerItin().end());
  }
  void testParseOSC_givenOscIsNotInItn_shouldNotStoreOscDataInItin()
  {
    CPPUNIT_ASSERT_NO_THROW(_handlerM->parse(_dataHandle,
      R"(<AncillaryPricingRequest Version="1.0.0">
         <ITN>
           <SGI><FLI A01="LO1" A02="LO2" B00="AA" /></SGI>
         </ITN>
         <OSC>This shouldn't be parsed</OSC>
         </AncillaryPricingRequest>)"));

    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(0UL, trx->itin()[0]->getAncillaryPriceModifiers().size());
  }
  void testParseOSC_givenOscIsInItn_whenOscHasAidAttribute_shouldStoreAidInItin()
  {
    CPPUNIT_ASSERT_NO_THROW(_handlerM->parse(_dataHandle,
      R"(<AncillaryPricingRequest Version="1.0.0">
         <ITN>
           <SGI><FLI A01="LO1" A02="LO2" B00="AA" /></SGI>
           <OSC AID=")" + _validAid1 + R"(" />
         </ITN>
         </AncillaryPricingRequest>)"));

    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_NO_THROW(trx->itin()[0]->getAncillaryPriceModifiers().at(AncillaryIdentifier(_validAid1)));
  }
  void testParseOSC_givenOscIsInItn_whenOscDoesNotHaveAidAttribute_shouldThrow()
  {
    CPPUNIT_ASSERT_THROW(_handlerM->parse(_dataHandle,
      R"(<AncillaryPricingRequest Version="1.0.0">
         <ITN>
           <SGI><FLI A01="LO1" A02="LO2" B00="AA" /></SGI>
           <OSC ID="not AID" />
         </ITN>
         </AncillaryPricingRequest>)"), ErrorResponseException);
  }
  void testParseOSC_givenOscIsInItn_whenOscHasPmiAttribute_shouldStorePmiInItin()
  {
    CPPUNIT_ASSERT_NO_THROW(_handlerM->parse(_dataHandle,
      R"(<AncillaryPricingRequest Version="1.0.0">
         <ITN>
           <SGI><FLI A01="LO1" A02="LO2" B00="AA" /></SGI>
           <OSC AID=")" + _validAid1 + R"(" PMI="5eX" />
         </ITN>
         </AncillaryPricingRequest>)"));

    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    AncillaryIdentifier aid{_validAid1};
    AncillaryPriceModifier modifier = trx->itin()[0]->getAncillaryPriceModifiers().at(aid)[0];
    CPPUNIT_ASSERT("5eX" == modifier._identifier.get());
  }
  void testParseOSC_givenOscIsInItn_whenOscDoesNotHavePmiAttribute_shouldStorePmiInItinWithNoValue()
  {
    CPPUNIT_ASSERT_NO_THROW(_handlerM->parse(_dataHandle,
      R"(<AncillaryPricingRequest Version="1.0.0">
         <ITN>
           <SGI><FLI A01="LO1" A02="LO2" B00="AA" /></SGI>
           <OSC AID=")" + _validAid1 + R"(" />
         </ITN>
         </AncillaryPricingRequest>)"));

    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    AncillaryIdentifier aid{_validAid1};
    AncillaryPriceModifier modifier = trx->itin()[0]->getAncillaryPriceModifiers().at(aid)[0];
    CPPUNIT_ASSERT(!modifier._identifier);
  }
  void testParseOSC_givenTwoOscInItn_whenTwoPmiAttributesHaveTheSameValue_shouldThrow()
  {
    CPPUNIT_ASSERT_THROW(_handlerM->parse(_dataHandle,
      R"(<AncillaryPricingRequest Version="1.0.0">
         <ITN>
           <SGI><FLI A01="LO1" A02="LO2" B00="AA" /></SGI>
           <OSC AID=")" + _validAid1 + R"(" PMI="5eX" />
           <OSC AID=")" + _validAid1 + R"(" PMI="5eX" />
         </ITN>
         </AncillaryPricingRequest>)"), ErrorResponseException);
  }
  void testParseOSC_givenOscIsInItn_whenOscHasQtyAttribute_shouldStoreQtyInItin()
  {
    CPPUNIT_ASSERT_NO_THROW(_handlerM->parse(_dataHandle,
      R"(<AncillaryPricingRequest Version="1.0.0">
         <ITN>
           <SGI><FLI A01="LO1" A02="LO2" B00="AA" /></SGI>
           <OSC AID=")" + _validAid1 + R"(" QTY="69" />
         </ITN>
         </AncillaryPricingRequest>)"));

    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    AncillaryIdentifier aid{_validAid1};
    AncillaryPriceModifier modifier = trx->itin()[0]->getAncillaryPriceModifiers().at(aid)[0];
    CPPUNIT_ASSERT_EQUAL(69U, modifier._quantity);
  }
  void testParseOSC_givenOscIsInItn_whenOscDoesNotHaveAnyAttributesBesidesAid_shouldCreateDefaultAncPriceModifier()
  {
    CPPUNIT_ASSERT_NO_THROW(_handlerM->parse(_dataHandle,
      R"(<AncillaryPricingRequest Version="1.0.0">
         <ITN>
           <SGI><FLI A01="LO1" A02="LO2" B00="AA" /></SGI>
           <OSC AID=")" + _validAid1 + R"(" />
         </ITN>
         </AncillaryPricingRequest>)"));

    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    AncillaryIdentifier aid{_validAid1};
    AncillaryPriceModifier modifier = trx->itin()[0]->getAncillaryPriceModifiers().at(aid)[0];
    CPPUNIT_ASSERT_EQUAL(1U, modifier._quantity);
    CPPUNIT_ASSERT(!modifier._percentage);
  }
  void testParseOSC_givenOscIsInItn_whenOscHasQtyAttributeThatIsNotAnInteger_shouldThrow()
  {
    CPPUNIT_ASSERT_THROW(_handlerM->parse(_dataHandle,
      R"(<AncillaryPricingRequest Version="1.0.0">
               <ITN>
                 <SGI><FLI A01="LO1" A02="LO2" B00="AA" /></SGI>
                 <OSC AID=")" + _validAid1 + R"(" QTY="6.5" />
               </ITN>
               </AncillaryPricingRequest>)"), ErrorResponseException);
  }
  void testParseOSC_givenOscIsInItn_whenOscHasDrpAttribute_shouldStoreDrpInItin()
  {
    CPPUNIT_ASSERT_NO_THROW(_handlerM->parse(_dataHandle,
      R"(<AncillaryPricingRequest Version="1.0.0">
         <ITN>
           <SGI><FLI A01="LO1" A02="LO2" B00="AA" /></SGI>
           <OSC AID=")" + _validAid1 + R"(" DRP="30" />
         </ITN>
         </AncillaryPricingRequest>)"));

    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    AncillaryIdentifier aid{_validAid1};
    AncillaryPriceModifier modifier = trx->itin()[0]->getAncillaryPriceModifiers().at(aid)[0];
    CPPUNIT_ASSERT(modifier._percentage);
    CPPUNIT_ASSERT_EQUAL(30U, modifier._percentage.get());
  }
  void testParseOSC_givenOscIsInItn_whenOscHasDrpAttributeThatIsNotAnInteger_shouldThrow()
  {
    CPPUNIT_ASSERT_THROW(_handlerM->parse(_dataHandle,
      R"(<AncillaryPricingRequest Version="1.0.0">
               <ITN>
                 <SGI><FLI A01="LO1" A02="LO2" B00="AA" /></SGI>
                 <OSC AID=")" + _validAid1 + R"(" DRP="6.5" />
               </ITN>
               </AncillaryPricingRequest>)"), ErrorResponseException);
  }
  void testParseOSC_givenOscIsInItn_whenOscHasDrpDiscountAttributeThatIsGreaterThan100_shouldThrow()
  {
    CPPUNIT_ASSERT_THROW(_handlerM->parse(_dataHandle,
      R"(<AncillaryPricingRequest Version="1.0.0">
               <ITN>
                 <SGI><FLI A01="LO1" A02="LO2" B00="AA" /></SGI>
                 <OSC AID=")" + _validAid1 + R"(" DRT="D" DRP="102" />
               </ITN>
               </AncillaryPricingRequest>)"), ErrorResponseException);
  }

  void testParseOSC_givenOscIsInItn_whenOscHasDrpMarkupAttributeThatIsGreaterThan100_shouldNotThrow()
  {
    CPPUNIT_ASSERT_NO_THROW(_handlerM->parse(_dataHandle,
      R"(<AncillaryPricingRequest Version="1.0.0">
               <ITN>
                 <SGI><FLI A01="LO1" A02="LO2" B00="AA" /></SGI>
                 <OSC AID=")" + _validAid1 + R"(" DRT="R" DRP="102" />
               </ITN>
               </AncillaryPricingRequest>)"));
  }

  void testParseOSC_givenOscIsInItn_whenOscHasDrpAttributeThatIsLessThan0_shouldThrow()
  {
    CPPUNIT_ASSERT_THROW(_handlerM->parse(_dataHandle,
      R"(<AncillaryPricingRequest Version="1.0.0">
               <ITN>
                 <SGI><FLI A01="LO1" A02="LO2" B00="AA" /></SGI>
                 <OSC AID=")" + _validAid1 + R"(" DRP="-15" />
               </ITN>
               </AncillaryPricingRequest>)"), ErrorResponseException);
  }
  void testParseOSC_givenOscIsInItn_whenOscHasDrpAndDrm_shouldThrow()
  {
    CPPUNIT_ASSERT_THROW(_handlerM->parse(_dataHandle,
      R"(<AncillaryPricingRequest Version="1.0.0">
               <ITN>
                 <SGI><FLI A01="LO1" A02="LO2" B00="AA" /></SGI>
                 <OSC AID=")" + _validAid1 + R"(" DRP="30" DRT="D" DRM="15" DRC="USD" />
               </ITN>
               </AncillaryPricingRequest>)"), ErrorResponseException);
  }
  void testParseOSC_givenOscIsInItn_whenOscHasDrmAttributeThatIsLessThan0_shouldThrow()
  {
    CPPUNIT_ASSERT_THROW(_handlerM->parse(_dataHandle,
      R"(<AncillaryPricingRequest Version="1.0.0">
               <ITN>
                 <SGI><FLI A01="LO1" A02="LO2" B00="AA" /></SGI>
                 <OSC AID=")" + _validAid1 + R"(" DRT="D" DRM="-15.0" DRC="USD" />
               </ITN>
               </AncillaryPricingRequest>)"), ErrorResponseException);
  }
  void testParseOSC_givenOscIsInItn_whenOscHasDrmAttribute_shouldStoreDrmInItin()
  {
    CPPUNIT_ASSERT_NO_THROW(_handlerM->parse(_dataHandle,
      R"(<AncillaryPricingRequest Version="1.0.0">
         <ITN>
           <SGI><FLI A01="LO1" A02="LO2" B00="AA" /></SGI>
           <OSC AID=")" + _validAid1 + R"(" DRT="D" DRM="10.5" DRC="USD" />
         </ITN>
         </AncillaryPricingRequest>)"));

    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    AncillaryIdentifier aid{_validAid1};
    AncillaryPriceModifier modifier = trx->itin()[0]->getAncillaryPriceModifiers().at(aid)[0];
    CPPUNIT_ASSERT(modifier._money);
    CPPUNIT_ASSERT_EQUAL(10.5, modifier._money.get().value());
    CPPUNIT_ASSERT_EQUAL(CurrencyCode("USD"), modifier._money.get().code());
  }
  void testParseOSC_givenMultipleItnWithMultipleOsc_whenEveryOscIsValid_shouldStoreAllOscInItin()
  {
    CPPUNIT_ASSERT_NO_THROW(_handlerM->parse(_dataHandle,
      R"(<AncillaryPricingRequest Version="1.0.0">
         <ITN>
           <SGI><FLI A01="LO1" A02="LO2" B00="AA" /></SGI>
           <OSC AID=")" + _validAid1 + R"(" />
           <OSC AID=")" + _validAid1 + R"(" />
           <OSC AID=")" + _validAid1 + R"(" PMI="6" />
           <OSC AID=")" + _validAid2 + R"(" PMI="7" QTY="5" />
         </ITN>
         <ITN>
           <SGI><FLI A01="LO1" A02="LO2" B00="AA" /></SGI>
           <OSC AID=")" + _validAid2 + R"(" />
         </ITN>
         </AncillaryPricingRequest>)"));

    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    AncillaryIdentifier aid1{_validAid1};
    AncillaryIdentifier aid2{_validAid2};

    CPPUNIT_ASSERT_EQUAL(2UL, trx->itin()[0]->getAncillaryPriceModifiers().size());
    CPPUNIT_ASSERT_EQUAL(3UL, trx->itin()[0]->getAncillaryPriceModifiers().at(aid1).size());
    CPPUNIT_ASSERT_EQUAL(1UL, trx->itin()[0]->getAncillaryPriceModifiers().at(aid2).size());
    CPPUNIT_ASSERT_EQUAL(1UL, trx->itin()[1]->getAncillaryPriceModifiers().size());
    CPPUNIT_ASSERT_EQUAL(1UL, trx->itin()[1]->getAncillaryPriceModifiers().at(aid2).size());
    CPPUNIT_ASSERT_THROW(trx->itin()[1]->getAncillaryPriceModifiers().at(aid1), std::out_of_range);
  }
  void testParseITN_AddArunk()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(
        _dataHandle, "<AN><AGI A10=\"VIE\" /><ITN><SGI><FLI A01=\"LO1\" A02=\"LO2\" B00=\"AA\" "
                     "/></SGI><SGI><FLI A01=\"LO3\" A02=\"LO4\" B00=\"AA\" /></SGI></ITN></AN>"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(size_t(1), trx->itin().size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), trx->itin().front()->travelSeg().size());
    CPPUNIT_ASSERT(trx->itin().front()->travelSeg()[1]->segmentType() != Air);
    CPPUNIT_ASSERT_EQUAL(int16_t(1), trx->itin().front()->travelSeg()[0]->segmentOrder());
    CPPUNIT_ASSERT_EQUAL(int16_t(2), trx->itin().front()->travelSeg()[1]->segmentOrder());
    CPPUNIT_ASSERT_EQUAL(int16_t(3), trx->itin().front()->travelSeg()[2]->segmentOrder());
  }
  void testParseITN_SuccessHistorical()
  {
    // historical date
    _handler->parse(_dataHandle,
                    "<AncillaryPricingRequest Version=\"1.0.0\">"
                    "<AGI A10=\"LHR\" />"
                    "<ITN Q00=\"1\">"
                    "<SGI Q00=\"1\">"
                    "<FLI A01=\"LHR\" A02=\"BOS\" D01=\"1981-01-01\" "
                    "D31=\"420\" D02=\"2010-12-02\" D32=\"515\" B00=\"BA\" "
                    "B01=\"AA\" B30=\"Y\" Q0B=\"412\" S95=\"FLT\" N0E=\"Y\" "
                    "BB0=\"QF\" C7A=\"12\" C7B=\"T\" />"
                    "</SGI>"
                    "</ITN>"
                    "</AncillaryPricingRequest>");
  }
  void testParseITN_FailDateOrder()
  {
    // first segment later then first
    CPPUNIT_ASSERT_THROW(
        _handler->parse(_dataHandle,
                        ""
                        "<AncillaryPricingRequest Version=\"1.0.0\">"
                        "<AGI A10=\"LHR\" />"
                        "<ITN Q00=\"1\">"
                        "<SGI Q00=\"1\">"
                        "<FLI A01=\"LHR\" A02=\"BOS\" D01=\"2050-02-02\" D31=\"420\" "
                        "D02=\"2050-02-02\" D32=\"515\" B00=\"BA\" B01=\"AA\" B30=\"Y\" "
                        "Q0B=\"412\" S95=\"FLT\" N0E=\"Y\" BB0=\"QF\" C7A=\"12\" C7B=\"T\" />"
                        "</SGI>"
                        "<SGI>"
                        "<FLI A01=\"LHR\" A02=\"BOS\" D01=\"2050-01-01\" D31=\"420\" "
                        "D02=\"2050-01-01\" D32=\"515\" B00=\"BA\" B01=\"AA\" B30=\"Y\" "
                        "Q0B=\"412\" S95=\"FLT\" N0E=\"Y\" BB0=\"QF\" C7A=\"12\" C7B=\"T\" />"
                        "</SGI>"
                        "</ITN>"
                        "</AncillaryPricingRequest>"),
        ErrorResponseException);
  }
  void testParseITN_WPAENoRFG()
  {
    _handlerM->_reqType = AncillaryPricingRequestHandler::WPAE;
    CPPUNIT_ASSERT_NO_THROW(_handlerM->parse(
        _dataHandle, "<AN><AGI A10=\"VIE\" /><ITN><SGI><FLI A01=\"LO3\" A02=\"LO4\" B00=\"BA\" "
                     "B01=\"BA\" B30=\"Y\" D01=\"2025-01-01\" D02=\"2025-01-02\" D31=\"3\" "
                     "D32=\"4\" Q0B=\"5\" S95=\"6\" N0E=\"Y\" BB0=\"QF\" /><FBA Q6D=\"1\" "
                     "/></SGI><FBI Q00=\"1\" B02=\"BA\" C50=\"123.45\" B50=\"Y3\" S53=\"XEX\" "
                     "S37=\"ATP\" Q3W=\"123\" S90=\"0001\" Q3V=\"25\" P1Z=\"T\"  FTY=\"00\" "
                     "/></ITN></AN>"));
    CPPUNIT_ASSERT(_handlerM->_pricingTrx->getOptions()->isProcessAllGroups());
  }
  void testConvertDate_Match()
  {
    DateTime dt;
    CPPUNIT_ASSERT_NO_THROW(dt = _handler->convertDate(std::string("2010-12-12")));
    CPPUNIT_ASSERT_EQUAL(DateTime(2010, 12, 12), dt);
  }
  void testConvertDate_Fail_letter()
  {
    CPPUNIT_ASSERT_THROW(_handler->convertDate(std::string("201A-12-12")), ErrorResponseException);
  }
  void testConvertDate_Fail_format()
  {
    CPPUNIT_ASSERT_THROW(_handler->convertDate(std::string("10-12-12")), ErrorResponseException);
  }

  void testCheckForDateConsistency_First_NoDate()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->checkForDateConsistency(std::string("")));
    CPPUNIT_ASSERT_EQUAL(AncillaryPricingRequestHandler::DATE_NOT_PRESENT,
                         _handler->_datesInRequest);
  }
  void testCheckForDateConsistency_First_WithDate()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->checkForDateConsistency(std::string("2010-12-12")));
    CPPUNIT_ASSERT_EQUAL(AncillaryPricingRequestHandler::DATE_IN_FRIST_SEGMENT,
                         _handler->_datesInRequest);
  }
  void testCheckForDateConsistency_WasFirstDate_NoDate()
  {
    _handler->_datesInRequest = AncillaryPricingRequestHandler::DATE_IN_FRIST_SEGMENT;
    CPPUNIT_ASSERT_NO_THROW(_handler->checkForDateConsistency(std::string("")));
    CPPUNIT_ASSERT_EQUAL(AncillaryPricingRequestHandler::DATE_NOT_PRESENT,
                         _handler->_datesInRequest);
  }
  void testCheckForDateConsistency_WasFirstDate_WithDate()
  {
    _handler->_datesInRequest = AncillaryPricingRequestHandler::DATE_IN_FRIST_SEGMENT;
    CPPUNIT_ASSERT_NO_THROW(_handler->checkForDateConsistency(std::string("2010-12-12")));
    CPPUNIT_ASSERT_EQUAL(AncillaryPricingRequestHandler::DATE_PRESENT, _handler->_datesInRequest);
  }
  void testCheckForDateConsistency_WasDate_NoDate()
  {
    _handler->_datesInRequest = AncillaryPricingRequestHandler::DATE_PRESENT;
    CPPUNIT_ASSERT_THROW(_handler->checkForDateConsistency(std::string("")),
                         ErrorResponseException);
  }
  void testCheckForDateConsistency_WasDate_WithDate()
  {
    _handler->_datesInRequest = AncillaryPricingRequestHandler::DATE_PRESENT;
    CPPUNIT_ASSERT_NO_THROW(_handler->checkForDateConsistency(std::string("2010-12-12")));
  }
  void testCheckForDateConsistency_WasNoDate_NoDate()
  {
    _handler->_datesInRequest = AncillaryPricingRequestHandler::DATE_NOT_PRESENT;
    CPPUNIT_ASSERT_NO_THROW(_handler->checkForDateConsistency(std::string("")));
  }
  void testCheckForDateConsistency_WasNoDate_WithDate()
  {
    _handler->_datesInRequest = AncillaryPricingRequestHandler::DATE_NOT_PRESENT;
    CPPUNIT_ASSERT_THROW(_handler->checkForDateConsistency(std::string("2010-12-12")),
                         ErrorResponseException);
  }
  void testCheckForTimeConsistency_DepTime_NoArrTime()
  {
    CPPUNIT_ASSERT_THROW(
        _handler->checkForTimeConsistency(std::string("10-12-12"), std::string("")),
        ErrorResponseException);
  }
  void testCheckForTimeConsistency_NoDepTime_ArrTime()
  {
    CPPUNIT_ASSERT_THROW(
        _handler->checkForTimeConsistency(std::string(""), std::string("10-12-12")),
        ErrorResponseException);
  }
  void testCheckForTimeConsistency_First_NoDate()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->checkForTimeConsistency(std::string(""), std::string("")));
    CPPUNIT_ASSERT_EQUAL(AncillaryPricingRequestHandler::DATE_NOT_PRESENT,
                         _handler->_timesInRequest);
  }
  void testCheckForTimeConsistency_First_WithDate()
  {
    CPPUNIT_ASSERT_NO_THROW(
        _handler->checkForTimeConsistency(std::string("10-12-12"), std::string("10-12-12")));
    CPPUNIT_ASSERT_EQUAL(AncillaryPricingRequestHandler::DATE_PRESENT, _handler->_timesInRequest);
  }
  void testCheckForTimeConsistency_WasDate_NoDate()
  {
    _handler->_timesInRequest = AncillaryPricingRequestHandler::DATE_PRESENT;
    CPPUNIT_ASSERT_THROW(_handler->checkForTimeConsistency(std::string(""), std::string("")),
                         ErrorResponseException);
  }
  void testCheckForTimeConsistency_WasDate_WithDate()
  {
    _handler->_timesInRequest = AncillaryPricingRequestHandler::DATE_PRESENT;
    CPPUNIT_ASSERT_NO_THROW(
        _handler->checkForTimeConsistency(std::string("10-12-12"), std::string("10-12-12")));
  }
  void testCheckForTimeConsistency_WasNoDate_NoDate()
  {
    _handler->_timesInRequest = AncillaryPricingRequestHandler::DATE_NOT_PRESENT;
    CPPUNIT_ASSERT_NO_THROW(_handler->checkForTimeConsistency(std::string(""), std::string("")));
  }
  void testCheckForTimeConsistency_WasNoDate_WithDate()
  {
    _handler->_timesInRequest = AncillaryPricingRequestHandler::DATE_NOT_PRESENT;
    CPPUNIT_ASSERT_THROW(
        _handler->checkForTimeConsistency(std::string("10-12-12"), std::string("10-12-12")),
        ErrorResponseException);
  }

  void testSetCabin_R()
  {
    createTestData();
    _handler->_currentTvlSeg = _memHandle.create<AirSeg>();
    CPPUNIT_ASSERT_NO_THROW(_handler->setCabin('R'));
    CPPUNIT_ASSERT(_handler->_currentTvlSeg->bookedCabin().isPremiumFirstClass());
  }
  void testSetCabin_F()
  {
    createTestData();
    _handler->_currentTvlSeg = _memHandle.create<AirSeg>();
    CPPUNIT_ASSERT_NO_THROW(_handler->setCabin('F'));
    CPPUNIT_ASSERT(_handler->_currentTvlSeg->bookedCabin().isFirstClass());
  }
  void testSetCabin_J()
  {
    createTestData();
    _handler->_currentTvlSeg = _memHandle.create<AirSeg>();
    CPPUNIT_ASSERT_NO_THROW(_handler->setCabin('J'));
    CPPUNIT_ASSERT(_handler->_currentTvlSeg->bookedCabin().isPremiumBusinessClass());
  }
  void testSetCabin_C()
  {
    createTestData();
    _handler->_currentTvlSeg = _memHandle.create<AirSeg>();
    CPPUNIT_ASSERT_NO_THROW(_handler->setCabin('C'));
    CPPUNIT_ASSERT(_handler->_currentTvlSeg->bookedCabin().isBusinessClass());
  }
  void testSetCabin_P()
  {
    createTestData();
    _handler->_currentTvlSeg = _memHandle.create<AirSeg>();
    CPPUNIT_ASSERT_NO_THROW(_handler->setCabin('P'));
    CPPUNIT_ASSERT(_handler->_currentTvlSeg->bookedCabin().isPremiumEconomyClass());
  }
  void testSetCabin_Y()
  {
    createTestData();
    _handler->_currentTvlSeg = _memHandle.create<AirSeg>();
    CPPUNIT_ASSERT_NO_THROW(_handler->setCabin('Y'));
    CPPUNIT_ASSERT(_handler->_currentTvlSeg->bookedCabin().isEconomyClass());
  }
  void testSetCabin_X()
  {
    createTestData();
    _handler->_currentTvlSeg = _memHandle.create<AirSeg>();
    CPPUNIT_ASSERT_THROW(_handler->setCabin('X'), ErrorResponseException);
  }
  void testSetCabin_R_WithCOS()
  {
    createTestData();
    _handler->_currentTvlSeg = _memHandle.create<AirSeg>();
    _handler->_currentTvlSeg->carrier() = "BA";
    _handler->_currentTvlSeg->setBookingCode("Y");
    CPPUNIT_ASSERT_NO_THROW(_handler->setCabin('R'));
    CPPUNIT_ASSERT(_handler->_currentTvlSeg->bookedCabin().isEconomyClass());
  }
  void testSetCabin_Blank_WithCOS()
  {
    createTestData();
    _handler->_currentTvlSeg = _memHandle.create<AirSeg>();
    _handler->_currentTvlSeg->carrier() = "BA";
    _handler->_currentTvlSeg->setBookingCode("Y");
    CPPUNIT_ASSERT_NO_THROW(_handler->setCabin(' '));
    CPPUNIT_ASSERT(_handler->_currentTvlSeg->bookedCabin().isEconomyClass());
  }

  void testSetTime_Empty()
  {
    DateTime dt(2000, 1, 1);
    _handler->setTime(dt, std::string(""));
    CPPUNIT_ASSERT_EQUAL(DateTime(2000, 1, 1, 23, 59, 59), dt);
  }
  void testSetTime_1028()
  {
    DateTime dt(2000, 1, 1);
    _handler->setTime(dt, std::string("1028"));
    CPPUNIT_ASSERT_EQUAL(DateTime(2000, 1, 1, 10, 28, 0), dt);
  }
  void testPssTime_Empty() { CPPUNIT_ASSERT_EQUAL(std::string(""), _handler->pssTime("")); }
  void testPssTime_0() { CPPUNIT_ASSERT_EQUAL(std::string("0"), _handler->pssTime("0")); }
  void testPssTime_2359() { CPPUNIT_ASSERT_EQUAL(std::string("1439"), _handler->pssTime("2359")); }
  void testSetSectors()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(
        _dataHandle, "<AN><AGI A10=\"VIE\" /><ITN><SGI><FLI A01=\"LO1\" A02=\"LO2\" B00=\"AA\" "
                     "/></SGI><SGI><FLI A01=\"LO3\" A02=\"LO4\" B00=\"AA\" /></SGI></ITN></AN>"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    DateTime dt = _handler->_curDate;
    CPPUNIT_ASSERT_EQUAL(size_t(3), trx->itin().front()->travelSeg().size());
    CPPUNIT_ASSERT_EQUAL(DateTime(dt.year(), dt.month(), dt.day(), 23, 59, 59),
                         trx->itin().front()->travelSeg()[0]->departureDT());
    CPPUNIT_ASSERT_EQUAL(DateTime(dt.year(), dt.month(), dt.day(), 23, 59, 59),
                         trx->itin().front()->travelSeg()[1]->departureDT());
    DateTime nextDt = dt.nextDay();
    CPPUNIT_ASSERT_EQUAL(DateTime(nextDt.year(), nextDt.month(), nextDt.day(), 23, 59, 59),
                         trx->itin().front()->travelSeg()[2]->departureDT());
  }
  void testCheckSideTrip_First()
  {
    CPPUNIT_ASSERT_THROW(_handler->parse(_dataHandle, "<AN><AGI A10=\"VIE\" /><ITN>\
    <SGI Q00=\"1\"><FLI A01=\"LO1\" A02=\"LO2\" B00=\"AA\" /><FBA Q6D=\"1\" Q6E=\"1\" /></SGI>\
    <SGI Q00=\"2\"><FLI A01=\"LO3\" A02=\"LO4\" B00=\"AA\" /><FBA Q6D=\"2\" /></SGI>\
    <SGI Q00=\"3\"><FLI A01=\"LO4\" A02=\"LO1\" B00=\"AA\" /><FBA Q6D=\"3\" /></SGI>\
    <FBI Q00=\"1\" B02=\"WS\" C50=\"130.00\" B50=\"DA10UN2\" S53=\"0\" />\
    <FBI Q00=\"2\" B02=\"WS\" C50=\"130.00\" B50=\"DA10UN2\" S53=\"0\" />\
    <FBI Q00=\"3\" B02=\"WS\" C50=\"130.00\" B50=\"DA10UN2\" S53=\"0\" /></ITN></AN>"),
                         ErrorResponseException);
  }
  void testCheckSideTrip_Last()
  {
    CPPUNIT_ASSERT_THROW(_handler->parse(_dataHandle, "<AN><AGI A10=\"VIE\" /><ITN>\
    <SGI Q00=\"1\"><FLI A01=\"LO1\" A02=\"LO2\" B00=\"AA\" /><FBA Q6D=\"1\" /></SGI>\
    <SGI Q00=\"2\"><FLI A01=\"LO3\" A02=\"LO4\" B00=\"AA\" /><FBA Q6D=\"2\" /></SGI>\
    <SGI Q00=\"3\"><FLI A01=\"LO4\" A02=\"LO1\" B00=\"AA\" /><FBA Q6D=\"3\" Q6E=\"1\" /></SGI>\
    <FBI Q00=\"1\" B02=\"WS\" C50=\"130.00\" B50=\"DA10UN2\" S53=\"0\" />\
    <FBI Q00=\"2\" B02=\"WS\" C50=\"130.00\" B50=\"DA10UN2\" S53=\"0\" />\
    <FBI Q00=\"3\" B02=\"WS\" C50=\"130.00\" B50=\"DA10UN2\" S53=\"0\" /></ITN></AN>"),
                         ErrorResponseException);
  }
  void testCheckSideTrip_Second()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle, "<AN><AGI A10=\"VIE\" /><ITN>\
    <SGI Q00=\"1\"><FLI A01=\"LO1\" A02=\"LO2\" B00=\"AA\" /><FBA Q6D=\"1\" /></SGI>\
    <SGI Q00=\"2\"><FLI A01=\"LO3\" A02=\"LO4\" B00=\"AA\" /><FBA Q6D=\"2\" Q6E=\"1\" /></SGI>\
    <SGI Q00=\"3\"><FLI A01=\"LO4\" A02=\"LO1\" B00=\"AA\" /><FBA Q6D=\"3\" /></SGI>\
    <FBI Q00=\"1\" B02=\"WS\" C50=\"130.00\" B50=\"DA10UN2\" S53=\"0\" />\
    <FBI Q00=\"2\" B02=\"WS\" C50=\"130.00\" B50=\"DA10UN2\" S53=\"0\" />\
    <FBI Q00=\"3\" B02=\"WS\" C50=\"130.00\" B50=\"DA10UN2\" S53=\"0\" /></ITN></AN>"));
  }
  void testPXI_ADT_ADT()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(
        _dataHandle,
        "<AN><AGI A10=\"VIE\" /><PXI B70=\"ADT\" /><PXI B70=\"ADT\" Q0U=\"2\" /></AN>"));
    AncRequest* req =
        dynamic_cast<AncRequest*>(dynamic_cast<AncillaryPricingTrx*>(_trx)->getRequest());
    CPPUNIT_ASSERT(req != 0);
    CPPUNIT_ASSERT_EQUAL(size_t(1), req->paxTypesPerItin()[_handler->_itin].size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), req->paxTypesPerItin()[_handler->_itin].front()->number());
  }
  void testPXI_ADT_BLANK()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(
        _dataHandle, "<AN><AGI A10=\"VIE\" /><PXI B70=\"ADT\" /><PXI B70=\"\" /></AN>"));
    AncRequest* req =
        dynamic_cast<AncRequest*>(dynamic_cast<AncillaryPricingTrx*>(_trx)->getRequest());
    CPPUNIT_ASSERT(req != 0);
    CPPUNIT_ASSERT_EQUAL(size_t(1), req->paxTypesPerItin()[_handler->_itin].size());
  }
  void testPXI_BLANK_ADT()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(
        _dataHandle, "<AN><AGI A10=\"VIE\" /><PXI B70=\"\" /><PXI B70=\"ADT\" /></AN>"));
    AncRequest* req =
        dynamic_cast<AncRequest*>(dynamic_cast<AncillaryPricingTrx*>(_trx)->getRequest());
    CPPUNIT_ASSERT(req != 0);
    CPPUNIT_ASSERT_EQUAL(size_t(1), req->paxTypesPerItin()[_handler->_itin].size());
  }
  void testPXI_C06_C07()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(
        _dataHandle, "<AN><AGI A10=\"VIE\" /><PXI B70=\"C06\" /><PXI B70=\"C07\" /></AN>"));
    AncRequest* req =
        dynamic_cast<AncRequest*>(dynamic_cast<AncillaryPricingTrx*>(_trx)->getRequest());
    CPPUNIT_ASSERT(req != 0);
    CPPUNIT_ASSERT_EQUAL(size_t(2), req->paxTypesPerItin()[_handler->_itin].size());
  }
  void testPXI_C00()
  {
    CPPUNIT_ASSERT_THROW(_handler->parse(_dataHandle, "<PXI B70=\"C00\" />"),
                         ErrorResponseException);
  }
  void testPXI_Y01()
  {
    CPPUNIT_ASSERT_THROW(_handler->parse(_dataHandle, "<PXI B70=\"Y01\" />"),
                         ErrorResponseException);
  }
  /*
  void testCheckCurrency_BLANK()
  {
    _handler->createBasicInformation(_dataHandle, "");
    CPPUNIT_ASSERT_NO_THROW(_handler->checkCurrency(""));
  }
  void testCheckCurrency_CurrencyExists()
  {
    _handler->createBasicInformation(_dataHandle, "");
    CPPUNIT_ASSERT_NO_THROW(_handler->checkCurrency("GBP"));
  }
  void testCheckCurrency_CurrencyDontExists()
  {
    _handler->createBasicInformation(_dataHandle, "");
    CPPUNIT_ASSERT_THROW(_handler->checkCurrency("XXX"), ErrorResponseException);
  }
  */
  void testParsePNMWhenHasDataForLiberty()
  {
    _handler->_currentTvlSeg = _memHandle.create<AirSeg>();
    CPPUNIT_ASSERT_NO_THROW(
        _handler->parse(_dataHandle, "<AN><AGI A10=\"VIE\" /><BIL S0R=\"PLIB\" A70=\"WPAE*\"/><PXI "
                                     "B70=\"ADT\" Q0U=\"1\"><PNM S0L=\"1.2\" Q86=\"1002003004005\" "
                                     "Q87=\"1234567890\"/></PXI></AN>"));
    AncRequest* req =
        dynamic_cast<AncRequest*>(dynamic_cast<AncillaryPricingTrx*>(_trx)->getRequest());
    CPPUNIT_ASSERT(req != 0);
    CPPUNIT_ASSERT_EQUAL(size_t(1), req->paxTypesPerItin()[_handler->_itin].size());

    PaxType* paxType = req->paxTypesPerItin()[_handler->_itin].front();
    CPPUNIT_ASSERT_EQUAL(PaxTypeCode("ADT"), paxType->paxType());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), paxType->number());
    CPPUNIT_ASSERT_EQUAL(size_t(1), paxType->psgTktInfo().size());
    CPPUNIT_ASSERT_EQUAL(PsgNameNumber("1.2"), paxType->psgTktInfo().front()->psgNameNumber());
    CPPUNIT_ASSERT_EQUAL(TktNumber("1002003004005"), paxType->psgTktInfo().front()->tktRefNumber());
    CPPUNIT_ASSERT_EQUAL(TktNumber("1234567890"), paxType->psgTktInfo().front()->tktNumber());
    CPPUNIT_ASSERT_EQUAL(_handler->_reqType, AncillaryPricingRequestHandler::PostTkt);
  }

  void testParsePNMWhenHasDataForPSS()
  {
    _handler->_currentTvlSeg = _memHandle.create<AirSeg>();
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(
        _dataHandle, "<AN><AGI A10=\"VIE\" /><BIL S0R=\"PPSS\" A70=\"WPAE*\"/><PXI B70=\"ADT\" "
                     "Q0U=\"1\"><PNM S0L=\"1.2\" Q86=\"1002003004005\" Q87=\"1\" /></PXI></AN>"));
    AncRequest* req =
        dynamic_cast<AncRequest*>(dynamic_cast<AncillaryPricingTrx*>(_trx)->getRequest());
    CPPUNIT_ASSERT(req != 0);
    CPPUNIT_ASSERT_EQUAL(size_t(1), req->paxTypesPerItin()[_handler->_itin].size());

    PaxType* paxType = req->paxTypesPerItin()[_handler->_itin].front();
    CPPUNIT_ASSERT_EQUAL(PaxTypeCode("ADT"), paxType->paxType());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), paxType->number());
    CPPUNIT_ASSERT_EQUAL(size_t(1), paxType->psgTktInfo().size());
    CPPUNIT_ASSERT_EQUAL(PsgNameNumber("1.2"), paxType->psgTktInfo().front()->psgNameNumber());
    CPPUNIT_ASSERT_EQUAL(TktNumber("1002003004005"), paxType->psgTktInfo().front()->tktRefNumber());
    CPPUNIT_ASSERT_EQUAL(_handler->_reqType, AncillaryPricingRequestHandler::PostTkt);
  }

  void testParsePNMWhenHasDataForSabreWebService()
  {
    _handler->_currentTvlSeg = _memHandle.create<AirSeg>();
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(
        _dataHandle, "<AN><AGI A10=\"VIE\" /><BIL S0R=\"PSWS\" A70=\"WPAE*\"/><PXI B70=\"ADT\" "
                     "Q0U=\"1\"><PNM S0L=\"1.2\" Q86=\"1002003004005\" Q87=\"2\" /></PXI></AN>"));
    AncRequest* req =
        dynamic_cast<AncRequest*>(dynamic_cast<AncillaryPricingTrx*>(_trx)->getRequest());
    CPPUNIT_ASSERT(req != 0);
    CPPUNIT_ASSERT_EQUAL(size_t(1), req->paxTypesPerItin()[_handler->_itin].size());

    PaxType* paxType = req->paxTypesPerItin()[_handler->_itin].front();
    CPPUNIT_ASSERT_EQUAL(PaxTypeCode("ADT"), paxType->paxType());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), paxType->number());
    CPPUNIT_ASSERT_EQUAL(size_t(1), paxType->psgTktInfo().size());
    CPPUNIT_ASSERT_EQUAL(PsgNameNumber("1.2"), paxType->psgTktInfo().front()->psgNameNumber());
    CPPUNIT_ASSERT_EQUAL(TktNumber("1002003004005"), paxType->psgTktInfo().front()->tktRefNumber());
    CPPUNIT_ASSERT_EQUAL(_handler->_reqType, AncillaryPricingRequestHandler::PostTkt);
  }

  void testParsePNMWhenEmptyPsgNameNoForLiberty()
  {
    _handler->_currentTvlSeg = _memHandle.create<AirSeg>();
    _handler->_reqType = AncillaryPricingRequestHandler::PostTkt;
    CPPUNIT_ASSERT_THROW(_handler->parse(_dataHandle,
                                         "<AN><AGI A10=\"VIE\" />"
                                         "<BIL S0R=\"PLIB\"/>"
                                         "<PXI B70=\"ADT\" Q0U=\"1\">"
                                         "<PNM Q86=\"1002003004005\" Q87=\"2\" />"
                                         "</PXI></AN>"),
                         ErrorResponseException);
  }

  void testParsePNMWhen2xADTForLibertyAndAccumulatedNumberOfPsg()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle,
                                            "<AN>"
                                            "<AGI A10=\"VIE\" />"
                                            "<BIL S0R=\"PLIB\" A70=\"WPAE*\"/>"
                                            "<PXI B70=\"ADT\">"
                                            "<PNM S0L=\"1.2\" Q86=\"1002003004005\" Q87=\"1\"/>"
                                            "</PXI>"
                                            "<PXI B70=\"ADT\" Q0U=\"2\">"
                                            "<PNM S0L=\"1.3\" Q86=\"1002003004006\" Q87=\"2\"/>"
                                            "</PXI>"
                                            "</AN>"));
    AncRequest* req =
        dynamic_cast<AncRequest*>(dynamic_cast<AncillaryPricingTrx*>(_trx)->getRequest());
    CPPUNIT_ASSERT(req != 0);
    CPPUNIT_ASSERT_EQUAL(_handler->_reqType, AncillaryPricingRequestHandler::PostTkt);
    CPPUNIT_ASSERT_EQUAL(size_t(1), req->paxTypesPerItin()[_handler->_itin].size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(3), req->paxTypesPerItin()[_handler->_itin].front()->number());
    PaxType* paxType = req->paxTypesPerItin()[_handler->_itin].front();
    CPPUNIT_ASSERT_EQUAL(size_t(2), paxType->psgTktInfo().size());
    CPPUNIT_ASSERT_EQUAL(PsgNameNumber("1.2"), paxType->psgTktInfo().front()->psgNameNumber());
    CPPUNIT_ASSERT_EQUAL(TktNumber("1002003004005"), paxType->psgTktInfo().front()->tktRefNumber());
    CPPUNIT_ASSERT_EQUAL(PsgNameNumber("1.3"), paxType->psgTktInfo().back()->psgNameNumber());
    CPPUNIT_ASSERT_EQUAL(TktNumber("1002003004006"), paxType->psgTktInfo().back()->tktRefNumber());
  }
  void createTestData()
  {
    _handler->_pricingTrx = _memHandle.create<AncillaryPricingTrx>();
    AncRequest* req = _memHandle.create<AncRequest>();
    _handler->_pricingTrx->setRequest(req);
    _handler->_request = req;
    _handlerM->_pricingTrx = _memHandle.create<AncillaryPricingTrx>();
    _handlerM->_pricingTrx->setRequest(req);
    _handlerM->_request = req;
    req->ancRequestType() = AncRequest::WPAERequest;
    _handler->_itin = _memHandle.create<Itin>();
    AirSeg* as = _memHandle.create<AirSeg>();
    as->segmentOrder() = 1;
    _handler->_itin->travelSeg().push_back(as);
    as = _memHandle.create<AirSeg>();
    as->segmentOrder() = 2;
    _handler->_itin->travelSeg().push_back(as);
    as = _memHandle.create<AirSeg>();
    as->segmentOrder() = 3;
    _handler->_itin->travelSeg().push_back(as);

    AncRequest::AncFareBreakAssociation* fba =
        _memHandle.create<AncRequest::AncFareBreakAssociation>();
    fba->segmentID() = 1;
    fba->fareComponentID() = 1;
    req->fareBreakAssociationPerItin()[_handler->_itin].push_back(fba);
    fba = _memHandle.create<AncRequest::AncFareBreakAssociation>();
    fba->segmentID() = 2;
    fba->fareComponentID() = 1;
    req->fareBreakAssociationPerItin()[_handler->_itin].push_back(fba);
    fba = _memHandle.create<AncRequest::AncFareBreakAssociation>();
    fba->segmentID() = 3;
    fba->fareComponentID() = 2;
    req->fareBreakAssociationPerItin()[_handler->_itin].push_back(fba);

    AncRequest::AncFareBreakInfo* fbi = _memHandle.create<AncRequest::AncFareBreakInfo>();
    fbi->fareComponentID() = 1;
    req->fareBreakPerItin()[_handler->_itin].push_back(fbi);
    fbi = _memHandle.create<AncRequest::AncFareBreakInfo>();
    fbi->fareComponentID() = 2;
    req->fareBreakPerItin()[_handler->_itin].push_back(fbi);
  }
  void testCheckRequiredDataForWPAE_Pass()
  {
    createTestData();
    CPPUNIT_ASSERT_NO_THROW(_handler->checkFBAandFBIdata());
  }
  void testCheckRequiredDataForWPAE_NoMatchFBA()
  {
    createTestData();
    static_cast<AncRequest*>(_handler->_pricingTrx->getRequest())
        ->fareBreakAssociationPerItin()[_handler->_itin]
        .back()
        ->segmentID() = 2;
    CPPUNIT_ASSERT_THROW(_handler->checkFBAandFBIdata(), tse::ErrorResponseException);
  }
  void testCheckRequiredDataForWPAE_NoMatchFBI()
  {
    createTestData();
    static_cast<AncRequest*>(_handler->_pricingTrx->getRequest())
        ->fareBreakPerItin()[_handler->_itin]
        .back()
        ->fareComponentID() = 1;

    CPPUNIT_ASSERT_THROW(_handler->checkFBAandFBIdata(), tse::ErrorResponseException);
  }
  void testCheckFLIDataForWPAE_Pass()
  {
    _handlerM->_reqType = AncillaryPricingRequestHandler::WPAE;
    _handlerM->_currentTvlSeg = _memHandle.create<AirSeg>();
    CPPUNIT_ASSERT_NO_THROW(
        _handlerM->parse(_dataHandle, "<FLI A01=\"LHR\" A02=\"BOS\" D01=\"2010-12-01\" D31=\"420\" "
                                     "D02=\"2010-12-02\" D32=\"515\" B00=\"BA\" B01=\"AA\" "
                                     "B30=\"Y\" Q0B=\"412\" S95=\"FLT\" N0E=\"Y\" BB0=\"QF\" />"));
  }
  void testCheckFLIDataForWPAE_FailB01()
  {
    _handler->_reqType = AncillaryPricingRequestHandler::WPAE;
    _handler->_currentTvlSeg = _memHandle.create<AirSeg>();
    CPPUNIT_ASSERT_THROW(
       _handler->parse(_dataHandle, "<FLI A01=\"LHR\" A02=\"BOS\" D01=\"2010-12-01\" D31=\"420\" "
                                     "D02=\"2010-12-02\" D32=\"515\" B00=\"BA\" B30=\"Y\" "
                                     "Q0B=\"412\" S95=\"FLT\" N0E=\"Y\" BB0=\"OK\"/>"),
        tse::ErrorResponseException);
  }
  void testCheckFLIDataForWPAE_OpenSeg()
  {
    // BB0="  ", no S95 , but pass
    _handlerM->_reqType = AncillaryPricingRequestHandler::WPAE;
    _handlerM->_currentTvlSeg = _memHandle.create<AirSeg>();
    CPPUNIT_ASSERT_NO_THROW(
        _handlerM->parse(_dataHandle, "<FLI A01=\"LHR\" A02=\"BOS\" D01=\"2010-12-01\" D31=\"420\" "
                                     "D02=\"2010-12-02\" D32=\"515\" B00=\"BA\" B01=\"AA\" "
                                     "B30=\"Y\" Q0B=\"412\" N0E=\"Y\" BB0=\"  \" />"));
  }
  void testCheckFBADataForWPAE_Pass()
  {
    _handler->_reqType = AncillaryPricingRequestHandler::WPAE;
    _handler->_currentTvlSeg = _memHandle.create<AirSeg>();
    _handler->_itin = _memHandle.create<Itin>();
    CPPUNIT_ASSERT_NO_THROW(
        _handler->parse(_dataHandle, "<FBA Q6D=\"1\" Q6E=\"2\" S07=\"T\" S08=\"F\" />"));
  }
  void testCheckFBADataForWPAE_FailQ6D()
  {
    _handler->_reqType = AncillaryPricingRequestHandler::WPAE;
    _handler->_currentTvlSeg = _memHandle.create<AirSeg>();
    _handler->_itin = _memHandle.create<Itin>();
    CPPUNIT_ASSERT_THROW(_handler->parse(_dataHandle, "<FBA Q6E=\"2\" S07=\"T\" S08=\"F\" />"),
                         tse::ErrorResponseException);
  }
  void testCheckFBIDataForWPAE_Pass()
  {
    _handler->_currentTvlSeg = _memHandle.create<AirSeg>();
    _handler->_reqType = AncillaryPricingRequestHandler::WPAE;
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(
        _dataHandle, "<FBI Q00=\"1\" B02=\"BA\" C50=\"123.45\" B50=\"Y3\" S53=\"XEX\" S37=\"ATP\" "
                     "Q3W=\"123\" S90=\"0001\" Q3V=\"25\" P1Z=\"T\"  FTY=\"00\" />"));
  }
  void testCheckFBIDataForWPAE_FailC50()
  {
    _handler->_reqType = AncillaryPricingRequestHandler::WPAE;
    CPPUNIT_ASSERT_THROW(_handler->parse(_dataHandle, "<FBI Q00=\"1\" B02=\"BA\" B50=\"Y3\" "
                                                      "S53=\"XEX\" Q3W=\"123\" S90=\"0001\" "
                                                      "Q3V=\"25\" P1Z=\"T\"  FTY=\"00\" />"),
                         tse::ErrorResponseException);
  }
  /*
  void createTestCheckIfR7RequestData(AncRequest::AncRequestType rt, std::string sor, bool val)
  {
    _handler->createBasicInformation(_dataHandle, "");
    _handler->_sourceOfRequest = sor;
    static_cast<AncRequest*>(_handler->_pricingTrx->getRequest())->ancRequestType() = rt;
    if(val)
      _handler->_reqType = AncillaryPricingRequestHandler::PostTkt;
  }

  void testCheckIfR7Request_PostTkt_PSS()
  {
    createTestCheckIfR7RequestData(AncRequest::PostTktRequest, "PPSS", true);
    CPPUNIT_ASSERT_EQUAL(_handler->_reqType, AncillaryPricingRequestHandler::PostTkt);
  }
  void testCheckIfR7Request_M70_PSS()
  {
    createTestCheckIfR7RequestData(AncRequest::M70Request, "PPSS", false);
    CPPUNIT_ASSERT_EQUAL(_handler->_reqType, AncillaryPricingRequestHandler::M70);
  }
  void testCheckIfR7Request_PostTkt_Unknown()
  {
    createTestCheckIfR7RequestData(AncRequest::PostTktRequest, "UNKN", false);
    CPPUNIT_ASSERT_EQUAL(_handler->_reqType, AncillaryPricingRequestHandler::M70);
  }
  */
  void createDataForParseOriginamPricingCommand(std::string cmd)
  {
    _handler->_pricingTrx = _memHandle.create<AncillaryPricingTrx>();
    _handler->_pricingTrx->setRequest(_ancReq = _memHandle.create<AncRequest>());
    _handler->_request = _ancReq;
    _handler->_pricingTrx->setOptions(_memHandle.create<PricingOptions>());
    _handler->_itin = _memHandle.create<Itin>();
    _handler->_pricingTrx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    _handler->_pricingTrx->getRequest()->ticketingAgent()->agentCity() = "VIE";
    _handler->_itin->travelSeg().push_back(_memHandle.create<AirSeg>());
    _handler->_itin->travelSeg().push_back(_memHandle.create<AirSeg>());
    _handler->_itin->travelSeg().push_back(_memHandle.create<AirSeg>());
    _handler->_itin->travelSeg()[0]->segmentOrder() = 1;
    _handler->_itin->travelSeg()[1]->segmentOrder() = 2;
    _handler->_itin->travelSeg()[2]->segmentOrder() = 3;
    _handler->_itin->travelSeg()[0]->pnrSegment() = 1;
    _handler->_itin->travelSeg()[1]->pnrSegment() = 2;
    _handler->_itin->travelSeg()[2]->pnrSegment() = 3;
    _handler->_originalPricingCommand = cmd;
  }
  void testParseOriginalPricingCommand_AccountCode()
  {
    createDataForParseOriginamPricingCommand("WPAC*ABC1234$AC*XY1234567");
    CPPUNIT_ASSERT_NO_THROW(_handler->parseOriginalPricingCommandWPAE());
    CPPUNIT_ASSERT_EQUAL(size_t(2), _ancReq->accountCodeIdPerItin()[_handler->_itin].size());
    CPPUNIT_ASSERT_EQUAL(std::string("ABC1234"),
                         _ancReq->accountCodeIdPerItin()[_handler->_itin][0]);
    CPPUNIT_ASSERT_EQUAL(std::string("XY1234567"),
                         _ancReq->accountCodeIdPerItin()[_handler->_itin][1]);
  }
  void testParseOriginalPricingCommand_CorpId()
  {
    createDataForParseOriginamPricingCommand("WPICOR01$ICOR02");
    CPPUNIT_ASSERT_NO_THROW(_handler->parseOriginalPricingCommandWPAE());
    CPPUNIT_ASSERT_EQUAL(size_t(1), _ancReq->corpIdPerItin()[_handler->_itin].size());
    CPPUNIT_ASSERT_EQUAL(std::string("COR01"), _ancReq->corpIdPerItin()[_handler->_itin].front());
    CPPUNIT_ASSERT_EQUAL(size_t(1), _ancReq->invalidCorpIdPerItin()[_handler->_itin].size());
  }
  void testParseOriginalPricingCommand_BuyingDate()
  {
    createDataForParseOriginamPricingCommand("WPB19DEC25");
    CPPUNIT_ASSERT_NO_THROW(_handler->parseOriginalPricingCommandWPAE());
    CPPUNIT_ASSERT_EQUAL(DateTime(2025, 12, 19), _ancReq->ticketingDT());
  }
  void testParseOriginalPricingCommand_TktDesignator_SkipWhenExist()
  {
    createDataForParseOriginamPricingCommand("WPQY26/TKDESIG1");
    _ancReq->tktDesignatorPerItin()[_handler->_itin].insert(
        std::pair<uint16_t, TktDesignator>(1, "ABCD"));
    CPPUNIT_ASSERT_NO_THROW(_handler->parseOriginalPricingCommandWPAE());
    CPPUNIT_ASSERT_EQUAL(size_t(1), _ancReq->tktDesignatorPerItin()[_handler->_itin].size());
    CPPUNIT_ASSERT_EQUAL(TktDesignator("ABCD"),
                         _ancReq->tktDesignatorPerItin()[_handler->_itin][1]);
  }
  void testParseOriginalPricingCommand_TktDesignator()
  {
    createDataForParseOriginamPricingCommand("WPQY26/TKTDESIG1");
    CPPUNIT_ASSERT_NO_THROW(_handler->parseOriginalPricingCommandWPAE());
    CPPUNIT_ASSERT_EQUAL(size_t(3), _ancReq->tktDesignatorPerItin()[_handler->_itin].size());
    CPPUNIT_ASSERT_EQUAL(TktDesignator("Y26/TKTDESIG1"),
                         _ancReq->tktDesignatorPerItin()[_handler->_itin][1]);
    CPPUNIT_ASSERT_EQUAL(TktDesignator("Y26/TKTDESIG1"),
                         _ancReq->tktDesignatorPerItin()[_handler->_itin][2]);
    CPPUNIT_ASSERT_EQUAL(TktDesignator("Y26/TKTDESIG1"),
                         _ancReq->tktDesignatorPerItin()[_handler->_itin][3]);
  }
  void testParseOriginalPricingCommand_SegSelTktDesignator_SkipWhenExist()
  {
    createDataForParseOriginamPricingCommand("WPS1*QY26/TKDESIG1");
    _ancReq->tktDesignatorPerItin()[_handler->_itin].insert(
        std::pair<uint16_t, TktDesignator>(1, "ABCD"));
    CPPUNIT_ASSERT_NO_THROW(_handler->parseOriginalPricingCommandWPAE());
    CPPUNIT_ASSERT_EQUAL(size_t(1), _ancReq->tktDesignatorPerItin()[_handler->_itin].size());
    CPPUNIT_ASSERT_EQUAL(TktDesignator("ABCD"),
                         _ancReq->tktDesignatorPerItin()[_handler->_itin][1]);
  }
  void testParseOriginalPricingCommand_SegSelTktDesignator()
  {
    createDataForParseOriginamPricingCommand("WPS1*QY$S2/3*QYR/T1");
    CPPUNIT_ASSERT_NO_THROW(_handler->parseOriginalPricingCommandWPAE());
    CPPUNIT_ASSERT_EQUAL(size_t(3), _ancReq->tktDesignatorPerItin()[_handler->_itin].size());
    CPPUNIT_ASSERT_EQUAL(TktDesignator("Y"), _ancReq->tktDesignatorPerItin()[_handler->_itin][1]);
    CPPUNIT_ASSERT_EQUAL(TktDesignator("YR/T1"),
                         _ancReq->tktDesignatorPerItin()[_handler->_itin][2]);
    CPPUNIT_ASSERT_EQUAL(TktDesignator("YR/T1"),
                         _ancReq->tktDesignatorPerItin()[_handler->_itin][3]);
  }
  void testParseOriginalPricingCommand_RangeSegSelTktDesignator_SkipWhenExist()
  {
    createDataForParseOriginamPricingCommand("WPS1-3*QY26/TKDESIG1");
    _ancReq->tktDesignatorPerItin()[_handler->_itin].insert(
        std::pair<uint16_t, TktDesignator>(1, "ABCD"));
    CPPUNIT_ASSERT_NO_THROW(_handler->parseOriginalPricingCommandWPAE());
    CPPUNIT_ASSERT_EQUAL(size_t(1), _ancReq->tktDesignatorPerItin()[_handler->_itin].size());
    CPPUNIT_ASSERT_EQUAL(TktDesignator("ABCD"),
                         _ancReq->tktDesignatorPerItin()[_handler->_itin][1]);
  }
  void testParseOriginalPricingCommand_RangeSegSelTktDesignator()
  {
    createDataForParseOriginamPricingCommand("WPS1-3*QQOW/TKT");
    CPPUNIT_ASSERT_NO_THROW(_handler->parseOriginalPricingCommandWPAE());
    CPPUNIT_ASSERT_EQUAL(size_t(3), _ancReq->tktDesignatorPerItin()[_handler->_itin].size());
    CPPUNIT_ASSERT_EQUAL(TktDesignator("QOW/TKT"),
                         _ancReq->tktDesignatorPerItin()[_handler->_itin][1]);
    CPPUNIT_ASSERT_EQUAL(TktDesignator("QOW/TKT"),
                         _ancReq->tktDesignatorPerItin()[_handler->_itin][2]);
    CPPUNIT_ASSERT_EQUAL(TktDesignator("QOW/TKT"),
                         _ancReq->tktDesignatorPerItin()[_handler->_itin][3]);
  }
  void testParseOriginalPricingCommand_WPNCS()
  {
    createDataForParseOriginamPricingCommand("WPNCS");
    CPPUNIT_ASSERT_NO_THROW(_handler->parseOriginalPricingCommandWPAE());
    CPPUNIT_ASSERT_EQUAL('T', _handler->_pricingTrx->getRequest()->lowFareNoRebook());
  }
  void testParseOriginalPricingCommand_WPNCB()
  {
    createDataForParseOriginamPricingCommand("WPNCB");
    CPPUNIT_ASSERT_NO_THROW(_handler->parseOriginalPricingCommandWPAE());
    CPPUNIT_ASSERT_EQUAL(char(0), _handler->_pricingTrx->getRequest()->lowFareNoRebook());
  }
  void testParseOriginalPricingCommand_WPNC()
  {
    createDataForParseOriginamPricingCommand("WPNC");
    CPPUNIT_ASSERT_NO_THROW(_handler->parseOriginalPricingCommandWPAE());
    CPPUNIT_ASSERT_EQUAL('T', _handler->_pricingTrx->getRequest()->lowFareNoRebook());
  }
  void testParseOriginalPricingCommand_SalePointOverride()
  {
    createDataForParseOriginamPricingCommand("WPSLON");
    CPPUNIT_ASSERT_NO_THROW(_handler->parseOriginalPricingCommandWPAE());
    CPPUNIT_ASSERT_EQUAL(LocCode("LON"), _handler->_pricingTrx->getRequest()->salePointOverride());
  }
  void testParseOriginalPricingCommand_TicketPoinOverride()
  {
    createDataForParseOriginamPricingCommand("WPTLON");
    CPPUNIT_ASSERT_NO_THROW(_handler->parseOriginalPricingCommandWPAE());
    CPPUNIT_ASSERT_EQUAL(LocCode("LON"),
                         _handler->_pricingTrx->getRequest()->ticketPointOverride());
  }
  void testParseOriginalPricingCommand_CurrencyOverride()
  {
    createDataForParseOriginamPricingCommand("WPMGBP");
    CPPUNIT_ASSERT_NO_THROW(_handler->parseOriginalPricingCommandWPAE());
    CPPUNIT_ASSERT_EQUAL(CurrencyCode("GBP"),
                         _handler->_pricingTrx->getOptions()->currencyOverride());
  }
  void testParseOriginalPricingCommand_FFS()
  {
    createDataForParseOriginamPricingCommand("FFS-AZ2/UA3/DL4");
    CPPUNIT_ASSERT_NO_THROW(_handler->parseOriginalPricingCommandWPAE());
    CPPUNIT_ASSERT_EQUAL(size_t(3), _handler->_ffData.size());
  }
  void testCleanOriginalPricingCommandPostTkt()
  {
    _handler->_originalPricingCommand = "W$FCA$AAA$PCMP$S1-4$N7.1$AC*AC01$DSA$ICOR01$VV$QDUPA";
    CPPUNIT_ASSERT_EQUAL(std::string("S1-4$AC*AC01$ICOR01$QDUPA$"),
                         _handler->cleanOriginalPricingCommandPostTkt());
  }
  void testParseSchemaVersion_Empty()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle,
                                            "<AncillaryPricingRequest >"
                                            "<AGI A10=\"LHR\" />"
                                            "<ITN Q00=\"1\">"
                                            "<SGI Q00=\"1\">"
                                            "<FLI A01=\"LHR\" A02=\"BOS\" D01=\"2025-12-30\" "
                                            "D31=\"420\" D02=\"2025-12-31\" D32=\"515\" B00=\"BA\" "
                                            "B01=\"AA\" B30=\"Y\" Q0B=\"412\" S95=\"FLT\" "
                                            "N0E=\"Y\" BB0=\"QF\" C7A=\"12\" C7B=\"T\" />"
                                            "</SGI>"
                                            "</ITN>"
                                            "</AncillaryPricingRequest>"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL((uint16_t)1,
                         static_cast<AncRequest*>(trx->getRequest())->majorSchemaVersion());
  }
  void testParseSchemaVersion_Valid()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle,
                                            "<AncillaryPricingRequest Version=\"2.0.0\">"
                                            "<AGI A10=\"LHR\" />"
                                            "<ITN Q00=\"1\">"
                                            "<SGI Q00=\"1\">"
                                            "<FLI A01=\"LHR\" A02=\"BOS\" D01=\"2025-12-30\" "
                                            "D31=\"420\" D02=\"2025-12-31\" D32=\"515\" B00=\"BA\" "
                                            "B01=\"AA\" B30=\"Y\" Q0B=\"412\" S95=\"FLT\" "
                                            "N0E=\"Y\" BB0=\"QF\" C7A=\"12\" C7B=\"T\" />"
                                            "</SGI>"
                                            "</ITN>"
                                            "</AncillaryPricingRequest>"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL((uint16_t)2,
                         static_cast<AncRequest*>(trx->getRequest())->majorSchemaVersion());
  }
  void testParseSchemaVersion_Invalid()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle,
                                            "<AncillaryPricingRequest Version=\"wrong\">"
                                            "<AGI A10=\"LHR\" />"
                                            "<ITN Q00=\"1\">"
                                            "<SGI Q00=\"1\">"
                                            "<FLI A01=\"LHR\" A02=\"BOS\" D01=\"2025-12-30\" "
                                            "D31=\"420\" D02=\"2025-12-31\" D32=\"515\" B00=\"BA\" "
                                            "B01=\"AA\" B30=\"Y\" Q0B=\"412\" S95=\"FLT\" "
                                            "N0E=\"Y\" BB0=\"QF\" C7A=\"12\" C7B=\"T\" />"
                                            "</SGI>"
                                            "</ITN>"
                                            "</AncillaryPricingRequest>"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL((uint16_t)1,
                         static_cast<AncRequest*>(trx->getRequest())->majorSchemaVersion());
  }
  void test_parseAST()
  {
    _handler->_currentRequestedOcFeeGroup.groupCode() = "BG";
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle, "<AST AST=\"A\" />"));

    CPPUNIT_ASSERT(_handler->_currentRequestedOcFeeGroup.isAncillaryServiceType('A'));
    CPPUNIT_ASSERT(!_handler->_currentRequestedOcFeeGroup.isAncillaryServiceType('C'));
  }

  std::ofstream& getFile()
  {
    std::string fileName("/login/sg216830/logs");
    static std::ofstream file(fileName.c_str());
    return file;
  }

  void test_parseRFG_AST()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle,
                                            "<PRO AF0=\"MIA\"><RFG S01=\"PT\" ><AST AST=\"C\"/>"
                                            "                                                 "
                                            "</RFG><RFG S01=\"BG\"><AST AST=\"A\" /></RFG></PRO>"));
    CPPUNIT_ASSERT(
        !_handler->_pricingTrx->getOptions()->serviceGroupsVec()[0].isAncillaryServiceType('A'));
    CPPUNIT_ASSERT(
        _handler->_pricingTrx->getOptions()->serviceGroupsVec()[0].isAncillaryServiceType('C'));
    CPPUNIT_ASSERT(
        _handler->_pricingTrx->getOptions()->serviceGroupsVec()[1].isAncillaryServiceType('A'));
    CPPUNIT_ASSERT(
        !_handler->_pricingTrx->getOptions()->serviceGroupsVec()[1].isAncillaryServiceType('C'));
  }

  void testCheckFlights_Non_Air()
  {
    Itin itin1;
    SurfaceSeg seg1;
    itin1.travelSeg().push_back(&seg1);

    Itin itin2;
    SurfaceSeg seg2;
    itin2.travelSeg().push_back(&seg2);

    CPPUNIT_ASSERT_NO_THROW(_handler->checkFlights(&itin1, &itin2));
  }

  void testCheckFlights_Mixed()
  {
    Itin itin1;
    SurfaceSeg seg1;
    itin1.travelSeg().push_back(&seg1);

    Itin itin2;
    AirSeg seg2;
    itin2.travelSeg().push_back(&seg2);

    CPPUNIT_ASSERT_THROW(_handler->checkFlights(&itin1, &itin2), ErrorResponseException);
  }

  void testCheckFlights_diffSize()
  {
    Itin itin1;
    SurfaceSeg seg1;
    SurfaceSeg seg2;
    itin1.travelSeg().push_back(&seg1);
    itin1.travelSeg().push_back(&seg2);

    Itin itin2;
    AirSeg seg3;
    itin2.travelSeg().push_back(&seg3);

    CPPUNIT_ASSERT_THROW(_handler->checkFlights(&itin1, &itin2), ErrorResponseException);
  }

  void testParseITN_PassItn1()
  {
    CPPUNIT_ASSERT_NO_THROW(
        _handler->parse(_dataHandle,
                        "<AncillaryPricingRequest Version=\"2.0.0\">"
                        "<AGI A10=\"VIE\" />"
                        "<ITN Q00=\"1\">"
                        "<SGI Q00=\"1\">"
                        "<FLI A01=\"LO1\" A02=\"LO2\" B00=\"BA\" B01=\"BA\" B30=\"Y\" "
                        "C7B=\"T\" D01=\"2014-12-13\" D02=\"2014-12-13\" D31=\"1230\" "
                        "D32=\"1630\" Q0B=\"2306\" S95=\"JET\" />"
                        "</SGI>"
                        "</ITN>"
                        "<ITN Q00=\"2\">"
                        "<SGI Q00=\"1\">"
                        "<FLI A01=\"LO1\" A02=\"LO2\" B00=\"BA\" B01=\"BA\" B30=\"Y\" "
                        "C7B=\"T\" D01=\"2014-12-13\" D02=\"2014-12-13\" D31=\"1230\" "
                        "D32=\"1630\" Q0B=\"2306\" S95=\"JET\" />"
                        "</SGI>"
                        "</ITN>"
                        "</AncillaryPricingRequest>"));
  }

  void testParseITN_FailItn1_Origin()
  {
    CPPUNIT_ASSERT_THROW(
        _handler->parse(_dataHandle,
                        "<AncillaryPricingRequest Version=\"2.0.0\">"
                        "<AGI A10=\"VIE\" />"
                        "<ITN Q00=\"1\">"
                        "<SGI Q00=\"1\">"
                        "<FLI A01=\"LO1\" A02=\"LO2\" B00=\"BA\" B01=\"BA\" B30=\"Y\" "
                        "C7B=\"T\" D01=\"2014-12-13\" D02=\"2014-12-13\" D31=\"1230\" "
                        "D32=\"1630\" Q0B=\"2306\" S95=\"JET\" />"
                        "</SGI>"
                        "</ITN>"
                        "<ITN Q00=\"2\">"
                        "<SGI Q00=\"1\">"
                        "<FLI A01=\"LO3\" A02=\"LO2\" B00=\"BA\" B01=\"BA\" B30=\"Y\" "
                        "C7B=\"T\" D01=\"2014-12-13\" D02=\"2014-12-13\" D31=\"1230\" "
                        "D32=\"1630\" Q0B=\"2306\" S95=\"JET\" />"
                        "</SGI>"
                        "</ITN>"
                        "</AncillaryPricingRequest>"),
        ErrorResponseException);
  }

  void testParseITN_FailItn1_Destination()
  {
    CPPUNIT_ASSERT_THROW(
        _handler->parse(_dataHandle,
                        "<AncillaryPricingRequest Version=\"2.0.0\">"
                        "<AGI A10=\"VIE\" />"
                        "<ITN Q00=\"1\">"
                        "<SGI Q00=\"1\">"
                        "<FLI A01=\"LO1\" A02=\"LO2\" B00=\"BA\" B01=\"BA\" B30=\"Y\" "
                        "C7B=\"T\" D01=\"2014-12-13\" D02=\"2014-12-13\" D31=\"1230\" "
                        "D32=\"1630\" Q0B=\"2306\" S95=\"JET\" />"
                        "</SGI>"
                        "</ITN>"
                        "<ITN Q00=\"2\">"
                        "<SGI Q00=\"1\">"
                        "<FLI A01=\"LO1\" A02=\"LO3\" B00=\"BA\" B01=\"BA\" B30=\"Y\" "
                        "C7B=\"T\" D01=\"2014-12-13\" D02=\"2014-12-13\" D31=\"1230\" "
                        "D32=\"1630\" Q0B=\"2306\" S95=\"JET\" />"
                        "</SGI>"
                        "</ITN>"
                        "</AncillaryPricingRequest>"),
        ErrorResponseException);
  }

  void testParseITN_FailItn1_DepartureDate()
  {
    CPPUNIT_ASSERT_THROW(
        _handler->parse(_dataHandle,
                        "<AncillaryPricingRequest Version=\"2.0.0\">"
                        "<AGI A10=\"VIE\" />"
                        "<ITN Q00=\"1\">"
                        "<SGI Q00=\"1\">"
                        "<FLI A01=\"LO1\" A02=\"LO2\" B00=\"BA\" B01=\"BA\" B30=\"Y\" "
                        "C7B=\"T\" D01=\"2014-12-13\" D02=\"2014-12-13\" D31=\"1230\" "
                        "D32=\"1630\" Q0B=\"2306\" S95=\"JET\" />"
                        "</SGI>"
                        "</ITN>"
                        "<ITN Q00=\"2\">"
                        "<SGI Q00=\"1\">"
                        "<FLI A01=\"LO1\" A02=\"LO2\" B00=\"BA\" B01=\"BA\" B30=\"Y\" "
                        "C7B=\"T\" D01=\"2014-12-12\" D02=\"2014-12-13\" D31=\"1230\" "
                        "D32=\"1630\" Q0B=\"2306\" S95=\"JET\" />"
                        "</SGI>"
                        "</ITN>"
                        "</AncillaryPricingRequest>"),
        ErrorResponseException);
  }

  void testParseITN_FailItn1_DepartureTime()
  {
    CPPUNIT_ASSERT_THROW(
        _handler->parse(_dataHandle,
                        "<AncillaryPricingRequest Version=\"2.0.0\">"
                        "<AGI A10=\"VIE\" />"
                        "<ITN Q00=\"1\">"
                        "<SGI Q00=\"1\">"
                        "<FLI A01=\"LO1\" A02=\"LO2\" B00=\"BA\" B01=\"BA\" B30=\"Y\" "
                        "C7B=\"T\" D01=\"2014-12-13\" D02=\"2014-12-13\" D31=\"1230\" "
                        "D32=\"1630\" Q0B=\"2306\" S95=\"JET\" />"
                        "</SGI>"
                        "</ITN>"
                        "<ITN Q00=\"2\">"
                        "<SGI Q00=\"1\">"
                        "<FLI A01=\"LO1\" A02=\"LO2\" B00=\"BA\" B01=\"BA\" B30=\"Y\" "
                        "C7B=\"T\" D01=\"2014-12-13\" D02=\"2014-12-13\" D31=\"1130\" "
                        "D32=\"1630\" Q0B=\"2306\" S95=\"JET\" />"
                        "</SGI>"
                        "</ITN>"
                        "</AncillaryPricingRequest>"),
        ErrorResponseException);
  }

  void testParseITN_FailItn1_ArrivalDate()
  {
    CPPUNIT_ASSERT_THROW(
        _handler->parse(_dataHandle,
                        "<AncillaryPricingRequest Version=\"2.0.0\">"
                        "<AGI A10=\"VIE\" />"
                        "<ITN Q00=\"1\">"
                        "<SGI Q00=\"1\">"
                        "<FLI A01=\"LO1\" A02=\"LO2\" B00=\"BA\" B01=\"BA\" B30=\"Y\" "
                        "C7B=\"T\" D01=\"2014-12-13\" D02=\"2014-12-13\" D31=\"1230\" "
                        "D32=\"1630\" Q0B=\"2306\" S95=\"JET\" />"
                        "</SGI>"
                        "</ITN>"
                        "<ITN Q00=\"2\">"
                        "<SGI Q00=\"1\">"
                        "<FLI A01=\"LO1\" A02=\"LO2\" B00=\"BA\" B01=\"BA\" B30=\"Y\" "
                        "C7B=\"T\" D01=\"2014-12-13\" D02=\"2014-12-14\" D31=\"1230\" "
                        "D32=\"1630\" Q0B=\"2306\" S95=\"JET\" />"
                        "</SGI>"
                        "</ITN>"
                        "</AncillaryPricingRequest>"),
        ErrorResponseException);
  }

  void testParseITN_FailItn1_ArrivalTime()
  {
    CPPUNIT_ASSERT_THROW(
        _handler->parse(_dataHandle,
                        "<AncillaryPricingRequest Version=\"2.0.0\">"
                        "<AGI A10=\"VIE\" />"
                        "<ITN Q00=\"1\">"
                        "<SGI Q00=\"1\">"
                        "<FLI A01=\"LO1\" A02=\"LO2\" B00=\"BA\" B01=\"BA\" B30=\"Y\" "
                        "C7B=\"T\" D01=\"2014-12-13\" D02=\"2014-12-13\" D31=\"1230\" "
                        "D32=\"1630\" Q0B=\"2306\" S95=\"JET\" />"
                        "</SGI>"
                        "</ITN>"
                        "<ITN Q00=\"2\">"
                        "<SGI Q00=\"1\">"
                        "<FLI A01=\"LO1\" A02=\"LO2\" B00=\"BA\" B01=\"BA\" B30=\"Y\" "
                        "C7B=\"T\" D01=\"2014-12-13\" D02=\"2014-12-13\" D31=\"1230\" "
                        "D32=\"1730\" Q0B=\"2306\" S95=\"JET\" />"
                        "</SGI>"
                        "</ITN>"
                        "</AncillaryPricingRequest>"),
        ErrorResponseException);
  }

  void testParseITN_FailItn1_MarketingCxr()
  {
    CPPUNIT_ASSERT_THROW(
        _handler->parse(_dataHandle,
                        "<AncillaryPricingRequest Version=\"2.0.0\">"
                        "<AGI A10=\"VIE\" />"
                        "<ITN Q00=\"1\">"
                        "<SGI Q00=\"1\">"
                        "<FLI A01=\"LO1\" A02=\"LO2\" B00=\"BA\" B01=\"BA\" B30=\"Y\" "
                        "C7B=\"T\" D01=\"2014-12-13\" D02=\"2014-12-13\" D31=\"1230\" "
                        "D32=\"1630\" Q0B=\"2306\" S95=\"JET\" />"
                        "</SGI>"
                        "</ITN>"
                        "<ITN Q00=\"2\">"
                        "<SGI Q00=\"1\">"
                        "<FLI A01=\"LO1\" A02=\"LO2\" B00=\"LT\" B01=\"BA\" B30=\"Y\" "
                        "C7B=\"T\" D01=\"2014-12-13\" D02=\"2014-12-13\" D31=\"1230\" "
                        "D32=\"1630\" Q0B=\"2306\" S95=\"JET\" />"
                        "</SGI>"
                        "</ITN>"
                        "</AncillaryPricingRequest>"),
        ErrorResponseException);
  }

  void testParseITN_FailItn1_OperatingCxr()
  {
    CPPUNIT_ASSERT_THROW(
        _handler->parse(_dataHandle,
                        "<AncillaryPricingRequest Version=\"2.0.0\">"
                        "<AGI A10=\"VIE\" />"
                        "<ITN Q00=\"1\">"
                        "<SGI Q00=\"1\">"
                        "<FLI A01=\"LO1\" A02=\"LO2\" B00=\"BA\" B01=\"BA\" B30=\"Y\" "
                        "C7B=\"T\" D01=\"2014-12-13\" D02=\"2014-12-13\" D31=\"1230\" "
                        "D32=\"1630\" Q0B=\"2306\" S95=\"JET\" />"
                        "</SGI>"
                        "</ITN>"
                        "<ITN Q00=\"2\">"
                        "<SGI Q00=\"1\">"
                        "<FLI A01=\"LO1\" A02=\"LO2\" B00=\"BA\" B01=\"LT\" B30=\"Y\" "
                        "C7B=\"T\" D01=\"2014-12-13\" D02=\"2014-12-13\" D31=\"1230\" "
                        "D32=\"1630\" Q0B=\"2306\" S95=\"JET\" />"
                        "</SGI>"
                        "</ITN>"
                        "</AncillaryPricingRequest>"),
        ErrorResponseException);
  }

  void testParseITN_FailItn1_BookingCode()
  {
    CPPUNIT_ASSERT_THROW(
        _handler->parse(_dataHandle,
                        "<AncillaryPricingRequest Version=\"2.0.0\">"
                        "<AGI A10=\"VIE\" />"
                        "<ITN Q00=\"1\">"
                        "<SGI Q00=\"1\">"
                        "<FLI A01=\"LO1\" A02=\"LO2\" B00=\"BA\" B01=\"BA\" B30=\"Y\" "
                        "C7B=\"T\" D01=\"2014-12-13\" D02=\"2014-12-13\" D31=\"1230\" "
                        "D32=\"1630\" Q0B=\"2306\" S95=\"JET\" />"
                        "</SGI>"
                        "</ITN>"
                        "<ITN Q00=\"2\">"
                        "<SGI Q00=\"1\">"
                        "<FLI A01=\"LO1\" A02=\"LO2\" B00=\"BA\" B01=\"BA\" B30=\"C\" "
                        "C7B=\"T\" D01=\"2014-12-13\" D02=\"2014-12-13\" D31=\"1230\" "
                        "D32=\"1630\" Q0B=\"2306\" S95=\"JET\" />"
                        "</SGI>"
                        "</ITN>"
                        "</AncillaryPricingRequest>"),
        ErrorResponseException);
  }

  void testParseITN_FailItn1_FlightNo()
  {
    CPPUNIT_ASSERT_THROW(
        _handler->parse(_dataHandle,
                        "<AncillaryPricingRequest Version=\"2.0.0\">"
                        "<AGI A10=\"VIE\" />"
                        "<ITN Q00=\"1\">"
                        "<SGI Q00=\"1\">"
                        "<FLI A01=\"LO1\" A02=\"LO2\" B00=\"BA\" B01=\"BA\" B30=\"Y\" "
                        "C7B=\"T\" D01=\"2014-12-13\" D02=\"2014-12-13\" D31=\"1230\" "
                        "D32=\"1630\" Q0B=\"2306\" S95=\"JET\" />"
                        "</SGI>"
                        "</ITN>"
                        "<ITN Q00=\"2\">"
                        "<SGI Q00=\"1\">"
                        "<FLI A01=\"LO1\" A02=\"LO2\" B00=\"BA\" B01=\"BA\" B30=\"Y\" "
                        "C7B=\"T\" D01=\"2014-12-13\" D02=\"2014-12-13\" D31=\"1230\" "
                        "D32=\"1630\" Q0B=\"2307\" S95=\"JET\" />"
                        "</SGI>"
                        "</ITN>"
                        "</AncillaryPricingRequest>"),
        ErrorResponseException);
  }

  void testParseITN_FailItn1_Equipment()
  {
    CPPUNIT_ASSERT_THROW(
        _handler->parse(_dataHandle,
                        "<AncillaryPricingRequest Version=\"2.0.0\">"
                        "<AGI A10=\"VIE\" />"
                        "<ITN Q00=\"1\">"
                        "<SGI Q00=\"1\">"
                        "<FLI A01=\"LO1\" A02=\"LO2\" B00=\"BA\" B01=\"BA\" B30=\"Y\" "
                        "C7B=\"T\" D01=\"2014-12-13\" D02=\"2014-12-13\" D31=\"1230\" "
                        "D32=\"1630\" Q0B=\"2306\" S95=\"JET\" />"
                        "</SGI>"
                        "</ITN>"
                        "<ITN Q00=\"2\">"
                        "<SGI Q00=\"1\">"
                        "<FLI A01=\"LO1\" A02=\"LO2\" B00=\"BA\" B01=\"BA\" B30=\"Y\" "
                        "C7B=\"T\" D01=\"2014-12-13\" D02=\"2014-12-13\" D31=\"1230\" "
                        "D32=\"1630\" Q0B=\"2306\" S95=\"BUS\" />"
                        "</SGI>"
                        "</ITN>"
                        "</AncillaryPricingRequest>"),
        ErrorResponseException);
  }

  void testParseBTS()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle, "<BTS Q00=\"13\" />"));
    AncRequest* req =
        dynamic_cast<AncRequest*>(dynamic_cast<AncillaryPricingTrx*>(_trx)->getRequest());
    CPPUNIT_ASSERT(req);
    CPPUNIT_ASSERT(req->displayBaggageTravelIndices().find(13) !=
                   req->displayBaggageTravelIndices().end());
  }

  void testMultiAirportCityNoArunk()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(
        _dataHandle, "<AN><AGI A10=\"VIE\" /><ITN><SGI><FLI A01=\"DFW\" A02=\"LGA\" B00=\"U0\" "
                     "/></SGI><SGI><FLI A01=\"JFK\" A02=\"LHR\" B00=\"U0\" /></SGI></ITN></AN>"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(size_t(1), trx->itin().size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), trx->itin().front()->travelSeg().size());
    CPPUNIT_ASSERT(trx->itin().front()->travelSeg()[0]->segmentType() == Air);
    CPPUNIT_ASSERT(trx->itin().front()->travelSeg()[1]->segmentType() == Air);
    CPPUNIT_ASSERT_EQUAL(int16_t(1), trx->itin().front()->travelSeg()[0]->segmentOrder());
    CPPUNIT_ASSERT_EQUAL(int16_t(2), trx->itin().front()->travelSeg()[1]->segmentOrder());
  }

  void testNonMultiAirportCityAddArunk()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(
        _dataHandle, "<AN><AGI A10=\"VIE\" /><ITN><SGI><FLI A01=\"DFW\" A02=\"LGA\" B00=\"U0\" "
                     "/></SGI><SGI><FLI A01=\"ORD\" A02=\"LHR\" B00=\"U0\" /></SGI></ITN></AN>"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(size_t(1), trx->itin().size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), trx->itin().front()->travelSeg().size());
    CPPUNIT_ASSERT(trx->itin().front()->travelSeg()[1]->segmentType() != Air);
    CPPUNIT_ASSERT_EQUAL(int16_t(1), trx->itin().front()->travelSeg()[0]->segmentOrder());
    CPPUNIT_ASSERT_EQUAL(int16_t(2), trx->itin().front()->travelSeg()[1]->segmentOrder());
    CPPUNIT_ASSERT_EQUAL(int16_t(3), trx->itin().front()->travelSeg()[2]->segmentOrder());
  }

  AncRequest::AncFareBreakAssociation* prepareRtw()
  {
    _handler->_pricingTrx = _memHandle.create<PricingTrx>();
    _handler->_pricingTrx->setOptions(_memHandle.create<PricingOptions>());
    Itin* itin = _memHandle.create<Itin>();
    _handler->_pricingTrx->itin().push_back(itin);
    TravelSeg* ts = _memHandle.create<AirSeg>();
    ts->origAirport() = ts->destAirport() = "RTW";
    itin->travelSeg().push_back(ts);
    _handler->_request = _memHandle.create<AncRequest>();

    for (size_t i = 0; i < 4; ++i)
    {
      _handler->_request->fareBreakAssociationPerItin()[itin].push_back(
          _memHandle.create<AncRequest::AncFareBreakAssociation>());
      _handler->_request->fareBreakAssociationPerItin()[itin].back()->fareComponentID() = 1;
    }

    return _handler->_request->fareBreakAssociationPerItin()[itin][1];
  }

  void testIsRtwA()
  {
    prepareRtw();
    _handler->_actionCode = "A";
    _handler->detectRtw();
    CPPUNIT_ASSERT(_handler->_pricingTrx->getOptions()->isRtw());
  }

  void testIsRtwWPAE()
  {
    prepareRtw();
    _handler->_actionCode = "WP*AE";
    _handler->_reqType = CommonRequestHandler::WPAE;
    _handler->detectRtw();
    CPPUNIT_ASSERT(_handler->_pricingTrx->getOptions()->isRtw());
  }

  void testIsNotRtwFcNo()
  {
    _handler->_actionCode = "A";
    prepareRtw()->fareComponentID() = 2;
    _handler->detectRtw();
    CPPUNIT_ASSERT(!_handler->_pricingTrx->getOptions()->isRtw());
  }

  void testIsNotRtwNoActionCode()
  {
    prepareRtw();
    _handler->detectRtw();
    CPPUNIT_ASSERT(!_handler->_pricingTrx->getOptions()->isRtw());
  }

  void testIsNotRtwWrongActionCode()
  {
    _handler->_actionCode = "B";
    prepareRtw();
    _handler->detectRtw();
    CPPUNIT_ASSERT(!_handler->_pricingTrx->getOptions()->isRtw());
  }

  void testParseOfficeStationCodePresent()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(
        _dataHandle, "<AN><AGI A10=\"VIE\" AE2=\"0230840\"/><ITN><SGI><FLI A01=\"DFW\" A02=\"LGA\" B00=\"U0\" "
                     "/></SGI><SGI><FLI A01=\"ORD\" A02=\"LHR\" B00=\"U0\" /></SGI></ITN></AN>"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(std::string("0230840"), trx->getRequest()->ticketingAgent()->officeStationCode());
  }

  void testParseOfficeStationCodeNotPresent()
  {
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(
        _dataHandle, "<AN><AGI A10=\"VIE\" /><ITN><SGI><FLI A01=\"DFW\" A02=\"LGA\" B00=\"U0\" "
                     "/></SGI><SGI><FLI A01=\"ORD\" A02=\"LHR\" B00=\"U0\" /></SGI></ITN></AN>"));
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT(trx->getRequest()->ticketingAgent()->officeStationCode().empty());
  }

  void validateGBA_DXT()
  {
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(size_t(1), trx->getOptions()->serviceGroupsVec().size());
    CPPUNIT_ASSERT_EQUAL(ServiceGroup("BG"),
                         trx->getOptions()->serviceGroupsVec().front().groupCode());
    CPPUNIT_ASSERT(trx->getOptions()->serviceGroupsVec().front().isAncillaryServiceType('A'));
    CPPUNIT_ASSERT(trx->getOptions()->serviceGroupsVec().front().isAncillaryServiceType('C'));
    CPPUNIT_ASSERT(trx->getOptions()->serviceGroupsVec().front().isAncillaryServiceType('B'));
    CPPUNIT_ASSERT(trx->getOptions()->serviceGroupsVec().front().isAncillaryServiceType('E'));
    CPPUNIT_ASSERT_EQUAL(RequestedOcFeeGroup::DisclosureData, trx->getOptions()->serviceGroupsVec().front().getRequestedInformation());
  }

  bool compareServiceGroups(const RequestedOcFeeGroup& rfg, ServiceGroup sg) const
  {
    return rfg.groupCode() == sg;
  }

  void validateGBA_CAT_BG()
  {
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    std::vector<RequestedOcFeeGroup>& serviceGroups = trx->getOptions()->serviceGroupsVec();

    std::vector<RequestedOcFeeGroup>::iterator it = std::find_if(serviceGroups.begin(), serviceGroups.end(),
                  boost::lambda::bind(&AncillaryPricingRequestHandlerTest::compareServiceGroups, this, boost::lambda::_1, "BG"));


    CPPUNIT_ASSERT(serviceGroups.end() != it);
    CPPUNIT_ASSERT(ServiceGroup("BG") == it->groupCode());
    CPPUNIT_ASSERT(it->isAncillaryServiceType('C'));
    CPPUNIT_ASSERT_EQUAL(RequestedOcFeeGroup::CatalogData, it->getRequestedInformation());
  }

  void validateGBA_CAT_PT()
  {
    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    std::vector<RequestedOcFeeGroup>& serviceGroups = trx->getOptions()->serviceGroupsVec();

    std::vector<RequestedOcFeeGroup>::iterator it = std::find_if(serviceGroups.begin(), serviceGroups.end(),
                  boost::lambda::bind(&AncillaryPricingRequestHandlerTest::compareServiceGroups, this, boost::lambda::_1, "PT"));

    CPPUNIT_ASSERT(serviceGroups.end() != it);
    CPPUNIT_ASSERT(ServiceGroup("PT") == it->groupCode());
    CPPUNIT_ASSERT(it->isAncillaryServiceType('C'));
    CPPUNIT_ASSERT_EQUAL(RequestedOcFeeGroup::CatalogData, it->getRequestedInformation());
  }


  void testParseGBA_DXT_empty()
  {
    CPPUNIT_ASSERT_NO_THROW(_handlerM->parse(_dataHandle, "<AncillaryPricingRequest Version=\"3.0.0\">"
                           "<AGI A10=\"VIE\" />"
                           "<BIL S0R=\"PANC\" A70=\"A\"/>"
                           "<PRO> <RFG GBA=\"DXT\"/> </PRO>"
                           + _itinInAncPricingReq));
    validateGBA_DXT();
  }

  void testParseGBA_DXT_empty_withPNM()
  {
    CPPUNIT_ASSERT_NO_THROW(_handlerM->parse(_dataHandle, "<AncillaryPricingRequest Version=\"3.0.0\">"
                           "<AGI A10=\"VIE\" />"
                           "<BIL S0R=\"PANC\" A70=\"A\"/>"
                           "<PRO> <RFG GBA=\"DXT\"/> </PRO>"
                           + _itinInAncPricingReqWithPnm));
    validateGBA_DXT();
  }

  void testParseGBA_DXT_BGonly()
  {
    CPPUNIT_ASSERT_NO_THROW(_handlerM->parse(_dataHandle, "<AncillaryPricingRequest Version=\"3.0.0\">"
                           "<AGI A10=\"VIE\" />"
                           "<BIL S0R=\"PANC\" A70=\"A\"/>"
                           "<PRO><RFG GBA=\"DXT\"><S01>BG</S01></RFG></PRO>"
                           + _itinInAncPricingReq));
    validateGBA_DXT();
  }

  void testParseGBA_DXT_MLonly()
  {
    CPPUNIT_ASSERT_NO_THROW(_handlerM->parse(_dataHandle, "<AncillaryPricingRequest Version=\"3.0.0\">"
                           "<AGI A10=\"VIE\" />"
                           "<BIL S0R=\"PANC\" A70=\"A\"/>"
                           "<PRO><RFG GBA=\"DXT\"><S01>ML</S01></RFG></PRO>"
                           + _itinInAncPricingReq));
    validateGBA_DXT();
  }

  void testParseGBA_DXT_BGandML()
  {
    CPPUNIT_ASSERT_NO_THROW(_handlerM->parse(_dataHandle, "<AncillaryPricingRequest Version=\"3.0.0\">"
                           "<AGI A10=\"VIE\" />"
                           "<BIL S0R=\"PANC\" A70=\"A\"/>"
                           "<PRO><RFG GBA=\"DXT\"><S01>ML</S01><S01>BG</S01></RFG></PRO>"
                           + _itinInAncPricingReq));
    validateGBA_DXT();
  }


  void testParseGBA_CAT_empty()
  {
    CPPUNIT_ASSERT_NO_THROW(_handlerM->parse(_dataHandle, "<AncillaryPricingRequest Version=\"3.0.0\">"
                           "<AGI A10=\"VIE\" />"
                           "<BIL S0R=\"PANC\" A70=\"A\"/>"
                           "<PRO> <RFG GBA=\"CAT\"/> </PRO>"
                           + _itinInAncPricingReqWithPnm));

    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(size_t(2), trx->getOptions()->serviceGroupsVec().size());

    validateGBA_CAT_BG();
    validateGBA_CAT_PT();
  }

  void testParseGBA_CAT_empty_noPNM()
  {
    CPPUNIT_ASSERT_THROW(_handlerM->parse(_dataHandle, "<AncillaryPricingRequest Version=\"3.0.0\">"
                         "<AGI A10=\"VIE\" />"
                         "<BIL S0R=\"PANC\" A70=\"POST\"/>"
                         "<PRO> <RFG GBA=\"CAT\"/> </PRO>"
                         + _itinInAncPricingReq),
                          tse::ErrorResponseException);
  }

  void testParseGBA_CAT_BGonly()
  {
    CPPUNIT_ASSERT_NO_THROW(_handlerM->parse(_dataHandle, "<AncillaryPricingRequest Version=\"3.0.0\">"
                           "<AGI A10=\"VIE\" />"
                           "<BIL S0R=\"PANC\" A70=\"A\"/>"
                           "<PRO><RFG GBA=\"CAT\"><S01>BG</S01></RFG></PRO>"
                           + _itinInAncPricingReqWithPnm));

    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(size_t(1), trx->getOptions()->serviceGroupsVec().size());

    validateGBA_CAT_BG();
  }

  void testParseGBA_CAT_MLonly()
  {
    CPPUNIT_ASSERT_NO_THROW(_handlerM->parse(_dataHandle, "<AncillaryPricingRequest Version=\"3.0.0\">"
                           "<AGI A10=\"VIE\" />"
                           "<BIL S0R=\"PANC\" A70=\"A\"/>"
                           "<PRO><RFG GBA=\"CAT\"><S01>ML</S01></RFG></PRO>"
                           + _itinInAncPricingReqWithPnm));

    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(size_t(2), trx->getOptions()->serviceGroupsVec().size());

    validateGBA_CAT_BG();
    validateGBA_CAT_PT();
  }

  void testParseGBA_CAT_BGandML()
  {
    CPPUNIT_ASSERT_NO_THROW(_handlerM->parse(_dataHandle, "<AncillaryPricingRequest Version=\"3.0.0\">"
                           "<AGI A10=\"VIE\" />"
                           "<BIL S0R=\"PANC\" A70=\"A\"/>"
                           "<PRO><RFG GBA=\"CAT\"><S01>BG</S01></RFG></PRO>"
                           + _itinInAncPricingReqWithPnm));

    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(size_t(1), trx->getOptions()->serviceGroupsVec().size());

    validateGBA_CAT_BG();
  }


  void testParseGBA_OCF_empty()
  {
    CPPUNIT_ASSERT_NO_THROW(_handlerM->parse(_dataHandle, "<AncillaryPricingRequest Version=\"3.0.0\">"
                           "<AGI A10=\"VIE\" />"
                           "<PRO> <RFG GBA=\"OCF\"/> </PRO>"
                           + _itinInAncPricingReq));

    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(size_t(0), trx->getOptions()->serviceGroupsVec().size());
    CPPUNIT_ASSERT( trx->getOptions()->isProcessAllGroups());
  }

  void testParseGBA_OCF_1group()
  {
    CPPUNIT_ASSERT_NO_THROW(_handlerM->parse(_dataHandle, "<AncillaryPricingRequest Version=\"3.0.0\">"
                           "<AGI A10=\"VIE\" />"
                           "<PRO><RFG GBA=\"OCF\"><S01>ML</S01></RFG></PRO>"
                           + _itinInAncPricingReq));

    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(size_t(1), trx->getOptions()->serviceGroupsVec().size());
    CPPUNIT_ASSERT_EQUAL(ServiceGroup("ML"),
                         trx->getOptions()->serviceGroupsVec().front().groupCode());

    CPPUNIT_ASSERT(trx->getOptions()->serviceGroupsVec().front().isEmptyAncillaryServiceType());
    CPPUNIT_ASSERT_EQUAL(RequestedOcFeeGroup::AncillaryData, trx->getOptions()->serviceGroupsVec().front().getRequestedInformation());
  }

  void testParseGBA_OCF_2groups()
  {
    CPPUNIT_ASSERT_NO_THROW(_handlerM->parse(_dataHandle, "<AncillaryPricingRequest Version=\"3.0.0\">"
                           "<AGI A10=\"VIE\" />"
                           "<PRO><RFG GBA=\"OCF\"><S01>ML</S01><S01>SA</S01></RFG></PRO>"
                           + _itinInAncPricingReq));

    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    std::vector<RequestedOcFeeGroup>& serviceGroups = trx->getOptions()->serviceGroupsVec();
    CPPUNIT_ASSERT_EQUAL(size_t(2), serviceGroups.size());

    std::vector<RequestedOcFeeGroup>::iterator it = std::find_if(serviceGroups.begin(), serviceGroups.end(),
                  boost::lambda::bind(&AncillaryPricingRequestHandlerTest::compareServiceGroups, this, boost::lambda::_1, "ML"));
    CPPUNIT_ASSERT(serviceGroups.end() != it);
    CPPUNIT_ASSERT(ServiceGroup("ML") == it->groupCode());
    CPPUNIT_ASSERT(it->isEmptyAncillaryServiceType());
    CPPUNIT_ASSERT_EQUAL(RequestedOcFeeGroup::AncillaryData, it->getRequestedInformation());


    it = std::find_if(serviceGroups.begin(), serviceGroups.end(),
                      boost::lambda::bind(&AncillaryPricingRequestHandlerTest::compareServiceGroups, this, boost::lambda::_1, "SA"));
    CPPUNIT_ASSERT(serviceGroups.end() != it);
    CPPUNIT_ASSERT(ServiceGroup("SA") == it->groupCode());
    CPPUNIT_ASSERT(it->isEmptyAncillaryServiceType());
    CPPUNIT_ASSERT_EQUAL(RequestedOcFeeGroup::AncillaryData, it->getRequestedInformation());
  }


  void testParseGBA_Incorrect()
  {
    CPPUNIT_ASSERT_THROW(_handlerM->parse(_dataHandle, "<AncillaryPricingRequest Version=\"3.0.0\">"
                           "<AGI A10=\"VIE\" />"
                           "<PRO><RFG GBA=\"WTF\"><S01>ML</S01><S01>SA</S01></RFG></PRO>"
                           + _itinInAncPricingReq), ErrorResponseException);
  }


  void testParseGBA_IncorrectS01()
  {
    CPPUNIT_ASSERT_NO_THROW(_handlerM->parse(_dataHandle, "<AncillaryPricingRequest Version=\"3.0.0\">"
                           "<AGI A10=\"VIE\" />"
                           "<PRO><S01>BG</S01><RFG GBA=\"OCF\"><S01>ML</S01></RFG></PRO>"
                           + _itinInAncPricingReq));

    AncillaryPricingTrx* trx = dynamic_cast<AncillaryPricingTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(size_t(1), trx->getOptions()->serviceGroupsVec().size());
    CPPUNIT_ASSERT_EQUAL(ServiceGroup("ML"),
                         trx->getOptions()->serviceGroupsVec().front().groupCode());

    CPPUNIT_ASSERT(trx->getOptions()->serviceGroupsVec().front().isEmptyAncillaryServiceType());
    CPPUNIT_ASSERT_EQUAL(RequestedOcFeeGroup::AncillaryData, trx->getOptions()->serviceGroupsVec().front().getRequestedInformation());
  }

  void testParseGBA_DXT_RouteIndicator_Pass()
  {
    CPPUNIT_ASSERT_NO_THROW(_handlerM->parse(_dataHandle, "<AncillaryPricingRequest Version=\"3.0.0\">"
                           "<AGI A10=\"VIE\" />"
                           "<BIL S0R=\"PANC\" A70=\"A\"/>"
                           "<PRO><RFG GBA=\"DXT\"></RFG></PRO>"
                           + _itinInAncPricingReq));
    validateGBA_DXT();
  }

  void testParseGBA_DXT_RouteIndicator_Fail()
  {
    CPPUNIT_ASSERT_THROW(_handlerM->parse(_dataHandle, "<AncillaryPricingRequest Version=\"3.0.0\">"
                           "<AGI A10=\"VIE\" />"
                           "<BIL S0R=\"PANC\" A70=\"PRET\"/>"
                           "<PRO><RFG GBA=\"DXT\"></RFG></PRO>"
                           + _itinInAncPricingReqNoRouteIndicator), ErrorResponseException);
  }


  void testParseRCP_RES()
  {
    CPPUNIT_ASSERT_NO_THROW(_handlerM->parse(_dataHandle, "<AncillaryPricingRequest Version=\"3.0.0\">"
                           "<AGI A10=\"VIE\" />"
                           "<BIL S0R=\"PANC\" A70=\"A\"/>"
                           "<PRO RCP=\"RES\"></PRO>"
                           + _itinInAncPricingReq));


  }

  void testParseRCP_CKI()
  {
    CPPUNIT_ASSERT_NO_THROW(_handlerM->parse(_dataHandle, "<AncillaryPricingRequest Version=\"3.0.0\">"
                           "<AGI A10=\"VIE\" />"
                           "<BIL S0R=\"PANC\" A70=\"A\"/>"
                           "<PRO RCP=\"CKI\"></PRO>"
                           + _itinInAncPricingReq ));

    CPPUNIT_ASSERT(_handlerM->_pricingTrx->getOptions()->getAncRequestPath() == AncRequestPath::AncCheckInPath);
  }

  void testParseRCP_empty()
  {
    CPPUNIT_ASSERT_NO_THROW(_handlerM->parse(_dataHandle, "<AncillaryPricingRequest Version=\"3.0.0\">"
                           "<AGI A10=\"VIE\" />"
                           "<BIL S0R=\"PANC\" A70=\"A\"/>"
                           "<PRO></PRO>"
                           + _itinInAncPricingReq ));
  }


  void testParseRCP_incorrect()
  {
    CPPUNIT_ASSERT_THROW(_handlerM->parse(_dataHandle, "<AncillaryPricingRequest Version=\"3.0.0\">"
                           "<AGI A10=\"VIE\" />"
                           "<BIL S0R=\"PANC\" A70=\"A\"/>"
                           "<PRO RCP=\"CLL\"></PRO>"
                           + _itinInAncPricingReq), ErrorResponseException);
  }


  void test_ItinNoFBAFail()
  {
    CPPUNIT_ASSERT_THROW(_handler->parse(_dataHandle, "<AN><AGI A10=\"VIE\" /><ITN>\
        <SGI Q00=\"1\"><FLI A01=\"LO1\" A02=\"LO2\" B00=\"AA\" /><FBA Q6D=\"1\" /></SGI>\
        <SGI Q00=\"2\"><FLI A01=\"LO3\" A02=\"LO4\" B00=\"AA\" /></SGI>\
        <FBI Q00=\"1\" B02=\"WS\" C50=\"130.00\" B50=\"DA10UN2\" S53=\"0\" /></ITN></AN>"), ErrorResponseException);
  }

  void test_ItinNoFBIFail()
  {
    CPPUNIT_ASSERT_THROW(_handler->parse(_dataHandle, "<AN><AGI A10=\"VIE\" /><ITN>\
        <SGI Q00=\"1\"><FLI A01=\"LO1\" A02=\"LO2\" B00=\"AA\" /><FBA Q6D=\"1\" /></SGI>\
        <SGI Q00=\"2\"><FLI A01=\"LO3\" A02=\"LO4\" B00=\"AA\" /><FBA Q6D=\"2\" /></SGI>\
        <FBI Q00=\"1\" B02=\"WS\" C50=\"130.00\" B50=\"DA10UN2\" S53=\"0\" /></ITN></AN>"), ErrorResponseException);
  }


private:
  AncillaryPricingRequestHandler* _handler;
  AncillaryPricingRequestHandlerMock* _handlerM;
  TestMemHandle _memHandle;
  Trx* _trx;
  DataHandle _dataHandle;
  AncRequest* _ancReq;

  static const std::string _validAid1;
  static const std::string _validAid2;
  static const std::string _itinInAncPricingReq;
  static const std::string _itinInAncPricingReqWithPnm;
  static const std::string _itinInAncPricingReqNoRouteIndicator;
  static const std::string _itinFfyInAncPricingReq;
};

const std::string AncillaryPricingRequestHandlerTest::_validAid1 = "1S|C|0DF|1.2|1000|1500";
const std::string AncillaryPricingRequestHandlerTest::_validAid2 = "1S|C|0DF|1.2|1000";

const std::string AncillaryPricingRequestHandlerTest::_itinInAncPricingReq = "<ITN Q00=\"1\">"
                                                "<SGI Q00=\"1\">"
                                                "<FLI A01=\"LO1\" A02=\"LO2\" B00=\"BA\" B01=\"BA\" B30=\"Y\" "
                                                "C7B=\"T\" D01=\"2014-12-13\" D02=\"2014-12-13\" D31=\"1230\" "
                                                "D32=\"1630\" Q0B=\"2306\" S95=\"JET\" N0E=\"Y\" />"
                                                "</SGI> "
                                                "</ITN> "
                                                "<ITN Q00=\"2\">"
                                                "<SGI Q00=\"1\">"
                                                "<FLI A01=\"LO1\" A02=\"LO2\" B00=\"BA\" B01=\"BA\" B30=\"Y\" "
                                                "C7B=\"T\" D01=\"2014-12-13\" D02=\"2014-12-13\" D31=\"1230\" "
                                                "D32=\"1630\" Q0B=\"2306\" S95=\"JET\" N0E=\"Y\" />"
                                                "<FBA Q6D=\"1\"/> "
                                                "</SGI> "
                                                "<FBI Q00=\"1\" B02=\"BA\" C50=\"810.42\" B50=\"YFF59OW\" S53=\"XOX\" "
                                                "Q3W=\"21\" S90=\"P2P8\" FTY=\"00\" S37=\"ATP\" />"
                                                "</ITN> "
                                                "</AncillaryPricingRequest> ";

const std::string AncillaryPricingRequestHandlerTest::_itinInAncPricingReqWithPnm =
                                                "<ITN Q00=\"1\">"
                                                "<SGI Q00=\"1\">"
                                                "<FLI A01=\"LO1\" A02=\"LO2\" B00=\"BA\" B01=\"BA\" B30=\"Y\" "
                                                "C7B=\"T\" D01=\"2014-12-13\" D02=\"2014-12-13\" D31=\"1230\" "
                                                "D32=\"1630\" Q0B=\"2306\" S95=\"JET\" N0E=\"Y\" />"
                                                "</SGI> "
                                                "</ITN> "
                                                "<ITN Q00=\"2\">"
                                                "<IRO>"
                                                "  <PXI B70=\"ADT\" Q0U=\"1\">"
                                                "    <PNM S0L=\"01.01\" Q86=\"2\" Q87=\"1397503187540\" />"
                                                "  </PXI>"
                                                "</IRO>"
                                                "<SGI Q00=\"1\">"
                                                "<FLI A01=\"LO1\" A02=\"LO2\" B00=\"BA\" B01=\"BA\" B30=\"Y\" "
                                                "C7B=\"T\" D01=\"2014-12-13\" D02=\"2014-12-13\" D31=\"1230\" "
                                                "D32=\"1630\" Q0B=\"2306\" S95=\"JET\" N0E=\"Y\" />"
                                                "<FBA Q6D=\"1\"/> "
                                                "</SGI> "
                                                "<FBI Q00=\"1\" B02=\"BA\" C50=\"810.42\" B50=\"YFF59OW\" S53=\"XOX\" "
                                                "Q3W=\"21\" S90=\"P2P8\" FTY=\"00\" S37=\"ATP\" />"
                                                "</ITN> "
                                                "</AncillaryPricingRequest> ";

const std::string AncillaryPricingRequestHandlerTest::_itinInAncPricingReqNoRouteIndicator = "<ITN Q00=\"1\">"
                                                "<SGI Q00=\"1\">"
                                                "<FLI A01=\"LO1\" A02=\"LO2\" B00=\"BA\" B01=\"BA\" B30=\"Y\" "
                                                "D01=\"2014-12-13\" D02=\"2014-12-13\" D31=\"1230\" "
                                                "D32=\"1630\" Q0B=\"2306\" S95=\"JET\" N0E=\"Y\" />"
                                                "</SGI> "
                                                "</ITN> "
                                                "<ITN Q00=\"2\">"
                                                "<SGI Q00=\"1\">"
                                                "<FLI A01=\"LO1\" A02=\"LO2\" B00=\"BA\" B01=\"BA\" B30=\"Y\" "
                                                "D01=\"2014-12-13\" D02=\"2014-12-13\" D31=\"1230\" "
                                                "D32=\"1630\" Q0B=\"2306\" S95=\"JET\" N0E=\"Y\" />"
                                                "<FBA Q6D=\"1\"/> "
                                                "</SGI> "
                                                "<FBI Q00=\"1\" B02=\"BA\" C50=\"810.42\" B50=\"YFF59OW\" S53=\"XOX\" "
                                                "Q3W=\"21\" S90=\"P2P8\" FTY=\"00\" S37=\"ATP\" />"
                                                "</ITN> "
                                                "</AncillaryPricingRequest> ";

const std::string AncillaryPricingRequestHandlerTest::_itinFfyInAncPricingReq = "<ITN Q00=\"1\">"
                                                "<SGI Q00=\"1\">"
                                                "<FLI A01=\"LO1\" A02=\"LO2\" B00=\"BA\" B01=\"BA\" B30=\"Y\" "
                                                "C7B=\"T\" D01=\"2014-12-13\" D02=\"2014-12-13\" D31=\"1230\" "
                                                "D32=\"1630\" Q0B=\"2306\" S95=\"JET\" N0E=\"Y\" />"
                                                "</SGI> "
                                                "</ITN> "
                                                "<ITN Q00=\"2\">"
                                                "<FFY Q7D=\"1\" B00=\"BA\" />"
                                                "<IRO>"
                                                "<PXI B70=\"ADT\" Q0U=\"1\" />"
                                                "</IRO>"
                                                "<SGI Q00=\"1\">"
                                                "<FLI A01=\"LO1\" A02=\"LO2\" B00=\"BA\" B01=\"BA\" B30=\"Y\" "
                                                "C7B=\"T\" D01=\"2014-12-13\" D02=\"2014-12-13\" D31=\"1230\" "
                                                "D32=\"1630\" Q0B=\"2306\" S95=\"JET\" N0E=\"Y\" />"
                                                "<FBA Q6D=\"1\"/> "
                                                "</SGI> "
                                                "<FBI Q00=\"1\" B02=\"BA\" C50=\"810.42\" B50=\"YFF59OW\" S53=\"XOX\" "
                                                "Q3W=\"21\" S90=\"P2P8\" FTY=\"00\" S37=\"ATP\" />"
                                                "</ITN> "
                                                "</AncillaryPricingRequest> ";

CPPUNIT_TEST_SUITE_REGISTRATION(AncillaryPricingRequestHandlerTest);

} // namespace
