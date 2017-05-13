//-------------------------------------------------------------------
//
//  File:        DecodeResponseFormatterTest.cpp
//  Created:     September 12, 2014
//  Authors:     Roland Kwolek
//
//  Description: Decode Response Formatter tests
//
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

#include "Xform/DecodeResponseFormatter.h"

#include "Common/TseStringTypes.h"
#include "DataModel/DecodeTrx.h"
#include "test/include/TestMemHandle.h"

#include <gtest/gtest.h>
#include <string>

namespace tse
{
class DecodeResponseFormatterTest : public testing::Test
{
public:
  void SetUp() { _trx = _memHandle(new DecodeTrx()); }

  void TearDown() { _memHandle.clear(); }

protected:
  TestMemHandle _memHandle;
  DecodeTrx* _trx;
};

TEST_F(DecodeResponseFormatterTest, OneLine)
{
  _trx->addToResponse("Old MacDonald had a farm, E-I-E-I-O.");

  DecodeResponseFormatter drf(*_trx);
  std::string result = drf.formatResponse();

  std::string expected = "<DecodeResponse><MSG N06=\"X\" Q0K=\"0\" S18=\"Old MacDonald had a "
                         "farm, E-I-E-I-O.\"/></DecodeResponse>";
  EXPECT_EQ(expected, result);
}

TEST_F(DecodeResponseFormatterTest, LineBreak)
{
  _trx->addToResponse("Old MacDonald had a farm, E-I-E-I-O. And on that farm he had a cow, "
                      "E-I-E-I-O. With a moo moo here and a moo moo there Here a moo, there a moo, "
                      "everywhere a moo moo Old MacDonald had a farm, E-I-E-I-O. ");

  DecodeResponseFormatter drf(*_trx);
  std::string result = drf.formatResponse();

  std::string expected = "<DecodeResponse><MSG N06=\"X\" Q0K=\"0\" S18=\"Old MacDonald had a "
                         "farm, E-I-E-I-O. And on that farm he had a \"/><MSG N06=\"X\" "
                         "Q0K=\"1\" S18=\"cow, E-I-E-I-O. With a moo moo here and a moo moo "
                         "there Here a \"/><MSG N06=\"X\" Q0K=\"2\" S18=\"moo, there a moo, "
                         "everywhere a moo moo Old MacDonald had a \"/><MSG N06=\"X\" "
                         "Q0K=\"3\" S18=\"farm, E-I-E-I-O. \"/></DecodeResponse>";
  EXPECT_EQ(expected, result);
}

TEST_F(DecodeResponseFormatterTest, LineBreakLongStrings)
{
  _trx->addToResponse("aaaaaaaaaaaaaaaaaaaaaaaaaaaaa bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb");

  DecodeResponseFormatter drf(*_trx);
  std::string result = drf.formatResponse();

  std::string expected = "<DecodeResponse><MSG N06=\"X\" Q0K=\"0\" "
                         "S18=\"aaaaaaaaaaaaaaaaaaaaaaaaaaaaa \"/><MSG N06=\"X\" Q0K=\"1\" "
                         "S18=\"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\"/></DecodeResponse>";
  EXPECT_EQ(expected, result);
}

TEST_F(DecodeResponseFormatterTest, ErrorMessage)
{
  _trx->addToResponse("Error: User can not be found!");

  DecodeResponseFormatter drf(*_trx);
  std::string result = drf.formatResponse('E');

  std::string expected = "<DecodeResponse><MSG N06=\"E\" Q0K=\"0\" "
                         "S18=\"Error: User can not be found!\"/></DecodeResponse>";
  EXPECT_EQ(expected, result);
}
}
