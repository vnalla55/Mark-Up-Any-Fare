#include "test/include/CppUnitHelperMacros.h"
#include "ItinAnalyzer/SoloCarnivalFareFamiliesGrouperImpl.h"
#include "DataModel/Itin.h"
#include "ItinAnalyzer/test/TravelSegmentTestUtil.h"

namespace
{

typedef std::vector<tse::Itin*> ItinVec;

tse::TravelSeg*
createSegment(const std::string& carrier, const std::string& origin, const std::string& destination)
{
  return tse::createAirSegment(origin, destination, carrier, "2011-01-01");
}

tse::TravelSeg*
createArunk(const std::string& origin, const std::string& destination)
{
  return tse::createArunkSegment(origin, destination);
}

tse::Itin*
createItin(int o, const std::vector<tse::TravelSeg*>& s)
{
  tse::Itin* i = new tse::Itin();

  i->setItinFamily(o);
  for (tse::TravelSeg* seg : s)
  {
    i->travelSeg().push_back(seg);
  }

  return i;
}
}

class SoloCarnivalRegroupThruFamiliesTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SoloCarnivalRegroupThruFamiliesTest);
  CPPUNIT_TEST(testGrouping1);
  CPPUNIT_TEST_SUITE_END();

public:
  void testGrouping1()
  {
    /*
    Input:

    Itin1
      segments:
        BA: LON -> DFW
        AA: DFW -> LAX
      original-family: 3

    Itin2
      segments:
        BA: LON -> DFW
        AA: DFW -> LAX
      original-family: 3

    Itin3
      segments:
        BA: LON -> DFW
        LX: DFW -> LAX
      original-family: 3

    Itin4
      segments:
        BA: LON -> DFW
        AA: DFW -> LAX
      original-family: 18
    */
    /*
    Expect as output:

    Itin1
      family: 9001
      segments:
        BA: LON -> DFW
        AA: DFW -> LAX

      similar:
        Itin2, Itin4

    Itin3
      family: 9002
      segments:
        BA: LON -> DFW
        LX: DFW -> LAX
    */
    tse::Itin* i1 =
        createItin(3, {createSegment("BA", "LON", "DFW"), createSegment("AA", "DFW", "LAX")});
    tse::Itin* i2 =
        createItin(3, {createSegment("BA", "LON", "DFW"), createSegment("AA", "DFW", "LAX")});

    // carrier differs from previous two, as well as from the next one
    // (expect this one to be in separate family)         vv
    //                                                    VV
    tse::Itin* i3 =
        createItin(3, {createSegment("BA", "LON", "DFW"), createSegment("LX", "DFW", "LAX")});

    tse::Itin* i4 =
        createItin(18, {createSegment("BA", "LON", "DFW"), createSegment("AA", "DFW", "LAX")});

    ItinVec itins;
    itins = {i1, i2, i3, i4};

    ItinVec result = groupFareFamilies(itins);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), result.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), i1->getSimilarItins().size());
    CPPUNIT_ASSERT_EQUAL(i2, i1->getSimilarItins().at(0).itin);
    CPPUNIT_ASSERT_EQUAL(i4, i1->getSimilarItins().at(1).itin);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), i2->getSimilarItins().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), i3->getSimilarItins().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), i4->getSimilarItins().size());

    delete i4;
    delete i3;
    delete i2;
    delete i1;
  }

protected:
  ItinVec groupFareFamilies(const ItinVec& input)
  {
    ItinVec result(input);
    tse::SoloCarnivalFareFamiliesGrouperImpl::groupThruFamilies(result);
    return result;
  }
};

class SoloCarnivalRegroupWithinFamiliesTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SoloCarnivalRegroupWithinFamiliesTest);
  CPPUNIT_TEST(testGrouping1);
  CPPUNIT_TEST(testGrouping2);
  CPPUNIT_TEST(testGrouping3);
  CPPUNIT_TEST(testGrouping4);
  CPPUNIT_TEST_SUITE_END();

