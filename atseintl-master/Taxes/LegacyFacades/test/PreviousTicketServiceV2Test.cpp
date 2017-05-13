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

#include "DataModel/PricingTrx.h"
#include "LegacyFacades/PreviousTicketServiceV2.h"

#include "gmock/gmock.h"
#include <set>

namespace tax
{
class UtcConfigStub : public tse::UtcConfig
{
  mutable std::map<std::string, std::set<std::string>> _config;

public:
  UtcConfigStub(tse::PricingTrx& trx) : tse::UtcConfig(trx) {}

  void readConfig(const std::string& configName) override {}

  const std::map<std::string, std::set<std::string>>& get() const override { return _config; }

  const std::set<std::string>& get(const std::string& paramName) const override
  {
    return _config[paramName];
  }
};

class APreviousTicketService : public testing::Test
{
public:
  tse::PricingTrx trx;
  std::unique_ptr<tse::PreviousTicketServiceV2> previousTicketService;
  std::set<tse::PreviousTicketTaxInfo> parentTaxes;

  APreviousTicketService() {}

  void SetUp() override
  {
    previousTicketService.reset(new tse::PreviousTicketServiceV2(trx, new UtcConfigStub(trx)));
  }
};

TEST_F(APreviousTicketService, ReturnEmptyVectorIfNoTaxesForPreviousTicket)
{
  ASSERT_THAT(previousTicketService->getTaxesForPreviousTicket().size(), testing::Eq(0));
}

TEST_F(APreviousTicketService, ReturnNonEmptyVectorIfTaxesForPreviousTicket)
{
  parentTaxes.emplace("XG2", 10, true);
  parentTaxes.emplace("XG3", 12, true);
  parentTaxes.emplace("XG4", 16, true);

  trx.addParentTaxes(parentTaxes);

  previousTicketService.reset(new tse::PreviousTicketServiceV2(trx, new UtcConfigStub(trx)));

  ASSERT_THAT(previousTicketService->getTaxesForPreviousTicket().size(), testing::Eq(3));
}

TEST_F(APreviousTicketService, IsParamNamePARENTTAXESXG2)
{
  ASSERT_THAT(previousTicketService->getParamName("XG2"), testing::Eq("PARENTTAXESXG2"));
}
}
