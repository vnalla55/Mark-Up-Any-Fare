//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//               Andrzej Fediuk
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "test/include/GtestHelperMacros.h"

#include "Common/CabinType.h"
#include "Common/ErrorResponseException.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TNBrands/TNBrandsFunctions.h"
#include "Common/TNBrands/test/TNBrandsMocks.h"

#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"

#include <memory>
#include <vector>


using namespace ::testing;

namespace tse
{
FALLBACKVALUE_DECL(fallbackBrandDirectionality);

namespace skipper
{

class TNBrandsFunctionsTest: public Test
{
public:
  void SetUp()
  {
    _itinCalculator.reset(new MockItinGeometryCalculator<>());
    _trxCalculator.reset(new MockITrxGeometryCalculator());
    _fm1.reset(new MockFareMarket());
    _fm2.reset(new MockFareMarket());
    _fareMarkets.push_back(_fm1.get());
    _fareMarkets.push_back(_fm2.get());
    _brands1.insert("A");
    _brands1.insert("B");
    _brands1.insert("C");
    _brands2.insert("B");
    _brands2.insert("C");
    _brands2.insert("D");
    _brands3.insert("SV");
    _brands3.insert("BZ");
    _brands3.insert("PE");
    _brands3.insert(NO_BRAND);
    //set to false when directionality is added
    fallback::value::fallbackBrandDirectionality.set(true);
  }

  void TearDown(){}

  void assureProportionalIndex(size_t expectedInputIndex,
                               size_t outputIndex,
                               size_t inputSetSize,
                               size_t outputSetSize)
  {
    ASSERT_EQ(expectedInputIndex,
        TNBrandsFunctions::calculateProportionalIndex(
            outputIndex, inputSetSize, outputSetSize));
  }

  void assureBottomPreferredIndex(size_t expectedInputIndex,
                                  size_t outputIndex,
                                  size_t inputSetSize,
                                  size_t outputSetSize)
  {
    ASSERT_EQ(expectedInputIndex,
        TNBrandsFunctions::calculateBottomPreferredIndex(
            outputIndex, inputSetSize, outputSetSize));
  }

  void assureTopPreferredIndex(size_t expectedInputIndex,
                               size_t outputIndex,
                               size_t inputSetSize,
                               size_t outputSetSize)
  {
    ASSERT_EQ(expectedInputIndex,
        TNBrandsFunctions::calculateTopPreferredIndex(
            outputIndex, inputSetSize, outputSetSize));
  }

  void fillTrxAndItinWithCabinInfo(
    TestMemHandle& memHandle, PricingTrx& trx, Itin& itin, bool skipPremium = false)
  {
    Loc* org1 = memHandle.create<Loc>();
    org1->loc() = "AKL";
    Loc* dst1 = memHandle.create<Loc>();
    dst1->loc() = "SYD";

    Loc* org2 = memHandle.create<Loc>();
    org2->loc() = "XXX";
    Loc* dst2 = memHandle.create<Loc>();
    dst2->loc() = "ZZZ";

    // Create brands in TRX
    BrandProgram* programO = memHandle.create<BrandProgram>();
    programO->originsRequested() = {org1->loc(), org2->loc()};
    programO->destinationsRequested() = {dst1->loc(), dst2->loc()};
    BrandInfo* economyBrandO = memHandle.create<BrandInfo>();
    economyBrandO->brandCode() = ECONOMY_BRAND;
    BrandInfo* businessBrandO = memHandle.create<BrandInfo>();
    businessBrandO->brandCode() = BUSINESS_BRAND;
    BrandInfo* premiumBrandO = memHandle.create<BrandInfo>();
    premiumBrandO->brandCode() = PREMIUM_BRAND;
    BrandProgram* programR = memHandle.create<BrandProgram>();
    programR->originsRequested() = {dst2->loc()};
    programR->destinationsRequested() = {org2->loc()};
    BrandInfo* economyBrandR = memHandle.create<BrandInfo>();
    economyBrandR->brandCode() = ECONOMY_BRAND;
    BrandInfo* businessBrandR = memHandle.create<BrandInfo>();
    businessBrandR->brandCode() = BUSINESS_BRAND;
    BrandInfo* premiumBrandR = memHandle.create<BrandInfo>();
    premiumBrandR->brandCode() = PREMIUM_BRAND;

    BrandProgram* programB = memHandle.create<BrandProgram>();
    programB->originsRequested() = {org1->loc(), dst2->loc()};
    programB->destinationsRequested() = {org1->loc(), dst2->loc()};
    BrandInfo* economyBrandB = memHandle.create<BrandInfo>();
    economyBrandB->brandCode() = ECONOMY_BRAND;

    trx.brandProgramVec().push_back(std::make_pair(programO, economyBrandO));
    trx.brandProgramVec().push_back(std::make_pair(programO, businessBrandO));
    trx.brandProgramVec().push_back(std::make_pair(programO, premiumBrandO));
    trx.brandProgramVec().push_back(std::make_pair(programR, economyBrandR));
    trx.brandProgramVec().push_back(std::make_pair(programR, businessBrandR));
    trx.brandProgramVec().push_back(std::make_pair(programR, premiumBrandR));
    trx.brandProgramVec().push_back(std::make_pair(programB, economyBrandB));

    // Creating vectors of brands statuses for PaxTypeFare
    // Each vector holds status that point at different brands
    // ( economy, business, premium )
    std::vector<PaxTypeFare::BrandStatusWithDirection> economyPassO(
        7, std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS));
    economyPassO[0].first = PaxTypeFare::BS_HARD_PASS;
    economyPassO[0].second = Direction::ORIGINAL;
    std::vector<PaxTypeFare::BrandStatusWithDirection> businessPassO(
        7, std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS));
    businessPassO[1].first = PaxTypeFare::BS_HARD_PASS;
    businessPassO[1].second = Direction::ORIGINAL;
    std::vector<PaxTypeFare::BrandStatusWithDirection> premiumPassO(
        7, std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS));
    premiumPassO[2].first = PaxTypeFare::BS_HARD_PASS;
    premiumPassO[2].second = Direction::ORIGINAL;
    std::vector<PaxTypeFare::BrandStatusWithDirection> economyPassR(
        7, std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS));
    economyPassR[3].first = PaxTypeFare::BS_HARD_PASS;
    economyPassR[3].second = Direction::REVERSED;
    std::vector<PaxTypeFare::BrandStatusWithDirection> businessPassR(
        7, std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS));
    businessPassR[4].first = PaxTypeFare::BS_HARD_PASS;
    businessPassR[4].second = Direction::REVERSED;
    std::vector<PaxTypeFare::BrandStatusWithDirection> premiumPassR(
        7, std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS));
    premiumPassR[5].first = PaxTypeFare::BS_HARD_PASS;
    premiumPassR[5].second = Direction::REVERSED;
    std::vector<PaxTypeFare::BrandStatusWithDirection> economyPassB(
        7, std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS));
    economyPassB[6].first = PaxTypeFare::BS_HARD_PASS;

    // ******************************************************
    TravelSeg* segment1 = memHandle.create<AirSeg>();
    TravelSeg* segment2 = memHandle.create<AirSeg>();

    itin.travelSeg().push_back(segment1);
    itin.travelSeg().push_back(segment2);

    FareMarket *localFm1, *localFm2, *thruFm, *localFm1C2, *localFm2C2;

    std::vector<int> brandsIndices = {0, 1, 2, 3, 4, 5, 6};
    localFm1 = memHandle.create<FareMarket>();
    localFm1->travelSeg().push_back(segment1);
    localFm1->legIndex() = 0;
    localFm1->brandProgramIndexVec() = brandsIndices;
    localFm1->origin() = org1;
    localFm1->destination() = dst1;
    localFm1->addOriginRequestedForOriginalDirection(org1->loc());

    localFm2 = memHandle.create<FareMarket>();
    localFm2->travelSeg().push_back(segment2);
    localFm2->legIndex() = 1;
    localFm2->brandProgramIndexVec() = brandsIndices;
    localFm2->origin() = org2;
    localFm2->destination() = dst2;
    localFm2->addOriginRequestedForOriginalDirection(org2->loc());
    localFm2->addDestinationRequestedForReversedDirection(org2->loc());

    thruFm = memHandle.create<FareMarket>();
    thruFm->travelSeg().push_back(segment1);
    thruFm->travelSeg().push_back(segment2);
    thruFm->legIndex() = 0;
    thruFm->brandProgramIndexVec() = brandsIndices;
    thruFm->origin() = org1;
    thruFm->destination() = dst2;
    thruFm->addOriginRequestedForOriginalDirection(org1->loc());
    thruFm->addDestinationRequestedForReversedDirection(org1->loc());

    localFm1C2 = memHandle.create<FareMarket>();
    localFm1C2->travelSeg().push_back(segment1);
    localFm1C2->legIndex() = 0;
    localFm1C2->brandProgramIndexVec() = brandsIndices;
    localFm1C2->origin() = org1;
    localFm1C2->destination() = dst1;
    localFm1C2->addOriginRequestedForOriginalDirection(org1->loc());
    localFm1C2->addDestinationRequestedForReversedDirection(org1->loc());

    localFm2C2 = memHandle.create<FareMarket>();
    localFm2C2->travelSeg().push_back(segment2);
    localFm2C2->legIndex() = 1;
    localFm2C2->brandProgramIndexVec() = brandsIndices;
    localFm2C2->origin() = org2;
    localFm2C2->destination() = dst2;
    localFm2C2->addOriginRequestedForOriginalDirection(org2->loc());
    localFm2C2->addDestinationRequestedForReversedDirection(org2->loc());

    itin.fareMarket().push_back(localFm1);
    itin.fareMarket().push_back(localFm2);
    itin.fareMarket().push_back(thruFm);
    itin.fareMarket().push_back(localFm1C2);
    itin.fareMarket().push_back(localFm2C2);

    // Creating pax type fares of different cabins:
    // economy, premium economy, business and premium

    FareInfo* fareInfo = memHandle.create<FareInfo>();
    fareInfo->directionality() = BOTH;
    fareInfo->market1() = "SYD";

    Fare* fare = memHandle.create<Fare>();
    fare->initialize(Fare::FareState::FS_Domestic, fareInfo, *localFm1, nullptr, nullptr);

    PaxTypeFare* ptf1O = memHandle.create<PaxTypeFare>();
    ptf1O->setFare(fare);
    ptf1O->fareMarket() = localFm1;
    ptf1O->cabin().setEconomyClass();
    ptf1O->getMutableBrandStatusVec() = economyPassO;

    PaxTypeFare* ptf2O = memHandle.create<PaxTypeFare>();
    ptf2O->setFare(fare);
    ptf2O->fareMarket() = localFm2;
    ptf2O->cabin().setPremiumEconomyClass();
    ptf2O->getMutableBrandStatusVec() = economyPassO;

    PaxTypeFare* ptf2R = memHandle.create<PaxTypeFare>();
    ptf2R->setFare(fare);
    ptf2R->fareMarket() = localFm2;
    ptf2R->cabin().setPremiumEconomyClass();
    ptf2R->getMutableBrandStatusVec() = economyPassR;

    PaxTypeFare* ptf3O = memHandle.create<PaxTypeFare>();
    ptf3O->setFare(fare);
    ptf3O->fareMarket() = localFm2;
    ptf3O->cabin().setBusinessClass();
    ptf3O->getMutableBrandStatusVec() = businessPassO;

    PaxTypeFare* ptf3R = memHandle.create<PaxTypeFare>();
    ptf3R->setFare(fare);
    ptf3R->fareMarket() = localFm2;
    ptf3R->cabin().setBusinessClass();
    ptf3R->getMutableBrandStatusVec() = businessPassR;

    PaxTypeFare* ptf4O = memHandle.create<PaxTypeFare>();
    ptf4O->setFare(fare);
    ptf4O->fareMarket() = thruFm;
    PaxTypeFare* ptf4R = memHandle.create<PaxTypeFare>();
    ptf4R->setFare(fare);
    ptf4R->fareMarket() = thruFm;
    if (skipPremium)
    {
      ptf4O->cabin().setBusinessClass();
      ptf4O->getMutableBrandStatusVec() = businessPassO;
      ptf4R->cabin().setBusinessClass();
      ptf4R->getMutableBrandStatusVec() = businessPassR;
    }
    else
    {
      ptf4O->cabin().setFirstClass();
      ptf4O->getMutableBrandStatusVec() = premiumPassO;
      ptf4R->cabin().setFirstClass();
      ptf4R->getMutableBrandStatusVec() = premiumPassR;
    }

    PaxTypeFare* ptf5B = memHandle.create<PaxTypeFare>();
    ptf5B->setFare(fare);
    ptf5B->fareMarket() = localFm1C2;
    ptf5B->cabin().setEconomyClass();
    ptf5B->getMutableBrandStatusVec() = economyPassB;

    PaxTypeFare* ptf6B = memHandle.create<PaxTypeFare>();
    ptf6B->setFare(fare);
    ptf6B->fareMarket() = localFm2C2;
    ptf6B->cabin().setEconomyClass();
    ptf6B->getMutableBrandStatusVec() = economyPassB;

    // market on leg 0 has only economy
    localFm1->allPaxTypeFare().push_back(ptf1O);
    ptf1O->fareMarket() = localFm1;
    localFm1->governingCarrier() = GOVERNING_CARRIER;

    localFm1C2->allPaxTypeFare().push_back(ptf5B);
    ptf5B->fareMarket() = localFm1C2;
    localFm1C2->governingCarrier() = GOVERNING_CARRIER2;

    // market on leg 1 has (premium) economy and business
    localFm2->allPaxTypeFare().push_back(ptf2O);
    ptf2O->fareMarket() = localFm2;
    localFm2->governingCarrier() = GOVERNING_CARRIER;
    localFm2->allPaxTypeFare().push_back(ptf3O);
    ptf3O->fareMarket() = localFm2;
    localFm2->allPaxTypeFare().push_back(ptf2R);
    ptf2R->fareMarket() = localFm2;
    localFm2->allPaxTypeFare().push_back(ptf3R);
    ptf3R->fareMarket() = localFm2;

    localFm2C2->allPaxTypeFare().push_back(ptf6B);
    ptf6B->fareMarket() = localFm2C2;
    localFm2C2->governingCarrier() = GOVERNING_CARRIER2;

    // fare market spanning both legs has premium
    thruFm->allPaxTypeFare().push_back(ptf4O);
    ptf4O->fareMarket() = thruFm;
    thruFm->governingCarrier() = GOVERNING_CARRIER;
    thruFm->allPaxTypeFare().push_back(ptf4R);
    ptf4R->fareMarket() = thruFm;
  }

  std::shared_ptr<MockItinGeometryCalculator<>> _itinCalculator;
  std::shared_ptr<MockITrxGeometryCalculator> _trxCalculator;
  std::vector<MockFareMarket*> _fareMarkets;
  std::shared_ptr<MockFareMarket> _fm1;
  std::shared_ptr<MockFareMarket> _fm2;
  UnorderedBrandCodes _brands1;
  UnorderedBrandCodes _brands2;
  UnorderedBrandCodes _brands3;

  TestConfigInitializer _config;

  static const size_t SEGMENT_COUNT;
  static const CarrierCode GOVERNING_CARRIER;
  static const CarrierCode GOVERNING_CARRIER2;

  static const size_t INPUT_SET_SIZE;
  static const size_t OUTPUT_SET_SIZE;
  static const size_t BIG_INPUT_SET_SIZE;
  static const size_t SMALL_OUTPUT_SET_SIZE;

  static const BrandCode ECONOMY_BRAND;
  static const BrandCode BUSINESS_BRAND;
  static const BrandCode PREMIUM_BRAND;
};

