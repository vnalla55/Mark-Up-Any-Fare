// -------------------------------------------------------------------
//
//!  Copyright (C) Sabre 2011
//!
//!          The copyright to the computer program(s) herein
//!          is the property of Sabre.
//!          The program(s) may be used and/or copied only with
//!          the written permission of Sabre or in accordance
//!          with the terms and conditions stipulated in the
//!          agreement/contract under which the program(s)
//!          have been supplied.
// -------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/CxrFareMarkets.h"
#include "DataModel/DirFMPath.h"
#include "DataModel/DirFMPathList.h"
#include "DataModel/DirFMPathListCollector.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/ShoppingTrx.h"
#include "DBAccess/DataHandle.h"
#include "Pricing/Shopping/PQ/CxrFMCollector.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestLogger.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestPaxTypeFareFactory.h"
#include "test/testdata/TestShoppingTrxFactory.h"

#include <memory>
#include <vector>

namespace tse
{
namespace shpq
{

class CxrFMCollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CxrFMCollectorTest);
  CPPUNIT_TEST(testAddFareMarkets);
  CPPUNIT_TEST(testCollectDirFMPathList);
  CPPUNIT_TEST_SUITE_END();

private:
  typedef std::shared_ptr<FareMarket> FareMarketPtr;
  typedef std::vector<FareMarketPtr> FMVector;

  static const uint16_t LOCNUMBER = 4;
  static const uint16_t CXRNUMBER = 2;
  static const FMVector::size_type FMNUMBER = 6; // number of FareMarkets with the same OnD

  TestMemHandle _dataHandle;
  ShoppingTrx* _trx;
  std::vector<MultiAirportLocationPair*> _loc;
  CarrierCode _cxr[CXRNUMBER];
  FMVector _fmVector;
  PaxTypeFare* _paxTypeFare;

  CxrFMCollector::CitySet _emptyCitySet;

  CxrFMCollectorPtr _cxrFMCollector;
  DirFMPathListCollectorPtr _dirPathCollector;

  MultiAirportLocationPair* _legOrigin;
  MultiAirportLocationPair* _legDestination;

  FareMarketPtr newFareMarket(CarrierCode& cxr,
                              const Loc* origin,
                              const Loc* destination,
                              FareMarket::SOL_COMPONENT_DIRECTION direction)
  {
    FareMarketPtr fm(new FareMarket());
    fm->origin() = origin;
    fm->destination() = destination;
    fm->governingCarrier() = cxr;
    fm->boardMultiCity() = origin->loc();
    fm->offMultiCity() = destination->loc();
    fm->setSolComponentDirection(direction);

    return fm;
  }

  void createFareMarkets()
  {
    for (uint16_t indOrig = 0; indOrig < LOCNUMBER; ++indOrig)
    {
      for (uint16_t indDest = indOrig + 1; indDest < LOCNUMBER; ++indDest)
      {
        bool isThru = (indOrig == 0 && indDest == LOCNUMBER - 1);

        FareMarket::SOL_COMPONENT_DIRECTION dir(FareMarket::SOL_COMPONENT_UNDEFINED);
        if (indOrig == 0)
          dir = FareMarket::SOL_COMPONENT_ORIGIN;
        else if (indDest == LOCNUMBER - 1)
          dir = FareMarket::SOL_COMPONENT_DESTINATION;

        _fmVector.push_back(newFareMarket(
            _cxr[0], _loc[indOrig]->getLocation(), _loc[indDest]->getLocation(), dir));
        _fmVector.back()->setFmTypeSol(
            (isThru ? FareMarket::SOL_FM_THRU : FareMarket::SOL_FM_LOCAL));
        _fmVector.push_back(newFareMarket(
            _cxr[1], _loc[indOrig]->getLocation(), _loc[indDest]->getLocation(), dir));
        _fmVector.back()->setFmTypeSol(
            (isThru ? FareMarket::SOL_FM_THRU : FareMarket::SOL_FM_LOCAL));
      }
    }
  }

