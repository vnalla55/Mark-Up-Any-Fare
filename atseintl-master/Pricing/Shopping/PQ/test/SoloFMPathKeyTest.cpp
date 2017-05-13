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

#include <vector>
#include <set>

#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>

#include "DataModel/FareMarket.h"
#include "DBAccess/Loc.h"
#include "Pricing/Shopping/PQ/SoloFMPathKey.h"

#include <memory>

namespace tse
{
namespace shpq
{

using boost::assign::operator+=;

const uint16_t LocNumber = 4;
const uint16_t CxrNumber = 2;

class SoloFMPathKeyTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SoloFMPathKeyTest);
  CPPUNIT_TEST(mfmKeyTest);
  CPPUNIT_TEST(SoloFMKeyTest);
  CPPUNIT_TEST_SUITE_END();

private:
  typedef std::shared_ptr<FareMarket> FareMarketPtr;
  typedef std::vector<FareMarketPtr> FMVector;

  Loc _loc[LocNumber];
  CarrierCode _cxr[CxrNumber];
  FMVector _fmVector;

private:
  FareMarketPtr newFareMarket(Loc& origin, Loc& destination, CarrierCode& cxr)
  {
    FareMarketPtr fm(new FareMarket());
    fm->origin() = &origin;
    fm->destination() = &destination;
    fm->governingCarrier() = cxr;

    return fm;
  }

public:
  void setUp()
  {
    // low to high order
    _loc[0].loc() = "DFW";
    _loc[1].loc() = "JFK";
    _loc[2].loc() = "LAX";
    _loc[3].loc() = "SFA";

    // low to high order
    _cxr[0] = "DL";
    _cxr[1] = "LH";
  }

  void mfmKeyTest()
  {
    typedef std::set<MergedFMKey, MFMKeyLess> MFMKeySet;
    typedef std::pair<MFMKeySet::iterator, bool> InsertResult;

    FareMarketPtr lowestFM = newFareMarket(_loc[0], _loc[0], _cxr[0]);
    FMVector fmVector;
    fmVector += newFareMarket(_loc[0], _loc[1], _cxr[0]), newFareMarket(_loc[0], _loc[2], _cxr[0]),
        newFareMarket(_loc[0], _loc[3], _cxr[0]), newFareMarket(_loc[1], _loc[0], _cxr[0]),
        newFareMarket(_loc[2], _loc[0], _cxr[0]), newFareMarket(_loc[3], _loc[0], _cxr[0]),
        newFareMarket(_loc[3], _loc[0], _cxr[1]);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("MFMKey number", (size_t)7, fmVector.size());

    MFMKeySet mfmSet;
    MergedFMKey lowestMFMKey(lowestFM.get());

    InsertResult insertResult = mfmSet.insert(lowestMFMKey);
    CPPUNIT_ASSERT_MESSAGE("Insert MFMKey fail (lowestMFMKey)", insertResult.second);

    insertResult = mfmSet.insert(lowestMFMKey);
    CPPUNIT_ASSERT_MESSAGE("Second insert MFMKey fail (lowestMFMKey)", !insertResult.second);

    MFMKeyLess mfmLess;
    for (FareMarketPtr fm : fmVector)
    {
      MergedFMKey mfmKey(fm.get());
      CPPUNIT_ASSERT_MESSAGE("MergedFMKey compare less fail", mfmLess(lowestMFMKey, mfmKey));
      CPPUNIT_ASSERT_MESSAGE("MergedFMKey reverse compare less fail",
                             !mfmLess(mfmKey, lowestMFMKey));

      insertResult = mfmSet.insert(mfmKey);
      CPPUNIT_ASSERT_MESSAGE("Insert MFMKey fail (mfmKey)", insertResult.second);

      insertResult = mfmSet.insert(mfmKey);
      CPPUNIT_ASSERT_MESSAGE("Second insert MFMKey fail (mfmKey)", !insertResult.second);
    }

    for (FMVector::size_type ind = 0, size = fmVector.size(); ind < size; ++ind)
    {
      MergedFMKey lowestKey(fmVector[ind].get());
      for (FMVector::size_type i = ind + 1; i < size; ++i)
      {
        MergedFMKey testKey(fmVector[i].get());
        CPPUNIT_ASSERT_MESSAGE("MFMKey compare Less (loop)", mfmLess(lowestKey, testKey));
        CPPUNIT_ASSERT_MESSAGE("MFMKey reverse compare Less (loop)", !mfmLess(testKey, lowestKey));
      }
    }
  }

