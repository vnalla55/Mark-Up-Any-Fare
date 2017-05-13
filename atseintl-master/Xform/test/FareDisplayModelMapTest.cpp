//----------------------------------------------------------------------------
//
//      File: FareDisplayModelMapTest.cpp
//      Description: Unit test for FareDisplayModelMapTest class
//      Created: September 05, 2008
//      Authors: Konrad Koch
//
//  Copyright Sabre 2008, 2009
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include <xercesc/sax2/DefaultHandler.hpp>
#include "Xform/FareDisplayModelMap.h"
#include "DBAccess/DataHandle.h"
#include "Common/Config/ConfigMan.h"
#include "Common/NonFatalErrorResponseException.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/NonPublishedValues.h"
#include "DataModel/ERDFareComp.h"
#include "DataModel/ERDFltSeg.h"
#include "Common/XMLChString.h"
#include "test/include/TestMemHandle.h"
#include "Xform/test/MockXercescAttributes.h"

namespace tse
{
class FareDisplayModelMapTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareDisplayModelMapTest);
  CPPUNIT_TEST(testTrxValid);
  CPPUNIT_TEST(testTrxOptions);
  CPPUNIT_TEST(testFareDisplayInfoParsing);
  CPPUNIT_TEST(testFareDisplayFQRequestType);
  CPPUNIT_TEST(testFareDisplayRDRequestType);
  CPPUNIT_TEST(testFareDisplayERDRequestType);
  CPPUNIT_TEST(testAgentValid);
  CPPUNIT_TEST(testAgentDataParsing);
  CPPUNIT_TEST(testBillingDataParsing);
  CPPUNIT_TEST(testCommonBillingDataParsing);
  CPPUNIT_TEST(testDiagDataParsing);
  CPPUNIT_TEST(testPassengerDataParsing);
  CPPUNIT_TEST(testProcessingOptionsParsing);
  CPPUNIT_TEST(testFareComponentDataParsing);
  CPPUNIT_TEST(testSegmentDataParsing);
  CPPUNIT_TEST(testFQCat25DataParsing);
  CPPUNIT_TEST(testERDCat25DataParsing);
  CPPUNIT_TEST(testFQCat35DataParsing);
  CPPUNIT_TEST(testERDCat35DataParsing);
  CPPUNIT_TEST(testFQDiscountedDataParsing);
  CPPUNIT_TEST(testERDDiscountedDataParsing);
  CPPUNIT_TEST(testERDDataParsing);
  CPPUNIT_TEST(testStoreProcessingOptions_MAC1);
  CPPUNIT_TEST(testSaveProcessingOptions_MAC1);
  CPPUNIT_TEST(testStoreProcessingOptions_MAC2);
  CPPUNIT_TEST(testSaveProcessingOptions_MAC2);
  CPPUNIT_TEST(testStoreProcessingOptions_MAC3);
  CPPUNIT_TEST(testSaveProcessingOptions_MAC3);
  CPPUNIT_TEST(testStoreProcessingOptions_MAC4);
  CPPUNIT_TEST(testSaveProcessingOptions_MAC4);
  CPPUNIT_TEST(testStoreProcessingOptions_MAC5);
  CPPUNIT_TEST(testStoreProcessingOptions_MAC6);

  CPPUNIT_TEST(testStoreProcessingOptions_XFF);
  CPPUNIT_TEST(testSaveProcessingOptions_XFF_EPR_NoError);
  CPPUNIT_TEST(testSaveProcessingOptions_XFF_NOEPR_Error);

  CPPUNIT_TEST(testStoreProcessingOptions_PDO);
  CPPUNIT_TEST(testSaveProcessingOptions_PDO_EPR_NoError);
  CPPUNIT_TEST(testSaveProcessingOptions_PDO_NOEPR_Error);

  CPPUNIT_TEST(testStoreProcessingOptions_PDR);
  CPPUNIT_TEST(testSaveProcessingOptions_PDR_EPR_NoError);
  CPPUNIT_TEST(testSaveProcessingOptions_PDR_NOEPR_Error);

  CPPUNIT_TEST(testStoreProcessingOptions_XRS);
  CPPUNIT_TEST(testSaveProcessingOptions_XRS_EPR_NoError);
  CPPUNIT_TEST(testSaveProcessingOptions_XRS_NOEPR_Error);

  CPPUNIT_TEST(testSaveProcessingOptions_XRSandPDR_Error);
  CPPUNIT_TEST(testSaveProcessingOptions_XRSandPDO_Error);
  CPPUNIT_TEST(testMultipleInclusionCodes_AB_True);
  CPPUNIT_TEST(testMultipleInclusionCodes_LIST_True);
  CPPUNIT_TEST(testMultipleInclusionCodesThrowsExceptionWhenDuplicateCabinEntered);
  CPPUNIT_TEST(testMultipleInclusionCodesThrowsExceptionWhen_AB_AndOtherCabinEntered);
  CPPUNIT_TEST(testMultipleInclusionCodesThrowsExceptionWhenInvalidCabinEntered);

  CPPUNIT_TEST(testStoreProcessingOptions_PRM_RCQ_NoError);

  CPPUNIT_TEST_SUITE_END();

