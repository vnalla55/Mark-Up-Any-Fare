#include "test/testdata/test/TestFactoryTest.h"
#include "test/testdata/TestFactoryManager.h"
#include "test/testdata/TestAddonFareInfoFactory.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestCarrierPreferenceFactory.h"
#include "test/testdata/TestCombinabilityRuleInfoFactory.h"
#include "test/testdata/TestDateTimeFactory.h"
#include "test/testdata/TestDifferentialDataFactory.h"
#include "test/testdata/TestFactoryInstanceMgr.h"
#include "test/testdata/TestFareFactory.h"
#include "test/testdata/TestFareClassAppInfoFactory.h"
#include "test/testdata/TestFareClassAppSegInfoFactory.h"
#include "test/testdata/TestFareInfoFactory.h"
#include "test/testdata/TestFareMarketFactory.h"
#include "test/testdata/TestFareUsageFactory.h"
#include "test/testdata/TestLocFactory.h"
#include "test/testdata/TestPaxTypeFactory.h"
#include "test/testdata/TestPaxTypeBucketFactory.h"
#include "test/testdata/TestPaxTypeFareFactory.h"
#include "test/testdata/TestPaxTypeInfoFactory.h"
#include "test/testdata/TestPfcItemFactory.h"
#include "test/testdata/TestPlusUpFactory.h"
#include "test/testdata/TestPricingUnitFactory.h"
#include "test/testdata/TestTariffCrossRefInfoFactory.h"
#include "test/testdata/TestTaxCodeRegFactory.h"
#include "test/testdata/TestTaxItemFactory.h"
#include "test/testdata/TestTaxRecordFactory.h"
#include "test/testdata/TestTaxResponseFactory.h"

#include "DBAccess/AddonFareInfo.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DataModel/DifferentialData.h"
#include "Common/ClassOfService.h"
#include "Common/DateTime.h"
#include "DataModel/Fare.h"
#include "DBAccess/FareClassAppInfo.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/FareMarket.h"
#include "DBAccess/Loc.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/PaxTypeInfo.h"
#include "Taxes/Pfc/PfcItem.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/TariffCrossRefInfo.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"
#include "DataModel/TaxResponse.h"
#include "Common/TseEnums.h"
#include "Common/Vendor.h"

#include <fstream>

CPPUNIT_TEST_SUITE_REGISTRATION(TestFactoryTest);

using namespace tse;

void
TestFactoryTest::setUp()
{
  CppUnit::TestFixture::setUp();
  redirectIO("/dev/null");
}

void
TestFactoryTest::tearDown()
{
  resetIO();
  CppUnit::TestFixture::tearDown();

  TestFactoryManager::instance()->destroyAll();
}

void
TestFactoryTest::testTestAddonFareInfoFactory()
{
  CPPUNIT_ASSERT_THROW(TestAddonFareInfoFactory::create(
                           "/vobs/atseintl/test/testdata/data/AddonFareInfoBADNAME.xml"),
                       std::exception);

  AddonFareInfo* item =
      TestAddonFareInfoFactory::create("/vobs/atseintl/test/testdata/data/AddonFareInfo.xml");

  CPPUNIT_ASSERT(item->effDate().year() == 2004);
  CPPUNIT_ASSERT(item->discDate().hours() == 10);
  CPPUNIT_ASSERT(item->expireDate().year() == 2005);
  CPPUNIT_ASSERT(item->gatewayMarket() == "gwmkt");
  CPPUNIT_ASSERT(item->interiorMarket() == "intmkt");
  CPPUNIT_ASSERT(item->fareClass() == "FIRST");
  CPPUNIT_ASSERT(item->routing() == "foo");
  CPPUNIT_ASSERT(item->directionality() == '1');
  CPPUNIT_ASSERT(item->owrt() == ALL_WAYS);
  //  CPPUNIT_ASSERT( item->arbZone() == "zone" );
  CPPUNIT_ASSERT(item->addonTariff() == 23);
  CPPUNIT_ASSERT(item->vendor() == Vendor::SABRE);
  CPPUNIT_ASSERT(item->cur() == "USD");
  CPPUNIT_ASSERT(item->fareAmt() == 100.50);
  CPPUNIT_ASSERT(item->noDec() == 2);
  CPPUNIT_ASSERT(item->footNote1() == "footnote 1");
  CPPUNIT_ASSERT(item->footNote2() == "footnote 2");

  // Now write out the file, and compare against some baseline elements.
  std::string dirName = "addonFareInfo";
  std::string fullName = dirName + "/" + dirName + ".xml";
  mkdir(dirName);
  TestAddonFareInfoFactory::write(dirName + "/" + dirName, *item);

  // We only have to test the leading file; all other files are derivatives. If the file= tag
  // is present in the first, we'll assume all the others were written out.

  CPPUNIT_ASSERT(!diff(fullName, "baseline/" + fullName));

  rmdir(dirName);
}

