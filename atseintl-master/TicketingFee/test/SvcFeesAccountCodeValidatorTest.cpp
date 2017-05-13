//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include "Common/TseBoostStringTypes.h"
#include "DataModel/PricingTrx.h"
#include "TicketingFee/SvcFeesAccountCodeValidator.h"
#include "DBAccess/SvcFeesAccCodeInfo.h"
#include "Diagnostic/SvcFeesDiagCollector.h"
#include "Diagnostic/DCFactory.h"
#include "DBAccess/DiskCache.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

namespace tse
{
namespace
{
class SvcFeesAccountCodeValidatorMock : public SvcFeesAccountCodeValidator
{
public:
  SvcFeesAccountCodeValidatorMock(PricingTrx& trx) : SvcFeesAccountCodeValidator(trx, NULL) {}

  const std::vector<SvcFeesAccCodeInfo*>& getSvcFeesAccCodeInfo(int itemNo) const
  {
    return _accCodeInfos;
  }

  std::vector<SvcFeesAccCodeInfo*> _accCodeInfos;
};
}

class SvcFeesAccountCodeValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SvcFeesAccountCodeValidatorTest);

  CPPUNIT_TEST(testValidate_Pass);
  CPPUNIT_TEST(testValidate_Fail_AccCode);
  CPPUNIT_TEST(testValidate_Pass_WithDiagnostic);

  CPPUNIT_TEST(testValidateAccCode_Pass_Single);
  CPPUNIT_TEST(testValidateAccCode_Fail_Single);
  CPPUNIT_TEST(testValidateAccCode_Fail_SingleNoInput);
  CPPUNIT_TEST(testValidateAccCode_Fail_SingleEmpty);
  CPPUNIT_TEST(testValidateAccCode_Pass_SingleAsteriskNoAccCode);
  CPPUNIT_TEST(testValidateAccCode_Pass_SingleAsteriskNoCorpId);
  CPPUNIT_TEST(testValidateAccCode_Pass_Multi);
  CPPUNIT_TEST(testValidateAccCode_Fail_Multi);
  CPPUNIT_TEST(testValidateAccCode_Pass_MultiAsteriskNoInput);

  CPPUNIT_TEST(testValidateMultiAccCode_Fail);
  CPPUNIT_TEST(testValidateMultiAccCode_Fail_NoInput);
  CPPUNIT_TEST(testValidateMultiAccCode_Fail_NoInputAndAccCode);
  CPPUNIT_TEST(testValidateMultiAccCode_Pass_AccCode);
  CPPUNIT_TEST(testValidateMultiAccCode_Pass_CorpId);
  CPPUNIT_TEST(testValidateMultiAccCode_Fail_AsteriskNoInput);

  CPPUNIT_TEST(testValidateSingleAccCode_Pass_AccCode);
  CPPUNIT_TEST(testValidateSingleAccCode_Fail_AccCode);
  CPPUNIT_TEST(testValidateSingleAccCode_Pass_CorpId);
  CPPUNIT_TEST(testValidateSingleAccCode_Fail_CorpId);
  CPPUNIT_TEST(testValidateSingleAccCode_Pass_CorpIdForAccCode);
  CPPUNIT_TEST(testValidateSingleAccCode_Fail_NoRequest);

  CPPUNIT_TEST(testMatchAccountCode_Pass_Equal);
  CPPUNIT_TEST(testMatchAccountCode_Fail_DifferentChar);
  CPPUNIT_TEST(testMatchAccountCode_Fail_TooShort);
  CPPUNIT_TEST(testMatchAccountCode_Fail_Empty);
  CPPUNIT_TEST(testMatchAccountCode_Pass_AsteriskBegins);
  CPPUNIT_TEST(testMatchAccountCode_Fail_AsteriskBegins);
  CPPUNIT_TEST(testMatchAccountCode_Pass_AsteriskEnds);
  CPPUNIT_TEST(testMatchAccountCode_Pass_AsteriskInMiddle);
  CPPUNIT_TEST(testMatchAccountCode_Fail_AsteriskInMiddle);

  CPPUNIT_TEST_SUITE_END();

  class SpecificTestConfigInitializer : public TestConfigInitializer
  {
  public:
    SpecificTestConfigInitializer()
    {
      DiskCache::initialize(_config);
      _memHandle.create<MockDataManager>();
    }

    ~SpecificTestConfigInitializer() { _memHandle.clear(); }

  private:
    TestMemHandle _memHandle;
  };

