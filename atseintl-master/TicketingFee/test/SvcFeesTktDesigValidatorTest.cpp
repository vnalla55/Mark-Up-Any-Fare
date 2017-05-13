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
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "TicketingFee/SvcFeesTktDesigValidator.h"
#include "DBAccess/SvcFeesTktDesignatorInfo.h"
#include "Diagnostic/SvcFeesDiagCollector.h"
#include "Diagnostic/DCFactory.h"
#include "DBAccess/DiskCache.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

namespace tse
{

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

namespace
{
class SvcFeesTktDesigValidatorMock : public SvcFeesTktDesigValidator
{
public:
  SvcFeesTktDesigValidatorMock(PricingTrx& trx) : SvcFeesTktDesigValidator(trx, nullptr) {}
  bool getTicketDesignators(std::set<TktDesignator>&) const { return false; }
};

class SvcFeesInputTktDesigValidatorMock : public SvcFeesInputTktDesigValidator
{
public:
  SvcFeesInputTktDesigValidatorMock(PricingTrx& trx, const FarePath& farePath)
    : SvcFeesInputTktDesigValidator(trx, *farePath.itin(), nullptr)
  {
  }

  const std::vector<SvcFeesTktDesignatorInfo*>& getSvcFeesTicketDesignator(const int itemNo) const
  {
    return _tktDesigInfos;
  }

  std::vector<SvcFeesTktDesignatorInfo*> _tktDesigInfos;
};

class SvcFeesOutputTktDesigValidatorMock : public SvcFeesOutputTktDesigValidator
{
public:
  SvcFeesOutputTktDesigValidatorMock(PricingTrx& trx, const FarePath& farePath)
    : SvcFeesOutputTktDesigValidator(trx,
                                     farePath,
                                     farePath.itin()->travelSeg().begin(),
                                     farePath.itin()->travelSeg().end(),
                                     NULL)
  {
  }

  const std::vector<SvcFeesTktDesignatorInfo*>& getSvcFeesTicketDesignator(const int itemNo) const
  {
    return _tktDesigInfos;
  }

  std::string getFareBasis(const PaxTypeFare& paxTypeFare) const
  {
    return paxTypeFare.fare() ? "A" : "A/DES";
  }

  std::vector<SvcFeesTktDesignatorInfo*> _tktDesigInfos;
};
}

class SvcFeesTktDesigValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SvcFeesTktDesigValidatorTest);

  CPPUNIT_TEST(testWildCardMatch_AloneInTheBeginning);
  CPPUNIT_TEST(testWildCardMatch_Beginning);
  CPPUNIT_TEST(testWildCardMatch_Beginning_NonEmptyWildcard);
  CPPUNIT_TEST(testWildCardMatch_Beginning_ImplicitWildcardAtEnd);
  CPPUNIT_TEST(testWildCardMatch_Beginning_Negative);
  CPPUNIT_TEST(testWildCardMatch_Beginning_DigitFail1);
  CPPUNIT_TEST(testWildCardMatch_Beginning_DigitFail2);
  CPPUNIT_TEST(testWildCardMatch_Beginning_DigitPass);

  CPPUNIT_TEST(testWildCardMatch_AtTheEnd);
  CPPUNIT_TEST(testWildCardMatch_AtTheEnd_EmptyWildcard);
  CPPUNIT_TEST(testWildCardMatch_AtTheEnd_DigitFail);
  CPPUNIT_TEST(testWildCardMatch_AtTheEnd_DigitPass);
  CPPUNIT_TEST(testWildCardMatch_Fail);

  CPPUNIT_TEST(testWildCardMatch_InTheMiddle);
  CPPUNIT_TEST(testWildCardMatch_InTheMiddle_EmptyWildcard);
  CPPUNIT_TEST(testWildCardMatch_InTheMiddle_ImplicitWildcardAtEnd);
  CPPUNIT_TEST(testWildCardMatch_InTheMiddle_Fail);

  CPPUNIT_TEST(testWildCardMatch_InputTktDesignatorShorterThanT173);
  CPPUNIT_TEST(testWildCardMatch_InputTktDesignatorInBackwardDirectionVSTktDesignT173);

  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _mh;
  PricingTrx* _trx = nullptr;

