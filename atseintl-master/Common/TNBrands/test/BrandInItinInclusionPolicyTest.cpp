//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//               Andrzej Fediuk
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "test/include/GtestHelperMacros.h"

#include "Common/ErrorResponseException.h"
#include "Common/TNBrands/ItinGeometryCalculator.h"
#include "Common/TNBrands/test/TNBrandsMocks.h"

#include <memory>

using namespace ::testing;
using namespace boost;

namespace tse
{

namespace skipper
{

class BrandInItinInclusionPolicyTest: public Test
{
public:
  void SetUp()
  {
    _policy.reset(new BrandInItinInclusionPolicy());
    _relations.reset(new BrandProgramRelations());
    _filterMap.reset(new BrandFilterMap());
  }

  void TearDown(){}

  std::shared_ptr<BrandInItinInclusionPolicy> _policy;
  std::shared_ptr<BrandProgramRelations> _relations;
  std::shared_ptr<BrandFilterMap> _filterMap;
};

TEST_F(BrandInItinInclusionPolicyTest, testPassesAllBrandsIfFilterMapIsEmpty)
{
  ASSERT_EQ(true, _policy->isBrandCodeInItin(
      "DUMMY_BRAND", *_relations, *_filterMap));
}

TEST_F(BrandInItinInclusionPolicyTest, testDiscardsBrandIfNotInFilterMap)
{
  (*_filterMap)["ANOTHER_BRAND"].insert("PR1");
  ASSERT_EQ(false, _policy->isBrandCodeInItin(
      "DUMMY_BRAND", *_relations, *_filterMap));
}

TEST_F(BrandInItinInclusionPolicyTest, testPassesBrandIfCorrespondingProgramSetEmpty)
{
  (*_filterMap)["DUMMY_BRAND"];
  ASSERT_EQ(true, _policy->isBrandCodeInItin(
      "DUMMY_BRAND", *_relations, *_filterMap));
}

TEST_F(BrandInItinInclusionPolicyTest, testPassesBrandIfAvailableInProgramFromFilterMap)
{
  _relations->addBrandProgramPair("DUMMY_BRAND", "PR1", 1);
  (*_filterMap)["DUMMY_BRAND"].insert("PR1");
  ASSERT_EQ(true, _policy->isBrandCodeInItin(
      "DUMMY_BRAND", *_relations, *_filterMap));
}

TEST_F(BrandInItinInclusionPolicyTest, testDiscardsBrandIfNotAvailableInProgramFromFilterMap)
{
  _relations->addBrandProgramPair("DUMMY_BRAND", "PR1", 1);
  (*_filterMap)["DUMMY_BRAND"].insert("PR2");
  ASSERT_EQ(false, _policy->isBrandCodeInItin(
      "DUMMY_BRAND", *_relations, *_filterMap));
}

TEST_F(BrandInItinInclusionPolicyTest, testPassesAllQualifiedBrandIndicesIfFilterMapIsEmpty)
{
  _relations->addBrandProgramPair("DUMMY_BRAND", "PR1", 1);
  _relations->addBrandProgramPair("DUMMY_BRAND", "PR2", 2);
  _relations->addBrandProgramPair("DUMMY_BRAND", "PR3", 3);

  QualifiedBrandIndices expected;
  expected.insert(1);
  expected.insert(2);
  expected.insert(3);

  ASSERT_EQ(expected, _policy->getIndicesForBrandCodeWithFiltering(
      "DUMMY_BRAND", *_relations, *_filterMap));
}

TEST_F(BrandInItinInclusionPolicyTest, testReturnsNoQualifiedBrandIndicesIfBrandNotInFilterMap)
{
  (*_filterMap)["ANOTHER_BRAND"].insert("PR1");

  _relations->addBrandProgramPair("DUMMY_BRAND", "PR1", 1);
  _relations->addBrandProgramPair("ANOTHER_BRAND", "PR1", 2);

  QualifiedBrandIndices expected;
  ASSERT_EQ(expected, _policy->getIndicesForBrandCodeWithFiltering(
      "DUMMY_BRAND", *_relations, *_filterMap));
}

TEST_F(BrandInItinInclusionPolicyTest, testPassesAllQualifiedBrandIndicesIfCorrespondingProgramSetEmpty)
{
  (*_filterMap)["DUMMY_BRAND"];
  _relations->addBrandProgramPair("DUMMY_BRAND", "PR1", 1);
  _relations->addBrandProgramPair("DUMMY_BRAND", "PR2", 2);
  _relations->addBrandProgramPair("DUMMY_BRAND", "PR3", 3);

  QualifiedBrandIndices expected;
  expected.insert(1);
  expected.insert(2);
  expected.insert(3);

  ASSERT_EQ(expected, _policy->getIndicesForBrandCodeWithFiltering(
      "DUMMY_BRAND", *_relations, *_filterMap));
}

TEST_F(BrandInItinInclusionPolicyTest, testPassesOnlyQualifiedBrandIndicesIfAvailableInProgramFromFilterMap)
{
  (*_filterMap)["DUMMY_BRAND"].insert("PR1");

  _relations->addBrandProgramPair("DUMMY_BRAND", "PR1", 1);
  _relations->addBrandProgramPair("DUMMY_BRAND", "PR2", 2);
  _relations->addBrandProgramPair("DUMMY_BRAND", "PR3", 3);

  QualifiedBrandIndices expected;
  expected.insert(1);

  ASSERT_EQ(expected, _policy->getIndicesForBrandCodeWithFiltering(
      "DUMMY_BRAND", *_relations, *_filterMap));
}

TEST_F(BrandInItinInclusionPolicyTest, testReturnsNoQualifiedBrandIndicesIfNotAvailableInProgramFromFilterMap)
{
  (*_filterMap)["DUMMY_BRAND"].insert("PR4");

  _relations->addBrandProgramPair("DUMMY_BRAND", "PR1", 1);
  _relations->addBrandProgramPair("DUMMY_BRAND", "PR2", 2);
  _relations->addBrandProgramPair("DUMMY_BRAND", "PR3", 3);

  QualifiedBrandIndices expected;
  ASSERT_EQ(expected, _policy->getIndicesForBrandCodeWithFiltering(
      "DUMMY_BRAND", *_relations, *_filterMap));
}

} // namespace skipper

} // namespace tse