void
TestFactoryTest::testTestAirSegFactory()
{
  CPPUNIT_ASSERT_THROW(
      TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegDFW_LONBADNAME.xml"),
      std::exception);

  Itin itin;

  AirSeg* item = TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegDFW_LON.xml");
  CPPUNIT_ASSERT(item->departureDT() == DateTime(2004, 6, 23, 13, 0, 0));

  CPPUNIT_ASSERT(item->arrivalDT() == DateTime(2004, 6, 23, 15, 0, 0));
  CPPUNIT_ASSERT(item->bookingDT() == DateTime(2004, 6, 22, 14, 30, 0));

  // Do just enough of class of service so we know we loaded it okay (tested elsewhere)
  CPPUNIT_ASSERT(item->classOfService().size() > 0);
  CPPUNIT_ASSERT((*item->classOfService().begin())->numSeats() == 60);

  CPPUNIT_ASSERT(item->pnrSegment() == 1);
  CPPUNIT_ASSERT(item->segmentOrder() == 1);
  CPPUNIT_ASSERT(item->stopOver() == false);
  CPPUNIT_ASSERT(item->forcedStopOver() == 0);
  CPPUNIT_ASSERT(item->forcedConx() == 0);
  CPPUNIT_ASSERT(item->geoTravelType() == International);
  CPPUNIT_ASSERT(item->carrier() == "AA");

  CPPUNIT_ASSERT(item->marketingFlightNumber() == 3189);
  CPPUNIT_ASSERT(item->operatingFlightNumber() == 0);
  CPPUNIT_ASSERT(item->equipmentType() == "767");
  CPPUNIT_ASSERT(item->origin() != NULL); // this is sufficient
  CPPUNIT_ASSERT(item->destination() != NULL); // this is sufficient
  CPPUNIT_ASSERT(item->validatedBookingCode() == "");
  CPPUNIT_ASSERT(item->resStatus() == "");
  CPPUNIT_ASSERT(item->fareBasisCode() == "");
  CPPUNIT_ASSERT(item->bookingCode() == "Y");
  CPPUNIT_ASSERT(item->furthestPoint(itin) == false);
  CPPUNIT_ASSERT(item->considerOnlyCabin() == "ABC");
  CPPUNIT_ASSERT(item->forcedFareBrk() == 0);
  CPPUNIT_ASSERT(item->forcedNoFareBrk() == 0);
  CPPUNIT_ASSERT(item->forcedSideTrip() == 0);

  // Now write out the file, and compare against some baseline elements.
  std::string dirName = "airSeg";
  std::string fullName = dirName + "/" + dirName + ".xml";
  mkdir(dirName);
  TestAirSegFactory::write(dirName + "/" + dirName, *item);

  CPPUNIT_ASSERT(!diff(fullName, "baseline/" + fullName));
  rmdir(dirName);
}

void
TestFactoryTest::testTestCarrierPreferenceFactory()
{
  CPPUNIT_ASSERT_THROW(TestCarrierPreferenceFactory::create(
                           "/vobs/atseintl/test/testdata/data/CarrierPreferenceBADNAME.xml"),
                       std::exception);
  CarrierPreference* item = TestCarrierPreferenceFactory::create(
      "/vobs/atseintl/test/testdata/data/CarrierPreference.xml");

  CPPUNIT_ASSERT(item->expireDate() == DateTime(2004, 6, 23, 12, 0, 0));
  CPPUNIT_ASSERT(item->effDate() == DateTime(2004, 6, 23, 12, 0, 0));
  CPPUNIT_ASSERT(item->discDate() == DateTime(2004, 6, 23, 12, 0, 0));
  CPPUNIT_ASSERT(item->firstTvlDate() == DateTime(2004, 6, 23, 12, 0, 0));
  CPPUNIT_ASSERT(item->lastTvlDate() == DateTime(2004, 6, 23, 12, 0, 0));

  CPPUNIT_ASSERT(item->memoNo() == 1);
  CPPUNIT_ASSERT(item->freebaggageexempt() == 'N');
  CPPUNIT_ASSERT(item->availabilityApplyrul2st() == ' ');
  CPPUNIT_ASSERT(item->availabilityApplyrul3st() == ' ');
  CPPUNIT_ASSERT(item->bypassosc() == ' ');
  CPPUNIT_ASSERT(item->bypassrsc() == ' ');
  CPPUNIT_ASSERT(item->applysamenuctort() == ' ');
  // CPPUNIT_ASSERT( item->openjawdefinition() == ' ');
  CPPUNIT_ASSERT(item->applyrtevaltoterminalpt() == ' ');
  CPPUNIT_ASSERT(item->noApplydrvexceptus() == ' ');
  CPPUNIT_ASSERT(item->applyleastRestrStOptopu() == ' ');
  CPPUNIT_ASSERT(item->applyleastRestrtrnsftopu() == ' ');
  CPPUNIT_ASSERT(item->noApplycombtag1and3() == ' ');
  CPPUNIT_ASSERT(item->applysingleaddonconstr() == ' ');
  CPPUNIT_ASSERT(item->applyspecoveraddon() == ' ');
  CPPUNIT_ASSERT(item->noApplynigeriaCuradj() == ' ');
  CPPUNIT_ASSERT(item->carrier() == "AA");
  CPPUNIT_ASSERT(item->carrierbasenation() == "US");
  CPPUNIT_ASSERT(item->description() == "Default AA preference.");

  // Now write out the file, and compare against some baseline elements.
  std::string dirName = "carrierPreference";
  std::string fullName = dirName + "/" + dirName + ".xml";
  mkdir(dirName);
  TestCarrierPreferenceFactory::write(dirName + "/" + dirName, *item);

  // CPPUNIT_ASSERT( !diff( fullName, "baseline/" + fullName ) );
  rmdir(dirName);
}