const size_t TNBrandsFunctionsTest::SEGMENT_COUNT = 2;
const CarrierCode TNBrandsFunctionsTest::GOVERNING_CARRIER = "AA";
const CarrierCode TNBrandsFunctionsTest::GOVERNING_CARRIER2 = "BB";

const size_t TNBrandsFunctionsTest::INPUT_SET_SIZE = 5;
const size_t TNBrandsFunctionsTest::OUTPUT_SET_SIZE = 7;
const size_t TNBrandsFunctionsTest::BIG_INPUT_SET_SIZE = 13;
const size_t TNBrandsFunctionsTest::SMALL_OUTPUT_SET_SIZE = 9;

const BrandCode TNBrandsFunctionsTest::ECONOMY_BRAND = "EC";
const BrandCode TNBrandsFunctionsTest::BUSINESS_BRAND = "BZ";
const BrandCode TNBrandsFunctionsTest::PREMIUM_BRAND = "PM";

TEST_F(TNBrandsFunctionsTest, testCalculatesSegmentOrientedBrandsPerCarrier)
{
  PricingTrx trx;
  fallback::value::fallbackBrandDirectionality.set(false);

  EXPECT_CALL(*_itinCalculator, getTrxGeometryCalculator())
      .WillRepeatedly(ReturnRef(*_trxCalculator));

  EXPECT_CALL(*_trxCalculator, getTrx())
      .WillRepeatedly(ReturnRef(trx));

  EXPECT_CALL(*_itinCalculator, getSegmentCount()).WillOnce(Return(SEGMENT_COUNT));
  EXPECT_CALL(*_itinCalculator, getFareMarkets()).WillOnce(ReturnRef(_fareMarkets));
  // We decide that the fare markets fm1, fm2 start from segment 1.
  EXPECT_CALL(*_itinCalculator, getFareMarketStartSegmentIndex(_))
      .Times(2).WillRepeatedly(Return(1));

  // We assume _brands1 belong to _fm1 and the same for 2.
  EXPECT_CALL(*_itinCalculator, getItinSpecificBrandCodes(_))
      .WillOnce(Return(_brands1))
      .WillOnce(Return(_brands2));

  EXPECT_CALL(*_fm1, governingCarrier()).WillRepeatedly(ReturnRef(GOVERNING_CARRIER));
  EXPECT_CALL(*_fm2, governingCarrier()).WillRepeatedly(ReturnRef(GOVERNING_CARRIER));

  SegmentOrientedBrandCodesPerCarrier result;
  TNBrandsFunctions::calculateSegmentOrientedBrandCodesPerCarrier(
      *_itinCalculator, 0, result);

  SegmentOrientedBrandCodesPerCarrier expected;

  BrandCodesPerCarrier tmp;
  expected.push_back(tmp);
  tmp[CarrierDirection(GOVERNING_CARRIER, Direction::BOTHWAYS)].insert(_brands1.begin(), _brands1.end());
  tmp[CarrierDirection(GOVERNING_CARRIER, Direction::BOTHWAYS)].insert(_brands2.begin(), _brands2.end());
  expected.push_back(tmp);

  ASSERT_EQ(expected, result);
}

TEST_F(TNBrandsFunctionsTest, testCalculatesSegmentOrientedBrandsPerCarrierWithDirectionality)
{
  PricingTrx trx;
  trx.setTrxType(PricingTrx::MIP_TRX);
  fallback::value::fallbackBrandDirectionality.set(false);

  EXPECT_CALL(*_itinCalculator, getTrxGeometryCalculator()).WillRepeatedly(ReturnRef(*_trxCalculator));
  EXPECT_CALL(*_trxCalculator, getTrx()).WillRepeatedly(ReturnRef(trx));

  // use 4 segments in this test
  EXPECT_CALL(*_itinCalculator, getSegmentCount()).WillOnce(Return(4));
  EXPECT_CALL(*_itinCalculator, getFareMarkets()).WillOnce(ReturnRef(_fareMarkets));

  std::shared_ptr<MockFareMarket> _fm3;
  _fm3.reset(new MockFareMarket);
  _fareMarkets.push_back(_fm3.get());
  std::shared_ptr<MockFareMarket> _fm4;
  _fm4.reset(new MockFareMarket);
  _fareMarkets.push_back(_fm4.get());

  // We decide that the fare markets fm1 and fm2 start from segment 1,
  // fm3 starts form segment 2, and fm4 on segment 3
  EXPECT_CALL(*_itinCalculator, getFareMarketStartSegmentIndex(Ref(*_fm1)))
    .WillOnce(Return(1));
  EXPECT_CALL(*_itinCalculator, getFareMarketStartSegmentIndex(Ref(*_fm2)))
    .WillOnce(Return(1));
  EXPECT_CALL(*_itinCalculator, getFareMarketStartSegmentIndex(Ref(*_fm3)))
    .WillOnce(Return(2));
  EXPECT_CALL(*_itinCalculator, getFareMarketStartSegmentIndex(Ref(*_fm4)))
    .WillOnce(Return(3));

  BrandProgram program;
  BrandInfo b1, b2, b3, b4;
  b1.brandCode() = "A";
  b2.brandCode() = "B";
  b3.brandCode() = "C";
  b4.brandCode() = "D";
  std::vector<QualifiedBrand> allBrands = {{&program, &b1}, {&program, &b2},
                                           {&program, &b3}, {&program, &b4}};
  QualifiedBrandIndices brandCodeIndicesFm1 = {0, 1, 2}; //_brands1
  QualifiedBrandIndices brandCodeIndicesFm2 = {1, 2, 3}; //_brands2
  QualifiedBrandIndices brandCodeIndicesFm3 = {0, 1, 2}; //_brands1
  QualifiedBrandIndices brandCodeIndicesFm4 = {}; //no brands
  EXPECT_CALL(*_itinCalculator, getItinSpecificBrandCodesIndices(Ref(*_fm1)))
    .WillOnce(Return(brandCodeIndicesFm1));
  EXPECT_CALL(*_itinCalculator, getItinSpecificBrandCodesIndices(Ref(*_fm2)))
    .WillOnce(Return(brandCodeIndicesFm2));
  EXPECT_CALL(*_itinCalculator, getItinSpecificBrandCodesIndices(Ref(*_fm3)))
    .WillOnce(Return(brandCodeIndicesFm3));
  EXPECT_CALL(*_itinCalculator, getItinSpecificBrandCodesIndices(Ref(*_fm4)))
    .WillOnce(Return(brandCodeIndicesFm4));
  EXPECT_CALL(*_trxCalculator, getQualifiedBrands())
    .WillRepeatedly(ReturnRef(allBrands));

  EXPECT_CALL(*_fm1, governingCarrier()).WillRepeatedly(ReturnRef(GOVERNING_CARRIER));
  EXPECT_CALL(*_fm2, governingCarrier()).WillRepeatedly(ReturnRef(GOVERNING_CARRIER));
  EXPECT_CALL(*_fm3, governingCarrier()).WillRepeatedly(ReturnRef(GOVERNING_CARRIER));
  EXPECT_CALL(*_fm4, governingCarrier()).WillRepeatedly(ReturnRef(GOVERNING_CARRIER));

  Direction dirOriginal = Direction::ORIGINAL;
  Direction dirReversed = Direction::REVERSED;
  EXPECT_CALL(*_itinCalculator, getProgramDirection(_, Ref(*_fm1), _))
    .WillRepeatedly(DoAll(SetArgReferee<2>(dirOriginal), Return(true)));
  EXPECT_CALL(*_itinCalculator, getProgramDirection(_, Ref(*_fm2), _))
    .WillRepeatedly(DoAll(SetArgReferee<2>(dirReversed), Return(true)));
  EXPECT_CALL(*_itinCalculator, getProgramDirection(_, Ref(*_fm3), _))
    .WillRepeatedly(DoAll(SetArgReferee<2>(dirOriginal), Return(true)));

  SegmentOrientedBrandCodesPerCarrier result;
  TNBrandsFunctions::calculateSegmentOrientedBrandCodesPerCarrier(
      *_itinCalculator, 0, result);

  SegmentOrientedBrandCodesPerCarrier expected;
  BrandCodesPerCarrier tmp;
  //segment 0
  expected.push_back(tmp);
  //segment 1
  tmp[CarrierDirection(GOVERNING_CARRIER, Direction::ORIGINAL)].insert(_brands1.begin(), _brands1.end());
  tmp[CarrierDirection(GOVERNING_CARRIER, Direction::REVERSED)].insert(_brands2.begin(), _brands2.end());
  expected.push_back(tmp);
  //segment 2
  tmp.clear();
  tmp[CarrierDirection(GOVERNING_CARRIER, Direction::ORIGINAL)].insert(_brands1.begin(), _brands1.end());
  tmp[CarrierDirection(GOVERNING_CARRIER, Direction::REVERSED)]; //no brand
  expected.push_back(tmp);
  //segment 3
  tmp.clear();
  tmp[CarrierDirection(GOVERNING_CARRIER, Direction::BOTHWAYS)]; //no brand
  expected.push_back(tmp);

  ASSERT_EQ(expected, result);
}

TEST_F(TNBrandsFunctionsTest, testCalculatesSegmentOrientedBrandsPerCarrierWithFiltering)
{
  PricingTrx trx;
  fallback::value::fallbackBrandDirectionality.set(false);

  EXPECT_CALL(*_itinCalculator, getTrxGeometryCalculator())
      .WillRepeatedly(ReturnRef(*_trxCalculator));

  EXPECT_CALL(*_trxCalculator, getTrx())
      .WillRepeatedly(ReturnRef(trx));

  EXPECT_CALL(*_itinCalculator, getSegmentCount()).WillOnce(Return(SEGMENT_COUNT));
  EXPECT_CALL(*_itinCalculator, getFareMarkets()).WillOnce(ReturnRef(_fareMarkets));
  // We decide that the fare markets fm1, fm2 start from segment 1.
  EXPECT_CALL(*_itinCalculator, getFareMarketStartSegmentIndex(_))
    .Times(2).WillRepeatedly(Return(1));

  // We assume _brands1 belong to _fm1 and the same for 2.
  EXPECT_CALL(*_itinCalculator, getItinSpecificBrandCodes(_))
    .WillOnce(Return(_brands1))
    .WillOnce(Return(_brands2));

  EXPECT_CALL(*_fm1, governingCarrier()).WillRepeatedly(ReturnRef(GOVERNING_CARRIER));
  EXPECT_CALL(*_fm2, governingCarrier()).WillRepeatedly(ReturnRef(GOVERNING_CARRIER));

  SegmentOrientedBrandCodesPerCarrier result, filter, expected;
  BrandCodesPerCarrier tmp;
  expected.push_back(tmp);
  filter.push_back(tmp);
  tmp[CarrierDirection(GOVERNING_CARRIER, Direction::BOTHWAYS)]
      .insert(_brands2.begin(), _brands2.end());
  expected.push_back(tmp);
  // brands in filter that are not present in the processed set should not affect the results
  tmp[CarrierDirection("LO", Direction::BOTHWAYS)].insert(_brands3.begin(), _brands3.end());
  filter.push_back(tmp);

  TNBrandsFunctions::calculateSegmentOrientedBrandCodesPerCarrier(*_itinCalculator, &filter, result);

  ASSERT_EQ(expected, result);
}

