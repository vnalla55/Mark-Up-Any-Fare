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

#include "Util/Algorithm/Bitset.h"
#include "Util/Algorithm/Container.h"
#include "Util/Algorithm/Comparison.h"
#include "Util/Algorithm/Integer.h"
#include "Util/Algorithm/Iterator.h"
#include "Util/Algorithm/Tuple.h"

#include <map>
#include <set>
#include <tuple>
#include <vector>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "test/include/GtestHelperMacros.h"

namespace tse
{
namespace alg
{
namespace
{
template <uint64_t... Bit>
uint64_t
getMask()
{
  uint64_t result = 0;
  for (unsigned bit : {Bit...})
    result |= (1ULL << bit);
  return result;
}

const auto IsNegative = [](int i)
{ return i < 0; };

TEST(BitsetAlgTest, testFillBitset_1)
{
  std::bitset<10> bs(getMask<2, 8>());
  fill_bitset(bs, 0, 1);
  EXPECT_EQ((getMask<0, 2, 8>()), bs.to_ullong());
  fill_bitset(bs, 9, 1000);
  EXPECT_EQ((getMask<0, 2, 8, 9>()), bs.to_ullong());
}

TEST(BitsetAlgTest, testFillBitset_1_CornerCase)
{
  std::bitset<64> bs;
  fill_bitset(bs, 63, 64);
  EXPECT_EQ((getMask<63>()), bs.to_ullong());
}

TEST(BitsetAlgTest, testFillBitset_0)
{
  std::bitset<64> bs;
  bs.set();
  fill_bitset(bs, 1, 63, false);
  EXPECT_EQ((getMask<0, 63>()), bs.to_ullong());
  fill_bitset(bs, 1, 64, false);
  EXPECT_EQ((getMask<0>()), bs.to_ullong());
}

TEST(ContainerAlgTest, testContains_Positive)
{
  std::vector<int> v = {1, 3, 4, 2};
  EXPECT_TRUE(contains(v, 1));
  EXPECT_TRUE(contains(v, 4));
}

TEST(ContainerAlgTest, testContains_Negative)
{
  std::vector<int> v = {1, 3, 4, 2};
  EXPECT_FALSE(contains(v, 5));
}

TEST(IntegerAlgTest, testSignificantBits)
{
  EXPECT_EQ(1, significantBits(uint8_t(1)));
  EXPECT_EQ(2, significantBits(uint8_t(2)));
  EXPECT_EQ(2, significantBits(uint8_t(3)));
  EXPECT_EQ(7, significantBits(uint8_t(65)));

  EXPECT_EQ(1, significantBits(uint64_t(1)));
  EXPECT_EQ(8, significantBits(uint64_t(255)));
  EXPECT_EQ(9, significantBits(uint64_t(256)));
  EXPECT_EQ(9, significantBits(uint64_t(257)));
}

TEST(IntegerAlgTest, testTrailingZeros)
{
  EXPECT_EQ(0, trailingZeros(uint8_t(1)));
  EXPECT_EQ(1, trailingZeros(uint8_t(2)));
  EXPECT_EQ(0, trailingZeros(uint8_t(3)));
  EXPECT_EQ(4, trailingZeros(uint8_t(48)));
  EXPECT_EQ(0, trailingZeros(uint8_t(65)));

  EXPECT_EQ(0, trailingZeros(uint64_t(1)));
  EXPECT_EQ(0, trailingZeros(uint64_t(255)));
  EXPECT_EQ(8, trailingZeros(uint64_t(256)));
  EXPECT_EQ(0, trailingZeros(uint64_t(257)));
  EXPECT_EQ(1, trailingZeros(uint64_t(258)));
}

TEST(IntegerAlgTest, testMulHigh)
{
  EXPECT_EQ(uint8_t(0), mulHigh(uint16_t(1), uint16_t(1)));
  EXPECT_EQ(uint8_t(0), mulHigh(uint16_t(15), uint16_t(17)));
  EXPECT_EQ(uint8_t(1), mulHigh(uint16_t(16), uint16_t(16)));
  EXPECT_EQ(uint8_t(39), mulHigh(uint16_t(100), uint16_t(100)));

  EXPECT_EQ(uint32_t(0), mulHigh(uint64_t(1), uint64_t(1)));
  EXPECT_EQ(uint32_t(0), mulHigh(uint64_t(65535), uint64_t(65537)));
  EXPECT_EQ(uint32_t(1), mulHigh(uint64_t(65536), uint64_t(65536)));
  EXPECT_EQ(uint32_t(2328306), mulHigh(uint64_t(100000000), uint64_t(100000000)));

  const auto twoPow32 = uint128_t(1) << 32;
  EXPECT_EQ(uint64_t(0), mulHigh(uint128_t(1), uint128_t(1)));
  EXPECT_EQ(uint64_t(0), mulHigh(twoPow32 - 1, twoPow32 + 1));
  EXPECT_EQ(uint64_t(1), mulHigh(twoPow32, twoPow32));
  EXPECT_EQ(uint64_t(54210108624L),
            mulHigh(uint128_t(1000000000000000L), uint128_t(1000000000000000L)));
}

TEST(IntegerAlgTest, testRotateLeft)
{
  EXPECT_EQ(uint8_t(0), rotateLeft(uint8_t(0), 0));
  EXPECT_EQ(uint8_t(0), rotateLeft(uint8_t(0), 5));
  EXPECT_EQ(uint8_t(1), rotateLeft(uint8_t(1), 0));
  EXPECT_EQ(uint8_t(32), rotateLeft(uint8_t(1), 5));
  EXPECT_EQ(uint8_t(219), rotateLeft(uint8_t(123), 3));

  EXPECT_EQ(uint16_t(0), rotateLeft(uint16_t(0), 0));
  EXPECT_EQ(uint16_t(0), rotateLeft(uint16_t(0), 12));
  EXPECT_EQ(uint16_t(16), rotateLeft(uint16_t(16), 0));
  EXPECT_EQ(uint16_t(32768), rotateLeft(uint16_t(2), 14));
  EXPECT_EQ(uint16_t(58947), rotateLeft(uint16_t(34764), 7));

  EXPECT_EQ(uint32_t(0), rotateLeft(uint32_t(0), 0));
  EXPECT_EQ(uint32_t(0), rotateLeft(uint32_t(0), 23));
  EXPECT_EQ(uint32_t(4576434), rotateLeft(uint32_t(4576434), 0));
  EXPECT_EQ(uint32_t(950272000), rotateLeft(uint32_t(3625), 18));
  EXPECT_EQ(uint32_t(2451570715), rotateLeft(uint32_t(56465), 21));

  EXPECT_EQ(uint64_t(0), rotateLeft(uint64_t(0), 0));
  EXPECT_EQ(uint64_t(0), rotateLeft(uint64_t(0), 33));
  EXPECT_EQ(uint64_t(356736756823464812), rotateLeft(uint64_t(356736756823464812), 0));
  EXPECT_EQ(uint64_t(937583028480245760), rotateLeft(uint64_t(54574515), 34));
  EXPECT_EQ(uint64_t(8703487754873125264), rotateLeft(uint64_t(768367256356), 46));
}

TEST(IntegerAlgTest, testRotateRight)
{
  EXPECT_EQ(uint8_t(0), rotateRight(uint8_t(0), 0));
  EXPECT_EQ(uint8_t(0), rotateRight(uint8_t(0), 5));
  EXPECT_EQ(uint8_t(1), rotateRight(uint8_t(1), 0));
  EXPECT_EQ(uint8_t(1), rotateRight(uint8_t(32), 5));
  EXPECT_EQ(uint8_t(123), rotateRight(uint8_t(219), 3));

  EXPECT_EQ(uint16_t(0), rotateRight(uint16_t(0), 0));
  EXPECT_EQ(uint16_t(0), rotateRight(uint16_t(0), 12));
  EXPECT_EQ(uint16_t(16), rotateRight(uint16_t(16), 0));
  EXPECT_EQ(uint16_t(2), rotateRight(uint16_t(32768), 14));
  EXPECT_EQ(uint16_t(34764), rotateRight(uint16_t(58947), 7));

  EXPECT_EQ(uint32_t(0), rotateRight(uint32_t(0), 0));
  EXPECT_EQ(uint32_t(0), rotateRight(uint32_t(0), 23));
  EXPECT_EQ(uint32_t(4576434), rotateRight(uint32_t(4576434), 0));
  EXPECT_EQ(uint32_t(3625), rotateRight(uint32_t(950272000), 18));
  EXPECT_EQ(uint32_t(56465), rotateRight(uint32_t(2451570715), 21));

  EXPECT_EQ(uint64_t(0), rotateRight(uint64_t(0), 0));
  EXPECT_EQ(uint64_t(0), rotateRight(uint64_t(0), 33));
  EXPECT_EQ(uint64_t(356736756823464812), rotateRight(uint64_t(356736756823464812), 0));
  EXPECT_EQ(uint64_t(54574515), rotateRight(uint64_t(937583028480245760), 34));
  EXPECT_EQ(uint64_t(768367256356), rotateRight(uint64_t(8703487754873125264), 46));
}

TEST(IteratorAlgTest, testFindLastIf_NotFound)
{
  std::vector<int> v = {1, 2, 2, 1, 3};
  EXPECT_TRUE(find_last_if(v.begin(), v.end(), IsNegative) == v.end());
}

TEST(IteratorAlgTest, testFindLastIf_Found)
{
  std::vector<int> v = {1, -1, 2, 1, -3, -3};
  EXPECT_TRUE(find_last_if(v.begin(), v.end(), IsNegative) == v.end() - 1);
}

TEST(IteratorAlgTest, testFindEqualRangeAround_OnlyOne)
{
  std::vector<int> v = {1, 2, 1, 1, 1};
  auto range = find_equal_range_around(v.begin(), v.end(), v.begin(), std::equal_to<int>());

  EXPECT_EQ(ptrdiff_t(0), range.first - v.begin());
  EXPECT_EQ(ptrdiff_t(1), range.second - v.begin());
}

TEST(IteratorAlgTest, testFindEqualRangeAround_End)
{
  std::vector<int> v = {1, 2, 1, 1, 1};
  auto range = find_equal_range_around(v.begin(), v.end(), v.end() - 1, std::equal_to<int>());

  EXPECT_EQ(ptrdiff_t(2), range.first - v.begin());
  EXPECT_EQ(ptrdiff_t(v.size()), range.second - v.begin());
}

TEST(IteratorAlgTest, testFindEqualRangeAround_Middle)
{
  std::vector<int> v = {1, 2, 2, 1};
  auto range = find_equal_range_around(v.begin(), v.end(), v.begin() + 1, std::equal_to<int>());

  EXPECT_EQ(ptrdiff_t(1), range.first - v.begin());
  EXPECT_EQ(ptrdiff_t(3), range.second - v.begin());
}

struct StructWithGetters
{
  int i;
  char c;
  float f;