void
TestFactoryTest::testTestCombinabilityRuleInfoFactory()
{
  //  const std::vector<CombinabilityRuleInfo*>& item =
  // TestCombinabilityRuleInfoFactory::create("/vobs/atseintl/test/testdata/data/CombinabilityRuleInfo.xml");

  //  CPPUNIT_ASSERT( item.size() > 0 );
}
/*******************
void TestFactoryTest::testTestConstructedFareBucketFactory()
{
  CPPUNIT_ASSERT_THROW(
TestConstructedFareBucketFactory::create("/vobs/atseintl/test/testdata/data/ConstructedFareBucketBADNAME.xml"),
std::exception);
  ConstructedFareBucket* item =
TestConstructedFareBucketFactory::create("/vobs/atseintl/test/testdata/data/ConstructedFareBucket.xml");

  CPPUNIT_ASSERT( item->specifiedFare() != NULL );
  CPPUNIT_ASSERT( item->origAddon() != NULL );
  CPPUNIT_ASSERT( item->destAddon() != NULL );
  CPPUNIT_ASSERT( item->gateway1() == "LAX" );
  CPPUNIT_ASSERT( item->gateway2() == "LON" );
  CPPUNIT_ASSERT( item->isDoubleEnded() == false );
  CPPUNIT_ASSERT( item->oneWayAmount() == 250.55 );

  // Now write out the file, and compare against some baseline elements.
  std::string dirName = "constructedFareBucket";
  std::string fullName = dirName + "/" + dirName + ".xml";
  mkdir( dirName );
  TestConstructedFareBucketFactory::write( dirName + "/" + dirName, *item );

  CPPUNIT_ASSERT( !diff( fullName, "baseline/" + fullName ) );
  rmdir( dirName );
}
******************/

void
TestFactoryTest::testTestDifferentialDataFactory()
{
  CPPUNIT_ASSERT_THROW(TestDifferentialDataFactory::create(
                           "/vobs/atseintl/test/testdata/data/DifferentialDataBADNAME.xml"),
                       std::exception);
  DifferentialData* item =
      TestDifferentialDataFactory::create("/vobs/atseintl/test/testdata/data/DifferentialData.xml");

  CPPUNIT_ASSERT(item->origin() != NULL);
  CPPUNIT_ASSERT(item->destination() != NULL);
  CPPUNIT_ASSERT(item->travelSeg().size() > 0);
  CPPUNIT_ASSERT(item->amount() == 100.0);
  CPPUNIT_ASSERT(item->amountFareClassHigh() == 150.0);
  CPPUNIT_ASSERT(item->amountFareClassLow() == 75.5);
  CPPUNIT_ASSERT(item->hipAmtFareClassHigh() == 120.6);
  CPPUNIT_ASSERT(item->hipAmtFareClassLow() == 80.5);
  CPPUNIT_ASSERT(item->surchargeAmt() == 12.0);
  CPPUNIT_ASSERT(item->fareClassHigh() == "AA");
  CPPUNIT_ASSERT(item->fareClassLow() == "BB");
  CPPUNIT_ASSERT(item->cabin() == 'A');
  CPPUNIT_ASSERT(item->mileage() == 'F');
  CPPUNIT_ASSERT(item->maxPermittedMileage() == 1000);
  CPPUNIT_ASSERT(item->tripType() == 'B');
  CPPUNIT_ASSERT(item->carrier().size() == 1);
  //  std::cout << "carrier = " <<  *(item->carrier().begin()) << std::endl;
  CPPUNIT_ASSERT(*(item->carrier().begin()) == "BA");
  CPPUNIT_ASSERT(item->bookingCode() == "XX");
  CPPUNIT_ASSERT(item->stops() == true);
  CPPUNIT_ASSERT(item->sameCarrier() == false);
  CPPUNIT_ASSERT(item->tag() == "abcd");
  CPPUNIT_ASSERT(item->status() == DifferentialData::SC_NOT_PROCESSED_YET);

  // Now write out the file, and compare against some baseline elements.
  std::string dirName = "differentialData";
  std::string fullName = dirName + "/" + dirName + ".xml";
  mkdir(dirName);
  TestDifferentialDataFactory::write(dirName + "/" + dirName, *item);

  CPPUNIT_ASSERT(!diff(fullName, "baseline/" + fullName));
  rmdir(dirName);
}

void
TestFactoryTest::testTestFareFactory()
{
  CPPUNIT_ASSERT_THROW(TestFareFactory::create("/vobs/atseintl/test/testdata/data/FareBADNAME.xml"),
                       std::exception);
  Fare* item = TestFareFactory::create("/vobs/atseintl/test/testdata/data/Fare.xml");

  CPPUNIT_ASSERT(item->fareInfo() != NULL);
  // CPPUNIT_ASSERT( item->fareBucket() != NULL );
  CPPUNIT_ASSERT(item->tariffCrossRefInfo() != NULL);
  CPPUNIT_ASSERT(item->nucFareAmount() == 275.50);
  // CPPUNIT_ASSERT( item->differSeqNumber() == 2 );
  // CPPUNIT_ASSERT( item->calculationInd() == 'L' );

  CPPUNIT_ASSERT(item->status().isSet(Fare::FS_Domestic));
  CPPUNIT_ASSERT(!(item->status().isSet(Fare::FS_Transborder)));
  CPPUNIT_ASSERT(item->status().isSet(Fare::FS_PublishedFare));

  CPPUNIT_ASSERT(!item->isCategoryValid(3));
  CPPUNIT_ASSERT(item->isCategoryValid(4));
  CPPUNIT_ASSERT(!item->isCategoryValid(5));
  CPPUNIT_ASSERT(!item->isCategoryValid(15));

  CPPUNIT_ASSERT(!item->isCategoryProcessed(4));
  CPPUNIT_ASSERT(item->isCategoryProcessed(7));
  CPPUNIT_ASSERT(item->isCategoryProcessed(14));

  // Now write out the file, and compare against some baseline elements.
  std::string dirName = "fare";
  std::string fullName = dirName + "/" + dirName + ".xml";
  mkdir(dirName);
  TestFareFactory::write(dirName + "/" + dirName, *item);

  CPPUNIT_ASSERT(!diff(fullName, "baseline/" + fullName));
  rmdir(dirName);
}

