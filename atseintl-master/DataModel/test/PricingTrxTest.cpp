#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>

#include "DataModel/PricingTrx.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/Customer.h"
#include "Common/TrxUtil.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestFallbackUtil.h"

namespace tse
{

using namespace boost::assign;

class PricingTrxTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PricingTrxTest);
  CPPUNIT_TEST(testSetItinsTimeSpan_oneItin);
  CPPUNIT_TEST(testSetItinsTimeSpan_twoItins_noOverlap);
  CPPUNIT_TEST(testSetItinsTimeSpan_twoItins_secondOverlap);
  CPPUNIT_TEST(testSetItinsTimeSpan_twoItins_firstOverlap);
  CPPUNIT_TEST(testSetItinsTimeSpan_twoItins_secondInside);
  CPPUNIT_TEST(testNotCommandPricingRq);
  CPPUNIT_TEST(testCommandPricingRq);
  CPPUNIT_TEST(testIsProcess2CcPass);
  CPPUNIT_TEST(testIsProcess2CcFail1stCard);
  CPPUNIT_TEST(testIsProcess2CcFail2ndCard);
  CPPUNIT_TEST(testIsProcess2CcFailNoDigit);
  CPPUNIT_TEST(testIsProcess2CcFailZeroPayment);
  CPPUNIT_TEST(testIsProcess2CcFailNoCard);
  CPPUNIT_TEST(testIsObFeeAppliedPossitiveCase);
  CPPUNIT_TEST(testIsObFeeAppliedNegativeCase);
  CPPUNIT_TEST(testIsSingleMatchReturnFalseForPricingTrx);
  CPPUNIT_TEST(testIsSingleMatchReturnTrueForAltTrx);
  CPPUNIT_TEST(testIsSingleMatchReturnFalseWhenNoJv);
  CPPUNIT_TEST(testIsSingleMatchReturnFalseWhenNoMatch);
  CPPUNIT_TEST(testIsSingleMatchReturnTrueForTwoPaxTypes);
  CPPUNIT_TEST(testIsSingleMatchReturnFalseForTwoPaxTypesWhenFirstNoMatch);
  CPPUNIT_TEST(testIsSingleMatchReturnFalseForTwoPaxTypesWhenSecondNoMatch);
  CPPUNIT_TEST(testCheckFareMarketsCoverTravelSegsPositive);
  CPPUNIT_TEST(testCheckFareMarketsCoverTravelSegsNoFares);
  CPPUNIT_TEST(testCheckFareMarketsCoverTravelSegsFailOnRule);
  CPPUNIT_TEST(testCheckFareMarketsCoverTravelSegsFailOnRouting);
  CPPUNIT_TEST(testCheckFareMarketsCoverTravelSegsFailOnBookingCode);
  CPPUNIT_TEST(testDiscountAmountTwoGroups);
  CPPUNIT_TEST(testDiscountAmountTwoGroupsInvalidForPricingUnit);
  CPPUNIT_TEST(testDiscountPercentageTwoGroups);
  CPPUNIT_TEST(testDiscountPercentageTwoGroupsInvalidForPricingUnit);
  CPPUNIT_TEST(testisNormalWpPricingNormal);
  CPPUNIT_TEST(testisNormalWpPricingWPA);
  CPPUNIT_TEST(testisNormalWpPricingAllBrandsWp);
  CPPUNIT_TEST(testisNormalWpPricingAllBrandsWpa);

  CPPUNIT_TEST_SUITE_END();