private:
  DataHandle _dataHandle;
  FareDisplayModelMap* _modelMap;
  Trx* _baseTrx;
  FareDisplayTrx* _trx;
  FareDisplayRequest* _request;
  FareDisplayOptions* _options;
  ERDFareComp* _erdFareComp;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    // Create trx objects
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<FareDisplayTrx>();
    _request = _memHandle.create<FareDisplayRequest>();
    _options = _memHandle.create<FareDisplayOptions>();
    _trx->setRequest(_request);
    _trx->setOptions(_options);
    _baseTrx = _trx;
    _erdFareComp = _memHandle.create<ERDFareComp>();

    // Create modelmap with fake config and trx
    tse::ConfigMan enhancedRDConfig;
    _modelMap = _memHandle.insert<FareDisplayModelMap>(
        new FareDisplayModelMap(enhancedRDConfig, _dataHandle, _baseTrx));

    xercesc::XMLPlatformUtils::Initialize();
  }

  void tearDown() { _memHandle.clear(); }

  void testTrxValid()
  {
    MockXercescAttributes attrs;

    _modelMap->storeFareDisplayInformation(attrs);

    CPPUNIT_ASSERT(_modelMap->_fareDisplayTrx != 0);
  }

  void testTrxOptions()
  {
    MockXercescAttributes attrs;

    _modelMap->storeFareDisplayInformation(attrs);

    CPPUNIT_ASSERT(_modelMap->_fareDisplayTrx->getOptions() != 0);
    CPPUNIT_ASSERT(_modelMap->_fareDisplayTrx->getRequest() != 0);
  }

  void testAgentValid()
  {
    MockXercescAttributes attrs;

    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->storeAgentInformation(attrs);

    CPPUNIT_ASSERT(_modelMap->_fareDisplayTrx->getRequest()->ticketingAgent() != 0);
  }

  void testProcessingOptionsParsing()
  {
    FareDisplayModelMap::Mapping map = {0, 0};

    map.members[_modelMap->SDBMHash("Q0C")] = 86;
    map.members[_modelMap->SDBMHash("N0F")] = 87;

    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;

    attrs.add("Q0C", "1/2/3");
    attrs.add("N0F", "2/3");

    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->storeProcessingOptions(attrs);

    FareDisplayOptions* options = _modelMap->_fareDisplayTrx->getOptions();

    CPPUNIT_ASSERT(options->requestedSegments().size() == 3);
    CPPUNIT_ASSERT(options->requestedSegments()[0] == 1);
    CPPUNIT_ASSERT(options->requestedSegments()[1] == 2);
    CPPUNIT_ASSERT(options->requestedSegments()[2] == 3);
    CPPUNIT_ASSERT(options->surfaceSegments().size() == 2);
    CPPUNIT_ASSERT(options->surfaceSegments()[0] == 2);
    CPPUNIT_ASSERT(options->surfaceSegments()[1] == 3);
  }

  void testFareDisplayInfoParsing()
  {
    FareDisplayModelMap::Mapping map = {0, 0};

    map.members[_modelMap->SDBMHash("A01")] = 1;
    map.members[_modelMap->SDBMHash("A02")] = 2;
    map.members[_modelMap->SDBMHash("D06")] = 3;
    map.members[_modelMap->SDBMHash("D01")] = 4;
    map.members[_modelMap->SDBMHash("BI0")] = 5;
    map.members[_modelMap->SDBMHash("Q46")] = 7;
    map.members[_modelMap->SDBMHash("PBB")] = 8;
    map.members[_modelMap->SDBMHash("NOP")] = 9;
    map.members[_modelMap->SDBMHash("S82")] = 10;
    map.members[_modelMap->SDBMHash("D54")] = 11;

    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;

    attrs.add("A01", "PAR");
    attrs.add("A02", "EZE");
    attrs.add("D06", "2008-10-19");
    attrs.add("D01", "2008-09-08");
    attrs.add("BI0", "NLX");
    attrs.add("Q46", "01");
    attrs.add("PBB", "F");
    attrs.add("NOP", "S");
    attrs.add("D54", "0922");

    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->storeFareDisplayInformation(attrs);

    AirSeg* airseg = _modelMap->_airSeg;
    FareDisplayOptions* options = _modelMap->_fareDisplayTrx->getOptions();
    FareDisplayRequest* request = _modelMap->_fareDisplayTrx->getRequest();
    DateTime ticketingDT(2008, 10, 19, 15, 22, 0);
    DateTime departureDT(2008, 9, 8);

    CPPUNIT_ASSERT(airseg->origAirport() == "PAR");
    CPPUNIT_ASSERT(airseg->destAirport() == "EZE");
    CPPUNIT_ASSERT(request->ticketingDT() == ticketingDT);
    CPPUNIT_ASSERT(airseg->departureDT() == departureDT);
    CPPUNIT_ASSERT(request->inclusionCode() == "NLX");
    CPPUNIT_ASSERT(request->inclusionCode() == "NLX");
    CPPUNIT_ASSERT(request->carrierNotEntered() == 'F');
    CPPUNIT_ASSERT(options->templateType() == 'S');
    CPPUNIT_ASSERT(options->lineNumber() == 1);
    CPPUNIT_ASSERT(options->ticketTimeOverride() == 922);
  }

  void testFareDisplayFQRequestType()
  {
    FareDisplayModelMap::Mapping map = {0, 0};

    map.members[_modelMap->SDBMHash("S58")] = 6;

    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;

    attrs.add("S58", FARE_DISPLAY_REQUEST);

    _modelMap->storeFareDisplayInformation(attrs);

    FareDisplayRequest* request = _modelMap->_fareDisplayTrx->getRequest();

    CPPUNIT_ASSERT(request->requestType() == FARE_DISPLAY_REQUEST);
  }

  void testFareDisplayRDRequestType()
  {
    FareDisplayModelMap::Mapping map = {0, 0};

    map.members[_modelMap->SDBMHash("S58")] = 6;

    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;

    attrs.add("S58", FARE_RULES_REQUEST);

    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->storeFareDisplayInformation(attrs);

    FareDisplayRequest* request = _modelMap->_fareDisplayTrx->getRequest();

    CPPUNIT_ASSERT(request->requestType() == FARE_RULES_REQUEST);
  }

  void testFareDisplayERDRequestType()
  {
    FareDisplayModelMap::Mapping map = {0, 0};

    map.members[_modelMap->SDBMHash("S58")] = 6;

    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;

    attrs.add("S58", ENHANCED_RD_REQUEST);

    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->storeFareDisplayInformation(attrs);

    FareDisplayRequest* request = _modelMap->_fareDisplayTrx->getRequest();

    CPPUNIT_ASSERT(request->requestType() == ENHANCED_RD_REQUEST);
  }

  void testAgentDataParsing()
  {
    FareDisplayModelMap::Mapping map = {0, 0};

    map.members[_modelMap->SDBMHash("A10")] = 1;
    map.members[_modelMap->SDBMHash("A20")] = 2;
    map.members[_modelMap->SDBMHash("A21")] = 3;
    map.members[_modelMap->SDBMHash("AB0")] = 4;
    map.members[_modelMap->SDBMHash("AB1")] = 5;
    map.members[_modelMap->SDBMHash("A90")] = 6;
    map.members[_modelMap->SDBMHash("N0G")] = 7;
    map.members[_modelMap->SDBMHash("A80")] = 8;
    map.members[_modelMap->SDBMHash("B00")] = 9;
    map.members[_modelMap->SDBMHash("C40")] = 10;
    map.members[_modelMap->SDBMHash("Q01")] = 11;
    map.members[_modelMap->SDBMHash("N0L")] = 12;
    map.members[_modelMap->SDBMHash("C6C")] = 13;
    map.members[_modelMap->SDBMHash("AE1")] = 14;
    map.members[_modelMap->SDBMHash("AE0")] = 15;

    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;

    attrs.add("A10", "KRK");
    attrs.add("A20", "HDQ");
    attrs.add("A21", "HDQM");
    attrs.add("AB0", "2025448");
    attrs.add("AB1", "2025448");
    attrs.add("A90", "5000");
    attrs.add("N0G", "*");
    attrs.add("A80", "HDQ");
    attrs.add("B00", "LO");
    attrs.add("C40", "PLN");
    attrs.add("N0L", "1");
    attrs.add("AE1", "OD");
    attrs.add("AE0", "AM");

    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->storeAgentInformation(attrs);

    Agent* agent = _modelMap->_fareDisplayTrx->getRequest()->ticketingAgent();

    CPPUNIT_ASSERT(agent->agentCity() == "KRK");
    CPPUNIT_ASSERT(agent->tvlAgencyPCC() == "HDQ");
    CPPUNIT_ASSERT(agent->mainTvlAgencyPCC() == "HDQM");
    CPPUNIT_ASSERT(agent->tvlAgencyIATA() == "2025448");
    CPPUNIT_ASSERT(agent->homeAgencyIATA() == "2025448");
    CPPUNIT_ASSERT(agent->agentFunctions() == "5000");
    CPPUNIT_ASSERT(agent->agentDuty() == "*");
    CPPUNIT_ASSERT(agent->airlineDept() == "HDQ");
    CPPUNIT_ASSERT(agent->cxrCode() == "LO");
    CPPUNIT_ASSERT(agent->currencyCodeAgent() == "PLN");
    CPPUNIT_ASSERT(agent->agentCommissionType() == "1");
    CPPUNIT_ASSERT(agent->officeDesignator() == "OD");
    CPPUNIT_ASSERT(agent->hostCarrier() == "AM");
  }

  void testBillingDataParsing()
  {
    FareDisplayModelMap::Mapping map = {0, 0};

    map.members[_modelMap->SDBMHash("A20")] = 1;
    map.members[_modelMap->SDBMHash("Q03")] = 2;
    map.members[_modelMap->SDBMHash("Q02")] = 3;
    map.members[_modelMap->SDBMHash("AE0")] = 4;
    map.members[_modelMap->SDBMHash("AD0")] = 5;
    map.members[_modelMap->SDBMHash("C20")] = 6;
    map.members[_modelMap->SDBMHash("A22")] = 7;
    map.members[_modelMap->SDBMHash("AA0")] = 8;
    map.members[_modelMap->SDBMHash("A70")] = 9;
    map.members[_modelMap->SDBMHash("C01")] = 10;
    map.members[_modelMap->SDBMHash("C00")] = 11;
    map.members[_modelMap->SDBMHash("C21")] = 12;
    map.members[_modelMap->SDBMHash("S0R")] = 13;

    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;

    attrs.add("A20", "KRK");
    attrs.add("Q03", "5642");
    attrs.add("Q02", "6148");
    attrs.add("AE0", "AA");
    attrs.add("AD0", "3B08C3");
    attrs.add("C20", "BKGCDSP1");
    attrs.add("A22", "TP42");
    attrs.add("AA0", "-A8");
    attrs.add("A70", "RB1JJ");
    attrs.add("S0R", PSS_PO_ATSE_PATH);

    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->storeBillingInformation(attrs);

    Billing* billing = _modelMap->_fareDisplayTrx->billing();

    CPPUNIT_ASSERT(billing->userPseudoCityCode() == "KRK");
    CPPUNIT_ASSERT(billing->userStation() == "5642");
    CPPUNIT_ASSERT(billing->userBranch() == "6148");
    CPPUNIT_ASSERT(billing->partitionID() == "AA");
    CPPUNIT_ASSERT(billing->userSetAddress() == "3B08C3");
    CPPUNIT_ASSERT(billing->parentServiceName() == "BKGCDSP1");
    CPPUNIT_ASSERT(billing->aaaCity() == "TP42");
    CPPUNIT_ASSERT(billing->aaaSine() == "-A8");
    CPPUNIT_ASSERT(billing->actionCode() == "RB1JJ");
    CPPUNIT_ASSERT(billing->requestPath() == PSS_PO_ATSE_PATH);
  }

  void testCommonBillingDataParsing()
  {
    FareDisplayModelMap::Mapping map = {0, 0};

    map.members[_modelMap->SDBMHash("A20")] = 1;
    map.members[_modelMap->SDBMHash("Q03")] = 2;
    map.members[_modelMap->SDBMHash("Q02")] = 3;
    map.members[_modelMap->SDBMHash("AE0")] = 4;
    map.members[_modelMap->SDBMHash("AD0")] = 5;
    map.members[_modelMap->SDBMHash("A22")] = 7;
    map.members[_modelMap->SDBMHash("AA0")] = 8;
    map.members[_modelMap->SDBMHash("A70")] = 9;

    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;

    attrs.add("A20", "DFW");
    attrs.add("Q03", "925");
    attrs.add("Q02", "2902");
    attrs.add("AE0", "AA");
    attrs.add("AD0", "547ED9");
    attrs.add("A22", "5KAD");
    attrs.add("AA0", "HSY");
    attrs.add("A70", "WPA$B");

    Billing* billing;
    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->_fareDisplayTrx->dataHandle().get(billing);
    _modelMap->_fareDisplayTrx->billing() = billing;

    int numAtts = attrs.getLength();
    for (int i = 0; i < numAtts; i++)
    {
      _modelMap->storeCommonBillingInformation(i, attrs, *billing);
    }

    CPPUNIT_ASSERT(billing->userPseudoCityCode() == "DFW");
    CPPUNIT_ASSERT(billing->userStation() == "925");
    CPPUNIT_ASSERT(billing->userBranch() == "2902");
    CPPUNIT_ASSERT(billing->partitionID() == "AA");
    CPPUNIT_ASSERT(billing->userSetAddress() == "547ED9");
    CPPUNIT_ASSERT(billing->aaaCity() == "5KAD");
    CPPUNIT_ASSERT(billing->aaaSine() == "HSY");
    CPPUNIT_ASSERT(billing->actionCode() == "WPA$B");
  }

  void testDiagDataParsing()
  {
    FareDisplayModelMap::Mapping map = {0, 0};

    map.members[_modelMap->SDBMHash("QOA")] = 1;
    map.members[_modelMap->SDBMHash("S01")] = 2;

    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;

    attrs.add("QOA", "TestType");
    attrs.add("S01", "TestData");

    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->storeDiagInformation(attrs);

    FareDisplayRequest* request = _modelMap->_fareDisplayTrx->getRequest();

    CPPUNIT_ASSERT(request->diagArgType()[0] == "TestType");
    CPPUNIT_ASSERT(request->diagArgData()[0] == "TestData");
  }

  void testPassengerDataParsing()
  {
    FareDisplayModelMap::Mapping map = {0, 0};
    map.members[_modelMap->SDBMHash("B70")] = 1;
    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;
    attrs.add("B70", "ADT");

    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->storePassengerInformation(attrs);

    FareDisplayRequest* request = _modelMap->_fareDisplayTrx->getRequest();

    CPPUNIT_ASSERT(request->displayPassengerTypes()[0] == "ADT");
    CPPUNIT_ASSERT(request->inputPassengerTypes()[0] == "ADT");
  }

  void testFareComponentDataParsing()
  {
    FareDisplayModelMap::Mapping map = {0, 0};

    map.members[_modelMap->SDBMHash("B08")] = 1;
    map.members[_modelMap->SDBMHash("B02")] = 2;
    map.members[_modelMap->SDBMHash("A11")] = 3;
    map.members[_modelMap->SDBMHash("A12")] = 4;
    map.members[_modelMap->SDBMHash("A01")] = 5;
    map.members[_modelMap->SDBMHash("A02")] = 6;
    map.members[_modelMap->SDBMHash("B50")] = 7;
    map.members[_modelMap->SDBMHash("C40")] = 8;
    map.members[_modelMap->SDBMHash("Q4J")] = 9;
    map.members[_modelMap->SDBMHash("P04")] = 10;
    map.members[_modelMap->SDBMHash("P05")] = 11;
    map.members[_modelMap->SDBMHash("S70")] = 12;
    map.members[_modelMap->SDBMHash("AC0")] = 13;
    map.members[_modelMap->SDBMHash("C50")] = 14;

    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;

    attrs.add("B08", "LO");
    attrs.add("B02", "YY");
    attrs.add("A11", "KRK");
    attrs.add("A12", "DFW");
    attrs.add("A01", "KRK");
    attrs.add("A02", "DFW");
    attrs.add("B50", "M3C75N");
    attrs.add("C40", "PLN");
    attrs.add("Q4J", "1");
    attrs.add("P04", "T");
    attrs.add("P05", "F");
    attrs.add("S70", "FROM");
    attrs.add("AC0", "AC123");
    attrs.add("C50", "123.45");

    _trx->getRequest()->requestType() = ENHANCED_RD_REQUEST;
    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->storeCALInformation(attrs);
    _modelMap->saveCALInformation();

    ERDFareComp* erdFareComp = _trx->getOptions()->erdFareComps()[0];

    CPPUNIT_ASSERT(erdFareComp->trueGoverningCarrier() == "LO");
    CPPUNIT_ASSERT(erdFareComp->governingCarrier() == "YY");
    CPPUNIT_ASSERT(erdFareComp->departureCity() == "KRK");
    CPPUNIT_ASSERT(erdFareComp->arrivalCity() == "DFW");
    CPPUNIT_ASSERT(erdFareComp->departureAirport() == "KRK");
    CPPUNIT_ASSERT(erdFareComp->arrivalAirport() == "DFW");
    CPPUNIT_ASSERT(erdFareComp->fareBasis() == "M3C75N");
    CPPUNIT_ASSERT(erdFareComp->fareCurrency() == "PLN");
    CPPUNIT_ASSERT(erdFareComp->pricingUnitNumber() == 1);
    CPPUNIT_ASSERT(erdFareComp->oneWayFare() == 'T');
    CPPUNIT_ASSERT(erdFareComp->roundTripFare() == 'F');
    CPPUNIT_ASSERT(erdFareComp->directionality() == "FROM");
    CPPUNIT_ASSERT(erdFareComp->accountCode() == "AC123");
    CPPUNIT_ASSERT(erdFareComp->nucFareAmount() == 123.45);
  }

  void testSegmentDataParsing()
  {
    FareDisplayModelMap::Mapping map = {0, 0};

    map.members[_modelMap->SDBMHash("Q0Z")] = 1;
    map.members[_modelMap->SDBMHash("S10")] = 2;
    map.members[_modelMap->SDBMHash("C6I")] = 3;
    map.members[_modelMap->SDBMHash("A02")] = 4;
    map.members[_modelMap->SDBMHash("A11")] = 5;
    map.members[_modelMap->SDBMHash("A12")] = 6;
    map.members[_modelMap->SDBMHash("S37")] = 7;
    map.members[_modelMap->SDBMHash("S89")] = 8;
    map.members[_modelMap->SDBMHash("S90")] = 9;
    map.members[_modelMap->SDBMHash("P2F")] = 10;
    map.members[_modelMap->SDBMHash("C13")] = 11;

    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;

    attrs.add("Q0Z", "1");
    attrs.add("C6I", "KRK");
    attrs.add("A02", "DFW");
    attrs.add("A11", "KRK");
    attrs.add("A12", "DFW");
    attrs.add("S37", "ATP");
    attrs.add("S89", "100");
    attrs.add("S90", "200");
    attrs.add("P2F", "T");
    attrs.add("C13", "I");

    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->_erdFareComp = _erdFareComp;
    _trx->getRequest()->requestType() = ENHANCED_RD_REQUEST;
    _modelMap->storeSEGInformation(attrs);
    _modelMap->saveSEGInformation();

    ERDFltSeg* erdFltSeg = _erdFareComp->segments().front();

    CPPUNIT_ASSERT(erdFltSeg);
    CPPUNIT_ASSERT(erdFltSeg->itinSegNumber() == 1);
    CPPUNIT_ASSERT(erdFltSeg->departureAirport() == "KRK");
    CPPUNIT_ASSERT(erdFltSeg->arrivalAirport() == "DFW");
    CPPUNIT_ASSERT(erdFltSeg->departureCity() == "KRK");
    CPPUNIT_ASSERT(erdFltSeg->arrivalCity() == "DFW");
    CPPUNIT_ASSERT(erdFltSeg->geoTravelType() == GeoTravelType::International);
    CPPUNIT_ASSERT(_erdFareComp->vendor() == "ATP");
    CPPUNIT_ASSERT(_erdFareComp->fareTariff() == 100);
    CPPUNIT_ASSERT(_erdFareComp->ruleNumber() == "200");
  }

  void testFQCat25DataParsing()
  {
    FareDisplayModelMap::Mapping map = {0, 0};

    map.members[_modelMap->SDBMHash("S37")] = 1;
    map.members[_modelMap->SDBMHash("Q41")] = 2;

    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;

    attrs.add("S37", "ATP");
    attrs.add("Q41", "1200");

    _trx->setOptions(_options);
    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->_erdFareComp = _erdFareComp;

    _modelMap->storeCAT25Information(attrs);

    CPPUNIT_ASSERT(_options->cat25Values().vendorCode() == "ATP");
    CPPUNIT_ASSERT(_options->cat25Values().itemNo() == 1200);
  }

  void testERDCat25DataParsing()
  {
    FareDisplayModelMap::Mapping map = {0, 0};

    map.members[_modelMap->SDBMHash("S37")] = 1;
    map.members[_modelMap->SDBMHash("Q41")] = 2;

    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;

    attrs.add("S37", "ATP");
    attrs.add("Q41", "1200");

    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->_erdFareComp = _erdFareComp;
    _trx->getRequest()->requestType() = ENHANCED_RD_REQUEST;

    _modelMap->storeCAT25Information(attrs);

    CPPUNIT_ASSERT(_erdFareComp->cat25Values().vendorCode() == "ATP");
    CPPUNIT_ASSERT(_erdFareComp->cat25Values().itemNo() == 1200);
  }

  void testFQCat35DataParsing()
  {
    FareDisplayModelMap::Mapping map = {0, 0};

    map.members[_modelMap->SDBMHash("S37")] = 1;
    map.members[_modelMap->SDBMHash("Q41")] = 2;

    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;

    attrs.add("S37", "ATP");
    attrs.add("Q41", "1200");

    _trx->setOptions(_options);
    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->_erdFareComp = _erdFareComp;

    _modelMap->storeCAT35Information(attrs);

    CPPUNIT_ASSERT(_options->cat35Values().vendorCode() == "ATP");
    CPPUNIT_ASSERT(_options->cat35Values().itemNo() == 1200);
  }

  void testERDCat35DataParsing()
  {
    FareDisplayModelMap::Mapping map = {0, 0};

    map.members[_modelMap->SDBMHash("S37")] = 1;
    map.members[_modelMap->SDBMHash("Q41")] = 2;

    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;

    attrs.add("S37", "ATP");
    attrs.add("Q41", "1200");

    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->_erdFareComp = _erdFareComp;
    _trx->getRequest()->requestType() = ENHANCED_RD_REQUEST;

    _modelMap->storeCAT35Information(attrs);

    CPPUNIT_ASSERT(_erdFareComp->cat35Values().vendorCode() == "ATP");
    CPPUNIT_ASSERT(_erdFareComp->cat35Values().itemNo() == 1200);
  }

  void testFQDiscountedDataParsing()
  {
    FareDisplayModelMap::Mapping map = {0, 0};

    map.members[_modelMap->SDBMHash("S37")] = 1;
    map.members[_modelMap->SDBMHash("Q41")] = 2;

    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;

    attrs.add("S37", "ATP");
    attrs.add("Q41", "1200");

    _trx->setOptions(_options);
    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->_erdFareComp = _erdFareComp;

    _modelMap->storeDFIInformation(attrs);

    CPPUNIT_ASSERT(_options->discountedValues().vendorCode() == "ATP");
    CPPUNIT_ASSERT(_options->discountedValues().itemNo() == 1200);
  }

  void testERDDiscountedDataParsing()
  {
    FareDisplayModelMap::Mapping map = {0, 0};

    map.members[_modelMap->SDBMHash("S37")] = 1;
    map.members[_modelMap->SDBMHash("Q41")] = 2;

    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;

    attrs.add("S37", "ATP");
    attrs.add("Q41", "1200");

    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->_erdFareComp = _erdFareComp;
    _trx->getRequest()->requestType() = ENHANCED_RD_REQUEST;

    _modelMap->storeDFIInformation(attrs);

    CPPUNIT_ASSERT(_erdFareComp->discountedValues().vendorCode() == "ATP");
    CPPUNIT_ASSERT(_erdFareComp->discountedValues().itemNo() == 1200);
  }

  void testERDDataParsing()
  {
    FareDisplayModelMap::Mapping map = {0, 0};

    map.members[_modelMap->SDBMHash("Q46")] = 1;
    map.members[_modelMap->SDBMHash("Q1K")] = 2;
    map.members[_modelMap->SDBMHash("D12")] = 3;
    map.members[_modelMap->SDBMHash("D55")] = 4;
    map.members[_modelMap->SDBMHash("S53")] = 5;
    map.members[_modelMap->SDBMHash("BJ0")] = 6;
    map.members[_modelMap->SDBMHash("N1P")] = 7;
    map.members[_modelMap->SDBMHash("C5A")] = 8;
    map.members[_modelMap->SDBMHash("P72")] = 9;
    map.members[_modelMap->SDBMHash("B50")] = 10;
    map.members[_modelMap->SDBMHash("BE0")] = 11;
    map.members[_modelMap->SDBMHash("AC0")] = 12;
    map.members[_modelMap->SDBMHash("D08")] = 13;
    map.members[_modelMap->SDBMHash("B00")] = 14;

    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;

    attrs.add("Q46", "123");
    attrs.add("Q1K", "321");
    attrs.add("D12", "2008-11-10");
    attrs.add("D55", "09-28-00");
    attrs.add("S53", "A");
    attrs.add("BJ0", "VTEST");
    attrs.add("N1P", "X");
    attrs.add("C5A", "3745.12345");
    attrs.add("P72", "C");
    attrs.add("B50", "VTEST/A");
    attrs.add("BE0", "TEST");
    attrs.add("AC0", "CORPID");
    attrs.add("D08", "2008-11-04");
    attrs.add("B00", "LO");

    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->_erdFareComp = _erdFareComp;
    _trx->getRequest()->requestType() = ENHANCED_RD_REQUEST;

    _modelMap->storeERDInformation(attrs);

    DateTime createDate(2008, 11, 10);
    DateTime departureDT(2008, 11, 4);

    CPPUNIT_ASSERT(_erdFareComp->linkNumber() == 123);
    CPPUNIT_ASSERT(_erdFareComp->sequenceNumber() == 321);
    CPPUNIT_ASSERT(_erdFareComp->createDate() == createDate);
    CPPUNIT_ASSERT(_erdFareComp->createTime() == "09:28:00");
    CPPUNIT_ASSERT(_erdFareComp->fareType() == "A");
    CPPUNIT_ASSERT(_erdFareComp->fareClass() == "VTEST");
    CPPUNIT_ASSERT(_erdFareComp->cat35Type() == "X");
    CPPUNIT_ASSERT(_erdFareComp->fareAmount() == 3745.12345);
    CPPUNIT_ASSERT(_erdFareComp->bookingCode() == "C");
    CPPUNIT_ASSERT(_erdFareComp->fareBasis() == "VTEST/A");
    CPPUNIT_ASSERT(_erdFareComp->ticketDesignator() == "TEST");
    CPPUNIT_ASSERT(_erdFareComp->accountCode() == "CORPID");
    CPPUNIT_ASSERT(_erdFareComp->departureDT() == departureDT);
    CPPUNIT_ASSERT(_erdFareComp->validatingCarrier() == "LO");
  }

  void testStoreProcessingOptions_MAC1()
  {
    FareDisplayModelMap::Mapping map = {0, 0};

    map.members[_modelMap->SDBMHash("SM1")] = 81;

    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;

    attrs.add("SM1", "ACCNTCODE01");

    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->storeProcessingOptions(attrs);

    CPPUNIT_ASSERT(_trx->getRequest()->incorrectCorpIdVec().size() == 0);
    CPPUNIT_ASSERT(_trx->getRequest()->corpIdVec().size() == 0);
    CPPUNIT_ASSERT(_trx->getRequest()->accCodeVec().size() == 1);

    CPPUNIT_ASSERT(_trx->getRequest()->accCodeVec()[0] == "ACCNTCODE01");
  }

  void testSaveProcessingOptions_MAC1()
  {
    FareDisplayModelMap::Mapping map = {0, 0};

    map.members[_modelMap->SDBMHash("SM1")] = 81;

    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;

    attrs.add("SM1", "ACCNTCODE01");

    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->storeProcessingOptions(attrs);
    _modelMap->saveProcessingOptions();

    CPPUNIT_ASSERT(_trx->getRequest()->isMultiAccCorpId() == true);
  }

  void testStoreProcessingOptions_MAC2()
  {
    FareDisplayModelMap::Mapping map = {0, 0};

    map.members[_modelMap->SDBMHash("S07")] = 21;

    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;

    attrs.add("S07", "ACCNTCODE01");

    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->storeProcessingOptions(attrs);

    CPPUNIT_ASSERT(_trx->getRequest()->incorrectCorpIdVec().size() == 0);
    CPPUNIT_ASSERT(_trx->getRequest()->corpIdVec().size() == 0);
    CPPUNIT_ASSERT(_trx->getRequest()->accCodeVec().size() == 0);

    CPPUNIT_ASSERT(_trx->getRequest()->accountCode() == "ACCNTCODE01");
  }

  void testSaveProcessingOptions_MAC2()
  {
    FareDisplayModelMap::Mapping map = {0, 0};

    map.members[_modelMap->SDBMHash("S07")] = 21;

    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;

    attrs.add("S07", "ACCNTCODE01");

    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->storeProcessingOptions(attrs);
    _modelMap->saveProcessingOptions();

    CPPUNIT_ASSERT(_trx->getRequest()->isMultiAccCorpId() == false);
  }

  void testStoreProcessingOptions_MAC3()
  {
    FareDisplayModelMap::Mapping map = {0, 0};

    map.members[_modelMap->SDBMHash("S07")] = 21;
    map.members[_modelMap->SDBMHash("SM1")] = 81;

    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;

    attrs.add("S07", "ACCNTCODE01");
    attrs.add("SM1", "ACCNTCODE01");

    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->storeProcessingOptions(attrs);

    CPPUNIT_ASSERT(_trx->getRequest()->incorrectCorpIdVec().size() == 0);
    CPPUNIT_ASSERT(_trx->getRequest()->corpIdVec().size() == 0);
    CPPUNIT_ASSERT(_trx->getRequest()->accCodeVec().size() == 1);

    CPPUNIT_ASSERT(_trx->getRequest()->accCodeVec()[0] == "ACCNTCODE01");
  }

  void testSaveProcessingOptions_MAC3()
  {
    FareDisplayModelMap::Mapping map = {0, 0};

    map.members[_modelMap->SDBMHash("S07")] = 21;
    map.members[_modelMap->SDBMHash("SM1")] = 81;

    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;

    attrs.add("S07", "ACCNTCODE01");
    attrs.add("SM1", "ACCNTCODE01");

    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->storeProcessingOptions(attrs);
    _modelMap->saveProcessingOptions();

    CPPUNIT_ASSERT(_trx->getRequest()->isMultiAccCorpId() == true);
  }

  void testStoreProcessingOptions_MAC4()
  {
    FareDisplayModelMap::Mapping map = {0, 0};

    map.members[_modelMap->SDBMHash("SM1")] = 81;
    map.members[_modelMap->SDBMHash("SM2")] = 82;
    map.members[_modelMap->SDBMHash("SM3")] = 83;
    map.members[_modelMap->SDBMHash("SM4")] = 84;

    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;

    attrs.add("SM1", "ACCNTCODE01");
    attrs.add("SM2", "ACCNTCODE02");
    attrs.add("SM3", "ACCNTCODE03");
    attrs.add("SM4", "ACCNTCODE04");

    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->storeProcessingOptions(attrs);

    CPPUNIT_ASSERT(_trx->getRequest()->incorrectCorpIdVec().size() == 0);
    CPPUNIT_ASSERT(_trx->getRequest()->corpIdVec().size() == 0);
    CPPUNIT_ASSERT(_trx->getRequest()->accCodeVec().size() == 4);

    CPPUNIT_ASSERT(_trx->getRequest()->accCodeVec()[0] == "ACCNTCODE01");
    CPPUNIT_ASSERT(_trx->getRequest()->accCodeVec()[1] == "ACCNTCODE02");
    CPPUNIT_ASSERT(_trx->getRequest()->accCodeVec()[2] == "ACCNTCODE03");
    CPPUNIT_ASSERT(_trx->getRequest()->accCodeVec()[3] == "ACCNTCODE04");
  }

  void testSaveProcessingOptions_MAC4()
  {
    FareDisplayModelMap::Mapping map = {0, 0};

    map.members[_modelMap->SDBMHash("SM1")] = 81;
    map.members[_modelMap->SDBMHash("SM2")] = 82;
    map.members[_modelMap->SDBMHash("SM3")] = 83;
    map.members[_modelMap->SDBMHash("SM4")] = 84;

    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;

    attrs.add("SM1", "ACCNTCODE01");
    attrs.add("SM2", "ACCNTCODE02");
    attrs.add("SM3", "ACCNTCODE03");
    attrs.add("SM4", "ACCNTCODE04");

    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->storeProcessingOptions(attrs);
    _modelMap->saveProcessingOptions();

    CPPUNIT_ASSERT(_trx->getRequest()->isMultiAccCorpId() == true);
  }

  void testStoreProcessingOptions_MAC5()
  {
    FareDisplayModelMap::Mapping map = {0, 0};

    map.members[_modelMap->SDBMHash("AD1")] = 85;

    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;

    attrs.add("AD1", "T");

    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->storeProcessingOptions(attrs);

    CPPUNIT_ASSERT(_trx->getRequest()->displayAccCode() == true);
  }

  void testStoreProcessingOptions_MAC6()
  {
    FareDisplayModelMap::Mapping map = {0, 0};

    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;

    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->storeProcessingOptions(attrs);
    _modelMap->saveProcessingOptions();

    CPPUNIT_ASSERT(_trx->getRequest()->displayAccCode() == false);
  }

  void testStoreProcessingOptions_XFF()
  {
    FareDisplayModelMap::Mapping map = {0, 0};
    map.members[_modelMap->SDBMHash("XFF")] = 93;
    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;
    attrs.add("XFF", "T");

    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->storeProcessingOptions(attrs);

    CPPUNIT_ASSERT(_trx->getOptions()->isExcludeFareFocusRule() == true);
  }

  void testSaveProcessingOptions_XFF_EPR_NoError()
  {
    FareDisplayModelMap::Mapping map = {0, 0};
    map.members[_modelMap->SDBMHash("S15")] = 92;
    map.members[_modelMap->SDBMHash("XFF")] = 93;
    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;
    attrs.add("S15", "TEST,FFOCUS,TEST2");
    attrs.add("XFF", "T");

    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->storeProcessingOptions(attrs);
    CPPUNIT_ASSERT_NO_THROW(_modelMap->saveProcessingOptions());
  }

  void testSaveProcessingOptions_XFF_NOEPR_Error()
  {
    FareDisplayModelMap::Mapping map = {0, 0};
    map.members[_modelMap->SDBMHash("S15")] = 92;
    map.members[_modelMap->SDBMHash("XFF")] = 93;
    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;
    attrs.add("S15", "TEST,MISSING,TEST2");
    attrs.add("XFF", "T");

    _modelMap->_fareDisplayTrx = _trx;
    _modelMap->storeProcessingOptions(attrs);
    CPPUNIT_ASSERT_THROW(_modelMap->saveProcessingOptions(), ErrorResponseException);
  }

