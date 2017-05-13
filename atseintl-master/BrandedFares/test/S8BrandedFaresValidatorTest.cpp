//-------------------------------------------------------------------
//
//  File:        S8BrandedFaresSelectorTest.cpp
//  Created:     2013
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------


#include "test/include/CppUnitHelperMacros.h"

#include "BrandedFares/test/S8BrandedFaresSelectorInitializer.h"
#include "BrandedFares/S8BrandedFaresSelector.h"

using namespace std;
namespace tse
{
class TestDiagnosticInitializer
{
public:
  TestDiagnosticInitializer(PricingTrx&, Diag889Collector*&)
  {
  }
};

class S8BrandedFaresValidatorTest : public CppUnit::TestFixture, S8BrandedFaresSelectorInitializer<S8BrandedFaresSelector>
{
  CPPUNIT_TEST_SUITE(S8BrandedFaresValidatorTest);
  CPPUNIT_TEST(testValidateT189DataPass);
  CPPUNIT_TEST(testValidateT189DataFailOwrt);
  CPPUNIT_TEST(testValidateT189DataFailMatchRuleTariff);
  CPPUNIT_TEST(testValidateT189DataFailRule);
  CPPUNIT_TEST(testValidateT189DataFailFareClass);
  CPPUNIT_TEST(testValidateT189DataFailFareType);
  CPPUNIT_TEST(testValidateT189DataFailApplNegative);
  CPPUNIT_TEST(testValidateT189DataFailPassengerType);
  CPPUNIT_TEST(testValidateT189DataFailRouting);
  CPPUNIT_TEST(testValidateT189DataFailSecondaryBookingCode);
  CPPUNIT_TEST(testValidateT189DataFailSource);
  CPPUNIT_TEST(testValidateT189DataFailRange2Currency);
  CPPUNIT_TEST(testValidateT189DataFailRange2Decimal);
  CPPUNIT_TEST(testValidateT189DataFailRange2MinAmount);
  CPPUNIT_TEST(testValidateT189DataFailRange2MaxAmount);
  CPPUNIT_TEST(testValidateT189DataFailApplIndNegative);
  CPPUNIT_TEST(testValidateBookingCodeNoStatus);
  CPPUNIT_TEST(testValidateBookingCodeFailPrimeRBD);
  CPPUNIT_TEST(testValidateBookingCodeFailSecT189);
  CPPUNIT_TEST(testGetFBRBookingCode);
  CPPUNIT_TEST(testIndustryFareBookingCodeFail);
  CPPUNIT_TEST(testIndustryFareBookingCodePass);
  CPPUNIT_TEST(testGetOneCharBookingCode);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    init();
    _s8BrandedFaresSelector = _memHandle.create<S8BrandedFaresSelector>(*_trx);
    _fdS8BrandedFaresSelector = _memHandle.create<S8BrandedFaresSelector>(*_fdTrx);
  }

  void tearDown() { _memHandle.clear(); }