public:

  void setUp()
  {
    _mh(new TestConfigInitializer);
    _trx = _mh(new PricingTrx);
  }

  void tearDown()
  {
    _mh.clear();
  }

  // BEGINNING

  void testWildCardMatch_AloneInTheBeginning()
  {
    CPPUNIT_ASSERT(SvcFeesTktDesigValidatorMock(*_trx).wildCardMatch("*", "ABCD"));
  }

  void testWildCardMatch_Beginning()
  {
    CPPUNIT_ASSERT(SvcFeesTktDesigValidatorMock(*_trx).wildCardMatch("*XYZ", "AAXYZ"));
  }

  void testWildCardMatch_Beginning_NonEmptyWildcard()
  {
    CPPUNIT_ASSERT(!SvcFeesTktDesigValidatorMock(*_trx).wildCardMatch("*XYZ", "XYZ"));
  }

  void testWildCardMatch_Beginning_ImplicitWildcardAtEnd()
  {
    CPPUNIT_ASSERT(SvcFeesTktDesigValidatorMock(*_trx).wildCardMatch("*XYZ", "ABCXYZDEF"));
  }

  void testWildCardMatch_Beginning_Negative()
  {
    CPPUNIT_ASSERT(!SvcFeesTktDesigValidatorMock(*_trx).wildCardMatch("*XYZ", "AAXY"));
  }

  void testWildCardMatch_Beginning_DigitFail1()
  {
    CPPUNIT_ASSERT(!SvcFeesTktDesigValidatorMock(*_trx).wildCardMatch("*1YZ", "21YZ"));
  }

  void testWildCardMatch_Beginning_DigitFail2()
  {
    CPPUNIT_ASSERT(!SvcFeesTktDesigValidatorMock(*_trx).wildCardMatch("*XY1", "ABCXY12"));
  }

  void testWildCardMatch_Beginning_DigitPass()
  {
    CPPUNIT_ASSERT(SvcFeesTktDesigValidatorMock(*_trx).wildCardMatch("*1Y1", "X1Y1Z"));
  }

  // END

  void testWildCardMatch_AtTheEnd()
  {
    CPPUNIT_ASSERT(SvcFeesTktDesigValidatorMock(*_trx).wildCardMatch("XYZ*", "XYZABC"));
  }

  void testWildCardMatch_AtTheEnd_EmptyWildcard()
  {
    CPPUNIT_ASSERT(SvcFeesTktDesigValidatorMock(*_trx).wildCardMatch("XYZ*", "XYZ"));
  }

  void testWildCardMatch_AtTheEnd_DigitFail()
  {
    CPPUNIT_ASSERT(!SvcFeesTktDesigValidatorMock(*_trx).wildCardMatch("XY1*", "XY12"));
  }

  void testWildCardMatch_AtTheEnd_DigitPass()
  {
    CPPUNIT_ASSERT(SvcFeesTktDesigValidatorMock(*_trx).wildCardMatch("XY1*", "XY1A"));
  }

  void testWildCardMatch_Fail()
  {
    CPPUNIT_ASSERT(!SvcFeesTktDesigValidatorMock(*_trx).wildCardMatch("XYZ*", "XYAZ"));
  }

  // MIDDLE

  void testWildCardMatch_InTheMiddle()
  {
    CPPUNIT_ASSERT(SvcFeesTktDesigValidatorMock(*_trx).wildCardMatch("X*Z", "XYZ"));
  }

  void testWildCardMatch_InTheMiddle_EmptyWildcard()
  {
    CPPUNIT_ASSERT(SvcFeesTktDesigValidatorMock(*_trx).wildCardMatch("X*Z", "XZ"));
  }

  void testWildCardMatch_InTheMiddle_ImplicitWildcardAtEnd()
  {
    CPPUNIT_ASSERT(SvcFeesTktDesigValidatorMock(*_trx).wildCardMatch("X*Z", "XYZW"));
  }

  void testWildCardMatch_InTheMiddle_Fail()
  {
    CPPUNIT_ASSERT(!SvcFeesTktDesigValidatorMock(*_trx).wildCardMatch("X*Z", "XY"));
  }

  void testWildCardMatch_InputTktDesignatorShorterThanT173()
  {
    CPPUNIT_ASSERT(!SvcFeesTktDesigValidatorMock(*_trx).wildCardMatch("*ABCDEF", "ABCD"));
  }

  void testWildCardMatch_InputTktDesignatorInBackwardDirectionVSTktDesignT173()
  {
    CPPUNIT_ASSERT(!SvcFeesTktDesigValidatorMock(*_trx).wildCardMatch("A*D", "CDBA"));
  }
};

class SvcFeesInputTktDesigValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SvcFeesInputTktDesigValidatorTest);

  CPPUNIT_TEST(testValidate_Pass_ItemNo);
  CPPUNIT_TEST(testValidate_Fail_ReqTktDesEmpty);
  CPPUNIT_TEST(testValidate_Fail_InTktDesEmpty);
  CPPUNIT_TEST(testValidate_Fail_NotFoundInDB);
  CPPUNIT_TEST(testValidate_Fail_EmptyTktDes);
  CPPUNIT_TEST(testValidate_Fail_TktDes);
  CPPUNIT_TEST(testValidate_Pass_TktDes);

  CPPUNIT_TEST(testValidate_Pass_ItemNo_WithDiag);
  CPPUNIT_TEST(testValidate_Fail_ReqTktDesEmpty_WithDiag);
  CPPUNIT_TEST(testValidate_Fail_InTktDesEmpty_WithDiag);
  CPPUNIT_TEST(testValidate_Fail_NotFoundInDB_WithDiag);
  CPPUNIT_TEST(testValidate_Fail_EmptyTktDes_WithDiag);
  CPPUNIT_TEST(testValidate_Fail_TktDes_WithDiag);
  CPPUNIT_TEST(testValidate_Pass_TktDes_WithDiag);

  CPPUNIT_TEST_SUITE_END();

protected:
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  FarePath* _farePath;
  Itin* _itin;
  PricingUnit* _pricingUnit;
  AirSeg* _airSeg;
  PricingRequest* _request;
  SvcFeesInputTktDesigValidatorMock* _validator;
  SvcFeesTktDesignatorInfo* _tktDesigInfo;

public:
  void setUp()
  {
    _memHandle.create<SpecificTestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _request = _memHandle.create<PricingRequest>();
    _trx->setRequest(_request);
    _farePath = _memHandle.create<FarePath>();
    _airSeg = _memHandle.create<AirSeg>();

    _pricingUnit = _memHandle.create<PricingUnit>();
    _pricingUnit->travelSeg().push_back(_airSeg);
    _farePath->pricingUnit().push_back(_pricingUnit);
    _itin = _memHandle.create<Itin>();
    _itin->travelSeg().push_back(_airSeg);
    _farePath->itin() = _itin;

    _validator = _memHandle.insert(new SvcFeesInputTktDesigValidatorMock(*_trx, *_farePath));
    _tktDesigInfo = _memHandle.create<SvcFeesTktDesignatorInfo>();
    _tktDesigInfo->seqNo() = 10000;
    _tktDesigInfo->itemNo() = 5555;
    _tktDesigInfo->tktDesignator() = "ABCD";
    _validator->_tktDesigInfos.push_back(_tktDesigInfo);

    _request->tktDesignator().insert(std::make_pair(int16_t(0), _tktDesigInfo->tktDesignator()));
  }

  void tearDown() { _memHandle.clear(); }

  void createDiagnostic()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic877;
    SvcFeesDiagCollector* diag =
        dynamic_cast<SvcFeesDiagCollector*>(DCFactory::instance()->create(*_trx));
    diag->activate();
    _validator->_diag = diag;
  }

  // TESTS
  void testValidate_Pass_ItemNo() { CPPUNIT_ASSERT(_validator->validate(0)); }

  void testValidate_Fail_ReqTktDesEmpty()
  {
    _request->tktDesignator().clear();
    CPPUNIT_ASSERT(!_validator->validate(1));
  }

  void testValidate_Fail_InTktDesEmpty()
  {
    _request->tktDesignator().clear();
    _request->tktDesignator().insert(std::make_pair(int16_t(0), ""));
    CPPUNIT_ASSERT(!_validator->validate(1));
  }

  void testValidate_Fail_NotFoundInDB()
  {
    ((SvcFeesInputTktDesigValidatorMock*)_validator)->_tktDesigInfos.clear();
    CPPUNIT_ASSERT(!_validator->validate(1));
  }

  void testValidate_Fail_EmptyTktDes()
  {
    ((SvcFeesInputTktDesigValidatorMock*)_validator)->_tktDesigInfos.front()->tktDesignator() = "";
    CPPUNIT_ASSERT(!_validator->validate(1));
  }

  void testValidate_Fail_TktDes()
  {
    ((SvcFeesInputTktDesigValidatorMock*)_validator)->_tktDesigInfos.front()->tktDesignator() =
        "WRONG";
    CPPUNIT_ASSERT(!_validator->validate(1));
  }

  void testValidate_Pass_TktDes() { CPPUNIT_ASSERT(_validator->validate(1)); }

  void testValidate_Pass_ItemNo_WithDiag()
  {
    createDiagnostic();
    _validator->validate(0);
    CPPUNIT_ASSERT(_validator->_diag->str().find("TKT DESIGNATOR T173") != std::string::npos);
  }

  void testValidate_Fail_ReqTktDesEmpty_WithDiag()
  {
    createDiagnostic();
    _request->tktDesignator().clear();
    _validator->validate(1);
    CPPUNIT_ASSERT(_validator->_diag->str().find("INPUT DESIGNATOR MISSING") != std::string::npos);
  }

  void testValidate_Fail_InTktDesEmpty_WithDiag()
  {
    createDiagnostic();
    _request->tktDesignator().clear();
    _request->tktDesignator().insert(std::make_pair(int16_t(0), ""));
    _validator->validate(1);
    CPPUNIT_ASSERT(_validator->_diag->str().find("INPUT DESIGNATOR MISSING") != std::string::npos);
  }

  void testValidate_Fail_NotFoundInDB_WithDiag()
  {
    createDiagnostic();
    ((SvcFeesInputTktDesigValidatorMock*)_validator)->_tktDesigInfos.clear();
    _validator->validate(1);
    CPPUNIT_ASSERT_EQUAL(std::string::npos, _validator->_diag->str().find("FAIL"));
    CPPUNIT_ASSERT_EQUAL(std::string::npos, _validator->_diag->str().find("PASS"));
  }

  void testValidate_Fail_EmptyTktDes_WithDiag()
  {
    createDiagnostic();
    ((SvcFeesInputTktDesigValidatorMock*)_validator)->_tktDesigInfos.front()->tktDesignator() = "";
    _validator->validate(1);
    CPPUNIT_ASSERT(_validator->_diag->str().find("DESIGNATOR IS EMPTY") != std::string::npos);
  }

  void testValidate_Fail_TktDes_WithDiag()
  {
    createDiagnostic();
    ((SvcFeesInputTktDesigValidatorMock*)_validator)->_tktDesigInfos.front()->tktDesignator() =
        "WRONG";
    _validator->validate(1);
    CPPUNIT_ASSERT(_validator->_diag->str().find("FAIL") != std::string::npos);
  }

  void testValidate_Pass_TktDes_WithDiag()
  {
    createDiagnostic();
    _validator->validate(1);
    CPPUNIT_ASSERT(_validator->_diag->str().find("PASS") != std::string::npos);
  }
};

class SvcFeesOutputTktDesigValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SvcFeesOutputTktDesigValidatorTest);

  CPPUNIT_TEST(testGetTicketDesignators_EmptyPricingUnit);
  CPPUNIT_TEST(testGetTicketDesignators_EmptyFareUsage);
  CPPUNIT_TEST(testGetTicketDesignators_EmptyFareUsageSegs);
  CPPUNIT_TEST(testGetTicketDesignators_EmptyItinSegments);
  CPPUNIT_TEST(testGetTicketDesignators_Pass);
  CPPUNIT_TEST(testGetTicketDesignators_Pass_Request);
  CPPUNIT_TEST(testGetTicketDesignators_Fail_WithDiag);

  CPPUNIT_TEST(testValidate_Pass);
  CPPUNIT_TEST(testValidate_Fail);
  CPPUNIT_TEST(testValidate_Pass_WithDiag);
  CPPUNIT_TEST_SUITE_END();

protected:
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  AirSeg* _airSeg;
  std::vector<TravelSeg*> _travelSegs;
  std::set<TktDesignator>* _tktDesignators;
  SvcFeesOutputTktDesigValidator* _validator;
  SvcFeesTktDesignatorInfo* _tktDesigInfo;
  PricingRequest* _request;
  FareMarket* _fm;
  FareUsage* _fareUsage;
  FarePath* _farePath;
  PricingUnit* _pricingUnit;
  PaxTypeFare* _ptf;
  Itin* _itin;