TEST_F(TNBrandsFunctionsTest, testCalculatesSegmentOrientedBrandsPerCarrierWithFilteringAndDirectionality)
{
  PricingTrx trx;
  trx.setTrxType(PricingTrx::MIP_TRX);
  fallback::value::fallbackBrandDirectionality.set(false);

  EXPECT_CALL(*_itinCalculator, getTrxGeometryCalculator()).WillRepeatedly(ReturnRef(*_trxCalculator));
  EXPECT_CALL(*_trxCalculator, getTrx()).WillRepeatedly(ReturnRef(trx));

  // use 4 segments in this test
  EXPECT_CALL(*_itinCalculator, getSegmentCount()).WillOnce(Return(4));
  EXPECT_CALL(*_itinCalculator, getFareMarkets()).WillOnce(ReturnRef(_fareMarkets));

  std::shared_ptr<MockFareMarket> _fm3;
  _fm3.reset(new MockFareMarket);
  _fareMarkets.push_back(_fm3.get());
  std::shared_ptr<MockFareMarket> _fm4;
  _fm4.reset(new MockFareMarket);
  _fareMarkets.push_back(_fm4.get());

  // We decide that the fare markets fm1 and fm2 start from segment 1,
  // fm3 starts form segment 2, and fm4 on segment 3
  EXPECT_CALL(*_itinCalculator, getFareMarketStartSegmentIndex(Ref(*_fm1)))
    .WillOnce(Return(1));
  EXPECT_CALL(*_itinCalculator, getFareMarketStartSegmentIndex(Ref(*_fm2)))
    .WillOnce(Return(1));
  EXPECT_CALL(*_itinCalculator, getFareMarketStartSegmentIndex(Ref(*_fm3)))
    .WillOnce(Return(2));
  EXPECT_CALL(*_itinCalculator, getFareMarketStartSegmentIndex(Ref(*_fm4)))
    .WillOnce(Return(3));

  BrandProgram program;
  BrandInfo b1, b2, b3, b4;
  b1.brandCode() = "A";
  b2.brandCode() = "B";
  b3.brandCode() = "C";
  b4.brandCode() = "D";
  std::vector<QualifiedBrand> allBrands = {{&program, &b1}, {&program, &b2},
                                           {&program, &b3}, {&program, &b4}};
  QualifiedBrandIndices brandCodeIndicesFm1 = {0, 1, 2}; //_brands1
  QualifiedBrandIndices brandCodeIndicesFm2 = {1, 2, 3}; //_brands2
  QualifiedBrandIndices brandCodeIndicesFm3 = {0, 1, 2}; //_brands1
  QualifiedBrandIndices brandCodeIndicesFm4 = {}; //no brands
  EXPECT_CALL(*_itinCalculator, getItinSpecificBrandCodesIndices(Ref(*_fm1)))
    .WillOnce(Return(brandCodeIndicesFm1));
  EXPECT_CALL(*_itinCalculator, getItinSpecificBrandCodesIndices(Ref(*_fm2)))
    .WillOnce(Return(brandCodeIndicesFm2));
  EXPECT_CALL(*_itinCalculator, getItinSpecificBrandCodesIndices(Ref(*_fm3)))
    .WillOnce(Return(brandCodeIndicesFm3));
  EXPECT_CALL(*_itinCalculator, getItinSpecificBrandCodesIndices(Ref(*_fm4)))
    .WillOnce(Return(brandCodeIndicesFm4));
  EXPECT_CALL(*_trxCalculator, getQualifiedBrands())
    .WillRepeatedly(ReturnRef(allBrands));

  EXPECT_CALL(*_fm1, governingCarrier()).WillRepeatedly(ReturnRef(GOVERNING_CARRIER));
  EXPECT_CALL(*_fm2, governingCarrier()).WillRepeatedly(ReturnRef(GOVERNING_CARRIER));
  EXPECT_CALL(*_fm3, governingCarrier()).WillRepeatedly(ReturnRef(GOVERNING_CARRIER));
  EXPECT_CALL(*_fm4, governingCarrier()).WillRepeatedly(ReturnRef(GOVERNING_CARRIER));

  Direction dirOriginal = Direction::ORIGINAL;
  Direction dirReversed = Direction::REVERSED;
  EXPECT_CALL(*_itinCalculator, getProgramDirection(_, Ref(*_fm1), _))
    .WillRepeatedly(DoAll(SetArgReferee<2>(dirOriginal), Return(true)));
  EXPECT_CALL(*_itinCalculator, getProgramDirection(_, Ref(*_fm2), _))
    .WillRepeatedly(DoAll(SetArgReferee<2>(dirReversed), Return(true)));
  EXPECT_CALL(*_itinCalculator, getProgramDirection(_, Ref(*_fm3), _))
    .WillRepeatedly(DoAll(SetArgReferee<2>(dirOriginal), Return(true)));

  SegmentOrientedBrandCodesPerCarrier filter;
  BrandCodesPerCarrier tmp;
  //segment 0
  filter.push_back(tmp);
  //segment 1
  tmp[CarrierDirection(GOVERNING_CARRIER, Direction::ORIGINAL)].insert(b3.brandCode());
  tmp[CarrierDirection(GOVERNING_CARRIER, Direction::ORIGINAL)].insert(b4.brandCode());
  tmp[CarrierDirection(GOVERNING_CARRIER, Direction::REVERSED)].insert(b1.brandCode());
  tmp[CarrierDirection(GOVERNING_CARRIER, Direction::REVERSED)].insert(b2.brandCode());
  filter.push_back(tmp);
  //segment 2
  tmp.clear();
  tmp[CarrierDirection(GOVERNING_CARRIER, Direction::ORIGINAL)].insert(b3.brandCode());
  // brands in filter that are not present in the processed set should not affect the results
  tmp[CarrierDirection("LO", Direction::BOTHWAYS)].insert(_brands3.begin(), _brands3.end());
  filter.push_back(tmp);
  //segment 2
  tmp.clear();
  filter.push_back(tmp);

  SegmentOrientedBrandCodesPerCarrier result;
  TNBrandsFunctions::calculateSegmentOrientedBrandCodesPerCarrier(
      *_itinCalculator, &filter, result);

  SegmentOrientedBrandCodesPerCarrier expected;
  //segment 0
  tmp.clear();
  expected.push_back(tmp);
  //segment 1
  tmp[CarrierDirection(GOVERNING_CARRIER, Direction::ORIGINAL)].insert(b3.brandCode());
  tmp[CarrierDirection(GOVERNING_CARRIER, Direction::REVERSED)].insert(b2.brandCode());
  expected.push_back(tmp);
  //segment 2
  tmp.clear();
  tmp[CarrierDirection(GOVERNING_CARRIER, Direction::ORIGINAL)].insert(b3.brandCode());
  tmp[CarrierDirection(GOVERNING_CARRIER, Direction::REVERSED)]; //no brand
  expected.push_back(tmp);
  //segment 3
  tmp.clear();
  tmp[CarrierDirection(GOVERNING_CARRIER, Direction::BOTHWAYS)]; //no brand
  expected.push_back(tmp);

  ASSERT_EQ(expected, result);
}

TEST_F(TNBrandsFunctionsTest, testThrowsOnBadFareMarket)
{
  PricingTrx trx;

  EXPECT_CALL(*_itinCalculator, getTrxGeometryCalculator())
      .WillRepeatedly(ReturnRef(*_trxCalculator));

  EXPECT_CALL(*_trxCalculator, getTrx())
      .WillRepeatedly(ReturnRef(trx));

  std::vector<MockFareMarket*> badFareMarkets;
  badFareMarkets.push_back(static_cast<MockFareMarket*>(0));
  EXPECT_CALL(*_itinCalculator, getSegmentCount()).WillOnce(Return(badFareMarkets.size()));
  EXPECT_CALL(*_itinCalculator, getFareMarkets()).WillOnce(ReturnRef(badFareMarkets));

  SegmentOrientedBrandCodesPerCarrier result;
  ASSERT_THROW(
      TNBrandsFunctions::calculateSegmentOrientedBrandCodesPerCarrier(
          *_itinCalculator, 0, result),
      ErrorResponseException);
}

TEST_F(TNBrandsFunctionsTest, testSortsBrandsPerCarrierProperly)
{
  // segments
  // 1              2
  // AA, O: A B C   AA, O: B C D
  //                AA, R: A B C
  SegmentOrientedBrandCodesPerCarrier dummyInput;
  BrandCodesPerCarrier tmp;
  tmp[CarrierDirection(GOVERNING_CARRIER, Direction::ORIGINAL)]
      .insert(_brands1.begin(), _brands1.end());
  dummyInput.push_back(tmp);
  tmp.clear();
  tmp[CarrierDirection(GOVERNING_CARRIER, Direction::ORIGINAL)]
      .insert(_brands2.begin(), _brands2.end());
  tmp[CarrierDirection(GOVERNING_CARRIER, Direction::REVERSED)]
      .insert(_brands1.begin(), _brands1.end());
  dummyInput.push_back(tmp);

  FakeBrandedFaresComparator comparator;

  SegmentOrientedBrandCodeArraysPerCarrier expected;
  BrandCodeArraysPerCarrier sorted;
  sorted[CarrierDirection(GOVERNING_CARRIER, Direction::ORIGINAL)].push_back("C");
  sorted[CarrierDirection(GOVERNING_CARRIER, Direction::ORIGINAL)].push_back("B");
  sorted[CarrierDirection(GOVERNING_CARRIER, Direction::ORIGINAL)].push_back("A");
  expected.push_back(sorted);
  sorted.clear();
  sorted[CarrierDirection(GOVERNING_CARRIER, Direction::ORIGINAL)].push_back("D");
  sorted[CarrierDirection(GOVERNING_CARRIER, Direction::ORIGINAL)].push_back("C");
  sorted[CarrierDirection(GOVERNING_CARRIER, Direction::ORIGINAL)].push_back("B");
  sorted[CarrierDirection(GOVERNING_CARRIER, Direction::REVERSED)].push_back("C");
  sorted[CarrierDirection(GOVERNING_CARRIER, Direction::REVERSED)].push_back("B");
  sorted[CarrierDirection(GOVERNING_CARRIER, Direction::REVERSED)].push_back("A");

  expected.push_back(sorted);

  SegmentOrientedBrandCodeArraysPerCarrier result;
  TNBrandsFunctions::sortSegmentOrientedBrandCodesPerCarrier(
      dummyInput, comparator, result);
  ASSERT_EQ(expected, result);
}

TEST_F(TNBrandsFunctionsTest, testSortsProperlyIfNoBrandsForCarrier)
{
  // segments
  // 1           2
  // AA: -       AA: B C D
  SegmentOrientedBrandCodesPerCarrier dummyInput;
  BrandCodesPerCarrier tmp;
  tmp[CarrierDirection(GOVERNING_CARRIER, Direction::BOTHWAYS)];
  dummyInput.push_back(tmp);
  tmp.clear();
  tmp[CarrierDirection(GOVERNING_CARRIER, Direction::BOTHWAYS)].insert(_brands2.begin(), _brands2.end());
  dummyInput.push_back(tmp);

  FakeBrandedFaresComparator comparator;

  SegmentOrientedBrandCodeArraysPerCarrier expected;
  BrandCodeArraysPerCarrier sorted;
  sorted[CarrierDirection(GOVERNING_CARRIER, Direction::BOTHWAYS)];
  expected.push_back(sorted);
  sorted.clear();
  sorted[CarrierDirection(GOVERNING_CARRIER, Direction::BOTHWAYS)].push_back("D");
  sorted[CarrierDirection(GOVERNING_CARRIER, Direction::BOTHWAYS)].push_back("C");
  sorted[CarrierDirection(GOVERNING_CARRIER, Direction::BOTHWAYS)].push_back("B");
  expected.push_back(sorted);

  SegmentOrientedBrandCodeArraysPerCarrier result;
  TNBrandsFunctions::sortSegmentOrientedBrandCodesPerCarrier(
      dummyInput, comparator, result);
  ASSERT_EQ(expected, result);
}

TEST_F(TNBrandsFunctionsTest, testCalculatesMaxBrandsCountPerCarrierProperly)
{
  SegmentOrientedBrandCodeArraysPerCarrier input;
  BrandCodeArraysPerCarrier segmentInfo;
  segmentInfo[CarrierDirection(GOVERNING_CARRIER, Direction::ORIGINAL)].push_back("A");
  segmentInfo[CarrierDirection(GOVERNING_CARRIER, Direction::ORIGINAL)].push_back("B");
  segmentInfo[CarrierDirection(GOVERNING_CARRIER, Direction::ORIGINAL)].push_back("C");
  segmentInfo[CarrierDirection(GOVERNING_CARRIER, Direction::REVERSED)].push_back("X");
  segmentInfo[CarrierDirection(GOVERNING_CARRIER, Direction::REVERSED)].push_back("Y");
  input.push_back(segmentInfo);
  segmentInfo[CarrierDirection(GOVERNING_CARRIER, Direction::REVERSED)].push_back("D");
  segmentInfo[CarrierDirection(GOVERNING_CARRIER, Direction::REVERSED)].push_back("E");
  input.push_back(segmentInfo);

  // second segmentInfo contains a carrier with 4 brands (seg 1: 3 o, 2 r; seg 2: 3 o, 4 r)
  ASSERT_EQ(4, TNBrandsFunctions::calculateMaxBrandsCountPerCarrier(input));
  ASSERT_EQ(3, TNBrandsFunctions::calculateMaxBrandsCountPerCarrier(input, 0));
  ASSERT_EQ(4, TNBrandsFunctions::calculateMaxBrandsCountPerCarrier(input, 1));
  ASSERT_EQ(0, TNBrandsFunctions::calculateMaxBrandsCountPerCarrier(input, 999));
}

TEST_F(TNBrandsFunctionsTest, testCalculatesProportionalIndexProperly)
{
  // Elements shall be distributed as below:
  // o    o
  //      o
  // ------
  // o    o
  // ------
  // o    o
  //      o
  // ------
  // o    o
  // ------
  // o    o
  assureProportionalIndex(0, 0, INPUT_SET_SIZE, OUTPUT_SET_SIZE);
  assureProportionalIndex(0, 1, INPUT_SET_SIZE, OUTPUT_SET_SIZE);
  assureProportionalIndex(1, 2, INPUT_SET_SIZE, OUTPUT_SET_SIZE);
  assureProportionalIndex(2, 3, INPUT_SET_SIZE, OUTPUT_SET_SIZE);
  assureProportionalIndex(2, 4, INPUT_SET_SIZE, OUTPUT_SET_SIZE);
  assureProportionalIndex(3, 5, INPUT_SET_SIZE, OUTPUT_SET_SIZE);
  assureProportionalIndex(4, 6, INPUT_SET_SIZE, OUTPUT_SET_SIZE);
}

