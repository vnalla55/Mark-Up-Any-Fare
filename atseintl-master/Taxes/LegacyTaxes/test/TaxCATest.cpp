#include <string>
#include <time.h>
#include <iostream>
#include <vector>

#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/LegacyTaxes/TaxCA01.h"
#include "Taxes/LegacyTaxes/TaxCA02.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxMap.h"

#include "DataModel/PricingRequest.h"
#include "DBAccess/Loc.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PaxType.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Diagnostic/Diagnostic.h"
#include "DataModel/Itin.h"
#include "DataModel/FarePath.h"
#include "DataModel/Response.h"
#include "DataModel/Itin.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingOptions.h"
#include "Common/DateTime.h"
#include "DataModel/TravelSeg.h"

#include "Server/TseServer.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseUtil.h"
#include "Diagnostic/Diag804Collector.h"

#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestLocFactory.h"
#include "test/testdata/TestTaxCodeRegFactory.h"
#include "test/testdata/TestClassOfServiceFactory.h"
#include "test/testdata/TestXMLHelper.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include <unistd.h>

using namespace std;
namespace tse
{
class TaxCATest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxCATest);
  CPPUNIT_TEST(testConstructor);
  //CPPUNIT_TEST(testTaxCA);
  CPPUNIT_TEST(testTaxCACxrExemption);
  CPPUNIT_TEST(testAdjustTaxFalseCorrectFlag);
  CPPUNIT_TEST(testAdjustTaxTrueCorrectFlag);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }
  void tearDown() { _memHandle.clear(); }