  void SoloFMKeyTest()
  {
    typedef std::set<SoloFMPathKey, SoloFMPathKeyLess> FMPathSet;
    typedef std::pair<FMPathSet::iterator, bool> InsertResult;

    FareMarketPtr lowestFM = newFareMarket(_loc[0], _loc[0], _cxr[0]);
    FMVector fmVector;
    fmVector += newFareMarket(_loc[0], _loc[1], _cxr[0]), newFareMarket(_loc[0], _loc[2], _cxr[0]),
        newFareMarket(_loc[0], _loc[2], _cxr[1]);

    FMPathSet fmPathSet;
    SoloFMPathKeyLess fmPathLess;

    SoloFMPathKey lowestFMKey(lowestFM.get());
    lowestFMKey.addFareMarket(fmVector[0].get());

    InsertResult insertResult = fmPathSet.insert(lowestFMKey);
    CPPUNIT_ASSERT_MESSAGE("Insert SoloFMPathKey fail (lowest)", insertResult.second);

    insertResult = fmPathSet.insert(lowestFMKey);
    CPPUNIT_ASSERT_MESSAGE("Second insert SoloFMPathKey fail (lowest)", !insertResult.second);

    SoloFMPathKey testKey(lowestFM.get());
    testKey.addFareMarket(fmVector[1].get());
    insertResult = fmPathSet.insert(testKey);
    CPPUNIT_ASSERT_MESSAGE("Insert SoloFMPathKey fail (testKey)", insertResult.second);

    insertResult = fmPathSet.insert(testKey);
    CPPUNIT_ASSERT_MESSAGE("Second insert SoloFMPathKey fail (testKey)", !insertResult.second);

    CPPUNIT_ASSERT_MESSAGE("SoloFMPathKey compare less fail", fmPathLess(lowestFMKey, testKey));

    CPPUNIT_ASSERT_MESSAGE("SoloFMPathKey reverse compare less fail",
                           !fmPathLess(testKey, lowestFMKey));

    // 3 FM keys
    // lowestKey already have two faremarkets(lowestFM and fmVector[0])
    lowestFMKey.addFareMarket(fmVector[1].get());
    insertResult = fmPathSet.insert(lowestFMKey);
    CPPUNIT_ASSERT_MESSAGE("Insert SoloFMPathKey fail (lowest 3FM key)", insertResult.second);

    insertResult = insertResult = fmPathSet.insert(lowestFMKey);
    CPPUNIT_ASSERT_MESSAGE("Second insert SoloFMPathKey fail (lowest 3 FM key)",
                           !insertResult.second);

    SoloFMPathKey testKey2(lowestFM.get());
    testKey2.addFareMarket(fmVector[0].get());
    testKey2.addFareMarket(fmVector[2].get()); // shoud be different from lowestFMKey at third FM
    insertResult = insertResult = fmPathSet.insert(testKey2);
    CPPUNIT_ASSERT_MESSAGE("Insert SoloFMPathKey fail (testKey2)", insertResult.second);

    insertResult = insertResult = fmPathSet.insert(testKey2);
    CPPUNIT_ASSERT_MESSAGE("Second insert SoloFMPathKey fail (testKey2)", !insertResult.second);

    CPPUNIT_ASSERT_MESSAGE("SoloFMPathKey compare less fail", fmPathLess(lowestFMKey, testKey2));

    CPPUNIT_ASSERT_MESSAGE("SoloFMPathKey reverse compare less fail",
                           !fmPathLess(testKey2, lowestFMKey));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SoloFMPathKeyTest);
}
} // namespace tse::shqp
