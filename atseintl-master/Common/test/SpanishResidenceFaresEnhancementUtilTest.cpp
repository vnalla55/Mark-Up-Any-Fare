//-------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "Common/SpanishResidentFaresEnhancementUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FareCalcConfig.h"

#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include "test/include/ATSEv2Test.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>

namespace tse
{
FALLBACKVALUE_DECL(srfeFareBasisSuffix);
FALLBACKVALUE_DECL(srfeFareBasisSuffixOldWay);

class SRFDUtilTest : public ::testing::Test
{
protected:
  class StubPricingTrx : public PricingTrx
  {
  public:
    StubPricingTrx(TestMemHandle& memHandle) : PricingTrx() {}
  };

  StubPricingTrx* _trx;
  TestMemHandle _memHandle;

public:
  void SetUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<StubPricingTrx>(_memHandle);
    TestConfigInitializer::setValue("SPANISH_TERRITORIES", "RBP|RRC|RRM|RCE", "SRFE");
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->itin().push_back(_memHandle.create<Itin>());
    AirSeg* ts = _memHandle.create<AirSeg>();
    Loc* origin = _memHandle.create<Loc>();
    origin->nation() = "ES";
    Loc* destination = _memHandle.create<Loc>();
    destination->nation() = "ES";
    ts->origin() = origin;
    ts->destination() = destination;
    _trx->itin().front()->travelSeg().push_back(ts);
  }

  void TearDown() { _memHandle.clear(); }

protected:
  template <std::size_t N>
  void ResidenceState(const std::array<LocCode, N>& cities, StateCode state, bool validResult)
  {
    for (const auto& city : cities)
    {
      _trx->getOptions()->residency() = city;
      if (validResult)
      {
        EXPECT_TRUE(SRFEUtil::isPassengerApplicable(_trx->getOptions()->residency(), _trx->residencyState()));
        EXPECT_EQ(state, SRFEUtil::mapCityToState(city));
        EXPECT_EQ(_trx->residencyState(), SRFEUtil::mapCityToState(city));
      }
      else
      {
        EXPECT_FALSE(SRFEUtil::isPassengerApplicable(_trx->getOptions()->residency(), _trx->residencyState()));
        EXPECT_NE(state, SRFEUtil::mapCityToState(city));
      }
    }
  }
};

TEST_F(SRFDUtilTest, testIsPassengerBalearic)
{
  _trx->getOptions()->residency() = LOC_PMI;
  EXPECT_TRUE(
      SRFEUtil::isPassengerApplicable(_trx->getOptions()->residency(), _trx->residencyState()));
  EXPECT_EQ(_trx->residencyState(), "RBP");
}

TEST_F(SRFDUtilTest, testIsPassengerCanary)
{
  _trx->getOptions()->residency() = LOC_TCI;
  EXPECT_TRUE(
      SRFEUtil::isPassengerApplicable(_trx->getOptions()->residency(), _trx->residencyState()));
  EXPECT_EQ(_trx->residencyState(), "RRC");
}

TEST_F(SRFDUtilTest, testIsPassengerMelilla)
{
  _trx->getOptions()->residency() = LOC_MLN;
  EXPECT_TRUE(
      SRFEUtil::isPassengerApplicable(_trx->getOptions()->residency(), _trx->residencyState()));
  EXPECT_EQ(_trx->residencyState(), "RRM");
}

TEST_F(SRFDUtilTest, testIsPassengerCeuta)
{
  _trx->getOptions()->residency() = LOC_JCU;
  EXPECT_TRUE(
      SRFEUtil::isPassengerApplicable(_trx->getOptions()->residency(), _trx->residencyState()));
  EXPECT_EQ(_trx->residencyState(), "RCE");
}

TEST_F(SRFDUtilTest, testPassengerNotApplicable)
{
  _trx->getOptions()->residency() = "AAA";
  auto prevState = _trx->residencyState();
  EXPECT_FALSE(
      SRFEUtil::isPassengerApplicable(_trx->getOptions()->residency(), _trx->residencyState()));
  EXPECT_EQ(_trx->residencyState(), prevState);
}

TEST_F(SRFDUtilTest, testMapToCityRRC)
{
  std::array<LocCode, 7> cities{LOC_TFN, LOC_TFS, LOC_TCI, LOC_LPA, LOC_ACE, LOC_FUE, LOC_SPC};
  ResidenceState(cities, ST_RRC, true);

  std::array<LocCode, 1> citiesNoSpain{LOC_SIN};
  for (StateCode sc : {ST_RBP, ST_RRC, ST_RRM, ST_RCE})
    ResidenceState(citiesNoSpain, sc, false);
}

TEST_F(SRFDUtilTest, testMapToCityRBP)
{
  std::array<LocCode, 3> cities{LOC_PMI, LOC_IBZ, LOC_MAH};
  ResidenceState(cities, ST_RBP, true);
}

TEST_F(SRFDUtilTest, testMapToCityOthers)
{
  std::array<LocCode, 1> cities1{LOC_MLN};
  std::array<LocCode, 1> cities2{LOC_JCU};
  ResidenceState(cities1, ST_RRM, true);
  ResidenceState(cities2, ST_RCE, true);
}

