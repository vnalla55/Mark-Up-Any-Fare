//----------------------------------------------------------------------------
//
//  File:        TseUtil.h
//  Created:     4/12/2004
//  Authors:     KS
//
//  Description: Common functions required for ATSE shopping/pricing.
//
//  Updates:
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

namespace tse {

inline bool isDigit (char ch)
{
  return ch >= '0' && ch <= '9';
}

} // end tse namespace
