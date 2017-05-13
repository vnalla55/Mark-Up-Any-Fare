#include <string>

#include "Common/CabinType.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/ShoppingTrx.h"
#include "DBAccess/Cabin.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "Xform/PreferredCabin.h"

namespace tse
{

class MyDataHandle : public DataHandleMock
{

public:
  MyDataHandle(TestMemHandle& memHandle) : _memHandle(memHandle) {}

  const Loc* getLoc(const LocCode& locCode, const DateTime& date)
  {
    const Loc* loc = DataHandleMock::getLoc(locCode, date);

    CPPUNIT_ASSERT_MESSAGE(locCode.c_str(), loc != 0);

    return loc;
  }

  const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                          const CarrierCode& carrierCode,
                                          GeoTravelType tvlType,
                                          const DateTime& tvlDate)
  {
    return locCode;
  }

private:
  TestMemHandle& _memHandle;
};

namespace
{
class PreferredCabinMock : public PreferredCabin
{
public:
  PreferredCabinMock() : PreferredCabin() {}

protected:
  const Cabin* getCabin(TravelSeg* tvl)
  {
    const AirSeg* airSeg = static_cast<const AirSeg*>(tvl);
    Cabin* cabin = _memHandle.create<Cabin>();

    if (airSeg->getBookingCode() == "P")
    {
      cabin->cabin().setPremiumFirstClass();
      return cabin;
    }

    if (airSeg->getBookingCode() == "F")
    {
      cabin->cabin().setFirstClass();
      return cabin;
    }

    if (airSeg->getBookingCode() == "J")
    {
      cabin->cabin().setPremiumBusinessClass();
      return cabin;
    }

    if (airSeg->getBookingCode() == "C")
    {
      cabin->cabin().setBusinessClass();
      return cabin;
    }

    if (airSeg->getBookingCode() == "S")
    {
      cabin->cabin().setPremiumEconomyClass();
      return cabin;
    }

    if (airSeg->getBookingCode() == "Y")
    {
      cabin->cabin().setEconomyClass();
      return cabin;
    }

    CPPUNIT_ASSERT_MESSAGE("GetCabin failed!", cabin->cabin().isValidCabin());

    return cabin;
  }
  TestMemHandle _memHandle;
};
}


class PreferredCabinTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PreferredCabinTest);
  CPPUNIT_TEST(preferredCabinProcessTest);
  CPPUNIT_TEST(domesticUSCATest);
  CPPUNIT_TEST(domesticUSCATest2);
  CPPUNIT_TEST(subArea11Test);
  CPPUNIT_TEST(subArea11Test2);
  CPPUNIT_SKIP_TEST(subArea21Test); // commented out because pricing logic works differently
  CPPUNIT_SKIP_TEST(subArea21Test2);
  CPPUNIT_TEST(singleSubAreaTest);
  CPPUNIT_TEST(singleAreaTest);
  CPPUNIT_TEST(twoAreasTest);
  CPPUNIT_TEST(allAreasWithStartInFirstAreaTest);
  CPPUNIT_TEST(allAreasWithEndInFirstAreaTest);
  CPPUNIT_TEST(allAreasWithAreaOneViaPoint);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  ShoppingTrx* _trx;

