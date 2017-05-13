// -------------------------------------------------------------------
//
//
//  Copyright (C) Sabre 2011
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
// -------------------------------------------------------------------

#include <boost/format.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/CxrFareMarkets.h"
#include "DataModel/DirFMPath.h"
#include "DataModel/OwrtFareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/ShoppingTrx.h"
#include "DBAccess/Loc.h"
#include "Pricing/Shopping/PQ/ConxRoutePQItem.h"
#include "Pricing/Shopping/PQ/DiagSoloPQCollector.h"
#include "Pricing/Shopping/PQ/PUFullType.h"
#include "Pricing/Shopping/PQ/SoloPQItemManager.h"
#include "Pricing/Shopping/PQ/SolutionPattern.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestPaxTypeFareFactory.h"
#include "test/testdata/TestShoppingTrxFactory.h"

#include <memory>
#include <vector>

namespace tse
{
namespace shpq
{

const uint16_t LocNumber = 4;

class PUFullTypeTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PUFullTypeTest);
  CPPUNIT_TEST(testPUValidation);
  CPPUNIT_TEST_SUITE_END();

private:
  typedef std::shared_ptr<FareMarket> FareMarketPtr;
  typedef std::vector<DirFMPathPtr> LegVector;
  typedef std::vector<FareMarketPtr> FMVector;
  typedef std::shared_ptr<DiagSoloPQCollector> DiagPtr;

  TestMemHandle _dataHandle;
  ShoppingTrx* _trx;
  PaxTypeFare* _paxTypeFare;
  DiagPtr _dc;

  Loc _loc[LocNumber];
  LegVector _obVector;
  LegVector _ibVector;

  SoloPQItemManager _pqItemMgr;
  FMVector _fmVector; // to release FM
private:
  CxrFareMarketsPtr getFM(const size_t origin, const size_t destination)
  {
    FareMarketPtr fm(new FareMarket());
    fm->origin() = &_loc[origin];
    fm->destination() = &_loc[destination];
    fm->boardMultiCity() = _loc[origin].loc();
    fm->offMultiCity() = _loc[destination].loc();
    fm->allPaxTypeFare().push_back(_paxTypeFare);
    _fmVector.push_back(fm);
    OwrtFareMarketPtr owFM = OwrtFareMarket::create(*_trx, HRT, fm.get(), 0);
    CPPUNIT_ASSERT_MESSAGE("OwrtFareMarket create", owFM);

    CxrFareMarketsPtr cxrFM = CxrFareMarkets::create(*_trx, HRT);
    CPPUNIT_ASSERT_MESSAGE("CxrFareMarkets create", cxrFM);

    cxrFM->insert(owFM);

    return cxrFM;
  }

  DirFMPathPtr
  getDirFMPath(CxrFareMarketsPtr firstSeg, CxrFareMarketsPtr secondSeg = CxrFareMarketsPtr())
  {
    return DirFMPath::create(*_trx, firstSeg, secondSeg);
  }

  void validateSP(const SolutionPattern& sp,
                  const ConxRoutePQItemPtr crItem,
                  bool isRoundTrip,
                  bool diffConxPoint)
  {
    ConxRoutePQItem::CxrFMVector fmVector = crItem->getFMVector();
    std::string fmPath("");
    for (ConxRoutePQItem::CxrFMVector::iterator it = fmVector.begin(), end = fmVector.end();
         it != end;
         ++it)
    {
      fmPath += (boost::format("%s-%s | ") % (*it)->getOriginLocation().toString() %
                 (*it)->getDestinationLocation().toString()).str();
    }

    std::string msg = (boost::format("%s: %s (%s - %s) [%s]") % sp.getSPIdStr() %
                       sp.getPUPathStr() % (isRoundTrip ? "RT" : "noRT") %
                       (diffConxPoint ? "diffConxPoint" : "equalConxPoint") % fmPath).str();

    bool expectedValue = (isRoundTrip && !diffConxPoint) || (!isRoundTrip && diffConxPoint);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, expectedValue, sp.isItemValid(crItem.get(), *_dc));
  }