void testStoreProcessingOptions_PDO()
{
  FareDisplayModelMap::Mapping map = {0, 0};
  map.members[_modelMap->SDBMHash("PDO")] = 94;
  _modelMap->_currentMapEntry = (void*)&map;
  MockXercescAttributes attrs;
  attrs.add("PDO", "T");
  _modelMap->_fareDisplayTrx = _trx;
  _modelMap->storeProcessingOptions(attrs);
  CPPUNIT_ASSERT(_trx->getOptions()->isPDOForFRRule() == true);
 }

void testSaveProcessingOptions_PDO_EPR_NoError()
{
  FareDisplayModelMap::Mapping map = {0, 0};
  map.members[_modelMap->SDBMHash("S15")] = 92;
  map.members[_modelMap->SDBMHash("PDO")] = 94;
  _modelMap->_currentMapEntry = (void*)&map;
  MockXercescAttributes attrs;
  attrs.add("S15", "TEST,ORGFQD,TEST2");
  attrs.add("PDO", "T");
  _modelMap->_fareDisplayTrx = _trx;
  _modelMap->storeProcessingOptions(attrs);
  CPPUNIT_ASSERT_NO_THROW(_modelMap->saveProcessingOptions());
 }

void testSaveProcessingOptions_PDO_NOEPR_Error()
{
  FareDisplayModelMap::Mapping map = {0, 0};
  map.members[_modelMap->SDBMHash("S15")] = 92;
  map.members[_modelMap->SDBMHash("PDO")] = 94;
  _modelMap->_currentMapEntry = (void*)&map;

  MockXercescAttributes attrs;
  attrs.add("S15", "TEST,MISSING,TEST2");
  attrs.add("PDO", "T");

  _modelMap->_fareDisplayTrx = _trx;
  _modelMap->storeProcessingOptions(attrs);
  CPPUNIT_ASSERT_THROW(_modelMap->saveProcessingOptions(), ErrorResponseException);
}

