#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>

#include <iostream>
#include "DataModel/FlightFinderTrx.h"
#include "Fares/FareCollectorOrchestrator.h"
#include "Diagnostic/DiagCollector.h"
#include "test/include/TestConfigInitializer.h"
#include "Common/Config/ConfigMan.h"
#include "DataModel/PricingOptions.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

using namespace std;

/* Main test class */
class FFFailedFaresTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(FFFailedFaresTest);
  CPPUNIT_TEST(noFareSelectedAllDataDifferent);
  CPPUNIT_TEST(oneFareRequestedNoSelected);
  CPPUNIT_TEST(twoFareRequestedTwoFareSelected);
  CPPUNIT_TEST(twoFareRequestedTwoSelectedRestReleased);
  CPPUNIT_TEST_SUITE_END();

private:
  Itin _itin;
  DiagCollector _diagColl;
  FareMarket* _fareMarket;
  FlightFinderTrx* _ffTrx;
  PricingOptions* _opt;
  std::map<PaxTypeCode, bool> _psgPrevalidationStatus;

  TestMemHandle _memH;

public:
  void setUp()
  {
    _memH.create<TestConfigInitializer>();

    _fareMarket = _memH.insert(new FareMarket);

    _ffTrx = _memH.insert(new FlightFinderTrx);

    _opt = _memH.insert(new PricingOptions);

    _ffTrx->setOptions(_opt);
  }

  void tearDown()
  {
    _memH.clear();
  }

