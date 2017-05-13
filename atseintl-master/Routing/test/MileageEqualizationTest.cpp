#include <vector>

#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Mileage.h"
#include "Routing/MileageEqualization.h"
#include "Routing/MileageRouteItem.h"
#include "Routing/MileageRoute.h"
#include "Routing/MileageUtil.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"

namespace tse
{
namespace
{
struct MileageData
{
  LocCode city1, city2;
  uint16_t tpm, mpm, tpd;

  MileageData(const LocCode& city1, const LocCode& city2, uint16_t tpm, uint16_t mpm, uint16_t tpd)
    : city1(city1), city2(city2), tpm(tpm), mpm(mpm), tpd(tpd)
  {
  }
};

const MileageData _mileage[] = {
  //           orig   dest   tpm   mpm   tpd
  MileageData("PIT", "NYC",  331,    0,    0),
  MileageData("NYC", "BNA",  758,    0,    0),
  MileageData("BNA", "MIA",  808,    0,    0),
  MileageData("MIA", "SAO", 4070,    0,    0),
  MileageData("PIT", "SAO",    0, 5911,    0),
  MileageData("NYC", "SAO", 4758, 5511,    0),
  MileageData("MIA", "RIO", 4179,    0,    0),
  MileageData("PIT", "RIO",    0, 5495,    0),
  MileageData("NYC", "RIO", 4816, 5500,    0),
  // TPD specific data
  MileageData("ORL", "CHI",  991,    0,  800),
  MileageData("CHI", "NYC",  725,    0,  800),
  MileageData("ORL", "SAO",    0, 5125,    0),
  MileageData("ORL", "RIO",    0, 5256,    0),
  MileageData("DFW", "SAO", 5000, 4256,  400),
  MileageData("DFW", "RIO", 5300, 4682,  600),
};

const MileageData*
getData(const LocCode& origin, const LocCode& dest)
{
  for (const MileageData& data : _mileage)
  {
    if (!(origin == data.city1 && dest == data.city2) &
        !(origin == data.city2 && dest == data.city1))
      continue;

    return &data;
  }

  return 0;
}

class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  const Mileage* getMileage(const LocCode& origin,
                            const LocCode& dest,
                            Indicator mileageType,
                            const GlobalDirection globalDir,
                            const DateTime& date)
  {
    const uint16_t mileage = getMileage(origin, dest, mileageType);

    if (mileage == 0)
      return DataHandleMock::getMileage(origin, dest, mileageType, globalDir, date);

    Mileage* ret = _memHandle.create<Mileage>();
    ret->mileage() = mileage;
    return ret;
  }