void
TestFactoryTest::testTestFareClassAppInfoFactory()
{
  CPPUNIT_ASSERT_THROW(TestFareClassAppInfoFactory::create(
                           "/vobs/atseintl/test/testdata/data/FareClassAppInfoBADNAME.xml"),
                       std::exception);
  FareClassAppInfo* item =
      TestFareClassAppInfoFactory::create("/vobs/atseintl/test/testdata/data/FareClassAppInfo.xml");

  CPPUNIT_ASSERT(item->_effectiveDate.year() == 2004);
  CPPUNIT_ASSERT(item->_expirationDate.year() == 2004);

  CPPUNIT_ASSERT(item->_location1Type == LOCTYPE_CITY);
  // CPPUNIT_ASSERT_EQUAL( item->_location1, std::string("DFW") );
  CPPUNIT_ASSERT(item->_location2Type == LOCTYPE_CITY);
  // CPPUNIT_ASSERT_EQUAL( item->_location2, std::string("LAX") );

  CPPUNIT_ASSERT(item->_vendor == Vendor::ATPCO);
  CPPUNIT_ASSERT(item->_carrier == "AA");
  CPPUNIT_ASSERT(item->_ruleTariff == 0);
  CPPUNIT_ASSERT(item->_ruleNumber == "0");
  CPPUNIT_ASSERT(item->_footnote1 == "LM");
  CPPUNIT_ASSERT(item->_footnote2 == "NO");
  CPPUNIT_ASSERT(item->_fareClass == "Y");
  CPPUNIT_ASSERT(item->_MCN == 0);
  CPPUNIT_ASSERT(item->_textTBLItemNo == 0);
  CPPUNIT_ASSERT(item->_owrt == ALL_WAYS);
  CPPUNIT_ASSERT(item->_routingAppl == ' ');
  CPPUNIT_ASSERT(item->_routingNumber == "XXXX");
  CPPUNIT_ASSERT(item->_fareType == "EU");
  CPPUNIT_ASSERT(item->_seasonType == ' ');
  CPPUNIT_ASSERT(item->_dowType == ' ');
  CPPUNIT_ASSERT(item->_pricingCatType == 'N');
  CPPUNIT_ASSERT(item->_displayCatType == ' ');
  CPPUNIT_ASSERT(item->_seqNo == 1);
  CPPUNIT_ASSERT(item->_unavailTag == ' ');

  CPPUNIT_ASSERT(item->_segs.size() == 2);

  // Now write out the file, and compare against some baseline elements.
  std::string dirName = "fareClassAppInfo";
  std::string fullName = dirName + "/" + dirName + ".xml";
  mkdir(dirName);
  TestFareClassAppInfoFactory::write(dirName + "/" + dirName, *item);

  CPPUNIT_ASSERT(!diff(fullName, "baseline/" + fullName));
  rmdir(dirName);
}

void
TestFactoryTest::testTestFareClassAppSegInfoFactory()
{
  /* FareClassAppSegInfo* fcasi = */ TestFareClassAppSegInfoFactory::create(
      "/vobs/atseintl/test/testdata/data/FareClassAppSegInfo.xml");
  // std::cout << fcasi->_tktCode << std::endl;
}

void
TestFactoryTest::testTestFareInfoFactory()
{
  CPPUNIT_ASSERT_THROW(
      TestFareInfoFactory::create("/vobs/atseintl/test/testdata/data/FareInfoBADNAME.xml"),
      std::exception);
  FareInfo* item = TestFareInfoFactory::create("/vobs/atseintl/test/testdata/data/FareInfo.xml");
  // CPPUNIT_ASSERT( item->_effectiveDate.year() == 2004 );
  // CPPUNIT_ASSERT( item->_effectiveDate.month() == 6 );
  // CPPUNIT_ASSERT( item->_effectiveDate.day() == 23 );
  // CPPUNIT_ASSERT( item->_expirationDate.year() == 2004 );
  // CPPUNIT_ASSERT( item->_expirationDate.month() == 6 );
  // CPPUNIT_ASSERT( item->_expirationDate.day() == 23 );
  CPPUNIT_ASSERT(item->_fareAmount == 239.0);
  CPPUNIT_ASSERT(item->_originalFareAmount == 240.0);
  CPPUNIT_ASSERT(item->_noDec == 2);
  CPPUNIT_ASSERT(item->_fareTariff == 0);
  CPPUNIT_ASSERT(item->_currency == "USD");
  CPPUNIT_ASSERT(item->_vendor == Vendor::ATPCO);
  CPPUNIT_ASSERT(item->_carrier == "AA");
  CPPUNIT_ASSERT(item->_fareClass == "Y");
  CPPUNIT_ASSERT(item->_market1 == "LAX");
  CPPUNIT_ASSERT(item->_market2 == "LON");
  CPPUNIT_ASSERT(item->_footnote1 == "LM");
  CPPUNIT_ASSERT(item->_footnote2 == "NO");
  CPPUNIT_ASSERT(item->_owrt == ALL_WAYS);
  CPPUNIT_ASSERT(item->_ruleNumber == "0");
  CPPUNIT_ASSERT(item->_routingNumber == "XXXX");
  CPPUNIT_ASSERT(item->_globalDirection == GlobalDirection::AL);
  CPPUNIT_ASSERT(item->_directionality == BETWEEN);

  // std::cout << fcai->_footnote1 << std::endl;
  // std::cout << "Seg count = " << fcai->_segCount << std::endl;

  // Now write out the file, and compare against some baseline elements.
  std::string dirName = "fareInfo";
  std::string fullName = dirName + "/" + dirName + ".xml";
  mkdir(dirName);
  TestFareInfoFactory::write(dirName + "/" + dirName, *item);

  // CPPUNIT_ASSERT( !diff( fullName, "baseline/" + fullName ) );
  rmdir(dirName);
}

