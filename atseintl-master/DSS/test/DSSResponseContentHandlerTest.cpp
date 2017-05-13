#include "test/include/CppUnitHelperMacros.h"
#include "DSS/DSSResponseContentHandler.h"

#include "DSS/FlightCount.h"
#include "DataModel/FareDisplayTrx.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

namespace tse
{
class DSSResponseContentHandlerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DSSResponseContentHandlerTest);
  CPPUNIT_TEST(testParse);
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

public:
  void setUp()
  {
    _memHandle.create<SpecificTestConfigInitializer>();
    _trx = _memHandle.create<FareDisplayTrx>();
    _doc = _memHandle.insert(new DSSResponseContentHandler(*_trx, _fcs));
    _doc->initialize();
  }
  void tearDown()
  {
    _memHandle.clear();
    _fcs.clear();
  }
  void testParse()
  {
    CPPUNIT_ASSERT(
        _doc->parse("<DSS VER=\"2.0\" COR=\"    \" TXN=\"12345\"><FSD COD=\"F9\" ICD=\"false\" "
                    "NST=\"0\" DIR=\"1\" ONL=\"2\" ISE=\"Y\"/><FSD COD=\"CO\" ICD=\"true\" "
                    "NST=\"4\" DIR=\"5\" ONL=\"6\"/><PTM D83=\"0.030000\" D84=\"0.032423\" "
                    "D85=\"0.000000\" D86=\"0.000346\" D87=\"0.000000\" D88=\"0.000206\"/></DSS>"));
    CPPUNIT_ASSERT_EQUAL(size_t(2), _fcs.size());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("F9"), _fcs[0]->_carrier);
    CPPUNIT_ASSERT_EQUAL(false, _fcs[0]->_isDirectCarrier);
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), _fcs[0]->_nonStop);
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), _fcs[0]->_direct);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), _fcs[0]->_onlineConnection);
    CPPUNIT_ASSERT_EQUAL(true, _fcs[0]->_interLineServiceExist);

    CPPUNIT_ASSERT_EQUAL(CarrierCode("CO"), _fcs[1]->_carrier);
    CPPUNIT_ASSERT_EQUAL(true, _fcs[1]->_isDirectCarrier);
    CPPUNIT_ASSERT_EQUAL(uint16_t(4), _fcs[1]->_nonStop);
    CPPUNIT_ASSERT_EQUAL(uint16_t(5), _fcs[1]->_direct);
    CPPUNIT_ASSERT_EQUAL(uint16_t(6), _fcs[1]->_onlineConnection);
    CPPUNIT_ASSERT_EQUAL(false, _fcs[1]->_interLineServiceExist);
  }

private:
  TestMemHandle _memHandle;
  FareDisplayTrx* _trx;
  std::vector<FlightCount*> _fcs;
  DSSResponseContentHandler* _doc;
};

CPPUNIT_TEST_SUITE_REGISTRATION(DSSResponseContentHandlerTest);
}