void testStoreProcessingOptions_PDR()
{
  FareDisplayModelMap::Mapping map = {0, 0};
  map.members[_modelMap->SDBMHash("PDR")] = 95;
  _modelMap->_currentMapEntry = (void*)&map;
  MockXercescAttributes attrs;
  attrs.add("PDR", "T");
  _modelMap->_fareDisplayTrx = _trx;
  _modelMap->storeProcessingOptions(attrs);
  CPPUNIT_ASSERT(_trx->getOptions()->isPDRForFRRule() == true);
 }

void testSaveProcessingOptions_PDR_EPR_NoError()
{
  FareDisplayModelMap::Mapping map = {0, 0};
  map.members[_modelMap->SDBMHash("S15")] = 92;
  map.members[_modelMap->SDBMHash("PDR")] = 95;
  _modelMap->_currentMapEntry = (void*)&map;
  MockXercescAttributes attrs;
  attrs.add("S15", "TEST,AGYRET,TEST2");
  attrs.add("PDR", "T");
  _modelMap->_fareDisplayTrx = _trx;
  _modelMap->storeProcessingOptions(attrs);
  CPPUNIT_ASSERT_NO_THROW(_modelMap->saveProcessingOptions());
 }

void testSaveProcessingOptions_PDR_NOEPR_Error()
{
  FareDisplayModelMap::Mapping map = {0, 0};
  map.members[_modelMap->SDBMHash("S15")] = 92;
  map.members[_modelMap->SDBMHash("PDR")] = 95;
  _modelMap->_currentMapEntry = (void*)&map;

  MockXercescAttributes attrs;
  attrs.add("S15", "TEST,MISSING,TEST2");
  attrs.add("PDR", "T");

  _modelMap->_fareDisplayTrx = _trx;
  _modelMap->storeProcessingOptions(attrs);
  CPPUNIT_ASSERT_THROW(_modelMap->saveProcessingOptions(), ErrorResponseException);
}