public:
  void testConstructor()
  {
    try { TaxCATest taxCATest; }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void prepareData()
  {

    taxCodeRegs[0].taxCode() = "CA1";
    taxCodeRegs[0].seqNo() = 100;
    taxCodeRegs[0].loc1() = "00019";
    taxCodeRegs[0].loc1ExclInd() = 'N';
    taxCodeRegs[0].loc1Type() = 'Z';
    taxCodeRegs[0].loc2() = "00019";
    taxCodeRegs[0].loc2ExclInd() = 'N';
    taxCodeRegs[0].loc2Type() = 'Z';
    taxCodeRegs[0].maxTax() = 9.33;
    taxCodeRegs[0].taxAmt() = 4.67;
    taxCodeRegs[0].tripType() = 'B';

    taxCodeRegs[1].taxCode() = "CA1";
    taxCodeRegs[1].seqNo() = 200;
    taxCodeRegs[1].loc1() = "00019";
    taxCodeRegs[1].loc1ExclInd() = 'N';
    taxCodeRegs[1].loc1Type() = 'Z';
    taxCodeRegs[1].loc2() = "00911";
    taxCodeRegs[1].loc2ExclInd() = 'N';
    taxCodeRegs[1].loc2Type() = 'Z';
    taxCodeRegs[1].maxTax() = 15.89;
    taxCodeRegs[1].taxAmt() = 7.94;
    taxCodeRegs[1].tripType() = 'F';

    taxCodeRegs[2].taxCode() = "CA2";
    taxCodeRegs[2].seqNo() = 100;
    taxCodeRegs[2].loc1() = "00019";
    taxCodeRegs[2].loc1ExclInd() = 'N';
    taxCodeRegs[2].loc1Type() = 'Z';
    taxCodeRegs[2].loc2() = "00019";
    taxCodeRegs[2].loc2ExclInd() = 'N';
    taxCodeRegs[2].loc2Type() = 'Z';
    taxCodeRegs[2].maxTax() = 9.80;
    taxCodeRegs[2].taxAmt() = 4.90;
    taxCodeRegs[2].tripType() = 'B';

    taxCodeRegs[3].taxCode() = "CA2";
    taxCodeRegs[3].seqNo() = 200;
    taxCodeRegs[3].loc1() = "00019";
    taxCodeRegs[3].loc1ExclInd() = 'N';
    taxCodeRegs[3].loc1Type() = 'Z';
    taxCodeRegs[3].loc2() = "00911";
    taxCodeRegs[3].loc2ExclInd() = 'N';
    taxCodeRegs[3].loc2Type() = 'Z';
    taxCodeRegs[3].maxTax() = 16.68;
    taxCodeRegs[3].taxAmt() = 8.34;
    taxCodeRegs[3].tripType() = 'F';

    taxCodeRegs[4].taxCode() = "CA3";
    taxCodeRegs[4].seqNo() = 100;
    taxCodeRegs[4].loc1() = "00019";
    taxCodeRegs[4].loc1ExclInd() = 'N';
    taxCodeRegs[4].loc1Type() = 'Z';
    taxCodeRegs[4].loc2() = "00912";
    taxCodeRegs[4].loc2ExclInd() = 'N';
    taxCodeRegs[4].loc2Type() = 'Z';
    taxCodeRegs[4].taxAmt() = 17.00;
    taxCodeRegs[4].tripType() = 'F';
  }

  void testTaxCA()
  {
    prepareData();

    PricingTrx trx;

    Agent agent;
    agent.currencyCodeAgent() = "CAD";

    Loc* agentLoc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocYUL.xml", true);

    PricingRequest request;
    trx.setRequest(&request);
    trx.getRequest()->ticketingAgent() = &agent;
    trx.getRequest()->ticketingDT() = DateTime(2008, 3, 26, 16, 0, 0);
    trx.getRequest()->ticketingAgent()->currencyCodeAgent() = "CAD";
    trx.getRequest()->ticketingAgent()->agentLocation() = agentLoc; // AAA in Nova Scotia - Canada

    PricingOptions options;
    trx.setOptions(&options);

    Itin itin;
    itin.tripCharacteristics().set(Itin::OneWay, true);
    trx.itin().push_back(&itin);

    FarePath farePath;
    farePath.itin() = &itin;
    itin.farePath().push_back(&farePath);
    TaxResponse taxResponse;
    taxResponse.farePath() = &farePath;
    taxResponse.paxTypeCode() = std::string("ADT");

    Diagnostic* diagroot = _memHandle.insert(new Diagnostic(LegacyTaxDiagnostic24));
    //   diagroot->activate();
    Diag804Collector diag(*diagroot);
    taxResponse.diagCollector() = &diag;

    uint16_t result[5] = {};

    farePath.itin()->travelSeg().push_back(
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegYUL_YYZ_AC.xml", true));
    test(result, trx, taxResponse);
    CPPUNIT_ASSERT_EQUAL(1, (int)result[0]);
    CPPUNIT_ASSERT_EQUAL(0, (int)result[1]);
    CPPUNIT_ASSERT_EQUAL(0, (int)result[2]);
    CPPUNIT_ASSERT_EQUAL(0, (int)result[3]);
    CPPUNIT_ASSERT_EQUAL(0, (int)result[4]);

    farePath.itin()->travelSeg().push_back(
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegYYC_YVR_AC.xml", true));
    farePath.itin()->travelSeg().push_back(
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegYVR_YYJ_AC.xml", true));
    farePath.itin()->travelSeg().push_back(
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegYYJ_YVR_AC.xml", true));
    farePath.itin()->travelSeg().push_back(
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegYVR_YYC_AC.xml", true));
    test(result, trx, taxResponse);
    CPPUNIT_ASSERT_EQUAL(11, (int)result[0]);
    CPPUNIT_ASSERT_EQUAL(0, (int)result[1]);
    CPPUNIT_ASSERT_EQUAL(0, (int)result[2]);
    CPPUNIT_ASSERT_EQUAL(0, (int)result[3]);
    CPPUNIT_ASSERT_EQUAL(0, (int)result[4]);

    farePath.itin()->travelSeg().push_back(
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegDFW_YYC_AA.xml", true));
    farePath.itin()->travelSeg().push_back(
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegYYC_DFW_AA.xml", true));
    test(result, trx, taxResponse);
    CPPUNIT_ASSERT_EQUAL(0, (int)result[0]);
    CPPUNIT_ASSERT_EQUAL(0, (int)result[1]);
    CPPUNIT_ASSERT_EQUAL(0, (int)result[2]);
    CPPUNIT_ASSERT_EQUAL(2, (int)result[3]);
    CPPUNIT_ASSERT_EQUAL(0, (int)result[4]);

    farePath.itin()->travelSeg().push_back(TestAirSegFactory::create(
        "/vobs/atseintl/Taxes/LegacyTaxes/test/testdata/0_YYC_DFW_YYC_0.xml", true));
    farePath.itin()->travelSeg().push_back(TestAirSegFactory::create(
        "/vobs/atseintl/Taxes/LegacyTaxes/test/testdata/0_YYC_DFW_YYC_1.xml", true));
    test(result, trx, taxResponse);
    CPPUNIT_ASSERT_EQUAL(0, (int)result[0]);
    CPPUNIT_ASSERT_EQUAL(1, (int)result[1]);
    CPPUNIT_ASSERT_EQUAL(0, (int)result[2]);
    CPPUNIT_ASSERT_EQUAL(0, (int)result[3]);
    CPPUNIT_ASSERT_EQUAL(0, (int)result[4]);

    farePath.itin()->travelSeg().push_back(TestAirSegFactory::create(
        "/vobs/atseintl/Taxes/LegacyTaxes/test/testdata/1_FRA_DFW_YYC_YYZ_0.xml", true));
    farePath.itin()->travelSeg().push_back(TestAirSegFactory::create(
        "/vobs/atseintl/Taxes/LegacyTaxes/test/testdata/1_FRA_DFW_YYC_YYZ_1.xml", true));
    farePath.itin()->travelSeg().push_back(TestAirSegFactory::create(
        "/vobs/atseintl/Taxes/LegacyTaxes/test/testdata/1_FRA_DFW_YYC_YYZ_2.xml", true));
    test(result, trx, taxResponse);
    CPPUNIT_ASSERT_EQUAL(0, (int)result[0]);
    CPPUNIT_ASSERT_EQUAL(0, (int)result[1]);
    CPPUNIT_ASSERT_EQUAL(4, (int)result[2]);
    CPPUNIT_ASSERT_EQUAL(0, (int)result[3]);
    CPPUNIT_ASSERT_EQUAL(0, (int)result[4]);

    farePath.itin()->travelSeg().push_back(
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegYYC_SEA_NW.xml", true));
    farePath.itin()->travelSeg().push_back(
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegSEA_HNL_NW.xml", true));
    farePath.itin()->travelSeg().push_back(
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegHNL_SEA_NW.xml", true));
    farePath.itin()->travelSeg().push_back(
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegSEA_YYC_NW.xml", true));
    test(result, trx, taxResponse);
    CPPUNIT_ASSERT_EQUAL(0, (int)result[0]);
    CPPUNIT_ASSERT_EQUAL(0, (int)result[1]);
    CPPUNIT_ASSERT_EQUAL(0, (int)result[2]);
    CPPUNIT_ASSERT_EQUAL(0, (int)result[3]);
    CPPUNIT_ASSERT_EQUAL(1, (int)result[4]);
  }

  void testTaxCACxrExemption()
  {
    TaxCodeReg taxCodeReg;
    taxCodeReg.taxCode() = "CA1";
    taxCodeReg.seqNo() = 100;
    taxCodeReg.loc1() = "00019";
    taxCodeReg.loc1ExclInd() = 'N';
    taxCodeReg.loc1Type() = 'Z';
    taxCodeReg.loc2() = "00019";
    taxCodeReg.loc2ExclInd() = 'N';
    taxCodeReg.loc2Type() = 'Z';
    taxCodeReg.maxTax() = 9.33;
    taxCodeReg.taxAmt() = 4.67;
    taxCodeReg.tripType() = 'B';

    PricingTrx trx;

    Agent agent;
    agent.currencyCodeAgent() = "CAD";

    Loc* agentLoc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocYUL.xml", true);

    PricingRequest request;
    trx.setRequest(&request);
    trx.getRequest()->ticketingAgent() = &agent;
    trx.getRequest()->ticketingDT() = DateTime(2008, 3, 26, 16, 0, 0);
    trx.getRequest()->ticketingAgent()->currencyCodeAgent() = "CAD";
    trx.getRequest()->ticketingAgent()->agentLocation() = agentLoc; // AAA in Nova Scotia - Canada

    PricingOptions options;
    trx.setOptions(&options);

    Itin itin;
    itin.tripCharacteristics().set(Itin::OneWay, true);
    trx.itin().push_back(&itin);

    FarePath farePath;
    farePath.itin() = &itin;
    itin.farePath().push_back(&farePath);
    TaxResponse taxResponse;
    taxResponse.farePath() = &farePath;
    taxResponse.paxTypeCode() = std::string("ADT");

    TaxExemptionCarrier exemptionCxr;
    exemptionCxr.carrier() = "AC";
    taxCodeReg.exemptionCxr().push_back(exemptionCxr);
    taxCodeReg.exempcxrExclInd() = 'Y';

    farePath.itin()->travelSeg().push_back(
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegYUL_YYZ_AC.xml", true));

    uint16_t startIndex = 0;
    uint16_t endIndex = 3;

    TaxCA01 taxCA;

    CPPUNIT_ASSERT(!taxCA.validateTripTypes(trx, taxResponse, taxCodeReg, startIndex, endIndex));

    taxResponse.farePath()->itin()->travelSeg().clear();
  }

  void test(uint16_t* result, PricingTrx& trx, TaxResponse& taxResponse)
  {
    TaxCA01 taxCA;

    uint16_t itinLen = taxResponse.farePath()->itin()->travelSeg().size();
    uint16_t startIndex;
    uint16_t endIndex;
    uint16_t taxCodeSeqI;

    for (taxCodeSeqI = 0; taxCodeSeqI < 5; ++taxCodeSeqI)
    {
      result[taxCodeSeqI] = 0;
      for (startIndex = 0; startIndex < itinLen; ++startIndex)
      {
        endIndex = 3;
        if (taxCA.validateTripTypes(
                trx, taxResponse, taxCodeRegs[taxCodeSeqI], startIndex, endIndex))
        {
          result[taxCodeSeqI] += (uint16_t(1) << startIndex);
        }
      }
    }

    // for (startIndex = 0; startIndex < itinLen; ++startIndex)
    //  delete (AirSeg*)(taxResponse.farePath()->itin()->travelSeg()[startIndex]);

    taxResponse.farePath()->itin()->travelSeg().clear();
  }

  void testAdjustTaxFalseCorrectFlag()
  {

    MoneyAmount taxAmt = 7.12;
    MoneyAmount maxTaxAmt = 14.25;
    _taxCodeReg.maxTax() = maxTaxAmt;

    _taxMock.setTaxAmount(taxAmt);
    _taxMock.itinAnalyzer.correctFlag = false;
    _taxMock.adjustTax(_trx, _taxResponse, _taxCodeReg);

    CPPUNIT_ASSERT_EQUAL(_taxMock.taxAmount(), taxAmt);
    CPPUNIT_ASSERT_EQUAL(_taxMock.itinAnalyzer.correctFlag, false);
  }

  void testAdjustTaxTrueCorrectFlag()
  {
    MoneyAmount taxAmt = 7.12;
    MoneyAmount maxTaxAmt = 14.25;
    _taxCodeReg.maxTax() = maxTaxAmt;

    _taxMock.setTaxAmount(taxAmt);
    _taxMock.itinAnalyzer.correctFlag = true;
    _taxMock.adjustTax(_trx, _taxResponse, _taxCodeReg);

    CPPUNIT_ASSERT_EQUAL(_taxMock.taxAmount(), maxTaxAmt - taxAmt);
    CPPUNIT_ASSERT_EQUAL(_taxMock.itinAnalyzer.correctFlag, false);
  }

private:
  tse::TaxCodeReg taxCodeRegs[5];

  class TaxCAMock : public tse::TaxCA02
  {
  public:
    void setTaxAmount(tse::MoneyAmount taxAmount) { _taxAmount = taxAmount; }
  };

  TaxCAMock _taxMock;
  tse::PricingTrx _trx;
  tse::TaxResponse _taxResponse;
  tse::TaxCodeReg _taxCodeReg;
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxCATest);
}
