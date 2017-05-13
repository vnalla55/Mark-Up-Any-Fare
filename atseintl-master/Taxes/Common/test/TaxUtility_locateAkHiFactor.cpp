#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxAkHiFactor.h"
#include "Taxes/Common/TaxUtility.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  ~MyDataHandle() { _memHandle.clear(); }
  const std::vector<TaxAkHiFactor*>& getTaxAkHiFactor(const LocCode& key, const DateTime& date)
  {
    std::vector<TaxAkHiFactor*>& ret = *_memHandle.create<std::vector<TaxAkHiFactor*> >();
    if (key == "LAX")
    {
      ret.push_back(_memHandle.create<TaxAkHiFactor>());
      ret.front()->zoneAPercent() = 0.0349;
      ret.front()->zoneBPercent() = 0.0386;
      ret.front()->zoneCPercent() = 0.0442;
      ret.front()->zoneDPercent() = 0.5020;
      ret.front()->hawaiiPercent() = 0.0003;
    }
    return ret;
  }
};
}

class TaxUtility_locateAkHiFactor: public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxUtility_locateAkHiFactor);
  CPPUNIT_TEST(locateAkFactorZoneA);
  CPPUNIT_TEST(locateAkFactorZoneB);
  CPPUNIT_TEST(locateAkFactorZoneC);
  CPPUNIT_TEST(locateAkFactorZoneD);
  CPPUNIT_TEST(locateHiFactor);
  CPPUNIT_TEST(locateAkFactorZoneUnknown);
  CPPUNIT_TEST(locateAkFactorEmptyAnswer);
  CPPUNIT_TEST(locateHiFactorEmptyAnswer);
  CPPUNIT_TEST_SUITE_END();

  LocCode LAX;
  LocCode DFW;
  Loc _zoneLoc;
  PricingRequest _request;
  PricingTrx _trx;

public:
  void setUp()
  {
    LAX = "LAX";
    DFW = "DFW";

    _trx.setRequest(&_request);
  }

  void locateAkFactorZoneA()
  {
    MyDataHandle myDataHandle;
    _zoneLoc.alaskazone() = 'A';
    CPPUNIT_ASSERT_EQUAL(0.0349, taxUtil::locateAkFactor(_trx, &_zoneLoc, LAX));
  }

  void locateAkFactorZoneB()
  {
    MyDataHandle myDataHandle;
    _zoneLoc.alaskazone() = 'B';
    CPPUNIT_ASSERT_EQUAL(0.0386, taxUtil::locateAkFactor(_trx, &_zoneLoc, LAX));
  }

  void locateAkFactorZoneC()
  {
    MyDataHandle myDataHandle;
    _zoneLoc.alaskazone() = 'C';
    CPPUNIT_ASSERT_EQUAL(0.0442, taxUtil::locateAkFactor(_trx, &_zoneLoc, LAX));
  }

  void locateAkFactorZoneD()
  {
    MyDataHandle myDataHandle;
    _zoneLoc.alaskazone() = 'D';
    CPPUNIT_ASSERT_EQUAL(0.5020, taxUtil::locateAkFactor(_trx, &_zoneLoc, LAX));
  }

  void locateHiFactor()
  {
    MyDataHandle myDataHandle;
    CPPUNIT_ASSERT_EQUAL(0.0003, taxUtil::locateHiFactor(_trx, LAX));
  }

  void locateAkFactorZoneUnknown()
  {
    MyDataHandle myDataHandle;
    _zoneLoc.alaskazone() = ' ';
    CPPUNIT_ASSERT_EQUAL(0.0, taxUtil::locateAkFactor(_trx, &_zoneLoc, LAX));
  }

  void locateAkFactorEmptyAnswer()
  {
    MyDataHandle myDataHandle;
    _zoneLoc.alaskazone() = 'A';
    CPPUNIT_ASSERT_EQUAL(0.0, taxUtil::locateAkFactor(_trx, &_zoneLoc, DFW));
  }

  void locateHiFactorEmptyAnswer()
  {
    MyDataHandle myDataHandle;
    CPPUNIT_ASSERT_EQUAL(0.0, taxUtil::locateHiFactor(_trx, DFW));
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxUtility_locateAkHiFactor);
};