TEST_F(TNBrandsFunctionsTest, testCalculatesProportionalIndexProperlyForOutputSmallerThenInput)
{
  // Elements shall be distributed as below:
  // o    o
  // ------
  // o    o
  // ------
  // o    o
  // ------
  // o    x (removed)
  // ------
  // o    o
  // ------
  // o    o
  // ------
  // o    x (removed)
  // ------
  // o    o
  // ------
  // o    o
  // ------
  // o    x (removed)
  // ------
  // o    o
  // ------
  // o    o
  // ------
  // o    x (removed)
  assureProportionalIndex(0,  0, BIG_INPUT_SET_SIZE, SMALL_OUTPUT_SET_SIZE);
  assureProportionalIndex(1,  1, BIG_INPUT_SET_SIZE, SMALL_OUTPUT_SET_SIZE);
  assureProportionalIndex(2,  2, BIG_INPUT_SET_SIZE, SMALL_OUTPUT_SET_SIZE);
  assureProportionalIndex(4,  3, BIG_INPUT_SET_SIZE, SMALL_OUTPUT_SET_SIZE);
  assureProportionalIndex(5,  4, BIG_INPUT_SET_SIZE, SMALL_OUTPUT_SET_SIZE);
  assureProportionalIndex(7,  5, BIG_INPUT_SET_SIZE, SMALL_OUTPUT_SET_SIZE);
  assureProportionalIndex(8,  6, BIG_INPUT_SET_SIZE, SMALL_OUTPUT_SET_SIZE);
  assureProportionalIndex(10, 7, BIG_INPUT_SET_SIZE, SMALL_OUTPUT_SET_SIZE);
  assureProportionalIndex(11, 8, BIG_INPUT_SET_SIZE, SMALL_OUTPUT_SET_SIZE);
}

TEST_F(TNBrandsFunctionsTest, testProportionalIndexRaisesOnInputSetSizeZero)
{
  ASSERT_THROW(TNBrandsFunctions::calculateProportionalIndex(
      0, 0, OUTPUT_SET_SIZE),
      ErrorResponseException);
}

TEST_F(TNBrandsFunctionsTest, testProportionalIndexRaisesOnOutputSetSizeZero)
{
  ASSERT_THROW(TNBrandsFunctions::calculateProportionalIndex(
      0, INPUT_SET_SIZE, 0),
      ErrorResponseException);
}

TEST_F(TNBrandsFunctionsTest, testProportionalIndexRaisesOnOutputIndexOutOfRange)
{
  ASSERT_THROW(TNBrandsFunctions::calculateProportionalIndex(
      OUTPUT_SET_SIZE, INPUT_SET_SIZE, OUTPUT_SET_SIZE),
      ErrorResponseException);
}

TEST_F(TNBrandsFunctionsTest, testCalculatesBottomPreferredIndexProperly)
{
  // Elements shall be distributed as below:
  // o    o
  // ------
  // o    o
  // ------
  // o    o
  // ------
  // o    o
  //      o
  // ------
  // o    o
  //      o
  assureBottomPreferredIndex(0, 0, INPUT_SET_SIZE, OUTPUT_SET_SIZE);
  assureBottomPreferredIndex(1, 1, INPUT_SET_SIZE, OUTPUT_SET_SIZE);
  assureBottomPreferredIndex(2, 2, INPUT_SET_SIZE, OUTPUT_SET_SIZE);
  assureBottomPreferredIndex(3, 3, INPUT_SET_SIZE, OUTPUT_SET_SIZE);
  assureBottomPreferredIndex(3, 4, INPUT_SET_SIZE, OUTPUT_SET_SIZE);
  assureBottomPreferredIndex(4, 5, INPUT_SET_SIZE, OUTPUT_SET_SIZE);
  assureBottomPreferredIndex(4, 6, INPUT_SET_SIZE, OUTPUT_SET_SIZE);
}

TEST_F(TNBrandsFunctionsTest, testCalculatesBottomPreferredIndexProperlyForOutputSmallerThenInput)
{
  // Elements shall be distributed as below:
  // o    x (removed)
  // ------
  // o    o
  // ------
  // o    o
  // ------
  // o    x (removed)
  // ------
  // o    o
  // ------
  // o    o
  // ------
  // o    x (removed)
  // ------
  // o    o
  // ------
  // o    o
  // ------
  // o    x (removed)
  // ------
  // o    o
  // ------
  // o    o
  // ------
  // o    o
  assureBottomPreferredIndex(1,  0, BIG_INPUT_SET_SIZE, SMALL_OUTPUT_SET_SIZE);
  assureBottomPreferredIndex(2,  1, BIG_INPUT_SET_SIZE, SMALL_OUTPUT_SET_SIZE);
  assureBottomPreferredIndex(4,  2, BIG_INPUT_SET_SIZE, SMALL_OUTPUT_SET_SIZE);
  assureBottomPreferredIndex(5,  3, BIG_INPUT_SET_SIZE, SMALL_OUTPUT_SET_SIZE);
  assureBottomPreferredIndex(7,  4, BIG_INPUT_SET_SIZE, SMALL_OUTPUT_SET_SIZE);
  assureBottomPreferredIndex(8,  5, BIG_INPUT_SET_SIZE, SMALL_OUTPUT_SET_SIZE);
  assureBottomPreferredIndex(10, 6, BIG_INPUT_SET_SIZE, SMALL_OUTPUT_SET_SIZE);
  assureBottomPreferredIndex(11, 7, BIG_INPUT_SET_SIZE, SMALL_OUTPUT_SET_SIZE);
  assureBottomPreferredIndex(12, 8, BIG_INPUT_SET_SIZE, SMALL_OUTPUT_SET_SIZE);
}

TEST_F(TNBrandsFunctionsTest, testBottomPreferredIndexRaisesOnInputSetSizeZero)
{
  ASSERT_THROW(TNBrandsFunctions::calculateBottomPreferredIndex(
      0, 0, OUTPUT_SET_SIZE),
      ErrorResponseException);
}

TEST_F(TNBrandsFunctionsTest, testBottomPreferredIndexRaisesOnOutputIndexOutOfRange)
{
  ASSERT_THROW(TNBrandsFunctions::calculateBottomPreferredIndex(
      OUTPUT_SET_SIZE, INPUT_SET_SIZE, OUTPUT_SET_SIZE),
      ErrorResponseException);
}

TEST_F(TNBrandsFunctionsTest, testCalculatesTopPreferredIndexProperly)
{
  // Elements shall be distributed as below:
  // o    o
  //      o
  // ------
  // o    o
  //      o
  // ------
  // o    o
  // ------
  // o    o
  // ------
  // o    o
  assureTopPreferredIndex(0, 0, INPUT_SET_SIZE, OUTPUT_SET_SIZE);
  assureTopPreferredIndex(0, 1, INPUT_SET_SIZE, OUTPUT_SET_SIZE);
  assureTopPreferredIndex(1, 2, INPUT_SET_SIZE, OUTPUT_SET_SIZE);
  assureTopPreferredIndex(1, 3, INPUT_SET_SIZE, OUTPUT_SET_SIZE);
  assureTopPreferredIndex(2, 4, INPUT_SET_SIZE, OUTPUT_SET_SIZE);
  assureTopPreferredIndex(3, 5, INPUT_SET_SIZE, OUTPUT_SET_SIZE);
  assureTopPreferredIndex(4, 6, INPUT_SET_SIZE, OUTPUT_SET_SIZE);
}

TEST_F(TNBrandsFunctionsTest, testCalculatesTopPreferredIndexProperlyForOutputSmallerThenInput)
{
  // Elements shall be distributed as below:
  // o    o
  // ------
  // o    o
  // ------
  // o    o
  // ------
  // o    x (removed)
  // ------
  // o    o
  // ------
  // o    o
  // ------
  // o    x (removed)
  // ------
  // o    o
  // ------
  // o    o
  // ------
  // o    x (removed)
  // ------
  // o    o
  // ------
  // o    o
  // ------
  // o    x (removed)
  assureTopPreferredIndex(0,  0, BIG_INPUT_SET_SIZE, SMALL_OUTPUT_SET_SIZE);
  assureTopPreferredIndex(1,  1, BIG_INPUT_SET_SIZE, SMALL_OUTPUT_SET_SIZE);
  assureTopPreferredIndex(2,  2, BIG_INPUT_SET_SIZE, SMALL_OUTPUT_SET_SIZE);
  assureTopPreferredIndex(4,  3, BIG_INPUT_SET_SIZE, SMALL_OUTPUT_SET_SIZE);
  assureTopPreferredIndex(5,  4, BIG_INPUT_SET_SIZE, SMALL_OUTPUT_SET_SIZE);
  assureTopPreferredIndex(7,  5, BIG_INPUT_SET_SIZE, SMALL_OUTPUT_SET_SIZE);
  assureTopPreferredIndex(8,  6, BIG_INPUT_SET_SIZE, SMALL_OUTPUT_SET_SIZE);
  assureTopPreferredIndex(10, 7, BIG_INPUT_SET_SIZE, SMALL_OUTPUT_SET_SIZE);
  assureTopPreferredIndex(11, 8, BIG_INPUT_SET_SIZE, SMALL_OUTPUT_SET_SIZE);
}

TEST_F(TNBrandsFunctionsTest, testNoBrandIndicatorIsInsertedIfNoBrandsForCarrier)
{
  SegmentOrientedBrandCodeArraysPerCarrier input;
  BrandCodeArraysPerCarrier segmentInfo;
  segmentInfo[CarrierDirection(GOVERNING_CARRIER, Direction::ORIGINAL)];
  segmentInfo[CarrierDirection(GOVERNING_CARRIER, Direction::REVERSED)];
  input.push_back(segmentInfo);
  segmentInfo.clear();
  segmentInfo[CarrierDirection(GOVERNING_CARRIER, Direction::BOTHWAYS)].push_back("A");
  input.push_back(segmentInfo);

  SegmentOrientedBrandCodeArraysPerCarrier expected;
  segmentInfo.clear();
  segmentInfo[CarrierDirection(GOVERNING_CARRIER, Direction::ORIGINAL)].push_back(NO_BRAND);
  segmentInfo[CarrierDirection(GOVERNING_CARRIER, Direction::REVERSED)].push_back(NO_BRAND);
  expected.push_back(segmentInfo);
  segmentInfo.clear();
  segmentInfo[CarrierDirection(GOVERNING_CARRIER, Direction::BOTHWAYS)].push_back("A");
  expected.push_back(segmentInfo);

  TNBrandsFunctions::fillEmptyBrandsArraysPerCarrier(input);
  ASSERT_EQ(expected, input);
}

TEST_F(TNBrandsFunctionsTest, testUpdateReservedBrandingOptionSpace)
{
  BrandingOptionSpaces brandingSpaces(2);
  BrandingOptionSpace& filled = brandingSpaces[1];
  filled.resize(2);
  filled[0][CarrierDirection("AA", Direction::ORIGINAL)] = "ZA";
  filled[0][CarrierDirection("UA", Direction::ORIGINAL)] = "ZU";
  filled[1][CarrierDirection("AA", Direction::ORIGINAL)] = "ZA";
  filled[1][CarrierDirection("AA", Direction::REVERSED)] = "ZA";
  filled[1][CarrierDirection("UA", Direction::BOTHWAYS)] = "ZA";

  TNBrandsFunctions::updateReservedBrandingOptionSpace(brandingSpaces);

  ASSERT_EQ(2, brandingSpaces.size());

  // Check the "reserved" non-branded option
  ASSERT_EQ(2, brandingSpaces[0].size());
  ASSERT_EQ(2, brandingSpaces[0][0].size());
  ASSERT_EQ(1, brandingSpaces[0][0].count(CarrierDirection("AA", Direction::ORIGINAL)));
  ASSERT_EQ(1, brandingSpaces[0][0].count(CarrierDirection("UA", Direction::ORIGINAL)));
  ASSERT_EQ(NO_BRAND, brandingSpaces[0][0][CarrierDirection("AA", Direction::ORIGINAL)]);
  ASSERT_EQ(NO_BRAND, brandingSpaces[0][0][CarrierDirection("UA", Direction::ORIGINAL)]);
  ASSERT_EQ(2, brandingSpaces[0][1].size());
  //AA ORIGIN+REVERSED should be "compressed" to BOTHWAYS
  ASSERT_EQ(1, brandingSpaces[0][1].count(CarrierDirection("AA", Direction::BOTHWAYS)));
  ASSERT_EQ(1, brandingSpaces[0][1].count(CarrierDirection("UA", Direction::BOTHWAYS)));
  ASSERT_EQ(NO_BRAND, brandingSpaces[0][1][CarrierDirection("AA", Direction::BOTHWAYS)]);
  ASSERT_EQ(NO_BRAND, brandingSpaces[0][1][CarrierDirection("UA", Direction::BOTHWAYS)]);

  // Check that the first branded option wasn't touched
  ASSERT_EQ(2, brandingSpaces[1][0].size());
  ASSERT_EQ(1, brandingSpaces[1][0].count(CarrierDirection("AA", Direction::ORIGINAL)));
  ASSERT_EQ(1, brandingSpaces[1][0].count(CarrierDirection("UA", Direction::ORIGINAL)));
  ASSERT_EQ("ZA", brandingSpaces[1][0][CarrierDirection("AA", Direction::ORIGINAL)]);
  ASSERT_EQ("ZU", brandingSpaces[1][0][CarrierDirection("UA", Direction::ORIGINAL)]);
  ASSERT_EQ(3, brandingSpaces[1][1].size());
  ASSERT_EQ(1, brandingSpaces[1][1].count(CarrierDirection("AA", Direction::ORIGINAL)));
  ASSERT_EQ(1, brandingSpaces[1][1].count(CarrierDirection("AA", Direction::REVERSED)));
  ASSERT_EQ(1, brandingSpaces[1][1].count(CarrierDirection("UA", Direction::BOTHWAYS)));
  ASSERT_EQ("ZA", brandingSpaces[1][1][CarrierDirection("AA", Direction::ORIGINAL)]);
  ASSERT_EQ("ZA", brandingSpaces[1][1][CarrierDirection("AA", Direction::REVERSED)]);
  ASSERT_EQ("ZA", brandingSpaces[1][1][CarrierDirection("UA", Direction::BOTHWAYS)]);
}

