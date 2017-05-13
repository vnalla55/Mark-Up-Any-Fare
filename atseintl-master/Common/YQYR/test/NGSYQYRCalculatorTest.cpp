#include "test/include/CppUnitHelperMacros.h"

#include "Common/YQYR/NGSYQYRCalculator.h"
#include "Common/YQYR/NGSYQYRCalculator.t.h"
#include "DataModel/Agent.h"
#include "DataModel/ShoppingTrx.h"
#include "DBAccess/TaxCarrierAppl.h"
#include "DBAccess/TaxCarrierFlightInfo.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/MockGlobal.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class NGSYQYRCalculatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(NGSYQYRCalculatorTest);

  CPPUNIT_TEST(testDetermineJourneyOrigin);
  CPPUNIT_TEST(testDetermineFurthestPointWithStopovers);
  CPPUNIT_TEST(testDetermineFurthestPointWithoutStopovers);

  CPPUNIT_TEST(testOriginalBucketCreation);

  CPPUNIT_TEST(testReadRecords);
  CPPUNIT_TEST(testDetermineCarriersToProcess);
  CPPUNIT_TEST_SUITE_END();

public:
  AirSeg* createSegment(const LocCode &from,
                        const LocCode &to,
                        const CarrierCode &carrier,
                        const DateTime &departureDT)
  {
    const Loc *locFrom = _dataHandle->getLoc(from);
    const Loc *locTo = _dataHandle->getLoc(to);

    AirSeg *airSeg = _memHandle.create<AirSeg>();
    airSeg->origin() = locFrom;
    airSeg->destination() = locTo;
    airSeg->departureDT() = departureDT;
    airSeg->arrivalDT() = departureDT + boost::posix_time::hours(1);
    airSeg->setMarketingCarrierCode(carrier);

    return airSeg;
  }

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _dataHandle = _memHandle.create<MyDataHandle>();
    _trx = _memHandle.create<ShoppingTrx>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _request = _memHandle.create<PricingRequest>();
    _request->ticketingAgent() = _memHandle.create<Agent>();
    _request->ticketingAgent()->currencyCodeAgent() = "GBP";
    Loc* agentLoc = _memHandle.create<Loc>();
    agentLoc->nation() = "GB";
    _request->ticketingAgent()->agentLocation() = agentLoc;
    _request->specifiedTktDesignator()[1] = "AAA";
    _request->ticketingDT() = DateTime(2015, 1, 12);
    _trx->setRequest(_request);

    _trx->legs().resize(2);
    ShoppingTrx::Leg &leg0(_trx->legs().front());
    leg0.directionalIndicator() = FMDirection::OUTBOUND;

    _trx->journeyItin() = _memHandle.create<Itin>();
    _trx->journeyItin()->travelSeg().push_back(
        createSegment("KRK", "LON", "LO", DateTime(2015, 1, 1)));
    _trx->journeyItin()->travelSeg().push_back(
        createSegment("LON", "KRK", "LO", DateTime(2015, 2, 1)));

    Itin *itin = 0;

    itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(createSegment("KRK", "WAW", "LO", DateTime(2015, 1, 1)));
    itin->travelSeg().push_back(createSegment("WAW", "LON", "LO", DateTime(2015, 1, 3)));

    leg0.sop().push_back(ShoppingTrx::SchedulingOption(itin, 0));

    itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(createSegment("KRK", "FRA", "LH", DateTime(2015, 1, 1)));
    itin->travelSeg().push_back(createSegment("FRA", "LON", "LH", DateTime(2015, 1, 3)));

    leg0.sop().push_back(ShoppingTrx::SchedulingOption(itin, 1));

    itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(createSegment("KRK", "DUB", "BA", DateTime(2015, 1, 1)));
    itin->travelSeg().push_back(createSegment("DUB", "LON", "BA", DateTime(2015, 1, 3)));

    leg0.sop().push_back(ShoppingTrx::SchedulingOption(itin, 2));

    itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(createSegment("KRK", "CDG", "BA", DateTime(2015, 1, 1)));
    itin->travelSeg().push_back(createSegment("CDG", "LON", "BA", DateTime(2015, 1, 1)));

    leg0.sop().push_back(ShoppingTrx::SchedulingOption(itin, 3));

    ShoppingTrx::Leg &leg1(_trx->legs().back());
    leg1.directionalIndicator() = FMDirection::INBOUND;

    itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(createSegment("LON", "CDG", "LO", DateTime(2015, 2, 1)));
    itin->travelSeg().push_back(createSegment("CDG", "KRK", "LO", DateTime(2015, 2, 3)));

    leg1.sop().push_back(ShoppingTrx::SchedulingOption(itin, 0));

    itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(createSegment("LON", "FRA", "LH", DateTime(2015, 2, 1)));
    itin->travelSeg().push_back(createSegment("FRA", "KRK", "LH", DateTime(2015, 2, 3)));

    leg1.sop().push_back(ShoppingTrx::SchedulingOption(itin, 1));

    itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(createSegment("LON", "MAD", "BA", DateTime(2015, 2, 1)));
    itin->travelSeg().push_back(createSegment("MAD", "FRA", "BA", DateTime(2015, 2, 3)));
    itin->travelSeg().push_back(createSegment("FRA", "KRK", "BA", DateTime(2015, 2, 5)));

    leg1.sop().push_back(ShoppingTrx::SchedulingOption(itin, 2));

    itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(createSegment("LON", "DUB", "BA", DateTime(2015, 2, 1)));
    itin->travelSeg().push_back(createSegment("DUB", "KRK", "BA", DateTime(2015, 2, 1)));

    leg1.sop().push_back(ShoppingTrx::SchedulingOption(itin, 3));

    _calculator = _memHandle.create<YQYR::NGSYQYRCalculator>(*_trx);
    _calculator->init();
  }

  void tearDown() { _memHandle.clear(); }

  void testDetermineJourneyOrigin()
  {
    const Itin *itin(_trx->legs().front().sop()[0].itin());
    const std::vector<TravelSeg*> *travelSegs(&itin->travelSeg());

    const AirSeg *origin = _calculator->determineJourneyOrigin(*travelSegs);
    const TravelSeg *originalSeg(travelSegs->front());

    CPPUNIT_ASSERT_EQUAL(originalSeg, static_cast<const TravelSeg*>(origin));
  }

  void testDetermineFurthestPointWithStopovers()
  {
    const AirSeg *journeyOrigin(createSegment("KRK", "WAW", "LO", DateTime(2015, 1, 1)));

    const Itin *itin(_trx->legs().front().sop()[0].itin());
    const std::vector<TravelSeg*> *travelSegs(&itin->travelSeg());
    const Loc *furthestPoint = 0;

    furthestPoint = _calculator->determineFurthestPoint(travelSegs->rbegin(), travelSegs->rend(),
                                                        journeyOrigin, itin, true);
    CPPUNIT_ASSERT_EQUAL(LocCode("LON"), furthestPoint->loc());

    itin = _trx->legs().front().sop()[2].itin();
    travelSegs = &itin->travelSeg();
    furthestPoint = _calculator->determineFurthestPoint(travelSegs->rbegin(), travelSegs->rend(),
                                                        journeyOrigin, itin, true);
    CPPUNIT_ASSERT_EQUAL(LocCode("DUB"), furthestPoint->loc());

    itin = _trx->legs().back().sop()[0].itin();
    travelSegs = &itin->travelSeg();
    furthestPoint = _calculator->determineFurthestPoint(travelSegs->begin(), travelSegs->end(),
                                                        journeyOrigin, itin, false);
    CPPUNIT_ASSERT_EQUAL(LocCode("LON"), furthestPoint->loc());

    itin = _trx->legs().back().sop()[2].itin();
    travelSegs = &itin->travelSeg();
    furthestPoint = _calculator->determineFurthestPoint(travelSegs->begin(), travelSegs->end(),
                                                        journeyOrigin, itin, false);
    CPPUNIT_ASSERT_EQUAL(LocCode("MAD"), furthestPoint->loc());
  }

  void testDetermineFurthestPointWithoutStopovers()
  {
    const AirSeg *journeyOrigin(createSegment("KRK", "WAW", "LO", DateTime(2015, 1, 1)));

    const Itin *itin(_trx->legs().front().sop()[3].itin());
    const std::vector<TravelSeg*> *travelSegs(&itin->travelSeg());
    const Loc *furthestPoint = 0;

    furthestPoint = _calculator->determineFurthestPoint(travelSegs->rbegin(), travelSegs->rend(),
                                                        journeyOrigin, itin, true);
    CPPUNIT_ASSERT_EQUAL(LocCode("LON"), furthestPoint->loc());

    itin = _trx->legs().back().sop()[3].itin();
    travelSegs = &itin->travelSeg();
    furthestPoint = _calculator->determineFurthestPoint(travelSegs->begin(), travelSegs->end(),
                                                        journeyOrigin, itin, false);
    CPPUNIT_ASSERT_EQUAL(LocCode("LON"), furthestPoint->loc());
  }

  int16_t findBucketIndex(const LocCode &furthestLoc,
                          const std::vector<YQYR::YQYRBucket> &buckets) const
  {
    for (uint16_t i = 0; i < buckets.size(); ++i)
    {
      if (buckets[i].getFurthestPoint()->loc() == furthestLoc)
        return i;
    }

    return -1;
  }

  void testOriginalBucketCreation()
  {
    const std::vector<YQYR::YQYRBucket> &originalBuckets(_calculator->_originalBuckets);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), originalBuckets.size());

    const int16_t bucketLonInt = findBucketIndex(LocCode("LON"), originalBuckets);
    CPPUNIT_ASSERT(bucketLonInt != -1);

    const uint16_t bucketLon = static_cast<uint16_t>(bucketLonInt);
    CPPUNIT_ASSERT_EQUAL(bucketLon, _calculator->_sopToBucketIndex[std::make_pair(0, 0)]);
    CPPUNIT_ASSERT_EQUAL(bucketLon, _calculator->_sopToBucketIndex[std::make_pair(1, 0)]);
    CPPUNIT_ASSERT_EQUAL(bucketLon, _calculator->_sopToBucketIndex[std::make_pair(3, 0)]);
    CPPUNIT_ASSERT_EQUAL(bucketLon, _calculator->_sopToBucketIndex[std::make_pair(0, 1)]);
    CPPUNIT_ASSERT_EQUAL(bucketLon, _calculator->_sopToBucketIndex[std::make_pair(1, 1)]);
    CPPUNIT_ASSERT_EQUAL(bucketLon, _calculator->_sopToBucketIndex[std::make_pair(3, 1)]);

    const int16_t bucketDubInt = findBucketIndex(LocCode("DUB"), originalBuckets);
    CPPUNIT_ASSERT(bucketDubInt != -1);

    const uint16_t bucketDub = static_cast<uint16_t>(bucketDubInt);
    CPPUNIT_ASSERT_EQUAL(bucketDub, _calculator->_sopToBucketIndex[std::make_pair(2, 0)]);

    const int16_t bucketMadInt = findBucketIndex(LocCode("MAD"), originalBuckets);
    CPPUNIT_ASSERT(bucketMadInt != -1);

    const uint16_t bucketMad = static_cast<uint16_t>(bucketMadInt);
    CPPUNIT_ASSERT_EQUAL(bucketMad, _calculator->_sopToBucketIndex[std::make_pair(2, 1)]);
  }

  void testReadRecords()
  {
    const CarrierCode carrierAA("AA");
    _calculator->readRecords(carrierAA);
    YQYR::CarrierStorage& carrierStorageAA = _calculator->_feesPerCarrier[carrierAA];

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), carrierStorageAA._feesPerCode.size());
  }

  void testDetermineCarriersToProcess()
  {
    StdVectorFlatSet<CarrierCode> result;
    const Itin *itin(_trx->legs().front().sop()[0].itin());

    result = _calculator->determineCarriersToProcess(itin, "AA");
    CPPUNIT_ASSERT_EQUAL(true, result.empty());

    result = _calculator->determineCarriersToProcess(itin, "LO");
    CPPUNIT_ASSERT_EQUAL(static_cast<const size_t>(1), result.size());
    CPPUNIT_ASSERT(result.find("LO") != result.end());
  }