class FareBasisSuffixTest : public ATSEv2Test
{
public:
  PaxTypeFare*
  createPaxTypeFare(LocCode origin, LocCode dest)
  {
    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fareInfo->market1() = origin;
    fareInfo->market2() = dest;

    Fare* fare = _memHandle.create<Fare>();
    fare->setFareInfo(fareInfo);

    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    ptf->setFare(fare);

    return ptf;
  }
};

TEST_F(FareBasisSuffixTest, testBaleares)
{
  _pTrx->residencyState() = ST_RBP;

  PaxTypeFare* ptf = createPaxTypeFare(LOC_PMI, LOC_IBZ);
  SRFEUtil::addFareBasisSuffix(*ptf, *_pTrx);
  ASSERT_EQ(ptf->createFareBasis(nullptr), "BI");

  ptf = createPaxTypeFare(LOC_PMI, "MAD");
  SRFEUtil::addFareBasisSuffix(*ptf, *_pTrx);
  ASSERT_EQ(ptf->createFareBasis(nullptr), "BP");

  ptf = createPaxTypeFare(LOC_PMI, LOC_TCI);
  SRFEUtil::addFareBasisSuffix(*ptf, *_pTrx);
  ASSERT_EQ(ptf->createFareBasis(nullptr), "BP");

  fallback::value::srfeFareBasisSuffixOldWay.set(true);

  ptf = createPaxTypeFare(LOC_PMI, LOC_IBZ);
  SRFEUtil::addFareBasisSuffix(*ptf, *_pTrx);
  ASSERT_EQ(ptf->createFareBasis(nullptr), "IB");

  ptf = createPaxTypeFare(LOC_PMI, "MAD");
  SRFEUtil::addFareBasisSuffix(*ptf, *_pTrx);
  ASSERT_EQ(ptf->createFareBasis(nullptr), "RB");

  ptf = createPaxTypeFare(LOC_PMI, LOC_TCI);
  SRFEUtil::addFareBasisSuffix(*ptf, *_pTrx);
  ASSERT_EQ(ptf->createFareBasis(nullptr), "RB");

  fallback::value::srfeFareBasisSuffix.set(true);

  ptf = createPaxTypeFare(LOC_PMI, LOC_IBZ);
  SRFEUtil::addFareBasisSuffix(*ptf, *_pTrx);
  ASSERT_EQ(ptf->createFareBasis(nullptr), "");
}

TEST_F(FareBasisSuffixTest, testCanarias)
{
  _pTrx->residencyState() = ST_RRC;

  PaxTypeFare* ptf = createPaxTypeFare(LOC_TCI, LOC_LPA);
  SRFEUtil::addFareBasisSuffix(*ptf, *_pTrx);
  ASSERT_EQ(ptf->createFareBasis(nullptr), "DC");

  ptf = createPaxTypeFare(LOC_TCI, "MAD");
  SRFEUtil::addFareBasisSuffix(*ptf, *_pTrx);
  ASSERT_EQ(ptf->createFareBasis(nullptr), "RC");

  ptf = createPaxTypeFare(LOC_TCI, LOC_IBZ);
  SRFEUtil::addFareBasisSuffix(*ptf, *_pTrx);
  ASSERT_EQ(ptf->createFareBasis(nullptr), "RC");

  fallback::value::srfeFareBasisSuffixOldWay.set(true);

  ptf = createPaxTypeFare(LOC_TCI, LOC_LPA);
  SRFEUtil::addFareBasisSuffix(*ptf, *_pTrx);
  ASSERT_EQ(ptf->createFareBasis(nullptr), "IC");
}

TEST_F(FareBasisSuffixTest, testMelilla)
{
  _pTrx->residencyState() = ST_RRM;

  PaxTypeFare* ptf = createPaxTypeFare(LOC_MLN, LOC_IBZ);
  SRFEUtil::addFareBasisSuffix(*ptf, *_pTrx);
  ASSERT_EQ(ptf->createFareBasis(nullptr), "RM");

  ptf = createPaxTypeFare(LOC_MLN, "MAD");
  SRFEUtil::addFareBasisSuffix(*ptf, *_pTrx);
  ASSERT_EQ(ptf->createFareBasis(nullptr), "RM");
}

TEST_F(FareBasisSuffixTest, testCeuta)
{
  _pTrx->residencyState() = ST_RCE;

  PaxTypeFare* ptf = createPaxTypeFare(LOC_JCU, LOC_IBZ);
  SRFEUtil::addFareBasisSuffix(*ptf, *_pTrx);
  ASSERT_EQ(ptf->createFareBasis(nullptr), "RE");

  ptf = createPaxTypeFare(LOC_JCU, "MAD");
  SRFEUtil::addFareBasisSuffix(*ptf, *_pTrx);
  ASSERT_EQ(ptf->createFareBasis(nullptr), "RE");

  ptf = createPaxTypeFare(LOC_AGP, "MAD");
  SRFEUtil::addFareBasisSuffix(*ptf, *_pTrx);
  ASSERT_EQ(ptf->createFareBasis(nullptr), "RE");
}
// ---------------------------------------------------------------------------------------------
// Test for SRFEUtil::isItinApplicable()
class ItinTest : public ATSEv2Test
{
};

TEST_F(ItinTest, testEmptyItin)
{
  Itin* itin = _memHandle.create<Itin>();
  ASSERT_FALSE(SRFEUtil::isItinApplicable(*itin));
}

}
