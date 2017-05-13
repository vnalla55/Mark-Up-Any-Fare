//-------------------------------------------------------------------
//
//  File:        BrandingResponse.h
//  Created:     Sep 2013
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2013
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

#include "Common/TseStringTypes.h"

#include <boost/noncopyable.hpp>

namespace tse
{

class BrandingResponse : boost::noncopyable
{
public:
  std::string& messageCode() { return _messageCode; }
  const std::string& messageCode() const { return _messageCode; }

  std::string& failCode() { return _failCode; }
  const std::string& failCode() const { return _failCode; }

  std::string& messageText() { return _messageText; }
  const std::string& messageText() const { return _messageText; }

private:
  std::string _messageCode;
  std::string _failCode;
  std::string _messageText;
};
} // tse
