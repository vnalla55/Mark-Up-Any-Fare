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
#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "Pricing/Shopping/PQ/ConxRoutePQItem.h"
#include "DataModel/CxrFareMarkets.h"
#include "DataModel/FareMarket.h"
#include "DBAccess/Loc.h"
#include "DataModel/OwrtFareMarket.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/PQ/SoloPQItemManager.h"
#include "Pricing/Shopping/PQ/SolutionPattern.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/testdata/TestPaxTypeFareFactory.h"
#include "test/testdata/TestShoppingTrxFactory.h"

#include <memory>
#include <vector>

namespace tse
{
namespace shpq
{

using namespace boost::assign;

class ConxRoutePQItemTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ConxRoutePQItemTest);
  CPPUNIT_TEST(onlineTest);
  CPPUNIT_TEST_SUITE_END();

private:
  typedef std::vector<ConxRoutePQItemPtr> CRVector;
  typedef std::shared_ptr<FareMarket> FareMarketPtr;
  typedef std::vector<FareMarketPtr> FMVector;

  TestMemHandle _dataHandle;
  ShoppingTrx* _trx;
  PaxTypeFare* _paxTypeFare;
  Loc _origin;
  Loc _destination;

  CarrierCode _cxr1;
  CarrierCode _cxr2;

  SoloPQItemManager _pqItemMgr;
  FMVector _fmVector; // to release FM

private:
  CxrFareMarketsPtr getFM(const CarrierCode& cxr)
  {
    CxrFareMarketsPtr cxrFM;
    if (!cxr.empty())
    {
      FareMarketPtr fm(new FareMarket());
      fm->origin() = &_origin;
      fm->destination() = &_destination;
      fm->governingCarrier() = cxr;
      fm->allPaxTypeFare().push_back(_paxTypeFare);
      _fmVector.push_back(fm);
      OwrtFareMarketPtr owFM = OwrtFareMarket::create(*_trx, HRT, fm.get(), 0);
      CPPUNIT_ASSERT_MESSAGE("OwrtFareMarket create", owFM);

      cxrFM = CxrFareMarkets::create(*_trx, HRT);
      CPPUNIT_ASSERT_MESSAGE("CxrFareMarkets create", cxrFM);

      cxrFM->insert(owFM);
    }
    return cxrFM;
  }

  DirFMPathPtr getDirFMPath(const CarrierCode& cxr1, const CarrierCode& cxr2)
  {
    DirFMPathPtr dirFM;
    if (!cxr1.empty() || !cxr2.empty())
      dirFM = DirFMPath::create(*_trx, getFM(cxr1), getFM(cxr2));
    return dirFM;
  }

  ConxRoutePQItemPtr getCRItem(CarrierCode cxr1 = "",
                               CarrierCode cxr2 = "",
                               CarrierCode cxr3 = "",
                               CarrierCode cxr4 = "")
  {
    const SolutionPattern& defaultSP =
        SolutionPatternStorage::instance().getSPById(SolutionPattern::SP31);

    DirFMPathPtr leg1 = getDirFMPath(cxr1, cxr2);
    DirFMPathPtr leg2 = getDirFMPath(cxr3, cxr4);
    CPPUNIT_ASSERT_MESSAGE("No legs for CRItem", leg1 || leg2);

    ConxRoutePQItemPtr cxrItem = _pqItemMgr.constructCRPQItem(defaultSP, leg1, leg2);
    CPPUNIT_ASSERT_MESSAGE("ConxRoutePQItem create", cxrItem);

    return cxrItem;
  }

public:
  void setUp()
  {
    _dataHandle.create<TestConfigInitializer>();
    _cxr1 = "DL";
    _cxr2 = "BA";

    _origin.loc() = "JFK";
    _destination.loc() = "DFW";

    _trx =
        TestShoppingTrxFactory::create("/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    _paxTypeFare = TestPaxTypeFareFactory::create("testdata/PaxTypeFare.xml", true);

    CPPUNIT_ASSERT(_trx);
    CPPUNIT_ASSERT(_paxTypeFare);
    _paxTypeFare->setIsShoppingFare();
  }

  void tearDown() { _dataHandle.clear(); }

  void onlineTest()
  {
    CRVector crVec;

    crVec += getCRItem(_cxr1), getCRItem(_cxr1, _cxr1), getCRItem(_cxr1, "", _cxr1),
        getCRItem(_cxr1, _cxr1, _cxr1), getCRItem(_cxr1, "", _cxr1, _cxr1),
        getCRItem(_cxr1, _cxr1, _cxr1, _cxr1);

    for (CRVector::size_type ind = 0, size = crVec.size(); ind < size; ++ind)
    {
      CPPUNIT_ASSERT_MESSAGE((boost::format("Online solution test - index %d") % ind).str(),
                             crVec[ind]->isOnlineSolution());
    }
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ConxRoutePQItemTest);
}
} // namespace tse::shpq