void testStoreProcessingOptions_XRS()
{
  FareDisplayModelMap::Mapping map = {0, 0};
  map.members[_modelMap->SDBMHash("XRS")] = 96;
  _modelMap->_currentMapEntry = (void*)&map;
  MockXercescAttributes attrs;
  attrs.add("XRS", "T");
  _modelMap->_fareDisplayTrx = _trx;
  _modelMap->storeProcessingOptions(attrs);
  CPPUNIT_ASSERT(_trx->getOptions()->isXRSForFRRule() == true);
 }

void testSaveProcessingOptions_XRS_EPR_NoError()
{
  FareDisplayModelMap::Mapping map = {0, 0};
  map.members[_modelMap->SDBMHash("S15")] = 92;
  map.members[_modelMap->SDBMHash("XRS")] = 96;
  _modelMap->_currentMapEntry = (void*)&map;
  MockXercescAttributes attrs;
  attrs.add("S15", "TEST,ORGFQD,TEST2");
  attrs.add("XRS", "T");
  _modelMap->_fareDisplayTrx = _trx;
  _modelMap->storeProcessingOptions(attrs);
  CPPUNIT_ASSERT_NO_THROW(_modelMap->saveProcessingOptions());
 }

void testSaveProcessingOptions_XRS_NOEPR_Error()
{
  FareDisplayModelMap::Mapping map = {0, 0};
  map.members[_modelMap->SDBMHash("S15")] = 92;
  map.members[_modelMap->SDBMHash("XRS")] = 96;
  _modelMap->_currentMapEntry = (void*)&map;

  MockXercescAttributes attrs;
  attrs.add("S15", "TEST,MISSING,TEST2");
  attrs.add("XRS", "T");

  _modelMap->_fareDisplayTrx = _trx;
  _modelMap->storeProcessingOptions(attrs);
  CPPUNIT_ASSERT_THROW(_modelMap->saveProcessingOptions(), ErrorResponseException);
}

