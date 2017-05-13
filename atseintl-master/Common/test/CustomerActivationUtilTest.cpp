//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "test/include/GtestHelperMacros.h"
#include "Common/CustomerActivationUtil.h"
#include "DataModel/PricingTrx.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
using namespace ::testing;

class CustomerActivationUtilGut : public CustomerActivationUtil
{
public:
  CustomerActivationUtilGut(PricingTrx& trx, const CarrierCode* cxr)
    : CustomerActivationUtil(trx, cxr)
  {
  }

  MOCK_METHOD3(matchCarrierActivationAppl,
               bool(std::vector<CustomerActivationControl::CarrierActivation*>& cxrActVector,
                    DateTime& activationDate,
                    DiagCollector* diag));
};

class CustomerActivationUtilTest : public ::testing::Test
{
public:
  PricingTrx* _trx;
  CarrierCode* _govCxr;
  CustomerActivationUtilGut* _cau;
  DateTime* _activationDate;
  std::vector<CustomerActivationControl::CarrierActivation*>* _cxrActVector;

  TestMemHandle _memHandle;

  void SetUp()
  {
    _trx = _memHandle(new PricingTrx);
    _govCxr = _memHandle(new CarrierCode(CARRIER_WS));
    _cau = _memHandle(new CustomerActivationUtilGut(*_trx, _govCxr));
    _activationDate = _memHandle(new DateTime);
    _cxrActVector = _memHandle(new std::vector<CustomerActivationControl::CarrierActivation*>);
  }

  void TearDown() { _memHandle.clear(); }

  inline void addCarrierEntry(const CarrierCode& cc)
  {
    _cxrActVector->push_back(_memHandle(new CustomerActivationControl::CarrierActivation));
    _cxrActVector->back()->cxr() = cc;
  }
};

TEST_F(CustomerActivationUtilTest, testMatchCarrierActivationApplEmpty)
{
  EXPECT_CALL(*_cau, matchCarrierActivationAppl(_, _, _)).WillOnce(Return(false));
  ASSERT_FALSE(_cau->matchCarrierActivationAppl(*_cxrActVector, *_activationDate, NULL));
}

TEST_F(CustomerActivationUtilTest, testMatchCarrierActivationApplOneMatch)
{
  addCarrierEntry(CARRIER_WS);
  EXPECT_CALL(*_cau, matchCarrierActivationAppl(_, _, _)).WillOnce(Return(true));
  ASSERT_TRUE(_cau->matchCarrierActivationAppl(*_cxrActVector, *_activationDate, NULL));
}

TEST_F(CustomerActivationUtilTest, testMatchCarrierActivationApplDollar)
{
  addCarrierEntry(DOLLAR_CARRIER);
  EXPECT_CALL(*_cau, matchCarrierActivationAppl(_, _, _)).WillOnce(Return(true));
  ASSERT_TRUE(_cau->matchCarrierActivationAppl(*_cxrActVector, *_activationDate, NULL));
}

TEST_F(CustomerActivationUtilTest, testMatchCarrierActivationApplTwoDiffrent)
{
  addCarrierEntry(BAD_CARRIER);
  addCarrierEntry(CARRIER_9B);
  EXPECT_CALL(*_cau, matchCarrierActivationAppl(_, _, _)).WillOnce(Return(false));
  ASSERT_FALSE(_cau->matchCarrierActivationAppl(*_cxrActVector, *_activationDate, NULL));
}

TEST_F(CustomerActivationUtilTest, testMatchCarrierActivationApplMiddleMan)
{
  addCarrierEntry(CARRIER_9B);
  addCarrierEntry(CARRIER_WS);
  addCarrierEntry(CARRIER_2R);
  EXPECT_CALL(*_cau, matchCarrierActivationAppl(_, _, _)).WillOnce(Return(true));
  ASSERT_TRUE(_cau->matchCarrierActivationAppl(*_cxrActVector, *_activationDate, NULL));
}
}
