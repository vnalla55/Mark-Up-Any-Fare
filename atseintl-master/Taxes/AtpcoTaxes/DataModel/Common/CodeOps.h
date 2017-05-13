// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

#include "AtpcoTaxes/DataModel/Common/Code.h"

namespace tax
{

template <typename tag, int L, int H>
bool equal(const tax::Code<tag, L, H>& code, const std::string& str)
{
  uint32_t len = code.length();
  return str.length() == len && (std::memcmp(str.c_str(), code.rawArray(), len) == 0);
}

template <typename tag, int L, int H>
bool matches(const std::string& ref, tax::Code<tag, L, H> code)
{
  return ref.empty() || equal(code, ref);
}

template <typename tag, int L, int H>
bool matches(tax::Code<tag, L, H> ref, tax::Code<tag, L, H> code)
{
  return ref.empty() || code == ref;
}

template <typename tag, int L, int H>
int toInt(tax::Code<tag, L, H> code)
{
  char array[H + 1] = {};
  memcpy(array, code.rawArray(), H);
  return atoi(array);
}


} // namespace tax

