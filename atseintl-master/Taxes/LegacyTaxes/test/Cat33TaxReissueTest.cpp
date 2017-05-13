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

#include "Taxes/LegacyTaxes/Cat33TaxReissue.h"
#include "gmock/gmock.h"

namespace tse
{

class ATaxReissueSelector : public testing::Test
{
public:
  TaxReissue first, second, third, fourth, fifth;
  std::vector<TaxReissue*> taxReissueVector;

  void SetUp() override
  {
    first.taxType() = "001";
    first.validationCxr() = {"ABC", "DEF"};
    second.taxType() = "002";
    second.validationCxr() = {"GHI", "ABC"};
    third.taxType() = "003";
    third.validationCxr() = {"DEF", "GHI"};
    fourth.taxType() = "004";
    fourth.validationCxr() = {};
    fifth.taxType() = "005";
    fifth.validationCxr() = {"ABC", "DEF"};
    fifth.cat33onlyInd() = 'Y';

    taxReissueVector.emplace_back(&first);
    taxReissueVector.emplace_back(&second);
    taxReissueVector.emplace_back(&third);
    taxReissueVector.emplace_back(&fourth);
    taxReissueVector.emplace_back(&fifth);
  }
};

TEST_F(ATaxReissueSelector, IsTaxReissueNotFoundIfTaxTypeMismatches)
{
  TaxReissueSelector selector(taxReissueVector);
  ASSERT_THAT(selector.getTaxReissue("105", "ABC"), testing::Eq(nullptr));
  ASSERT_THAT(selector.getTaxReissue("001", "GHI"), testing::Eq(nullptr));
  ASSERT_THAT(selector.getTaxReissue("002", "DEF"), testing::Eq(nullptr));
  ASSERT_THAT(selector.getTaxReissue("003", "ABC"), testing::Eq(nullptr));
  ASSERT_THAT(selector.getTaxReissue("005", "ABC", true), testing::Eq(nullptr));
}

TEST_F(ATaxReissueSelector, IsTaxReissueFoundIfTaxTypeMatch)
{
  TaxReissueSelector selector(taxReissueVector);
  ASSERT_THAT(selector.getTaxReissue("001", ""), testing::Eq(&first));
  ASSERT_THAT(selector.getTaxReissue("001", "ABC"), testing::Eq(&first));
  ASSERT_THAT(selector.getTaxReissue("001", "DEF"), testing::Eq(&first));
  ASSERT_THAT(selector.getTaxReissue("002", "ABC"), testing::Eq(&second));
  ASSERT_THAT(selector.getTaxReissue("002", "GHI"), testing::Eq(&second));
  ASSERT_THAT(selector.getTaxReissue("003", "DEF"), testing::Eq(&third));
  ASSERT_THAT(selector.getTaxReissue("003", "GHI"), testing::Eq(&third));
  ASSERT_THAT(selector.getTaxReissue("004", "ABC"), testing::Eq(&fourth));
  ASSERT_THAT(selector.getTaxReissue("004", "DEF"), testing::Eq(&fourth));
  ASSERT_THAT(selector.getTaxReissue("004", "GHI"), testing::Eq(&fourth));
  ASSERT_THAT(selector.getTaxReissue("004", "XYZ"), testing::Eq(&fourth));
  ASSERT_THAT(selector.getTaxReissue("004", ""), testing::Eq(&fourth));
  ASSERT_THAT(selector.getTaxReissue("005", "ABC", false), testing::Eq(&fifth));
}

class ACat33TaxReissue : public testing::Test
{
public:
  TaxReissue taxReissue;
};

TEST_F(ACat33TaxReissue, IsRefundIndYIfTaxReissueNullptr)
{
  Cat33TaxReissue cat33TaxReissue(nullptr);
  ASSERT_THAT(cat33TaxReissue.getRefundableTaxTag(), testing::Eq('Y'));
}

TEST_F(ACat33TaxReissue, IsRefundIndYIfRefundIndBlank)
{
  Cat33TaxReissue cat33TaxReissue(&taxReissue);
  ASSERT_THAT(cat33TaxReissue.getRefundableTaxTag(), testing::Eq('Y'));
}

TEST_F(ACat33TaxReissue, IsRefundIndYReundIndY)
{
  taxReissue.cat33onlyInd() = 'Y';
  taxReissue.refundInd() = 'Y';
  Cat33TaxReissue cat33TaxReissue(&taxReissue);
  ASSERT_THAT(cat33TaxReissue.getRefundableTaxTag(), testing::Eq('Y'));
}

} // end of tse namespace