public:
  void testGrouping1()
  {
    /*
    Input:

    Itin1
      segments:
        BA: LON -> DFW
        AA: DFW -> LAX
      original-family: 3

    Itin2
      segments:
        BA: LON -> DFW
        AA: DFW -> LAX
      original-family: 3

    Itin3
      segments:
        BA: LON -> DFW
        LX: DFW -> LAX
      original-family: 3

    Itin4
      segments:
        BA: LON -> DFW
        AA: DFW -> LAX
      original-family: 18
    */
    /*
    Expect as output:

    Itin1
      family: 9001
      segments:
        BA: LON -> DFW
        AA: DFW -> LAX

      similar:
        Itin2

    Itin3
      family: 9002
      segments:
        BA: LON -> DFW
        LX: DFW -> LAX

    Itin4
      family: 9003
      segments:
        BA: LON -> DFW
        AA: DFW -> LAX
    */
    tse::Itin* i1 =
        createItin(3, {createSegment("BA", "LON", "DFW"), createSegment("AA", "DFW", "LAX")});
    tse::Itin* i2 =
        createItin(3, {createSegment("BA", "LON", "DFW"), createSegment("AA", "DFW", "LAX")});
    tse::Itin* i3 =
        createItin(3, {createSegment("BA", "LON", "DFW"), createSegment("LX", "DFW", "LAX")});
    tse::Itin* i4 =
        createItin(18, {createSegment("BA", "LON", "DFW"), createSegment("AA", "DFW", "LAX")});

    ItinVec itins;
    itins = {i1, i2, i3, i4};

    ItinVec result = groupFareFamilies(itins);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), result.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), i1->getSimilarItins().size());
    CPPUNIT_ASSERT_EQUAL(i2, i1->getSimilarItins().at(0).itin);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), i2->getSimilarItins().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), i3->getSimilarItins().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), i4->getSimilarItins().size());

    delete i4;
    delete i3;
    delete i2;
    delete i1;
  }

  void testGrouping2()
  {
    tse::Itin* i1 =
        createItin(3, {createSegment("BA", "LON", "DFW"), createSegment("AA", "DFW", "LAX")});
    tse::Itin* i2 =
        createItin(3, {createSegment("BA", "LON", "DFW"), createSegment("AA", "DFW", "LAX")});
    tse::Itin* i3 =
        createItin(3, {createSegment("BA", "LON", "DFW"), createSegment("LX", "DFW", "LAX")});
    tse::Itin* i4 =
        createItin(3, {createSegment("BA", "LON", "DFW"), createSegment("AA", "DFW", "LAX")});

    ItinVec itins;
    itins = {i1, i2, i3, i4};

    ItinVec result = groupFareFamilies(itins);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), result.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), i1->getSimilarItins().size());
    CPPUNIT_ASSERT_EQUAL(i2, i1->getSimilarItins().at(0).itin);
    CPPUNIT_ASSERT_EQUAL(i4, i1->getSimilarItins().at(1).itin);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), i2->getSimilarItins().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), i3->getSimilarItins().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), i4->getSimilarItins().size());

    delete i4;
    delete i3;
    delete i2;
    delete i1;
  }

  void testGrouping3()
  {
    tse::Itin* i1 = createItin(3,
                               {createSegment("BA", "LON", "DFW"),
                                createSegment("AA", "DFW", "LAX"),
                                createSegment("AA", "LAX", "NYC")});

    tse::Itin* i2 =
        createItin(3, {createSegment("BA", "LON", "DFW"), createSegment("AA", "DFW", "NYC")});

    tse::Itin* i3 =
        createItin(3, {createSegment("BA", "LON", "DFW"), createSegment("AA", "DFW", "LAX")});

    tse::Itin* i4 = createItin(3, {createSegment("BA", "LON", "DFW"), createArunk("DFW", "LAX")});

    ItinVec itins;
    itins = {i1, i2, i3, i4};

    ItinVec result = groupFareFamilies(itins);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), result.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), i1->getSimilarItins().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), i2->getSimilarItins().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), i3->getSimilarItins().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), i4->getSimilarItins().size());

    delete i4;
    delete i3;
    delete i2;
    delete i1;
  }

  void testGrouping4()
  {
    tse::Itin* i1 = createItin(3,
                               {createSegment("BA", "LON", "DFW"),
                                createArunk("DFW", "LAX"),
                                createSegment("AA", "LAX", "NYC")});

    tse::Itin* i2 = createItin(3,
                               {createSegment("BA", "LON", "DFW"),
                                createArunk("DFW", "LAX"),
                                createSegment("AA", "LAX", "NYC")});

    ItinVec itins;
    itins = {i1, i2};

    ItinVec result = groupFareFamilies(itins);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), result.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), i1->getSimilarItins().size());
    CPPUNIT_ASSERT_EQUAL(i2, i1->getSimilarItins().at(0).itin);

    delete i2;
    delete i1;
  }

protected:
  ItinVec groupFareFamilies(const ItinVec& input)
  {
    ItinVec result(input);
    tse::SoloCarnivalFareFamiliesGrouperImpl::groupWithinFamilies(result);
    return result;
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SoloCarnivalRegroupThruFamiliesTest);
CPPUNIT_TEST_SUITE_REGISTRATION(SoloCarnivalRegroupWithinFamiliesTest);