TEST_F(TNBrandsFunctionsTest, testUpdateReservedDeduplicatesIfNoBrands)
{
  BrandingOptionSpaces brandingSpaces(2);
  BrandingOptionSpace& filled = brandingSpaces[1];
  filled.resize(2);
  filled[0][CarrierDirection("AA", Direction::BOTHWAYS)] = NO_BRAND;
  filled[0][CarrierDirection("UA", Direction::BOTHWAYS)] = NO_BRAND;
  filled[1][CarrierDirection("AA", Direction::BOTHWAYS)] = NO_BRAND;

  TNBrandsFunctions::updateReservedBrandingOptionSpace(brandingSpaces);

  ASSERT_EQ(1, brandingSpaces.size());

  // Check the only remaining option
  ASSERT_EQ(2, brandingSpaces[0].size());
  ASSERT_EQ(2, brandingSpaces[0][0].size());
  ASSERT_EQ(1, brandingSpaces[0][0].count(CarrierDirection("AA", Direction::BOTHWAYS)));
  ASSERT_EQ(1, brandingSpaces[0][0].count(CarrierDirection("UA", Direction::BOTHWAYS)));
  ASSERT_EQ(NO_BRAND, brandingSpaces[0][0][CarrierDirection("AA", Direction::BOTHWAYS)]);
  ASSERT_EQ(NO_BRAND, brandingSpaces[0][0][CarrierDirection("UA", Direction::BOTHWAYS)]);
  ASSERT_EQ(1, brandingSpaces[0][1].size());
  ASSERT_EQ(1, brandingSpaces[0][1].count(CarrierDirection("AA", Direction::BOTHWAYS)));
  ASSERT_EQ(0, brandingSpaces[0][1].count(CarrierDirection("UA", Direction::BOTHWAYS)));
  ASSERT_EQ(NO_BRAND, brandingSpaces[0][1][CarrierDirection("AA", Direction::BOTHWAYS)]);
}

TEST_F(TNBrandsFunctionsTest, testUpdateReservedPosThrowsIfNotReserved)
{
  BrandingOptionSpaces brandingSpaces(1);
  ASSERT_THROW(TNBrandsFunctions::updateReservedBrandingOptionSpace(brandingSpaces),
               ErrorResponseException);
}

TEST_F(TNBrandsFunctionsTest, testSelectBrandsValidForCabin)
{
  TestMemHandle memHandle;
  PricingTrx* trx = memHandle.create<PricingTrx>();
  trx->setTrxType(PricingTrx::MIP_TRX);
  Itin* itin = memHandle.create<Itin>();

  fillTrxAndItinWithCabinInfo(memHandle, *trx, *itin);

  // **************************************************************
  // 1st TEST : No specific cabin requested, all brands passed
  SegmentOrientedBrandCodesPerCarrier result1, expected1;

  if (fallback::value::fallbackBrandDirectionality.get(trx))
    expected1 = {
      {
        {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {ECONOMY_BRAND, PREMIUM_BRAND}},
        {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND}}
      },
      {
        {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {ECONOMY_BRAND, BUSINESS_BRAND}},
        {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND}}
      }
    };
  else
    expected1 = {
      {
        {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND, PREMIUM_BRAND}},
        {{GOVERNING_CARRIER, Direction::REVERSED}, {PREMIUM_BRAND}}, //from thru fare
        {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND}}
      },
      {
        {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND, BUSINESS_BRAND}},
        {{GOVERNING_CARRIER, Direction::REVERSED}, {ECONOMY_BRAND, BUSINESS_BRAND}},
        {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND}}
      }
    };

  TNBrandsFunctions::selectBrandsValidForReqCabin(*trx, itin, result1, 0, false);
  ASSERT_EQ(expected1, result1);

  // 2nd TEST : Requested business on the second leg
  // all brands on 1st leg passed, only business on the second passed
  CabinType requestedCabin;
  requestedCabin.setBusinessClass();
  trx->setCabinForLeg(1, requestedCabin);

  SegmentOrientedBrandCodesPerCarrier result2, expected2;
  if (fallback::value::fallbackBrandDirectionality.get(trx))
    expected2 = {
      {
        {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {ECONOMY_BRAND, PREMIUM_BRAND}},
        {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND}}
      },
      {
        {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {BUSINESS_BRAND}}
      }
    };
    else
    expected2 = {
      {
        {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND, PREMIUM_BRAND}},
        {{GOVERNING_CARRIER, Direction::REVERSED}, {PREMIUM_BRAND}}, //from thru fare
        {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND}}
      },
      {
        {{GOVERNING_CARRIER, Direction::ORIGINAL}, {BUSINESS_BRAND}},
        {{GOVERNING_CARRIER, Direction::REVERSED}, {BUSINESS_BRAND}}
      }
    };

  TNBrandsFunctions::selectBrandsValidForReqCabin(*trx, itin, result2, 0, false);
  ASSERT_EQ(expected2, result2);

  // 3rd TEST : Requested premium on both legs,
  // premium passed on 1st leg, nothing left on the second leg
  requestedCabin.setFirstClass();
  trx->setCabinForLeg(0, requestedCabin);
  trx->setCabinForLeg(1, requestedCabin);

  SegmentOrientedBrandCodesPerCarrier result3, expected3;

  if (fallback::value::fallbackBrandDirectionality.get(trx))
    expected3 = {
    {
      {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {PREMIUM_BRAND}},
    },
    {}
  };
  else
    expected3 = {
      {
        {{GOVERNING_CARRIER, Direction::ORIGINAL}, {PREMIUM_BRAND}},
        {{GOVERNING_CARRIER, Direction::REVERSED}, {PREMIUM_BRAND}}, //from thru fare
      },
      {}
    };

  TNBrandsFunctions::selectBrandsValidForReqCabin(*trx, itin, result3, 0, false);
  ASSERT_EQ(expected3, result3);
}

TEST_F(TNBrandsFunctionsTest, testSelectBrandsValidForAnyCabinNoJumpCabin)
{
  TestMemHandle memHandle;
  PricingTrx* trx = memHandle.create<PricingTrx>();
  trx->setTrxType(PricingTrx::MIP_TRX);
  Itin* itin = memHandle.create<Itin>();

  fillTrxAndItinWithCabinInfo(memHandle, *trx, *itin);

  // No specific cabin requested, all brands passed
  SegmentOrientedBrandCodesPerCarrier result1, expected1;
  if (fallback::value::fallbackBrandDirectionality.get(trx))
    expected1 = {
      {
        {{GOVERNING_CARRIER, Direction::BOTHWAYS} , {ECONOMY_BRAND, PREMIUM_BRAND}},
        {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND}}
      },
      {
        {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {ECONOMY_BRAND, BUSINESS_BRAND}},
        {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND}}
      }
    };
  else
    expected1 = {
      {
        {{GOVERNING_CARRIER, Direction::ORIGINAL} , {ECONOMY_BRAND, PREMIUM_BRAND}},
        {{GOVERNING_CARRIER, Direction::REVERSED}, {PREMIUM_BRAND}}, //from thru fare
        {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND}}
      },
      {
        {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND, BUSINESS_BRAND}},
        {{GOVERNING_CARRIER, Direction::REVERSED}, {ECONOMY_BRAND, BUSINESS_BRAND}},
        {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND}}
      }
    };

  TNBrandsFunctions::selectBrandsValidForReqCabin(*trx, itin, result1, 0, true);
  ASSERT_EQ(expected1, result1);
}

TEST_F(TNBrandsFunctionsTest, testSelectBrandsValidForEconomyNoJumpCabin1)
{
  TestMemHandle memHandle;
  PricingTrx* trx = memHandle.create<PricingTrx>();
  trx->setTrxType(PricingTrx::MIP_TRX);
  Itin* itin = memHandle.create<Itin>();

  fillTrxAndItinWithCabinInfo(memHandle, *trx, *itin);

  // Requested economy on the second leg
  // only economy brand on 1st leg passed, all brands on 2nd leg passed
  CabinType requestedCabin;
  requestedCabin.setEconomyClass();
  trx->setCabinForLeg(0, requestedCabin);

  SegmentOrientedBrandCodesPerCarrier result2, expected2;
  if (fallback::value::fallbackBrandDirectionality.get(trx))
    expected2 = {
      {
        {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {ECONOMY_BRAND}},
        {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND}}
      },
      {
        {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {ECONOMY_BRAND, BUSINESS_BRAND}},
        {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND}}
      }
    };
  else
    expected2 = {
      {
        {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND}},
        {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND}}
      },
      {
        {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND, BUSINESS_BRAND}},
        {{GOVERNING_CARRIER, Direction::REVERSED}, {ECONOMY_BRAND, BUSINESS_BRAND}},
        {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND}}
      }
    };

  TNBrandsFunctions::selectBrandsValidForReqCabin(*trx, itin, result2, 0, true);
  ASSERT_EQ(expected2, result2);
}

TEST_F(TNBrandsFunctionsTest, testSelectBrandsValidForEconomyNoJumpCabin2)
{
  TestMemHandle memHandle;
  PricingTrx* trx = memHandle.create<PricingTrx>();
  trx->setTrxType(PricingTrx::MIP_TRX);
  Itin* itin = memHandle.create<Itin>();

  fillTrxAndItinWithCabinInfo(memHandle, *trx, *itin);

  // Requested economy on the second leg
  // all brands on 1st leg passed, only economy on the second passed
  CabinType requestedCabin;
  requestedCabin.setEconomyClass();
  trx->setCabinForLeg(1, requestedCabin);

  SegmentOrientedBrandCodesPerCarrier result2, expected2;
  if (fallback::value::fallbackBrandDirectionality.get(trx))
    expected2 = {
      {
        {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {ECONOMY_BRAND, PREMIUM_BRAND}},
        {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND}}
      },
      {
        {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {ECONOMY_BRAND}},
        {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND}}
      }
    };
  else
    expected2 = {
      {
        {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND, PREMIUM_BRAND}},
        {{GOVERNING_CARRIER, Direction::REVERSED}, {PREMIUM_BRAND}},
        {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND}}
      },
      {
        {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND}},
        {{GOVERNING_CARRIER, Direction::REVERSED}, {ECONOMY_BRAND}},
        {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND}}
      }
    };

  TNBrandsFunctions::selectBrandsValidForReqCabin(*trx, itin, result2, 0, true);
  ASSERT_EQ(expected2, result2);
}

TEST_F(TNBrandsFunctionsTest, testSelectBrandsValidForBusinessNoJumpCabin)
{
  TestMemHandle memHandle;
  PricingTrx* trx = memHandle.create<PricingTrx>();
  trx->setTrxType(PricingTrx::MIP_TRX);
  Itin* itin = memHandle.create<Itin>();

  fillTrxAndItinWithCabinInfo(memHandle, *trx, *itin);

  // Requested business on the second leg
  // all brands on 1st leg passed, only business on the second passed
  CabinType requestedCabin;
  requestedCabin.setBusinessClass();
  trx->setCabinForLeg(1, requestedCabin);

  SegmentOrientedBrandCodesPerCarrier result2, expected2;
  if (fallback::value::fallbackBrandDirectionality.get(trx))
    expected2 = {
      {
        {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {ECONOMY_BRAND, PREMIUM_BRAND}},
        {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND}}
      },
      {
        {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {BUSINESS_BRAND}}
      }
    };
  else
    expected2 = {
      {
        {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND, PREMIUM_BRAND}},
        {{GOVERNING_CARRIER, Direction::REVERSED}, {PREMIUM_BRAND}},
        {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND}}
      },
      {
        {{GOVERNING_CARRIER, Direction::ORIGINAL}, {BUSINESS_BRAND}},
        {{GOVERNING_CARRIER, Direction::REVERSED}, {BUSINESS_BRAND}}
      }
    };

  TNBrandsFunctions::selectBrandsValidForReqCabin(*trx, itin, result2, 0, true);
  ASSERT_EQ(expected2, result2);
}

TEST_F(TNBrandsFunctionsTest, testSelectBrandsValidForFirstNoJumpCabin)
{
  TestMemHandle memHandle;
  PricingTrx* trx = memHandle.create<PricingTrx>();
  trx->setTrxType(PricingTrx::MIP_TRX);
  Itin* itin = memHandle.create<Itin>();

  fillTrxAndItinWithCabinInfo(memHandle, *trx, *itin);

  // Requested premium on both legs,
  // premium pased on 1st leg, nothing left on the second leg
  CabinType requestedCabin;
  requestedCabin.setFirstClass();
  trx->setCabinForLeg(0, requestedCabin);
  trx->setCabinForLeg(1, requestedCabin);

  SegmentOrientedBrandCodesPerCarrier result3, expected3;
  if (fallback::value::fallbackBrandDirectionality.get(trx))
    expected3 = {
      {
        {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {PREMIUM_BRAND}}
      },
      {}
    };
  else
    expected3 = {
      {
        {{GOVERNING_CARRIER, Direction::ORIGINAL}, {PREMIUM_BRAND}},
        {{GOVERNING_CARRIER, Direction::REVERSED}, {PREMIUM_BRAND}}
      },
      {}
    };

  TNBrandsFunctions::selectBrandsValidForReqCabin(*trx, itin, result3, 0, true);
  ASSERT_EQ(expected3, result3);
}

