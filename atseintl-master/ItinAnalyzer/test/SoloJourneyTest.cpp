#include "Common/ClassOfService.h"
#include "Common/ShoppingUtil.h"
#include "DataModel/AirSeg.h"
#include "ItinAnalyzer/SoloJourney.h"

#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include "test/include/CppUnitHelperMacros.h"

namespace tse
{
class SoloJourneyTest : public CppUnit::TestFixture
{
public:
  CPPUNIT_TEST_SUITE(SoloJourneyTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testGetLocalAvl);
  CPPUNIT_TEST(testGetJourneyFlowAvl);
  CPPUNIT_TEST(testGetFlowAvl);
  CPPUNIT_TEST(testIsFlowAvailFromAvs_False);
  CPPUNIT_TEST(testIsFlowAvailFromAvs_True);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
  }


  void tearDown()
  {
    _memHandle.clear();
  }

  void testConstructor();
  void testGetLocalAvl();
  void testGetJourneyFlowAvl();
  void testGetFlowAvl();
  void testIsFlowAvailFromAvs_False();
  void testIsFlowAvailFromAvs_True();

private:
  void buildTravelSeg(Itin& itin,
                      std::vector<AirSeg>& segments,
                      std::vector<ClassOfService>& cosVec,
                      const size_t size);

  void insertAvl(PricingTrx& trx,
                 Itin& itin,
                 std::vector<ClassOfServiceList>* avl,
                 SegmentRange range);

  ClassOfServiceList buildCOSList(size_t numClasses, uint16_t numSeats);

  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SoloJourneyTest);

// CPPUNIT_ASSERT(range4.intersects(range1));

void
SoloJourneyTest::testConstructor()
{
  PricingTrx trx;
  Itin itin;
  SoloJourney journey(trx, &itin, 0);

  SegmentRange range = journey.getRange();
  CPPUNIT_ASSERT_EQUAL(size_t(0), range.getStartIdx());
  CPPUNIT_ASSERT_EQUAL(size_t(0), range.getEndIdx());
  CPPUNIT_ASSERT_EQUAL(size_t(0), range.getSize());

  journey.endJourney(2);
  range = journey.getRange();
  CPPUNIT_ASSERT_EQUAL(size_t(0), range.getStartIdx());
  CPPUNIT_ASSERT_EQUAL(size_t(2), range.getEndIdx());
  CPPUNIT_ASSERT_EQUAL(size_t(2), range.getSize());
}

void
SoloJourneyTest::buildTravelSeg(Itin& itin,
                                std::vector<AirSeg>& segments,
                                std::vector<ClassOfService>& cosVec,
                                const size_t size)
{
  cosVec.resize(size);
  segments.resize(size);
  for (size_t i = 0; i < size; ++i)
  {
    ClassOfService& cos = cosVec[i];
    cos.numSeats() = i;
    cos.bookingCode() = 'A' + i;

    AirSeg& seg = segments[i];
    seg.originalId() = i;
    seg.classOfService().push_back(&cos);

    itin.travelSeg().push_back(&seg);
  }
}

void
SoloJourneyTest::insertAvl(PricingTrx& trx,
                           Itin& itin,
                           std::vector<ClassOfServiceList>* avl,
                           SegmentRange range)
{
  std::vector<TravelSeg*> keySeg;
  for (size_t i = range.getStartIdx(); i < range.getEndIdx(); ++i)
  {
    keySeg.push_back(itin.travelSeg()[i]);
  }
  trx.availabilityMap().insert(std::make_pair(ShoppingUtil::buildAvlKey(keySeg), avl));
}

ClassOfServiceList
SoloJourneyTest::buildCOSList(size_t numClasses, uint16_t numSeats)
{
  ClassOfServiceList cosList;

  for (size_t i = 0; i < numClasses; ++i)
  {
    ClassOfService* cos = _memHandle.create<ClassOfService>();
    cos->cabin().setEconomyClass();
    cos->bookingCode() = static_cast<char>('A' + i);
    cos->numSeats() = numSeats;

    cosList.push_back(cos);
  }

  return cosList;
}