  void testValidateT189DataPass()
  {
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->validateT189Data(_ptf1,
                                                             _svcFeesFareIdInfo,
                                                             *_svcFeesFareIdInfoVector,
                                                             *_secSecSvcFeesFareIdInfoVector,
                                                             _bProgram1) == PASS_T189);
  }

  void testValidateT189DataFailOwrt()
  {
    _svcFeesFareIdInfo->owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->validateT189Data(_ptf1,
                                                             _svcFeesFareIdInfo,
                                                             *_svcFeesFareIdInfoVector,
                                                             *_secSecSvcFeesFareIdInfoVector,
                                                             _bProgram1) == FAIL_OWRT);
  }

  void testValidateT189DataFailMatchRuleTariff()
  {
    _svcFeesFareIdInfo->ruleTariffInd() = "PRI";
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->validateT189Data(_ptf1,
                                                             _svcFeesFareIdInfo,
                                                             *_svcFeesFareIdInfoVector,
                                                             *_secSecSvcFeesFareIdInfoVector,
                                                             _bProgram1) == FAIL_RULE_TARIFF_IND);
  }

  void testValidateT189DataFailRule()
  {
    _svcFeesFareIdInfo->rule() = "345";
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->validateT189Data(_ptf1,
                                                             _svcFeesFareIdInfo,
                                                             *_svcFeesFareIdInfoVector,
                                                             *_secSecSvcFeesFareIdInfoVector,
                                                             _bProgram1) == FAIL_RULE);
  }

  void testValidateT189DataFailFareClass()
  {
    _svcFeesFareIdInfo->fareClass() = "FCTest";
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->validateT189Data(_ptf1,
                                                             _svcFeesFareIdInfo,
                                                             *_svcFeesFareIdInfoVector,
                                                             *_secSecSvcFeesFareIdInfoVector,
                                                             _bProgram1) == FAIL_FARECLASS);
  }

  void testValidateT189DataFailFareType()
  {
    _svcFeesFareIdInfo->fareType() = "Tes";
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->validateT189Data(_ptf1,
                                                             _svcFeesFareIdInfo,
                                                             *_svcFeesFareIdInfoVector,
                                                             *_secSecSvcFeesFareIdInfoVector,
                                                             _bProgram1) == FAIL_FARETYPE);
  }

  void testValidateT189DataFailApplNegative()
  {
    _svcFeesFareIdInfo->fareApplInd() = 'N';
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->validateT189Data(_ptf1,
                                                             _svcFeesFareIdInfo,
                                                             *_svcFeesFareIdInfoVector,
                                                             *_secSecSvcFeesFareIdInfoVector,
                                                             _bProgram1) == APPL_NEGATIVE);
  }

  void testValidateT189DataFailPassengerType()
  {
    _svcFeesFareIdInfo->paxType() = "ABC";
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->validateT189Data(_ptf1,
                                                             _svcFeesFareIdInfo,
                                                             *_svcFeesFareIdInfoVector,
                                                             *_secSecSvcFeesFareIdInfoVector,
                                                             _bProgram1) == FAIL_PAXTYPE);
  }

  void testValidateT189DataFailRouting()
  {
    _svcFeesFareIdInfo->routing() = 11111;
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->validateT189Data(_ptf1,
                                                             _svcFeesFareIdInfo,
                                                             *_svcFeesFareIdInfoVector,
                                                             *_secSecSvcFeesFareIdInfoVector,
                                                             _bProgram1) == FAIL_ROUTING);
  }

  void testValidateT189DataFailSecondaryBookingCode()
  {
    _svcFeesFareIdInfo->bookingCode1() = "D";
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->validateT189Data(_ptf1,
                                                             _svcFeesFareIdInfo,
                                                             *_svcFeesFareIdInfoVector,
                                                             *_secSecSvcFeesFareIdInfoVector,
                                                             _bProgram1) == FAIL_SEC_T189);
  }

  void testValidateT189DataFailSource()
  {
    _svcFeesFareIdInfo->source() = 'B';
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->validateT189Data(_ptf1,
                                                             _svcFeesFareIdInfo,
                                                             *_svcFeesFareIdInfoVector,
                                                             *_secSecSvcFeesFareIdInfoVector,
                                                             _bProgram1) == FAIL_SOURCE);
  }

  void testValidateT189DataFailRange2Currency()
  {
    _svcFeesFareIdInfo->cur1() = "USD";
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->validateT189Data(_ptf1,
                                                             _svcFeesFareIdInfo,
                                                             *_svcFeesFareIdInfoVector,
                                                             *_secSecSvcFeesFareIdInfoVector,
                                                             _bProgram1) == FAIL_RANGE2_CURR);
  }

  void testValidateT189DataFailRange2Decimal()
  {
    _fi1->currency() = "AUD";
    _fi1->noDec() = 2;

    _svcFeesFareIdInfo->cur1() = "USD";
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->validateT189Data(_ptf1,
                                                             _svcFeesFareIdInfo,
                                                             *_svcFeesFareIdInfoVector,
                                                             *_secSecSvcFeesFareIdInfoVector,
                                                             _bProgram1) == FAIL_RANGE2_DECIMAL);
  }

  void testValidateT189DataFailRange2MinAmount()
  {
    _fi1->currency() = "AUD";
    _fi1->noDec() = 3;
    _fi1->_fareAmount = 19.0;
    _svcFeesFareIdInfo->cur1() = "USD";
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->validateT189Data(_ptf1,
                                                             _svcFeesFareIdInfo,
                                                             *_svcFeesFareIdInfoVector,
                                                             *_secSecSvcFeesFareIdInfoVector,
                                                             _bProgram1) == FAIL_RANGE2_MIN);
  }

  void testValidateT189DataFailRange2MaxAmount()
  {
    _fi1->currency() = "AUD";
    _fi1->noDec() = 3;
    _fi1->_fareAmount = 778.0;
    _svcFeesFareIdInfo->cur1() = "USD";
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->validateT189Data(_ptf1,
                                                             _svcFeesFareIdInfo,
                                                             *_svcFeesFareIdInfoVector,
                                                             *_secSecSvcFeesFareIdInfoVector,
                                                             _bProgram1) == FAIL_RANGE2_MAX);
  }

  void testValidateT189DataFailApplIndNegative()
  {
    _svcFeesFareIdInfo->fareApplInd() = 'N';
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->validateT189Data(_ptf1,
                                                             _svcFeesFareIdInfo,
                                                             *_svcFeesFareIdInfoVector,
                                                             *_secSecSvcFeesFareIdInfoVector,
                                                             _bProgram1) == APPL_NEGATIVE);
  }

  void testValidateBookingCodeFailPrimeRBD()
  {
    bool matchSecondaryT189 = false;
    _svcFeesFareIdInfo->bookingCode1() = "FN";
    _svcFeesFareIdInfo->bookingCode2() = "";
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->validateBookingCode(_ptf1,
                                                                _svcFeesFareIdInfo,
                                                                *_secSvcFeesFareIdInfoVector,
                                                                matchSecondaryT189,
                                                                *_secSecSvcFeesFareIdInfoVector) ==
                   NO_STATUS);
  }

  void testValidateBookingCodeFailSecT189()
  {
    bool matchSecondaryT189 = false;
    _svcFeesFareIdInfo->bookingCode1() = "FN";
    _svcFeesFareIdInfo->bookingCode2() = "A";

    _secSvcFeesFareIdInfo->bookingCode1() = "FN";
    _secSvcFeesFareIdInfo->bookingCode2() = "";

    CPPUNIT_ASSERT(_s8BrandedFaresSelector->validateBookingCode(_ptf1,
                                                                _svcFeesFareIdInfo,
                                                                *_secSvcFeesFareIdInfoVector,
                                                                matchSecondaryT189,
                                                                *_secSecSvcFeesFareIdInfoVector) ==
                   FAIL_SEC_T189);
  }

  void testValidateBookingCodeNoStatus()
  {
    bool matchSecondaryT189 = false;
    CPPUNIT_ASSERT(_s8BrandedFaresSelector->validateBookingCode(_ptf1,
                                                                _svcFeesFareIdInfo,
                                                                *_secSvcFeesFareIdInfoVector,
                                                                matchSecondaryT189,
                                                                *_secSecSvcFeesFareIdInfoVector) ==
                   NO_STATUS);
  }

  void testGetFBRBookingCode()
  {
    _ptf1->status().set(PaxTypeFare::PTF_FareByRule);
    (*_ptf1->paxTypeFareRuleDataMap())[25] = _ptfARD;
    std::vector<BookingCode> primeBookingCodeVec;
    _ptf1->setRuleData(RuleConst::FARE_BY_RULE, _trx->dataHandle(), _fbrPaxTypeFareRuleData, true);
    _s8BrandedFaresSelector->getFBRBookingCode(_ptf1, primeBookingCodeVec);
    CPPUNIT_ASSERT(!primeBookingCodeVec.empty());
    CPPUNIT_ASSERT_EQUAL(4, (int)primeBookingCodeVec.size());
  }

  void testIndustryFareBookingCodeFail()
  {
    bool matchSecondaryT189 = false;
    _svcFeesFareIdInfo->bookingCode1() = "FN";
    _svcFeesFareIdInfo->bookingCode2() = "A";

    _secSvcFeesFareIdInfo->bookingCode1() = "FN";
    _secSvcFeesFareIdInfo->bookingCode2() = "";
    _ptf1->fare()->status().set(Fare::FS_IndustryFare, true);
    _ptf1->bookingCode() = "Z$";
    CPPUNIT_ASSERT(_fdS8BrandedFaresSelector->validateBookingCode(
                       _ptf1,
                       _svcFeesFareIdInfo,
                       *_secSvcFeesFareIdInfoVector,
                       matchSecondaryT189,
                       *_secSecSvcFeesFareIdInfoVector) == FAIL_SEC_T189);
  }

  void testIndustryFareBookingCodePass()
  {
    bool matchSecondaryT189 = false;
    _svcFeesFareIdInfo->bookingCode1() = "FN";
    _svcFeesFareIdInfo->bookingCode2() = "A";
    _ptf1->fareClassAppInfo() = _fareClassAppInfo;
    _ptf1->fareClassAppSegInfo() = _fareClassAppSegInfo;
    _fareClassAppInfo->_segs.push_back(_fareClassAppSegInfo);
    _fareClassAppSegInfo->_bookingCode[0] = "F";
    _ptf1->fare()->status().set(Fare::FS_IndustryFare, true);
    CPPUNIT_ASSERT(_fdS8BrandedFaresSelector->validateBookingCode(
                       _ptf1,
                       _svcFeesFareIdInfo,
                       *_secSvcFeesFareIdInfoVector,
                       matchSecondaryT189,
                       *_secSecSvcFeesFareIdInfoVector) == NO_STATUS);
  }
  void testGetOneCharBookingCode()
  {
    BookingCode bookingCode = "ZN";
    _s8BrandedFaresSelector->getOneCharBookingCode(bookingCode);
    CPPUNIT_ASSERT(strcmp(bookingCode.c_str(), "Z") == 0);
  }

};
CPPUNIT_TEST_SUITE_REGISTRATION(S8BrandedFaresValidatorTest);
}
