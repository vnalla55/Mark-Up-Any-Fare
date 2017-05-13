#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/CsoPricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/AirSeg.h"
#include "Common/ClassOfService.h"
#include "DataModel/ConsolidatorPlusUp.h"
#include "DBAccess/Loc.h"
#include "test/include/TestMemHandle.h"
#include <boost/assign/std/vector.hpp>

namespace tse
{

using namespace boost::assign;

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T> vec)
{
  std::copy(vec.begin(), vec.end(), std::ostream_iterator<T>(os, ", "));
  return os;
}

class MockTravelSeg : public TravelSeg
{
public:
  MockTravelSeg(const TravelSegType& type, bool unflown)
  {
    _segmentType = type;
    _unflown = unflown;
  }
  MockTravelSeg* clone(DataHandle& dh) const { return 0; }
};

class CsoPricingTrxTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CsoPricingTrxTest);
  CPPUNIT_TEST(testCloneClassOfServicePass);
  CPPUNIT_TEST(testCloneClassOfServiceFail);
  CPPUNIT_TEST(testCloneTravelSegmentsPass);
  CPPUNIT_TEST(testCloneTravelSegmentsAllFlown);
  CPPUNIT_TEST(testCloneTravelSegmentsWithArunkFlown);
  CPPUNIT_TEST(testCloneTravelSegmentsWithArunkUnflown);
  CPPUNIT_TEST(testClonePlusUpPricingFail);
  CPPUNIT_TEST(testClonePlusUpPricingPass);
  CPPUNIT_TEST_SUITE_END();

private:
  CsoPricingTrx* _trx;
  Itin* _itin;
  RexPricingTrx* _rexTrx;
  TestMemHandle _memHandle;

  enum
  {
    Flown = false,
    Unflown = true
  };