void
TestFactoryTest::testTestFareMarketFactory()
{
  CPPUNIT_ASSERT_THROW(
      TestFareMarketFactory::create("/vobs/atseintl/test/testdata/data/FareMarketBADNAME.xml"),
      std::exception);
  FareMarket* item =
      TestFareMarketFactory::create("/vobs/atseintl/test/testdata/data/FareMarket.xml");

  CPPUNIT_ASSERT(item->allPaxTypeFare().size() == 1);

  CPPUNIT_ASSERT(item->allPaxTypeFare()[0]->fareMarket() == item);

  CPPUNIT_ASSERT(item->paxTypeCortege().size() == 1);

  // Now write out the file, and compare against some baseline elements.
  std::string dirName = "fareMarket";
  std::string fullName = dirName + "/" + dirName + ".xml";
  mkdir(dirName);
  TestFareMarketFactory::write(dirName + "/" + dirName, *item);

  CPPUNIT_ASSERT(!diff(fullName, "baseline/" + fullName));
  rmdir(dirName);
}

void
TestFactoryTest::testTestFareUsageFactory()
{

  CPPUNIT_ASSERT_THROW(
      TestFareUsageFactory::create("/vobs/atseintl/test/testdata/data/FareUsageBADNAME.xml"),
      std::exception);
  FareUsage* item = TestFareUsageFactory::create("/vobs/atseintl/test/testdata/data/FareUsage.xml");
  CPPUNIT_ASSERT(item->surchargeAmt() == 100);
  CPPUNIT_ASSERT(item->transferAmt() == 140);
  CPPUNIT_ASSERT(item->stopOverAmt() == 130);
  CPPUNIT_ASSERT(item->absorptionAdjustment() == 120);
  CPPUNIT_ASSERT(item->adjustedStopOvers() == 160);
  CPPUNIT_ASSERT(item->hasSideTrip() == true);
  CPPUNIT_ASSERT(item->paxTypeFare() != NULL);
  CPPUNIT_ASSERT(item->travelSeg().size() == 2);
  CPPUNIT_ASSERT(item->isInbound() == true);
  CPPUNIT_ASSERT(item->isAppendNR() == true);
  CPPUNIT_ASSERT(item->penaltyRestInd() == 'I');
  CPPUNIT_ASSERT(item->appendageCode() == "ABCDE");
  // CPPUNIT_ASSERT(item->rec2Cat10() != NULL );

  // Now write out the file, and compare against some baseline elements.
  std::string dirName = "fareUsage";
  std::string fullName = dirName + "/" + dirName + ".xml";
  mkdir(dirName);
  TestFareUsageFactory::write(dirName + "/" + dirName, *item);

  CPPUNIT_ASSERT(!diff(fullName, "baseline/" + fullName));
  rmdir(dirName);
}

void
TestFactoryTest::testTestLocFactory()
{
  CPPUNIT_ASSERT_THROW(
      TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFWBADNAME.xml"),
      std::exception);
  Loc* item = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
  CPPUNIT_ASSERT(item->loc() == "DFW");
  CPPUNIT_ASSERT(item->subarea() == "11");
  CPPUNIT_ASSERT(item->area() == "1");
  //  CPPUNIT_ASSERT( item->transtype() == "C" );
  CPPUNIT_ASSERT(item->nation() == "US");
  CPPUNIT_ASSERT(item->state() == "TX");
  CPPUNIT_ASSERT(item->latdeg() == 32);
  CPPUNIT_ASSERT(item->latmin() == 53);
  CPPUNIT_ASSERT(item->latsec() == 45);
  CPPUNIT_ASSERT(item->lngdeg() == -96);
  CPPUNIT_ASSERT(item->lngmin() == -2);
  CPPUNIT_ASSERT(item->lngsec() == -14);
  CPPUNIT_ASSERT(item->expireDate() == DateTime(2004, 6, 23));
  CPPUNIT_ASSERT(item->effDate() == DateTime(2004, 6, 23));
  CPPUNIT_ASSERT(item->cityInd() == true);
  CPPUNIT_ASSERT(item->bufferZoneInd() == false);
  CPPUNIT_ASSERT(item->ruralarpind() == false);
  CPPUNIT_ASSERT(item->multitransind() == false);
  CPPUNIT_ASSERT(item->faresind() == false);
  CPPUNIT_ASSERT(item->dstgrp() == "1234");
  CPPUNIT_ASSERT(item->description() == "Dallas-Fort Worth airport");

  // Now write out the file, and compare against some baseline elements.
  std::string dirName = "loc";
  std::string fullName = dirName + "/" + dirName + ".xml";
  mkdir(dirName);
  TestLocFactory::write(dirName + "/" + dirName, *item);

  CPPUNIT_ASSERT(!diff(fullName, "baseline/" + fullName));
  rmdir(dirName);
}

