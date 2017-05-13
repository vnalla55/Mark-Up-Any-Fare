// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexBaseTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxNation.h"
#include "Taxes/LegacyFacades/ItinSelector.h"
#include "Taxes/LegacyTaxes/GetTaxCodeRegAdapter.h"
#include "Taxes/LegacyTaxes/GetTaxNation.h"
#include "Taxes/LegacyTaxes/GetTicketingDate.h"
#include "Taxes/LegacyTaxes/TaxOnChangeFeeConfig.h"
#include "Taxes/LegacyTaxes/TaxOnChangeFeeDriver.h"

#include "gmock/gmock.h"

#include <vector>

namespace tse
{

class GetTaxNationMock : public AbstractGetTaxNation
{
public:
  MOCK_CONST_METHOD0(get, const std::vector<const TaxNation*>&());
};

class GetTaxCodeRegMock : public AbstractGetTaxCodeReg
{
public:
  MOCK_CONST_METHOD1(get, const std::vector<TaxCodeReg*>*(const TaxCode&));
};

class TaxOnChangeFeeConfigMock : public TaxOnChangeFeeConfig
{
public:
  MOCK_CONST_METHOD1(isTaxOnChangeFee, bool(TaxCodeReg*));
};

class ATaxOnChangeFeeDriver : public testing::Test
{
public:
  GetTaxNationMock getTaxNation;
  GetTaxCodeRegMock getTaxCodeReg;
  PricingRequest pricingRequest;
  TaxOnChangeFeeConfigMock taxOnChangeFeeConfig;
};

TEST_F(ATaxOnChangeFeeDriver, IsTaxCodeRegEmptyWhenAutomatedExchnage)
{
  RexPricingTrx trx;
  trx.setRequest(&pricingRequest);
  TaxOnChangeFeeDriver driver(trx, getTaxNation, getTaxCodeReg, taxOnChangeFeeConfig);
  ASSERT_THAT(driver.getTaxCodeReg().size(), testing::Eq(0));
}

TEST_F(ATaxOnChangeFeeDriver, IsGetTaxNationNotCalledIfAutomatedExchange)
{
  EXPECT_CALL(getTaxNation, get()).Times(0);

  RexPricingTrx trx;
  trx.setRequest(&pricingRequest);
  TaxOnChangeFeeDriver driver(trx, getTaxNation, getTaxCodeReg, taxOnChangeFeeConfig);
}

TEST_F(ATaxOnChangeFeeDriver, IsGetTaxNationNotCalledIfAutomatedReissueAndExcItin)
{
  EXPECT_CALL(getTaxNation, get()).Times(0);

  RefundPricingTrx trx;
  trx.trxPhase() = RexBaseTrx::REPRICE_EXCITIN_PHASE;
  trx.setRequest(&pricingRequest);
  TaxOnChangeFeeDriver driver(trx, getTaxNation, getTaxCodeReg, taxOnChangeFeeConfig);
}

TEST_F(ATaxOnChangeFeeDriver, IsGetTaxNationCalledWhenAutomatedRefund)
{
  std::vector<const TaxNation*> taxNations{};
  EXPECT_CALL(getTaxNation, get()).WillOnce(testing::ReturnRef(taxNations));

  RefundPricingTrx trx;
  trx.setRequest(&pricingRequest);
  TaxOnChangeFeeDriver driver(trx, getTaxNation, getTaxCodeReg, taxOnChangeFeeConfig);
}

TEST_F(ATaxOnChangeFeeDriver, IsTaxCodeRegsGetFromGetTaxCodeReg)
{
  std::unique_ptr<TaxNation> taxNation1(new TaxNation());
  taxNation1->taxCodeOrder().push_back("JR2");
  std::vector<const TaxNation*> taxNations{ taxNation1.get() };
  EXPECT_CALL(getTaxNation, get()).WillOnce(testing::ReturnRef(taxNations));

  std::vector<TaxCodeReg*> taxCodeRegs;
  EXPECT_CALL(getTaxCodeReg, get(testing::_)).WillOnce(testing::Return(&taxCodeRegs));

  RefundPricingTrx trx;
  trx.setRequest(&pricingRequest);
  TaxOnChangeFeeDriver driver(trx, getTaxNation, getTaxCodeReg, taxOnChangeFeeConfig);
}

TEST_F(ATaxOnChangeFeeDriver, IsTaxCodeReqSizeEqualsTo4)
{
  std::unique_ptr<TaxNation> taxNation1(new TaxNation());
  taxNation1->taxCodeOrder().push_back("JR2");
  taxNation1->taxCodeOrder().push_back("PL1");
  std::unique_ptr<TaxNation> taxNation2(new TaxNation());
  taxNation2->taxCodeOrder().push_back("US1");
  taxNation2->taxCodeOrder().push_back("US2");
  std::vector<const TaxNation*> taxNations{ taxNation1.get(), taxNation2.get() };
  EXPECT_CALL(getTaxNation, get()).WillOnce(testing::ReturnRef(taxNations));

  TaxCodeReg taxCodeReg;
  std::vector<TaxCodeReg*> taxCodeRegs { &taxCodeReg };
  EXPECT_CALL(getTaxCodeReg, get(testing::_)).Times(4).WillRepeatedly(testing::Return(&taxCodeRegs));

  EXPECT_CALL(taxOnChangeFeeConfig, isTaxOnChangeFee(testing::_)).Times(4).WillRepeatedly(testing::Return(true));

  RefundPricingTrx trx;
  trx.setRequest(&pricingRequest);
  TaxOnChangeFeeDriver driver(trx, getTaxNation, getTaxCodeReg, taxOnChangeFeeConfig);
  ASSERT_THAT(driver.getTaxCodeReg().size(), testing::Eq(4));
}

TEST_F(ATaxOnChangeFeeDriver, IfTaxCodeRegSizeEqualsTo1IfJR2IsOnChangeFee)
{
  std::unique_ptr<TaxNation> taxNation1(new TaxNation());
  taxNation1->taxCodeOrder().push_back("JR2");
  taxNation1->taxCodeOrder().push_back("PL1");
  std::unique_ptr<TaxNation> taxNation2(new TaxNation());
  taxNation2->taxCodeOrder().push_back("US1");
  taxNation2->taxCodeOrder().push_back("US2");
  std::vector<const TaxNation*> taxNations{ taxNation1.get(), taxNation2.get() };
  EXPECT_CALL(getTaxNation, get()).WillOnce(testing::ReturnRef(taxNations));

  TaxCodeReg taxCodeReg;
  std::vector<TaxCodeReg*> taxCodeRegs { &taxCodeReg };
  EXPECT_CALL(getTaxCodeReg, get(testing::_)).Times(4).WillRepeatedly(testing::Return(&taxCodeRegs));

  EXPECT_CALL(taxOnChangeFeeConfig, isTaxOnChangeFee(testing::_))
    .WillOnce(testing::Return(false))
    .WillOnce(testing::Return(false))
    .WillOnce(testing::Return(true))
    .WillOnce(testing::Return(false));

  RefundPricingTrx trx;
  trx.setRequest(&pricingRequest);
  TaxOnChangeFeeDriver driver(trx, getTaxNation, getTaxCodeReg, taxOnChangeFeeConfig);
  ASSERT_THAT(driver.getTaxCodeReg().size(), testing::Eq(1));
}
} // end of tse namespace
