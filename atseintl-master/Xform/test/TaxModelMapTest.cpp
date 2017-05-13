#include "Common/Config/ConfigMan.h"
#include "Common/ErrorResponseException.h"
#include "Common/XMLChString.h"
#include "DataModel/TaxTrx.h"
#include "DBAccess/DataHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "Xform/test/MockXercescAttributes.h"
#include "Xform/TaxModelMap.h"
#include "test/include/CppUnitHelperMacros.h"
#include <xercesc/sax2/DefaultHandler.hpp>

namespace tse
{
class TaxModelMapTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxModelMapTest);
  CPPUNIT_TEST(testPassengerDataParsing_ADT);
  CPPUNIT_TEST(testPassengerDataParsing_withAge);
  CPPUNIT_TEST_SUITE_END();

private:
  DataHandle _dataHandle;
  TaxModelMap* _modelMap;
  Trx* _baseTrx;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _baseTrx = _memHandle.create<TaxTrx>();

    tse::ConfigMan enhancedRDConfig;
    _modelMap = _memHandle.insert<TaxModelMap>(
        new TaxModelMap(enhancedRDConfig, _dataHandle, _baseTrx));

    xercesc::XMLPlatformUtils::Initialize();
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void testPassengerDataParsing_ADT()
  {
    TaxModelMap::Mapping map = {0, 0};
    map.members[_modelMap->SDBMHash("B70")] = 1;
    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;
    attrs.add("B70", "ADT");

    _modelMap->storePassengerInformation(attrs);

    CPPUNIT_ASSERT_EQUAL(tse::PaxTypeCode(""), _modelMap->_paxType->requestedPaxType());
    CPPUNIT_ASSERT_EQUAL(tse::PaxTypeCode("ADT"), _modelMap->_paxType->paxType());
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), _modelMap->_paxType->age());
  }

  void testPassengerDataParsing_withAge()
  {
    TaxModelMap::Mapping map = {0, 0};
    map.members[_modelMap->SDBMHash("B70")] = 1;
    _modelMap->_currentMapEntry = (void*)&map;

    MockXercescAttributes attrs;
    attrs.add("B70", "A15");

    _modelMap->storePassengerInformation(attrs);

    CPPUNIT_ASSERT_EQUAL(tse::PaxTypeCode(""), _modelMap->_paxType->requestedPaxType());
    CPPUNIT_ASSERT_EQUAL(tse::PaxTypeCode("A15"), _modelMap->_paxType->paxType());
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), _modelMap->_paxType->age());
  }

};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxModelMapTest);

} // end of tse namespace