void testSaveProcessingOptions_XRSandPDR_Error()
{
  FareDisplayModelMap::Mapping map = {0, 0};
  map.members[_modelMap->SDBMHash("XRS")] = 96;
  map.members[_modelMap->SDBMHash("PDR")] = 95;
  _modelMap->_currentMapEntry = (void*)&map;

  MockXercescAttributes attrs;
  attrs.add("XRS", "T");
  attrs.add("PDR", "T");

  _modelMap->_fareDisplayTrx = _trx;
  _modelMap->storeProcessingOptions(attrs);
  CPPUNIT_ASSERT_THROW(_modelMap->saveProcessingOptions(), ErrorResponseException);
}

void testSaveProcessingOptions_XRSandPDO_Error()
{
  FareDisplayModelMap::Mapping map = {0, 0};
  map.members[_modelMap->SDBMHash("XRS")] = 96;
  map.members[_modelMap->SDBMHash("PDO")] = 94;
  _modelMap->_currentMapEntry = (void*)&map;

  MockXercescAttributes attrs;
  attrs.add("XRS", "T");
  attrs.add("PDR", "T");

  _modelMap->_fareDisplayTrx = _trx;
  _modelMap->storeProcessingOptions(attrs);
  CPPUNIT_ASSERT_THROW(_modelMap->saveProcessingOptions(), ErrorResponseException);
}

