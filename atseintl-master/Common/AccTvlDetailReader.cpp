//-------------------------------------------------------------------
//
//  File:        AccTvlDetailReader.cpp
//  Created:     Mar 2, 2006
//  Authors:     Simon Li
//
//  Updates:
//
//  Description:
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

#include "Common/AccTvlDetailReader.h"

namespace tse
{
AccTvlDetailReader&
AccTvlDetailReader::
operator>>(std::string& newStr)
{
  std::istringstream& istr = static_cast<std::istringstream&>(*this);
  istr >> tmpBuf;
  newStr = tmpBuf;

  return *this;
}

AccTvlDetailReader&
AccTvlDetailReader::
operator>>(uint16_t& num)
{
  std::istringstream& istr = static_cast<std::istringstream&>(*this);
  istr >> num;
  return *this;
}

AccTvlDetailReader&
AccTvlDetailReader::
operator>>(int& num)
{
  std::istringstream& istr = static_cast<std::istringstream&>(*this);
  istr >> num;
  return *this;
}

AccTvlDetailReader&
AccTvlDetailReader::
operator>>(BookingCode& code)
{
  std::istringstream& istr = static_cast<std::istringstream&>(*this);
  istr >> tmpBuf;
  code = tmpBuf;
  return *this;
}

AccTvlDetailReader&
AccTvlDetailReader::
operator>>(PaxTypeCode& code)
{
  std::istringstream& istr = static_cast<std::istringstream&>(*this);
  istr >> tmpBuf;
  code = tmpBuf;
  return *this;
}

AccTvlDetailReader&
AccTvlDetailReader::
operator>>(RuleNumber& code)
{
  std::istringstream& istr = static_cast<std::istringstream&>(*this);
  istr >> tmpBuf;
  code = tmpBuf;
  return *this;
}

bool
AccTvlDetailReader::getting(const char chr)
{
  // return false if we are at end
  std::istringstream& istr = static_cast<std::istringstream&>(*this);
  if (istr.eof())
    return false;

  char nextChr;
  istr >> nextChr;
  if (nextChr == chr)
  {
    return true;
  }
  // recover the read streampos
  istr.seekg(-1, ios_base::cur);
  return false;
}
}