private:
  class MyDataHandle : public DataHandleMock
  {
    TestMemHandle _memHandle;

    typedef std::map<LocCode, Loc*> LocationsMap;
    LocationsMap _locations;

  public:
    const Loc* getLoc(const LocCode& locCode)
    {
      LocationsMap::const_iterator it(_locations.find(locCode));
      if (it != _locations.end())
        return it->second;

      Loc *loc = _memHandle.create<Loc>();
      loc->loc() = locCode;

      _locations.insert(std::make_pair(locCode, loc));

      return loc;
    }

    const std::vector<YQYRFees*>& getYQYRFees(const CarrierCode& carrier)
    {
      std::vector<YQYRFees*>* vec = _memHandle.create<std::vector<YQYRFees*>>();
      if (carrier == "AA") // no restrictions
      {
        for (size_t i=0; i < 10; ++i)
        {
          YQYRFees* fee = _memHandle.create<YQYRFees>();
          fee->carrier() = "AA";
          fee->taxCode() = "YQ";
          fee->subCode() = 'F';
          fee->firstTktDate() = DateTime(2015, 1, 1);
          fee->lastTktDate() = DateTime(2015, 12, 31);

          if (i > 5)
            fee->subCode() = 'I';

          vec->push_back(fee);
        }
      }

      return *vec;
    }

    const Mileage* getMileage(const LocCode& origin,
                              const LocCode& destination,
                              Indicator mileageType,
                              const GlobalDirection globalDirection,
                              const DateTime& dateTime)
    {
      if (origin != "KRK") // in all test scenarios, origin is allways KRK
        return 0;

      if (destination == "WAW")
      {
        Mileage* m = _memHandle.create<Mileage>();
        m->mileage() = 100;
        return m;
      }

      if (destination == "LON")
      {
        Mileage* m = _memHandle.create<Mileage>();
        m->mileage() = 1000;
        return m;
      }

      if (destination == "FRA")
      {
        Mileage* m = _memHandle.create<Mileage>();
        m->mileage() = 500;
        return m;
      }

      if (destination == "DUB")
      {
        Mileage* m = _memHandle.create<Mileage>();
        m->mileage() = 2000;
        return m;
      }

      if (destination == "CDG")
      {
        Mileage* m = _memHandle.create<Mileage>();
        m->mileage() = 800;
        return m;
      }

      if (destination == "MAD")
      {
        Mileage* m = _memHandle.create<Mileage>();
        m->mileage() = 3000;
        return m;
      }

      return 0;
    }
  };

private:
  TestMemHandle _memHandle;
  MyDataHandle *_dataHandle;
  ShoppingTrx* _trx;
  PricingRequest* _request;
  YQYR::NGSYQYRCalculator* _calculator;
};

CPPUNIT_TEST_SUITE_REGISTRATION(NGSYQYRCalculatorTest);
}
