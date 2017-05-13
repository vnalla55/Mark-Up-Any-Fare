//----------------------------------------------------------------------------
//	File: FBDisplayTest.cpp
//
//	Author: Partha Kumar Chakraborti
//  	Created:      05/20/2005
//  	Description:  This is a unit test class for FBDisplayTest.cpp
//
//  Copyright Sabre 2005
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

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FBCategoryRuleRecord.h"
#include "DataModel/FBDisplay.h"
#include "DataModel/Itin.h"
#include "DataModel/TaxRequest.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TaxTrx.h"
#include "DBAccess/DBAForwardDecl.h"
#include "DBAccess/FareByRuleCtrlInfo.h"
#include "DBAccess/FootNoteCtrlInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxReissue.h"
#include "Taxes/LegacyTaxes/Reissue.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "Taxes/LegacyTaxes/TaxDisplayList.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include <cstdio>
#include <iostream>
#include <string>
#include <time.h>
#include <unistd.h>

#include <log4cxx/propertyconfigurator.h>

using namespace boost;

namespace tse
{
class FBDisplayMock : public FBDisplay
{
public:
  FBDisplayMock() {}
  virtual ~FBDisplayMock() {}

protected:
private:
  static log4cxx::LoggerPtr _logger;
};

class FBDisplayTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FBDisplayTest);

  CPPUNIT_TEST(testgetCategoryApplicability);
  CPPUNIT_TEST(testsetData);
  CPPUNIT_TEST(testsetgetRuleRecordData);
  CPPUNIT_TEST(testinitialize);
  CPPUNIT_TEST_SUITE_END();

