// -------------------------------------------------------------------
//
//  Copyright (C) Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
// -------------------------------------------------------------------

#include <gtest/gtest.h>

#include "test/include/GtestHelperMacros.h"
#include "DataModel/FlexFares/GroupsData.h"
#include "DataModel/FlexFares/GroupsDataAnalyzer.h"
#include "DataModel/FlexFares/TotalAttrs.h"
#include "DataModel/FlexFares/Types.h"

#include "Common/TseConsts.h"
#include "DataModel/ShoppingTrx.h"

#include "test/include/TestMemHandle.h"

namespace tse
{
namespace flexFares
{

class GroupsDataAnalyzerTest : public ::testing::Test
{
};

TEST_F(GroupsDataAnalyzerTest, testCorpIds)
{
  TotalAttrs totalAttrs;
  GroupsData ffgData;

  std::string corpA("a"), corpB("b"), corpC("c"), corpD("d");
  // a -> 1, 2, 4, 5
  // b -> 1, 3
  // c -> 2, 3, 4
  // d -> 2,
  ffgData.addCorpId(corpA, 1u);
  ffgData.addCorpId(corpB, 1u);
  ffgData.addCorpId(corpA, 2u);
  ffgData.addCorpId(corpC, 2u);
  ffgData.addCorpId(corpD, 2u);
  ffgData.addCorpId(corpB, 3u);
  ffgData.addCorpId(corpC, 3u);
  ffgData.addCorpId(corpA, 4u);
  ffgData.addCorpId(corpC, 4u);
  ffgData.addCorpId(corpA, 5u);

  GroupsDataAnalyzer::analyzeGroupsAndPutResultsTo(ffgData, totalAttrs);

  std::vector<std::string> corpIdsInOrder;
  for (const AttrComplexStats<std::string>::value_type& corpIdData :
       totalAttrs.getAllGroups<CORP_IDS>())
  {
    corpIdsInOrder.push_back(corpIdData.first);
  }

  ASSERT_FALSE(totalAttrs.matchEmptyAccCode());
  ASSERT_EQ(size_t(4), corpIdsInOrder.size());
  ASSERT_EQ(corpA, corpIdsInOrder[0]);
  ASSERT_EQ(corpC, corpIdsInOrder[1]);
  ASSERT_EQ(corpB, corpIdsInOrder[2]);
  ASSERT_EQ(corpD, corpIdsInOrder[3]);
}

TEST_F(GroupsDataAnalyzerTest, testMatchEmptyAccCode)
{
  TotalAttrs totalAttrs;
  GroupsData ffgData;

  std::string corpA("A"), accCode("001");
  ffgData.addCorpId(corpA, 1u);
  ffgData.createNewGroup(2u);
  ffgData.addAccCode(accCode, 3u);

  GroupsDataAnalyzer::analyzeGroupsAndPutResultsTo(ffgData, totalAttrs);
  ASSERT_TRUE(totalAttrs.matchEmptyAccCode());
}

TEST_F(GroupsDataAnalyzerTest, testPublicFares)
{
  TotalAttrs totalAttrs;
  GroupsData ffgData;

  ffgData.setPublicFares(true, 0u);
  ffgData.setPublicFares(false, 1u);
  ffgData.setPublicFares(false, 2u);
  ffgData.setPublicFares(true, 3u);
  ffgData.setPublicFares(true, 4u);

  GroupsDataAnalyzer::analyzeGroupsAndPutResultsTo(ffgData, totalAttrs);

  const GroupsIds& publicFaresGroups = totalAttrs.getAllGroups<PUBLIC_FARES>();

  ASSERT_EQ(size_t(3), publicFaresGroups.size());
  ASSERT_EQ(size_t(1), publicFaresGroups.count(0u));
  ASSERT_EQ(size_t(0), publicFaresGroups.count(1u));
  ASSERT_EQ(size_t(0), publicFaresGroups.count(2u));
  ASSERT_EQ(size_t(1), publicFaresGroups.count(3u));
  ASSERT_EQ(size_t(1), publicFaresGroups.count(4u));
}

TEST_F(GroupsDataAnalyzerTest, testPrivateFares)
{
  TotalAttrs totalAttrs;
  GroupsData ffgData;

  ffgData.setPrivateFares(false, 0u);
  ffgData.setPrivateFares(true, 1u);
  ffgData.setPrivateFares(false, 2u);
  ffgData.setPrivateFares(false, 3u);
  ffgData.setPrivateFares(false, 4u);

  GroupsDataAnalyzer::analyzeGroupsAndPutResultsTo(ffgData, totalAttrs);

  const GroupsIds& result = totalAttrs.getAllGroups<PRIVATE_FARES>();

  ASSERT_EQ(size_t(1), result.size());
  ASSERT_EQ(size_t(0), result.count(0u));
  ASSERT_EQ(size_t(1), result.count(1u));
  ASSERT_EQ(size_t(0), result.count(2u));
  ASSERT_EQ(size_t(0), result.count(3u));
  ASSERT_EQ(size_t(0), result.count(4u));
}

TEST_F(GroupsDataAnalyzerTest, testTariffTypeMixed1)
{
  TotalAttrs totalAttrs;
  GroupsData ffgData;

  // Some Private, some undefined
  ffgData.setPrivateFares(false, 0u);
  ffgData.setPrivateFares(true, 1u);
  ffgData.setPrivateFares(false, 2u);
  ffgData.setPrivateFares(true, 3u);
  ffgData.setPrivateFares(false, 4u);

  GroupsDataAnalyzer::analyzeGroupsAndPutResultsTo(ffgData, totalAttrs);

  const TariffType tariffType = totalAttrs.getTariffType();

  ASSERT_EQ(true, tariffType == TariffType::Mixed);
}

TEST_F(GroupsDataAnalyzerTest, testTariffTypeMixed2)
{
  TotalAttrs totalAttrs;
  GroupsData ffgData;

  // Some Private, some Published
  ffgData.setPublicFares(true, 0u);
  ffgData.setPrivateFares(true, 1u);
  ffgData.setPrivateFares(true, 2u);
  ffgData.setPrivateFares(true, 3u);
  ffgData.setPublicFares(true, 4u);

  GroupsDataAnalyzer::analyzeGroupsAndPutResultsTo(ffgData, totalAttrs);

  const TariffType tariffType = totalAttrs.getTariffType();

  ASSERT_EQ(true, tariffType == TariffType::Mixed);
}

TEST_F(GroupsDataAnalyzerTest, testTariffTypePrivate)
{
  TotalAttrs totalAttrs;
  GroupsData ffgData;

  // All Private
  ffgData.setPrivateFares(true, 0u);
  ffgData.setPrivateFares(true, 1u);
  ffgData.setPrivateFares(true, 2u);

  GroupsDataAnalyzer::analyzeGroupsAndPutResultsTo(ffgData, totalAttrs);

  const TariffType tariffType = totalAttrs.getTariffType();

  ASSERT_EQ(true, tariffType == TariffType::Private);
}

TEST_F(GroupsDataAnalyzerTest, testTariffTypePublished)
{
  TotalAttrs totalAttrs;
  GroupsData ffgData;

  // All Published
  ffgData.setPublicFares(true, 0u);
  ffgData.setPublicFares(true, 1u);
  ffgData.setPublicFares(true, 2u);
  ffgData.setPublicFares(true, 3u);

  GroupsDataAnalyzer::analyzeGroupsAndPutResultsTo(ffgData, totalAttrs);

  const TariffType tariffType = totalAttrs.getTariffType();

  ASSERT_EQ(true, tariffType == TariffType::Published);
}
TEST_F(GroupsDataAnalyzerTest, testNoAdvancePuschase)
{
  TotalAttrs totalAttrs;
  GroupsData ffgData;

  ffgData.setPrivateFares(false, 0u);
  ffgData.setPrivateFares(false, 1u);
  ffgData.setPrivateFares(false, 2u);
  ffgData.setPrivateFares(false, 3u);
  ffgData.setPrivateFares(false, 4u);

  ffgData.requireNoAdvancePurchase(3u);
  ffgData.requireNoAdvancePurchase(4u);

  GroupsDataAnalyzer::analyzeGroupsAndPutResultsTo(ffgData, totalAttrs);

  const GroupsIds& result = totalAttrs.getAllGroups<NO_ADVANCE_PURCHASE>();

  ASSERT_EQ(size_t(2), result.size());
  ASSERT_EQ(size_t(0), result.count(0u));
  ASSERT_EQ(size_t(0), result.count(1u));
  ASSERT_EQ(size_t(0), result.count(2u));
  ASSERT_EQ(size_t(1), result.count(3u));
  ASSERT_EQ(size_t(1), result.count(4u));
}

TEST_F(GroupsDataAnalyzerTest, testNoPenalty)
{
  TotalAttrs totalAttrs;
  GroupsData ffgData;

  ffgData.setPrivateFares(false, 0u);
  ffgData.setPrivateFares(false, 1u);
  ffgData.setPrivateFares(false, 2u);
  ffgData.setPrivateFares(false, 3u);
  ffgData.setPrivateFares(false, 4u);

  ffgData.requireNoPenalties(0u);
  ffgData.requireNoPenalties(1u);
  ffgData.requireNoPenalties(3u);
  ffgData.requireNoPenalties(4u);

  GroupsDataAnalyzer::analyzeGroupsAndPutResultsTo(ffgData, totalAttrs);

  const GroupsIds& result = totalAttrs.getAllGroups<NO_PENALTIES>();

  ASSERT_EQ(size_t(4), result.size());
  ASSERT_EQ(size_t(1), result.count(0u));
  ASSERT_EQ(size_t(1), result.count(1u));
  ASSERT_EQ(size_t(0), result.count(2u));
  ASSERT_EQ(size_t(1), result.count(3u));
  ASSERT_EQ(size_t(1), result.count(4u));
}

TEST_F(GroupsDataAnalyzerTest, testNoMinMaxStay)
{
  TotalAttrs totalAttrs;
  GroupsData ffgData;

  ffgData.setPrivateFares(false, 0u);
  ffgData.setPrivateFares(false, 1u);
  ffgData.setPrivateFares(false, 2u);
  ffgData.setPrivateFares(false, 3u);
  ffgData.setPrivateFares(false, 4u);

  ffgData.requireNoMinMaxStay(1u);
  ffgData.requireNoMinMaxStay(2u);

  GroupsDataAnalyzer::analyzeGroupsAndPutResultsTo(ffgData, totalAttrs);

  const GroupsIds& result = totalAttrs.getAllGroups<NO_MIN_MAX_STAY>();

  ASSERT_EQ(size_t(2), result.size());
  ASSERT_EQ(size_t(0), result.count(0u));
  ASSERT_EQ(size_t(1), result.count(1u));
  ASSERT_EQ(size_t(1), result.count(2u));
  ASSERT_EQ(size_t(0), result.count(3u));
  ASSERT_EQ(size_t(0), result.count(4u));
}

} // flexFares
} // tse
