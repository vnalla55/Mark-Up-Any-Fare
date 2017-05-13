//------------------------------------------------------------------
//
//  File: PUPathMatrixUseReducedConstructionsTest.cpp
//
//  Copyright Sabre 2014
//
//    The copyright to the computer program(s) herein
//    is the property of Sabre.
//    The program(s) may be used and/or copied only with
//    the written permission of Sabre or in accordance
//    with the terms and conditions stipAulated in the
//    agreement/contract under which the program(s)
//    have been supplied.
//
//-------------------------------------------------------------------
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "Pricing/PUPathMatrix.h"
#include "Pricing/FareMarketPath.h"

#include "test/include/GtestHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
using namespace ::testing;


class PUPathMatrixUseReducedConstructionsMock : public PUPathMatrix
{
public:
  MOCK_METHOD2(isOWoutOfIntlRT, bool(const FareMarketPath&, PUPath&));
};


class PUPathMatrixUseReducedConstructionsTest : public Test
{
public:
  void SetUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx.setRequest(&_request);
    _owPuResult = false;
    _puPathMatrixMock.reset(new PUPathMatrixUseReducedConstructionsMock);
    _puPathMatrixMock->trx() = &_trx;
    _puPathMatrixMock->itin() = &_itin;
    _itin.itinLegs().resize(2);
  }

  void TearDown() { _memHandle.clear(); }

  // Delegate to protected methods

  bool delegate_isPUPathValid(const FareMarketPath& fmp, PUPath& puPath,
                              const uint16_t totalMktCnt)
  {
    return _puPathMatrixMock->isPUPathValid(fmp, puPath, totalMktCnt);
  }

  // Helper methods

  typedef std::pair<PricingUnit::Type, size_t> PuFillType;
  typedef std::vector<PuFillType> PuFillTypeVec;

  void fillPuPath(PuFillTypeVec& puData)
  {
    for (size_t i = 0; i < puData.size(); ++i)
    {
      PU *pu = 0;
      _trx.dataHandle().get(pu);
      pu->puType() = puData[i].first;
      pu->fareMarket().resize(puData[i].second); // We don't care about pointer values
      pu->setFCCount();
      _puPath.puPath().push_back(pu);
    }
  }

  PricingTrx _trx;
  PricingRequest _request;
  FareMarketPath _fmPath;
  PUPath _puPath;
  Itin _itin;
  bool _owPuResult;
  std::shared_ptr<PUPathMatrixUseReducedConstructionsMock> _puPathMatrixMock;

  TestMemHandle _memHandle;
};


TEST_F(PUPathMatrixUseReducedConstructionsTest, passSimpleNoRC)
{
  _request.setUseReducedConstructions(false);

  PuFillTypeVec puFill;
  puFill.resize(1);
  puFill[0].first = PricingUnit::Type::ONEWAY;
  puFill[0].second = 1;

  fillPuPath(puFill);

  EXPECT_CALL(*_puPathMatrixMock, isOWoutOfIntlRT(_, _)).WillOnce(Return(_owPuResult));

  ASSERT_TRUE(delegate_isPUPathValid(_fmPath, _puPath, 1));
}


TEST_F(PUPathMatrixUseReducedConstructionsTest, passSimpleRC)
{
  _request.setUseReducedConstructions(true);

  PuFillTypeVec puFill;
  puFill.resize(1);
  puFill[0].first = PricingUnit::Type::ONEWAY;
  puFill[0].second = 1;

  fillPuPath(puFill);

  EXPECT_CALL(*_puPathMatrixMock, isOWoutOfIntlRT(_, _)).WillOnce(Return(_owPuResult));

  ASSERT_TRUE(delegate_isPUPathValid(_fmPath, _puPath, 1));
}


TEST_F(PUPathMatrixUseReducedConstructionsTest, pass2OW)
{
  _request.setUseReducedConstructions(true);

  PuFillTypeVec puFill;
  puFill.resize(2);
  puFill[0].first = PricingUnit::Type::ONEWAY;
  puFill[0].second = 1;
  puFill[1].first = PricingUnit::Type::ONEWAY;
  puFill[1].second = 1;

  fillPuPath(puFill);

  EXPECT_CALL(*_puPathMatrixMock, isOWoutOfIntlRT(_, _)).WillOnce(Return(_owPuResult));

  ASSERT_TRUE(delegate_isPUPathValid(_fmPath, _puPath, 2));
}


