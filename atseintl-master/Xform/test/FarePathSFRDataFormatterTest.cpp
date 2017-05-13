// ----------------------------------------------------------------
//
//   Copyright Sabre 2016
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "Xform/FarePathSFRDataFormatter.h"

#include "Common/XMLConstruct.h"
#include "DataModel/StructuredRuleData.h"
#include "DataModel/FarePath.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace tse
{
using namespace ::testing;

class FarePathSFRDataFormatterTest : public Test
{
};

// ________________________________________________________________________________________________
// Test description:
//
// If there isn't any most restrictive data,
// MRJ data shouldn't occur
// ________________________________________________________________________________________________

TEST_F(FarePathSFRDataFormatterTest, testMostRestrictiveDataNotExists)
{
  XMLConstruct construct;
  MostRestrictiveJourneySFRData mostRestrictiveData;
  FarePathSFRDataFormatter formatter(construct);
  formatter.format(mostRestrictiveData);
  EXPECT_THAT(construct.getXMLData(), MatchesRegex(".*[^M][^R][^J].*"));
}

// ________________________________________________________________________________________________
// Test description:
//
// If we have most restrictive Advance Reservation data,
// Formatter should print data within MRJ tags
// ________________________________________________________________________________________________

TEST_F(FarePathSFRDataFormatterTest, testMostRestrictiveDataExists)
{
  XMLConstruct construct;
  MostRestrictiveJourneySFRData mostRestrictiveData;
  mostRestrictiveData._advanceReservation = DateTime(2016, 2, 12, 23, 59, 0);
  FarePathSFRDataFormatter formatter(construct);
  formatter.format(mostRestrictiveData);
  ASSERT_STREQ(construct.getXMLData().c_str(),
               "<MRJ><ADV LDB=\"2016-02-12\" LTB=\"23:59\"/></MRJ>");
}
}