private:
  PricingTrx* _trx;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _trx = _memHandle.create<PricingTrx>();
    _memHandle.create<TestConfigInitializer>();
  }

  void tearDown() { _memHandle.clear(); }

  void populateItin(const std::string& departure, const std::string& arrival)
  {
    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg() += _memHandle.create<AirSeg>(), _memHandle.create<AirSeg>();

    itin->travelSeg().front()->departureDT() = DateTime(departure, 0);
    itin->travelSeg().front()->arrivalDT() = DateTime(departure, 60);

    itin->travelSeg().back()->departureDT() = DateTime(arrival, 0);
    itin->travelSeg().back()->arrivalDT() = DateTime(arrival, 60);

    _trx->itin().push_back(itin);
  }

  void createSegments()
  {
    for (int nCnt = 0; nCnt < 3; ++nCnt)
      _trx->travelSeg().push_back(_memHandle.create<AirSeg>());
  }

  const DateTime* getEarliestItinDate(const Itin* itin)
  {
    return &itin->travelSeg().front()->departureDT();
  }

  const DateTime* getLatestItinDate(const Itin* itin)
  {
    return &itin->travelSeg().back()->arrivalDT();
  }

  const Itin* firstItin() { return _trx->itin().front(); }

  const Itin* lastItin() { return _trx->itin().back(); }

  void setFop(FopBinNumber fop1, FopBinNumber fop2, MoneyAmount amt, Indicator isCard)
  {
    PricingRequest* request = _memHandle.create<PricingRequest>();
    _trx->setRequest(request);
    request->formOfPayment() = fop1;
    request->secondFormOfPayment() = fop2;
    request->paymentAmountFop() = amt;
    request->formOfPaymentCard() = isCard;
  }

  void testSetItinsTimeSpan_oneItin()
  {
    populateItin("2012-03-01", "2012-03-02");

    _trx->setItinsTimeSpan();

    CPPUNIT_ASSERT_EQUAL(getEarliestItinDate(firstItin()), &_trx->getEarliestItinDate());
    CPPUNIT_ASSERT_EQUAL(getLatestItinDate(lastItin()), &_trx->getLatestItinDate());
  }

  void testSetItinsTimeSpan_twoItins_noOverlap()
  {
    populateItin("2012-03-01", "2012-03-03");
    populateItin("2012-03-04", "2012-03-06");

    _trx->setItinsTimeSpan();

    CPPUNIT_ASSERT_EQUAL(getEarliestItinDate(firstItin()), &_trx->getEarliestItinDate());
    CPPUNIT_ASSERT_EQUAL(getLatestItinDate(lastItin()), &_trx->getLatestItinDate());
  }

  void testSetItinsTimeSpan_twoItins_secondOverlap()
  {
    populateItin("2012-03-01", "2012-03-03");
    populateItin("2012-03-02", "2012-03-06");

    _trx->setItinsTimeSpan();

    CPPUNIT_ASSERT_EQUAL(getEarliestItinDate(firstItin()), &_trx->getEarliestItinDate());
    CPPUNIT_ASSERT_EQUAL(getLatestItinDate(lastItin()), &_trx->getLatestItinDate());
  }

  void testSetItinsTimeSpan_twoItins_firstOverlap()
  {
    populateItin("2012-03-04", "2012-03-08");
    populateItin("2012-03-02", "2012-03-06");

    _trx->setItinsTimeSpan();

    CPPUNIT_ASSERT_EQUAL(getEarliestItinDate(lastItin()), &_trx->getEarliestItinDate());
    CPPUNIT_ASSERT_EQUAL(getLatestItinDate(firstItin()), &_trx->getLatestItinDate());
  }

  void testSetItinsTimeSpan_twoItins_secondInside()
  {
    populateItin("2012-03-01", "2012-03-08");
    populateItin("2012-03-02", "2012-03-06");

    _trx->setItinsTimeSpan();

    CPPUNIT_ASSERT_EQUAL(getEarliestItinDate(firstItin()), &_trx->getEarliestItinDate());
    CPPUNIT_ASSERT_EQUAL(getLatestItinDate(firstItin()), &_trx->getLatestItinDate());
  }

  void testNotCommandPricingRq()
  {
    createSegments();

    CPPUNIT_ASSERT(!_trx->isCommandPricingRq());
  }

  void testCommandPricingRq()
  {
    createSegments();
    _trx->travelSeg().back()->fareBasisCode() = "BFFWEB";

    CPPUNIT_ASSERT(_trx->isCommandPricingRq());
  }

  void testIsProcess2CcPass()
  {
    setFop("123456", "222222", 100.5, 'T');

    CPPUNIT_ASSERT(_trx->isProcess2CC());
  }

  void testIsProcess2CcFail1stCard()
  {
    setFop("13456", "222222", 100.5, 'T');

    CPPUNIT_ASSERT(!_trx->isProcess2CC());
  }

  void testIsProcess2CcFail2ndCard()
  {
    setFop("123456", "2222", 100.5, 'T');

    CPPUNIT_ASSERT(!_trx->isProcess2CC());
  }

  void testIsProcess2CcFailNoDigit()
  {
    setFop("1X6456", "2222", 100.5, 'T');

    CPPUNIT_ASSERT(!_trx->isProcess2CC());
  }

  void testIsProcess2CcFailZeroPayment()
  {
    setFop("1X6456", "2222", 0.0, 'T');

    CPPUNIT_ASSERT(!_trx->isProcess2CC());
  }

  void testIsProcess2CcFailNoCard()
  {
    setFop("1X6456", "2222", 100.5, 'N');

    CPPUNIT_ASSERT(!_trx->isProcess2CC());
  }

  void testIsObFeeAppliedPossitiveCase()
  {
    PricingRequest req;
    req.collectOBFee() = 'T';
    req.formOfPaymentCard() = 'T';
    req.formOfPayment() = "123456";
    _trx->setRequest(&req);
    CPPUNIT_ASSERT(_trx->isObFeeApplied());
  }

  void testIsObFeeAppliedNegativeCase()
  {
    PricingRequest req;
    req.collectOBFee() = 'F';
    _trx->setRequest(&req);
    CPPUNIT_ASSERT(!_trx->isObFeeApplied());
  }

  void addFarePathToTrx(bool isProcessed = true)
  {
    FarePath* farePath = _memHandle.create<FarePath>();
    _trx->itin().front()->farePath() += farePath;
    farePath->processed() = isProcessed;
  }

  void addPaxToTrx()
  {
    PaxType* paxType;
    paxType = _memHandle.create<PaxType>();
    _trx->paxType() += paxType;
  }

  void setDataForSingleMatch(bool isProcessed = true, const std::string& hostName = ABACUS_USER)
  {
    _trx = _memHandle.create<AltPricingTrx>();
    PricingRequest* request = _memHandle.create<PricingRequest>();
    _trx->setRequest(request);
    Itin* itin = _memHandle.create<Itin>();
    _trx->itin() += itin;
    addFarePathToTrx(isProcessed);
    _trx->altTrxType() = PricingTrx::WPA;
    Agent* agent;
    agent = _memHandle.create<Agent>();
    Customer* agentTJR;
    agentTJR = _memHandle.create<Customer>();
    agentTJR->crsCarrier() = "1B";
    agentTJR->hostName() = hostName;
    agent->agentTJR() = agentTJR;
    TrxUtil::enableAbacus();
    request->ticketingAgent() = agent;
    addPaxToTrx();
  }

  void testIsSingleMatchReturnTrueForAltTrx()
  {
    setDataForSingleMatch();
    CPPUNIT_ASSERT(_trx->isSingleMatch());
  }
  void testIsSingleMatchReturnFalseForPricingTrx() { CPPUNIT_ASSERT(!_trx->isSingleMatch()); }

  void testIsSingleMatchReturnFalseWhenNoJv()
  {
    setDataForSingleMatch(true, "XXXX");
    CPPUNIT_ASSERT(!_trx->isSingleMatch());
  }

  void testIsSingleMatchReturnFalseWhenNoMatch()
  {
    setDataForSingleMatch(false);
    CPPUNIT_ASSERT(!_trx->isSingleMatch());
  }

  void testIsSingleMatchReturnTrueForTwoPaxTypes()
  {
    setDataForSingleMatch();
    addPaxToTrx();
    addFarePathToTrx();
    CPPUNIT_ASSERT(_trx->isSingleMatch());
  }

  void testIsSingleMatchReturnFalseForTwoPaxTypesWhenFirstNoMatch()
  {
    setDataForSingleMatch(false);
    addPaxToTrx();
    addFarePathToTrx();
    CPPUNIT_ASSERT(!_trx->isSingleMatch());
  }

  void testIsSingleMatchReturnFalseForTwoPaxTypesWhenSecondNoMatch()
  {
    setDataForSingleMatch();
    addPaxToTrx();
    addFarePathToTrx(false);
    CPPUNIT_ASSERT(!_trx->isSingleMatch());
  }

  void buildValidFare(Fare* fare)
  {
    fare->status().set(Fare::FS_ScopeIsDefined);
    fare->setMissingFootnote(false);
    fare->setFareInfo(_memHandle.create<FareInfo>());
    fare->setTariffCrossRefInfo(_memHandle.create<TariffCrossRefInfo>());
    fare->setCat15SecurityValid(true);
    fare->setGlobalDirectionValid(true);
    fare->setCalcCurrForDomItinValid(true);
    fare->setFareNotValidForDisplay(false);
  }

  void buildValidPaxTypeFare(PaxTypeFare* ptf)
  {
    ptf->actualPaxType() = _memHandle.create<PaxType>();
    ptf->fareClassAppInfo() = _memHandle.create<FareClassAppInfo>();
    ptf->fareClassAppSegInfo() = _memHandle.create<FareClassAppSegInfo>();
    ptf->setFare(_memHandle.create<Fare>());
    buildValidFare(ptf->fare());
    for (unsigned int ci = 1; ci < 36; ++ci)
    {
      ptf->setCategoryProcessed(ci);
      ptf->setCategoryValid(ci);
    }
    ptf->setIsMipUniqueFare(true);
    ptf->setRoutingProcessed(true);
    ptf->setRoutingValid(true);
    ptf->bookingCodeStatus().set(PaxTypeFare::BKS_NOT_YET_PROCESSED, false);
    ptf->bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    ptf->setRequestedFareBasisInvalid(false);
  }

  void buildTravelSegments(std::vector<TravelSeg*>& container, int orderMin, int orderMax)
  {
    for (int ti = orderMin; ti <= orderMax; ++ti)
    {
      TravelSeg* ts = _memHandle.create<AirSeg>();
      ts->segmentOrder() = ti;
      container.push_back(ts);
    }
  }

  void buildEssentials()
  {
    Itin* itin = _memHandle.create<Itin>();
    buildTravelSegments(_trx->travelSeg(), 1, 6);
    _trx->itin().push_back(itin);

    for (int fi = 1; fi <= 3; ++fi)
    {
      FareMarket* fm = _memHandle.create<FareMarket>();

      buildTravelSegments(fm->travelSeg(), fi, fi + 3);

      for (int pi = 0; pi < 10; ++pi)
      {
        PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
        buildValidPaxTypeFare(ptf);
        ptf->fareMarket() = fm;
        fm->allPaxTypeFare().push_back(ptf);
      }
      _trx->fareMarket().push_back(fm);
    }
  }

  void testCheckFareMarketsCoverTravelSegsPositive()
  {
    buildEssentials();
    CPPUNIT_ASSERT_NO_THROW(_trx->checkFareMarketsCoverTravelSegs());
  }

  void testCheckFareMarketsCoverTravelSegsNoFares()
  {
    buildEssentials();
    _trx->fareMarket().back()->allPaxTypeFare().clear();
    CPPUNIT_ASSERT_THROW(_trx->checkFareMarketsCoverTravelSegs(), ErrorResponseException);
  }

  void testCheckFareMarketsCoverTravelSegsFailOnRule()
  {
    buildEssentials();
    for (PaxTypeFare* ptf : _trx->fareMarket().back()->allPaxTypeFare())
    {
      ptf->setCategoryValid(3, false);
    }
    CPPUNIT_ASSERT_THROW(_trx->checkFareMarketsCoverTravelSegs(), ErrorResponseException);
  }

  void testCheckFareMarketsCoverTravelSegsFailOnRouting()
  {
    buildEssentials();
    for (PaxTypeFare* ptf : _trx->fareMarket().back()->allPaxTypeFare())
    {
      ptf->setRoutingValid(false);
    }
    CPPUNIT_ASSERT_THROW(_trx->checkFareMarketsCoverTravelSegs(), ErrorResponseException);
  }

  void testCheckFareMarketsCoverTravelSegsFailOnBookingCode()
  {
    buildEssentials();
    for (PaxTypeFare* ptf : _trx->fareMarket().back()->allPaxTypeFare())
    {
      ptf->bookingCodeStatus().setNull();
      ptf->bookingCodeStatus().set(PaxTypeFare::BKS_FAIL);
    }
    CPPUNIT_ASSERT_THROW(_trx->checkFareMarketsCoverTravelSegs(), ErrorResponseException);
  }

  void testDiscountAmountTwoGroups()
  {
    AirSeg airSeg1;
    airSeg1.segmentOrder() = 1;
    AirSeg airSeg2;
    airSeg2.segmentOrder() = 2;

    FareMarket fareMarket1;
    fareMarket1.travelSeg().push_back(&airSeg1);
    FareMarket fareMarket2;
    fareMarket2.travelSeg().push_back(&airSeg2);

    PaxTypeFare paxTypeFare1;
    paxTypeFare1.fareMarket() = &fareMarket1;
    PaxTypeFare paxTypeFare2;
    paxTypeFare2.fareMarket() = &fareMarket2;

    FareUsage fareUsage1;
    fareUsage1.paxTypeFare() = &paxTypeFare1;
    FareUsage fareUsage2;
    fareUsage2.paxTypeFare() = &paxTypeFare2;

    PricingUnit pricingUnit;
    pricingUnit.fareUsage().push_back(&fareUsage1);
    pricingUnit.fareUsage().push_back(&fareUsage2);

    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->getRequest()->addDiscountAmountNew(1, 1, 12.34, "USD");
    _trx->getRequest()->addDiscountAmountNew(1, 2, 12.34, "USD");

    CPPUNIT_ASSERT_DOUBLES_EQUAL(12.34, _trx->getDiscountAmountNew(pricingUnit)->amount, EPSILON);
    CPPUNIT_ASSERT(TrxUtil::validateDiscountNew(*_trx, pricingUnit));

    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->getRequest()->addDiscountAmountNew(1, 1, -12.34, "USD");
    _trx->getRequest()->addDiscountAmountNew(1, 2, -12.34, "USD");

    CPPUNIT_ASSERT_DOUBLES_EQUAL(-12.34, _trx->getDiscountAmountNew(pricingUnit)->amount, EPSILON);
    CPPUNIT_ASSERT(TrxUtil::validateDiscountNew(*_trx, pricingUnit));
  }

  void testDiscountAmountTwoGroupsInvalidForPricingUnit()
  {
    AirSeg airSeg1;
    airSeg1.segmentOrder() = 1;
    AirSeg airSeg2;
    airSeg2.segmentOrder() = 2;

    FareMarket fareMarket1;
    fareMarket1.travelSeg().push_back(&airSeg1);
    FareMarket fareMarket2;
    fareMarket2.travelSeg().push_back(&airSeg2);

    PaxTypeFare paxTypeFare1;
    paxTypeFare1.fareMarket() = &fareMarket1;
    PaxTypeFare paxTypeFare2;
    paxTypeFare2.fareMarket() = &fareMarket2;

    FareUsage fareUsage1;
    fareUsage1.paxTypeFare() = &paxTypeFare1;
    FareUsage fareUsage2;
    fareUsage2.paxTypeFare() = &paxTypeFare2;

    PricingUnit pricingUnit;
    pricingUnit.fareUsage().push_back(&fareUsage1);
    pricingUnit.fareUsage().push_back(&fareUsage2);

    _trx->setRequest(&*_memHandle.create<PricingRequest>());
    _trx->getRequest()->addDiscountAmountNew(1, 1, 12.34, "USD");
    _trx->getRequest()->addDiscountAmountNew(2, 2, 23.45, "USD");

    CPPUNIT_ASSERT(_trx->getDiscountAmountNew(pricingUnit) == nullptr);
    CPPUNIT_ASSERT(TrxUtil::validateDiscountNew(*_trx, pricingUnit) == false);

    _trx->setRequest(&*_memHandle.create<PricingRequest>());
    _trx->getRequest()->addDiscountAmountNew(1, 1, -12.34, "USD");
    _trx->getRequest()->addDiscountAmountNew(2, 2, -23.45, "USD");

    CPPUNIT_ASSERT(_trx->getDiscountAmountNew(pricingUnit) == nullptr);
    CPPUNIT_ASSERT(TrxUtil::validateDiscountNew(*_trx, pricingUnit) == false);
  }

  void testDiscountPercentageTwoGroups()
  {
    AirSeg airSeg1;
    airSeg1.segmentOrder() = 1;
    AirSeg airSeg2;
    airSeg2.segmentOrder() = 2;

    FareMarket fareMarket1;
    fareMarket1.travelSeg().push_back(&airSeg1);
    FareMarket fareMarket2;
    fareMarket2.travelSeg().push_back(&airSeg2);

    PaxTypeFare paxTypeFare1;
    paxTypeFare1.fareMarket() = &fareMarket1;
    PaxTypeFare paxTypeFare2;
    paxTypeFare2.fareMarket() = &fareMarket2;

    FareUsage fareUsage1;
    fareUsage1.paxTypeFare() = &paxTypeFare1;
    FareUsage fareUsage2;
    fareUsage2.paxTypeFare() = &paxTypeFare2;

    PricingUnit pricingUnit;
    pricingUnit.fareUsage().push_back(&fareUsage1);
    pricingUnit.fareUsage().push_back(&fareUsage2);

    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->getRequest()->addDiscountPercentage(1, 30);
    _trx->getRequest()->addDiscountPercentage(2, 30);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(30, *_trx->getDiscountPercentageNew(pricingUnit), EPSILON);
    CPPUNIT_ASSERT(TrxUtil::validateDiscountNew(*_trx, pricingUnit));

    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->getRequest()->addDiscountPercentage(1, -30);
    _trx->getRequest()->addDiscountPercentage(2, -30);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(-30, *_trx->getDiscountPercentageNew(pricingUnit), EPSILON);
    CPPUNIT_ASSERT(TrxUtil::validateDiscountNew(*_trx, pricingUnit));
  }

  void testDiscountPercentageTwoGroupsInvalidForPricingUnit()
  {
    AirSeg airSeg1;
    airSeg1.segmentOrder() = 1;
    AirSeg airSeg2;
    airSeg2.segmentOrder() = 2;

    FareMarket fareMarket1;
    fareMarket1.travelSeg().push_back(&airSeg1);
    FareMarket fareMarket2;
    fareMarket2.travelSeg().push_back(&airSeg2);

    PaxTypeFare paxTypeFare1;
    paxTypeFare1.fareMarket() = &fareMarket1;
    PaxTypeFare paxTypeFare2;
    paxTypeFare2.fareMarket() = &fareMarket2;

    FareUsage fareUsage1;
    fareUsage1.paxTypeFare() = &paxTypeFare1;
    FareUsage fareUsage2;
    fareUsage2.paxTypeFare() = &paxTypeFare2;

    PricingUnit pricingUnit;
    pricingUnit.fareUsage().push_back(&fareUsage1);
    pricingUnit.fareUsage().push_back(&fareUsage2);

    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->getRequest()->addDiscountPercentage(1, 30);
    _trx->getRequest()->addDiscountPercentage(2, 50);

    CPPUNIT_ASSERT(_trx->getDiscountPercentageNew(pricingUnit) == nullptr);
    CPPUNIT_ASSERT(TrxUtil::validateDiscountNew(*_trx, pricingUnit) == false);

    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->getRequest()->addDiscountPercentage(1, -30);
    _trx->getRequest()->addDiscountPercentage(2, -50);

    CPPUNIT_ASSERT(_trx->getDiscountPercentageNew(pricingUnit) == nullptr);
    CPPUNIT_ASSERT(TrxUtil::validateDiscountNew(*_trx, pricingUnit) == false);
  }

  void testisNormalWpPricingNormal()
  {
    _trx->modifiableActivationFlags().setSearchForBrandsPricing(false);
    _trx->altTrxType() = PricingTrx::WP;
    CPPUNIT_ASSERT(_trx->isNormalWpPricing() == true);
  }

  void testisNormalWpPricingWPA()
  {
    _trx->modifiableActivationFlags().setSearchForBrandsPricing(false);
    _trx->altTrxType() = PricingTrx::WPA;
    CPPUNIT_ASSERT(_trx->isNormalWpPricing() == false);
  }

  void testisNormalWpPricingAllBrandsWp()
  {
    _trx->modifiableActivationFlags().setSearchForBrandsPricing(true);
    _trx->altTrxType() = PricingTrx::WP;
    CPPUNIT_ASSERT(_trx->isNormalWpPricing() == false);
  }

  void testisNormalWpPricingAllBrandsWpa()
  {
    _trx->modifiableActivationFlags().setSearchForBrandsPricing(true);
    _trx->altTrxType() = PricingTrx::WPA;
    CPPUNIT_ASSERT(_trx->isNormalWpPricing() == false);
  }




};

CPPUNIT_TEST_SUITE_REGISTRATION(PricingTrxTest);
}
