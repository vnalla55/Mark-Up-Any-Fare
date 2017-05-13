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

#include "Common/TaxName.h"
#include "Rules/PaymentRuleData.h"
#include "Rules/PreviousTicketApplicator.h"
#include "Rules/PreviousTicketRule.h"
#include "ServiceInterfaces/DefaultServices.h"
#include "ServiceInterfaces/FallbackService.h"
#include "ServiceInterfaces/PreviousTicketService.h"

#include "gmock/gmock.h"

#include <set>
#include <memory>

namespace tax
{
class PreviousTicketServiceStub : public PreviousTicketService
{
public:
  MOCK_CONST_METHOD0(getTaxesForPreviousTicket, const std::set<PreviousTicketTaxInfo>&());
  MOCK_CONST_METHOD1(getParentTaxes, std::set<std::string>(const std::string&));
};

class FallbackServiceStub : public FallbackService
{
public:
  bool isSet(const std::function<bool(const tse::Trx*)>&) const override { return false; }

  bool isSet(const std::function<bool()>&) const override { return false; }
};

class PreviousTicketRuleStub : public PreviousTicketRule
{
public:
  PreviousTicketRuleStub() : PreviousTicketRule(100000) {}
};

class APreviousTicketTaxesRule : public testing::Test
{
public:
  PreviousTicketRuleStub previousTicketRule;
  DefaultServices services;
  PreviousTicketServiceStub* previousTicketService;

  void SetUp() override
  {
    previousTicketService = new PreviousTicketServiceStub();
    services.setPreviousTicketService(previousTicketService);
  }
};

TEST_F(APreviousTicketTaxesRule, IsDescriptionContainsTaxesComputedForPreviousTicket)
{
  std::set<PreviousTicketTaxInfo> previousTicketTaxes{PreviousTicketTaxInfo("XG", 10, true),
                                                      PreviousTicketTaxInfo("XG5", 12, true)};

  std::cerr << "size=" << previousTicketTaxes.size() << "\n";

  EXPECT_CALL(*previousTicketService, getTaxesForPreviousTicket())
      .WillOnce(testing::ReturnRef(previousTicketTaxes));

  ASSERT_THAT(previousTicketRule.getDescription(services),
              testing::Eq("REQUIRED TAX NOT CALCULATED FOR PREVIOUS TICKET\n"
                          "TAXES COMPUTED FOR PREVIOUS TICKET:\n"
                          "XG XG5 \n"));
}

class PaymentDetailStub : public PaymentDetail
{
public:
  PaymentDetailStub()
    : PaymentDetail(
          PaymentRuleData(RulesRecord(), type::ProcessingGroup::Itinerary), Geo(), Geo(), TaxName())
  {
  }

  MOCK_CONST_METHOD0(taxName, const TaxName&());
};

class APreviousTicketTaxesApplicator : public testing::Test
{
public:
  PreviousTicketRuleStub previousTicketRule;
  std::unique_ptr<PreviousTicketApplicator> previousTicketApplicator;
  DefaultServices services;
  TaxName expectedTaxName;
  PaymentDetailStub paymentDetailStub;
  PreviousTicketServiceStub* previousTicketService;
  const type::Percent percent{10};

  void SetUp() override
  {
    previousTicketApplicator.reset(new PreviousTicketApplicator(&previousTicketRule, services, percent));

    expectedTaxName = createTaxName("CA", "XG", "002");
    previousTicketService = new PreviousTicketServiceStub();
    services.setPreviousTicketService(previousTicketService);
    services.setFallbackService(new FallbackServiceStub());
  }

