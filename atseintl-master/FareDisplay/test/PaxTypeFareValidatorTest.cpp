#include "test/include/CppUnitHelperMacros.h"
#include <iostream>

#include "DataModel/FareMarket.h"
#include "DataModel/FareDisplayRequest.h"
#include "Common/TseCodeTypes.h"
#include "Common/DateTime.h"
#include "DataModel/AirSeg.h"
#include "test/include/MockGlobal.h"
#include "DataModel/Fare.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxType.h"
#include "DBAccess/PaxTypeInfo.h"
#include "test/testdata/TestPaxTypeFareFactory.h"
#include "test/testdata/TestFactoryManager.h"
#include "FareDisplay/PaxTypeValidator.h"
#include "FareDisplay/QualifierPaxTypeValidator.h"
#include "DataModel/PaxTypeFare.h"
#include "test/include/TestMemHandle.h"

using namespace std;
namespace tse
{
class MockPaxTypeValidator : public PaxTypeValidator
{
public:
  MockPaxTypeValidator() {}
  ~MockPaxTypeValidator() {}
  bool initialize(const FareDisplayTrx& trx)
  {
    _trx = &trx;
    _inclCdPaxSet.insert("ADT");
    _inclCdPaxSet.insert("CNN");
    _exceptPsgData = false;
    return true;
  }

protected:
  bool isDiscounted(const PaxTypeFare& fare) { return false; }
};

class MockPaxTypeValidatorEmptySet : public PaxTypeValidator
{
public:
  MockPaxTypeValidatorEmptySet() {}
  ~MockPaxTypeValidatorEmptySet() {}
  bool initialize(const FareDisplayTrx& trx)
  {
    _trx = &trx;
    return true;
  }
};

class MockQualifierPaxTypeValidator : public QualifierPaxTypeValidator
{
public:
  MockQualifierPaxTypeValidator() { _isDiscounted = false; }
  ~MockQualifierPaxTypeValidator() { delete _trx; }
  bool initialize(FareDisplayTrx& trx)
  {
    FareDisplayTrx* newTrx = new FareDisplayTrx;
    newTrx->setOptions(&_options);
    newTrx->setRequest(trx.getRequest());
    _trx = newTrx;
    _publishedChildInfant.insert("ABC");
    _inclCdPaxSet.insert("ADT");
    _inclCdPaxSet.insert("ABC");
    return true;
  }
  FareDisplayOptions _options;
  bool _isDiscounted;
  PaxTypeCode _basePaxType;

protected:
  bool isDiscounted(const PaxTypeFare& fare) const { return _isDiscounted; }
  const PaxTypeCode& getBaseFarePaxType(const PaxTypeFare& fare) const { return _basePaxType; }
};

class PaxTypeFareValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PaxTypeFareValidatorTest);
  CPPUNIT_TEST(testValidate);
  CPPUNIT_TEST(testisPaxTypeCodeMatch);
  CPPUNIT_TEST_SUITE_END();

  PaxTypeFare* _pfare1;
  PaxTypeFare* _pfare2;
  PaxType* _pType1;
  PaxType* _pType2;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _pfare1 = _memHandle.create<PaxTypeFare>();
    _pfare2 = _memHandle.create<PaxTypeFare>();
    _pType1 = _memHandle.create<PaxType>();
    _pType2 = _memHandle.create<PaxType>();
  }
  void tearDown()
  {
    _memHandle.clear();
    TestFactoryManager::instance()->destroyAll();
  }

  void testValidate()
  {
    // set up first pax type as ADT
    PaxTypeInfo info;
    info.adultInd() = 'Y';
    info.childInd() = 'N';
    info.infantInd() = 'N';
    info.paxType() = "ADT";
    info.initPsgType();
    _pType1->paxType() = info.paxType();
    _pType1->paxTypeInfo() = &info;
    _pfare1->actualPaxType() = _pType1;
    MockPaxTypeValidator validator;
    FareDisplayTrx trx;
    FareDisplayRequest req;
    FareDisplayOptions opt;
    trx.setRequest(&req);
    trx.setOptions(&opt);
    validator.initialize(trx);
    CPPUNIT_ASSERT(validator.validate(*_pfare1));

    info.paxType() = "ABC";
    _pType1->paxType() = info.paxType();

    CPPUNIT_ASSERT(!validator.validate(*_pfare1));

    MockPaxTypeValidatorEmptySet validator1;
    validator1.initialize(trx);
    CPPUNIT_ASSERT(info.isAdult());
    CPPUNIT_ASSERT(validator1.validate(*_pfare1)); // NOW ANY ADULT FARE IS VALID

    info.adultInd() = 'N';
    info.childInd() = 'Y';
    info.infantInd() = 'N';
    info.initPsgType();

    CPPUNIT_ASSERT(!validator1.validate(*_pfare1)); // NOW ANY NON-ADULT FARE IS VALID

    // now validating qualifier pax type

    MockQualifierPaxTypeValidator qValidator;

    qValidator.initialize(trx);
    qValidator._options.childFares() = 'Y';
    CPPUNIT_ASSERT(trx.getOptions() != 0);
    CPPUNIT_ASSERT(qValidator.validate(*_pfare1)); // validating published child fares;

    info.adultInd() = 'N';
    info.childInd() = 'N';
    info.infantInd() = 'Y';
    info.initPsgType();
    CPPUNIT_ASSERT(!qValidator.validate(*_pfare1)); // validating published child fares;

    qValidator._options.childFares() = 'N';
    qValidator._options.infantFares() = 'Y';
    CPPUNIT_ASSERT(qValidator.validate(*_pfare1)); // validating published child fares;
    info.adultInd() = 'Y';
    info.childInd() = 'N';
    info.infantInd() = 'N';
    info.initPsgType();
    CPPUNIT_ASSERT(!qValidator.validate(*_pfare1));

    qValidator._options.adultFares() = 'Y';
    qValidator._options.infantFares() = 'Y';
    CPPUNIT_ASSERT(qValidator.validate(*_pfare1)); // validating published child fares;

    qValidator._isDiscounted = true;
    qValidator._basePaxType = "ADT";
    info.adultInd() = 'N';
    info.childInd() = 'Y';
    info.infantInd() = 'N';
    info.initPsgType();
    qValidator._options.adultFares() = 'N';
    qValidator._options.infantFares() = 'N';
    qValidator._options.childFares() = 'Y';
    CPPUNIT_ASSERT(qValidator.validate(*_pfare1)); // validating published child fares;
    qValidator._basePaxType = "AAA";
    CPPUNIT_ASSERT(!qValidator.validate(*_pfare1)); // validating published child fares;

    info.adultInd() = 'N';
    info.childInd() = 'N';
    info.infantInd() = 'N';
    info.paxType() = "ADT";
    info.initPsgType();
    CPPUNIT_ASSERT(!info.isAdult());
    info.paxType() = "JCB";
    info.initPsgType();
    CPPUNIT_ASSERT(info.isAdult());

    trx.getRequest()->requestType() = "RD";
    trx.getOptions()->lineNumber() = 1;
    trx.getRequest()->inputPassengerTypes().push_back("INF");
    info.paxType() = "INF";
    _pType1->paxType() = info.paxType();
    CPPUNIT_ASSERT(validator.validate(*_pfare1));
  }

  void testisPaxTypeCodeMatch()
  {
    FareDisplayTrx trx;
    FareDisplayOptions options;
    trx.setOptions(&options);
    PaxTypeInfo info;
    info.adultInd() = 'N';
    info.childInd() = 'Y';
    info.infantInd() = 'N';
    info.initPsgType();
    QualifierPaxTypeValidator validator;
    validator.initialize(trx);
    CPPUNIT_ASSERT(!validator.isMatchQualifier(info));
    options.infantFares() = 'Y';
    CPPUNIT_ASSERT(!validator.isMatchQualifier(info));
    options.childFares() = 'Y';
    CPPUNIT_ASSERT(validator.isMatchQualifier(info));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(PaxTypeFareValidatorTest);
}