  int getI() const { return i; }
  char getC() const { return c; }
  float getF() const { return f; }
};

TEST(ComparisonAlgTest, testMembersEqual_SameStruct)
{
  const StructWithGetters o1{1, 'X', 1.2f};
  const StructWithGetters o2 = o1;

  EXPECT_TRUE(alg::members_equal(
      o1, o1, &StructWithGetters::getI, &StructWithGetters::getC, &StructWithGetters::getF));
  EXPECT_TRUE(alg::members_equal(
      o1, o2, &StructWithGetters::getI, &StructWithGetters::getC, &StructWithGetters::getF));
}

TEST(ComparisonAlgTest, testMembersEqual_DifferentStruct)
{
  const StructWithGetters o1{1, 'X', 1.2f};
  const StructWithGetters o2{1, 'X', 1.3f};

  EXPECT_TRUE(alg::members_equal(o1, o2, &StructWithGetters::getI, &StructWithGetters::getC));
  EXPECT_FALSE(alg::members_equal(
      o1, o2, &StructWithGetters::getI, &StructWithGetters::getC, &StructWithGetters::getF));
}

TEST(TupleAlgTest, testVisitTuple_Empty)
{
  const std::tuple<> t{};

  std::ostringstream oss;
  alg::visit_tuple(t, [&](const auto& item) { oss << item; });

  EXPECT_EQ(std::string(), oss.str());
}

TEST(TupleAlgTest, testVisitTuple_UniqueTypes)
{
  const std::tuple<int, const char*> t(100, "USD");

  std::ostringstream oss;
  alg::visit_tuple(t, [&](const auto& item) { oss << item; });

  EXPECT_EQ(std::string("100USD"), oss.str());
}

TEST(TupleAlgTest, testVisitTuple_NonUniqueTypes)
{
  const std::tuple<int, const char*, const char*> t(100, "USD", "SUFFIX");

  std::ostringstream oss;
  alg::visit_tuple(t, [&](const auto& item) { oss << item; });

  EXPECT_EQ(std::string("100USDSUFFIX"), oss.str());
}

TEST(TupleAlgTest, testVisitTuple_Mutable)
{
  std::tuple<int, std::string> t(100, "USD");

  alg::visit_tuple(t, [&](auto& item) { item = std::decay_t<decltype(item)>(); });

  std::ostringstream oss;
  alg::visit_tuple(t, [&](const auto& item) { oss << item; });

  EXPECT_EQ(std::string("0"), oss.str());
}

TEST(TupleAlgTest, testEnumerateTuple)
{
  const std::tuple<int, const char*, const char*> t(100, "USD", "SUFFIX");

  std::ostringstream oss;
  alg::enumerate_tuple(t, [&](const auto i, const auto& item)
  {
    oss << (i() + 1) << ":" << item << "\n";
  });

  EXPECT_EQ(std::string("1:100\n2:USD\n3:SUFFIX\n"), oss.str());
}
TEST(eraseIf, testIfSize3StringsRemovedSet)
{
  std::set<std::string> container{"USD", "1337", "SUFFIX", "JOHN", "BIN"};
  alg::erase_if(container,
                [](const std::string item)
                { return item.size() == 3; });
  EXPECT_THAT(container, testing::ElementsAre("1337", "JOHN", "SUFFIX"));
}
TEST(eraseIf, testIfSize3StringsRemovedVector)
{
  std::vector<std::string> container{"USD", "1337", "SUFFIX", "JOHN", "BIN"};
  alg::erase_if(container,
                [](const std::string item)
                { return item.size() == 3; });
  EXPECT_THAT(container, testing::ElementsAre("1337", "SUFFIX", "JOHN"));
}

TEST(eraseIf, testIfSameKeyValueStringRemovedMap)
{
  std::map<std::string, std::string> container{{"USD", "1337"}, {"SUFFIX", "JOHN"}, {"BIN", "BIN"}};
  alg::erase_if(container,
                [](const auto item)
                { return item.first == item.second; });
  EXPECT_THAT(container,
              testing::ElementsAre(testing::Pair("SUFFIX", "JOHN"), testing::Pair("USD", "1337")));
}
}
}
}
