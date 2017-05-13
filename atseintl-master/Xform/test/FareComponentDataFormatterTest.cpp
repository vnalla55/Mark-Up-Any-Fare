// ----------------------------------------------------------------
//
//   Copyright Sabre 2015
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

#include "Xform/FareComponentDataFormatter.h"

#include "Common/XMLConstruct.h"
#include "DataModel/StructuredRuleData.h"
#include "Pricing/StructuredFareRulesUtils.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace tse
{
using namespace ::testing;

class FareComponentDataFormatterTest : public Test
{
};
// ________________________________________________________________________________________________
// Test  description:
//
// Checking only if XML tag related with Minimum Stay category
//
// ________________________________________________________________________________________________
TEST_F(FareComponentDataFormatterTest, testMinimumStayXMLDataExists)
{
  XMLConstruct construct;
  StructuredRuleData structuredData;
  structuredData._minStayDate = DateTime(2008, 2, 13, 12, 0, 0);
  structuredData._minStayLocation = LocCode("LON");

  FareComponentDataFormatter formatter(construct);
  formatter.format(structuredData, 1, 2);
  EXPECT_THAT(construct.getXMLData(),
              MatchesRegex("<FCD Q6D=\"1\" PUN=\"2\".*<MIN MSD=.*MST=.*LOC=.*</FCD>"));
}

// ________________________________________________________________________________________________
// Test  description:
//
// We prepare some fare components and check if they are correctly processed
//
// ________________________________________________________________________________________________
TEST_F(FareComponentDataFormatterTest, testMinimumStayDataAreProccessedCorrectly)
{
  XMLConstruct construct;
  StructuredRuleData structuredData;
  FareComponentDataFormatter formatter(construct);

  structuredData._minStayLocation = LocCode("LON");
  structuredData._minStayDate = DateTime(2008, 2, 13, 12, 0, 0);

  formatter.format(structuredData, 1, 1);

  structuredData._minStayLocation = LocCode("SIN");
  structuredData._minStayDate = DateTime(2018, 2, 13, 12, 0, 0);

  formatter.format(structuredData, 2, 1);

  structuredData._minStayLocation = LocCode("BRU");
  structuredData._minStayDate = DateTime(2018, 2, 14, 12, 0, 0);

  formatter.format(structuredData, 3, 2);

  structuredData._minStayLocation = LocCode("LON");
  structuredData._minStayDate = DateTime(2018, 2, 14, 12, 0, 1);

  formatter.format(structuredData, 4, 2);
  ASSERT_STREQ(
      construct.getXMLData().c_str(),
      "<FCD Q6D=\"1\" PUN=\"1\"><MIN MSD=\"2008-02-13\" MST=\"12:00\" LOC=\"LON\"/></FCD><FCD "
      "Q6D=\"2\" PUN=\"1\"><MIN MSD=\"2018-02-13\" MST=\"12:00\" LOC=\"SIN\"/></FCD><FCD "
      "Q6D=\"3\" PUN=\"2\"><MIN MSD=\"2018-02-14\" MST=\"12:00\" LOC=\"BRU\"/></FCD><FCD "
      "Q6D=\"4\" PUN=\"2\"><MIN MSD=\"2018-02-14\" MST=\"12:00\" LOC=\"LON\"/></FCD>");
}

// ________________________________________________________________________________________________
// Test  description:
//
// Checking only if XML tag related with Maximum Stay category
//
// ________________________________________________________________________________________________
TEST_F(FareComponentDataFormatterTest, testMaximumStayXMLDataExists)
{
  XMLConstruct construct;
  StructuredRuleData structuredData;
  MaxStayMap& maxStayMap = structuredData._maxStayMostRestrictiveFCData;

  structuredFareRulesUtils::updateMaxStayTrvCommenceData(
      maxStayMap, 1, DateTime(2018, 2, 13, 12, 0, 1), LocCode("LON"));

  structuredFareRulesUtils::updateMaxStayTrvCompleteData(
      maxStayMap, 1, DateTime(2000, 2, 14, 12, 0, 1), LocCode("LON"));

  FareComponentDataFormatter formatter(construct);
  formatter.format(structuredData, 1, 1);
  EXPECT_THAT(construct.getXMLData(),
              MatchesRegex("<FCD Q6D=\"1\" PUN=\"1\"><MAX MSD=.*MST=.*LDC=.*LTC=.*LOC=.*</FCD>"));
}

// ________________________________________________________________________________________________
// Test  description:
//
// Check if data for fare components is correctly processed for cat 7 / cat 14
// We check only commence restriction
//
// ________________________________________________________________________________________________
TEST_F(FareComponentDataFormatterTest,
       testMaximumStayDataAreProccessedCorrectly_CommenceRestriction)
{
  XMLConstruct construct;
  StructuredRuleData structuredData;
  MaxStayMap& maxStayMap = structuredData._maxStayMostRestrictiveFCData;
  FareComponentDataFormatter formatter(construct);

  structuredFareRulesUtils::updateMaxStayTrvCommenceData(
      maxStayMap, 1, DateTime(2008, 2, 13, 12, 0, 0), LocCode("LON"));

  formatter.format(structuredData, 1, 1);

  StructuredRuleData structuredData2;
  MaxStayMap& maxStayMap2 = structuredData2._maxStayMostRestrictiveFCData;
  structuredFareRulesUtils::updateMaxStayTrvCommenceData(
      maxStayMap2, 1, DateTime(2018, 2, 13, 12, 0, 0), LocCode("SIN"));

  formatter.format(structuredData2, 2, 1);

  StructuredRuleData structuredData3;
  MaxStayMap& maxStayMap3 = structuredData3._maxStayMostRestrictiveFCData;
  structuredFareRulesUtils::updateMaxStayTrvCommenceData(
      maxStayMap3, 1, DateTime(2018, 2, 14, 12, 0, 0), LocCode("BRU"));

  formatter.format(structuredData3, 3, 2);

  StructuredRuleData structuredData4;
  MaxStayMap& maxStayMap4 = structuredData4._maxStayMostRestrictiveFCData;
  structuredFareRulesUtils::updateMaxStayTrvCommenceData(
      maxStayMap4, 1, DateTime(2018, 2, 14, 12, 0, 1), LocCode("LON"));

  formatter.format(structuredData4, 4, 2);
  ASSERT_STREQ(construct.getXMLData().c_str(),
               "<FCD Q6D=\"1\" PUN=\"1\"><MAX MSD=\"2008-02-13\" MST=\"12:00\" "
               "LOC=\"LON\"/></FCD><FCD Q6D=\"2\" PUN=\"1\"><MAX MSD=\"2018-02-13\" "
               "MST=\"12:00\" LOC=\"SIN\"/></FCD><FCD Q6D=\"3\" PUN=\"2\"><MAX "
               "MSD=\"2018-02-14\" MST=\"12:00\" LOC=\"BRU\"/></FCD><FCD Q6D=\"4\" "
               "PUN=\"2\"><MAX MSD=\"2018-02-14\" MST=\"12:00\" LOC=\"LON\"/></FCD>");
}

// ________________________________________________________________________________________________
// Test  description:
//
// Check if data for fare components is correctly processed for cat 7 / cat 14
// We check commence and complete restriction as well
//
// ________________________________________________________________________________________________
TEST_F(FareComponentDataFormatterTest,
       testMaximumStayDataAreProccessedCorrectly_CommenceAndCompleteRestriction)
{
  XMLConstruct construct;
  StructuredRuleData structuredData;
  MaxStayMap& maxStayMap = structuredData._maxStayMostRestrictiveFCData;
  FareComponentDataFormatter formatter(construct);

  structuredFareRulesUtils::updateMaxStayTrvCommenceData(
      maxStayMap, 1, DateTime(2008, 2, 13, 12, 0, 0), LocCode("LON"));

  formatter.format(structuredData, 1, 1);

  StructuredRuleData structuredData2;
  MaxStayMap& maxStayMap2 = structuredData2._maxStayMostRestrictiveFCData;
  structuredFareRulesUtils::updateMaxStayTrvCommenceData(
      maxStayMap2, 1, DateTime(2018, 2, 13, 12, 0, 0), LocCode("SIN"));

  formatter.format(structuredData2, 2, 1);

  StructuredRuleData structuredData3;
  MaxStayMap& maxStayMap3 = structuredData3._maxStayMostRestrictiveFCData;
  structuredFareRulesUtils::updateMaxStayTrvCommenceData(
      maxStayMap3, 1, DateTime(2018, 2, 14, 12, 0, 0), LocCode("BRU"));

  structuredFareRulesUtils::updateMaxStayTrvCompleteData(
      maxStayMap3, 1, DateTime(4018, 2, 14, 15, 4, 0), LocCode("BRU"));

  formatter.format(structuredData3, 3, 2);

  StructuredRuleData structuredData4;
  MaxStayMap& maxStayMap4 = structuredData4._maxStayMostRestrictiveFCData;
  structuredFareRulesUtils::updateMaxStayTrvCommenceData(
      maxStayMap4, 1, DateTime(2018, 2, 14, 12, 0, 1), LocCode("LON"));

  structuredFareRulesUtils::updateMaxStayTrvCompleteData(
      maxStayMap4, 1, DateTime(4018, 2, 14, 15, 4, 0), LocCode("LON"));
  formatter.format(structuredData4, 4, 2);

  ASSERT_STREQ(construct.getXMLData().c_str(),
               "<FCD Q6D=\"1\" PUN=\"1\"><MAX MSD=\"2008-02-13\" MST=\"12:00\" "
               "LOC=\"LON\"/></FCD><FCD Q6D=\"2\" PUN=\"1\"><MAX MSD=\"2018-02-13\" "
               "MST=\"12:00\" LOC=\"SIN\"/></FCD><FCD Q6D=\"3\" PUN=\"2\"><MAX "
               "MSD=\"2018-02-14\" MST=\"12:00\" LDC=\"4018-02-14\" LTC=\"15:04\" "
               "LOC=\"BRU\"/></FCD><FCD Q6D=\"4\" "
               "PUN=\"2\"><MAX MSD=\"2018-02-14\" MST=\"12:00\" LDC=\"4018-02-14\" LTC=\"15:04\" "
               "LOC=\"LON\"/></FCD>");
}

// ________________________________________________________________________________________________
// Test  description:
//
// Check if advance reservation is presented when exists and valid
//
// ________________________________________________________________________________________________
TEST_F(FareComponentDataFormatterTest, testAdvanceReservation)
{
  XMLConstruct construct;
  StructuredRuleData structuredData;
  structuredData._advanceReservation = DateTime(2016, 2, 12, 23, 59, 0);

  FareComponentDataFormatter formatter(construct);
  formatter.format(structuredData, 1, 1);
  ASSERT_STREQ(construct.getXMLData().c_str(),
               "<FCD Q6D=\"1\" PUN=\"1\"><ADV LDB=\"2016-02-12\" LTB=\"23:59\"/></FCD>");
}

// ________________________________________________________________________________________________
// Test  description:
//
// Check if advance reservation is not presented when date is invalid
//
// ________________________________________________________________________________________________
TEST_F(FareComponentDataFormatterTest, testAdvanceReservationInvalidDate)
{
  XMLConstruct construct;
  StructuredRuleData structuredData;
  structuredData._advanceReservation = DateTime::openDate();// default value

  FareComponentDataFormatter formatter(construct);
  formatter.format(structuredData, 1, 1);
  ASSERT_STREQ(construct.getXMLData().c_str(), "<FCD Q6D=\"1\" PUN=\"1\"/>");
}
}