void
TestFactoryTest::testTestPaxTypeFactory()
{
  PaxType* item = TestPaxTypeFactory::create("/vobs/atseintl/test/testdata/data/PaxType.xml");
  // Now write out the file, and compare against some baseline elements.
  std::string dirName = "paxType";
  std::string fullName = dirName + "/" + dirName + ".xml";
  mkdir(dirName);
  TestPaxTypeFactory::write(dirName + "/" + dirName, *item);
  rmdir(dirName);
}

void
TestFactoryTest::testTestPaxTypeBucketFactory()
{
  /* PaxTypeBucket* ptc= */ TestPaxTypeBucketFactory::create(
      "/vobs/atseintl/test/testdata/data/PaxTypeBucket.xml");
  // std::cout << ptc->inboundCurrency() << std::endl;
}

void
TestFactoryTest::testTestPaxTypeFareFactory()
{
  CPPUNIT_ASSERT_THROW(
      TestPaxTypeFareFactory::create("/vobs/atseintl/test/testdata/data/PaxTypeFareBADNAME.xml"),
      std::exception);
  PaxTypeFare* item =
      TestPaxTypeFareFactory::create("/vobs/atseintl/test/testdata/data/PaxTypeFare.xml");

  CPPUNIT_ASSERT(item->status().isSet(PaxTypeFare::PTF_FareByRule));
  CPPUNIT_ASSERT(item->segmentStatus().size() == 1);
  CPPUNIT_ASSERT(item->cabin() == 0); // empty
  CPPUNIT_ASSERT(item->fareTypeDesignator() == 0);
  CPPUNIT_ASSERT(item->fareTypeApplication() == 0);
  CPPUNIT_ASSERT(item->actualPaxType() != NULL);
  CPPUNIT_ASSERT(item->fareClassAppInfo() != NULL);
  CPPUNIT_ASSERT(item->fareClassAppSegInfo() != NULL);
  CPPUNIT_ASSERT(item->fare() != NULL);

  // Now write out the file, and compare against some baseline elements.
  std::string dirName = "paxTypeFare";
  std::string fullName = dirName + "/" + dirName + ".xml";
  mkdir(dirName);
  TestPaxTypeFareFactory::write(dirName + "/" + dirName, *item);

  CPPUNIT_ASSERT(!diff(fullName, "baseline/" + fullName));
  rmdir(dirName);
}

void
TestFactoryTest::testTestPaxTypeInfoFactory()
{
  /* PaxTypeInfo* pti= */ TestPaxTypeInfoFactory::create(
      "/vobs/atseintl/test/testdata/data/PaxTypeInfo.xml");
  // std::cout << pti->description() << std::endl;
}

void
TestFactoryTest::testTestPfcItemFactory()
{
  CPPUNIT_ASSERT_THROW(
      TestPfcItemFactory::create("/vobs/atseintl/test/testdata/data/PfcItemBADNAME.xml"),
      std::exception);
  PfcItem* item = TestPfcItemFactory::create("/vobs/atseintl/test/testdata/data/PfcItem.xml");
  CPPUNIT_ASSERT(item->absorptionInd() == true);
  CPPUNIT_ASSERT(item->pfcAmount() == 123);
  CPPUNIT_ASSERT(item->pfcCurrencyCode() == "USD");
  CPPUNIT_ASSERT(item->pfcDecimals() == 2);
  CPPUNIT_ASSERT(item->couponNumber() == 12345);
  CPPUNIT_ASSERT(item->pfcAirportCode() == "DFW");

  // std::cout << pti->description() << std::endl;

  // Now write out the file, and compare against some baseline elements.
  std::string dirName = "pfcItem";
  std::string fullName = dirName + "/" + dirName + ".xml";
  mkdir(dirName);
  TestPfcItemFactory::write(dirName + "/" + dirName, *item);

  CPPUNIT_ASSERT(!diff(fullName, "baseline/" + fullName));
  rmdir(dirName);
}

void
TestFactoryTest::testTestPricingUnitFactory()
{
  CPPUNIT_ASSERT_THROW(
      TestPricingUnitFactory::create("/vobs/atseintl/test/testdata/data/PricingUnitBADNAME.xml"),
      std::exception);
  PricingUnit* item =
      TestPricingUnitFactory::create("/vobs/atseintl/test/testdata/data/PricingUnit.xml");

  CPPUNIT_ASSERT_EQUAL(item->puType(), PricingUnit::Type::OPENJAW);
  CPPUNIT_ASSERT(item->puSubType() == PricingUnit::ORIG_OPENJAW);
  CPPUNIT_ASSERT(item->puFareType() == PricingUnit::SP);
  CPPUNIT_ASSERT(item->travelSeg().size() == 1);
  CPPUNIT_ASSERT(item->fareUsage().size() == 1);

  // Now write out the file, and compare against some baseline elements.
  std::string dirName = "pricingUnit";
  std::string fullName = dirName + "/" + dirName + ".xml";
  mkdir(dirName);
  TestPricingUnitFactory::write(dirName + "/" + dirName, *item);

  CPPUNIT_ASSERT(!diff(fullName, "baseline/" + fullName));
  rmdir(dirName);
}

