//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/Itin.h"
#include "test/testdata/TestLocFactory.h"
#include "test/testdata/TestAirSegFactory.h"
#include "Taxes/LegacyTaxes/TaxSP3601.h"
#include <string>
#include "DataModel/TravelSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/FareUsage.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/PricingOptions.h"
#include "DBAccess/TaxCodeReg.h"
#include "Diagnostic/Diagnostic.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Diagnostic/Diag804Collector.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayRequest.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"

using namespace std;

namespace tse
{

namespace
{
class MyDataHandle : public DataHandleMock
{
public:
  const Mileage* getMileage(const LocCode& origin,
                            const LocCode& dest,
                            Indicator mileageType,
                            const GlobalDirection globalDir,
                            const DateTime& date)
  {
    if (globalDir == GlobalDirection::XX)
      return 0;
    return DataHandleMock::getMileage(origin, dest, mileageType, globalDir, date);
  }
};
}
class TaxSP3601Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxSP3601Test);
  CPPUNIT_TEST(testFindFarthestPoint_noFareBreake);
  CPPUNIT_TEST(testFindFarthestPoint_lastPoint);
  CPPUNIT_TEST(testFindFarthestPoint_allFareBreaks);
  CPPUNIT_TEST(testFindFareBreaks);
  CPPUNIT_TEST(testValidateDestination_noFarthest_shouldNotValidate);
  CPPUNIT_TEST(testValidateDestination_noFarthest_shouldNotValidate_fareDisplay);
  CPPUNIT_TEST(testValidateDestination_noFarthest_shouldValidate);
  CPPUNIT_TEST(testValidateDestination_isFarthest_shouldNotValidate);
  CPPUNIT_TEST(testValidateDestination_isFarthest_shouldValidate);
  CPPUNIT_TEST_SUITE_END();

  Itin itin;
  Itin itin2;
  PricingTrx trx;
  FareDisplayTrx fdTrx;
  std::string xmlPath;
  PricingRequest request;
  FareDisplayRequest fdRequest;
  TaxResponse taxResponse;
  TaxResponse taxResponse2;
  TaxCodeReg taxCodeReg;
  FarePath farePath;
  FarePath farePath2;
  PricingUnit pricingUnit;
  FareUsage fareUsage1;
  FareUsage fareUsage2;
  uint16_t start;
  uint16_t stop;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();
    trx.setRequest(&request);
    trx.getRequest()->ticketingDT() = (DateTime)10;
    trx.setOptions(_memHandle.create<PricingOptions>());
    xmlPath = "/vobs/atseintl/Taxes/LegacyTaxes/test/testdata/";
    itin.travelSeg().clear();
    itin.travelSeg().push_back(
        (TravelSeg*)TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_0.xml"));
    itin.travelSeg().push_back(
        (TravelSeg*)TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_1.xml"));
    itin.travelSeg().push_back(
        (TravelSeg*)TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_2.xml"));
    farePath.pricingUnit().push_back(&pricingUnit);
    pricingUnit.fareUsage().push_back(&fareUsage1);
    pricingUnit.fareUsage().push_back(&fareUsage2);
    fareUsage1.travelSeg().push_back(itin.travelSeg()[0]);
    fareUsage2.travelSeg().push_back(itin.travelSeg()[1]);
    fareUsage2.travelSeg().push_back(itin.travelSeg()[2]);
    farePath.itin() = &itin;
    taxResponse.farePath() = &farePath;
    Diagnostic* diagroot = _memHandle.insert(new Diagnostic(LegacyTaxDiagnostic24));
    Diag804Collector* diag804 = _memHandle.insert(new Diag804Collector(*diagroot));
    taxResponse.diagCollector() = diag804;
    taxCodeReg.loc2Type() = 'N';
    taxCodeReg.loc2() = "FR";

    fdTrx.setRequest(&fdRequest);
    fdTrx.getRequest()->ticketingDT() = (DateTime)10;
    itin2.travelSeg().clear();
    itin2.travelSeg().push_back(
        (TravelSeg*)TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_0.xml"));
    farePath2.itin() = &itin2;
    taxResponse2.farePath() = &farePath2;
  }

  void testFindFarthestPoint_noFareBreake()
  {
    LocRestrictionValidator3601 locRestrictionValidator;
    locRestrictionValidator.findFarthestPoint(trx, itin, 0);
    CPPUNIT_ASSERT(!locRestrictionValidator._farthestPointFound);
  }

  void testFindFarthestPoint_lastPoint()
  {
    LocRestrictionValidator3601 locRestrictionValidator;
    locRestrictionValidator._fareBreaksSet.insert(2);
    locRestrictionValidator.findFarthestPoint(trx, itin, 0);
    CPPUNIT_ASSERT(locRestrictionValidator._farthestPointFound);
    CPPUNIT_ASSERT_EQUAL((uint16_t)2, locRestrictionValidator._farthestSegIndex);
  }

  void testFindFarthestPoint_allFareBreaks()
  {
    LocRestrictionValidator3601 locRestrictionValidator;
    locRestrictionValidator._fareBreaksSet.insert(0);
    locRestrictionValidator._fareBreaksSet.insert(1);
    locRestrictionValidator._fareBreaksSet.insert(2);
    locRestrictionValidator.findFarthestPoint(trx, itin, 0);
    CPPUNIT_ASSERT(locRestrictionValidator._farthestPointFound);
    CPPUNIT_ASSERT_EQUAL((uint16_t)1, locRestrictionValidator._farthestSegIndex);
  }

  void testFindFareBreaks()
  {
    LocRestrictionValidator3601 locRestrictionValidator;
    locRestrictionValidator.findFareBreaks(farePath);
    CPPUNIT_ASSERT(locRestrictionValidator._fareBreaksSet.find(0) !=
                   locRestrictionValidator._fareBreaksSet.end());
    CPPUNIT_ASSERT(locRestrictionValidator._fareBreaksSet.find(2) !=
                   locRestrictionValidator._fareBreaksSet.end());
    CPPUNIT_ASSERT(locRestrictionValidator._fareBreaksSet.size() == 2);
  }

  void testValidateDestination_noFarthest_shouldNotValidate()
  {
    start = 0;
    stop = 0;
    LocRestrictionValidator3601 locRestrictionValidator;
    CPPUNIT_ASSERT(
        !locRestrictionValidator.validateDestination(trx, taxResponse, taxCodeReg, start, stop));
  }

  void testValidateDestination_noFarthest_shouldNotValidate_fareDisplay()
  {
    start = 0;
    stop = 0;
    LocRestrictionValidator3601 locRestrictionValidator;
    CPPUNIT_ASSERT(
        !locRestrictionValidator.validateDestination(fdTrx, taxResponse, taxCodeReg, start, stop));
  }

  void testValidateDestination_noFarthest_shouldValidate()
  {
    taxCodeReg.loc2Type() = ' ';
    start = 0;
    stop = 0;
    LocRestrictionValidator3601 locRestrictionValidator;
    CPPUNIT_ASSERT(
        locRestrictionValidator.validateDestination(trx, taxResponse, taxCodeReg, start, stop));
  }

  void testValidateDestination_isFarthest_shouldNotValidate()
  {
    start = 0;
    stop = 0;
    LocRestrictionValidator3601 locRestrictionValidator;
    locRestrictionValidator._farthestPointFound = true;
    locRestrictionValidator._farthestSegIndex = 1;
    CPPUNIT_ASSERT(
        !locRestrictionValidator.validateDestination(trx, taxResponse, taxCodeReg, start, stop));
  }

  void testValidateDestination_isFarthest_shouldValidate()
  {
    start = 0;
    stop = 0;
    LocRestrictionValidator3601 locRestrictionValidator;
    locRestrictionValidator._farthestPointFound = true;
    locRestrictionValidator._farthestSegIndex = 0;
    CPPUNIT_ASSERT(
        locRestrictionValidator.validateDestination(trx, taxResponse, taxCodeReg, start, stop));
  }

  void tearDown() { _memHandle.clear(); }

private:
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxSP3601Test);
} // namespace tse
