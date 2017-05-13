//-------------------------------------------------------------------
//
//  File:        AccTvlDetailReader.h
//  Created:     Mar 1, 2006
//  Authors:     Simon Li
//
//  Updates:
//
//  Description: Object passing data
//
//  Copyright Sabre 2006
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

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"

#include <istream>

namespace tse
{

class AccTvlDetailReader : public std::istringstream
{
public:
  AccTvlDetailReader(const std::string& inputStr)
  {
    this->str(inputStr);
    this->width(TMP_BUFSZ); // no overflow read things longer than tmpBuf
  }

  AccTvlDetailReader& operator>>(std::string& newStr);

  AccTvlDetailReader& operator>>(uint16_t& num);

  AccTvlDetailReader& operator>>(int& num);

  AccTvlDetailReader& operator>>(BookingCode& code);

  AccTvlDetailReader& operator>>(PaxTypeCode& code);

  AccTvlDetailReader& operator>>(RuleNumber& code);

  bool getting(const char chr);

private:
  const static uint16_t TMP_BUFSZ = 10;
  char tmpBuf[10];
};

} // tse namespace

