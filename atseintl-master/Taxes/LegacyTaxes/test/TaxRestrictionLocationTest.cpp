#include "test/include/CppUnitHelperMacros.h"
#include "test/testdata/TestXMLHelper.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/Agent.h"
#include "DBAccess/TaxRestrictionLocationInfo.h"
#include "Common/Vendor.h"
#include "Taxes/LegacyTaxes/TaxCodeValidator.h"
#include "test/testdata/TestLocFactory.h"
#include "test/testdata/TestXMLHelper.h"

namespace tse
{

static const VendorCode vendorCode = Vendor::SABRE;
static const ZoneType zoneType = MANUAL;
static const LocUtil::ApplicationType applicationType = LocUtil::TAXES;

class TaxRestrictionLocationTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxRestrictionLocationTest);
  CPPUNIT_TEST(TestCheckRestriction_noRestriction);
  CPPUNIT_TEST(TestCheckRestriction_noRestriction2);
  CPPUNIT_TEST(TestCheckRestriction_pccRestriction);
  CPPUNIT_TEST(TestCheckRestriction_area1Restriction);
  CPPUNIT_TEST(TestCheckRestriction_pccExcludedFromRestriction);
  CPPUNIT_TEST(TestCheckRestriction_marketExcludedFromRestriction);
  CPPUNIT_TEST(TestCheckRestriction_marketNotExcludedFromRestriction);
  CPPUNIT_TEST_SUITE_END();

  PricingTrx* trx;
  Agent* agent;
  PricingRequest* request;
  DateTime* dt;
  PricingOptions* options;
  TaxRestrictionLocationInfo* location;
  TaxCodeValidator* taxCodeValidator;
  Loc* agentLoc;

public:
  void setUp()
  {
    trx = new PricingTrx();
    agent = new Agent();
    request = new PricingRequest();
    dt = new DateTime();
    options = new PricingOptions();
    location = new TaxRestrictionLocationInfo();
    taxCodeValidator = new TaxCodeValidator();
    agentLoc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");

    trx->setRequest(request);
    trx->getRequest()->ticketingAgent() = agent;
    trx->getRequest()->ticketingDT() = dt->localTime();
    trx->setOptions(options);
    options->currencyOverride() = "";
    agent->tvlAgencyPCC() = "AB12";
    agent->tvlAgencyIATA() = "1234567";
    agent->agentLocation() = agentLoc;
  }

  void tearDown()
  {
    delete agent;
    delete options;
    delete request;
    delete trx;
    delete taxCodeValidator;
    delete location;
    delete dt;
  }

  void TestCheckRestriction_noRestriction()
  {
    bool res = taxCodeValidator->checkRestrictionLocation(
        location, *trx, vendorCode, zoneType, applicationType);
    CPPUNIT_ASSERT(res);
  }

  void TestCheckRestriction_noRestriction2()
  {

    std::vector<TaxRestrictionLocationInfo::TaxRestrictionLocationInfoSeq> seqs;
    location->seqs() = seqs;
    bool res = taxCodeValidator->checkRestrictionLocation(
        location, *trx, vendorCode, zoneType, applicationType);
    CPPUNIT_ASSERT(res);
  }

  void TestCheckRestriction_pccRestriction()
  {

    std::vector<TaxRestrictionLocationInfo::TaxRestrictionLocationInfoSeq> seqs;
    seqs.resize(1);
    seqs[0].saleIssueInd() = 'B';
    seqs[0].inclExclInd() = 'E';
    seqs[0].locType() = 'X';
    seqs[0].loc() = "AB12";
    location->seqs() = seqs;
    bool res = taxCodeValidator->checkRestrictionLocation(
        location, *trx, vendorCode, zoneType, applicationType);
    CPPUNIT_ASSERT(!res);
  }

  void TestCheckRestriction_area1Restriction()
  {

    std::vector<TaxRestrictionLocationInfo::TaxRestrictionLocationInfoSeq> seqs;
    seqs.resize(1);
    seqs[0].saleIssueInd() = 'B';
    seqs[0].inclExclInd() = 'E';
    seqs[0].locType() = 'A';
    seqs[0].loc() = "1";
    location->seqs() = seqs;
    bool res = taxCodeValidator->checkRestrictionLocation(
        location, *trx, vendorCode, zoneType, applicationType);
    CPPUNIT_ASSERT(!res);
  }

  void TestCheckRestriction_pccExcludedFromRestriction()
  {

    std::vector<TaxRestrictionLocationInfo::TaxRestrictionLocationInfoSeq> seqs;
    seqs.resize(2);
    seqs[0].saleIssueInd() = 'B';
    seqs[0].inclExclInd() = 'E';
    seqs[0].locType() = 'A';
    seqs[0].loc() = "1";
    seqs[1].saleIssueInd() = 'B';
    seqs[1].inclExclInd() = 'I';
    seqs[1].locType() = 'X';
    seqs[1].loc() = "AB12";
    location->seqs() = seqs;
    bool res = taxCodeValidator->checkRestrictionLocation(
        location, *trx, vendorCode, zoneType, applicationType);
    CPPUNIT_ASSERT(res);
  }

  void TestCheckRestriction_marketExcludedFromRestriction()
  {

    std::vector<TaxRestrictionLocationInfo::TaxRestrictionLocationInfoSeq> seqs;
    seqs.resize(2);
    seqs[0].saleIssueInd() = 'B';
    seqs[0].inclExclInd() = 'E';
    seqs[0].locType() = 'A';
    seqs[0].loc() = "1";
    seqs[1].saleIssueInd() = 'B';
    seqs[1].inclExclInd() = 'I';
    seqs[1].locType() = 'C';
    seqs[1].loc() = "DFW";
    location->seqs() = seqs;
    bool res = taxCodeValidator->checkRestrictionLocation(
        location, *trx, vendorCode, zoneType, applicationType);
    CPPUNIT_ASSERT(res);
  }

  void TestCheckRestriction_marketNotExcludedFromRestriction()
  {

    std::vector<TaxRestrictionLocationInfo::TaxRestrictionLocationInfoSeq> seqs;
    seqs.resize(2);
    seqs[0].saleIssueInd() = 'B';
    seqs[0].inclExclInd() = 'E';
    seqs[0].locType() = 'A';
    seqs[0].loc() = "1";
    seqs[1].saleIssueInd() = 'B';
    seqs[1].inclExclInd() = 'I';
    seqs[1].locType() = 'C';
    seqs[1].loc() = "DEN";
    location->seqs() = seqs;
    bool res = taxCodeValidator->checkRestrictionLocation(
        location, *trx, vendorCode, zoneType, applicationType);
    CPPUNIT_ASSERT(!res);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxRestrictionLocationTest);
}