TEST_F(PUPathMatrixUseReducedConstructionsTest, pass1OW1OJ)
{
  _request.setUseReducedConstructions(true);

  PuFillTypeVec puFill;
  puFill.resize(2);
  puFill[0].first = PricingUnit::Type::ONEWAY;
  puFill[0].second = 1;
  puFill[1].first = PricingUnit::Type::OPENJAW;
  puFill[1].second = 2;

  fillPuPath(puFill);

  EXPECT_CALL(*_puPathMatrixMock, isOWoutOfIntlRT(_, _)).WillOnce(Return(_owPuResult));

  ASSERT_TRUE(delegate_isPUPathValid(_fmPath, _puPath, 3));
}


TEST_F(PUPathMatrixUseReducedConstructionsTest, pass2OW1OJ)
{
  _request.setUseReducedConstructions(true);

  PuFillTypeVec puFill;
  puFill.resize(3);
  puFill[0].first = PricingUnit::Type::ONEWAY;
  puFill[0].second = 1;
  puFill[1].first = PricingUnit::Type::ONEWAY;
  puFill[1].second = 1;
  puFill[2].first = PricingUnit::Type::OPENJAW;
  puFill[2].second = 2;

  fillPuPath(puFill);

  EXPECT_CALL(*_puPathMatrixMock, isOWoutOfIntlRT(_, _)).WillOnce(Return(_owPuResult));

  ASSERT_TRUE(delegate_isPUPathValid(_fmPath, _puPath, 4));
}


TEST_F(PUPathMatrixUseReducedConstructionsTest, fail2OW1OJ1OW)
{
  _request.setUseReducedConstructions(true);

  PuFillTypeVec puFill;
  puFill.resize(4);
  puFill[0].first = PricingUnit::Type::ONEWAY;
  puFill[0].second = 1;
  puFill[1].first = PricingUnit::Type::ONEWAY;
  puFill[1].second = 1;
  puFill[2].first = PricingUnit::Type::OPENJAW;
  puFill[2].second = 2;
  puFill[3].first = PricingUnit::Type::ONEWAY;
  puFill[3].second = 1;

  fillPuPath(puFill);

  EXPECT_CALL(*_puPathMatrixMock, isOWoutOfIntlRT(_, _)).WillOnce(Return(_owPuResult));

  ASSERT_FALSE(delegate_isPUPathValid(_fmPath, _puPath, 5));
}


TEST_F(PUPathMatrixUseReducedConstructionsTest, pass2OJ)
{
  _request.setUseReducedConstructions(true);

  PuFillTypeVec puFill;
  puFill.resize(2);
  puFill[0].first = PricingUnit::Type::OPENJAW;
  puFill[0].second = 2;
  puFill[1].first = PricingUnit::Type::OPENJAW;
  puFill[1].second = 2;

  fillPuPath(puFill);

  EXPECT_CALL(*_puPathMatrixMock, isOWoutOfIntlRT(_, _)).WillOnce(Return(_owPuResult));

  ASSERT_TRUE(delegate_isPUPathValid(_fmPath, _puPath, 4));
}


TEST_F(PUPathMatrixUseReducedConstructionsTest, fail2OJLongFirst)
{
  _request.setUseReducedConstructions(true);

  PuFillTypeVec puFill;
  puFill.resize(2);
  puFill[0].first = PricingUnit::Type::OPENJAW;
  puFill[0].second = 3;
  puFill[1].first = PricingUnit::Type::OPENJAW;
  puFill[1].second = 2;

  fillPuPath(puFill);

  EXPECT_CALL(*_puPathMatrixMock, isOWoutOfIntlRT(_, _)).WillOnce(Return(_owPuResult));

  ASSERT_FALSE(delegate_isPUPathValid(_fmPath, _puPath, 5));
}