  uint16_t getMileage(const LocCode& origin, const LocCode& dest, Indicator mileageType)
  {
    const MileageData* const data = getData(origin, dest);

    if (!data)
      return 0;

    switch (mileageType)
    {
    case 'T':
      return data->tpm;
    case 'M':
      return data->mpm;
    default:
      return 0;
    }
  }
};

class MockTpdProcessor : public MileageExclusion
{
public:
  virtual ~MockTpdProcessor() {}
  virtual bool apply(MileageRoute& route) const
  {
    uint16_t grandTPD = 0;

    for (MileageRouteItem& item : route.mileageRouteItems())
    {
      const MileageData* const data = getData(item.city1()->loc(), item.city2()->loc());
      if (!data || data->tpd <= grandTPD)
        continue;
      grandTPD = data->tpd;
    }

    route.tpd() = route.mileageRouteItems().back().tpd() = grandTPD;

    return grandTPD > 0;
  }
};
}
class MileageEqualizationTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MileageEqualizationTest);
  CPPUNIT_TEST(testApplyNegativeEmpty);
  CPPUNIT_TEST(testApplyNegativeNotWH);
  CPPUNIT_TEST(testApplyNegativeNoSAO_RIO);
  CPPUNIT_TEST(testApplyNegativeWorseRatio);
  CPPUNIT_TEST(testApplyReverseNegativeWorseRatio);
  CPPUNIT_TEST(testApplyNegativeEmsUnchanged);
  CPPUNIT_TEST(testApplyReverseNegativeEmsUnchanged);
  CPPUNIT_TEST(testApplyNegativeReplacedInside1);
  CPPUNIT_TEST(testApplyNegativeReplacedInside2);
  CPPUNIT_TEST(testApplyNegativeReplacedInside3);
  CPPUNIT_TEST(testApplyNegativeReplacedInside4);
  CPPUNIT_TEST(testApplyPositive);
  CPPUNIT_TEST(testApplyReversePositive);
  CPPUNIT_TEST(testApplyPositiveTpdSame);
  CPPUNIT_TEST(testApplyReversePositiveTpdSame);
  CPPUNIT_TEST(testApplyNegativeTpdSame);
  CPPUNIT_TEST(testApplyReverseNegativeTpdSame);
  CPPUNIT_TEST(testApplyPositiveTpdDiffers);
  CPPUNIT_TEST(testApplyReversePositiveTpdDiffers);
  CPPUNIT_TEST(testApplyNegativeTpdDiffers);
  CPPUNIT_TEST(testApplyReverseNegativeTpdDiffers);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _mileageMock = _memHandle.create<MyDataHandle>();
    _tpd = _memHandle.create<MockTpdProcessor>();
    _target = _memHandle(new MileageEqualization(*_tpd));
    _dataHandle = _memHandle.create<DataHandle>();
    _pit = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocPIT.xml");
    _nyc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNYC.xml");
    _bna = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocBNA.xml");
    _mia = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMIA.xml");
    _sao = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSAO.xml");
    _rio = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocRIO.xml");
    _orl = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocORL.xml");
    _chi = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocCHI.xml");
    _dfw = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
  }

  void tearDown() { _memHandle.clear(); }

  void startRoute(MileageRoute& route)
  {
    route.dataHandle() = _dataHandle;
    route.globalDirection() = GlobalDirection::WH;
    route.mileageRouteTPM() = 0;
  }

  void addItem(MileageRoute& route, const Loc* from)
  {
    const size_t index = route.mileageRouteItems().size();
    route.mileageRouteItems().resize(index + 1);
    MileageRouteItem& item = route.mileageRouteItems()[index];

    item.city1() = const_cast<Loc*>(from);
    item.travelDate() = DateTime::localTime();
    item.globalDirection(TPM) = GlobalDirection::WH;
    item.globalDirection(MPM) = GlobalDirection::WH;
    item.tpm() = 0;
    item.mpm() = 0;

    if (index > 0)
    {
      MileageRouteItem& prev = route.mileageRouteItems()[index - 1];

      prev.city2() = const_cast<Loc*>(from);
      prev.tpm() = _mileageMock->getMileage(prev.city1()->loc(), from->loc(), 'T');
      route.mileageRouteTPM() += prev.tpm();
    }
  }

  void endRoute(MileageRoute& route, const Loc* to)
  {
    MileageRouteItem& front = route.mileageRouteItems().front();
    MileageRouteItem& back = route.mileageRouteItems().back();

    back.city2() = const_cast<Loc*>(to);
    back.tpm() = _mileageMock->getMileage(back.city1()->loc(), to->loc(), 'T');
    route.mileageRouteTPM() += back.tpm();

    back.mpm() = _mileageMock->getMileage(front.city1()->loc(), to->loc(), 'M');
    route.mileageRouteMPM() = back.mpm();

    _tpd->apply(route);
    route.mileageRouteTPM() -= route.tpd();

    route.ems() = MileageUtil::getEMS(route.mileageRouteTPM(), route.mileageRouteMPM());
  }

  void testApplyNegativeEmpty()
  {
    MileageRoute route;
    startRoute(route);

    // Don't apply: No route items.
    CPPUNIT_ASSERT(!_target->apply(route));
  }

  void testApplyNegativeNotWH()
  {
    MileageRoute route;
    startRoute(route);
    addItem(route, _pit);
    addItem(route, _nyc);
    addItem(route, _bna);
    addItem(route, _mia);
    endRoute(route, _sao);

    route.globalDirection() = GlobalDirection::CA;

    // Don't apply: Wrong global direction.
    CPPUNIT_ASSERT(!_target->apply(route));
    CPPUNIT_ASSERT_EQUAL(uint16_t(5911), route.mileageRouteItems().back().mpm());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5911), route.mileageRouteMPM());
    CPPUNIT_ASSERT_EQUAL(uint16_t(4070), route.mileageRouteItems().back().tpm());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5967), route.mileageRouteTPM());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5), route.ems());
  }

  void testApplyNegativeNoSAO_RIO()
  {
    MileageRoute route;
    startRoute(route);
    addItem(route, _pit);
    addItem(route, _nyc);
    addItem(route, _bna);
    endRoute(route, _mia);

    // Don't apply: No SAO/RIO on route.
    CPPUNIT_ASSERT(!_target->apply(route));
  }

  void testApplyNegativeWorseRatio()
  {
    MileageRoute route;
    startRoute(route);
    addItem(route, _pit);
    addItem(route, _nyc);
    addItem(route, _bna);
    addItem(route, _mia);
    endRoute(route, _sao);

    // Don't apply: RIO travel has worse TPM/MPM ratio.
    CPPUNIT_ASSERT(!_target->apply(route));
    CPPUNIT_ASSERT_EQUAL(uint16_t(5911), route.mileageRouteItems().back().mpm());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5911), route.mileageRouteMPM());
    CPPUNIT_ASSERT_EQUAL(uint16_t(4070), route.mileageRouteItems().back().tpm());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5967), route.mileageRouteTPM());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5), route.ems());
  }

  void testApplyReverseNegativeWorseRatio()
  {
    MileageRoute route;
    startRoute(route);
    addItem(route, _sao);
    addItem(route, _mia);
    addItem(route, _bna);
    addItem(route, _nyc);
    endRoute(route, _pit);

    // Don't apply: RIO travel has worse TPM/MPM ratio.
    CPPUNIT_ASSERT(!_target->apply(route));
    CPPUNIT_ASSERT_EQUAL(uint16_t(5911), route.mileageRouteItems().back().mpm());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5911), route.mileageRouteMPM());
    CPPUNIT_ASSERT_EQUAL(uint16_t(4070), route.mileageRouteItems().front().tpm());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5967), route.mileageRouteTPM());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5), route.ems());
  }

  void testApplyNegativeEmsUnchanged()
  {
    MileageRoute route;
    startRoute(route);
    addItem(route, _nyc);
    addItem(route, _bna);
    addItem(route, _mia);
    endRoute(route, _rio);

    // Don't apply: TPM/MPM is lower, but EMS is the same.
    CPPUNIT_ASSERT(!_target->apply(route));
    CPPUNIT_ASSERT_EQUAL(uint16_t(5500), route.mileageRouteItems().back().mpm());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5500), route.mileageRouteMPM());
    CPPUNIT_ASSERT_EQUAL(uint16_t(4179), route.mileageRouteItems().back().tpm());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5745), route.mileageRouteTPM());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5), route.ems());
  }

  void testApplyReverseNegativeEmsUnchanged()
  {
    MileageRoute route;
    startRoute(route);
    addItem(route, _rio);
    addItem(route, _mia);
    addItem(route, _bna);
    endRoute(route, _nyc);

    // Don't apply: TPM/MPM is lower, but EMS is the same.
    CPPUNIT_ASSERT(!_target->apply(route));
    CPPUNIT_ASSERT_EQUAL(uint16_t(5500), route.mileageRouteItems().back().mpm());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5500), route.mileageRouteMPM());
    CPPUNIT_ASSERT_EQUAL(uint16_t(4179), route.mileageRouteItems().front().tpm());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5745), route.mileageRouteTPM());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5), route.ems());
  }

  void testApplyNegativeReplacedInside1()
  {
    MileageRoute route;
    startRoute(route);
    addItem(route, _rio);
    addItem(route, _nyc);
    addItem(route, _bna);
    addItem(route, _mia);
    endRoute(route, _sao);

    // Don't apply: replaced city is already on the route.
    CPPUNIT_ASSERT(!_target->apply(route));
  }

  void testApplyNegativeReplacedInside2()
  {
    MileageRoute route;
    startRoute(route);
    addItem(route, _nyc);
    addItem(route, _rio);
    addItem(route, _bna);
    addItem(route, _mia);
    endRoute(route, _sao);

    // Don't apply: replaced city is already on the route.
    CPPUNIT_ASSERT(!_target->apply(route));
  }

  void testApplyNegativeReplacedInside3()
  {
    MileageRoute route;
    startRoute(route);
    addItem(route, _sao);
    addItem(route, _rio);
    addItem(route, _nyc);
    addItem(route, _bna);
    endRoute(route, _mia);

    // Don't apply: replaced city is already on the route.
    CPPUNIT_ASSERT(!_target->apply(route));
  }

  void testApplyNegativeReplacedInside4()
  {
    MileageRoute route;
    startRoute(route);
    addItem(route, _sao);
    endRoute(route, _rio);

    // Don't apply: replaced city is already on the route.
    CPPUNIT_ASSERT(!_target->apply(route));
  }

  void testApplyPositive()
  {
    MileageRoute route;
    startRoute(route);
    addItem(route, _pit);
    addItem(route, _nyc);
    addItem(route, _bna);
    addItem(route, _mia);
    endRoute(route, _rio);

    CPPUNIT_ASSERT(_target->apply(route));

    // Applied: The TPM and MPM should be the same as above.
    CPPUNIT_ASSERT_EQUAL(uint16_t(5911), route.mileageRouteItems().back().mpm());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5911), route.mileageRouteMPM());
    CPPUNIT_ASSERT_EQUAL(uint16_t(4070), route.mileageRouteItems().back().tpm());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5967), route.mileageRouteTPM());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5), route.ems());
  }

  void testApplyReversePositive()
  {
    MileageRoute route;
    startRoute(route);
    addItem(route, _rio);
    addItem(route, _mia);
    addItem(route, _bna);
    addItem(route, _nyc);
    endRoute(route, _pit);

    CPPUNIT_ASSERT(_target->apply(route));

    // Applied: The TPM and MPM should be the same as above.
    CPPUNIT_ASSERT_EQUAL(uint16_t(5911), route.mileageRouteItems().back().mpm());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5911), route.mileageRouteMPM());
    CPPUNIT_ASSERT_EQUAL(uint16_t(4070), route.mileageRouteItems().front().tpm());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5967), route.mileageRouteTPM());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5), route.ems());
  }

  void testApplyPositiveTpdSame()
  {
    MileageRoute route;
    startRoute(route);
    addItem(route, _orl);
    addItem(route, _chi);
    addItem(route, _nyc);
    endRoute(route, _sao);

    CPPUNIT_ASSERT(_target->apply(route));

    // Applied
    CPPUNIT_ASSERT_EQUAL(uint16_t(5256), route.mileageRouteItems().back().mpm());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5256), route.mileageRouteMPM());
    CPPUNIT_ASSERT_EQUAL(uint16_t(4816), route.mileageRouteItems().back().tpm());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5732), route.mileageRouteTPM());
    CPPUNIT_ASSERT_EQUAL(uint16_t(10), route.ems());
  }

  void testApplyReversePositiveTpdSame()
  {
    MileageRoute route;
    startRoute(route);
    addItem(route, _sao);
    addItem(route, _nyc);
    addItem(route, _chi);
    endRoute(route, _orl);

    CPPUNIT_ASSERT(_target->apply(route));

    // Applied
    CPPUNIT_ASSERT_EQUAL(uint16_t(5256), route.mileageRouteItems().back().mpm());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5256), route.mileageRouteMPM());
    CPPUNIT_ASSERT_EQUAL(uint16_t(4816), route.mileageRouteItems().front().tpm());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5732), route.mileageRouteTPM());
    CPPUNIT_ASSERT_EQUAL(uint16_t(10), route.ems());
  }

  void testApplyNegativeTpdSame()
  {
    MileageRoute route;
    startRoute(route);
    addItem(route, _orl);
    addItem(route, _chi);
    addItem(route, _nyc);
    endRoute(route, _rio);

    CPPUNIT_ASSERT(!_target->apply(route));

    // Not applied: ems no lower.
    CPPUNIT_ASSERT_EQUAL(uint16_t(5256), route.mileageRouteItems().back().mpm());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5256), route.mileageRouteMPM());
    CPPUNIT_ASSERT_EQUAL(uint16_t(4816), route.mileageRouteItems().back().tpm());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5732), route.mileageRouteTPM());
    CPPUNIT_ASSERT_EQUAL(uint16_t(10), route.ems());
  }

  void testApplyReverseNegativeTpdSame()
  {
    MileageRoute route;
    startRoute(route);
    addItem(route, _rio);
    addItem(route, _nyc);
    addItem(route, _chi);
    endRoute(route, _orl);

    CPPUNIT_ASSERT(!_target->apply(route));

    // Not applied: ems no lower.
    CPPUNIT_ASSERT_EQUAL(uint16_t(5256), route.mileageRouteItems().back().mpm());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5256), route.mileageRouteMPM());
    CPPUNIT_ASSERT_EQUAL(uint16_t(4816), route.mileageRouteItems().front().tpm());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5732), route.mileageRouteTPM());
    CPPUNIT_ASSERT_EQUAL(uint16_t(10), route.ems());
  }

  void testApplyPositiveTpdDiffers()
  {
    MileageRoute route;
    startRoute(route);
    addItem(route, _dfw);
    endRoute(route, _sao);

    CPPUNIT_ASSERT(_target->apply(route));

    // Applied
    CPPUNIT_ASSERT_EQUAL(uint16_t(4682), route.mileageRouteItems().back().mpm());
    CPPUNIT_ASSERT_EQUAL(uint16_t(4682), route.mileageRouteMPM());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5300), route.mileageRouteItems().back().tpm());
    CPPUNIT_ASSERT_EQUAL(uint16_t(4700), route.mileageRouteTPM());
    CPPUNIT_ASSERT_EQUAL(uint16_t(600), route.tpd());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5), route.ems());
  }

  void testApplyReversePositiveTpdDiffers()
  {
    MileageRoute route;
    startRoute(route);
    addItem(route, _sao);
    endRoute(route, _dfw);

    CPPUNIT_ASSERT(_target->apply(route));

    // Applied
    CPPUNIT_ASSERT_EQUAL(uint16_t(4682), route.mileageRouteItems().back().mpm());
    CPPUNIT_ASSERT_EQUAL(uint16_t(4682), route.mileageRouteMPM());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5300), route.mileageRouteItems().front().tpm());
    CPPUNIT_ASSERT_EQUAL(uint16_t(4700), route.mileageRouteTPM());
    CPPUNIT_ASSERT_EQUAL(uint16_t(600), route.tpd());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5), route.ems());
  }

  void testApplyNegativeTpdDiffers()
  {
    MileageRoute route;
    startRoute(route);
    addItem(route, _dfw);
    endRoute(route, _rio);

    CPPUNIT_ASSERT(!_target->apply(route));

    // Not applied: ems no lower after TPD reduction.
    CPPUNIT_ASSERT_EQUAL(uint16_t(4682), route.mileageRouteItems().back().mpm());
    CPPUNIT_ASSERT_EQUAL(uint16_t(4682), route.mileageRouteMPM());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5300), route.mileageRouteItems().back().tpm());
    CPPUNIT_ASSERT_EQUAL(uint16_t(4700), route.mileageRouteTPM());
    CPPUNIT_ASSERT_EQUAL(uint16_t(600), route.tpd());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5), route.ems());
  }

  void testApplyReverseNegativeTpdDiffers()
  {
    MileageRoute route;
    startRoute(route);
    addItem(route, _rio);
    endRoute(route, _dfw);

    CPPUNIT_ASSERT(!_target->apply(route));

    // Not applied: ems no lower after TPD reduction.
    CPPUNIT_ASSERT_EQUAL(uint16_t(4682), route.mileageRouteItems().back().mpm());
    CPPUNIT_ASSERT_EQUAL(uint16_t(4682), route.mileageRouteMPM());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5300), route.mileageRouteItems().front().tpm());
    CPPUNIT_ASSERT_EQUAL(uint16_t(4700), route.mileageRouteTPM());
    CPPUNIT_ASSERT_EQUAL(uint16_t(600), route.tpd());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5), route.ems());
  }

private:
  TestMemHandle _memHandle;
  MyDataHandle* _mileageMock;
  MileageExclusion* _tpd;
  MileageEqualization* _target;
  DataHandle* _dataHandle;
  const Loc* _pit;
  const Loc* _nyc;
  const Loc* _bna;
  const Loc* _mia;
  const Loc* _sao;
  const Loc* _rio;
  const Loc* _orl;
  const Loc* _chi;
  const Loc* _dfw;
};
CPPUNIT_TEST_SUITE_REGISTRATION(MileageEqualizationTest);
}
