#include <log4cxx/helpers/objectptr.h>
#include "test/include/CppUnitHelperMacros.h"
#include "Common/AirlineShoppingUtils.h"
#include "Common/Config/ConfigMan.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/DataHandle.h"
#include "test/include/TestMemHandle.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FarePath.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/PricingOptions.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseEnums.h"
#include "Common/TseConsts.h"
#include "DataModel/PaxType.h"
#include "DataModel/Itin.h"
#include "Rules/RuleConst.h"
#include "test/include/TestConfigInitializer.h"

using namespace std;
using namespace boost;

using namespace tse::AirlineShoppingUtils;

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}
namespace tse
{
namespace AirlineShoppingUtils
{

log4cxx::LoggerPtr
AseItinCombiner::_logger(
    log4cxx::Logger::getLogger("atseintl.AirlineShoppingUtils.AseItinCombiner"));
}

class AirlineShoppingUtilsTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(AirlineShoppingUtilsTest);
  CPPUNIT_TEST(combineItinsTest_sameBrandSamePax);
  CPPUNIT_TEST(combineItinsTest_differentBrandSamePax);
  CPPUNIT_TEST(combineItinsTest_differentBrandDifferentPax);
  CPPUNIT_TEST(combineOneWayItinerariesTest_noItinPresent);
  CPPUNIT_TEST(combineOneWayItinerariesTest_cheapestSolutionMatched);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memH.create<TestConfigInitializer>();
    _trx = _memH.create<PricingTrx>();
    _trx->setRequest(_memH(new PricingRequest));
    _trx->getRequest()->validatingCarrier() = "WN";
  }
  void tearDown()
  {
    _trx->itin().clear();
    _memH.clear();
  }

  FareUsage* createFareUsage(PaxTypeFare* ptf)
  {
    FareUsage* fu = _memH(new FareUsage);
    fu->paxTypeFare() = ptf;
    return fu;
  }

  PricingUnit* createPricingUnit() { return _memH(new PricingUnit); }

  PricingUnit* createPricingUnit(PaxTypeFare* ptf)
  {
    PricingUnit* pu = createPricingUnit();
    pu->fareUsage().push_back(createFareUsage(ptf));

    return pu;
  }

  FareMarket* createFareMarket()
  {
    FareMarket* fm = _memH(new FareMarket);
    fm->paxTypeCortege().resize(1);
    return fm;
  }

  PaxTypeFare& createPaxTypeFare(uint16_t brandIndex = 0)
  {
    FareInfo* fi = _memH(new FareInfo);
    fi->carrier() = "CX";
    fi->currency() = NUC;

    TariffCrossRefInfo* tcr = _memH(new TariffCrossRefInfo);
    tcr->ruleTariff() = 0;

    Fare* f = _memH(new Fare);
    f->setTariffCrossRefInfo(tcr);

    f->setInvBrand(brandIndex, false);

    f->setFareInfo(fi);

    PaxTypeFare* ptf = _memH(new PaxTypeFare);
    ptf->setFare(f);
    ptf->fareMarket() = createFareMarket();
    FareClassAppInfo* fca = _memH(new FareClassAppInfo);
    fca->_displayCatType = RuleConst::NET_SUBMIT_FARE;
    ptf->fareClassAppInfo() = fca;
    ptf->paxType() = createPaxType(ADULT);
    ptf->nucFareAmount() = 100;
    return *ptf;
  }

  Itin* createItin(uint16_t legId = 0, LocCode o = "DFW", LocCode d = "LAX")
  {
    Itin* i = _memH(new Itin);
    i->geoTravelType() = GeoTravelType::Transborder;
    i->calculationCurrency() = NUC;
    i->legID().push_back(std::pair<int, int>(legId, legId));
    i->travelSeg().push_back(createTravelSeg(o, d));
    i->datePair() = _memH(new DatePair);
    return i;
  }

  PaxType* createPaxType(PaxTypeCode code)
  {
    PaxType* pt = _memH(new PaxType);
    pt->paxType() = code;
    // pt->paxTypeInfo() = _memH(new PaxTypeInfo);
    return pt;
  }
  FarePath* createFarePath(Itin* itin = 0,
                           uint16_t brandIndex = 0,
                           PaxTypeCode code = ADULT)
  {
    FarePath* fp = _memH(new FarePath);
    fp->itin() = (itin == 0) ? createItin() : itin;
    fp->paxType() = createPaxType(code);
    fp->brandIndex() = brandIndex;
    fp->pricingUnit().push_back(createPricingUnit(&createPaxTypeFare(brandIndex)));
    return fp;
  }

  Loc* getLoc(std::string code)
  {
    Loc* loc = _memH.insert(new Loc);
    LocCode locCode = code;
    loc->loc() = (locCode);
    return loc;
  }

  AirSeg* createTravelSeg(LocCode o, LocCode d)
  {
    AirSeg* tvlseg = _memH.insert(new AirSeg);

    tvlseg->origin() = getLoc(o);
    tvlseg->destination() = getLoc(d);
    tvlseg->origAirport() = o;
    tvlseg->destAirport() = d;
    tvlseg->boardMultiCity() = o;
    tvlseg->offMultiCity() = d;
    tvlseg->departureDT() = DateTime::localTime();
    tvlseg->segmentOrder() = 0;
    return tvlseg;
  }

  void combineOneWayItinerariesTest_noItinPresent()
  {
    try
    {
      AirlineShoppingUtils::AseItinCombiner aseCombiner(*_trx);
      aseCombiner.combineOneWayItineraries();
      CPPUNIT_ASSERT(false);
    }
    catch (ErrorResponseException& e)
    {
      CPPUNIT_ASSERT_EQUAL(ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS, e.code());
    }
  }

  void combineItinsTest_sameBrandSamePax()
  {
    uint16_t FIRST_LEG = 0, SECOND_LEG = 1;
    uint16_t brandIndex = 0;

    Itin* outbound = createItin(FIRST_LEG);
    outbound->farePath().push_back(createFarePath(outbound, brandIndex));

    Itin* inbound = createItin(SECOND_LEG);
    inbound->farePath().push_back(createFarePath(inbound, brandIndex));

    CPPUNIT_ASSERT_EQUAL(0, (int)_trx->itin().size());

    AirlineShoppingUtils::AseItinCombiner aseCombiner(*_trx);
    aseCombiner.combineItins(*_trx, outbound, inbound);

    size_t expectedFP = 1;
    size_t expectedSeg = 2;
    Itin* resultItin = _trx->itin().back();
    CPPUNIT_ASSERT_EQUAL(expectedFP, resultItin->farePath().size());
    CPPUNIT_ASSERT_EQUAL(expectedSeg, resultItin->travelSeg().size());
  }

  void combineItinsTest_differentBrandSamePax()
  {
    uint16_t FIRST_LEG = 0, SECOND_LEG = 1;
    uint16_t brandIndex1 = 0, brandIndex2 = 1;

    Itin* outbound = createItin(FIRST_LEG);
    outbound->farePath().push_back(createFarePath(outbound, brandIndex1));

    Itin* inbound = createItin(SECOND_LEG);
    inbound->farePath().push_back(createFarePath(inbound, brandIndex2));

    CPPUNIT_ASSERT_EQUAL(0, (int)_trx->itin().size());

    AirlineShoppingUtils::AseItinCombiner aseCombiner(*_trx);
    aseCombiner.combineItins(*_trx, outbound, inbound);

    size_t expectedFP = 2;
    size_t expectedSeg = 2;
    Itin* resultItin = _trx->itin().back();
    CPPUNIT_ASSERT_EQUAL(expectedFP, resultItin->farePath().size());
    CPPUNIT_ASSERT_EQUAL(expectedSeg, resultItin->travelSeg().size());
  }

  void combineItinsTest_differentBrandDifferentPax()
  {
    uint16_t FIRST_LEG = 0, SECOND_LEG = 1;
    uint16_t brandIndex1 = 0, brandIndex2 = 1;
    PaxTypeCode ADULT = "ADT", CHILD = "CNN";

    Itin* outbound = createItin(FIRST_LEG);
    outbound->farePath().push_back(createFarePath(outbound, brandIndex1, ADULT));

    Itin* inbound = createItin(SECOND_LEG);
    inbound->farePath().push_back(createFarePath(inbound, brandIndex2, CHILD));

    CPPUNIT_ASSERT_EQUAL(0, (int)_trx->itin().size());

    AirlineShoppingUtils::AseItinCombiner aseCombiner(*_trx);
    aseCombiner.combineItins(*_trx, outbound, inbound);

    // size_t expectedFP =0;
    // size_t expectedSeg =2;
    CPPUNIT_ASSERT_EQUAL(0, (int)_trx->itin().size());
  }
  //------------------------------------------------

  void combineOneWayItinerariesTest_cheapestSolutionMatched()
  {
    uint16_t FIRST_LEG = 0, SECOND_LEG = 1;
    uint16_t brandIndex1 = 0;
    PaxTypeCode ADULT = "ADT";
    _trx->setAltDates(true);

    Itin* outbound = createItin(FIRST_LEG);
    _trx->itin().push_back(outbound);
    outbound->farePath().push_back(createFarePath(outbound, brandIndex1, ADULT));

    Itin* inbound = createItin(SECOND_LEG);
    _trx->itin().push_back(inbound);
    inbound->farePath().push_back(createFarePath(inbound, brandIndex1, ADULT));

    AirlineShoppingUtils::AseItinCombiner aseCombiner(*_trx);
    aseCombiner.combineOneWayItineraries();

    CPPUNIT_ASSERT_EQUAL(1, (int)_trx->itin().size());
    CPPUNIT_ASSERT_EQUAL(1, (int)_trx->itin().back()->farePath().size());
  }

private:
  PricingTrx* _trx;
  DataHandle _dataHandle;

  static log4cxx::LoggerPtr _logger;
  TestMemHandle _memH;
};

CPPUNIT_TEST_SUITE_REGISTRATION(AirlineShoppingUtilsTest);

//  log4cxx::LoggerPtr
//  AirlineShoppingUtilsTest::_logger(log4cxx::Logger::getLogger("atseintl.test.AirlineShoppingUtilsTest"));

} // namespace tse
