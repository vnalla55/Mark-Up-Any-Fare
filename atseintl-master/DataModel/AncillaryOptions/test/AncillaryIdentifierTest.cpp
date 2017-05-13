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

#include <gtest/gtest.h>
#include "test/include/TestMemHandle.h"
#include "DataModel/AncillaryOptions/AncillaryIdentifier.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SvcFeesResBkgDesigInfo.h"
#include "ServiceFees/OCFees.h"

namespace tse
{
TEST(AncillaryIdentifierTest_ValidAncillaryID, whenAncIdWithoutPadisIsValid_shouldNotThrow)
{
  ASSERT_NO_THROW(AncillaryIdentifier("1S|C|0DF|1.2|1000"));
}

TEST(AncillaryIdentifierTest_ValidAncillaryID, whenAncIdWithPadisIsValid_shouldNotThrow)
{
  ASSERT_NO_THROW(AncillaryIdentifier("1S|C|0DF|1.2|1000|1500"));
}

TEST(AncillaryIdentifierTest_StartsOrEndsWithWhiteSpace,
     whenAncIdWithoutPadisStartsWithWhiteSpace_shouldThrow)
{
  ASSERT_THROW(AncillaryIdentifier(" 1S|C|0DF|1.2|1000"), ErrorResponseException);
}

TEST(AncillaryIdentifierTest_StartsOrEndsWithWhiteSpace,
     whenAncIdWithoutPadisEndsWithWhiteSpace_shouldThrow)
{
  ASSERT_THROW(AncillaryIdentifier("1S|C|0DF|1.2|1000 "), ErrorResponseException);
}

TEST(AncillaryIdentifierTest_StartsOrEndsWithWhiteSpace,
     whenAncIdWithPadisStartsWithWhiteSpace_shouldThrow)
{
  ASSERT_THROW(AncillaryIdentifier(" 1S|C|0DF|1.2|1000|1500"), ErrorResponseException);
}

TEST(AncillaryIdentifierTest_StartsOrEndsWithWhiteSpace,
     whenAncIdWithPadisEndsWithWhiteSpace_shouldThrow)
{
  ASSERT_THROW(AncillaryIdentifier("1S|C|0DF|1.2|1000|1500 "), ErrorResponseException);
}

TEST(AncillaryIdentifierTest_IncorrectFormat, whenAncIdIsEmptyString_shouldThrow)
{
  ASSERT_THROW(AncillaryIdentifier(""), ErrorResponseException);
}

TEST(AncillaryIdentifierTest_IncorrectFormat,
     whenAncIdHasLessSeparatorsThanMinimumAllowed_shouldThrow)
{
  ASSERT_THROW(AncillaryIdentifier("1S|C|2000"), ErrorResponseException);
}

TEST(AncillaryIdentifierTest_IncorrectFormat,
     whenAncIdHasMoreSeparatorsThanMaximumAllowed_shouldThrow)
{
  ASSERT_THROW(AncillaryIdentifier("1S|C|0DF|1.2|2000|1500|aboveMax"), ErrorResponseException);
}

TEST(AncillaryIdentifierTest_MissingData, whenAncIdHasEmptyCarrierCode_shouldThrow)
{
  ASSERT_THROW(AncillaryIdentifier("|C|0DF|1.2|1000|1500"), ErrorResponseException);
}

TEST(AncillaryIdentifierTest_MissingData, whenAncIdHasEmptyServiceType_shouldThrow)
{
  ASSERT_THROW(AncillaryIdentifier("1S||0DF|1.2|1000|1500"), ErrorResponseException);
}

TEST(AncillaryIdentifierTest_MissingData, whenAncIdHasEmptySubCode_shouldThrow)
{
  ASSERT_THROW(AncillaryIdentifier("1S|C||1.2|1000|1500"), ErrorResponseException);
}

TEST(AncillaryIdentifierTest_MissingData, whenAncIdHasEmptySegmentList_shouldThrow)
{
  ASSERT_THROW(AncillaryIdentifier("1S|C|0DF||1000|1500"), ErrorResponseException);
}

TEST(AncillaryIdentifierTest_MissingData, whenAncIdHasEmptyS7Sequence_shouldThrow)
{
  ASSERT_THROW(AncillaryIdentifier("1S|C|0DF|1.2||1500"), ErrorResponseException);
}

TEST(AncillaryIdentifierTest_MissingData, givenAncIdWithPadis_whenPadisIsEmpty_shouldThrow)
{
  ASSERT_THROW(AncillaryIdentifier("1S|C|0DF|1.2|1000|"), ErrorResponseException);
}

TEST(AncillaryIdentifierTest_IncorrectDataLength,
     givenAncId_whenCarrierCodeLenghtIsDifferentThanTwo_shouldThrow)
{
  ASSERT_THROW(AncillaryIdentifier("1SS|C|0DF|1.2|1000"), ErrorResponseException);
  ASSERT_THROW(AncillaryIdentifier("1|C|0DF|1.2|1000"), ErrorResponseException);
}

TEST(AncillaryIdentifierTest_IncorrectDataLength,
     givenAncId_whenServiceTypeIsNotSingleCharacter_shouldThrow)
{
  ASSERT_THROW(AncillaryIdentifier("1S|CB|0DF|1.2|1000"), ErrorResponseException);
}

TEST(AncillaryIdentifierTest_IncorrectDataLength,
     givenAncId_whenSubcodeLenghtIsDifferentThanThree_shouldThrow)
{
  ASSERT_THROW(AncillaryIdentifier("1S|C|0D|1.2|1000"), ErrorResponseException);
  ASSERT_THROW(AncillaryIdentifier("1S|C|0DFU|1.2|1000"), ErrorResponseException);
}

TEST(AncillaryIdentifierTest_IncorrectDataLength,
     givenAncIdIsValid_whenOneOfTravelSegmentsNumberIsGreaterThanNine_shouldNotThrow)
{
  ASSERT_NO_THROW(AncillaryIdentifier("1S|C|0DF|12.2|1000"));
}

TEST(AncillaryIdentifierTest_CharactersAllowed,
     givenAncId_whenCarrierCodeContainsSpecialCharacter_shouldThrow)
{
  ASSERT_THROW(AncillaryIdentifier("1_|C|0DF|1.2|1000"), ErrorResponseException);
}

TEST(AncillaryIdentifierTest_CharactersAllowed,
     givenAncId_whenServiceTypeIsEitherSpecialCharacterOrDigit_shouldThrow)
{
  ASSERT_THROW(AncillaryIdentifier("1S|3|0DF|1.2|1000"), ErrorResponseException);
  ASSERT_THROW(AncillaryIdentifier("1S|*|0DF|1.2|1000"), ErrorResponseException);
}

TEST(AncillaryIdentifierTest_CharactersAllowed,
     givenAncId_whenSubcodeHasSpecialCharacter_shouldThrow)
{
  ASSERT_THROW(AncillaryIdentifier("1S|C|0_F|1.2|1000"), ErrorResponseException);
  ASSERT_THROW(AncillaryIdentifier("1S|C|0--|1.2|1000"), ErrorResponseException);
  ASSERT_THROW(AncillaryIdentifier("1S|C|0D*|1.2|1000"), ErrorResponseException);
}

TEST(AncillaryIdentifierTest_CharactersAllowed, givenAncId_whenSegmentListHasLetters_shouldThrow)
{
  ASSERT_THROW(AncillaryIdentifier("1S|C|0DF|1A2|1000"), ErrorResponseException);
  ASSERT_THROW(AncillaryIdentifier("1S|C|0DF|AA2|1000"), ErrorResponseException);
}

TEST(AncillaryIdentifierTest_CharactersAllowed,
     givenAncId_whenS7SequenceConsistsOfSpecialCharactersOrLetters_shouldThrow)
{
  ASSERT_THROW(AncillaryIdentifier("1S|C|0DF|1.2|abc"), ErrorResponseException);
  ASSERT_THROW(AncillaryIdentifier("1S|C|0DF|1.2|125i"), ErrorResponseException);
  ASSERT_THROW(AncillaryIdentifier("1S|C|0DF|1.2|11*9"), ErrorResponseException);
}

TEST(AncillaryIdentifierTest_CharactersAllowed,
     givenAncIdWithPadisData_whenPadisCodeContainsSpecialCharactersOrLetters_shouldThrow)
{
  ASSERT_THROW(AncillaryIdentifier("1S|C|0DF|1.2|1000|ab_"), ErrorResponseException);
  ASSERT_THROW(AncillaryIdentifier("1S|C|0DF|1.2|1000|--85"), ErrorResponseException);
  ASSERT_THROW(AncillaryIdentifier("1S|C|0DF|1.2|1000|ABC"), ErrorResponseException);
}

TEST(AncillaryIdentifierTest_GenerationFromOcFees, whenOcFeesIsEmptyStruct_shouldThrow)
{
  TestMemHandle _memHandle;
  OCFees ocFees;

  ASSERT_THROW(_memHandle.insert(new AncillaryIdentifier(ocFees)),
               ErrorResponseException);
}

TEST(AncillaryIdentifierTest_GenerationFromOcFees,
     whenOcFeesHasAllRequiredData_shouldGenerateCorrectAncId)
{
  TestMemHandle _memHandle;

  OCFees ocFees;
  SubCodeInfo* s5 = _memHandle.create<SubCodeInfo>();
  OptionalServicesInfo* s7 = _memHandle.create<OptionalServicesInfo>();

  ocFees.travelStart() = _memHandle.create<AirSeg>();
  ocFees.travelEnd() = _memHandle.create<AirSeg>();

  s5->fltTktMerchInd() = 'A';
  s5->serviceSubTypeCode() = ServiceSubTypeCode("0DF");
  ocFees.travelStart()->pnrSegment() = 1;
  ocFees.travelEnd()->pnrSegment() = 3;
  ocFees.carrierCode() = CarrierCode("1S");
  s7->seqNo() = 2000;
  s7->upgrdServiceFeesResBkgDesigTblItemNo() = 3000;

  ocFees.padisData().push_back(_memHandle.create<SvcFeesResBkgDesigInfo>());
  ocFees.optFee() = s7;
  ocFees.subCodeInfo() = s5;

  AncillaryIdentifier* ancId;
  ASSERT_NO_THROW(ancId = _memHandle.insert(new AncillaryIdentifier(ocFees)));
  EXPECT_EQ(std::string("1S|A|0DF|1.2.3|2000|3000"), ancId->getIdentifier());
}
}
