//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//               Andrzej Fediuk
//
//  Copyright Sabre 2014
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

#include "Common/TNBrands/BrandProgramRelations.h"
#include "Common/ErrorResponseException.h"

#include <memory>
#include <string>

using namespace ::testing;

namespace tse
{

namespace skipper
{

class BrandProgramRelationsTest: public Test
{
public:
  void SetUp()
  {
    _relations.reset(new BrandProgramRelations());
    feedRelations(*_relations);
  }

  void TearDown(){}

  void feedRelations(BrandProgramRelations& relations)
  {
    // program 1: bc1, bc2
    // program 2: bc1, bc3
    relations.addBrandProgramPair(DUMMY_BC_1, DUMMY_PROGRAM_ID_1, DUMMY_BC_PROGAM_INDEX_1);
    relations.addBrandProgramPair(DUMMY_BC_2, DUMMY_PROGRAM_ID_1, DUMMY_BC_PROGAM_INDEX_2);
    relations.addBrandProgramPair(DUMMY_BC_1, DUMMY_PROGRAM_ID_2, DUMMY_BC_PROGAM_INDEX_3);
    relations.addBrandProgramPair(DUMMY_BC_3, DUMMY_PROGRAM_ID_2, DUMMY_BC_PROGAM_INDEX_4);
  }

  std::shared_ptr<BrandProgramRelations> _relations;
  static const std::string DUMMY_PROGRAM_ID_1;
  static const std::string DUMMY_PROGRAM_ID_2;
  static const BrandCode DUMMY_BC_1;
  static const BrandCode DUMMY_BC_2;
  static const BrandCode DUMMY_BC_3;
  static const BrandCode DUMMY_BC_4;
  static const int DUMMY_BC_PROGAM_INDEX_1;
  static const int DUMMY_BC_PROGAM_INDEX_2;
  static const int DUMMY_BC_PROGAM_INDEX_3;
  static const int DUMMY_BC_PROGAM_INDEX_4;
  static const int DUMMY_BC_PROGAM_INDEX_5;
};


const std::string BrandProgramRelationsTest::DUMMY_PROGRAM_ID_1 = "Beethoven";
const std::string BrandProgramRelationsTest::DUMMY_PROGRAM_ID_2 = "Mozart";
const BrandCode BrandProgramRelationsTest::DUMMY_BC_1 = "ONE";
const BrandCode BrandProgramRelationsTest::DUMMY_BC_2 = "TWO";
const BrandCode BrandProgramRelationsTest::DUMMY_BC_3 = "THREE";
const BrandCode BrandProgramRelationsTest::DUMMY_BC_4 = "FOUR";
const int BrandProgramRelationsTest::DUMMY_BC_PROGAM_INDEX_1 = 0;
const int BrandProgramRelationsTest::DUMMY_BC_PROGAM_INDEX_2 = 1;
const int BrandProgramRelationsTest::DUMMY_BC_PROGAM_INDEX_3 = 2;
const int BrandProgramRelationsTest::DUMMY_BC_PROGAM_INDEX_4 = 3;
const int BrandProgramRelationsTest::DUMMY_BC_PROGAM_INDEX_5 = 4;

TEST_F(BrandProgramRelationsTest, testReturnsAllBrandsCorrectly)
{
  UnorderedBrandCodes expected;
  expected.insert(DUMMY_BC_1);
  expected.insert(DUMMY_BC_2);
  expected.insert(DUMMY_BC_3);
  ASSERT_EQ(expected, _relations->getAllBrands());
}

TEST_F(BrandProgramRelationsTest, testCorrectlyReturnsTrueIfBrandInProgram)
{
  ASSERT_EQ(true, _relations->isBrandCodeInProgram(DUMMY_BC_2, DUMMY_PROGRAM_ID_1));
  ASSERT_EQ(true, _relations->isBrandCodeInProgram(DUMMY_BC_1, DUMMY_PROGRAM_ID_2));
}

TEST_F(BrandProgramRelationsTest, testCorrectlyReturnsFalseIfBrandNotInProgram)
{
  ASSERT_EQ(false, _relations->isBrandCodeInProgram(DUMMY_BC_3, DUMMY_PROGRAM_ID_1));
  ASSERT_EQ(false, _relations->isBrandCodeInProgram(DUMMY_BC_2, DUMMY_PROGRAM_ID_2));
}

TEST_F(BrandProgramRelationsTest, testRaisesIfBrandCodeUnknown)
{
  ASSERT_THROW(_relations->isBrandCodeInProgram(DUMMY_BC_4, DUMMY_PROGRAM_ID_1),
      ErrorResponseException);
}

TEST_F(BrandProgramRelationsTest, testReturnsEqualityTrueForSameObjects)
{
  BrandProgramRelations second;
  feedRelations(second);
  ASSERT_TRUE((*_relations) == second);
}

TEST_F(BrandProgramRelationsTest, testReturnsEqualityFalseForDifferentObjects)
{
  BrandProgramRelations second;
  feedRelations(second);
  second.addBrandProgramPair("SOME_OTHER_BRAND", "OTHER_PROGRAM", DUMMY_BC_PROGAM_INDEX_5);
  ASSERT_FALSE((*_relations) == second);
}

TEST_F(BrandProgramRelationsTest, testReturnsProgramsForBrandCorrectly)
{
  ProgramIds expectedForBC1;
  expectedForBC1.insert(DUMMY_PROGRAM_ID_1);
  expectedForBC1.insert(DUMMY_PROGRAM_ID_2);

  ProgramIds expectedForBC3;
  expectedForBC3.insert(DUMMY_PROGRAM_ID_2);

  ASSERT_EQ(expectedForBC1, _relations->getProgramsForBrand(DUMMY_BC_1));
  ASSERT_EQ(expectedForBC3, _relations->getProgramsForBrand(DUMMY_BC_3));
}

TEST_F(BrandProgramRelationsTest, testRaisesOnNotExisingBrandCode)
{
  BrandCode notExisting = "NONE";
  ASSERT_THROW(_relations->getProgramsForBrand(notExisting),
      ErrorResponseException);
}

TEST_F(BrandProgramRelationsTest, testReturnsQualifiedBrandIndexCorrectly)
{
  // program 1: bc1, bc2
  // program 2: bc1, bc3
  // qualifiedBrandIndexes (id: (bc,pr)): 1 (1,1), 2 (2,1), 3(1,2), 4(3,2)
  ASSERT_EQ(DUMMY_BC_PROGAM_INDEX_1,
    _relations->getQualifiedBrandIndexForBrandProgramPair(DUMMY_BC_1, DUMMY_PROGRAM_ID_1));
  ASSERT_EQ(DUMMY_BC_PROGAM_INDEX_2,
    _relations->getQualifiedBrandIndexForBrandProgramPair(DUMMY_BC_2, DUMMY_PROGRAM_ID_1));
  ASSERT_EQ(DUMMY_BC_PROGAM_INDEX_3,
    _relations->getQualifiedBrandIndexForBrandProgramPair(DUMMY_BC_1, DUMMY_PROGRAM_ID_2));
  ASSERT_EQ(DUMMY_BC_PROGAM_INDEX_4,
    _relations->getQualifiedBrandIndexForBrandProgramPair(DUMMY_BC_3, DUMMY_PROGRAM_ID_2));
}

TEST_F(BrandProgramRelationsTest, testRaisesOnNotExistingBrandProgramPair)
{
  ASSERT_THROW(_relations->getQualifiedBrandIndexForBrandProgramPair(
    DUMMY_BC_3, DUMMY_PROGRAM_ID_1), ErrorResponseException);
}

} // namespace skipper

} // namespace tse