void testMultipleInclusionCodes_AB_True()
{
  FareDisplayModelMap::Mapping map = {0, 0};

  map.members[_modelMap->SDBMHash("BI0")] = 5;

  _modelMap->_currentMapEntry = (void*)&map;

  MockXercescAttributes attrs;

  attrs.add("BI0", "AB");
  _modelMap->_fareDisplayTrx = _trx;
  _modelMap->storeFareDisplayInformation(attrs);

  FareDisplayRequest* request = _modelMap->_fareDisplayTrx->getRequest();

  CPPUNIT_ASSERT(request->requestedInclusionCode() == "PBFBJBBBSBYB");
}

void testMultipleInclusionCodes_LIST_True()
{
  FareDisplayModelMap::Mapping map = {0, 0};

  map.members[_modelMap->SDBMHash("BI0")] = 5;

  _modelMap->_currentMapEntry = (void*)&map;

  MockXercescAttributes attrs;

  attrs.add("BI0", "PBFB");
  _modelMap->_fareDisplayTrx = _trx;
  _modelMap->storeFareDisplayInformation(attrs);

  FareDisplayRequest* request = _modelMap->_fareDisplayTrx->getRequest();

  CPPUNIT_ASSERT(request->requestedInclusionCode() == "PBFB");
}

void testMultipleInclusionCodesThrowsExceptionWhenDuplicateCabinEntered()
{
  FareDisplayModelMap::Mapping map = {0, 0};

  map.members[_modelMap->SDBMHash("BI0")] = 5;

  _modelMap->_currentMapEntry = (void*)&map;

  MockXercescAttributes attrs;

  attrs.add("BI0", "PBFBSBFB");
  _modelMap->_fareDisplayTrx = _trx;
  try {_modelMap->storeFareDisplayInformation(attrs);}
  catch (NonFatalErrorResponseException& nonFat) { CPPUNIT_ASSERT(true); }
  catch (...) { CPPUNIT_ASSERT(false); }
}

