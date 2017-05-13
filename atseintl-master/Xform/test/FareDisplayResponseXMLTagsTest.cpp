#include "Common/CabinType.h"
#include "Common/Logger.h"
#include "Common/Vendor.h"
#include "Common/XMLConstruct.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include "Xform/FareDisplayResponseXMLTags.h"

#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>

namespace tse
{
class FareDisplayResponseXMLTagsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareDisplayResponseXMLTagsTest);
  CPPUNIT_TEST(testVendorCode);
  CPPUNIT_TEST(testPremiumCabin);
  CPPUNIT_TEST(testConstructedFareIndicator);

  CPPUNIT_TEST_SUITE_END();

private:
  static log4cxx::LoggerPtr _logger;

protected:
  FareDisplayResponseXMLTags* _fdrXMLTags;
  FareDisplayTrx* _fdTrx;
  XMLConstruct* _xml;
  TestMemHandle _memHandle;
  PaxTypeFare* _paxTypeFare;
  FareInfo* _fareInfo;
  Fare* _fare;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _fdrXMLTags = _memHandle.insert(new FareDisplayResponseXMLTags);
    _fdTrx = _memHandle.insert(new FareDisplayTrx);
    _fareInfo = _memHandle.create<FareInfo>();
    _fare = _memHandle.create<Fare>();
    _fare->setFareInfo(_fareInfo);
    _paxTypeFare = _memHandle.create<PaxTypeFare>();
    _paxTypeFare->setFare(_fare);

  }

  void tearDown()
  {
    _memHandle.clear();
  }

  // TESTS
   void testVendorCode()
  {
    _fareInfo->vendor() = "ATP";
    CPPUNIT_ASSERT_EQUAL(std::string("A"), _fdrXMLTags->vendorCode(*_paxTypeFare, *_fdTrx));
  }

  void testPremiumCabin()
  {
    _paxTypeFare->cabin().setPremiumFirstClass();
    CPPUNIT_ASSERT_EQUAL(std::string("P"), _fdrXMLTags->allCabin(*_paxTypeFare, *_fdTrx));
  }

   void testConstructedFareIndicator()
  {
    _fare->status().set(Fare::FS_ConstructedFare, true);
    CPPUNIT_ASSERT_EQUAL(std::string("Y"), _fdrXMLTags->constructedFareIndicator(*_paxTypeFare));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FareDisplayResponseXMLTagsTest);
}
