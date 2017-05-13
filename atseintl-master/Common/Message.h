//----------------------------------------------------------------------------
//
//  Copyright Sabre 2006
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/ErrorResponseException.h"

#include <map>
#include <string>

namespace tse
{

class Message
{
public:
  static constexpr char TYPE_GENERAL = 'X';
  static constexpr char TYPE_ERROR = 'E';
  static constexpr char TYPE_WARNING = 'W';
  static constexpr char TYPE_ENDORSE = 'N';
  static constexpr char TYPE_RESTRICTION = 'R';
  static constexpr char TYPE_DISPLAY = 'D';
  static constexpr char TYPE_CURRENCY = 'F';
  static constexpr char TYPE_TICKETING_WARNING = 'C';
  static constexpr char TYPE_BAGGAGE = 'B';
  static constexpr char TYPE_BAGGAGE_NONUSDOT = 'Y';
  static constexpr char TYPE_AGENCY_COMM_MSG = 'A';

  static uint16_t errCode(const ErrorResponseException::ErrorResponseCode& errCode);

  // Match ErrorResponseCode to Legacy Pricing Fail Code
  static void fillMsgErrCodeMap();

private:
  static std::map<ErrorResponseException::ErrorResponseCode, uint16_t> _msgErrCodeMap;
};
}