void testMultipleInclusionCodesThrowsExceptionWhen_AB_AndOtherCabinEntered()
{
  FareDisplayModelMap::Mapping map = {0, 0};

  map.members[_modelMap->SDBMHash("BI0")] = 5;

  _modelMap->_currentMapEntry = (void*)&map;

  MockXercescAttributes attrs;

  attrs.add("BI0", "PBFBABSB");
  _modelMap->_fareDisplayTrx = _trx;
  try {_modelMap->storeFareDisplayInformation(attrs);}
  catch (NonFatalErrorResponseException& nonFat) { CPPUNIT_ASSERT(true); }
  catch (...) { CPPUNIT_ASSERT(false); }
}

void testMultipleInclusionCodesThrowsExceptionWhenInvalidCabinEntered()
{
  FareDisplayModelMap::Mapping map = {0, 0};

  map.members[_modelMap->SDBMHash("BI0")] = 5;

  _modelMap->_currentMapEntry = (void*)&map;

  MockXercescAttributes attrs;

  attrs.add("BI0", "PBFBAASB");
  _modelMap->_fareDisplayTrx = _trx;
  try {_modelMap->storeFareDisplayInformation(attrs);}
  catch (NonFatalErrorResponseException& nonFat) { CPPUNIT_ASSERT(true); }
  catch (...) { CPPUNIT_ASSERT(false); }
}

void testStoreProcessingOptions_PRM_RCQ_NoError()
{
  FareDisplayModelMap::Mapping map = {0, 0};
  map.members[_modelMap->SDBMHash("PRM")] = 107;
  map.members[_modelMap->SDBMHash("RCQ")] = 108;
  _modelMap->_currentMapEntry = (void*)&map;

  MockXercescAttributes attrs;
  attrs.add("PRM", "F");
  attrs.add("RCQ", "Abcdefghijk,Abcdefghijklmnopqrst,Abcdefghijklmnopqrst,Abcdefghijklmnopqrst");

  _modelMap->_fareDisplayTrx = _trx;
  _modelMap->storeProcessingOptions(attrs);

  FareDisplayRequest* request = _modelMap->_fareDisplayTrx->getRequest();

  CPPUNIT_ASSERT(request != nullptr);

  const std::vector<FareRetailerCode> rcqValues = request->rcqValues();
  CPPUNIT_ASSERT(rcqValues.size() == 4);
  CPPUNIT_ASSERT(rcqValues[0] == "Abcdefghijk");
  CPPUNIT_ASSERT(rcqValues[1] == "Abcdefghijklmnopqrst");
  CPPUNIT_ASSERT(rcqValues[2] == "Abcdefghijklmnopqrst");
  CPPUNIT_ASSERT(rcqValues[3] == "Abcdefghijklmnopqrst");
}
};
CPPUNIT_TEST_SUITE_REGISTRATION(FareDisplayModelMapTest);

} // end of tse namespace