void
TestFactoryTest::testTestTariffCrossRefInfoFactory()
{
  CPPUNIT_ASSERT_THROW(TestTariffCrossRefInfoFactory::create(
                           "/vobs/atseintl/test/testdata/data/TariffCrossRefInfoBADNAME.xml"),
                       std::exception);
  TariffCrossRefInfo* item = TestTariffCrossRefInfoFactory::create(
      "/vobs/atseintl/test/testdata/data/TariffCrossRefInfo.xml");
  // CPPUNIT_ASSERT( item->_effectiveDate.year() == 2004 );
  // CPPUNIT_ASSERT( item->_expirationDate.year() == 2005 );
  // CPPUNIT_ASSERT( item->_discDate.hours() == 11 );
  CPPUNIT_ASSERT(item->_vendor == Vendor::SABRE);
  CPPUNIT_ASSERT(item->_carrier == "AA");
  CPPUNIT_ASSERT(item->_crossRefType == DOMESTIC);
  CPPUNIT_ASSERT(item->_globalDirection == GlobalDirection::PN);
  CPPUNIT_ASSERT(item->_fareTariff == 1);
  CPPUNIT_ASSERT(item->_fareTariffCode == "A");
  CPPUNIT_ASSERT(item->_ruleTariff == 2);
  CPPUNIT_ASSERT(item->_ruleTariffCode == "B");
  CPPUNIT_ASSERT(item->_governingTariff == 3);
  CPPUNIT_ASSERT(item->_governingTariffCode == "C");
  CPPUNIT_ASSERT(item->_addonTariff1 == 4);
  CPPUNIT_ASSERT(item->_addonTariff1Code == "D");
  CPPUNIT_ASSERT(item->_addonTariff2 == 5);
  CPPUNIT_ASSERT(item->_addonTariff2Code == "E");
  CPPUNIT_ASSERT(item->_routingTariff1 == 6);
  CPPUNIT_ASSERT(item->_routingTariff1Code == "F");
  CPPUNIT_ASSERT(item->_routingTariff2 == 7);
  CPPUNIT_ASSERT(item->_routingTariff2Code == "G");

  CPPUNIT_ASSERT(item->_tariffCat == 1);
  CPPUNIT_ASSERT(item->_zoneNo == "0001");

  // Now write out the file, and compare against some baseline elements.
  std::string dirName = "tariffCrossRefInfo";
  std::string fullName = dirName + "/" + dirName + ".xml";
  mkdir(dirName);
  TestTariffCrossRefInfoFactory::write(dirName + "/" + dirName, *item);

  // CPPUNIT_ASSERT( !diff( fullName, "baseline/" + fullName ) );
  rmdir(dirName);
}

void
TestFactoryTest::testTestTaxCodeRegFactory()
{
  CPPUNIT_ASSERT_THROW(
      TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeRegBADNAME.xml"),
      std::exception);
  TaxCodeReg* item =
      TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg.xml");

  CPPUNIT_ASSERT_EQUAL(13, item->expireDate().hours());
  CPPUNIT_ASSERT_EQUAL(15, item->effDate().hours());
  CPPUNIT_ASSERT_EQUAL(14, item->discDate().hours());
  CPPUNIT_ASSERT_EQUAL(12, item->firstTvlDate().hours());
  CPPUNIT_ASSERT_EQUAL(11, item->lastTvlDate().hours());
  CPPUNIT_ASSERT_EQUAL(10, item->lockDate().hours());
  CPPUNIT_ASSERT_EQUAL('N', item->loc1Type());
  CPPUNIT_ASSERT(item->loc1() == "US");
  CPPUNIT_ASSERT_EQUAL('Y', item->loc1ExclInd());
  CPPUNIT_ASSERT_EQUAL('N', item->loc1Appl());
  CPPUNIT_ASSERT_EQUAL('N', item->loc2Type());
  CPPUNIT_ASSERT(item->loc2() == "US");
  CPPUNIT_ASSERT_EQUAL('Y', item->loc2ExclInd());
  CPPUNIT_ASSERT_EQUAL('N', item->loc2Appl());
  CPPUNIT_ASSERT_EQUAL(1, item->seqNo());
  CPPUNIT_ASSERT(item->taxCode() == "XV");
  CPPUNIT_ASSERT(item->specialProcessNo() == 41);
  // CPPUNIT_ASSERT( item->createDate() == 0 );
  CPPUNIT_ASSERT(item->versionDate() == 0);

  CPPUNIT_ASSERT_EQUAL(10.25, item->taxAmt());
  CPPUNIT_ASSERT_EQUAL(10.00, item->minTax());
  CPPUNIT_ASSERT_EQUAL(11.00, item->maxTax());
  CPPUNIT_ASSERT_EQUAL(1.50, item->plusupAmt());
  CPPUNIT_ASSERT_EQUAL(1.00, item->lowRange());
  CPPUNIT_ASSERT_EQUAL(22.00, item->highRange());
  CPPUNIT_ASSERT_EQUAL(2.00, item->rangeincrement());

  // Now write out the file, and compare against some baseline elements.
  // std::string dirName = "taxCodeReg";
  // std::string fullName = dirName + "/" + dirName + ".xml";
  // mkdir( dirName );
  // TestTaxCodeRegFactory::write( dirName + "/" + dirName, *item );

  // CPPUNIT_ASSERT( !diff( fullName, "baseline/" + fullName ) );
  // rmdir( dirName );
}

