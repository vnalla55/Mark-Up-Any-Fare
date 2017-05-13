#include "test/include/CppUnitHelperMacros.h"
#include "FareCalc/FcUtil.h"
#include "test/include/TestMemHandle.h"

using namespace std;

namespace tse
{
class FcUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FcUtilTest);
  CPPUNIT_TEST(testCollect);
  CPPUNIT_TEST(testForEachFareUsageCountDiscAmount);
  CPPUNIT_TEST(testForEachFareUsageCountSegments);
  CPPUNIT_TEST(testForEachFareUsageCountSegmentsWhenPnrSegmentNumerIs2);
  CPPUNIT_TEST_SUITE_END();

public:
  static const int FP_NO = 5;
  static const int PU_NO = 5;
  static const int FU_NO = 5;
  static const int TS_NO = 10;

  void setUp()
  {
    _itin = _memHandle.create<Itin>();
    for (int i = 0; i < FP_NO; i++)
    {
      FarePath* fp = _memHandle.create<FarePath>();
      _itin->farePath().push_back(fp);
      for (int j = 0; j < PU_NO; j++)
      {
        PricingUnit* pu = _memHandle.create<PricingUnit>();
        fp->pricingUnit().push_back(pu);
        for (int k = 0; k < FU_NO; k++)
        {
          FareUsage* fu = _memHandle.create<FareUsage>();
          fu->setDiscAmount(1.0);
          pu->fareUsage().push_back(fu);
          _fuList.push_back(fu);
          for (int m = 0; m < TS_NO; m++)
          {
            AirSeg* ts = _memHandle.create<AirSeg>();
            ts->pnrSegment() = 1;
            fu->travelSeg().push_back(ts);
          }
        }
      }
    }
  }

  void tearDown() { _memHandle.clear(); }

protected:
  void testCollect()
  {
    // collect (all)
    vector<FareUsage*> FuVect;
    FareCalc::forEachFareUsage(*_itin, FareCalc::collect(FuVect));
    CPPUNIT_ASSERT_EQUAL(FuVect.size(), _fuList.size());

    // find (first) fu where discAmount == 1.0
    vector<FareUsage*>::iterator iter;
    iter = find_if(
        FuVect.begin(),
        FuVect.end(),
        FareCalc::Equal<FareUsage>(mem_fun<const MoneyAmount>(&FareUsage::getDiscAmount), 1.0));
    CPPUNIT_ASSERT(iter == FuVect.begin());

    //
    FarePath* fp = _itin->farePath()[1];
    FuVect.clear();
    FareCalc::forEachFareUsage(*fp, FareCalc::collect(FuVect));
    CPPUNIT_ASSERT_EQUAL(size_t(25), FuVect.size());

    FuVect[3]->setDiscAmount(10.0);
    iter = find_if(FuVect.begin(),
                   FuVect.end(),
                   FareCalc::Equal<FareUsage>(mem_fun<const MoneyAmount>(&FareUsage::getDiscAmount),
                                              FuVect[3]->getDiscAmount()));
    CPPUNIT_ASSERT_EQUAL(3, (int)distance(FuVect.begin(), iter));
    FuVect[3]->setDiscAmount(1.0);

    //
    vector<const FareUsage*> holder;
    FareCalc::forEachFareUsage(*_itin, FareCalc::collect(holder));

    //
    FareUsage* fu = const_cast<FareUsage*>(holder[10]);
    fu->setDiscAmount(10.0);
    holder.clear();
    FareCalc::forEachFareUsage(
        *_itin,
        FareCalc::collect(holder,
                          FareCalc::Equal<FareUsage>(
                              mem_fun<const MoneyAmount>(&FareUsage::getDiscAmount), 10.0)));
    CPPUNIT_ASSERT_EQUAL(size_t(1), holder.size());
    CPPUNIT_ASSERT(holder.front() == fu);
    CPPUNIT_ASSERT_EQUAL(10.0, holder.front()->getDiscAmount());
    fu->setDiscAmount(1.0);
  }

  struct SetPnrSegmentTo2
  {
    void operator()(TravelSeg* ts) { ts->pnrSegment() = 2; }
  };

  void testForEachFareUsageCountDiscAmount()
  {
    FareCalc::forEachFareUsage(
        *_itin,
        FareCalc::Equal<FareUsage>(mem_fun<const MoneyAmount>(&FareUsage::getDiscAmount), 1.0));

    MoneyAmount tot_discAmount = 0.0;
    FareCalc::forEachFareUsage(
        *_itin,
        FareCalc::Accumulate<FareUsage>(mem_fun<const MoneyAmount>(&FareUsage::getDiscAmount),
                                        tot_discAmount));
    CPPUNIT_ASSERT_EQUAL(tot_discAmount, static_cast<MoneyAmount>(_fuList.size()) * 1.0);
    CPPUNIT_ASSERT_EQUAL(125.0, tot_discAmount);
  }

  void testForEachFareUsageCountSegments()
  {
    int tot_segment = 0;
    FareCalc::forEachTravelSeg(*_itin,
                               FareCalc::Accumulate<TravelSeg>(
                                   mem_fun<const int16_t&>(&TravelSeg::pnrSegment), tot_segment));
    CPPUNIT_ASSERT_EQUAL((FP_NO * PU_NO * FU_NO * TS_NO), tot_segment);
  }

  void testForEachFareUsageCountSegmentsWhenPnrSegmentNumerIs2()
  {
    int tot_segment = 0;
    FareCalc::forEachTravelSeg(*_itin, SetPnrSegmentTo2());
    tot_segment = 0;
    FareCalc::forEachTravelSeg(*_itin,
                               FareCalc::Accumulate<TravelSeg>(
                                   mem_fun<const int16_t&>(&TravelSeg::pnrSegment), tot_segment));
    CPPUNIT_ASSERT_EQUAL((FP_NO * PU_NO * FU_NO * TS_NO * 2), tot_segment);
  }

protected:
  Itin* _itin;
  vector<FareUsage*> _fuList;
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(FcUtilTest);
} // namespace tse
