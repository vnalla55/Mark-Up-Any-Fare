//-------------------------------------------------------------------
//
//  File:        DecodeResponseFormatter.cpp
//  Created:     September 12, 2014
//  Authors:     Roland Kwolek
//
//  Description: Response Formatter for Decode Transaction
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

#pragma once

#include "DataModel/DecodeTrx.h"
#include "Xform/ResponseFormatter.h"

#include <string>

namespace tse
{
class DecodeResponseFormatter : public ResponseFormatter
{
public:
  DecodeResponseFormatter(const DecodeTrx& trx) : ResponseFormatter(), _trx(trx) {}

  std::string formatResponse(const char msgType = 'X');
  static void formatResponse(const ErrorResponseException& ere, std::string& response);

private:
  static  void prepareMessage(XMLConstruct& construct,
                              const char msgType,
                              const uint16_t lineNum,
                              const std::string& msgText);
  const DecodeTrx& _trx;
};

} // namespace tse