TEST_F(TNBrandsFunctionsTest, testProperlyBuildBrandsPerCabinStructure)
{
  TestMemHandle memHandle;
  PricingTrx* trx = memHandle.create<PricingTrx>();
  trx->setTrxType(PricingTrx::MIP_TRX);
  fallback::value::fallbackBrandDirectionality.set(true);
  Itin* itin = memHandle.create<Itin>();

  fillTrxAndItinWithCabinInfo(memHandle, *trx, *itin, true);

  SegmentOrientedBrandCodesPerCarrierInCabin resultEconomy, resultBusiness, resultFirst;
  SegmentOrientedBrandCodesPerCarrierInCabin expectedEconomy, expectedBusiness, expectedFirst;

  expectedEconomy = {
    {{CabinType::generalIndex(CabinType::ECONOMY_CLASS)},
      {
        {
          {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {ECONOMY_BRAND}},
          {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND}}
        },
        {
          {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {ECONOMY_BRAND}},
          {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND}},
        },
      }
    },
    {{CabinType::generalIndex(CabinType::BUSINESS_CLASS)},
      {
        {{{GOVERNING_CARRIER, Direction::BOTHWAYS}, {BUSINESS_BRAND}}},
        {{{GOVERNING_CARRIER, Direction::BOTHWAYS}, {BUSINESS_BRAND}}},
      }
    },
    {{CabinType::generalIndex(CabinType::FIRST_CLASS)},
      {
        {},
        {}
      }
    }
  };

  expectedBusiness = {
    {{CabinType::generalIndex(CabinType::BUSINESS_CLASS)},
      {
        {
          {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {BUSINESS_BRAND}},
          {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {}}
        },
        {
          {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {BUSINESS_BRAND}},
          {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {}}
        },
      }
    },
    {{CabinType::generalIndex(CabinType::FIRST_CLASS)},
      {
        {},
        {},
      }
    }
  };

  expectedFirst = {
    {{CabinType::generalIndex(CabinType::FIRST_CLASS)},
      {
        {
          {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {}},
          {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {}}
        },
        {
          {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {}},
          {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {}}
        },
      }
    },
  };

  TNBrandsFunctions::selectBrandsPerCabin(*trx, itin,
    CabinType::generalIndex(CabinType::ECONOMY_CLASS), resultEconomy, 0, false);
  ASSERT_EQ(expectedEconomy, resultEconomy);

  TNBrandsFunctions::selectBrandsPerCabin(*trx, itin,
    CabinType::generalIndex(CabinType::BUSINESS_CLASS), resultBusiness, 0, false);
  ASSERT_EQ(expectedBusiness, resultBusiness);

  TNBrandsFunctions::selectBrandsPerCabin(*trx, itin,
    CabinType::generalIndex(CabinType::FIRST_CLASS), resultFirst, 0, false);
  ASSERT_EQ(expectedFirst, resultFirst);
}

TEST_F(TNBrandsFunctionsTest, testProperlyBuildBrandsPerCabinStructureWithDirectionality)
{
  TestMemHandle memHandle;
  PricingTrx* trx = memHandle.create<PricingTrx>();
  trx->setTrxType(PricingTrx::MIP_TRX);
  fallback::value::fallbackBrandDirectionality.set(false);
  Itin* itin = memHandle.create<Itin>();

  fillTrxAndItinWithCabinInfo(memHandle, *trx, *itin, true);

  SegmentOrientedBrandCodesPerCarrierInCabin resultEconomy, resultBusiness, resultFirst;
  SegmentOrientedBrandCodesPerCarrierInCabin expectedEconomy, expectedBusiness, expectedFirst;

  expectedEconomy = {
    {{CabinType::generalIndex(CabinType::ECONOMY_CLASS)},
      {
        {
          {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND}},
          {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND}},
        },
        {
          {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND}},
          {{GOVERNING_CARRIER, Direction::REVERSED}, {ECONOMY_BRAND}},
          {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND}},
        },
      }
    },
    {{CabinType::generalIndex(CabinType::BUSINESS_CLASS)},
      {
        {
          {{GOVERNING_CARRIER, Direction::ORIGINAL}, {BUSINESS_BRAND}},
          {{GOVERNING_CARRIER, Direction::REVERSED}, {BUSINESS_BRAND}}
        },
        {
          {{GOVERNING_CARRIER, Direction::ORIGINAL}, {BUSINESS_BRAND}},
          {{GOVERNING_CARRIER, Direction::REVERSED}, {BUSINESS_BRAND}}
        },
      }
    },
    {{CabinType::generalIndex(CabinType::FIRST_CLASS)},
      {
        {},
        {}
      }
    }
  };

  expectedBusiness = {
    {{CabinType::generalIndex(CabinType::BUSINESS_CLASS)},
      {
        {
          {{GOVERNING_CARRIER, Direction::ORIGINAL}, {BUSINESS_BRAND}},
          {{GOVERNING_CARRIER, Direction::REVERSED}, {BUSINESS_BRAND}},
          {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {}}
        },
        {
          {{GOVERNING_CARRIER, Direction::ORIGINAL}, {BUSINESS_BRAND}},
          {{GOVERNING_CARRIER, Direction::REVERSED}, {BUSINESS_BRAND}},
          {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {}}
        },
      }
    },
    {{CabinType::generalIndex(CabinType::FIRST_CLASS)},
      {
        {},
        {},
      }
    }
  };

  expectedFirst = {
    {{CabinType::generalIndex(CabinType::FIRST_CLASS)},
      {
        {
          {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {}},
          {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {}}
        },
        {
          {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {}},
          {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {}}
        },
      }
    },
  };

  TNBrandsFunctions::selectBrandsPerCabin(*trx, itin,
    CabinType::generalIndex(CabinType::ECONOMY_CLASS), resultEconomy, 0, false);

  ASSERT_EQ(expectedEconomy, resultEconomy);

  TNBrandsFunctions::selectBrandsPerCabin(*trx, itin,
    CabinType::generalIndex(CabinType::BUSINESS_CLASS), resultBusiness, 0, false);
  ASSERT_EQ(expectedBusiness, resultBusiness);

  TNBrandsFunctions::selectBrandsPerCabin(*trx, itin,
    CabinType::generalIndex(CabinType::FIRST_CLASS), resultFirst, 0, false);
  ASSERT_EQ(expectedFirst, resultFirst);
}

TEST_F(TNBrandsFunctionsTest, testProperlyBuildBrandsPerCabinStructureEconomyNoJumpCabin)
{
  TestMemHandle memHandle;
  PricingTrx* trx = memHandle.create<PricingTrx>();
  trx->setTrxType(PricingTrx::MIP_TRX);
  fallback::value::fallbackBrandDirectionality.set(true);
  Itin* itin = memHandle.create<Itin>();

  fillTrxAndItinWithCabinInfo(memHandle, *trx, *itin, true);

  SegmentOrientedBrandCodesPerCarrierInCabin resultEconomy, expected = {
    {{CabinType::generalIndex(CabinType::ECONOMY_CLASS)},
      {
        {
          {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {ECONOMY_BRAND}},
          {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND}}
        },
        {
          {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {ECONOMY_BRAND}},
          {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND}}
        },
      }
    }
  };

  TNBrandsFunctions::selectBrandsPerCabin(*trx, itin,
    CabinType::generalIndex(CabinType::ECONOMY_CLASS), resultEconomy, 0, true);
  ASSERT_EQ(expected, resultEconomy);
}

TEST_F(TNBrandsFunctionsTest, testProperlyBuildBrandsPerCabinStructureEconomyNoJumpCabinWithDirectionality)
{
  TestMemHandle memHandle;
  PricingTrx* trx = memHandle.create<PricingTrx>();
  trx->setTrxType(PricingTrx::MIP_TRX);
  fallback::value::fallbackBrandDirectionality.set(false);
  Itin* itin = memHandle.create<Itin>();

  fillTrxAndItinWithCabinInfo(memHandle, *trx, *itin, true);

  SegmentOrientedBrandCodesPerCarrierInCabin resultEconomy, expected = {
    {{CabinType::generalIndex(CabinType::ECONOMY_CLASS)},
      {
        {
          {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND}},
          {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND}}
        },
        {
          {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND}},
          {{GOVERNING_CARRIER, Direction::REVERSED}, {ECONOMY_BRAND}},
          {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND}}
        },
      }
    }
  };

  TNBrandsFunctions::selectBrandsPerCabin(*trx, itin,
    CabinType::generalIndex(CabinType::ECONOMY_CLASS), resultEconomy, 0, true);
  ASSERT_EQ(expected, resultEconomy);
}

TEST_F(TNBrandsFunctionsTest, testProperlyBuildBrandsPerCabinStructureBusinessNoJumpCabin)
{
  TestMemHandle memHandle;
  PricingTrx* trx = memHandle.create<PricingTrx>();
  trx->setTrxType(PricingTrx::MIP_TRX);
  fallback::value::fallbackBrandDirectionality.set(true);
  Itin* itin = memHandle.create<Itin>();

  fillTrxAndItinWithCabinInfo(memHandle, *trx, *itin, true);

  SegmentOrientedBrandCodesPerCarrierInCabin resultBusiness, expected = {
    {{CabinType::generalIndex(CabinType::BUSINESS_CLASS)},
      {
        {
          {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {BUSINESS_BRAND}},
          {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {}}
        },
        {
          {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {BUSINESS_BRAND}},
          {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {}}
        },
      }
    }
  };

  TNBrandsFunctions::selectBrandsPerCabin(*trx, itin,
    CabinType::generalIndex(CabinType::BUSINESS_CLASS), resultBusiness, 0, true);
  ASSERT_EQ(expected, resultBusiness);
}

TEST_F(TNBrandsFunctionsTest, testProperlyBuildBrandsPerCabinStructureBusinessNoJumpCabinWithDirectionality)
{
  TestMemHandle memHandle;
  PricingTrx* trx = memHandle.create<PricingTrx>();
  trx->setTrxType(PricingTrx::MIP_TRX);
  fallback::value::fallbackBrandDirectionality.set(false);
  Itin* itin = memHandle.create<Itin>();

  fillTrxAndItinWithCabinInfo(memHandle, *trx, *itin, true);

  SegmentOrientedBrandCodesPerCarrierInCabin resultBusiness, expected = {
    {{CabinType::generalIndex(CabinType::BUSINESS_CLASS)},
      {
        {
          {{GOVERNING_CARRIER, Direction::ORIGINAL}, {BUSINESS_BRAND}},
          {{GOVERNING_CARRIER, Direction::REVERSED}, {BUSINESS_BRAND}},
          {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {}}
        },
        {
          {{GOVERNING_CARRIER, Direction::ORIGINAL}, {BUSINESS_BRAND}},
          {{GOVERNING_CARRIER, Direction::REVERSED}, {BUSINESS_BRAND}},
          {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {}}
        },
      }
    }
  };

  TNBrandsFunctions::selectBrandsPerCabin(*trx, itin,
    CabinType::generalIndex(CabinType::BUSINESS_CLASS), resultBusiness, 0, true);
  ASSERT_EQ(expected, resultBusiness);
}

TEST_F(TNBrandsFunctionsTest, testProperlyBuildBrandsPerCabinStructureFirstNoJumpCabin)
{
  TestMemHandle memHandle;
  PricingTrx* trx = memHandle.create<PricingTrx>();
  trx->setTrxType(PricingTrx::MIP_TRX);
  Itin* itin = memHandle.create<Itin>();

  fillTrxAndItinWithCabinInfo(memHandle, *trx, *itin, true);

  SegmentOrientedBrandCodesPerCarrierInCabin resultFirst, expected = {
    {{CabinType::generalIndex(CabinType::FIRST_CLASS)},
      {
        {
          {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {}},
          {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {}}
        },
        {
          {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {}},
          {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {}}
        },
      }
    }
  };

  TNBrandsFunctions::selectBrandsPerCabin(*trx, itin,
    CabinType::generalIndex(CabinType::FIRST_CLASS), resultFirst, 0, true);
  ASSERT_EQ(expected, resultFirst);
}

TEST_F(TNBrandsFunctionsTest, testProperlyBuildBrandsPerCabinStructureFirstNoJumpCabinWithDirectionality)
{
  TestMemHandle memHandle;
  PricingTrx* trx = memHandle.create<PricingTrx>();
  trx->setTrxType(PricingTrx::MIP_TRX);
  fallback::value::fallbackBrandDirectionality.set(false);
  Itin* itin = memHandle.create<Itin>();

  fillTrxAndItinWithCabinInfo(memHandle, *trx, *itin, true);

  SegmentOrientedBrandCodesPerCarrierInCabin resultFirst, expected = {
    {{CabinType::generalIndex(CabinType::FIRST_CLASS)},
      {
        {
          {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {}},
          {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {}}
        },
        {
          {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {}},
          {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {}}
        },
      }
    }
  };

  TNBrandsFunctions::selectBrandsPerCabin(*trx, itin,
    CabinType::generalIndex(CabinType::FIRST_CLASS), resultFirst, 0, true);
  ASSERT_EQ(expected, resultFirst);
}

TEST_F(TNBrandsFunctionsTest, testRaisesOnUnknownCabinIndex)
{
  TestMemHandle memHandle;
  PricingTrx* trx = memHandle.create<PricingTrx>();
  trx->setTrxType(PricingTrx::MIP_TRX);
  Itin* itin = memHandle.create<Itin>();

  fillTrxAndItinWithCabinInfo(memHandle, *trx, *itin);

  SegmentOrientedBrandCodesPerCarrierInCabin result;

  ASSERT_THROW(TNBrandsFunctions::selectBrandsPerCabin(*trx, itin,
      CabinType::generalIndex(CabinType::UNKNOWN_CLASS), result, 0, false),
    ErrorResponseException);
}