TEST_F(PUPathMatrixUseReducedConstructionsTest, fail2OJLongSecond)
{
  _request.setUseReducedConstructions(true);

  PuFillTypeVec puFill;
  puFill.resize(2);
  puFill[0].first = PricingUnit::Type::OPENJAW;
  puFill[0].second = 2;
  puFill[1].first = PricingUnit::Type::OPENJAW;
  puFill[1].second = 3;

  fillPuPath(puFill);

  EXPECT_CALL(*_puPathMatrixMock, isOWoutOfIntlRT(_, _)).WillOnce(Return(_owPuResult));

  ASSERT_FALSE(delegate_isPUPathValid(_fmPath, _puPath, 5));
}


TEST_F(PUPathMatrixUseReducedConstructionsTest, passCT)
{
  _request.setUseReducedConstructions(true);

  PuFillTypeVec puFill;
  puFill.resize(1);
  puFill[0].first = PricingUnit::Type::CIRCLETRIP;
  puFill[0].second = 3;

  fillPuPath(puFill);

  EXPECT_CALL(*_puPathMatrixMock, isOWoutOfIntlRT(_, _)).WillOnce(Return(_owPuResult));

  ASSERT_TRUE(delegate_isPUPathValid(_fmPath, _puPath, 3));
}


TEST_F(PUPathMatrixUseReducedConstructionsTest, pass1ShortCT2OW)
{
  _request.setUseReducedConstructions(true);

  PuFillTypeVec puFill;
  puFill.resize(3);
  puFill[0].first = PricingUnit::Type::CIRCLETRIP;
  puFill[0].second = 2;
  puFill[1].first = PricingUnit::Type::ONEWAY;
  puFill[1].second = 1;
  puFill[2].first = PricingUnit::Type::ONEWAY;
  puFill[2].second = 1;

  fillPuPath(puFill);

  EXPECT_CALL(*_puPathMatrixMock, isOWoutOfIntlRT(_, _)).WillOnce(Return(_owPuResult));

  ASSERT_TRUE(delegate_isPUPathValid(_fmPath, _puPath, 4));
}


TEST_F(PUPathMatrixUseReducedConstructionsTest, fail1CT2OW)
{
  _request.setUseReducedConstructions(true);

  PuFillTypeVec puFill;
  puFill.resize(3);
  puFill[0].first = PricingUnit::Type::CIRCLETRIP;
  puFill[0].second = 3;
  puFill[1].first = PricingUnit::Type::ONEWAY;
  puFill[1].second = 1;
  puFill[2].first = PricingUnit::Type::ONEWAY;
  puFill[2].second = 1;

  fillPuPath(puFill);

  EXPECT_CALL(*_puPathMatrixMock, isOWoutOfIntlRT(_, _)).WillOnce(Return(_owPuResult));

  ASSERT_FALSE(delegate_isPUPathValid(_fmPath, _puPath, 5));
}


TEST_F(PUPathMatrixUseReducedConstructionsTest, pass1CT1RTnoRC)
{
  _request.setUseReducedConstructions(false);

  PuFillTypeVec puFill;
  puFill.resize(2);
  puFill[0].first = PricingUnit::Type::CIRCLETRIP;
  puFill[0].second = 3;
  puFill[1].first = PricingUnit::Type::ROUNDTRIP;
  puFill[1].second = 2;

  fillPuPath(puFill);

  EXPECT_CALL(*_puPathMatrixMock, isOWoutOfIntlRT(_, _)).WillOnce(Return(_owPuResult));

  ASSERT_TRUE(delegate_isPUPathValid(_fmPath, _puPath, 5));
}


TEST_F(PUPathMatrixUseReducedConstructionsTest, fail1CT1RT)
{
  _request.setUseReducedConstructions(true);

  PuFillTypeVec puFill;
  puFill.resize(2);
  puFill[0].first = PricingUnit::Type::CIRCLETRIP;
  puFill[0].second = 3;
  puFill[1].first = PricingUnit::Type::ROUNDTRIP;
  puFill[1].second = 2;

  fillPuPath(puFill);

  EXPECT_CALL(*_puPathMatrixMock, isOWoutOfIntlRT(_, _)).WillOnce(Return(_owPuResult));

  ASSERT_FALSE(delegate_isPUPathValid(_fmPath, _puPath, 5));
}