public:
  // ------------------------------------------------------
  // @MethodName  setUp()
  // ------------------------------------------------------
  void setUp()
  {
    LOG4CXX_DEBUG(_logger, "Enter setUp()");

    // ----------------------------------------------------------------------
    //	Creare an object put the reference to it so that everyone can use it.
    // ----------------------------------------------------------------------
    _fbDMock = _memHandle.create<FBDisplayMock>();

    LOG4CXX_DEBUG(_logger, "Leaving setUp(),_fbDMock:" << _fbDMock);
    return;
  }

  // -------------------------------------------------------------------
  // @MethodName  tearDown()
  // -----------------------------------------------------------
  void tearDown()
  {
    LOG4CXX_DEBUG(_logger, "Enter tearDown()");

    _memHandle.clear();

    LOG4CXX_DEBUG(_logger, "Leaving tearDown()");

    return;
  }

  // -------------------------------------------------------------------
  // @MethodName  testgetCategoryApplicability()
  // -----------------------------------------------------------
  void testgetCategoryApplicability()
  {
    LOG4CXX_DEBUG(_logger, "Enter testgetCategoryApplicability()");

    char ret;
    // FBCategoryRuleRecord fbCRR ;
    FBDisplay fbDisplay;
    DataHandle dataHandle;
    // ---------------------------
    // Test without initializing
    // ---------------------------
    ret = fbDisplay.getCategoryApplicability(false, 1);
    CPPUNIT_ASSERT_EQUAL(NO_PARAM, ret);

    fbDisplay.setData(1, 0, 0, 0, "Y26", dataHandle);
    ret = fbDisplay.getCategoryApplicability(false, 1);
    CPPUNIT_ASSERT_EQUAL(NO_PARAM, ret);

    fbDisplay.setData(1, 0, 0, 0, "Y26", dataHandle);
    ret = fbDisplay.getCategoryApplicability(true, 1);
    CPPUNIT_ASSERT_EQUAL(NO_PARAM, ret);

    // --------------------------------------------------
    // FareRule = yes, FootNote = empty, General = empty
    // --------------------------------------------------

    FareRuleRecord2Info fRR2;
    fRR2.addItemInfoSetNosync(new CategoryRuleItemInfoSet());

    // --------------------------------------------------
    // FareRule = empty, FootNote = empty, General = yes
    // isDutyCode7Or8 = true
    // Expected result : $
    // --------------------------------------------------

    GeneralRuleRecord2Info gRR2;
    gRR2.addItemInfoSetNosync(new CategoryRuleItemInfoSet());
    // FIXME add record to grr2
    fbDisplay.setData(2, 0, 0, &gRR2, "Y26", dataHandle);
    ret = fbDisplay.getCategoryApplicability(true, 2);
    CPPUNIT_ASSERT_EQUAL('$', ret);

    // --------------------------------------------------
    // FareRule = empty, FootNote = yes, General = empty
    // isDutyCode7Or8 = true
    // Expected result : -
    // --------------------------------------------------

    FootNoteRecord2Info fNR2;
    fNR2.addItemInfoSetNosync(new CategoryRuleItemInfoSet());

    fbDisplay.setData(3, 0, &fNR2, 0, "Y26", dataHandle);
    ret = fbDisplay.getCategoryApplicability(true, 3);
    CPPUNIT_ASSERT_EQUAL('-', ret);

    // --------------------------------------------------
    // For any other combination
    // and isDutyCode7Or8 = true
    // Expected result : /
    // --------------------------------------------------
    fbDisplay.setData(4, &fRR2, &fNR2, 0, "Y26", dataHandle);
    ret = fbDisplay.getCategoryApplicability(true, 4);
    CPPUNIT_ASSERT(ret == '/');

    // ============================================
    // Test for CAT10
    // ============================================

    // --------------------------------------
    // Combinability set= NO
    // isDutyCode7Or* = false
    // Expected result: ' '
    // --------------------------------------
    ret = fbDisplay.getCategoryApplicability(false, 10);
    CPPUNIT_ASSERT(ret == NO_PARAM);

    // --------------------------------------
    // Combinability set= NO
    // isDutyCode7Or* = true
    // Expected result: ' '
    // --------------------------------------
    ret = fbDisplay.getCategoryApplicability(true, 10);
    CPPUNIT_ASSERT(ret == NO_PARAM);

    // --------------------------------------
    // Combinability set: yes
    // isDutyCode7Or* = false
    // Expected result: *
    // --------------------------------------
    CombinabilityRuleInfo cRI;
    fbDisplay.setData(&cRI, tse::FareClassCode("Y26"), dataHandle, true);
    ret = fbDisplay.getCategoryApplicability(false, 10);
    CPPUNIT_ASSERT_EQUAL('*', ret); //*

    // ============================================
    // Test for CAT25
    // ============================================

    // --------------------------------------
    // FareByRule set= NO
    // isDutyCode7Or* = false
    // Expected result: ' '
    // --------------------------------------
    ret = fbDisplay.getCategoryApplicability(false, 25);
    CPPUNIT_ASSERT(ret == NO_PARAM);

    // --------------------------------------
    // FareByRule set= NO
    // isDutyCode7Or* = true
    // Expected result: ' '
    // --------------------------------------
    ret = fbDisplay.getCategoryApplicability(true, 25);
    CPPUNIT_ASSERT_EQUAL(NO_PARAM, ret);

    // --------------------------------------
    // FareByRule set: yes
    // isDutyCode7Or* = false
    // Expected result: *
    // --------------------------------------
    FareByRuleCtrlInfo cFBRCI;
    cFBRCI.addItemInfoSetNosync(new CategoryRuleItemInfoSet());
    fbDisplay.setData(&cFBRCI, "Y26", dataHandle);
    ret = fbDisplay.getCategoryApplicability(false, 25);
    CPPUNIT_ASSERT_EQUAL('*', ret); //*

    LOG4CXX_DEBUG(_logger, "Leaving testgetCategoryApplicability()");

    return;
  }

  // -------------------------------------------------------------------
  // @MethodName  testsetData()
  // -----------------------------------------------------------
  void testsetData()
  {

    LOG4CXX_DEBUG(_logger, "Enter testsetData()");

    FBCategoryRuleRecord* pfbCRR;
    FBDisplay fbDisplay;
    DataHandle dataHandle;
    FareRuleRecord2Info fRR2;
    fRR2.addItemInfoSetNosync(new CategoryRuleItemInfoSet());

    // ------------------------------
    // Seting Data for except CAT10, 25
    // ------------------------------
    fbDisplay.setData(1, &fRR2, 0, 0, "Y26", dataHandle);
    pfbCRR = fbDisplay.getRuleRecordData(1);

    CPPUNIT_ASSERT(pfbCRR->fareRuleRecord2Info() == &fRR2);
    CPPUNIT_ASSERT(pfbCRR->footNoteRecord2Info() == 0);
    CPPUNIT_ASSERT(pfbCRR->generalRuleRecord2Info() == 0);
    CPPUNIT_ASSERT(pfbCRR->combinabilityRecord2Info() == 0);
    CPPUNIT_ASSERT(pfbCRR->fareByRuleRecord2Info() == 0);

    CPPUNIT_ASSERT(pfbCRR->fareBasis() == "Y26");

    CPPUNIT_ASSERT_EQUAL(true, pfbCRR->hasFareRuleRecord2Info());
    CPPUNIT_ASSERT_EQUAL(false, pfbCRR->hasFootNoteRecord2Info());
    CPPUNIT_ASSERT_EQUAL(false, pfbCRR->hasGeneralRuleRecord2Info());
    CPPUNIT_ASSERT_EQUAL(false, pfbCRR->hasCombinabilityRecord2Info());
    CPPUNIT_ASSERT_EQUAL(false, pfbCRR->hasFareByRuleRecord2Info());

    // ------------------------------
    // Seting Data for except CAT10
    // ------------------------------
    CombinabilityRuleInfo cRI;
    fbDisplay.setData(&cRI, tse::FareClassCode("Y26"), dataHandle, true);

    pfbCRR = fbDisplay.getRuleRecordData(10);

    CPPUNIT_ASSERT(pfbCRR->fareRuleRecord2Info() == 0);
    CPPUNIT_ASSERT(pfbCRR->footNoteRecord2Info() == 0);
    CPPUNIT_ASSERT(pfbCRR->generalRuleRecord2Info() == 0);
    CPPUNIT_ASSERT(pfbCRR->combinabilityRecord2Info() == &cRI);
    CPPUNIT_ASSERT(pfbCRR->fareByRuleRecord2Info() == 0);
    CPPUNIT_ASSERT(pfbCRR->fareBasis() == "Y26");

    CPPUNIT_ASSERT(pfbCRR->hasFareRuleRecord2Info() == false);
    CPPUNIT_ASSERT(pfbCRR->hasFootNoteRecord2Info() == false);
    CPPUNIT_ASSERT(pfbCRR->hasGeneralRuleRecord2Info() == false);
    CPPUNIT_ASSERT(pfbCRR->hasCombinabilityRecord2Info() == true);
    CPPUNIT_ASSERT(pfbCRR->hasFareByRuleRecord2Info() == false);

    // ------------------------------
    // Seting Data for except CAT25
    // ------------------------------
    pfbCRR->combinabilityRecord2Info() = 0;

    FareByRuleCtrlInfo cFBRCI;
    cFBRCI.addItemInfoSetNosync(new CategoryRuleItemInfoSet());
    fbDisplay.setData(&cFBRCI, "Y26", dataHandle);

    pfbCRR = fbDisplay.getRuleRecordData(25);

    CPPUNIT_ASSERT(pfbCRR->fareRuleRecord2Info() == 0);
    CPPUNIT_ASSERT(pfbCRR->footNoteRecord2Info() == 0);
    CPPUNIT_ASSERT(pfbCRR->generalRuleRecord2Info() == 0);
    CPPUNIT_ASSERT(pfbCRR->combinabilityRecord2Info() == 0);
    CPPUNIT_ASSERT(pfbCRR->fareByRuleRecord2Info() == &cFBRCI);
    CPPUNIT_ASSERT(pfbCRR->fareBasis() == "Y26");

    CPPUNIT_ASSERT(pfbCRR->hasFareRuleRecord2Info() == false);
    CPPUNIT_ASSERT(pfbCRR->hasFootNoteRecord2Info() == false);
    CPPUNIT_ASSERT(pfbCRR->hasGeneralRuleRecord2Info() == false);
    CPPUNIT_ASSERT(pfbCRR->hasCombinabilityRecord2Info() == false);
    CPPUNIT_ASSERT(pfbCRR->hasFareByRuleRecord2Info() == true);

    LOG4CXX_DEBUG(_logger, "Leaving testsetData()");
  }

  // -------------------------------------------------------------------
  // @MethodName  setgetRuleRecordData()
  // -----------------------------------------------------------
  void testsetgetRuleRecordData()
  {
    LOG4CXX_DEBUG(_logger, "Enter setgetRuleRecordData()");

    FBCategoryRuleRecord fbCRR, *pfbCRR;

    FBDisplay fbDisplay;
    fbDisplay.setRuleRecordData(1, &fbCRR);

    pfbCRR = fbDisplay.getRuleRecordData(1);

    CPPUNIT_ASSERT(&fbCRR == pfbCRR);

    LOG4CXX_DEBUG(_logger, "Leaving setgetRuleRecordData()");
  }

  // -------------------------------------------------------------------
  // @MethodName  testinitialize()
  // -----------------------------------------------------------
  void testinitialize()
  {
    LOG4CXX_DEBUG(_logger, "Enter testinitialize()");

    // ------------------------------
    // Initializing the FBDisplay class
    // --------------------------------
    std::vector<CatNumber> rC;
    rC.push_back(1);
    rC.push_back(2);
    rC.push_back(3);
    rC.push_back(4);
    rC.push_back(5);

    FareDisplayOptions fdOptions;
    fdOptions.ruleCategories() = rC;

    FareDisplayTrx trx;
    trx.setOptions(&fdOptions);

    Itin itin;
    trx.itin().push_back(&itin);

    FBDisplay fbDisplay;
    fbDisplay.initialize(trx);

    // ----------------------------
    //  Testing initialization
    // ---------------------------
    FBCategoryRuleRecord* fbCRR = 0;
    fbCRR = fbDisplay.getRuleRecordData(1);
    CPPUNIT_ASSERT(fbCRR != 0);

    fbCRR = fbDisplay.getRuleRecordData(2);
    CPPUNIT_ASSERT(fbCRR != 0);

    fbCRR = fbDisplay.getRuleRecordData(3);
    CPPUNIT_ASSERT(fbCRR != 0);

    fbCRR = fbDisplay.getRuleRecordData(4);
    CPPUNIT_ASSERT(fbCRR != 0);

    fbCRR = fbDisplay.getRuleRecordData(5);
    CPPUNIT_ASSERT(fbCRR != 0);

    fbCRR = fbDisplay.getRuleRecordData(6);
    CPPUNIT_ASSERT(fbCRR == 0);

    fbCRR = fbDisplay.getRuleRecordData(7);
    CPPUNIT_ASSERT(fbCRR == 0);

    LOG4CXX_DEBUG(_logger, "Leaving testinitialize()");
  }

private:
  static log4cxx::LoggerPtr _logger;
  FBDisplay* _fbDMock;
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(FBDisplayTest);

log4cxx::LoggerPtr
FBDisplayMock::_logger(log4cxx::Logger::getLogger("atseintl.DataModel.test.FBDisplayMock"));

// ======================================================
//		class: FBDisplayTest
// ======================================================

log4cxx::LoggerPtr
FBDisplayTest::_logger(log4cxx::Logger::getLogger("atseintl.DataModel.test.FBDisplayTest"));
}
