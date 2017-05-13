//-------------------------------------------------------------------
//
//  File:        FareClassQualifierTest.cpp
//  Author:      Doug Batchelor
//  Created:     Mar 14, 2006
//  Description: This class does unit tests of the FareClassQualifier,
//               class.
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//--------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "FareDisplay/FareClassQualifier.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayOptions.h"
#include "Rules/RuleConst.h"
#include "Common/TseConsts.h"
#include "FareDisplay/InclusionCodeConsts.h"
#include "DBAccess/PaxTypeInfo.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class FareClassQualifierTest : public CppUnit::TestFixture
{
  class PaxTypeFareMock : public PaxTypeFare
  {
  public:
    PaxTypeFareMock() {}
    std::string createFareBasisCodeFD(FareDisplayTrx& trx) const { return _fareBasisCode; }

    std::string _fareBasisCode;
  };

  CPPUNIT_TEST_SUITE(FareClassQualifierTest);
  CPPUNIT_TEST(testQualify_Valid_EmptyFBC);
  CPPUNIT_TEST(testQualify_FareClassMatch);
  CPPUNIT_TEST(testQualify_Valid);
  CPPUNIT_TEST(testQualify_PaxTypeCode);
  CPPUNIT_TEST(testQualify_Valid_ERD);
  CPPUNIT_TEST(testQualify_Valid_ShortRD);
  CPPUNIT_TEST(testQualify_Valid_AllFares);

  CPPUNIT_TEST(testSetup_Pass);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _fdTrx = _memHandle.create<FareDisplayTrx>();
    _ptFare = _memHandle.create<PaxTypeFareMock>();
    _fareInfo = _memHandle.create<FareInfo>();
    _fare = _memHandle.create<Fare>();
    _fm = _memHandle.create<FareMarket>();
    _tcrInfo = _memHandle.create<TariffCrossRefInfo>();
    _request = _memHandle.create<FareDisplayRequest>();
    _options = _memHandle.create<FareDisplayOptions>();
    _fcq = _memHandle.create<FareClassQualifier>();
    _pt = _memHandle.create<PaxType>();
    _ptInfo = _memHandle.create<PaxTypeInfo>();
    _tk = _memHandle.create<TktDesignator>();

    _ptInfo->adultInd() = 'N';
    _ptInfo->childInd() = 'N';
    _ptInfo->infantInd() = 'Y';
    _ptInfo->paxType() = INFANT;

    _pt->paxType() = _ptInfo->paxType();
    _pt->paxTypeInfo() = _ptInfo;
    _ptFare->actualPaxType() = _pt;

    _fareInfo->_vendor = Vendor::ATPCO;
    _fare->initialize(Fare::FS_Domestic, _fareInfo, *_fm, _tcrInfo);
    _ptFare->setFare(_fare);

    _fdTrx->setRequest(_request);
    _fdTrx->setOptions(_options);
  }

  void tearDown() { _memHandle.clear(); }

  void prepareTestData(const std::string& requestedFBC, const std::string& ptfFBC)
  {
    _request->fareBasisCode() = requestedFBC;
    _request->ticketDesignator() = *_tk;
    _request->requestedInclusionCode() = MILITARY_FARES;
    _options->lineNumber() = 0;
    ((PaxTypeFareMock*)_ptFare)->_fareBasisCode = ptfFBC;
    _ptFare->status().set(PaxTypeFare::PTF_Discounted);
  }

  // TESTS

  void testQualify_Valid_EmptyFBC()
  {
    _request->fareBasisCode() = "";
    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::FD_Valid, _fcq->qualify(*_fdTrx, *_ptFare));
  }

  void testQualify_FareClassMatch()
  {
    prepareTestData("AAA", "BBB/CCC");
    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::FD_FareClass_Match, _fcq->qualify(*_fdTrx, *_ptFare));
  }

  void testQualify_Valid()
  {
    prepareTestData("AAA", "AAA");
    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::FD_Valid, _fcq->qualify(*_fdTrx, *_ptFare));
  }

  void testQualify_PaxTypeCode()
  {
    prepareTestData("AAA", "AAA/BBB");
    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::FD_Pax_Type_Code, _fcq->qualify(*_fdTrx, *_ptFare));
  }

  void testQualify_Valid_ERD()
  {
    prepareTestData("AAA", "AAA/BBB");
    _request->requestType() = ENHANCED_RD_REQUEST;
    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::FD_Valid, _fcq->qualify(*_fdTrx, *_ptFare));
  }

  void testQualify_Valid_ShortRD()
  {
    prepareTestData("AAA", "AAA/BBB");
    _options->lineNumber() = 1;
    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::FD_Valid, _fcq->qualify(*_fdTrx, *_ptFare));
  }

  void testQualify_Valid_AllFares()
  {
    prepareTestData("AAA", "AAA/BBB");
    _request->requestedInclusionCode() = ALL_FARES;
    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::FD_Valid, _fcq->qualify(*_fdTrx, *_ptFare));
  }

  void testSetup_Pass() { CPPUNIT_ASSERT(_fcq->setup(*_fdTrx)); }

private:
  FareDisplayTrx* _fdTrx;
  PaxTypeFare* _ptFare;
  FareInfo* _fareInfo;
  Fare* _fare;
  FareMarket* _fm;
  TariffCrossRefInfo* _tcrInfo;
  FareDisplayRequest* _request;
  FareDisplayOptions* _options;
  FareClassQualifier* _fcq;
  PaxType* _pt;
  PaxTypeInfo* _ptInfo;
  TktDesignator* _tk;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(FareClassQualifierTest);
}
