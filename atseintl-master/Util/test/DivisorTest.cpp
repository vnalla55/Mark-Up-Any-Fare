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

#include "Util/Divisor.h"

#include <tr1/tuple>

#include <gtest/gtest.h>
#include "test/include/GtestHelperMacros.h"

namespace tse
{
#if 0
// These tests are usually not used, since they generate a few hundred thousand test cases.
// They execute a few minutes, which is definitely too long.
// Use them only when you develop on the Divisor class.

class DivisorUint8Test : public ::testing::TestWithParam<std::tr1::tuple<uint16_t, uint16_t>>
{
};
class DivisorUint64Test : public ::testing::TestWithParam<std::tr1::tuple<uint128_t, uint128_t>>
{
};

INSTANTIATE_TEST_CASE_P(DivisorUint8TestInstantation,
                        DivisorUint8Test,
                        ::testing::Combine(::testing::Range<uint16_t>(0, 256),
                                           ::testing::Range<uint16_t>(1, 256)));
INSTANTIATE_TEST_CASE_P(
    DivisorUint64TestInstantation,
    DivisorUint64Test,
    ::testing::Combine(
        ::testing::Range<uint128_t>(0UL, 18422933325880639185UL, 28518472640682104UL),
        ::testing::Range<uint128_t>(1UL, 18395300937940068752UL, 73581203751760275UL)));

TEST_P(DivisorUint8Test, testDivide)
{
  const uint8_t x = std::tr1::get<0>(GetParam());
  const uint8_t y = std::tr1::get<1>(GetParam());

  auto divisor = Divisor<uint8_t>(y);
  ASSERT_EQ(x / y, x / divisor);
}

TEST_P(DivisorUint64Test, testDivide)
{
  const uint64_t x = std::tr1::get<0>(GetParam());
  const uint64_t y = std::tr1::get<1>(GetParam());

  auto divisor = Divisor<uint64_t>(y);
  ASSERT_EQ(x / y, x / divisor);
}
#else
class DivisorUint8Test : public ::testing::TestWithParam<uint16_t>
{
};
class DivisorUint64Test : public ::testing::TestWithParam<uint128_t>
{
};

INSTANTIATE_TEST_CASE_P(DivisorUint8TestInstantation,
                        DivisorUint8Test,
                        ::testing::Range<uint16_t>(0, 256));
INSTANTIATE_TEST_CASE_P(DivisorUint64TestInstantation,
                        DivisorUint64Test,
                        ::testing::Range<uint128_t>(0UL,
                                                    18422933325880639185UL,
                                                    28518472640682104UL));

TEST_P(DivisorUint8Test, testDivide)
{
  const uint8_t x = GetParam();
  for (uint16_t y_ = 1; y_ < 256; ++y_)
  {
    const uint8_t y = y_;
    auto divisor = Divisor<uint8_t>(y);
    EXPECT_EQ(x / y, x / divisor);
  }
}

TEST_P(DivisorUint64Test, testDivide)
{
  const uint64_t x = GetParam();
  for (uint128_t y_ = 1; y_ < 18395300937940068752UL; y_ += 73581203751760275UL)
  {
    const uint64_t y = y_;
    auto divisor = Divisor<uint64_t>(y);
    EXPECT_EQ(x / y, x / divisor);
  }
}
#endif
}
