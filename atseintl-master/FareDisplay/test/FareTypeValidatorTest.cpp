#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/FareDisplayRequest.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/DateTime.h"
#include "test/include/MockGlobal.h"
#include "DataModel/Fare.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxType.h"
#include "FareDisplay/FareTypeValidator.h"
#include "DBAccess/FareClassAppInfo.h"
#include "DBAccess/PaxTypeInfo.h"
#include "test/testdata/TestFactoryManager.h"
#include "DataModel/PaxTypeFare.h"
#include "test/include/TestMemHandle.h"

using namespace std;

// const FareType& fcaFareType() const { return _fareClassAppInfo-> _fareType; }
namespace tse
{
class MockFareTypeValidator : public FareTypeValidator
{
public:
  MockFareTypeValidator() {}
  ~MockFareTypeValidator() {}
  bool initialize(const FareDisplayTrx& trx)
  {
    _fareTypes.insert("A");
    _fareTypes.insert("B");
    _fareTypes.insert("C");
    _exceptFareData = false;
    return true;
  }

protected:
  bool isDiscounted(const PaxTypeFare& fare) { return false; }
};

class MockFareTypeValidatorDisc : public FareTypeValidator
{
public:
  MockFareTypeValidatorDisc() {}
  ~MockFareTypeValidatorDisc() {}
  bool initialize(const FareDisplayTrx& trx) { return true; }
  bool _isDiscounted;
  FareType _baseFareType;

protected:
  bool isDiscounted(const PaxTypeFare& fare) const { return _isDiscounted; }
  std::set<FareType>& fareTypes() { return _fareTypes; }
  const FareType& getBaseFareFareType(const PaxTypeFare& fare) const { return _baseFareType; }
};

class FareTypeValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareTypeValidatorTest);
  CPPUNIT_TEST(testValidate);
  CPPUNIT_TEST(testValidateDiscFare);
  CPPUNIT_TEST_SUITE_END();

  PaxType* _pType1;
  PaxTypeFare* _pfare1;
  PaxTypeFare* _pfare2;
  FareClassAppInfo* _appInfo1;
  FareClassAppInfo* _appInfo2;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    //    _pfare1 = new PaxTypeFare();
    _pfare1 = _memHandle.create<PaxTypeFare>();
    //    _pfare2 = new PaxTypeFare();
    _pfare2 = _memHandle.create<PaxTypeFare>();

    //    _appInfo1 = new FareClassAppInfo();
    _appInfo1 = _memHandle.create<FareClassAppInfo>();
    //    _appInfo2 = new FareClassAppInfo();
    _appInfo2 = _memHandle.create<FareClassAppInfo>();
    //    _pType1 = new PaxType();
    _pType1 = _memHandle.create<PaxType>();
  }
  void tearDown()
  {

    TestFactoryManager::instance()->destroyAll();
    //    delete _pfare1;
    //    delete _pfare2;
    //    delete _appInfo1;
    //    delete _appInfo2;
    //    delete _pType1;

    _memHandle.clear();
  }

  void testValidate()
  {
    /**
     //set up first pax type as ADT
     PaxTypeInfo info;
     info.adultInd()= 'Y';
     info.childInd()= 'N';
     info.infantInd()= 'N';
     info.paxType()= "ADT";
     info.initPsgType();
     _pType1->paxType()= info.paxType();
     _pType1->paxTypeInfo() = &info;
     _pfare1->actualPaxType()= _pType1;
     MockPaxTypeValidator validator;
     FareDisplayTrx trx;
     validator.initialize(trx);
     CPPUNIT_ASSERT(validator.validate(*_pfare1));

     info.paxType()= "ABC";
     _pType1->paxType()= info.paxType();

     CPPUNIT_ASSERT(!validator.validate(*_pfare1));

     MockPaxTypeValidatorEmptySet validator1;
     CPPUNIT_ASSERT(info.isAdult());
     CPPUNIT_ASSERT(validator1.validate(*_pfare1)); //NOW ANY ADULT FARE IS VALID

     info.adultInd()= 'N';
     info.childInd()= 'Y';
     info.infantInd()= 'N';
     info.initPsgType();

     CPPUNIT_ASSERT(!validator1.validate(*_pfare1)); //NOW ANY NON-ADULT FARE IS VALID

     //now validating qualifier pax type

     MockQualifierPaxTypeValidator qValidator;

     qValidator.initialize(trx);
     qValidator._options.childFares()='Y';
     CPPUNIT_ASSERT(trx.getOptions() != 0);
     CPPUNIT_ASSERT(qValidator.validate(*_pfare1)); //validating published child fares;

     info.adultInd()= 'N';
     info.childInd()= 'N';
     info.infantInd()= 'Y';
     info.initPsgType();
     CPPUNIT_ASSERT(!qValidator.validate(*_pfare1)); //validating published child fares;

     qValidator._options.childFares()='N';
     qValidator._options.infantFares()='Y';
     CPPUNIT_ASSERT(qValidator.validate(*_pfare1)); //validating published child fares;
     info.adultInd()= 'Y';
     info.childInd()= 'N';
     info.infantInd()= 'N';
     info.initPsgType();
     CPPUNIT_ASSERT(!qValidator.validate(*_pfare1));

     qValidator._options.adultFares()='Y';
     qValidator._options.infantFares()='Y';
     CPPUNIT_ASSERT(qValidator.validate(*_pfare1)); //validating published child fares;

     qValidator._isDiscounted = true;
     qValidator._basePaxType = "ADT";
     info.adultInd()= 'N';
     info.childInd()= 'Y';
     info.infantInd()= 'N';
     info.initPsgType();
     qValidator._options.adultFares()='N';
     qValidator._options.infantFares()='N';
     qValidator._options.childFares()='Y';
     CPPUNIT_ASSERT(qValidator.validate(*_pfare1)); //validating published child fares;
     qValidator._basePaxType = "AAA";
     CPPUNIT_ASSERT(!qValidator.validate(*_pfare1)); //validating published child fares;
     **/
  }

  void testValidateDiscFare()
  {
    FareDisplayTrx trx;
    _appInfo1->_fareType = "D";
    _pfare1->fareClassAppInfo() = _appInfo1;
    CPPUNIT_ASSERT(_pfare1->fcaFareType() == "D");
    PaxTypeInfo info;
    info.adultInd() = 'Y';
    info.childInd() = 'N';
    info.infantInd() = 'N';
    info.paxType() = "ADT";
    info.initPsgType();
    _pType1->paxType() = info.paxType();
    _pType1->paxTypeInfo() = &info;
    _pfare1->actualPaxType() = _pType1;

    MockFareTypeValidator validator;
    validator.initialize(trx);
    CPPUNIT_ASSERT(!validator.validate(*_pfare1));
    _appInfo1->_fareType = "A";
    CPPUNIT_ASSERT(validator.validate(*_pfare1));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(FareTypeValidatorTest);
}