void
SoloJourneyTest::testGetLocalAvl()
{
  PricingTrx trx;
  std::vector<AirSeg> segments;
  std::vector<ClassOfService> cosVec;
  Itin itin;
  SOPUsage sopUsage;
  sopUsage.itin_ = &itin;

  buildTravelSeg(itin, segments, cosVec, 3);

  SoloJourney journey(trx, &itin, 0);
  journey.endJourney(2);

  ClassOfServiceList* avl = journey.getLocalAvl(0, sopUsage);
  CPPUNIT_ASSERT(avl);
  CPPUNIT_ASSERT_EQUAL(size_t(1), avl->size());
  CPPUNIT_ASSERT_EQUAL(cosVec[0].bookingCode(), (*avl)[0]->bookingCode());

  avl = journey.getLocalAvl(1, sopUsage);
  CPPUNIT_ASSERT(avl);
  CPPUNIT_ASSERT_EQUAL(size_t(1), avl->size());
  CPPUNIT_ASSERT_EQUAL(cosVec[1].bookingCode(), (*avl)[0]->bookingCode());

  std::vector<ClassOfService> reqCosVec;
  std::vector<std::vector<ClassOfServiceList> > reqAvl;
  reqCosVec.resize(itin.travelSeg().size());
  reqAvl.resize(reqCosVec.size());

  for (size_t i = 0; i < itin.travelSeg().size(); ++i)
  {
    reqCosVec[i].bookingCode() = 'E' + i;
    reqAvl[i].resize(1);
    reqAvl[i][0].push_back(&reqCosVec[i]);
    insertAvl(trx, itin, &(reqAvl[i]), SegmentRange(i, i + 1));
  }

  avl = journey.getLocalAvl(0, sopUsage);
  CPPUNIT_ASSERT(avl);
  CPPUNIT_ASSERT_EQUAL(size_t(1), avl->size());
  CPPUNIT_ASSERT_EQUAL(reqCosVec[0].bookingCode(), (*avl)[0]->bookingCode());

  avl = journey.getLocalAvl(1, sopUsage);
  CPPUNIT_ASSERT(avl);
  CPPUNIT_ASSERT_EQUAL(size_t(1), avl->size());
  CPPUNIT_ASSERT_EQUAL(reqCosVec[1].bookingCode(), (*avl)[0]->bookingCode());
}

void
SoloJourneyTest::testGetJourneyFlowAvl()
{
  PricingTrx trx;
  std::vector<AirSeg> segments;
  std::vector<ClassOfService> cosVec;
  Itin itin;
  SOPUsage sopUsage;
  sopUsage.itin_ = &itin;

  buildTravelSeg(itin, segments, cosVec, 4);

  SoloJourney journey(trx, &itin, 0);
  journey.endJourney(3);

  std::vector<ClassOfService> reqCosVec;
  std::vector<std::vector<ClassOfServiceList> > reqAvl;
  reqCosVec.resize(itin.travelSeg().size() + journey.getRange().getSize());
  reqAvl.resize(reqCosVec.size());

  size_t i = 0;
  for (; i < itin.travelSeg().size(); ++i)
  {
    reqCosVec[i].bookingCode() = 'E' + i;
    reqAvl[i].resize(1);
    reqAvl[i][0].push_back(&reqCosVec[i]);
    insertAvl(trx, itin, &(reqAvl[i]), SegmentRange(i, i + 1));
  }

  reqAvl[i].resize(journey.getRange().getSize());
  for (size_t j = 0; j < journey.getRange().getSize(); ++j)
  {
    reqCosVec[i + j].bookingCode() = 'E' + i + j;
    reqAvl[i][j].push_back(&reqCosVec[i + j]);
  }
  insertAvl(trx, itin, &(reqAvl[i]), journey.getRange());

  ClassOfServiceList* avl = journey.getFlowAvl(0, sopUsage);
  CPPUNIT_ASSERT(avl);
  CPPUNIT_ASSERT_EQUAL(size_t(1), avl->size());
  CPPUNIT_ASSERT_EQUAL(reqCosVec[i].bookingCode(), (*avl)[0]->bookingCode());

  avl = journey.getFlowAvl(1, sopUsage);
  CPPUNIT_ASSERT(avl);
  CPPUNIT_ASSERT_EQUAL(size_t(1), avl->size());
  CPPUNIT_ASSERT_EQUAL(reqCosVec[i + 1].bookingCode(), (*avl)[0]->bookingCode());

  avl = journey.getFlowAvl(2, sopUsage);
  CPPUNIT_ASSERT(avl);
  CPPUNIT_ASSERT_EQUAL(size_t(1), avl->size());
  CPPUNIT_ASSERT_EQUAL(reqCosVec[i + 2].bookingCode(), (*avl)[0]->bookingCode());
}