TEST_F(TNBrandsFunctionsTest, testProperlyFiltersBrandsPerCabin)
{
  SegmentOrientedBrandCodesPerCarrierInCabin brandsPerCarrierByCabin;
  SegmentOrientedBrandCodeArraysPerCarrier validSortedBrands, dedupicatedBrands;
  SegmentOrientedBrandCodeArraysPerCarrier result, expected, expectedDedupicatedBrands;

  //Building input data structure:
  //Economy:  Seg 0: AA, O -> EC
  //          Seg 1: AA, O -> EC
  //                 AA, R -> EC
  //Business: Seg 0: AA, O -> BZ PM
  //          Seg 1: AA, O -> BZ
  //               : AA, R -> BZ
  //Premium:  Seg 0: AA, B -> PM
  //          Seg 1: AA, B -> PM

  brandsPerCarrierByCabin = {
    {{CabinType::generalIndex(CabinType::ECONOMY_CLASS)},
      {
        {{{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND}}},
        {
          {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND}},
          {{GOVERNING_CARRIER, Direction::REVERSED}, {ECONOMY_BRAND}}
        }
      }
    },
    {{CabinType::generalIndex(CabinType::BUSINESS_CLASS)},
      {
        {{{GOVERNING_CARRIER, Direction::ORIGINAL}, {BUSINESS_BRAND, PREMIUM_BRAND}}},
        {
          {{GOVERNING_CARRIER, Direction::ORIGINAL}, {BUSINESS_BRAND}},
          {{GOVERNING_CARRIER, Direction::REVERSED}, {BUSINESS_BRAND}}
        },
      }
    },
    {{CabinType::generalIndex(CabinType::FIRST_CLASS)},
      {
        {{{GOVERNING_CARRIER, Direction::BOTHWAYS}, {PREMIUM_BRAND}}},
        {{{GOVERNING_CARRIER, Direction::BOTHWAYS}, {PREMIUM_BRAND}}}
      }
    }
  };

  //Building carrier/brands data for validation/filtering.
  //All brands available on both segments.
  //Seg 0: AA, O -> EC BZ
  //       AA, B -> PM
  //Seg 1: AA, B -> EC BZ PM
  validSortedBrands = {
    {
      {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND, BUSINESS_BRAND}},
      {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {PREMIUM_BRAND}}
    },
    {
      {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND, BUSINESS_BRAND}},
      {{GOVERNING_CARRIER, Direction::REVERSED}, {ECONOMY_BRAND, BUSINESS_BRAND}},
      {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {PREMIUM_BRAND}}
    }
  };
  dedupicatedBrands = validSortedBrands;

  //Building expected data for economy class
  //deduplicated
  expectedDedupicatedBrands = {
    {
      {{GOVERNING_CARRIER, Direction::ORIGINAL}, {BUSINESS_BRAND}},
      {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {PREMIUM_BRAND}}
    },
    {
      {{GOVERNING_CARRIER, Direction::ORIGINAL}, {BUSINESS_BRAND}},
      {{GOVERNING_CARRIER, Direction::REVERSED}, {BUSINESS_BRAND}},
      {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {PREMIUM_BRAND}},
    }
  };
  //splited
  expected = {
    {{{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND}}},
    {
      {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND}},
      {{GOVERNING_CARRIER, Direction::REVERSED}, {ECONOMY_BRAND}}
    }
  };

  ASSERT_TRUE(
    TNBrandsFunctions::filterSortedSegmentOrientedBrandCodesPerCarrierByCabin(
      brandsPerCarrierByCabin[CabinType::generalIndex(CabinType::ECONOMY_CLASS)],
      validSortedBrands, dedupicatedBrands, result, false));

  ASSERT_EQ(expectedDedupicatedBrands, dedupicatedBrands);
  ASSERT_EQ(expected, result);



  //Building expected data for business class
  //deduplicated
  expectedDedupicatedBrands = {
    {
      {{GOVERNING_CARRIER, Direction::ORIGINAL}, {}},
      {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {PREMIUM_BRAND}}
    },
    {
      {{GOVERNING_CARRIER, Direction::ORIGINAL}, {}},
      {{GOVERNING_CARRIER, Direction::REVERSED}, {}},
      {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {PREMIUM_BRAND}}
    },
  };
  //splited
  expected = {
    {{{GOVERNING_CARRIER, Direction::ORIGINAL}, {BUSINESS_BRAND}}},
    {
      {{GOVERNING_CARRIER, Direction::ORIGINAL}, {BUSINESS_BRAND}},
      {{GOVERNING_CARRIER, Direction::REVERSED}, {BUSINESS_BRAND}}
    }
  };
  result.clear();

  ASSERT_TRUE(
    TNBrandsFunctions::filterSortedSegmentOrientedBrandCodesPerCarrierByCabin(
      brandsPerCarrierByCabin[CabinType::generalIndex(CabinType::BUSINESS_CLASS)],
      validSortedBrands, dedupicatedBrands, result, false));

  ASSERT_EQ(expectedDedupicatedBrands, dedupicatedBrands);
  ASSERT_EQ(expected, result);



  //Building expected data for first class
  //deduplicated
  expectedDedupicatedBrands = {
    {
      {{GOVERNING_CARRIER, Direction::ORIGINAL}, {}},
      {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {}},
    },
    {
      {{GOVERNING_CARRIER, Direction::ORIGINAL}, {}},
      {{GOVERNING_CARRIER, Direction::REVERSED}, {}},
      {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {}}
    },
  };
  //splited
  expected = {
    {
      {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {PREMIUM_BRAND}}
    },
    {
      {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {PREMIUM_BRAND}}
    },
  };
  result.clear();

  ASSERT_TRUE(
    TNBrandsFunctions::filterSortedSegmentOrientedBrandCodesPerCarrierByCabin(
      brandsPerCarrierByCabin[CabinType::generalIndex(CabinType::FIRST_CLASS)],
      validSortedBrands, dedupicatedBrands, result, false));

  ASSERT_EQ(expectedDedupicatedBrands, dedupicatedBrands);
  ASSERT_EQ(expected, result);
}

TEST_F(TNBrandsFunctionsTest, testProperlyFiltersBrandsPerCabinWithDirectionality)
{
  SegmentOrientedBrandCodesPerCarrierInCabin brandsPerCarrierByCabin;
  SegmentOrientedBrandCodeArraysPerCarrier validSortedBrands, dedupicatedBrands;
  SegmentOrientedBrandCodeArraysPerCarrier result, expectedDedupicatedBrands;

  //Building input data structure:
  //Economy:  Seg 0: AA, O -> EC
  //          Seg 1: AA, O -> EC
  //                 AA, R -> EC
  //Business: Seg 0: AA, O -> BZ PM
  //          Seg 1: AA, O -> BZ
  //               : AA, R -> BZ
  //Premium:  Seg 0: AA, B -> PM
  //          Seg 1: AA, B -> PM

  brandsPerCarrierByCabin = {
    {{CabinType::generalIndex(CabinType::ECONOMY_CLASS)},
      {
        {{{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND}}},
        {
          {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND}},
          {{GOVERNING_CARRIER, Direction::REVERSED}, {ECONOMY_BRAND}}
        }
      }
    },
    {{CabinType::generalIndex(CabinType::BUSINESS_CLASS)},
      {
        {{{GOVERNING_CARRIER, Direction::ORIGINAL}, {BUSINESS_BRAND, PREMIUM_BRAND}}},
        {
          {{GOVERNING_CARRIER, Direction::ORIGINAL}, {BUSINESS_BRAND}},
          {{GOVERNING_CARRIER, Direction::REVERSED}, {BUSINESS_BRAND}}
        },
      }
    },
    {{CabinType::generalIndex(CabinType::FIRST_CLASS)},
      {
        {{{GOVERNING_CARRIER, Direction::BOTHWAYS}, {PREMIUM_BRAND}}},
        {{{GOVERNING_CARRIER, Direction::BOTHWAYS}, {PREMIUM_BRAND}}}
      }
    }
  };

  //Building carrier/brands data for validation/filtering.
  //All brands available on both segments.
  //Seg 0: AA, O -> EC BZ
  //       AA, B -> PM
  //Seg 1: AA, B -> EC BZ PM
  validSortedBrands = {
    {
      {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND, BUSINESS_BRAND}},
      {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {PREMIUM_BRAND}}
    },
    {
      {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND, BUSINESS_BRAND}},
      {{GOVERNING_CARRIER, Direction::REVERSED}, {ECONOMY_BRAND, BUSINESS_BRAND}},
      {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {PREMIUM_BRAND}}
    }
  };
  dedupicatedBrands = validSortedBrands;

  //Building expected data for economy class
  //deduplicated
  expectedDedupicatedBrands = {
    {
      {{GOVERNING_CARRIER, Direction::ORIGINAL}, {BUSINESS_BRAND}},
      {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {PREMIUM_BRAND}}
    },
    {
      {{GOVERNING_CARRIER, Direction::ORIGINAL}, {BUSINESS_BRAND}},
      {{GOVERNING_CARRIER, Direction::REVERSED}, {BUSINESS_BRAND}},
      {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {PREMIUM_BRAND}},
    }
  };
  //splited
  SegmentOrientedBrandCodeArraysPerCarrier expectedEco = {
    {{{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND}}},
    {
      {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND}},
      {{GOVERNING_CARRIER, Direction::REVERSED}, {ECONOMY_BRAND}}
    }
  };

  ASSERT_TRUE(
    TNBrandsFunctions::filterSortedSegmentOrientedBrandCodesPerCarrierByCabin(
      brandsPerCarrierByCabin[CabinType::generalIndex(CabinType::ECONOMY_CLASS)],
      validSortedBrands, dedupicatedBrands, result, true));

  ASSERT_EQ(expectedDedupicatedBrands, dedupicatedBrands);
  ASSERT_EQ(expectedEco, result);



  //Building expected data for business class
  //deduplicated
  expectedDedupicatedBrands = {
    {
      {{GOVERNING_CARRIER, Direction::ORIGINAL}, {}},
      {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {PREMIUM_BRAND}}
    },
    {
      {{GOVERNING_CARRIER, Direction::ORIGINAL}, {}},
      {{GOVERNING_CARRIER, Direction::REVERSED}, {}},
      {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {PREMIUM_BRAND}}
    },
  };
  //splited
  SegmentOrientedBrandCodeArraysPerCarrier expectedBus = {
    {{{GOVERNING_CARRIER, Direction::ORIGINAL}, {BUSINESS_BRAND}}},
    {
      {{GOVERNING_CARRIER, Direction::ORIGINAL}, {BUSINESS_BRAND}},
      {{GOVERNING_CARRIER, Direction::REVERSED}, {BUSINESS_BRAND}}
    }
  };
  result.clear();

  ASSERT_TRUE(
    TNBrandsFunctions::filterSortedSegmentOrientedBrandCodesPerCarrierByCabin(
      brandsPerCarrierByCabin[CabinType::generalIndex(CabinType::BUSINESS_CLASS)],
      validSortedBrands, dedupicatedBrands, result, true));

  ASSERT_EQ(expectedDedupicatedBrands, dedupicatedBrands);
  ASSERT_EQ(expectedBus, result);



  //Building expected data for first class
  //deduplicated
  expectedDedupicatedBrands = {
    {
      {{GOVERNING_CARRIER, Direction::ORIGINAL}, {}},
      {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {}},
    },
    {
      {{GOVERNING_CARRIER, Direction::ORIGINAL}, {}},
      {{GOVERNING_CARRIER, Direction::REVERSED}, {}},
      {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {}}
    },
  };
  //splited
  SegmentOrientedBrandCodeArraysPerCarrier expectedPre = {
    {
      {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {PREMIUM_BRAND}}
    },
    {
      {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {PREMIUM_BRAND}}
    },
  };
  result.clear();

  ASSERT_TRUE(
    TNBrandsFunctions::filterSortedSegmentOrientedBrandCodesPerCarrierByCabin(
      brandsPerCarrierByCabin[CabinType::generalIndex(CabinType::FIRST_CLASS)],
      validSortedBrands, dedupicatedBrands, result, true));

  ASSERT_EQ(expectedDedupicatedBrands, dedupicatedBrands);
  ASSERT_EQ(expectedPre, result);
}

TEST_F(TNBrandsFunctionsTest, testProperlyFiltersBrandsPerCabinAtSegmentForInvalidCarrier)
{
  BrandCodesPerCarrier carrierBrandMap = {
    {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND, BUSINESS_BRAND, PREMIUM_BRAND}},
    {{GOVERNING_CARRIER, Direction::REVERSED}, {ECONOMY_BRAND}},
    {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND, BUSINESS_BRAND, PREMIUM_BRAND}}
  };
  BrandCodeArraysPerCarrier brandDeduplication = {
    {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND, BUSINESS_BRAND, PREMIUM_BRAND}},
    {{GOVERNING_CARRIER, Direction::REVERSED}, {ECONOMY_BRAND}},
    {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND, BUSINESS_BRAND, PREMIUM_BRAND}}
  };
  BrandCodeArraysPerCarrier expectedBrandDeduplication = brandDeduplication;

  BrandCodeArraysPerCarrier result, expected;
  expected = {};

  //TODO(andrzej.fediuk) DIR: add test for proper carrier with improper direction
  CarrierDirection otherCarrier("XX", Direction::BOTHWAYS);

  ASSERT_FALSE(
    TNBrandsFunctions::filterCarrierBrandCodesByCabinAtSegment(
      carrierBrandMap, otherCarrier, brandDeduplication,
      result, false));

  ASSERT_EQ(expectedBrandDeduplication, brandDeduplication);
  ASSERT_EQ(expected, result);
}