  TaxName
  createTaxName(const std::string& nation, const std::string& taxCode, const std::string& taxType)
  {
    TaxName taxName;
    codeFromString(nation, taxName.nation());
    codeFromString(taxCode, taxName.taxCode());
    codeFromString(taxType, taxName.taxType());

    return taxName;
  }
};

TEST_F(APreviousTicketTaxesApplicator, ReturnsTrueIfParentTaxesIsEmpty)
{
  EXPECT_CALL(*previousTicketService, getParentTaxes("XG2"))
      .WillOnce(testing::Return(std::set<std::string>{}));
  EXPECT_CALL(paymentDetailStub, taxName()).WillOnce(testing::ReturnRef(expectedTaxName));
  ASSERT_TRUE(previousTicketApplicator->apply(paymentDetailStub));
}

TEST_F(APreviousTicketTaxesApplicator, ReturnsFalseIfParentTaxesIsNotInTaxesForPreviousTicket)
{
  std::set<PreviousTicketTaxInfo> previousTicketTaxes{PreviousTicketTaxInfo("XG2", 10, true)};

  EXPECT_CALL(*previousTicketService, getParentTaxes("XG2"))
      .WillOnce(testing::Return(std::set<std::string>{"XQ5"}));
  EXPECT_CALL(*previousTicketService, getTaxesForPreviousTicket())
      .WillOnce(testing::ReturnRef(previousTicketTaxes));
  EXPECT_CALL(paymentDetailStub, taxName()).WillOnce(testing::ReturnRef(expectedTaxName));
  ASSERT_FALSE(previousTicketApplicator->apply(paymentDetailStub));
}

TEST_F(APreviousTicketTaxesApplicator,
       ReturnsFalseIfParentTaxesAreInTaxesForPreviousTicketButCanadaExcIsFalse)
{
  std::set<PreviousTicketTaxInfo> previousTicketTaxes{PreviousTicketTaxInfo("XG2", 10, false)};

  EXPECT_CALL(*previousTicketService, getParentTaxes("XG2"))
      .WillOnce(testing::Return(std::set<std::string>{"XG2"}));
  EXPECT_CALL(*previousTicketService, getTaxesForPreviousTicket())
      .WillOnce(testing::ReturnRef(previousTicketTaxes));
  EXPECT_CALL(paymentDetailStub, taxName()).WillOnce(testing::ReturnRef(expectedTaxName));
  ASSERT_FALSE(previousTicketApplicator->apply(paymentDetailStub));
}

TEST_F(APreviousTicketTaxesApplicator,
       ReturnsFalseIfParantTaxIsInTaxesForPreviousTicketAndPercentageMismatch)
{
  previousTicketApplicator.reset(new PreviousTicketApplicator(&previousTicketRule, services, 20));
  std::set<PreviousTicketTaxInfo> previousTicketTaxes{PreviousTicketTaxInfo("XG2", 10, true),
                                                      PreviousTicketTaxInfo("XG3", 12, true)};

  EXPECT_CALL(*previousTicketService, getParentTaxes(testing::_))
      .WillOnce(testing::Return(std::set<std::string>{"XG2"}));
  EXPECT_CALL(*previousTicketService, getTaxesForPreviousTicket())
      .WillOnce(testing::ReturnRef(previousTicketTaxes));
  EXPECT_CALL(paymentDetailStub, taxName()).WillOnce(testing::ReturnRef(expectedTaxName));
  ASSERT_FALSE(previousTicketApplicator->apply(paymentDetailStub));
}

TEST_F(APreviousTicketTaxesApplicator, ReturnsTrueIfParentTaxesIsInTaxesForPreviousTicket)
{
  std::set<PreviousTicketTaxInfo> previousTicketTaxes{PreviousTicketTaxInfo("XG2", 10, true),
                                                      PreviousTicketTaxInfo("XG3", 12, true)};

  EXPECT_CALL(*previousTicketService, getParentTaxes(testing::_))
      .WillOnce(testing::Return(std::set<std::string>{"XG2"}));
  EXPECT_CALL(*previousTicketService, getTaxesForPreviousTicket())
      .WillOnce(testing::ReturnRef(previousTicketTaxes));
  EXPECT_CALL(paymentDetailStub, taxName()).WillOnce(testing::ReturnRef(expectedTaxName));
  ASSERT_TRUE(previousTicketApplicator->apply(paymentDetailStub));
}

TEST_F(APreviousTicketTaxesApplicator, ReturnsFalseIfParentTaxDoesNotMatchTaxesForPreviousTicket)
{
  std::set<PreviousTicketTaxInfo> previousTicketTaxes{PreviousTicketTaxInfo("XG2", 10, true),
                                                      PreviousTicketTaxInfo("XG3", 12, true)};

  EXPECT_CALL(*previousTicketService, getParentTaxes(testing::_))
      .WillOnce(testing::Return(std::set<std::string>{"XR*"}));
  EXPECT_CALL(*previousTicketService, getTaxesForPreviousTicket())
      .WillOnce(testing::ReturnRef(previousTicketTaxes));
  EXPECT_CALL(paymentDetailStub, taxName()).WillOnce(testing::ReturnRef(expectedTaxName));
  ASSERT_FALSE(previousTicketApplicator->apply(paymentDetailStub));
}

TEST_F(APreviousTicketTaxesApplicator, ReturnsTrueIfParentTaxMatchTaxesForPreviousTicket)
{
  std::set<PreviousTicketTaxInfo> previousTicketTaxes{PreviousTicketTaxInfo("XG2", 10, true),
                                                      PreviousTicketTaxInfo("XG3", 12, true)};

  EXPECT_CALL(*previousTicketService, getParentTaxes(testing::_))
      .WillOnce(testing::Return(std::set<std::string>{"XG*"}));
  EXPECT_CALL(*previousTicketService, getTaxesForPreviousTicket())
      .WillOnce(testing::ReturnRef(previousTicketTaxes));
  EXPECT_CALL(paymentDetailStub, taxName()).WillOnce(testing::ReturnRef(expectedTaxName));
  ASSERT_TRUE(previousTicketApplicator->apply(paymentDetailStub));
}
}
