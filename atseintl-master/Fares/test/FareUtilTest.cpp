#include <gtest/gtest.h>
#include "test/include/GtestHelperMacros.h"

#include "Common/Config/DynamicConfigLoader.h"
#include "Common/TrxUtil.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/FareClassAppSegInfo.h"
#include "Fares/FareUtil.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"


namespace tse
{
class FareUtilTest : public ::testing::Test
{
protected:
  TestMemHandle _memHandle;
  Itin* _itin;
  PricingTrx* _trx;
  FareClassAppSegInfo* _fcas;
  PaxTypeFare* _paxTypeFare;

public:
  void SetUp()
  {
    _itin = _memHandle.create<Itin>();
    _trx = _memHandle.create<PricingTrx>();
    _fcas = _memHandle.create<FareClassAppSegInfo>();

    _trx->setOptions(_memHandle.create<PricingOptions>());
    _paxTypeFare = _memHandle.create<PaxTypeFare>();
    _memHandle.create<TestConfigInitializer>();
  }

  void TearDown() { _memHandle.clear(); }

  //-----------------------------------------------------------------
  // testValidate_Carrier_Same_TktAppBlank()
  // CASE: Validate same carrier when tktApp set to blank
  // Expected to PASS
  //-----------------------------------------------------------------
};

TEST_F(FareUtilTest, testValidate_Carrier_Same_TktAppBlank)
{
  _itin->validatingCarrier() = "LH";
  EXPECT_TRUE(FareUtil::isNegFareCarrierValid("LH", ' ', *_itin, false));
}

//-----------------------------------------------------------------
// testValidate_Carrier_Same_TktAppSet()
// CASE: Validate same carrier when tktApp set to 'X'
// Expected to FAIL
//-----------------------------------------------------------------
TEST_F(FareUtilTest, testValidate_Carrier_Same_TktAppSet)
{
  _itin->validatingCarrier() = "LH";
  EXPECT_FALSE(FareUtil::isNegFareCarrierValid("LH", 'X', *_itin, false));
}

//-----------------------------------------------------------------
// testValidate_Carrier_Different_TktAppBlank()
// CASE: Validate different carriers when tktApp set to blank
// Expected to FAIL
//-----------------------------------------------------------------
TEST_F(FareUtilTest, testValidate_Carrier_Different_TktAppBlank)
{
  _itin->validatingCarrier() = "LO";
  EXPECT_FALSE(FareUtil::isNegFareCarrierValid("LH", ' ', *_itin, false));
}

//-----------------------------------------------------------------
// testValidate_Carrier_Different_TktAppSet()
// CASE: Validate different carriers when tktApp set to 'X'
// Expected PASS
//-----------------------------------------------------------------
TEST_F(FareUtilTest, testValidate_Carrier_Different_TktAppSet)
{
  _itin->validatingCarrier() = "LO";
  EXPECT_TRUE(FareUtil::isNegFareCarrierValid("LH", 'X', *_itin, false));
}

TEST_F(FareUtilTest, testPassValidate_Carrier_Different_TktAppSetMultipleCarriers)
{
  _trx->setValidatingCxrGsaApplicable(true);
  FareMarket* fm = _memHandle.create<FareMarket>();

  fm->validatingCarriers().push_back("LH");
  fm->validatingCarriers().push_back("LO");
  _paxTypeFare->fareMarket() = fm;
  EXPECT_TRUE(FareUtil::isNegFareCarrierValid("LH", 'X', _paxTypeFare, false));
  EXPECT_TRUE(_paxTypeFare->validatingCarriers()[0] == "LO");
}
TEST_F(FareUtilTest, testFailValidate_Carrier_Different_TktAppSetMultipleCarriers)
{
  _trx->setValidatingCxrGsaApplicable(true);
  FareMarket* fm = _memHandle.create<FareMarket>();
  fm->validatingCarriers().push_back("AA");
  fm->validatingCarriers().push_back("DL");
  _paxTypeFare->fareMarket() = fm;
  EXPECT_FALSE(FareUtil::isNegFareCarrierValid("LH", ' ', _paxTypeFare, false));
}

TEST_F(FareUtilTest, testFailFareClassAppSegDirectioanlity_Fail)
{
  EXPECT_FALSE(FareUtil::failFareClassAppSegDirectioanlity(*_fcas, *_trx, true));
}

TEST_F(FareUtilTest, testFailFareClassAppSegDirectioanlity_LOC1)
{
  _fcas->_directionality = ORIGINATING_LOC1;
  EXPECT_TRUE(FareUtil::failFareClassAppSegDirectioanlity(*_fcas, *_trx, true));
}

TEST_F(FareUtilTest, testFailFareClassAppSegDirectioanlity_LOC2)
{
  _fcas->_directionality = ORIGINATING_LOC2;
  EXPECT_TRUE(FareUtil::failFareClassAppSegDirectioanlity(*_fcas, *_trx, false));
}

TEST_F(FareUtilTest, testFailFareClassAppSegDirectioanlity_RTW)
{
  PricingOptions opt;
  opt.setRtw(true);
  _trx->setOptions(&opt);
  _fcas->_directionality = ORIGINATING_LOC1;

  EXPECT_FALSE(FareUtil::failFareClassAppSegDirectioanlity(*_fcas, *_trx, true));
}

TEST_F(FareUtilTest, testPostCheckOutboundFare_hasRemoveOutboundFares_false)
{
  EXPECT_FALSE(FareUtil::postCheckOutboundFare(false, true, TO));
  EXPECT_FALSE(FareUtil::postCheckOutboundFare(false, true, FROM));
  EXPECT_FALSE(FareUtil::postCheckOutboundFare(false, false, TO));
  EXPECT_FALSE(FareUtil::postCheckOutboundFare(false, false, FROM));
}

TEST_F(FareUtilTest, testPostCheckOutboundFare_hasRemoveOutboundFares_true)
{
  EXPECT_TRUE(FareUtil::postCheckOutboundFare(true, true, TO));
  EXPECT_FALSE(FareUtil::postCheckOutboundFare(true, false, TO));

  EXPECT_FALSE(FareUtil::postCheckOutboundFare(true, true, FROM));
  EXPECT_TRUE(FareUtil::postCheckOutboundFare(true, false, FROM));
}

}