protected:
  //----------------------------------------------------*
  /* Util functions */
  Fare* createFareTest(std::string basisCode, double amount, std::string currency)
  {
    Fare* fare = _memH.insert(new Fare);
    ;

    FareInfo* _fareInfo = _memH.insert(new FareInfo);

    /*Significat data for tests */
    _fareInfo->currency() = currency;
    _fareInfo->fareClass() = basisCode;
    _fareInfo->fareAmount() = amount;
    _fareInfo->originalFareAmount() = amount;

    /*Setup Fares info in fare*/
    fare->setFareInfo(_fareInfo);
    return fare;
  }

  //----------------------------------------------------*

  PaxTypeFare* createPaxTypeFareTest(std::string basisCode, double amount, std::string currency)
  {
    Fare* fare = createFareTest(basisCode, amount, currency);

    PaxTypeFare* ptFare = _memH.insert(new PaxTypeFare);

    ptFare->initialize(fare, NULL, NULL, *_ffTrx);
    ptFare->setIsShoppingFare();

    return ptFare;
  }

  //----------------------------------------------------

  FlightFinderTrx::FareBasisCodeInfo
  createFareBasisInfoTest(const std::string& basisCode, double fareAmt, const std::string& currency)
  {
    FlightFinderTrx::FareBasisCodeInfo fareInfo;
    fareInfo.fareBasiscode = basisCode;
    fareInfo.fareAmount = fareAmt;
    fareInfo.currencyCode = currency;
    return fareInfo;
  }
  //----------------------------------------------------
  void noFareSelectedAllDataDifferent()
  {

    // Setup Fare Info in transaction
    _ffTrx->fareBasisCodesFF().push_back(createFareBasisInfoTest("YYY", 124.00, "USD"));

    std::vector<PaxTypeFare*> paxTypeFares;
    paxTypeFares.push_back(createPaxTypeFareTest("YY", 122., "PLN"));
    _fareMarket->allPaxTypeFare() = paxTypeFares;

    // One bucket is for one psg type
    PaxTypeBucket bucket;
    bucket.paxTypeFare() = paxTypeFares;
    _fareMarket->paxTypeCortege().push_back(bucket);

    CPPUNIT_ASSERT(_fareMarket->paxTypeCortege().back().paxTypeFare().size() == 1);
    CPPUNIT_ASSERT(_fareMarket->allPaxTypeFare().size() == 1);

    FareCollectorOrchestrator::releaseFFinderInvalidFares(
        *_ffTrx, _itin, *_fareMarket, _diagColl, _psgPrevalidationStatus);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cortege should be empty",
                                 0,
                                 (int)_fareMarket->paxTypeCortege().back().paxTypeFare().size());

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "AllPaxTypeFare should be empty", 0, (int)_fareMarket->allPaxTypeFare().size());
  }

  //----------------------------------------------------*
  //----------------------------------------------------*
  void oneFareRequestedNoSelected()
  {

    /* Setup Fare Info in transaction */
    _ffTrx->fareBasisCodesFF().push_back(createFareBasisInfoTest("YYY", 124.00, "USD"));

    std::vector<PaxTypeFare*> paxTypeFares;

    paxTypeFares.push_back(createPaxTypeFareTest("YYY", 122., "USD"));

    _fareMarket->allPaxTypeFare() = paxTypeFares;

    /* One bucket is for one psg type*/
    PaxTypeBucket bucket;
    bucket.paxTypeFare() = paxTypeFares;
    _fareMarket->paxTypeCortege().push_back(bucket);

    CPPUNIT_ASSERT(_fareMarket->paxTypeCortege().back().paxTypeFare().size() == 1);
    CPPUNIT_ASSERT(_fareMarket->allPaxTypeFare().size() == 1);

    FareCollectorOrchestrator::releaseFFinderInvalidFares(
        *_ffTrx, _itin, *_fareMarket, _diagColl, _psgPrevalidationStatus);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cortege should be empty",
                                 0,
                                 (int)_fareMarket->paxTypeCortege().back().paxTypeFare().size());

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "AllPaxTypeFare collection should be empty", 0, (int)_fareMarket->allPaxTypeFare().size());
  }
  //----------------------------------------------------*

  void twoFareRequestedTwoFareSelected()
  {

    /* Setup Fare Info in transaction */
    _ffTrx->fareBasisCodesFF().push_back(createFareBasisInfoTest("YYY", 124.00, "USD"));

    _ffTrx->fareBasisCodesFF().push_back(createFareBasisInfoTest("XXX", 222.00, "PLN"));

    std::vector<PaxTypeFare*> paxTypeFares;
    paxTypeFares.push_back(createPaxTypeFareTest("YYY", 124., "USD"));
    paxTypeFares.push_back(createPaxTypeFareTest("XXX", 222., "PLN"));

    _fareMarket->allPaxTypeFare() = paxTypeFares;

    /* One bucket is for one psg type*/
    PaxTypeBucket bucket;
    bucket.paxTypeFare() = paxTypeFares;
    _fareMarket->paxTypeCortege().push_back(bucket);

    CPPUNIT_ASSERT(_fareMarket->paxTypeCortege().back().paxTypeFare().size() == 2);
    CPPUNIT_ASSERT(_fareMarket->allPaxTypeFare().size() == 2);

    FareCollectorOrchestrator::releaseFFinderInvalidFares(
        *_ffTrx, _itin, *_fareMarket, _diagColl, _psgPrevalidationStatus);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Wrong Cortege size", 2, (int)_fareMarket->paxTypeCortege().back().paxTypeFare().size());

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Wrong AllPaxTypeFareSize", 2, (int)_fareMarket->allPaxTypeFare().size());
  }

  //----------------------------------------------------*

  void twoFareRequestedTwoSelectedRestReleased()
  {

    /* Setup Fare Info in transaction */
    _ffTrx->fareBasisCodesFF().push_back(createFareBasisInfoTest("YYY", 124.00, "USD"));

    _ffTrx->fareBasisCodesFF().push_back(createFareBasisInfoTest("XXX", 222.00, "PLN"));

    std::vector<PaxTypeFare*> paxTypeFares;
    paxTypeFares.push_back(createPaxTypeFareTest("YYY", 124., "USD"));
    paxTypeFares.push_back(createPaxTypeFareTest("XXX", 222., "PLN"));
    /*Fares that should be released */
    paxTypeFares.push_back(createPaxTypeFareTest("ZYYY", 14., "PLN"));
    paxTypeFares.push_back(createPaxTypeFareTest("WYYY", 88., "USD"));

    _fareMarket->allPaxTypeFare() = paxTypeFares;

    /* One bucket is for one psg type*/
    PaxTypeBucket bucket;
    bucket.paxTypeFare() = paxTypeFares;
    _fareMarket->paxTypeCortege().push_back(bucket);

    CPPUNIT_ASSERT(_fareMarket->paxTypeCortege().back().paxTypeFare().size() == 4);
    CPPUNIT_ASSERT(_fareMarket->allPaxTypeFare().size() == 4);

    FareCollectorOrchestrator::releaseFFinderInvalidFares(
        *_ffTrx, _itin, *_fareMarket, _diagColl, _psgPrevalidationStatus);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Wrong Cortege size", 2, (int)_fareMarket->paxTypeCortege().back().paxTypeFare().size());

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Wrong AllPaxTypeFareSize", 2, (int)_fareMarket->allPaxTypeFare().size());
  }
  //----------------------------------------------------*
};

CPPUNIT_TEST_SUITE_REGISTRATION(FFFailedFaresTest);
}
