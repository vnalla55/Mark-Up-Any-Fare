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
#include "DBAccess/FreqFlyerStatus.h"
#include "Xform/FrequentFlyerResponseFormatter.h"

#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>

namespace tse
{
class FrequentFlyerResponseFormatterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FrequentFlyerResponseFormatterTest);
  CPPUNIT_TEST(sendCarrierNotFound);
  CPPUNIT_TEST(sendSortedCarriersOrder);
  CPPUNIT_TEST(sendCarrierFound);

  CPPUNIT_TEST_SUITE_END();

private:
  TestMemHandle _memH;
  FrequentFlyerTrx* _trx;
  FrequentFlyerResponseFormatter* _ffrf;

public:
  void setUp()
  {
    _memH.create<TestConfigInitializer>();
    _trx = _memH.create<FrequentFlyerTrx>();
    _ffrf = _memH.insert(new FrequentFlyerResponseFormatter(*_trx));
  }

  void tearDown() { _memH.clear(); }

  // tests
  void sendCarrierNotFound()
  {
    std::set<CarrierCode> carriers;
    carriers.insert("XX");
    _trx->setCxrs(carriers);
    const std::string actual = _ffrf->formatResponse();
    const std::string expected = "<FrequentFlyerResponse><FFData CarrierCode=\"XX\" "
                                 "CarrierNotFound=\"T\"/></FrequentFlyerResponse>";
    CPPUNIT_ASSERT_EQUAL(actual, expected);
  }

  void sendSortedCarriersOrder()
  {
    std::set<CarrierCode> carriers;
    carriers.insert("XX");
    carriers.insert("X1");
    carriers.insert("1X");
    carriers.insert("VX");
    carriers.insert("V1");
    carriers.insert("1V");
    _trx->setCxrs(carriers);
    const std::string actual = _ffrf->formatResponse();
    const std::string expected = "<FrequentFlyerResponse><FFData CarrierCode=\"1V\" "
                                 "CarrierNotFound=\"T\"/><FFData CarrierCode=\"1X\" "
                                 "CarrierNotFound=\"T\"/><FFData CarrierCode=\"V1\" "
                                 "CarrierNotFound=\"T\"/><FFData CarrierCode=\"X1\" "
                                 "CarrierNotFound=\"T\"/><FFData CarrierCode=\"VX\" "
                                 "CarrierNotFound=\"T\"/><FFData CarrierCode=\"XX\" "
                                 "CarrierNotFound=\"T\"/></FrequentFlyerResponse>";
    CPPUNIT_ASSERT_EQUAL(actual, expected);
  }

  void sendCarrierFound()
  {
    std::set<CarrierCode> carriers;
    carriers.insert("XX");
    carriers.insert("LH");
    _trx->setCxrs(carriers);

    FreqFlyerStatus status1, status2;
    status1._carrier = "LH";
    status1._level = 1;
    status1._statusLevel = "GLOBAL SERVICES";
    status1._maxPassengersSamePNR = 8;
    status1._maxPassengersDiffPNR = 1;

    status2._carrier = "LH";
    status2._level = 2;
    status2._statusLevel = "COOKIES";
    status2._maxPassengersSamePNR = 6;
    status2._maxPassengersDiffPNR = 0;

    FrequentFlyerTrx::StatusList data;
    data.emplace_back(&status1);
    data.back()._maxPassengersTotal = 9;
    data.emplace_back(&status2);
    data.back()._maxPassengersTotal = 6;

    _trx->setCxrData("LH", std::move(data));
    const std::string actual = _ffrf->formatResponse();
    const std::string expected =
        "<FrequentFlyerResponse><FFData CarrierCode=\"LH\">"
        "<TierData Level=\"1\" LevelName=\"GLOBAL SERVICES\" "
        "MaxPaxSamePNR=\"8\" MaxPaxDifferentPNR=\"1\" MaxPaxTotal=\"9\"/>"
        "<TierData Level=\"2\" LevelName=\"COOKIES\" "
        "MaxPaxSamePNR=\"6\" MaxPaxDifferentPNR=\"0\" MaxPaxTotal=\"6\"/></FFData>"
        "<FFData CarrierCode=\"XX\" CarrierNotFound=\"T\"/>"
        "</FrequentFlyerResponse>";
    CPPUNIT_ASSERT_EQUAL(actual, expected);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FrequentFlyerResponseFormatterTest);
}