void
SoloJourneyTest::testGetFlowAvl()
{
  PricingTrx trx;
  std::vector<AirSeg> segments;
  std::vector<ClassOfService> cosVec;
  Itin itin;
  SOPUsage sopUsage;
  sopUsage.itin_ = &itin;

  buildTravelSeg(itin, segments, cosVec, 4);

  SoloJourney journey(trx, &itin, 0);
  journey.endJourney(3);
  SegmentRange fmRange(0, 2);

  std::vector<ClassOfService> reqCosVec;
  std::vector<std::vector<ClassOfServiceList> > reqAvl;
  reqCosVec.resize(itin.travelSeg().size() + fmRange.getSize());
  reqAvl.resize(reqCosVec.size());

  size_t i = 0;
  for (; i < itin.travelSeg().size(); ++i)
  {
    reqCosVec[i].bookingCode() = 'E' + i;
    reqAvl[i].resize(1);
    reqAvl[i][0].push_back(&reqCosVec[i]);
    insertAvl(trx, itin, &(reqAvl[i]), SegmentRange(i, i + 1));
  }

  reqAvl[i].resize(fmRange.getSize());
  for (size_t j = 0; j < fmRange.getSize(); ++j)
  {
    reqCosVec[i + j].bookingCode() = 'E' + i + j;
    reqAvl[i][j].push_back(&reqCosVec[i + j]);
  }
  insertAvl(trx, itin, &(reqAvl[i]), fmRange);

  ClassOfServiceList* avl = journey.getFlowAvl(fmRange, 0, sopUsage);
  CPPUNIT_ASSERT(avl);
  CPPUNIT_ASSERT_EQUAL(size_t(1), avl->size());
  CPPUNIT_ASSERT_EQUAL(reqCosVec[i].bookingCode(), (*avl)[0]->bookingCode());

  avl = journey.getFlowAvl(fmRange, 1, sopUsage);
  CPPUNIT_ASSERT(avl);
  CPPUNIT_ASSERT_EQUAL(size_t(1), avl->size());
  CPPUNIT_ASSERT_EQUAL(reqCosVec[i + 1].bookingCode(), (*avl)[0]->bookingCode());
}

void
SoloJourneyTest::testIsFlowAvailFromAvs_False()
{
  PricingTrx trx;
  Itin itin;
  std::vector<AirSeg> segments;
  std::vector<ClassOfService> segmentsCOS;
  buildTravelSeg(itin, segments, segmentsCOS, 2u);

  std::vector<ClassOfServiceList> local1;
  std::vector<ClassOfServiceList> local2;
  std::vector<ClassOfServiceList> flow;
  local1.push_back(buildCOSList(15, 1));
  local2.push_back(buildCOSList(11, 1));
  flow.push_back(buildCOSList(11, 1));
  flow.push_back(buildCOSList(9, 1));

  insertAvl(trx, itin, &local1, SegmentRange(0, 1));
  insertAvl(trx, itin, &local2, SegmentRange(1, 2));
  insertAvl(trx, itin, &flow, SegmentRange(0, 2));

  SoloJourney sj(trx, &itin, 0);
  sj.endJourney(2);
  sj._avsLocalToFlowCosSizeRatio = 1.2f;
  CPPUNIT_ASSERT(!sj.isFlowAvailFromAvs(SegmentRange(0, 2), flow));
}

void
SoloJourneyTest::testIsFlowAvailFromAvs_True()
{
  PricingTrx trx;
  Itin itin;
  std::vector<AirSeg> segments;
  std::vector<ClassOfService> segmentsCOS;
  buildTravelSeg(itin, segments, segmentsCOS, 2u);

  std::vector<ClassOfServiceList> local1;
  std::vector<ClassOfServiceList> local2;
  std::vector<ClassOfServiceList> flow;
  local1.push_back(buildCOSList(15, 1));
  local2.push_back(buildCOSList(13, 1));
  flow.push_back(buildCOSList(11, 1));
  flow.push_back(buildCOSList(9, 1));

  insertAvl(trx, itin, &local1, SegmentRange(0, 1));
  insertAvl(trx, itin, &local2, SegmentRange(1, 2));
  insertAvl(trx, itin, &flow, SegmentRange(0, 2));

  SoloJourney sj(trx, &itin, 0);
  sj.endJourney(2);
  sj._avsLocalToFlowCosSizeRatio = 1.2f;
  CPPUNIT_ASSERT(sj.isFlowAvailFromAvs(SegmentRange(0, 2), flow));
}

} // namespace tse
