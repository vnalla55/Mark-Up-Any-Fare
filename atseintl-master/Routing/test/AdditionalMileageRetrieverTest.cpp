#include "test/include/CppUnitHelperMacros.h"
#include "Routing/MileageDataRetriever.h"
#include "Routing/MileageRetriever.h"

#include "Routing/AdditionalMileageRetriever.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Mileage.h"
#include "Routing/MileageRouteItem.h"
#include "DBAccess/TariffMileageAddon.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include "DBAccess/Mileage.h"

namespace tse
{
namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;
  TariffMileageAddon* getMilAdd(CarrierCode carrier,
                                LocCode unpublishedAddonLoc,
                                GlobalDirection globalDir,
                                int mileage,
                                LocCode publishedLoc)
  {
    TariffMileageAddon* ret = _memHandle.create<TariffMileageAddon>();
    ret->unpublishedAddonLoc() = unpublishedAddonLoc;
    ret->carrier() = carrier;
    ret->milesAdded() = 397;
    ret->publishedLoc() = publishedLoc;
    return ret;
  }
  Mileage* getMil(int mileage)
  {
    Mileage* ret = _memHandle.create<Mileage>();
    ret->mileage() = mileage;
    return ret;
  }

public:
  const TariffMileageAddon* getTariffMileageAddon(const CarrierCode& carrier,
                                                  const LocCode& unpublishedAddonLoc,
                                                  const GlobalDirection& globalDir,
                                                  const DateTime& date)
  {
    if (carrier == "" && unpublishedAddonLoc == "IZT" && globalDir == GlobalDirection::AT)
      return getMilAdd(carrier, unpublishedAddonLoc, globalDir, 397, "MEX");
    else if (carrier == "AA" && unpublishedAddonLoc == "IZT" && globalDir == GlobalDirection::AT)
      return 0;
    else if (unpublishedAddonLoc == "LON" && globalDir == GlobalDirection::AT)
      return 0;
    else if (unpublishedAddonLoc == "BKK" && globalDir == GlobalDirection::PA)
      return 0;
    else if (carrier == "NW" && unpublishedAddonLoc == "ADQ" && globalDir == GlobalDirection::PA)
      return getMilAdd(carrier, unpublishedAddonLoc, globalDir, 300, "ANC");
    else if (carrier == "" && unpublishedAddonLoc == "ADQ" && globalDir == GlobalDirection::PA)
      return 0;

    return DataHandleMock::getTariffMileageAddon(carrier, unpublishedAddonLoc, globalDir, date);
  }
  const Mileage* getMileage(const LocCode& origin,
                            const LocCode& dest,
                            Indicator mileageType,
                            const GlobalDirection globalDir,
                            const DateTime& date)
  {
    if (origin == "MEX" && dest == "LON")
      return getMil(6668);
    else if (origin == "BKK" && dest == "ANC")
      return getMil(7468);
    return DataHandleMock::getMileage(origin, dest, mileageType, globalDir, date);
  }
};
}

class MockAdditionalMileageRetriever : public AdditionalMileageRetriever
{
private:
  const TariffMileageAddon* getData(DataHandle& dataHandle,
                                    const LocCode& unpublishedMkt,
                                    CarrierCode& govCxr,
                                    GlobalDirection& globalDir,
                                    const DateTime& travelDate) const

  {
    TariffMileageAddon* addOnMkt = new TariffMileageAddon();
    if (unpublishedMkt == "NUL")
      return 0;

    else if (unpublishedMkt == "MAN ")
    {
      addOnMkt->unpublishedAddonLoc() = "MAN";
      addOnMkt->publishedLoc() = "LON";
      addOnMkt->milesAdded() = 100;
      return addOnMkt;
    }
    else
    {
      addOnMkt->unpublishedAddonLoc() = "MAN";
      addOnMkt->publishedLoc() = "LON";
      addOnMkt->milesAdded() = 100;
      return addOnMkt;
    }
    return 0;
  }

  bool getMileage(DataHandle& dataHandle, MileageRouteItem& mItem) const
  {
    if (mItem.city1()->loc() == "NUL")
      return false;

    mItem.mpm() = 1000;
    return true;
  }
  friend class tse::Singleton<MockAdditionalMileageRetriever>;
};

class AdditionalMileageRetrieverTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AdditionalMileageRetrieverTest);
  CPPUNIT_TEST(testAdditionalMileageRetriever);
  CPPUNIT_TEST(testAdditionalMileageRetrieverWithDatabase);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() { _memHandle.create<MyDataHandle>(); }

  void testAdditionalMileageRetriever()
  {
    const AdditionalMileageRetriever& retriever(
        tse::Singleton<MockAdditionalMileageRetriever>::instance());
    MileageRouteItem item;
    DataHandle dataHandle;
    Loc loc1, loc2;
    item.city1() = &loc1;
    item.city2() = &loc2;
    item.mpm() = 0;
    item.city1()->loc() = item.city2()->loc() = "NUL";
    retriever.retrieve(item, dataHandle);
    CPPUNIT_ASSERT_EQUAL(item.mpm(), static_cast<uint16_t>(0));

    // with one Additional Mkt
    item.city1()->loc() = "MAN";
    item.city2()->loc() = "NUL";
    retriever.retrieve(item, dataHandle);

    CPPUNIT_ASSERT_EQUAL(item.mpm(), static_cast<uint16_t>(1100));

    // with Two Additional Mkt
    item.city2()->loc() = "DHK";
    retriever.retrieve(item, dataHandle);
    CPPUNIT_ASSERT_EQUAL(item.mpm(), static_cast<uint16_t>(1200));
  }

  void testAdditionalMileageRetrieverWithDatabase()
  {
    const AdditionalMileageRetriever& retriever(
        tse::Singleton<AdditionalMileageRetriever>::instance());

    DataHandle dataHandle;
    MileageRouteItem mItem;

    const Loc* loc1 = dataHandle.getLoc("IZT", DateTime::localTime());
    const Loc* loc2 = dataHandle.getLoc("LON", DateTime::localTime());
    mItem.segmentCarrier() = "AA";
    mItem.city1() = const_cast<Loc*>(loc1);
    mItem.city2() = const_cast<Loc*>(loc2);
    mItem.globalDirection(MPM) = GlobalDirection::AT;
    mItem.travelDate() = DateTime::localTime();
    CPPUNIT_ASSERT(retriever.retrieve(mItem, dataHandle));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(7065), mItem.mpm());
    CPPUNIT_ASSERT_EQUAL(LocCode("MEX"), mItem.city1()->loc());

    MileageRouteItem mItem2;

    const Loc* loc3 = dataHandle.getLoc("BKK", DateTime::localTime());
    const Loc* loc4 = dataHandle.getLoc("ADQ", DateTime::localTime());
    mItem2.segmentCarrier() = "NW";
    mItem2.city1() = const_cast<Loc*>(loc3);
    mItem2.city2() = const_cast<Loc*>(loc4);
    mItem2.globalDirection(MPM) = GlobalDirection::PA;
    mItem2.travelDate() = DateTime::localTime();
    CPPUNIT_ASSERT(retriever.retrieve(mItem2, dataHandle));
    CPPUNIT_ASSERT_EQUAL(LocCode("ANC"), mItem2.city2()->loc());
  }

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(AdditionalMileageRetrieverTest);
}