public:
  void setUp()
  {
    _dataHandle.create<TestConfigInitializer>();
    _trx =
        TestShoppingTrxFactory::create("/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    _paxTypeFare = TestPaxTypeFareFactory::create("testdata/PaxTypeFare.xml", true);
    CPPUNIT_ASSERT(_trx);
    CPPUNIT_ASSERT(_paxTypeFare);
    _paxTypeFare->setIsShoppingFare();
    _dc = DiagPtr(new DiagSoloPQCollector(*_trx, 0));

    _loc[0].loc() = "JFK";
    _loc[1].loc() = "DFW";
    _loc[2].loc() = "LAX";
    _loc[3].loc() = "SFA";

    _obVector.push_back(getDirFMPath(getFM(0, 1), getFM(1, 3)));
    _obVector.push_back(getDirFMPath(getFM(0, 2), getFM(2, 3)));

    _ibVector.push_back(getDirFMPath(getFM(3, 1), getFM(1, 0)));
    _ibVector.push_back(getDirFMPath(getFM(3, 2), getFM(2, 0)));
  }

  void tearDown() { _dataHandle.clear(); }

  void testPUValidation()
  {
    typedef std::pair<ConxRoutePQItemPtr, bool> CrItemPair; // bool: true - different connection
                                                            // point at OB and IB
    typedef std::vector<CrItemPair> CRItemVector;
    typedef std::pair<SolutionPattern::SPEnumType, bool> SPEnumPair; // bool: true - RT included,
                                                                     // false - no RT included

    const SolutionPatternStorage& storage(SolutionPatternStorage::instance());
    const SolutionPattern& defaultSp = storage.getSPById(SolutionPattern::SP31);

    // three components trip
    CRItemVector crItems3;
    // valid for OJ
    DirFMPathPtr leg1 = getDirFMPath(getFM(0, 3));
    DirFMPathPtr leg2 = getDirFMPath(getFM(3, 0));
    crItems3.push_back(
        CrItemPair(_pqItemMgr.constructCRPQItem(defaultSp, _obVector[0], leg2), true));
    crItems3.push_back(
        CrItemPair(_pqItemMgr.constructCRPQItem(defaultSp, _obVector[1], leg2), true));
    crItems3.push_back(
        CrItemPair(_pqItemMgr.constructCRPQItem(defaultSp, leg1, _ibVector[0]), true));
    crItems3.push_back(
        CrItemPair(_pqItemMgr.constructCRPQItem(defaultSp, leg1, _ibVector[1]), true));

    // invalid for OJ
    crItems3.push_back(CrItemPair(_pqItemMgr.constructCRPQItem(defaultSp, leg1, leg2), false));
    crItems3.push_back(CrItemPair(_pqItemMgr.constructCRPQItem(defaultSp, leg2, leg1), false));

    DirFMPathPtr fakeLeg1 = getDirFMPath(getFM(0, 0));
    crItems3.push_back(CrItemPair(
        _pqItemMgr.constructCRPQItem(defaultSp, fakeLeg1, getDirFMPath(getFM(0, 0))), false));

    std::vector<SPEnumPair> sp3Component;
    sp3Component.push_back(SPEnumPair(SolutionPattern::SP31, false)); // TOJ+OW
    sp3Component.push_back(SPEnumPair(SolutionPattern::SP32, false)); // OW+OOJ
    sp3Component.push_back(SPEnumPair(SolutionPattern::SP34, false)); // TOJ+OW
    sp3Component.push_back(SPEnumPair(SolutionPattern::SP35, false)); // OOJ+OW

    for (CrItemPair crItem : crItems3)
    {
      for (SPEnumPair spPair : sp3Component)
      {
        validateSP(storage.getSPById(spPair.first), crItem.first, spPair.second, crItem.second);
      }
    }

    // four components trip
    CRItemVector crItems4;
    // CR items with the same connection points
    crItems4.push_back(
        CrItemPair(_pqItemMgr.constructCRPQItem(defaultSp, _obVector[0], _ibVector[0]), false));
    crItems4.push_back(
        CrItemPair(_pqItemMgr.constructCRPQItem(defaultSp, _obVector[1], _ibVector[1]), false));
    // CR items with different connection points
    crItems4.push_back(
        CrItemPair(_pqItemMgr.constructCRPQItem(defaultSp, _obVector[0], _ibVector[1]), true));
    crItems4.push_back(
        CrItemPair(_pqItemMgr.constructCRPQItem(defaultSp, _obVector[1], _ibVector[0]), true));

    std::vector<SPEnumPair> sp4Component;
    sp4Component.push_back(SPEnumPair(SolutionPattern::SP41, true)); // RT+RT
    sp4Component.push_back(SPEnumPair(SolutionPattern::SP42, true)); // RT+OW+OW
    sp4Component.push_back(SPEnumPair(SolutionPattern::SP43, true)); // OW+RT+OW
    sp4Component.push_back(SPEnumPair(SolutionPattern::SP44, false)); // OJ+OJ
    sp4Component.push_back(SPEnumPair(SolutionPattern::SP45, false)); // TOJ+OW+OW
    sp4Component.push_back(SPEnumPair(SolutionPattern::SP46, false)); // OW+OOJ+OW
    sp4Component.push_back(SPEnumPair(SolutionPattern::SP49, false)); // CT

    for (CrItemPair crItem : crItems4)
    {
      for (SPEnumPair spPair : sp4Component)
      {
        validateSP(storage.getSPById(spPair.first), crItem.first, spPair.second, crItem.second);
      }
    }
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(PUFullTypeTest);
}
} // namespace tse::shpq
