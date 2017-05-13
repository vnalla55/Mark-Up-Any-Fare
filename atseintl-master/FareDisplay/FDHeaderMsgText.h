//----------------------------------------------------------------------------
//  File: FDHeaderMsgText.h
//
//  Author: Partha Kumar Chakraborti
//
//  Copyright Sabre 2005
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/TseConsts.h"
#include "Common/TseEnums.h"

namespace tse
{

class FDHeaderMsgText
{
public:
  FDHeaderMsgText() {};
  ~FDHeaderMsgText() {};

  std::string& text() { return _text; }
  const std::string& text() const { return _text; }

  std::string& startPoint() { return _startPoint; }
  const std::string& startPoint() const { return _startPoint; }

private:
  std::string _text; // Actual Header Message Text
  std::string _startPoint; // C2
};

} // End of namespace tse