public:
  void setUp()
  {
    _memHandle(new TestConfigInitializer);
    _trx = _memHandle.create<PricingTrx>();
    _request = _memHandle.create<PricingRequest>();
    _trx->setRequest(_request);
    _airSeg = _memHandle.create<AirSeg>();
    _travelSegs = {_airSeg};
    _tktDesignators = _memHandle.create<std::set<TktDesignator> >();

    _fm = _memHandle.create<FareMarket>();
    _fm->travelSeg() = _travelSegs;
    _ptf = _memHandle.create<PaxTypeFare>();
    _ptf->setFare(nullptr);
    _ptf->fareMarket() = _fm;
    _fareUsage = _memHandle.create<FareUsage>();
    _fareUsage->paxTypeFare() = _ptf;
    _fareUsage->travelSeg() = _travelSegs;

    _itin = _memHandle.create<Itin>();
    _itin->travelSeg() = _travelSegs;

    _pricingUnit = _memHandle.create<PricingUnit>();
    _pricingUnit->fareUsage().push_back(_fareUsage);
    _farePath = _memHandle.create<FarePath>();
    _farePath->pricingUnit().push_back(_pricingUnit);
    _farePath->itin() = _itin;

    SvcFeesOutputTktDesigValidatorMock* validator =
        _memHandle.insert(new SvcFeesOutputTktDesigValidatorMock(*_trx, *_farePath));
    _validator = validator;
    _tktDesigInfo = _memHandle.create<SvcFeesTktDesignatorInfo>();
    _tktDesigInfo->seqNo() = 10000;
    _tktDesigInfo->itemNo() = 5555;
    _tktDesigInfo->tktDesignator() = "ABCD";
    validator->_tktDesigInfos.push_back(_tktDesigInfo);
    _tktDesignators->insert(_tktDesigInfo->tktDesignator());
  }

  void tearDown() { _memHandle.clear(); }

  void createDiagnostic()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic877;
    SvcFeesDiagCollector* diag =
        dynamic_cast<SvcFeesDiagCollector*>(DCFactory::instance()->create(*_trx));
    diag->activate();
    _validator->_diag = diag;
  }

  // tests
  void testGetTicketDesignators_EmptyPricingUnit()
  {
    _farePath->pricingUnit().clear();
    _validator->getTicketDesignators(*_tktDesignators);
    CPPUNIT_ASSERT(_tktDesignators->empty());
  }

  void testGetTicketDesignators_EmptyFareUsage()
  {
    _pricingUnit->fareUsage().clear();
    _validator->getTicketDesignators(*_tktDesignators);
    CPPUNIT_ASSERT(_tktDesignators->empty());
  }

  void testGetTicketDesignators_EmptyFareUsageSegs()
  {
    _fareUsage->travelSeg().clear();
    _fm->travelSeg().clear();
    _validator->getTicketDesignators(*_tktDesignators);
    CPPUNIT_ASSERT(_tktDesignators->empty());
  }

  void testGetTicketDesignators_EmptyItinSegments()
  {
    _itin->travelSeg().clear();
    SvcFeesOutputTktDesigValidatorMock* validator =
        _memHandle.insert(new SvcFeesOutputTktDesigValidatorMock(*_trx, *_farePath));
    _itin->travelSeg().push_back(_airSeg);

    validator->getTicketDesignators(*_tktDesignators);
    CPPUNIT_ASSERT(_tktDesignators->empty());
  }

  void testGetTicketDesignators_Pass()
  {
    _validator->getTicketDesignators(*_tktDesignators);
    CPPUNIT_ASSERT_EQUAL((size_t)1, _tktDesignators->size());
  }

  void testGetTicketDesignators_Pass_Request()
  {
    _request->specifiedTktDesignator().insert(
        std::make_pair(int16_t(0), _tktDesigInfo->tktDesignator()));
    _ptf->setFare(_memHandle.create<Fare>());
    _validator->getTicketDesignators(*_tktDesignators);
    CPPUNIT_ASSERT_EQUAL((size_t)1, _tktDesignators->size());
  }

  void testGetTicketDesignators_Fail_WithDiag()
  {
    createDiagnostic();
    _ptf->setFare(_memHandle.create<Fare>());
    _validator->getTicketDesignators(*_tktDesignators);
    CPPUNIT_ASSERT(_tktDesignators->empty());
    CPPUNIT_ASSERT(_validator->_diag->str().find("OUTPUT DESIGNATOR MISSING") != std::string::npos);
  }

  void testValidate_Pass()
  {
    CPPUNIT_ASSERT(
        _validator->validate(*_tktDesignators, _validator->getSvcFeesTicketDesignator(1)));
  }

  void testValidate_Fail()
  {
    _tktDesignators->insert("WRONG");
    CPPUNIT_ASSERT(
        !_validator->validate(*_tktDesignators, _validator->getSvcFeesTicketDesignator(1)));
  }

  void testValidate_Pass_WithDiag()
  {
    createDiagnostic();
    _validator->validate(*_tktDesignators, _validator->getSvcFeesTicketDesignator(1));
    CPPUNIT_ASSERT(_validator->_diag->str().find("ABCD") != std::string::npos);
    CPPUNIT_ASSERT(_validator->_diag->str().find("ALL OUTPUT TKT DESIGNATORS PASSED") !=
                   std::string::npos);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SvcFeesTktDesigValidatorTest);
CPPUNIT_TEST_SUITE_REGISTRATION(SvcFeesInputTktDesigValidatorTest);
CPPUNIT_TEST_SUITE_REGISTRATION(SvcFeesOutputTktDesigValidatorTest);
}