TEST_F(PUPathMatrixUseReducedConstructionsTest, pass4OW)
{
  _request.setUseReducedConstructions(true);

  PuFillTypeVec puFill;
  puFill.resize(4);
  for (size_t i = 0; i < puFill.size(); ++i)
  {
    puFill[i].first = PricingUnit::Type::ONEWAY;
    puFill[i].second = 1;
  }

  fillPuPath(puFill);

  EXPECT_CALL(*_puPathMatrixMock, isOWoutOfIntlRT(_, _)).WillOnce(Return(_owPuResult));

  ASSERT_TRUE(delegate_isPUPathValid(_fmPath, _puPath, 4));
}


TEST_F(PUPathMatrixUseReducedConstructionsTest, pass5OWnoRC)
{
  _request.setUseReducedConstructions(false);

  PuFillTypeVec puFill;
  puFill.resize(5);
  for (size_t i = 0; i < puFill.size(); ++i)
  {
    puFill[i].first = PricingUnit::Type::ONEWAY;
    puFill[i].second = 1;
  }

  fillPuPath(puFill);

  EXPECT_CALL(*_puPathMatrixMock, isOWoutOfIntlRT(_, _)).WillOnce(Return(_owPuResult));

  ASSERT_TRUE(delegate_isPUPathValid(_fmPath, _puPath, 5));
}


TEST_F(PUPathMatrixUseReducedConstructionsTest, fail5OW)
{
  _request.setUseReducedConstructions(true);

  PuFillTypeVec puFill;
  puFill.resize(5);
  for (size_t i = 0; i < puFill.size(); ++i)
  {
    puFill[i].first = PricingUnit::Type::ONEWAY;
    puFill[i].second = 1;
  }

  fillPuPath(puFill);

  EXPECT_CALL(*_puPathMatrixMock, isOWoutOfIntlRT(_, _)).WillOnce(Return(_owPuResult));

  ASSERT_FALSE(delegate_isPUPathValid(_fmPath, _puPath, 5));
}


TEST_F(PUPathMatrixUseReducedConstructionsTest, pass5legs5OW)
{
  _request.setUseReducedConstructions(true);

  _itin.itinLegs().resize(5);

  PuFillTypeVec puFill;
  puFill.resize(5);
  for (size_t i = 0; i < puFill.size(); ++i)
  {
    puFill[i].first = PricingUnit::Type::ONEWAY;
    puFill[i].second = 1;
  }

  fillPuPath(puFill);

  EXPECT_CALL(*_puPathMatrixMock, isOWoutOfIntlRT(_, _)).WillOnce(Return(_owPuResult));

  ASSERT_TRUE(delegate_isPUPathValid(_fmPath, _puPath, 5));
}


TEST_F(PUPathMatrixUseReducedConstructionsTest, fail5legs6OW)
{
  _request.setUseReducedConstructions(true);

  _itin.itinLegs().resize(5);

  PuFillTypeVec puFill;
  puFill.resize(6);
  for (size_t i = 0; i < puFill.size(); ++i)
  {
    puFill[i].first = PricingUnit::Type::ONEWAY;
    puFill[i].second = 1;
  }

  fillPuPath(puFill);

  EXPECT_CALL(*_puPathMatrixMock, isOWoutOfIntlRT(_, _)).WillOnce(Return(_owPuResult));

  ASSERT_FALSE(delegate_isPUPathValid(_fmPath, _puPath, 6));
}

/*
PU type combinations for a 6-segment itin:

CT
CT OW OW
CT RT
OJ OJ
OJ OJ OJ
OJ OJ OW
OJ OJ OW OW
OJ OJ RT
OJ OW
OJ OW OJ
OJ OW OJ OW
OJ OW OW
OJ OW OW OJ
OJ OW OW OW
OJ OW OW OW OW
OJ OW RT
OJ OW RT OW
OJ RT OJ
OJ RT OW
OJ RT OW OW
OW OJ
OW OJ OJ
OW OJ OJ OW
OW OJ OW
OW OJ OW OJ
OW OJ OW OW
OW OJ OW OW OW
OW OJ RT
OW OJ RT OW
OW OW OJ
OW OW OJ OJ
OW OW OJ OW
OW OW OJ OW OW
OW OW OW OJ
OW OW OW OJ OW
OW OW OW OW
OW OW OW OW OJ
OW OW OW OW OW
OW OW OW OW OW OW
OW OW RT OJ
OW OW RT OW OW
*/


} // namespace tse
