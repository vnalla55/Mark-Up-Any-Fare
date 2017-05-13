#include <string>
#include <iostream>
#include <vector>

#include "Taxes/PfcTaxesExemption/AutomaticPfcTaxExemption.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"

#include "DataModel/PaxType.h"
#include "DBAccess/PaxTypeInfo.h"

#include "DataModel/PricingRequest.h"
#include "Common/DateTime.h"
#include "DBAccess/Loc.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/PaxType.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PaxType.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Diagnostic/Diagnostic.h"
#include "Diagnostic/DiagManager.h"
#include "DataModel/Itin.h"
#include "DataModel/FarePath.h"
#include "DataModel/Response.h"
#include "DataModel/Itin.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/FareUsage.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/FareByRuleApp.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

#include "test/include/MockTseServer.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include <unistd.h>
#include "DBAccess/PfcCoterminal.h"
#include "DBAccess/PfcMultiAirport.h"
#include <boost/assign/std/vector.hpp>

#include "test/testdata/TestFactoryManager.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestFareMarketFactory.h"
#include "test/testdata/TestFareUsageFactory.h"
#include "test/testdata/TestPaxTypeFactory.h"
#include "test/testdata/TestPaxTypeFareFactory.h"
#include "test/testdata/TestTktDesignatorExemptInfoFactory.h"

using namespace boost::assign;
using namespace std;

namespace tse
{

namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;
};
}

