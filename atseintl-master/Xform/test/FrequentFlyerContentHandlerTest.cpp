//----------------------------------------------------------------------------
//  Copyright Sabre 2015
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
#include "Common/TseCodeTypes.h"
#include "DataModel/FrequentFlyerTrx.h"
#include "Xform/FrequentFlyerContentHandler.h"

#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include <xercesc/util/PlatformUtils.hpp>

namespace tse
{
class FrequentFlyerContentHandlerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FrequentFlyerContentHandlerTest);
  CPPUNIT_TEST(parseXMLandSetTrxTest);

  CPPUNIT_TEST_SUITE_END();

private:
  TestMemHandle _memH;
  FrequentFlyerTrx* _trx;
  FrequentFlyerContentHandler* _ffch;

public:
  void setUp()
  {
    _memH.create<TestConfigInitializer>();
    _trx = _memH.create<FrequentFlyerTrx>();
    _ffch = _memH.create<FrequentFlyerContentHandler>();
    xercesc::XMLPlatformUtils::Initialize();
  }

  void tearDown()
  {
    _memH.clear();
    xercesc::XMLPlatformUtils::Terminate();
  }

  // tests
  void parseXMLandSetTrxTest()
  {
    const char* xml =
        "<FrequentFlyerRequest Version=\"1.0.0\"><RequestOptions CarrierCode=\"YY\" />"
        "<RequestOptions CarrierCode=\"XX\" />"
        "<RequestOptions CarrierCode=\"XX\" /></FrequentFlyerRequest>";
    CPPUNIT_ASSERT(_ffch->parse(xml));
    _ffch->setTrx(_trx);
    CPPUNIT_ASSERT_EQUAL(uint32_t(2), _trx->getCxrs().size());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("XX"), _trx->getCxrs().begin()->first);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("YY"), (++_trx->getCxrs().begin())->first);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FrequentFlyerContentHandlerTest);
}