  void addFareMarkets(bool result)
  {
    for (const FareMarketPtr fm : _fmVector)
    {
      CPPUNIT_ASSERT(fm != 0);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Adding fare market to CxrFMCollector",
                                   result,
                                   _cxrFMCollector->addFareMarket(fm.get(), 0));
    }
  }

  void checkFMSize(size_t size)
  {
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "CxrFareMarkets - added FM size", size, _cxrFMCollector->_addedFareMarkets.size());
  }

  bool isCxrFMCollectorEmpty(CxrFMCollectorPtr cxrFM)
  {
    if (!cxrFM->_addedFareMarkets.empty())
      return false;
    if (cxrFM->_thruFM.isValid())
      return false;
    for (CxrFMCollector::CxrFMMap::value_type mapPair : cxrFM->_twoSegmentCxrFM)
    {
      CxrFMCollector::TwoSegmentLeg& leg = mapPair.second;
      CxrFMCollector::OwHrtFMPair& cxrSeg1 =
          leg._segments[CxrFMCollector::TwoSegmentLeg::ORIGIN_SEG];
      CxrFMCollector::OwHrtFMPair& cxrSeg2 = leg._segments[CxrFMCollector::TwoSegmentLeg::DEST_SEG];
      if (cxrSeg1.isValid() || cxrSeg2.isValid())
        return false;
    }

    return true;
  }

  void checkCxrNumber(CxrFMCollector::OwHrtFMPair& cxrFM, const std::string& msg)
  {
    CxrFareMarketsPtr owCxrFM = cxrFM.getCxrFM(OW);
    CxrFareMarketsPtr hrtCxrFM = cxrFM.getCxrFM(HRT);
    CPPUNIT_ASSERT_MESSAGE(msg + "(OW)", owCxrFM);
    CPPUNIT_ASSERT_MESSAGE(msg + "(HRT)", hrtCxrFM);
  }

  void checkOnD(CxrFMCollector::OwHrtFMPair& cxrFM,
                const LocationPair& origin,
                const LocationPair& destination,
                const std::string& msg)
  {
    CxrFareMarketsPtr owCxrFM = cxrFM.getCxrFM(OW);
    CxrFareMarketsPtr hrtCxrFM = cxrFM.getCxrFM(HRT);

    std::ostringstream oss;
    oss << msg << "(origin OW) " << owCxrFM->getOriginLocation().toString() << ", "
        << origin.toString();

    CPPUNIT_ASSERT_MESSAGE(oss.str(), ((owCxrFM->getOriginLocation()) == origin));

    oss.str("");
    oss << msg << "(origin HRT) " << hrtCxrFM->getOriginLocation().toString() << ", "
        << origin.toString();
    CPPUNIT_ASSERT_MESSAGE(oss.str(), ((hrtCxrFM->getOriginLocation()) == origin));

    oss.str("");
    oss << msg << "(destination OW) " << owCxrFM->getDestinationLocation().toString() << ", "
        << destination.toString();
    CPPUNIT_ASSERT_MESSAGE(oss.str(), ((owCxrFM->getDestinationLocation()) == destination));

    oss.str("");
    oss << msg << "(destination HRT) " << hrtCxrFM->getDestinationLocation().toString() << ", "
        << destination.toString();
    CPPUNIT_ASSERT_MESSAGE(oss.str(), ((hrtCxrFM->getDestinationLocation()) == destination));
  }

  void checkCxrCollector()
  {
    CPPUNIT_ASSERT_MESSAGE("Invalid ThruFM", _cxrFMCollector->_thruFM.isValid());
    checkCxrNumber(_cxrFMCollector->_thruFM, "No Thru CxrFareMarkets");
    checkOnD(_cxrFMCollector->_thruFM, *_legOrigin, *_legDestination, "Thru location compare fail");

    CPPUNIT_ASSERT_MESSAGE("Two segments CxrFM empty", !_cxrFMCollector->_twoSegmentCxrFM.empty());

    for (CxrFMCollector::CxrFMMap::value_type& mapPair : _cxrFMCollector->_twoSegmentCxrFM)
    {
      CxrFMCollector::TwoSegmentLeg& leg = mapPair.second;
      CxrFMCollector::OwHrtFMPair& cxrSeg1 =
          leg._segments[CxrFMCollector::TwoSegmentLeg::ORIGIN_SEG];
      CxrFMCollector::OwHrtFMPair& cxrSeg2 = leg._segments[CxrFMCollector::TwoSegmentLeg::DEST_SEG];
      const LocationPair& breakPoint = mapPair.first;

      if (cxrSeg1.isValid())
      {
        checkCxrNumber(cxrSeg1, "No FBR CxrFareMarkets - segment1");
        checkOnD(cxrSeg1, *_legOrigin, breakPoint, "FBR location compare fail - segment1");
      }
      if (cxrSeg2.isValid())
      {
        checkCxrNumber(cxrSeg2, "No FBR CxrFareMarkets - segment2");
        checkOnD(cxrSeg2, breakPoint, *_legDestination, "FBR location compare fail - segment2");
      }
    }
  }

  void addPaxTypeFare()
  {
    for (FareMarketPtr fm : _fmVector)
    {
      fm->allPaxTypeFare().push_back(_paxTypeFare);
    }
  }