public:
  CsoPricingTrxTest() { log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getOff()); }

  void setUp()
  {
    _trx = _memHandle.create<CsoPricingTrx>();
    _itin = _memHandle.create<Itin>();
    _trx->itin().push_back(_itin);
    _trx->setRequest(helperSetupRequest());

    _rexTrx = _memHandle.create<RexPricingTrx>();
    _rexTrx->newItin().push_back(_memHandle.create<Itin>());
    _rexTrx->setRequest(helperSetupRequest());

    _trx->_parentTrx = _rexTrx;
  }

  PricingRequest* helperSetupRequest()
  {
    PricingRequest* rqst = _memHandle.create<PricingRequest>();
    rqst->ticketingAgent() = _memHandle.create<Agent>();
    rqst->ticketingAgent()->agentLocation() = _memHandle.create<Loc>();
    return rqst;
  }

  void tearDown() { _memHandle.clear(); }

  template <typename T>
  struct get_ptr
  {
    T* operator()(T& obj) const { return &obj; }
  };

  void testCloneClassOfServicePass()
  {
    AirSeg seg[10];
    FareMarket fm[5];
    ClassOfService cos[3];
    ClassOfService* cos_ptr[] = { cos, cos + 1, cos + 2 };
    std::vector<ClassOfService*> cosv1(cos_ptr, cos_ptr + 2), cosv2(cos_ptr + 1, cos_ptr + 3),
        cosv3(cos_ptr, cos_ptr + 1);

    // rex
    std::transform(seg, seg + 3, std::back_inserter(fm[0].travelSeg()), get_ptr<AirSeg>());
    std::transform(seg + 3, seg + 7, std::back_inserter(fm[1].travelSeg()), get_ptr<AirSeg>());
    std::transform(seg + 7, seg + 9, std::back_inserter(fm[2].travelSeg()), get_ptr<AirSeg>());

    fm[0].classOfServiceVec() += &cosv1, &cosv2;
    fm[1].classOfServiceVec() += &cosv3, &cosv1;
    fm[2].classOfServiceVec() += &cosv2, &cosv3;

    std::vector<FareMarket*>& rexFm = _rexTrx->newItin().front()->fareMarket();
    std::transform(fm, fm + 3, std::back_inserter(rexFm), get_ptr<FareMarket>());

    // cso
    std::transform(seg + 3, seg + 7, std::back_inserter(fm[3].travelSeg()), get_ptr<AirSeg>());
    std::transform(seg + 7, seg + 9, std::back_inserter(fm[4].travelSeg()), get_ptr<AirSeg>());

    std::vector<FareMarket*>& csoFm = _trx->itin().front()->fareMarket();
    std::transform(fm + 3, fm + 5, std::back_inserter(csoFm), get_ptr<FareMarket>());

    CPPUNIT_ASSERT_NO_THROW(_trx->cloneClassOfService());

    CPPUNIT_ASSERT_EQUAL(rexFm[1]->classOfServiceVec(), csoFm[0]->classOfServiceVec());
    CPPUNIT_ASSERT_EQUAL(rexFm[2]->classOfServiceVec(), csoFm[1]->classOfServiceVec());
  }

  void testCloneClassOfServiceFail()
  {
    AirSeg seg[2];
    FareMarket fm[2];

    // rex
    fm[0].travelSeg().push_back(seg);
    _rexTrx->newItin().front()->fareMarket().push_back(fm);

    // cso
    fm[1].travelSeg().push_back(seg + 1);
    _trx->itin().front()->fareMarket().push_back(fm + 1);

    CPPUNIT_ASSERT_THROW(_trx->cloneClassOfService(), ErrorResponseException);
  }

  void testCloneTravelSegmentsPass()
  {
    MockTravelSeg seg[3] = { MockTravelSeg(Air, Flown), MockTravelSeg(Air, Unflown),
                             MockTravelSeg(Air, Unflown) };

    std::vector<TravelSeg*> trvSeg;
    std::transform(seg, seg + 3, std::back_inserter(trvSeg), get_ptr<MockTravelSeg>());

    CPPUNIT_ASSERT_NO_THROW(_trx->cloneTravelSegments(trvSeg));

    std::vector<TravelSeg*> expTrvSeg;
    std::transform(seg + 1, seg + 3, std::back_inserter(expTrvSeg), get_ptr<MockTravelSeg>());

    CPPUNIT_ASSERT_EQUAL(expTrvSeg, _itin->travelSeg());
  }

  void testCloneTravelSegmentsAllFlown()
  {
    MockTravelSeg seg[3] = { MockTravelSeg(Air, Flown), MockTravelSeg(Air, Flown),
                             MockTravelSeg(Air, Flown) };

    std::vector<TravelSeg*> trvSeg;
    std::transform(seg, seg + 3, std::back_inserter(trvSeg), get_ptr<MockTravelSeg>());

    CPPUNIT_ASSERT_THROW(_trx->cloneTravelSegments(trvSeg), ErrorResponseException);
  }

  void testCloneTravelSegmentsWithArunkFlown()
  {
    MockTravelSeg seg[4] = { MockTravelSeg(Air, Flown), MockTravelSeg(Arunk, Flown),
                             MockTravelSeg(Air, Unflown), MockTravelSeg(Air, Unflown) };

    std::vector<TravelSeg*> trvSeg;
    std::transform(seg, seg + 4, std::back_inserter(trvSeg), get_ptr<MockTravelSeg>());

    CPPUNIT_ASSERT_NO_THROW(_trx->cloneTravelSegments(trvSeg));

    std::vector<TravelSeg*> expTrvSeg;
    std::transform(seg + 2, seg + 4, std::back_inserter(expTrvSeg), get_ptr<MockTravelSeg>());

    CPPUNIT_ASSERT_EQUAL(expTrvSeg, _itin->travelSeg());
  }

  void testCloneTravelSegmentsWithArunkUnflown()
  {
    MockTravelSeg seg[4] = { MockTravelSeg(Air, Flown), MockTravelSeg(Arunk, Unflown),
                             MockTravelSeg(Air, Unflown), MockTravelSeg(Air, Unflown) };

    std::vector<TravelSeg*> trvSeg;
    std::transform(seg, seg + 4, std::back_inserter(trvSeg), get_ptr<MockTravelSeg>());

    CPPUNIT_ASSERT_NO_THROW(_trx->cloneTravelSegments(trvSeg));

    std::vector<TravelSeg*> expTrvSeg;
    std::transform(seg + 2, seg + 4, std::back_inserter(expTrvSeg), get_ptr<MockTravelSeg>());

    CPPUNIT_ASSERT_EQUAL(expTrvSeg, _itin->travelSeg());
  }

  void testClonePlusUpPricingFail()
  {
    _trx->clonePlusUpPricing(*_rexTrx->itin().front());
    CPPUNIT_ASSERT(!_itin->isPlusUpPricing());
  }

  void testClonePlusUpPricingPass()
  {
    MoneyAmount amount = 111.22;
    CurrencyCode currency = "USD";
    TktDesignator designator = "tktDesignator";

    ConsolidatorPlusUp plusUp;
    plusUp.initialize(*_rexTrx, amount, currency, designator);

    _rexTrx->itin().front()->consolidatorPlusUp() = &plusUp;

    _trx->clonePlusUpPricing(*_rexTrx->itin().front());

    CPPUNIT_ASSERT(_itin->isPlusUpPricing());
    CPPUNIT_ASSERT_EQUAL(amount, _itin->consolidatorPlusUp()->amount());
    CPPUNIT_ASSERT_EQUAL(currency, _itin->consolidatorPlusUp()->currencyCode());
    CPPUNIT_ASSERT_EQUAL(designator, _itin->consolidatorPlusUp()->tktDesignator());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CsoPricingTrxTest);
}
