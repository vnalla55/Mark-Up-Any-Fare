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

#include "DataModel/Services/PreviousTicketTaxInfo.h"

#include "gmock/gmock.h"

namespace tax
{
TEST(APreviousTicketTaxInfo, EqualsIfSabreTaxCodeAndPercentAndIsCanadaExchTrue)
{
  PreviousTicketTaxInfo first("XG", 10, true);
  PreviousTicketTaxInfo second("XG", 10, true);

  ASSERT_FALSE(first < second);
  ASSERT_FALSE(second < first);
}

TEST(APreviousTicketTaxInfo, NotEqualsIfSabreTaxCodeDifferes)
{
  PreviousTicketTaxInfo first("XG", 10, true);
  PreviousTicketTaxInfo second("XG5", 10, true);

  ASSERT_TRUE(first < second);
}

TEST(APreviousTicketTaxInfo, NotEqualsIfPercentageDiffers)
{
  PreviousTicketTaxInfo first("XG", 10, true);
  PreviousTicketTaxInfo second("XG", 12, true);

  ASSERT_TRUE(first < second);
}

TEST(APreviousTicketTaxInfo, NotEqualsIfIsCanadaExchDiffers)
{
  PreviousTicketTaxInfo first("XG", 10, false);
  PreviousTicketTaxInfo second("XG", 10, true);

  ASSERT_TRUE(first < second);
}

} // end of tax namespace