public:
  void setUp()
  {
    _dataHandle.create<TestConfigInitializer>();
    _dataHandle.create<LoggerGetOff>("atseintl.DataModel.Diversity");

    _trx =
        TestShoppingTrxFactory::create("/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    _paxTypeFare = TestPaxTypeFareFactory::create("testdata/PaxTypeFare.xml", true);
    _paxTypeFare->setIsShoppingFare();

    _loc.push_back(createLocPair("JFK"));
    _loc.push_back(createLocPair("DFW"));
    _loc.push_back(createLocPair("LAX"));
    _loc.push_back(createLocPair("SFA"));

    _cxr[0] = "DL";
    _cxr[1] = "LH";

    _legOrigin = _loc[0];
    _legDestination = _loc[LOCNUMBER - 1];

    AirSeg* originSeg = _dataHandle.create<AirSeg>();
    originSeg->origin() = _legOrigin->getLocation();
    originSeg->boardMultiCity() = _legOrigin->getMultiCity();

    AirSeg* destinationSeg = _dataHandle.create<AirSeg>();
    destinationSeg->destination() = _legDestination->getLocation();
    destinationSeg->offMultiCity() = _legDestination->getMultiCity();

    CPPUNIT_ASSERT(_trx);
    _cxrFMCollector.reset(new CxrFMCollector(*_trx, originSeg, destinationSeg, _emptyCitySet));
    CPPUNIT_ASSERT(_cxrFMCollector);

    createFareMarkets();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("FareMarket vector size", FMNUMBER * CXRNUMBER, _fmVector.size());

    CPPUNIT_ASSERT_MESSAGE("PaxTypeFare - empty", _paxTypeFare);
    CPPUNIT_ASSERT_MESSAGE("PaxTypeFare is invalid", _paxTypeFare->isValid());
  }

  void tearDown()
  {
    _fmVector.clear();
    _dataHandle.clear();
  }

  MultiAirportLocationPair* createLocPair(const LocCode& code)
  {
    Loc* loc = _dataHandle.create<Loc>();
    loc->loc() = code;
    bool isConnection(true);
    return _dataHandle.create<MultiAirportLocationPair>(loc, code, isConnection);
  }

  void testAddFareMarkets()
  {
    // add empty fare markets
    addFareMarkets(false);
    checkFMSize(0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "CxrFMCollector is not empty", true, isCxrFMCollectorEmpty(_cxrFMCollector));

    // add not empty fare markets
    _cxrFMCollector->_addedFareMarkets.clear();
    addPaxTypeFare();
    addFareMarkets(true);
    checkFMSize(FMNUMBER * CXRNUMBER);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "CxrFMCollector is empty", false, isCxrFMCollectorEmpty(_cxrFMCollector));

    // try to add same fare markets again
    addFareMarkets(false);
    checkFMSize(FMNUMBER * CXRNUMBER);

    // check CxrFMCollector
    checkCxrCollector();
  }

  void testCollectDirFMPathList()
  {
    addPaxTypeFare();
    addFareMarkets(true);

    _dirPathCollector.reset(new DirFMPathListCollector(*_trx));
    CPPUNIT_ASSERT(_dirPathCollector);
    _cxrFMCollector->collectDirFMPathList(_dirPathCollector);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Solution type number", 6lu, _dirPathCollector->size());

    const SolutionType sTypes[] = { OW, HRT, OW_HRT, OW_OW, HRT_OW, HRT_HRT };

    // for each Solution type size of DirFMPath should be equal as number of break points
    // For solution type OW and HRT size is always 1.
    for (const SolutionType& sType : sTypes)
    {
      size_t numberOfSolution =
          (sType == OW || sType == HRT) ? 1 : LOCNUMBER - 2; // no of break points

      DirFMPathListPtr pathList = _dirPathCollector->getDirFMPathList(sType);
      CPPUNIT_ASSERT(pathList);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("DirFMPath number", numberOfSolution, pathList->size());

      for (DirFMPathPtr dirPath : *pathList)
      {
        CPPUNIT_ASSERT(dirPath);
        const LocationPair& origin = (*dirPath->_segments.begin())->getOriginLocation();
        const LocationPair& dest = (*dirPath->_segments.rbegin())->getDestinationLocation();
        CPPUNIT_ASSERT_MESSAGE("Origin compare", origin == *_loc[0]);

        CPPUNIT_ASSERT_MESSAGE("Destination compare", dest == *_loc[LOCNUMBER - 1]);
      }
    }
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CxrFMCollectorTest);
}
} // namespace tse::shpq