protected:
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  PricingRequest* _request;
  SvcFeesAccountCodeValidator* _validator;
  SvcFeesAccCodeInfo* _accCodeInfo;

public:
  void setUp()
  {
    _memHandle.create<SpecificTestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _request = _memHandle.create<PricingRequest>();
    _trx->setRequest(_request);

    SvcFeesAccountCodeValidatorMock* validator =
        _memHandle.insert(new SvcFeesAccountCodeValidatorMock(*_trx));
    _validator = validator;
    _accCodeInfo = _memHandle.create<SvcFeesAccCodeInfo>();

    _accCodeInfo->seqNo() = 10000;
    _accCodeInfo->itemNo() = 5555;
    _accCodeInfo->accountCode() = "ACC11";

    _request->isMultiAccCorpId() = false;
    _request->accountCode() = "ACC11";
    _request->corporateID() = "COR11";

    validator->_accCodeInfos.push_back(_accCodeInfo);
  }

  void tearDown() { _memHandle.clear(); }

  void setMultiInputAccCode()
  {
    _request->isMultiAccCorpId() = true;
    _request->corpIdVec().push_back("CRP01");
    _request->accCodeVec().push_back("MUL01");
  }

  // TESTS
  void testValidate_Pass() { CPPUNIT_ASSERT(_validator->validate(0)); }

  void testValidate_Fail_AccCode()
  {
    _request->accountCode() = "ACC22";
    CPPUNIT_ASSERT(!_validator->validate(0));
  }

  void testValidate_Pass_WithDiagnostic()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic877;
    SvcFeesDiagCollector* diag =
        dynamic_cast<SvcFeesDiagCollector*>(DCFactory::instance()->create(*_trx));
    diag->activate();
    _validator->_diag = diag;
    _validator->validate(0);
    CPPUNIT_ASSERT(diag->str().find("PASS") != std::string::npos);
  }

  void testValidateAccCode_Pass_Single()
  {
    CPPUNIT_ASSERT(_validator->validateAccCode(*_accCodeInfo));
  }

  void testValidateAccCode_Fail_Single()
  {
    _accCodeInfo->accountCode() = "BAD01";
    CPPUNIT_ASSERT(!_validator->validateAccCode(*_accCodeInfo));
  }

  void testValidateAccCode_Fail_SingleNoInput()
  {
    _accCodeInfo->accountCode() = "*";
    _request->accountCode() = "";
    _request->corporateID() = "";
    CPPUNIT_ASSERT(!_validator->validateAccCode(*_accCodeInfo));
  }

  void testValidateAccCode_Fail_SingleEmpty()
  {
    _accCodeInfo->accountCode() = "";
    CPPUNIT_ASSERT(!_validator->validateAccCode(*_accCodeInfo));
  }

  void testValidateAccCode_Pass_SingleAsteriskNoAccCode()
  {
    _accCodeInfo->accountCode() = "*";
    _request->accountCode() = "";
    CPPUNIT_ASSERT(_validator->validateAccCode(*_accCodeInfo));
  }

  void testValidateAccCode_Pass_SingleAsteriskNoCorpId()
  {
    _accCodeInfo->accountCode() = "*";
    _request->corporateID() = "";
    CPPUNIT_ASSERT(_validator->validateAccCode(*_accCodeInfo));
  }

  void testValidateAccCode_Pass_Multi()
  {
    setMultiInputAccCode();
    _accCodeInfo->accountCode() = "MUL01";
    CPPUNIT_ASSERT(_validator->validateAccCode(*_accCodeInfo));
  }

  void testValidateAccCode_Fail_Multi()
  {
    setMultiInputAccCode();

    CPPUNIT_ASSERT(!_validator->validateAccCode(*_accCodeInfo));
  }

  void testValidateAccCode_Pass_MultiAsteriskNoInput()
  {
    setMultiInputAccCode();
    _accCodeInfo->accountCode() = "*";
    _request->accountCode() = "";
    _request->corporateID() = "";
    CPPUNIT_ASSERT(_validator->validateAccCode(*_accCodeInfo));
  }

  void testValidateMultiAccCode_Fail()
  {
    setMultiInputAccCode();
    _accCodeInfo->accountCode() = "BAD01";
    CPPUNIT_ASSERT(!_validator->validateMultiAccCode(*_accCodeInfo));
  }

  void testValidateMultiAccCode_Fail_NoInput()
  {
    setMultiInputAccCode();
    _request->accountCode() = "";
    _request->corporateID() = "";
    CPPUNIT_ASSERT(!_validator->validateMultiAccCode(*_accCodeInfo));
  }

  void testValidateMultiAccCode_Fail_NoInputAndAccCode()
  {
    setMultiInputAccCode();
    _accCodeInfo->accountCode() = "";
    _request->accountCode() = "";
    _request->corporateID() = "";
    CPPUNIT_ASSERT(!_validator->validateMultiAccCode(*_accCodeInfo));
  }

  void testValidateMultiAccCode_Pass_AccCode()
  {
    setMultiInputAccCode();
    _accCodeInfo->accountCode() = "MUL01";
    CPPUNIT_ASSERT(_validator->validateMultiAccCode(*_accCodeInfo));
  }

  void testValidateMultiAccCode_Pass_CorpId()
  {
    setMultiInputAccCode();
    _accCodeInfo->accountCode() = "CRP01";
    CPPUNIT_ASSERT(_validator->validateMultiAccCode(*_accCodeInfo));
  }

  void testValidateMultiAccCode_Fail_AsteriskNoInput()
  {
    setMultiInputAccCode();
    _accCodeInfo->accountCode() = "*";
    _request->accountCode() = "";
    _request->corporateID() = "";
    CPPUNIT_ASSERT(!_validator->validateMultiAccCode(*_accCodeInfo));
  }

  void testValidateSingleAccCode_Pass_AccCode()
  {
    _accCodeInfo->accountCode() = "ACC11";
    CPPUNIT_ASSERT(_validator->validateSingleAccCode(*_accCodeInfo));
  }

  void testValidateSingleAccCode_Fail_AccCode()
  {
    _accCodeInfo->accountCode() = "ACC5";
    CPPUNIT_ASSERT(!_validator->validateSingleAccCode(*_accCodeInfo));
  }

  void testValidateSingleAccCode_Pass_CorpId()
  {
    _request->accountCode() = "";
    _accCodeInfo->accountCode() = "COR11";
    CPPUNIT_ASSERT(_validator->validateSingleAccCode(*_accCodeInfo));
  }

  void testValidateSingleAccCode_Fail_CorpId()
  {
    _request->accountCode() = "";
    _accCodeInfo->accountCode() = "COR5";
    CPPUNIT_ASSERT(!_validator->validateSingleAccCode(*_accCodeInfo));
  }

  void testValidateSingleAccCode_Pass_CorpIdForAccCode()
  {
    _request->accountCode() = "";
    _request->corporateID() = "ACC11";
    CPPUNIT_ASSERT(_validator->validateSingleAccCode(*_accCodeInfo));
  }

  void testValidateSingleAccCode_Fail_NoRequest()
  {
    _request->accountCode() = "";
    _request->corporateID() = "";
    CPPUNIT_ASSERT(!_validator->validateSingleAccCode(*_accCodeInfo));
  }

  void testMatchAccountCode_Pass_Equal()
  {
    CPPUNIT_ASSERT(_validator->matchAccountCode("ACCOUNT1234", "ACCOUNT1234"));
  }

  void testMatchAccountCode_Fail_DifferentChar()
  {
    CPPUNIT_ASSERT(!_validator->matchAccountCode("ACCOUNT1234", "ACCOUNT1239"));
  }

  void testMatchAccountCode_Fail_TooShort()
  {
    CPPUNIT_ASSERT(!_validator->matchAccountCode("ACCOUNT1234", "ACCOUNT12"));
  }

  void testMatchAccountCode_Fail_Empty()
  {
    CPPUNIT_ASSERT(!_validator->matchAccountCode("ACCOUNT1234", ""));
  }

  void testMatchAccountCode_Pass_AsteriskBegins()
  {
    CPPUNIT_ASSERT(_validator->matchAccountCode("*C21", "ABC21XX"));
    CPPUNIT_ASSERT(_validator->matchAccountCode("*C21", "ABC21XX50"));

    CPPUNIT_ASSERT(_validator->matchAccountCode("*E70", "BE70NR"));
    CPPUNIT_ASSERT(_validator->matchAccountCode("*E70", "BE70"));
    CPPUNIT_ASSERT(_validator->matchAccountCode("*E70", "YNWE70"));
    CPPUNIT_ASSERT(_validator->matchAccountCode("*E70", "Q123E70R"));
    CPPUNIT_ASSERT(_validator->matchAccountCode("*E70", "HE70NR"));

    CPPUNIT_ASSERT(_validator->matchAccountCode("*M", "YM"));
  }

  void testMatchAccountCode_Fail_AsteriskBegins()
  {
    CPPUNIT_ASSERT(!_validator->matchAccountCode("*AB12", "AB123"));
    CPPUNIT_ASSERT(!_validator->matchAccountCode("*E70", "E70"));
    CPPUNIT_ASSERT(!_validator->matchAccountCode("*M", "M"));
  }

  void testMatchAccountCode_Pass_AsteriskEnds()
  {
    CPPUNIT_ASSERT(_validator->matchAccountCode("M*", "M"));

    CPPUNIT_ASSERT(_validator->matchAccountCode("ABC*", "ABCDE12"));
    CPPUNIT_ASSERT(_validator->matchAccountCode("ABC*", "ABC12"));
  }

  void testMatchAccountCode_Pass_AsteriskInMiddle()
  {
    CPPUNIT_ASSERT(_validator->matchAccountCode("AB*12", "ABC12"));
    CPPUNIT_ASSERT(_validator->matchAccountCode("AB*12", "ABC33Z12"));
    CPPUNIT_ASSERT(_validator->matchAccountCode("AB*12", "AB3C12"));

    CPPUNIT_ASSERT(_validator->matchAccountCode("AB*ZZ", "ABCZZ21"));
    CPPUNIT_ASSERT(_validator->matchAccountCode("AB*ZZ", "AB123ZZQ"));
    CPPUNIT_ASSERT(_validator->matchAccountCode("AB*ZZ", "ABCZZQ1"));
    CPPUNIT_ASSERT(_validator->matchAccountCode("AB*ZZ", "ABZZ"));
    CPPUNIT_ASSERT(_validator->matchAccountCode("AB*ZZ", "ABCZZ"));
  }

  void testMatchAccountCode_Fail_AsteriskInMiddle()
  {
    CPPUNIT_ASSERT(!_validator->matchAccountCode("AB*12", "ABC312"));
    CPPUNIT_ASSERT(!_validator->matchAccountCode("AB*12", "AB312"));
    CPPUNIT_ASSERT(!_validator->matchAccountCode("AB*12", "AB123"));
    CPPUNIT_ASSERT(!_validator->matchAccountCode("AB*12", "ABC123"));

    CPPUNIT_ASSERT(!_validator->matchAccountCode("B*E70", "BE170"));
    CPPUNIT_ASSERT(!_validator->matchAccountCode("B*E70", "BE710"));
    CPPUNIT_ASSERT(!_validator->matchAccountCode("B*E70", "BE701"));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SvcFeesAccountCodeValidatorTest);
}
