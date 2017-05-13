#include <algorithm>
#include "test/include/CppUnitHelperMacros.h"
#include <gmock/gmock.h>

#include "Common/Config/ConfigMan.h"
#include "Common/DateTime.h"
#include "Common/LocUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/Vendor.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/test/MockPricingTrx.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareClassAppSegInfo.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/MileageSurchCxr.h"
#include "DBAccess/MileageSurchExcept.h"
#include "Routing/MileageInfo.h"
#include "Routing/MileageSurchargeException.h"
#include "Routing/RoutingConsts.h"
#include "Routing/test/MockMileageSurchargeException.h"
#include "Routing/TravelRoute.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestFareMarketFactory.h"
#include "test/testdata/TestPricingTrxFactory.h"

#include <memory>
#include <vector>

namespace tse
{

class MileageSurchargeExceptionTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MileageSurchargeExceptionTest);
  CPPUNIT_TEST(testcase1);
  CPPUNIT_TEST(testCxrRestrictionsMustPass1);
  CPPUNIT_TEST(testCxrRestrictionsMustPass2);
  CPPUNIT_TEST(testCxrRestrictionsMustFail1);
  CPPUNIT_TEST(testCxrRestrictionsMustFail2);
  CPPUNIT_TEST(testCxrRestrictionsExceptPass);
  CPPUNIT_TEST(testCxrRestrictionsExceptFail1);
  CPPUNIT_TEST(testCxrRestrictionsExceptFail2);
  CPPUNIT_TEST(testcase2WithoutStopOver);
  CPPUNIT_TEST(testUpdate);
  CPPUNIT_TEST(testApplyNoSurcharge);
  CPPUNIT_TEST(testApplyWithinMPM);
  CPPUNIT_TEST(testApplyNoUpdate);
  CPPUNIT_TEST(testApplyUpdate);
  CPPUNIT_TEST(testIsValidPaxTypeEmpty);
  CPPUNIT_TEST(testIsValidPaxTypeFound);
  CPPUNIT_TEST(testIsValidPaxTypeNotFound);
  CPPUNIT_TEST(testIsValidFareTypeEmpty);
  CPPUNIT_TEST(testIsValidFareTypeEqual);
  CPPUNIT_TEST(testIsValidFareTypeNotEqual);
  CPPUNIT_TEST(testValidateNoPositiveSurcharge);
  CPPUNIT_TEST(testValidateNoValidMarkets);
  CPPUNIT_TEST(testValidateNoValidMarketsPctgAbove25);
  CPPUNIT_TEST(testValidateStopOverRestrictionEmpty);
  CPPUNIT_TEST(testValidateFareRestriction);
  CPPUNIT_TEST(testIsExceptionApplicableNotValidFareRestriction);
  CPPUNIT_TEST(testValidateGeoRestrictionEmpty);
  CPPUNIT_TEST(testValidateSingleFareMileageSurchargePctgFail);
  CPPUNIT_TEST(testValidateSingleFareInvValidMileageSurchargePctgCase1);
  CPPUNIT_TEST(testValidateSingleFareInvValidMileageSurchargePctgCase2);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _fare.reset(new Fare);
    _fareInfo.reset(new FareInfo);
    _paxTypeFare.reset(new PaxTypeFare);
    _tvlRoute.reset(new TravelRoute);
    _trx.reset(new PricingTrx);
    _mockMileageSurchargeException.reset(new MockMileageSurchargeException);
    _dummyMileageSurchargePctg = 0;
  }

  void tearDown() { _memHandle.clear(); }
  void testcase1()
  {
    TravelRoute::CityCarrier city1, city2;

    city1.boardCity().loc() = "LON";
    city1.offCity().loc() = "MAN";
    city2.boardCity().loc() = "MAN";
    city2.offCity().loc() = "FRA";

    city2.stopover() = false;
    city1.stopover() = true;

    _tvlRoute->travelRoute().push_back(city1);
    _tvlRoute->travelRoute().push_back(city2);

    LocKey key;
    key.loc() = "GB";
    key.locType() = NATION;

    MileageSurchargeException validator;

    CPPUNIT_ASSERT(!validator.validateStopOverRestriction(key, *_tvlRoute));
  }
  void testcase2WithoutStopOver()
  {
    TravelRoute::CityCarrier city1, city2, city3;
    city1.boardCity().loc() = "LON";
    city1.offCity().loc() = "MAN";
    city2.boardCity().loc() = "FRA";
    city2.offCity().loc() = "MAN";
    city2.boardCity().loc() = "LON";
    city2.offCity().loc() = "CHI";
    city2.stopover() = false;
    city1.stopover() = false;
    city3.stopover() = false;

    _tvlRoute->travelRoute().push_back(city1);
    _tvlRoute->travelRoute().push_back(city2);
    _tvlRoute->travelRoute().push_back(city3);

    LocKey key, empty_key;
    key.loc() = "GB";
    key.locType() = NATION;

    MileageSurchargeException validator;

    CPPUNIT_ASSERT(validator.validateStopOverRestriction(key, *_tvlRoute));
    CPPUNIT_ASSERT(validator.validateStopOverRestriction(empty_key, *_tvlRoute));
  }

  void testCxrRestrictionsMustPass1()
  {
    TravelRoute::CityCarrier city1;
    city1.carrier() = "AA";

    _tvlRoute->travelRoute().push_back(city1);

    MileageSurchExcept* sur = _memHandle.create<MileageSurchExcept>();
    MileageSurchCxr* cxr1 = new MileageSurchCxr();
    MileageSurchCxr* cxr2 = new MileageSurchCxr();
    MileageSurchCxr* cxr3 = new MileageSurchCxr();

    cxr1->carrier() = "DL";
    cxr2->carrier() = "AA";
    cxr3->carrier() = "US";
    sur->cxrs().push_back(cxr1);
    sur->cxrs().push_back(cxr2);
    sur->cxrs().push_back(cxr3);
    sur->mustViaCxrExcept() = ' ';

    MileageSurchargeException validator;
    CPPUNIT_ASSERT(validator.validateCxrRestriction(*sur, *_tvlRoute));
  }

  void testCxrRestrictionsMustPass2()
  {
    TravelRoute::CityCarrier city1, city2;
    city1.carrier() = "AA";
    city2.carrier() = "DL";

    _tvlRoute->travelRoute().push_back(city1);
    _tvlRoute->travelRoute().push_back(city2);

    MileageSurchExcept* sur = _memHandle.create<MileageSurchExcept>();
    MileageSurchCxr* cxr1 = new MileageSurchCxr();
    MileageSurchCxr* cxr2 = new MileageSurchCxr();
    MileageSurchCxr* cxr3 = new MileageSurchCxr();

    cxr1->carrier() = "DL";
    cxr2->carrier() = "AA";
    cxr3->carrier() = "US";
    sur->cxrs().push_back(cxr1);
    sur->cxrs().push_back(cxr2);
    sur->cxrs().push_back(cxr3);
    sur->mustViaCxrExcept() = ' ';

    MileageSurchargeException validator;
    CPPUNIT_ASSERT(validator.validateCxrRestriction(*sur, *_tvlRoute));
  }

  void testCxrRestrictionsMustFail1()
  {
    TravelRoute::CityCarrier city1, city2, city3;
    city1.carrier() = "AA";
    city2.carrier() = "DL";
    city3.carrier() = "BA";

    _tvlRoute->travelRoute().push_back(city1);
    _tvlRoute->travelRoute().push_back(city2);
    _tvlRoute->travelRoute().push_back(city3);

    MileageSurchExcept* sur = _memHandle.create<MileageSurchExcept>();
    MileageSurchCxr* cxr1 = new MileageSurchCxr();
    MileageSurchCxr* cxr2 = new MileageSurchCxr();
    MileageSurchCxr* cxr3 = new MileageSurchCxr();

    cxr1->carrier() = "DL";
    cxr2->carrier() = "AA";
    cxr3->carrier() = "US";
    sur->cxrs().push_back(cxr1);
    sur->cxrs().push_back(cxr2);
    sur->cxrs().push_back(cxr3);
    sur->mustViaCxrExcept() = ' ';

    MileageSurchargeException validator;
    CPPUNIT_ASSERT(!validator.validateCxrRestriction(*sur, *_tvlRoute));
  }

  void testCxrRestrictionsMustFail2()
  {
    TravelRoute::CityCarrier city1, city2, city3;
    city1.carrier() = "AA";
    city2.carrier() = "AA";
    city3.carrier() = "AA";

    _tvlRoute->travelRoute().push_back(city1);
    _tvlRoute->travelRoute().push_back(city2);
    _tvlRoute->travelRoute().push_back(city3);

    MileageSurchExcept* sur = _memHandle.create<MileageSurchExcept>();
    MileageSurchCxr* cxr1 = new MileageSurchCxr();

    cxr1->carrier() = "DL";
    sur->cxrs().push_back(cxr1);
    sur->mustViaCxrExcept() = ' ';

    MileageSurchargeException validator;
    CPPUNIT_ASSERT(!validator.validateCxrRestriction(*sur, *_tvlRoute));
  }

  void testCxrRestrictionsExceptPass()
  {
    TravelRoute::CityCarrier city1, city2, city3;
    city1.carrier() = "AA";
    city2.carrier() = "DL";
    city3.carrier() = "BA";

    _tvlRoute->travelRoute().push_back(city1);
    _tvlRoute->travelRoute().push_back(city2);
    _tvlRoute->travelRoute().push_back(city3);

    MileageSurchExcept* sur = _memHandle.create<MileageSurchExcept>();
    MileageSurchCxr* cxr1 = new MileageSurchCxr();
    MileageSurchCxr* cxr2 = new MileageSurchCxr();
    MileageSurchCxr* cxr3 = new MileageSurchCxr();

    cxr1->carrier() = "F9";
    cxr2->carrier() = "UA";
    cxr3->carrier() = "US";
    sur->cxrs().push_back(cxr1);
    sur->cxrs().push_back(cxr2);
    sur->cxrs().push_back(cxr3);
    sur->mustViaCxrExcept() = 'Y';

    MileageSurchargeException validator;
    CPPUNIT_ASSERT(validator.validateCxrRestriction(*sur, *_tvlRoute));
  }

  void testCxrRestrictionsExceptFail1()
  {
    TravelRoute::CityCarrier city1, city2, city3;
    city1.carrier() = "AA";
    city2.carrier() = "DL";
    city3.carrier() = "BA";

    _tvlRoute->travelRoute().push_back(city1);
    _tvlRoute->travelRoute().push_back(city2);
    _tvlRoute->travelRoute().push_back(city3);

    MileageSurchExcept* sur = _memHandle.create<MileageSurchExcept>();
    MileageSurchCxr* cxr1 = new MileageSurchCxr();

    cxr1->carrier() = "DL";
    sur->cxrs().push_back(cxr1);
    sur->mustViaCxrExcept() = 'Y';

    MileageSurchargeException validator;
    CPPUNIT_ASSERT(!validator.validateCxrRestriction(*sur, *_tvlRoute));
  }

  void testCxrRestrictionsExceptFail2()
  {
    TravelRoute::CityCarrier city1, city2, city3;
    city1.carrier() = "AA";
    city2.carrier() = "DL";
    city3.carrier() = "BA";

    _tvlRoute->travelRoute().push_back(city1);
    _tvlRoute->travelRoute().push_back(city2);
    _tvlRoute->travelRoute().push_back(city3);

    MileageSurchExcept* sur = _memHandle.create<MileageSurchExcept>();
    MileageSurchCxr* cxr1 = new MileageSurchCxr();
    MileageSurchCxr* cxr2 = new MileageSurchCxr();
    MileageSurchCxr* cxr3 = new MileageSurchCxr();

    cxr1->carrier() = "DL";
    cxr2->carrier() = "AA";
    cxr3->carrier() = "US";
    sur->cxrs().push_back(cxr1);
    sur->cxrs().push_back(cxr2);
    sur->cxrs().push_back(cxr3);
    sur->mustViaCxrExcept() = 'Y';

    MileageSurchargeException validator;
    CPPUNIT_ASSERT(!validator.validateCxrRestriction(*sur, *_tvlRoute));
  }

  void testUpdate()
  {
    MileageSurchargeException mse;
    mse.update(*_paxTypeFare, _mInfo);

    CPPUNIT_ASSERT(_paxTypeFare->surchargeExceptionApplies());
    CPPUNIT_ASSERT_EQUAL(0, (int)_paxTypeFare->mileageSurchargePctg());
    CPPUNIT_ASSERT_EQUAL(0, (int)_paxTypeFare->mileageSurchargeAmt());
  }

  void testApplyNoSurcharge()
  {
    Indicator mpmSurchExcept = MileageSurchargeException::SURCHARGE_DOESNT_APPLY;

    MileageSurchargeException mse;
    mse.apply(mpmSurchExcept, *_paxTypeFare, _mInfo);

    CPPUNIT_ASSERT(_paxTypeFare->surchargeExceptionApplies());
    CPPUNIT_ASSERT_EQUAL(0, (int)_paxTypeFare->mileageSurchargePctg());
    CPPUNIT_ASSERT_EQUAL(0, (int)_paxTypeFare->mileageSurchargeAmt());
  }

  void testApplyWithinMPM()
  {
    Indicator mpmSurchExcept = MileageSurchargeException::TRAVEL_WITHIN_MPM;

    MileageSurchargeException mse;
    mse.apply(mpmSurchExcept, *_paxTypeFare, _mInfo);

    CPPUNIT_ASSERT(_paxTypeFare->surchargeExceptionApplies());
    CPPUNIT_ASSERT(_paxTypeFare->surchargeExceptionApplies());
    CPPUNIT_ASSERT_EQUAL(30, (int)_paxTypeFare->mileageSurchargePctg());
  }

  void testApplyNoUpdate()
  {
    Indicator mpmSurchExcept = '1';

    _paxTypeFare->surchargeExceptionApplies() = false;
    _paxTypeFare->mileageSurchargePctg() = 30;
    _paxTypeFare->mileageSurchargeAmt() = 10;

    MileageSurchargeException mse;
    mse.apply(mpmSurchExcept, *_paxTypeFare, _mInfo);

    CPPUNIT_ASSERT(!_paxTypeFare->surchargeExceptionApplies());
    CPPUNIT_ASSERT_EQUAL(30, (int)_paxTypeFare->mileageSurchargePctg());
    CPPUNIT_ASSERT_EQUAL(10, (int)_paxTypeFare->mileageSurchargeAmt());
  }

  void testApplyUpdate()
  {
    Indicator mpmSurchExcept = '9';

    _paxTypeFare->surchargeExceptionApplies() = false;
    _paxTypeFare->mileageSurchargePctg() = 30;
    _paxTypeFare->mileageSurchargeAmt() = 10;

    MileageSurchargeException mse;
    mse.apply(mpmSurchExcept, *_paxTypeFare, _mInfo);

    CPPUNIT_ASSERT(_paxTypeFare->surchargeExceptionApplies());
    CPPUNIT_ASSERT_EQUAL(0, (int)_paxTypeFare->mileageSurchargePctg());
    CPPUNIT_ASSERT_EQUAL(0, (int)_paxTypeFare->mileageSurchargeAmt());
  }

  void testIsValidPaxTypeEmpty()
  {
    std::vector<PaxTypeCode> paxTypes;
    PaxTypeCode paxType;

    MileageSurchargeException mse;
    CPPUNIT_ASSERT(mse.isValidPaxType(paxTypes, paxType));
  }

  void testIsValidPaxTypeFound()
  {
    std::vector<PaxTypeCode> paxTypes;
    paxTypes.push_back("AAA");
    PaxTypeCode paxType = "AAA";

    MileageSurchargeException mse;
    CPPUNIT_ASSERT(mse.isValidPaxType(paxTypes, paxType));
  }

  void testIsValidPaxTypeNotFound()
  {
    std::vector<PaxTypeCode> paxTypes;
    paxTypes.push_back("AAA");
    PaxTypeCode paxType = "ABC";

    MileageSurchargeException mse;
    CPPUNIT_ASSERT(!mse.isValidPaxType(paxTypes, paxType));
  }

  void testIsValidFareTypeEmpty()
  {
    FareClassCode actualFareType, expectedFareType;

    MileageSurchargeException mse;
    CPPUNIT_ASSERT(mse.isValidFareType(actualFareType, expectedFareType));
  }

  void testIsValidFareTypeEqual()
  {
    FareClassCode actualFareType = "ABC", expectedFareType = "ABC";

    MileageSurchargeException mse;
    CPPUNIT_ASSERT(mse.isValidFareType(actualFareType, expectedFareType));
  }

  void testIsValidFareTypeNotEqual()
  {
    FareClassCode actualFareType = "AAA", expectedFareType = "BBB";

    MileageSurchargeException mse;
    CPPUNIT_ASSERT(!mse.isValidFareType(actualFareType, expectedFareType));
  }

  void prepareFareForValidate(PaxTypeFare& paxTypeFare, uint16_t pctg = 0, bool routing = false)
  {
    Fare* fare = _memHandle.create<Fare>();
    PaxType* paxType = _memHandle.create<PaxType>();
    FareMarket* fareMarket = _memHandle.create<FareMarket>();

    paxTypeFare.initialize(fare, paxType, fareMarket);
    paxTypeFare.setIsShoppingFare(); // for isValid to pass
    paxTypeFare.setRoutingValid(routing);
    paxTypeFare.mileageSurchargePctg() = pctg;
  }
  void testValidateNoPositiveSurcharge()
  {
    PaxTypeFare paxTypeFare1, paxTypeFare2;
    prepareFareForValidate(paxTypeFare1);
    prepareFareForValidate(paxTypeFare2, 0, true);

    std::vector<PaxTypeFare*> pFares;
    pFares.push_back(&paxTypeFare1);
    pFares.push_back(&paxTypeFare2);

    MileageSurchargeException mse;
    mse.validate(*_tvlRoute, pFares, *_trx, _mInfo);

    CPPUNIT_ASSERT(!paxTypeFare1.isRoutingValid());
    CPPUNIT_ASSERT(paxTypeFare2.isRoutingValid());
  }

  void testValidateNoValidMarkets()
  {
    using namespace ::testing;

    prepareFareForValidate(*_paxTypeFare, 10, true);

    std::vector<PaxTypeFare*> pFares;
    pFares.push_back(_paxTypeFare.get());

    EXPECT_CALL(*_mockMileageSurchargeException, getData(_, _)).WillOnce(Return(_validMarkets));
    _mockMileageSurchargeException->MileageSurchargeException::validate(
        *_tvlRoute, pFares, *_trx, _mInfo);

    CPPUNIT_ASSERT(_paxTypeFare->isRoutingValid());
  }

  void testValidateNoValidMarketsPctgAbove25()
  {
    using namespace ::testing;

    prepareFareForValidate(*_paxTypeFare, 26, true);

    std::vector<PaxTypeFare*> pFares;
    pFares.push_back(_paxTypeFare.get());

    EXPECT_CALL(*_mockMileageSurchargeException, getData(_, _)).WillOnce(Return(_validMarkets));
    _mockMileageSurchargeException->MileageSurchargeException::validate(
        *_tvlRoute, pFares, *_trx, _mInfo);

    CPPUNIT_ASSERT(!_paxTypeFare->isRoutingValid());
  }

  void testValidateStopOverRestrictionEmpty()
  {
    MileageSurchargeException mse;
    LocKey locKey;

    CPPUNIT_ASSERT(mse.validateStopOverRestriction(locKey, *_tvlRoute));
  }

  void testValidateFareRestriction()
  {
    _fareInfo->fareClass() = "AAA";
    _fare->setFareInfo(_fareInfo.get());
    _paxTypeFare->setFare(_fare.get());

    MileageSurchExcept sur;
    sur.fareClass() = "BBB";

    MileageSurchargeException mse;
    CPPUNIT_ASSERT(!mse.validateFareRestriction(sur, *_paxTypeFare, *_trx));
  }

  void testValidateGeoRestrictionEmpty()
  {
    MileageSurchargeException mse;
    MileageSurchExcept sur;
    CPPUNIT_ASSERT(mse.validateGeoRestriction(sur, *_tvlRoute));
  }

  void testIsExceptionApplicableNotValidFareRestriction()
  {
    _fareInfo->fareClass() = "AAA";
    _fare->setFareInfo(_fareInfo.get());
    _paxTypeFare->setFare(_fare.get());

    MileageSurchExcept sur;
    sur.fareClass() = "BBB";

    MileageSurchargeException mse;
    CPPUNIT_ASSERT(!mse.isExceptionApplicable(sur, *_paxTypeFare, *_tvlRoute, *_trx));
  }

  void testValidateSingleFareMileageSurchargePctgFail()
  {
    using namespace ::testing;

    _dummyMileageSurchargePctg = 35;
    _paxTypeFare->mileageSurchargePctg() = _dummyMileageSurchargePctg;
    EXPECT_CALL(*_mockMileageSurchargeException, getData(_, _)).WillOnce(Return(_validMarkets));

    CPPUNIT_ASSERT(!_mockMileageSurchargeException->MileageSurchargeException::validateSingleFare(
        *_tvlRoute, *_paxTypeFare, *_trx, _mInfo));
    CPPUNIT_ASSERT(!_paxTypeFare->isRoutingValid());
  }

  void testValidateSingleFareInvValidMileageSurchargePctgCase1()
  {
    _paxTypeFare->mileageSurchargePctg() = _dummyMileageSurchargePctg;
    MileageSurchargeException mse;
    CPPUNIT_ASSERT(mse.validateSingleFare(*_tvlRoute, *_paxTypeFare, *_trx, _mInfo));
  }

  void deactivateFullMapRouting(MockPricingTrx& mockTrx)
  {
    using namespace ::testing;

    std::string activationDate = "2013-06-16";
    TestConfigInitializer::setValue("FULL_MAP_ROUTING_ACTIVATION_DATE", activationDate, "PRICING_SVC");
    DateTime currentDate = DateTime(2013, 6, 14);

    EXPECT_CALL(Const(mockTrx), ticketingDate()).WillOnce(ReturnRef(currentDate));
  }

  void testValidateSingleFareInvValidMileageSurchargePctgCase2()
  {
    using namespace ::testing;

    _dummyMileageSurchargePctg = 10;
    _paxTypeFare->mileageSurchargePctg() = _dummyMileageSurchargePctg;

    MileageSurchExcept* milage = new MileageSurchExcept();
    _validMarkets.push_back(milage);
    EXPECT_CALL(*_mockMileageSurchargeException, getData(_, _)).WillOnce(Return(_validMarkets));

    MockPricingTrx mockTrx;
    deactivateFullMapRouting(mockTrx);

    EXPECT_CALL(*_mockMileageSurchargeException, isExceptionApplicable(_, _, _, _))
        .WillOnce(Return(true));

    CPPUNIT_ASSERT_EQUAL(
        true,
        _mockMileageSurchargeException->MileageSurchargeException::validateSingleFare(
            *_tvlRoute, *_paxTypeFare, mockTrx, _mInfo));
  }

private:
  std::shared_ptr<Fare> _fare;
  std::shared_ptr<FareInfo> _fareInfo;
  std::shared_ptr<PaxTypeFare> _paxTypeFare;
  std::shared_ptr<TravelRoute> _tvlRoute;
  std::shared_ptr<PricingTrx> _trx;
  std::shared_ptr<MockMileageSurchargeException> _mockMileageSurchargeException;
  MileageInfo _mInfo;
  TestMemHandle _memHandle;
  ConfigMan* _configMan;
  uint16_t _dummyMileageSurchargePctg;
  std::vector<MileageSurchExcept*> _validMarkets;
};

CPPUNIT_TEST_SUITE_REGISTRATION(MileageSurchargeExceptionTest);
}
