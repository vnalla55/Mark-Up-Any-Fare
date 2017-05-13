#include "Common/TseUtil.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/GlobalDirSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Mileage.h"
#include "DBAccess/MileageSubstitution.h"
#include "DBAccess/MultiTransport.h"
#include "DBAccess/SurfaceSectorExempt.h"
#include "Routing/AdditionalMileageRetriever.h"
#include "Routing/GlobalDirectionRetriever.h"
#include "Routing/MileageDataRetriever.h"
#include "Routing/MileageRetriever.h"
#include "Routing/MileageSubstitutionRetriever.h"
#include "Routing/MultiTransportRetriever.h"
#include "Routing/SurfaceSectorExemptRetriever.h"
#include "Routing/TPMConstructor.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{

namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;
  GlobalDirSeg* getGD(LocCode l1, LocCode l2, std::string gd)
  {
    GlobalDirSeg* ret = _memHandle.create<GlobalDirSeg>();
    ret->loc1Type() = MARKET;
    ret->loc1() = l1;
    ret->loc2Type() = MARKET;
    ret->loc2() = l2;
    ret->globalDir() = gd;
    ret->directionality() = 'W';
    return ret;
  }

public:
  const Mileage* getMileage(const LocCode& origin,
                            const LocCode& dest,
                            Indicator mileageType,
                            const GlobalDirection globalDir,
                            const DateTime& date)
  {
    if (globalDir == GlobalDirection::XX)
      return 0;
    Mileage* ret = _memHandle.create<Mileage>();
    ret->mileage() = 1000;
    return ret;
  }
  const std::vector<MultiTransport*>& getMultiTransportCity(const LocCode& locCode,
                                                            const CarrierCode& carrierCode,
                                                            GeoTravelType tvlType,
                                                            const DateTime& tvlDate)
  {
    std::vector<MultiTransport*>* ret = _memHandle.create<std::vector<MultiTransport*> >();
    if (locCode != "NUL")
    {
      MultiTransport* m = _memHandle.create<MultiTransport>();
      m->multitranscity() = "LON";
      ret->push_back(m);
    }
    return *ret;
  }
  const MileageSubstitution* getMileageSubstitution(const LocCode& key, const DateTime& date)
  {
    if (key == "NUL")
      return 0;
    MileageSubstitution* mileageSubstitution = _memHandle.create<MileageSubstitution>();
    mileageSubstitution->publishedLoc() = "CIT";
    return mileageSubstitution;
  }
  const std::vector<GlobalDirSeg*>& getGlobalDirSeg(const DateTime& date)
  {
    std::vector<GlobalDirSeg*>* ret = _memHandle.create<std::vector<GlobalDirSeg*> >();
    ret->push_back(getGD("CT1", "CT2", "AT"));
    ret->push_back(getGD("NUL", "CT1", "PA"));
    ret->push_back(getGD("NUL", "CT2", "WH"));
    return *ret;
  }
  const SurfaceSectorExempt*
  getSurfaceSectorExempt(const LocCode& origLoc, const LocCode& destLoc, const DateTime& date)
  {
    if (origLoc == "NUL")
      return 0;
    return _memHandle.create<SurfaceSectorExempt>();
  }
  const TariffMileageAddon* getTariffMileageAddon(const CarrierCode& carrier,
                                                  const LocCode& unpublishedAddonLoc,
                                                  const GlobalDirection& globalDir,
                                                  const DateTime& date)
  {
    return 0;
  }
  const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                          const CarrierCode& carrierCode,
                                          GeoTravelType tvlType,
                                          const DateTime& tvlDate)
  {
    return locCode;
  }
};
}

class MileageDataRetrieverTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MileageDataRetrieverTest);
  CPPUNIT_TEST(testMileageRetriever);
  CPPUNIT_TEST(testMultiTransportRetriever);
  CPPUNIT_TEST(testMultiTransportRetriever2);
  CPPUNIT_TEST(testMultiTransportRetriever3);
  CPPUNIT_TEST(testMultiTransportRetriever4);
  CPPUNIT_TEST(testMileageSubstitutionRetriever);
  CPPUNIT_SKIP_TEST(testAdditionalMileageRetriever);
  CPPUNIT_TEST(testGlobalDirectionRetriever);
  CPPUNIT_TEST(testTPMConstructor);
  CPPUNIT_SKIP_TEST(testPSRRetriever);
  CPPUNIT_TEST(testSurfaceSectorExemptRetriever);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<MyDataHandle>();
    _memHandle.create<TestConfigInitializer>();
    _item = _memHandle.create<MileageRouteItem>();
    _item->city1() = _memHandle.create<Loc>();
    _item->city2() = _memHandle.create<Loc>();
    _item->city1()->loc() = "NUL";
    _item->city2()->loc() = "NUL";
    _item->city1()->area() = '1';
    _item->city2()->area() = '2';
    _item->city1()->cityInd() = true;
    _item->city2()->cityInd() = true;
    _item->tpmGlobalDirection() = GlobalDirection::XX;
    _item->mpmGlobalDirection() = GlobalDirection::XX;
    _item->tpm() = 10;
    _item->mpm() = 10;
  }
  void tearDown() { _memHandle.clear(); }

  void testMileageRetriever()
  {
    const MileageRetriever& retriever(tse::Singleton<MileageRetriever>::instance());
    DataHandle dataHandle;
    CPPUNIT_ASSERT(!retriever.retrieve(*_item, _dataHandle, TPM));
    CPPUNIT_ASSERT_EQUAL((uint16_t)10, _item->tpm());
    CPPUNIT_ASSERT(!retriever.retrieve(*_item, _dataHandle, MPM));
    CPPUNIT_ASSERT_EQUAL((uint16_t)10, _item->mpm());
    _item->tpmGlobalDirection() = _item->mpmGlobalDirection() = GlobalDirection::AF;
    CPPUNIT_ASSERT(retriever.retrieve(*_item, _dataHandle, TPM));
    CPPUNIT_ASSERT_EQUAL((uint16_t)1000, _item->tpm());
    CPPUNIT_ASSERT(retriever.retrieve(*_item, dataHandle, MPM));
    CPPUNIT_ASSERT_EQUAL((uint16_t)1000, _item->mpm());
  }
  void checkMultTransportLocs(LocCode loc1, LocCode loc2, LocCode mloc1, LocCode mloc2)
  {
    CPPUNIT_ASSERT_EQUAL(loc1, _item->city1()->loc());
    CPPUNIT_ASSERT_EQUAL(loc2, _item->city2()->loc());
    CPPUNIT_ASSERT_EQUAL(mloc1, _item->multiTransportOrigin()->loc());
    CPPUNIT_ASSERT_EQUAL(mloc2, _item->multiTransportDestination()->loc());
  }

  void testMultiTransportRetriever()
  {
    const MultiTransportRetriever& retriever(tse::Singleton<MultiTransportRetriever>::instance());
    _item->multiTransportOrigin() = _memHandle.create<Loc>();
    _item->multiTransportDestination() = _memHandle.create<Loc>();
    _item->multiTransportOrigin()->loc() = "NUL";
    _item->multiTransportDestination()->loc() = "NUL";
    CPPUNIT_ASSERT(!retriever.retrieve(*_item, _dataHandle));
    checkMultTransportLocs("NUL", "NUL", "NUL", "NUL");
  }

  void testMultiTransportRetriever2()
  {
    const MultiTransportRetriever& retriever(tse::Singleton<MultiTransportRetriever>::instance());
    _item->multiTransportOrigin() = _memHandle.create<Loc>();
    _item->multiTransportDestination() = _memHandle.create<Loc>();
    _item->city2()->loc() = "CI2";
    _item->multiTransportOrigin()->loc() = "NUL";
    _item->multiTransportDestination()->loc() = "NUL";
    CPPUNIT_ASSERT(retriever.retrieve(*_item, _dataHandle));
    checkMultTransportLocs("NUL", "CI2", "NUL", "LON");
  }

  void testMultiTransportRetriever3()
  {
    const MultiTransportRetriever& retriever(tse::Singleton<MultiTransportRetriever>::instance());
    _item->multiTransportOrigin() = _memHandle.create<Loc>();
    _item->multiTransportDestination() = _memHandle.create<Loc>();
    _item->city1()->loc() = "CI1";
    _item->city2()->loc() = "CI2";
    _item->multiTransportOrigin()->loc() = "NUL";
    _item->multiTransportDestination()->loc() = "NUL";
    CPPUNIT_ASSERT(retriever.retrieve(*_item, _dataHandle));
    checkMultTransportLocs("CI1", "CI2", "LON", "LON");
  }

  void testMultiTransportRetriever4()
  {
    const MultiTransportRetriever& retriever(tse::Singleton<MultiTransportRetriever>::instance());
    _item->multiTransportOrigin() = _memHandle.create<Loc>();
    _item->multiTransportDestination() = _memHandle.create<Loc>();
    _item->city1()->loc() = "CI1";
    _item->multiTransportOrigin()->loc() = "NUL";
    _item->multiTransportDestination()->loc() = "NUL";
    CPPUNIT_ASSERT(retriever.retrieve(*_item, _dataHandle));
    checkMultTransportLocs("CI1", "NUL", "LON", "NUL");
  }
  void checkSubstCity(LocCode loc1, LocCode loc2)
  {
    CPPUNIT_ASSERT_EQUAL(loc1, _item->city1()->loc());
    CPPUNIT_ASSERT_EQUAL(loc2, _item->city2()->loc());
  }

  void testMileageSubstitutionRetriever()
  {
    const MileageSubstitutionRetriever& retriever(
        tse::Singleton<MileageSubstitutionRetriever>::instance());
    CPPUNIT_ASSERT(!retriever.retrieve(*_item, _dataHandle));
    checkSubstCity("NUL", "NUL");
    _item->city2()->loc() = "CI2";
    CPPUNIT_ASSERT(retriever.retrieve(*_item, _dataHandle));
    checkSubstCity("NUL", "CIT");
    _item->city1()->loc() = "CI1";
    CPPUNIT_ASSERT(retriever.retrieve(*_item, _dataHandle));
    checkSubstCity("CIT", "CIT");
    _item->city2()->loc() = "NUL";
    CPPUNIT_ASSERT(retriever.retrieve(*_item, _dataHandle));
    checkSubstCity("CIT", "NUL");
  }

  void testAdditionalMileageRetriever() {}

  void testGlobalDirectionRetriever()
  {
    const GlobalDirectionRetriever& retriever(tse::Singleton<GlobalDirectionRetriever>::instance());
    CPPUNIT_ASSERT(!retriever.retrieve(*_item, _dataHandle));
    CPPUNIT_ASSERT_EQUAL(GlobalDirection::XX, _item->tpmGlobalDirection());
    _item->city2()->loc() = "CT1";
    CPPUNIT_ASSERT(retriever.retrieve(*_item, _dataHandle));
    CPPUNIT_ASSERT_EQUAL(GlobalDirection::PA, _item->tpmGlobalDirection());
    MileageRouteItem i2;
    _item->clone(_dataHandle, i2);
    i2.city1()->loc() = "CT1";
    i2.city2()->loc() = "NUL";
    MileageRouteItems items;
    items.push_back(*_item);
    items.push_back(i2);
    CPPUNIT_ASSERT(!retriever.retrieve(items, _dataHandle));
    CPPUNIT_ASSERT_EQUAL(GlobalDirection::XX, items.back().mpmGlobalDirection());
    items.front().city1()->loc() = "CT1";
    items.back().city2()->loc() = "CT2";
    CPPUNIT_ASSERT(retriever.retrieve(items, _dataHandle));
    CPPUNIT_ASSERT_EQUAL(GlobalDirection::AT, items.back().mpmGlobalDirection());
  }

  void testTPMConstructor()
  {
    const TPMConstructor& constructor(tse::Singleton<TPMConstructor>::instance());
    _item->tpmGlobalDirection() = _item->mpmGlobalDirection() = GlobalDirection::AF;
    CPPUNIT_ASSERT(constructor.retrieve(*_item, _dataHandle));
    CPPUNIT_ASSERT(_item->isConstructed());
    // CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(10/1.2), item.tpm());
    CPPUNIT_ASSERT_EQUAL((uint16_t)10, _item->mpm());
    _item->tpmGlobalDirection() = _item->mpmGlobalDirection() = GlobalDirection::XX;
    _item->isConstructed() = false;
    _item->city1()->loc() = "DFW";
    _item->city2()->loc() = "CHI";
    CPPUNIT_ASSERT(constructor.retrieve(*_item, _dataHandle));
    CPPUNIT_ASSERT(_item->isConstructed());
    // this two loc doesnt have lattitude and longitute hence it should return 0 TODO need to
    // discuss
    // do we really wanted to test this and expect to pass.
    // CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(TseUtil::greatCircleMiles(*loc1, *loc2)),
    // item.tpm());
    CPPUNIT_ASSERT_EQUAL((uint16_t)10, _item->mpm());
  }

  void testPSRRetriever() {}

  void testSurfaceSectorExemptRetriever()
  {
    const SurfaceSectorExemptRetriever& retriever(
        tse::Singleton<SurfaceSectorExemptRetriever>::instance());
    CPPUNIT_ASSERT(!retriever.retrieve(*_item, _dataHandle));
    CPPUNIT_ASSERT(!_item->tpmSurfaceSectorExempt());
    _item->city1()->loc() = _item->city2()->loc() = "CIT";
    CPPUNIT_ASSERT(retriever.retrieve(*_item, _dataHandle));
    CPPUNIT_ASSERT(_item->tpmSurfaceSectorExempt());
  }

private:
  TestMemHandle _memHandle;
  MileageRouteItem* _item;
  DataHandle _dataHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(MileageDataRetrieverTest);
}