public:
  void setUp()
  {
    _memHandle.create<MyDataHandle>(_memHandle);
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<ShoppingTrx>();
    _trx->travelSeg().push_back(_memHandle.create<AirSeg>());
    _trx->setOptions(_memHandle.create<PricingOptions>());

    _trx->setRequest(_memHandle.create<PricingRequest>());
  }

  void tearDown() { _memHandle.clear(); }

  void preferredCabinProcessTest()
  {
    PreferredCabinMock preferredCabin;
    preferredCabin.setup(_trx);

    preferredCabin.setPreferredCabinInd('4');

    preferredCabin.selectCabin();

    CPPUNIT_ASSERT_MESSAGE(preferredCabin.getPreferredCabin().printName(),
                           preferredCabin.getPreferredCabin().isPremiumBusinessClass());
  }

  void domesticUSCATest()
  {
    PreferredCabinMock preferredCabin;
    preferredCabin.setup(_trx);

    preferredCabin.addBkkNode("S", "LH", "BOS", "NYC");
    preferredCabin.addBkkNode("Y", "LH", "NYC", "WAS");
    preferredCabin.addBkkNode("S", "LH", "WAS", "MIA");

    preferredCabin.selectCabin();
    CPPUNIT_ASSERT_MESSAGE(preferredCabin.getPreferredCabin().printName(),
                           preferredCabin.getPreferredCabin().isPremiumEconomyClass());
  }

  void domesticUSCATest2()
  {
    PreferredCabinMock preferredCabin;
    preferredCabin.setup(_trx);

    preferredCabin.addBkkNode("J", "UA", "CHI", "YVR");
    preferredCabin.addBkkNode("J", "UA", "YVR", "SEA");
    preferredCabin.addBkkNode("S", "UA", "SEA", "SFO");
    preferredCabin.addBkkNode("C", "UA", "SFO", "LAX");

    preferredCabin.selectCabin();

    CPPUNIT_ASSERT_MESSAGE(preferredCabin.getPreferredCabin().printName(),
                           preferredCabin.getPreferredCabin().isPremiumBusinessClass());
  }

  void subArea11Test()
  {
    PreferredCabinMock preferredCabin;
    preferredCabin.setup(_trx);

    preferredCabin.addBkkNode("Y", "UA", "MEX", "SEA");
    preferredCabin.addBkkNode("S", "UA", "SEA", "LAX");
    preferredCabin.addBkkNode("P", "UA", "LAX", "WAS");

    preferredCabin.selectCabin();

    CPPUNIT_ASSERT_MESSAGE(preferredCabin.getPreferredCabin().printName(),
                           preferredCabin.getPreferredCabin().isEconomyClass());
  }

  void subArea11Test2()
  {
    PreferredCabinMock preferredCabin;
    preferredCabin.setup(_trx);

    preferredCabin.addBkkNode("P", "UA", "WAS", "CHI");
    preferredCabin.addBkkNode("S", "UA", "CHI", "SCL");
    preferredCabin.addBkkNode("Y", "UA", "SCL", "MEX");

    preferredCabin.selectCabin();

    CPPUNIT_ASSERT_MESSAGE(preferredCabin.getPreferredCabin().printName(),
                           preferredCabin.getPreferredCabin().isPremiumEconomyClass());
  }

  void subArea21Test()
  {
    PreferredCabinMock preferredCabin;
    preferredCabin.setup(_trx);

    preferredCabin.addBkkNode("Y", "UA", "PAR", "DUS");
    preferredCabin.addBkkNode("S", "UA", "DUS", "VIE");
    preferredCabin.addBkkNode("P", "UA", "VIE", "WAW");

    preferredCabin.selectCabin();

    CPPUNIT_ASSERT_MESSAGE(preferredCabin.getPreferredCabin().printName(),
                           preferredCabin.getPreferredCabin().isPremiumFirstClass());
  }

  void subArea21Test2()
  {
    PreferredCabinMock preferredCabin;
    preferredCabin.setup(_trx);

    preferredCabin.addBkkNode("Y", "UA", "LON", "PAR");
    preferredCabin.addBkkNode("S", "UA", "PAR", "MUC");
    preferredCabin.addBkkNode("P", "UA", "MUC", "WAW");

    preferredCabin.selectCabin();

    CPPUNIT_ASSERT_MESSAGE(preferredCabin.getPreferredCabin().printName(),
                           preferredCabin.getPreferredCabin().isPremiumEconomyClass());
  }

  void singleSubAreaTest()
  {
    PreferredCabinMock preferredCabin;
    preferredCabin.setup(_trx);

    preferredCabin.addBkkNode("C", "UA", "SAO", "RIO");
    preferredCabin.addBkkNode("S", "UA", "RIO", "CCS");
    preferredCabin.addBkkNode("J", "UA", "CCS", "BUE");

    preferredCabin.selectCabin();

    CPPUNIT_ASSERT_MESSAGE(preferredCabin.getPreferredCabin().printName(),
                           preferredCabin.getPreferredCabin().isPremiumEconomyClass());
  }

  void singleAreaTest()
  {
    PreferredCabinMock preferredCabin;
    preferredCabin.setup(_trx);

    preferredCabin.addBkkNode("P", "UA", "BUE", "RIO");
    preferredCabin.addBkkNode("C", "UA", "RIO", "LAX");
    preferredCabin.addBkkNode("F", "UA", "LAX", "CHI");

    preferredCabin.selectCabin();

    CPPUNIT_ASSERT_MESSAGE(preferredCabin.getPreferredCabin().printName(),
                           preferredCabin.getPreferredCabin().isBusinessClass());
  }

  void twoAreasTest()
  {
    PreferredCabinMock preferredCabin;
    preferredCabin.setup(_trx);

    preferredCabin.addBkkNode("S", "LH", "MEX", "NYC");
    preferredCabin.addBkkNode("F", "LH", "NYC", "FRA");
    preferredCabin.addBkkNode("C", "LH", "FRA", "WAW");

    preferredCabin.selectCabin();

    CPPUNIT_ASSERT_MESSAGE(preferredCabin.getPreferredCabin().printName(),
                           preferredCabin.getPreferredCabin().isFirstClass());
  }

  void allAreasWithStartInFirstAreaTest()
  {
    PreferredCabinMock preferredCabin;
    preferredCabin.setup(_trx);

    preferredCabin.addBkkNode("S", "UA", "LAX", "WAS");
    preferredCabin.addBkkNode("F", "UA", "WAS", "PAR");
    preferredCabin.addBkkNode("S", "UA", "PAR", "SGN");
    preferredCabin.addBkkNode("C", "UA", "SGN", "BKK");

    preferredCabin.selectCabin();

    CPPUNIT_ASSERT_MESSAGE(preferredCabin.getPreferredCabin().printName(),
                           preferredCabin.getPreferredCabin().isFirstClass());
  }

  void allAreasWithEndInFirstAreaTest()
  {
    PreferredCabinMock preferredCabin;
    preferredCabin.setup(_trx);

    preferredCabin.addBkkNode("C", "UA", "OSA", "BKK");
    preferredCabin.addBkkNode("S", "UA", "BKK", "FRA");
    preferredCabin.addBkkNode("F", "UA", "FRA", "WAS");
    preferredCabin.addBkkNode("S", "UA", "WAS", "CHI");

    preferredCabin.selectCabin();

    CPPUNIT_ASSERT_MESSAGE(preferredCabin.getPreferredCabin().printName(),
                           preferredCabin.getPreferredCabin().isFirstClass());
  }

  void allAreasWithAreaOneViaPoint()
  {
    PreferredCabinMock preferredCabin;
    preferredCabin.setup(_trx);

    preferredCabin.addBkkNode("C", "LH", "WAW", "LON");
    preferredCabin.addBkkNode("F", "LH", "LON", "WAS");
    preferredCabin.addBkkNode("C", "LH", "WAS", "SAN");
    preferredCabin.addBkkNode("S", "LH", "SAN", "SGN");

    preferredCabin.selectCabin();

    CPPUNIT_ASSERT_MESSAGE(preferredCabin.getPreferredCabin().printName(),
                           preferredCabin.getPreferredCabin().isFirstClass());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(PreferredCabinTest);
}