class AutomaticPfcTaxExemptionTest : public CppUnit::TestFixture
{
   CPPUNIT_TEST_SUITE(AutomaticPfcTaxExemptionTest);
   CPPUNIT_TEST(testAutomaticPfcTaxExemptionCase0);
   CPPUNIT_TEST(testAutomaticPfcTaxExemptionCase1);
   CPPUNIT_TEST(testmatchTktDes);
   CPPUNIT_TEST_SUITE_END();

public:

void setUp()
{
    _memHandle.create<MyDataHandle>();
}
void tearDown()
{
    _memHandle.clear();
}


void testAutomaticPfcTaxExemptionCase0()
{


   PricingTrx trx;

   Billing billing;
   DiagManager* diagManager = new DiagManager(trx);
   _memHandle.insert(diagManager);

   trx.diagnostic().diagnosticType() = Diagnostic807;
   trx.diagnostic().activate();

   trx.billing() = &billing;

   trx.billing()->partitionID() = "AA";

   Agent agent;
   agent.currencyCodeAgent() = "USD";

   PricingOptions options;
   trx.setOptions(&options);

   PricingRequest request;
   trx.setRequest(&request);

   trx.getRequest()->ticketingAgent() = &agent;
   DateTime dt;
   trx.getRequest()->ticketingDT() = dt.localTime();

   Loc origin;
   Loc destination;

   origin.loc() = std::string( "MIA");
   origin.nation() = std::string( "US" );

   destination.loc() = std::string( "DFW");
   destination.nation() = std::string("US" );
   trx.getRequest()->ticketingAgent()->agentLocation() = &destination;


   AirSeg airSeg;
   TravelSeg* travelSeg = &airSeg;
   airSeg.carrier() =  std::string( "AA");
   travelSeg->origin() = &origin;
   travelSeg->destination() = &destination;
   travelSeg->departureDT() = dt.localTime();
   travelSeg->pssDepartureDate() = "2014-03-01";
   Itin itin;
   itin.originationCurrency() = "USD";
   itin.calculationCurrency() = "USD";

   trx.itin().push_back(&itin);

   PaxType pt;
   PaxTypeInfo pti;
   pt.paxTypeInfo() = &pti;

   PricingUnit pu1;

   FarePath farePath;

   farePath.paxType() = &pt;
   farePath.pricingUnit().push_back(&pu1);

   farePath.itin() = &itin;

   farePath.itin()->travelSeg().push_back(travelSeg);

   itin.farePath().push_back(&farePath);


   DCFactory* factory = DCFactory::instance();
   DiagCollector& diagCollector = *(factory->create(trx));

   TaxResponse taxResponse;
   taxResponse.farePath() = &farePath;
   taxResponse.paxTypeCode() = std::string( "ADT" );
   taxResponse.diagCollector() = &diagCollector;

   TaxCodeReg  taxCodeReg;
   taxCodeReg.loc1Type() = tse::LocType('N');
   taxCodeReg.loc2Type() = tse::LocType('N');
   taxCodeReg.loc1() = tse::LocCode("MY");

   taxCodeReg.loc1ExclInd() = tse::Indicator( 'Y'  );

   taxCodeReg.loc2() = tse::LocCode("MY");

   taxCodeReg.loc2ExclInd() = tse::Indicator( 'N'  );

   Loc origin2;
   Loc destination2;

   origin2.loc() = std::string( "DFW");
   origin2.nation() = std::string( "US" );

   destination2.loc() = std::string( "LAX");
   destination2.nation() = std::string("US" );
   trx.getRequest()->ticketingAgent()->agentLocation() = &destination2;

   AirSeg airSeg2;
   TravelSeg* travelSeg2 = &airSeg2;
   airSeg2.carrier() =  std::string( "AA");

   travelSeg2->origin() = &origin2;
   travelSeg2->destination() = &destination2;
   travelSeg2->departureDT() = dt.localTime();

   farePath.itin()->travelSeg().push_back(travelSeg2);

   Loc origin3;
   Loc destination3;

   origin3.loc() = std::string( "LAX");
   origin3.nation() = std::string( "US" );

   destination3.loc() = std::string( "SEA");
   destination3.nation() = std::string("US" );
   trx.getRequest()->ticketingAgent()->agentLocation() = &destination3;

   AirSeg airSeg3;
   TravelSeg* travelSeg3 = &airSeg3;
   airSeg3.carrier() =  std::string( "AA");

   travelSeg3->origin() = &origin3;
   travelSeg3->destination() = &destination3;
   travelSeg3->departureDT() = dt;

   farePath.itin()->travelSeg().push_back(travelSeg3);

   Loc origin4;
   Loc destination4;

   origin4.loc() = std::string( "SEA");
   origin4.nation() = std::string( "US" );

   destination4.loc() = std::string( "MIA");
   destination4.nation() = std::string("US" );
   trx.getRequest()->ticketingAgent()->agentLocation() = &destination4;

   AirSeg airSeg4;
   TravelSeg* travelSeg4 = &airSeg4;
   airSeg4.carrier() =  std::string( "AA");

   travelSeg4->origin() = &origin4;
   travelSeg4->destination() = &destination4;
   travelSeg4->departureDT() = dt.localTime();

   farePath.itin()->travelSeg().push_back(travelSeg4);
   CPPUNIT_ASSERT_MESSAGE("Error in isAutomaticPfcExemtpionEnabled: Test 1",
               !AutomaticPfcTaxExemption::isAutomaticPfcExemtpionEnabled(trx, taxResponse));

   CPPUNIT_ASSERT(trx.diagnostic().toString().find("VALIDATION RESULT: FAIL") != std::string::npos);

   CPPUNIT_ASSERT_MESSAGE("Error in AutomaticPfcTaxExemption::isAAItinOnly: Test 2",
                AutomaticPfcTaxExemption::isAAItinOnly(farePath));
   airSeg.carrier() =  std::string( "IB");
   CPPUNIT_ASSERT_MESSAGE("Error in AutomaticPfcTaxExemption::isAAItinOnly: Test 3",
                !AutomaticPfcTaxExemption::isAAItinOnly(farePath));


}

void testAutomaticPfcTaxExemptionCase1()
{
         PricingTrx trx;
         Billing billing;
         trx.billing() = &billing;
         trx.billing()->partitionID() = "AA";
         Agent agent;
         PricingRequest request;
         trx.setRequest(&request);
         trx.getRequest()->ticketingAgent() = &agent;
         DiagManager* diagManager = new DiagManager(trx);
         _memHandle.insert(diagManager);

         trx.diagnostic().diagnosticType() = Diagnostic807;
         trx.diagnostic().activate();

         PaxType* paxTypeAdult = TestPaxTypeFactory::create( "testdata/PaxTypeAdult.xml");

         CPPUNIT_ASSERT(paxTypeAdult != 0);

         trx.paxType().push_back(paxTypeAdult);


         AirSeg* airSeg0 = TestAirSegFactory::create("testdata/AirSegDFWJFK.xml");
         CPPUNIT_ASSERT(airSeg0 != 0);
         AirSeg* airSeg1 = TestAirSegFactory::create("testdata/AirSegJFKDFW.xml");
         CPPUNIT_ASSERT(airSeg1 != 0);

         // We need FarePath and PricingUnit for GeoItemTbl validation, although
         // we do not declaim to support GeoItemTbl on accompanied travel yet

         FareUsage* fu0 = TestFareUsageFactory::create( "testdata/FareUsage0.xml");

         FareMarket* fareMarket = TestFareMarketFactory::create( "testdata/FareMarket.xml");
         CPPUNIT_ASSERT(fu0 != 0);

         trx.travelSeg().push_back(airSeg0);
         trx.travelSeg().push_back(airSeg1);
         trx.fareMarket().push_back(fareMarket);
         trx.paxType().push_back(paxTypeAdult);

         std::vector<FareUsage*> fareUsages;
         fareUsages.push_back(fu0);
         FareByRuleApp fbrApp;
         fbrApp.vendor() = "ATP";
         fbrApp.carrier() = "AA";
         fbrApp.segCnt() = 1;
         fbrApp.primePaxType() = "ADT";
         fbrApp.tktDesignator() = "TEST";

         FareByRuleItemInfo fbrItemInfo;
         fbrItemInfo.fareInd() = RuleConst::ADD_SPECIFIED_TO_CALCULATED;
         fbrItemInfo.percent() = 100;
         fbrItemInfo.specifiedFareAmt1() = 23.00;
         fbrItemInfo.specifiedCur1() = "GBP";


         CategoryRuleInfo  cRI;
         cRI.carrierCode() = "AA";

         FBRPaxTypeFareRuleData fbrPaxTypeFareRuleData;
         fbrPaxTypeFareRuleData.ruleItemInfo() = &fbrItemInfo;
         fbrPaxTypeFareRuleData.fbrApp() = &fbrApp;
         fbrPaxTypeFareRuleData.categoryRuleInfo() = &cRI;

         fbrPaxTypeFareRuleData.ruleItemInfo() = dynamic_cast<RuleItemInfo*>(&fbrItemInfo);
         fu0->paxTypeFare()->setRuleData(25, trx.dataHandle(), &fbrPaxTypeFareRuleData);

         PricingUnit pu;
         pu.fareUsage().push_back(fu0);
         Itin itin;
         itin.travelSeg().push_back(airSeg0);
         itin.travelSeg().push_back(airSeg1);

         FarePath fp;
         fp.itin() = &itin;
         fp.pricingUnit().push_back(&pu);

         TaxResponse taxResponse;
         taxResponse.farePath() = &fp;
         taxResponse.paxTypeCode() = std::string( "ADT" );

         taxResponse.farePath()->itin()->travelSeg().front()->pssDepartureDate() = "2014-03-01";
         TaxCodeReg  taxCodeReg;
         taxCodeReg.loc1Type() = tse::LocType('N');
         taxCodeReg.loc2Type() = tse::LocType('N');
         taxCodeReg.loc1() = tse::LocCode("MY");

         taxCodeReg.loc1ExclInd() = tse::Indicator( 'Y'  );

         taxCodeReg.loc2() = tse::LocCode("MY");
         taxCodeReg.loc2ExclInd() = tse::Indicator( 'N'  );

         CPPUNIT_ASSERT_MESSAGE("Error in isAutomaticPfcExemtpionEnabled: Test 4",
               AutomaticPfcTaxExemption::isAutomaticPfcExemtpionEnabled(trx, taxResponse));
         CPPUNIT_ASSERT(trx.diagnostic().toString().find("FINAL PFC/TAX EXEMPTION RESULTS") != std::string::npos);
 }

void testmatchTktDes()
{

   CPPUNIT_ASSERT_MESSAGE("Error in matchTktDes: Test 1",
         AutomaticPfcTaxExemption::matchTktDes("-CD", "ABCD"));
   CPPUNIT_ASSERT_MESSAGE("Error in matchTktDes: Test 2",
            AutomaticPfcTaxExemption::matchTktDes("AB-", "ABCD"));
   CPPUNIT_ASSERT_MESSAGE("Error in matchTktDes: Test 3",
            AutomaticPfcTaxExemption::matchTktDes("ABCD", "ABCD"));
   CPPUNIT_ASSERT_MESSAGE("Error in matchTktDes: Test 4",
            !AutomaticPfcTaxExemption::matchTktDes("ABCD-", "ABCD"));
   CPPUNIT_ASSERT_MESSAGE("Error in matchTktDes: Test 5",
            !AutomaticPfcTaxExemption::matchTktDes("-AB", "ABCD"));
   CPPUNIT_ASSERT_MESSAGE("Error in matchTktDes: Test 6",
            !AutomaticPfcTaxExemption::matchTktDes("CD-", "ABCD"));

}

 
private:
    TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(AutomaticPfcTaxExemptionTest);
}