void
TestFactoryTest::testTestTaxItemFactory()
{
  CPPUNIT_ASSERT_THROW(
      TestTaxItemFactory::create("/vobs/atseintl/test/testdata/data/TaxItemBADNAME.xml"),
      std::exception);

  TaxItem* item = TestTaxItemFactory::create("/vobs/atseintl/test/testdata/data/TaxItem.xml");

  // CPPUNIT_ASSERT( item->taxCodeReg() != NULL );
  CPPUNIT_ASSERT(item->paymentCurrency() == "USD");
  CPPUNIT_ASSERT(item->paymentCurrencyNoDec() == 2);
  CPPUNIT_ASSERT(item->taxAmount() == 20.5);
  CPPUNIT_ASSERT(item->taxableFare() == 10.5);
  CPPUNIT_ASSERT(item->taxablePartialFare() == 5.6);
  CPPUNIT_ASSERT(item->taxMilesLocal() == 100);
  CPPUNIT_ASSERT(item->taxMilesThru() == 200);
  CPPUNIT_ASSERT(item->taxLocalBoard() == "DFW");
  CPPUNIT_ASSERT(item->taxLocalOff() == "LAX");
  CPPUNIT_ASSERT(item->taxThruBoard() == "NYC");
  CPPUNIT_ASSERT(item->taxThruOff() == "BAL");
  CPPUNIT_ASSERT(item->taxDescription() == "The description.");
  CPPUNIT_ASSERT(item->taxOnTaxInfo() == "The tax on tax info.");
  // CPPUNIT_ASSERT( item->taxRecProcessed() == 't' );
  // CPPUNIT_ASSERT( item->gstTax() == true );

  CPPUNIT_ASSERT(item->failCode() == 'a');
  CPPUNIT_ASSERT(item->partialTax() == 'p');
  CPPUNIT_ASSERT(item->travelSegStartIndex() == 1);
  CPPUNIT_ASSERT(item->travelSegEndIndex() == 2);
  // CPPUNIT_ASSERT( item->interline() == 's' );

  // Now write out the file, and compare against some baseline elements.
  std::string dirName = "taxItem";
  std::string fullName = dirName + "/" + dirName + ".xml";
  mkdir(dirName);
  TestTaxItemFactory::write(dirName + "/" + dirName, *item);

  CPPUNIT_ASSERT(!diff(fullName, "baseline/" + fullName));
  rmdir(dirName);
}

void
TestFactoryTest::testTestTaxRecordFactory()
{
  CPPUNIT_ASSERT_THROW(
      TestTaxRecordFactory::create("/vobs/atseintl/test/testdata/data/TaxRecordBADNAME.xml"),
      std::exception);
  TaxRecord* item = TestTaxRecordFactory::create("/vobs/atseintl/test/testdata/data/TaxRecord.xml");

  CPPUNIT_ASSERT(item->getTaxAmount() == 100.50);
  CPPUNIT_ASSERT(item->taxNoDec() == 2);
  CPPUNIT_ASSERT(item->taxCurrencyCode() == "USD");
  CPPUNIT_ASSERT(item->taxCode() == "ABC");
  CPPUNIT_ASSERT(item->taxDescription() == "Tax applied.");
  CPPUNIT_ASSERT(item->taxNation() == "US");
  CPPUNIT_ASSERT(item->taxType() == 'f');
  CPPUNIT_ASSERT(item->rollXTNotAllowedInd() == 't');
  CPPUNIT_ASSERT(item->taxRolledXTInd() == 'f');
  CPPUNIT_ASSERT(item->multiOccConvRndInd() == 't');
  CPPUNIT_ASSERT(item->taxItemIndex() == 4);

  // std::cout << pti->description() << std::endl;

  // Now write out the file, and compare against some baseline elements.
  std::string dirName = "taxRecord";
  std::string fullName = dirName + "/" + dirName + ".xml";
  mkdir(dirName);
  TestTaxRecordFactory::write(dirName + "/" + dirName, *item);

  CPPUNIT_ASSERT(!diff(fullName, "baseline/" + fullName));
  rmdir(dirName);
}

void
TestFactoryTest::testTestTaxResponseFactory()
{
  CPPUNIT_ASSERT_THROW(
      TestTaxResponseFactory::create("/vobs/atseintl/test/testdata/data/TaxResponseBADNAME.xml"),
      std::exception);
  TaxResponse* tr =
      TestTaxResponseFactory::create("/vobs/atseintl/test/testdata/data/TaxResponse.xml");
  CPPUNIT_ASSERT(tr->paxTypeCode() == "ADT");
  CPPUNIT_ASSERT(tr->pfcItemVector().size() == 1);
  CPPUNIT_ASSERT(tr->taxRecordVector().size() == 1);

  // Now write out the file, and compare against some baseline elements.
  // std::string dirName = "tariffCrossRefInfo";
  // std::string fullName = dirName + "/" + dirName + ".xml";
  // mkdir( dirName );
  // TestTaxResponseFactory::write( dirName + "/" + dirName, *item );

  // CPPUNIT_ASSERT( !diff( fullName, "baseline/" + fullName ) );
  // rmdir( dirName );
}
