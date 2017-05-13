#include "DataModel/MaxPenaltyInfo.h"

#include "Common/Money.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <gtest/gtest.h>

namespace tse
{
class MaxPenaltyInfoTest : public ::testing::Test
{
public:
  void SetUp()
  {
    _memHandle.create<TestConfigInitializer>();
  }
  void TearDown() { _memHandle.clear(); }

protected:
  TestMemHandle _memHandle;
};

TEST_F(MaxPenaltyInfoTest, testFeeCompoundAssignmentOperatorBothValid)
{
  MaxPenaltyResponse::Fee first{Money{100.0, USD}};
  MaxPenaltyResponse::Fee second{Money{100.0, USD}};
  first += second;
  ASSERT_EQ(200.0, first._fee.get().value());
  ASSERT_FALSE(first._non);
}

TEST_F(MaxPenaltyInfoTest, testFeeCompoundAssignmentOperatorBothOtherwise)
{
  MaxPenaltyResponse::Fee first{Money{100.0, USD}, true};
  MaxPenaltyResponse::Fee second{Money{100.0, USD}, true};
  first += second;
  ASSERT_EQ(200.0, first._fee.get().value());
  ASSERT_TRUE(first._non);
}

TEST_F(MaxPenaltyInfoTest, testFeeCompoundAssignmentOperatorBothFullyNon)
{
  MaxPenaltyResponse::Fee first{{}, true};
  MaxPenaltyResponse::Fee second{{}, true};
  first += second;
  ASSERT_FALSE(first._fee.is_initialized());
  ASSERT_TRUE(first._non);
}

// VALID FEE
TEST_F(MaxPenaltyInfoTest, testFeeCompoundAssignmentOperatorWithOtherwise)
{
  MaxPenaltyResponse::Fee first{Money{100.0, USD}};
  MaxPenaltyResponse::Fee second{Money{100.0, USD}, true};
  first += second;
  ASSERT_FALSE(first.isMissingData());
  ASSERT_TRUE(first._fee.is_initialized());
  ASSERT_TRUE(first._non);
}

TEST_F(MaxPenaltyInfoTest, testFeeCompoundAssignmentOperatorWithMDT)
{
  MaxPenaltyResponse::Fee first{Money{100.0, USD}};
  MaxPenaltyResponse::Fee second{{}, false};
  ASSERT_TRUE(first._fee.is_initialized());
  first += second;
  ASSERT_TRUE(*first._fee == Money(100.0, USD));
  ASSERT_FALSE(first._non);
}

TEST_F(MaxPenaltyInfoTest, testFeeCompoundAssignmentOperatorWithFullyNon)
{
  MaxPenaltyResponse::Fee first{Money{100.0, USD}};
  MaxPenaltyResponse::Fee second{{}, true};
  EXPECT_FALSE(first._non);
  first += second;
  ASSERT_FALSE(first._fee.is_initialized());
  ASSERT_TRUE(first._non);
}

// OTHERWISE
TEST_F(MaxPenaltyInfoTest, testFeeCompoundAssignmentOperatorOtherwiseToOtherwise)
{
  MaxPenaltyResponse::Fee first{Money{100.0, USD}, true};
  MaxPenaltyResponse::Fee second{Money{100.0, USD}, true};
  first += second;
  ASSERT_TRUE(first._fee.is_initialized());
  ASSERT_TRUE(first._non);
  ASSERT_EQ(200.0, first._fee.get().value());
}

TEST_F(MaxPenaltyInfoTest, testFeeCompoundAssignmentOperatorMDTToOtherwise)
{
  MaxPenaltyResponse::Fee first{Money{100.0, USD}, true};
  MaxPenaltyResponse::Fee second{{}, false};
  first += second;
  ASSERT_TRUE(first._fee.is_initialized());
  ASSERT_TRUE(first._non);
}

TEST_F(MaxPenaltyInfoTest, testFeeCompoundAssignmentOperatorFullyNonToOtherwise)
{
  MaxPenaltyResponse::Fee first{Money{100.0, USD}, true};
  MaxPenaltyResponse::Fee second{{}, true};
  first += second;
  ASSERT_FALSE(first._fee.is_initialized());
  ASSERT_TRUE(first._non);
}

// FULLY NON
TEST_F(MaxPenaltyInfoTest, testFeeCompoundAssignmentOperatorToFullyNon)
{
  MaxPenaltyResponse::Fee first{{}, true};
  MaxPenaltyResponse::Fee second{Money{100.0, USD}};
  first += second;
  ASSERT_FALSE(first._fee.is_initialized());
  ASSERT_TRUE(first._non);
}

TEST_F(MaxPenaltyInfoTest, testFeeCompoundAssignmentOperatorFullyOtherwiseToFullyNon)
{
  MaxPenaltyResponse::Fee first{{}, true};
  MaxPenaltyResponse::Fee second{Money{100.0, USD}, true};
  first += second;
  ASSERT_FALSE(first._fee.is_initialized());
  ASSERT_TRUE(first._non);
}

TEST_F(MaxPenaltyInfoTest, testFeeCompoundAssignmentOperatorMDTToFullyNon)
{
  MaxPenaltyResponse::Fee first{{}, true};
  MaxPenaltyResponse::Fee second{{}, false};
  first += second;
  ASSERT_FALSE(first._fee.is_initialized());
  ASSERT_TRUE(first._non);
}

TEST_F(MaxPenaltyInfoTest, testFeeCompoundAssignmentOperatorOtherwiseToFully)
{
  MaxPenaltyResponse::Fee first{Money{100.0, USD}, true};
  MaxPenaltyResponse::Fee second{Money{100.0, USD}};
  first += second;
  ASSERT_EQ(200.0, first._fee.get().value());
  ASSERT_TRUE(first._non);
}
} // namespace tse
