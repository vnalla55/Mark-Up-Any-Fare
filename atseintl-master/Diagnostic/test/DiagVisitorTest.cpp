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

#include "DataModel/ExcItin.h"
#include "DataModel/Itin.h"
#include "DataModel/RexBaseTrx.h"
#include "DataModel/RefundPricingTrx.h"
#include "Diagnostic/DiagVisitor.h"
#include "Diagnostic/Diag817Collector.h"
#include "Diagnostic/Diag825Collector.h"
#include "DataModel/RefundPricingTrx.h"
#include "Taxes/LegacyTaxes/Cat33TaxReissue.h"
#include "Taxes/LegacyFacades/ItinSelector.h"

#include "gmock/gmock.h"
#include <vector>

namespace tse
{

class APrintHeader : public testing::Test
{
public:
  Diag825Collector diag;
  RefundPricingTrx trx;

  void SetUp() override
  {

  }
};

TEST_F(APrintHeader, IsHeaderEqualToExpected)
{
  PrintHeader visitor(trx);
  diag.accept(visitor);

  std::string expected = "- BEGIN EXC ITIN DIAGNOSTIC -\n"
      "***************************************************************\n"
      "                REFUNDABLE INDICATOR\n"
      "***************************************************************\n"
      "    CODE  CARRIER  REFUNDIND      SEQ\n";

  ASSERT_THAT(diag.str(), testing::Eq(expected));
}

TEST_F(APrintHeader, IsExcItinHeaderIfExcItin)
{
  trx.trxPhase() = RexBaseTrx::MATCH_EXC_RULE_PHASE;
  PrintHeader visitor(trx);
  diag.accept(visitor);

  std::string expected = "- BEGIN EXC ITIN DIAGNOSTIC -\n"
      "***************************************************************\n"
      "                REFUNDABLE INDICATOR\n"
      "***************************************************************\n"
      "    CODE  CARRIER  REFUNDIND      SEQ\n";

  ASSERT_THAT(diag.str(), testing::Eq(expected));
}

TEST_F(APrintHeader, IsNewItinHeaderIfNewItin)
{
  trx.trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
  PrintHeader visitor(trx);
  diag.accept(visitor);

  std::string expected = "- BEGIN NEW ITIN DIAGNOSTIC -\n"
      "***************************************************************\n"
      "                REFUNDABLE INDICATOR\n"
      "***************************************************************\n"
      "    CODE  CARRIER  REFUNDIND      SEQ\n";

  ASSERT_THAT(diag.str(), testing::Eq(expected));
}

class APrintTaxReissueInfo : public testing::Test
{
public:
  Diag825Collector diag;
};

TEST_F(APrintTaxReissueInfo, IsSeqZeroIfTaxReissueIsNull)
{
  Cat33TaxReissue cat33TaxReissue(nullptr);
  PrintTaxReissueInfo printTaxReissueInfo(cat33TaxReissue, "US1", "LH");
  diag.accept(printTaxReissueInfo);

  std::string expected = "  1  US1       LH          Y        0\n";

  ASSERT_THAT(diag.str(), testing::Eq(expected));
}

TEST_F(APrintTaxReissueInfo, IsSeqEquals123IfTaxReissueIsNotNull)
{
  TaxReissue taxReissue;
  taxReissue.seqNo() = 123;
  Cat33TaxReissue cat33TaxReissue(&taxReissue);
  PrintTaxReissueInfo printTaxReissueInfo(cat33TaxReissue, "US1", "LH");
  diag.accept(printTaxReissueInfo);

  std::string expected = "  1  US1       LH          Y      123\n";

  ASSERT_THAT(diag.str(), testing::Eq(expected));
}

class Diag817Mock : public Diag817Collector
{
public:
  MOCK_METHOD4(displayDiag817, void(std::vector<TaxItem*>&, DiagCollector&, uint32_t&, bool));
};

class ItinSelectorMock : public ItinSelector
{
public:
  ItinSelectorMock(PricingTrx& trx) : ItinSelector(trx) {}
  MOCK_CONST_METHOD0(get, std::vector<Itin*>());
  MOCK_CONST_METHOD0(getItin, std::vector<Itin*>());
  MOCK_CONST_METHOD0(isExcItin, bool());
  MOCK_CONST_METHOD0(isCat33FullRefund, bool());
  MOCK_CONST_METHOD0(isNewItin, bool());
};

class APrintTaxesOnChangeFee : public testing::Test
{
public:
  Diag817Collector diag;
  RefundPricingTrx trx;
};

TEST_F(APrintTaxesOnChangeFee, IsBlankIfExcItin)
{
  ItinSelectorMock itinSelectorMock(trx);
  EXPECT_CALL(itinSelectorMock, isNewItin()).WillOnce(testing::Return(false));
  PrintTaxesOnChangeFee printTaxesOnChangeFee(itinSelectorMock);
  diag.accept(printTaxesOnChangeFee);

  ASSERT_THAT(diag.str(), testing::Eq(""));
}

TEST_F(APrintTaxesOnChangeFee, IsTaxOnChangeFeePrintedIfNewItinEmpty)
{
  Diag817Mock diagMock;

  ExcItin excItin;
  TaxResponse taxResponse;
  excItin.mutableTaxResponses().push_back(&taxResponse);

  ItinSelectorMock itinSelectorMock(trx);
  EXPECT_CALL(itinSelectorMock, isNewItin()).WillOnce(testing::Return(true));
  EXPECT_CALL(itinSelectorMock, getItin()).WillOnce(testing::Return(std::vector<Itin*>{}));
  EXPECT_CALL(itinSelectorMock, get()).WillOnce(testing::Return(std::vector<Itin*> { &excItin} ));

  PrintTaxesOnChangeFee printTaxesOnChangeFee(itinSelectorMock);
  diag.accept(printTaxesOnChangeFee);
}

} // end of tse namespace