TEST_F(TNBrandsFunctionsTest, testProperlyFiltersBrandsPerCabinAtSegmentForInvalidCarrierWithDirectionality)
{
  BrandCodesPerCarrier carrierBrandMap = {
    {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND, BUSINESS_BRAND, PREMIUM_BRAND}},
    {{GOVERNING_CARRIER, Direction::REVERSED}, {ECONOMY_BRAND}},
    {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND, BUSINESS_BRAND, PREMIUM_BRAND}}
  };
  BrandCodeArraysPerCarrier brandDeduplication = {
    {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND, BUSINESS_BRAND, PREMIUM_BRAND}},
    {{GOVERNING_CARRIER, Direction::REVERSED}, {ECONOMY_BRAND}},
    {{GOVERNING_CARRIER2, Direction::BOTHWAYS}, {ECONOMY_BRAND, BUSINESS_BRAND, PREMIUM_BRAND}}
  };
  BrandCodeArraysPerCarrier expectedBrandDeduplication = brandDeduplication;

  BrandCodeArraysPerCarrier result, expected;
  expected = {};

  //TODO(andrzej.fediuk) DIR: add test for proper carrier with improper direction
  CarrierDirection otherCarrier("XX", Direction::BOTHWAYS);

  ASSERT_FALSE(
    TNBrandsFunctions::filterCarrierBrandCodesByCabinAtSegment(
      carrierBrandMap, otherCarrier, brandDeduplication,
      result, true));

  ASSERT_EQ(expectedBrandDeduplication, brandDeduplication);
  ASSERT_EQ(expected, result);
}

TEST_F(TNBrandsFunctionsTest, testProperlyFiltersBrandsPerCabinAtSegmentForCarrierWithoutBrands)
{
  BrandCodesPerCarrier carrierBrandMap = {
    {{GOVERNING_CARRIER, Direction::ORIGINAL}, {}},
    {{GOVERNING_CARRIER, Direction::REVERSED}, {}}
  };
  BrandCodeArraysPerCarrier brandDeduplication = {
    {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND, BUSINESS_BRAND, PREMIUM_BRAND}},
    {{GOVERNING_CARRIER, Direction::REVERSED}, {ECONOMY_BRAND, BUSINESS_BRAND, PREMIUM_BRAND}},
    {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {ECONOMY_BRAND, BUSINESS_BRAND, PREMIUM_BRAND}}
  };
  BrandCodeArraysPerCarrier expectedBrandDeduplication = brandDeduplication;

  BrandCodeArraysPerCarrier result, expected;
  expected = {
    {{GOVERNING_CARRIER, Direction::ORIGINAL}, {NO_BRAND}},
  };

  CarrierDirection governingWithDirection(GOVERNING_CARRIER, Direction::ORIGINAL);
  ASSERT_TRUE(
    TNBrandsFunctions::filterCarrierBrandCodesByCabinAtSegment(
      carrierBrandMap, governingWithDirection, brandDeduplication, result, false));

  ASSERT_EQ(expectedBrandDeduplication, brandDeduplication);
  ASSERT_EQ(expected, result);
}

TEST_F(TNBrandsFunctionsTest, testProperlyFiltersBrandsPerCabinAtSegmentForCarrierWithoutBrandsWithDirectionality)
{
  BrandCodesPerCarrier carrierBrandMap = {
    {{GOVERNING_CARRIER, Direction::ORIGINAL}, {}},
    {{GOVERNING_CARRIER, Direction::REVERSED}, {}}
  };
  BrandCodeArraysPerCarrier brandDeduplication = {
    {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND, BUSINESS_BRAND, PREMIUM_BRAND}},
    {{GOVERNING_CARRIER, Direction::REVERSED}, {ECONOMY_BRAND, BUSINESS_BRAND, PREMIUM_BRAND}},
    {{GOVERNING_CARRIER, Direction::BOTHWAYS}, {ECONOMY_BRAND, BUSINESS_BRAND, PREMIUM_BRAND}}
  };
  BrandCodeArraysPerCarrier expectedBrandDeduplication = brandDeduplication;

  BrandCodeArraysPerCarrier result, expected;
  expected = {
    {{GOVERNING_CARRIER, Direction::ORIGINAL}, {NO_BRAND}},
  };

  CarrierDirection governingWithDirection(GOVERNING_CARRIER, Direction::ORIGINAL);
  ASSERT_TRUE(
    TNBrandsFunctions::filterCarrierBrandCodesByCabinAtSegment(
      carrierBrandMap, governingWithDirection, brandDeduplication, result, true));

  ASSERT_EQ(expectedBrandDeduplication, brandDeduplication);
  ASSERT_EQ(expected, result);
}

TEST_F(TNBrandsFunctionsTest, testProperlyFiltersBrandsPerCabinAtSegment)
{
  BrandCodesPerCarrier carrierBrandMap = {
    {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND, BUSINESS_BRAND, PREMIUM_BRAND}},
    {{GOVERNING_CARRIER, Direction::REVERSED}, {ECONOMY_BRAND, BUSINESS_BRAND, PREMIUM_BRAND}}
  };
  BrandCodeArraysPerCarrier brandDeduplication = {
    {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND, PREMIUM_BRAND}},
    {{GOVERNING_CARRIER, Direction::REVERSED}, {ECONOMY_BRAND, BUSINESS_BRAND}}
  };
  BrandCodeArraysPerCarrier expectedBrandDeduplication = {
    {{GOVERNING_CARRIER, Direction::ORIGINAL}, {}}, //empty
    {{GOVERNING_CARRIER, Direction::REVERSED}, {ECONOMY_BRAND, BUSINESS_BRAND}}
  };
  BrandCodeArraysPerCarrier result, expected;
  expected = {
    {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND, PREMIUM_BRAND}},
  };

  CarrierDirection governingWithDirection(GOVERNING_CARRIER, Direction::ORIGINAL);
  ASSERT_TRUE(
    TNBrandsFunctions::filterCarrierBrandCodesByCabinAtSegment(
      carrierBrandMap, governingWithDirection, brandDeduplication, result, false));

  ASSERT_EQ(expectedBrandDeduplication, brandDeduplication);
  ASSERT_EQ(expected, result);
}

TEST_F(TNBrandsFunctionsTest, testProperlyFiltersBrandsPerCabinAtSegmentWithDirectionality)
{
  BrandCodesPerCarrier carrierBrandMap = {
    {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND, BUSINESS_BRAND, PREMIUM_BRAND}},
    {{GOVERNING_CARRIER, Direction::REVERSED}, {ECONOMY_BRAND, BUSINESS_BRAND, PREMIUM_BRAND}}
  };
  BrandCodeArraysPerCarrier brandDeduplication = {
    {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND, PREMIUM_BRAND}},
    {{GOVERNING_CARRIER, Direction::REVERSED}, {ECONOMY_BRAND, BUSINESS_BRAND}}
  };
  BrandCodeArraysPerCarrier expectedBrandDeduplication = {
    {{GOVERNING_CARRIER, Direction::ORIGINAL}, {}}, //empty
    {{GOVERNING_CARRIER, Direction::REVERSED}, {ECONOMY_BRAND, BUSINESS_BRAND}}
  };
  BrandCodeArraysPerCarrier result, expected;
  expected = {
    {{GOVERNING_CARRIER, Direction::ORIGINAL}, {ECONOMY_BRAND, PREMIUM_BRAND}},
  };

  CarrierDirection governingWithDirection(GOVERNING_CARRIER, Direction::ORIGINAL);
  ASSERT_TRUE(
    TNBrandsFunctions::filterCarrierBrandCodesByCabinAtSegment(
      carrierBrandMap, governingWithDirection, brandDeduplication, result, true));

  ASSERT_EQ(expectedBrandDeduplication, brandDeduplication);
  ASSERT_EQ(expected, result);
}

TEST_F(TNBrandsFunctionsTest, testFiltersAndDeduplicatesCarrierBrandsCorrectlyForNoValidBrands)
{
  UnorderedBrandCodes availableBrands = {ECONOMY_BRAND, BUSINESS_BRAND, PREMIUM_BRAND};

  OrderedBrandCodes validDeduplicatedBrands, expectedDeduplicatedBrands;

  OrderedBrandCodes result, expected = {NO_BRAND};

  ASSERT_TRUE(
    TNBrandsFunctions::filterAndDeduplicateCarrierBrandCodes(availableBrands,
      validDeduplicatedBrands, result));
  ASSERT_EQ(expected, result);
  ASSERT_EQ(expectedDeduplicatedBrands, validDeduplicatedBrands);
}

TEST_F(TNBrandsFunctionsTest, testFiltersAndDeduplicatesCarrierBrandsCorrectlyForNoAvailableBrands)
{
  UnorderedBrandCodes availableBrands;

  OrderedBrandCodes expectedDeduplicatedBrands, validDeduplicatedBrands = {
    ECONOMY_BRAND, BUSINESS_BRAND, PREMIUM_BRAND
  };
  expectedDeduplicatedBrands = validDeduplicatedBrands;

  OrderedBrandCodes result, expected;

  ASSERT_FALSE(
    TNBrandsFunctions::filterAndDeduplicateCarrierBrandCodes(availableBrands,
      validDeduplicatedBrands, result));
  ASSERT_EQ(expected, result);
  ASSERT_EQ(expectedDeduplicatedBrands, validDeduplicatedBrands);
}

TEST_F(TNBrandsFunctionsTest, testFiltersAndDeduplicatesCarrierBrandsCorrectly)
{
  UnorderedBrandCodes availableBrands = {ECONOMY_BRAND, BUSINESS_BRAND};

  OrderedBrandCodes expectedDeduplicatedBrands, validDeduplicatedBrands = {
    ECONOMY_BRAND, BUSINESS_BRAND, PREMIUM_BRAND
  };
  expectedDeduplicatedBrands = {PREMIUM_BRAND};

  OrderedBrandCodes result, expected = {ECONOMY_BRAND, BUSINESS_BRAND};

  ASSERT_TRUE(
    TNBrandsFunctions::filterAndDeduplicateCarrierBrandCodes(availableBrands,
      validDeduplicatedBrands, result));
  ASSERT_EQ(expected, result);
  ASSERT_EQ(expectedDeduplicatedBrands, validDeduplicatedBrands);
}

TEST_F(TNBrandsFunctionsTest, fixedLegTests)
{
  // first fixed leg's id = 3
  bool fixed[] = {true, true, true, false, false, false, false};
  std::vector<bool> fixedLegs(fixed, fixed + sizeof(fixed) / sizeof(bool));
  std::vector<TravelSeg*> travelSegs;

  TestMemHandle memHandle;

  TravelSeg* segment1 = memHandle.create<AirSeg>();
  TravelSeg* segment2 = memHandle.create<AirSeg>();
  TravelSeg* segment3 = memHandle.create<AirSeg>();
  TravelSeg* segment4 = memHandle.create<AirSeg>();
  TravelSeg* segment5 = memHandle.create<AirSeg>();

  segment1->legId() = 0;
  segment2->legId() = 1;
  segment3->legId() = 2;
  segment4->legId() = 3;
  segment5->legId() = 4;

  travelSegs.push_back(segment1);
  travelSegs.push_back(segment2);
  travelSegs.push_back(segment3);

  ASSERT_TRUE(TNBrandsFunctions::isAnySegmentOnFixedLeg(travelSegs, fixedLegs));
  ASSERT_FALSE(TNBrandsFunctions::isAnySegmentOnNonFixedLeg(travelSegs, fixedLegs));
  ASSERT_FALSE(TNBrandsFunctions::isAnySegmentOnCurrentlyShoppedLeg(travelSegs, fixedLegs));

  std::vector<TravelSeg*> travelSegs2;
  travelSegs2.push_back(segment5);
  ASSERT_FALSE(TNBrandsFunctions::isAnySegmentOnFixedLeg(travelSegs2, fixedLegs));
  ASSERT_TRUE(TNBrandsFunctions::isAnySegmentOnNonFixedLeg(travelSegs2, fixedLegs));
  ASSERT_FALSE(TNBrandsFunctions::isAnySegmentOnCurrentlyShoppedLeg(travelSegs2, fixedLegs));
  travelSegs2.push_back(segment4);
  ASSERT_FALSE(TNBrandsFunctions::isAnySegmentOnFixedLeg(travelSegs2, fixedLegs));
  ASSERT_TRUE(TNBrandsFunctions::isAnySegmentOnNonFixedLeg(travelSegs2, fixedLegs));
  ASSERT_TRUE(TNBrandsFunctions::isAnySegmentOnCurrentlyShoppedLeg(travelSegs2, fixedLegs));
  travelSegs2.push_back(segment3);
  ASSERT_TRUE(TNBrandsFunctions::isAnySegmentOnFixedLeg(travelSegs2, fixedLegs));
  ASSERT_TRUE(TNBrandsFunctions::isAnySegmentOnNonFixedLeg(travelSegs2, fixedLegs));
  ASSERT_TRUE(TNBrandsFunctions::isAnySegmentOnCurrentlyShoppedLeg(travelSegs2, fixedLegs));
}

TEST_F(TNBrandsFunctionsTest, setUpdateNoBrandMerge)
{
  CarrierDirection carrierWithDirection = {CarrierCode("XX"), Direction::ORIGINAL};
  CarrierBrandPairs spaceBlock = {{{CarrierCode("XX"), Direction::REVERSED}, NO_BRAND}};

  CarrierBrandPairs expectedSpaceBlock = {{{CarrierCode("XX"), Direction::BOTHWAYS}, NO_BRAND}};

  TNBrandsFunctions::setUpdateNoBrand(spaceBlock, carrierWithDirection);
  ASSERT_EQ(expectedSpaceBlock, spaceBlock);
}

TEST_F(TNBrandsFunctionsTest, setUpdateNoBrandNoMerge)
{
  CarrierDirection carrierWithDirection = {CarrierCode("XX"), Direction::ORIGINAL};
  CarrierBrandPairs spaceBlock = {{{CarrierCode("XX"), Direction::REVERSED}, BrandCode("AA")}};

  CarrierBrandPairs expectedSpaceBlock = {
    {{CarrierCode("XX"), Direction::REVERSED}, BrandCode("AA")},
    {{CarrierCode("XX"), Direction::ORIGINAL}, NO_BRAND}
  };

  TNBrandsFunctions::setUpdateNoBrand(spaceBlock, carrierWithDirection);
  ASSERT_EQ(expectedSpaceBlock, spaceBlock);
}


} // namespace skipper

} // namespace tse
