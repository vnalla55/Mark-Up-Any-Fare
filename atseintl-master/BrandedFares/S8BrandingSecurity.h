//-------------------------------------------------------------------
//
//  File:        S8BrandingSecurity.h
//  Created:     Nov 2013
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

#include <vector>

namespace tse
{

struct SecurityInfo
{
  std::string _securityName;
  std::string _securityValue;
  StatusS8 _status = StatusS8::PASS_S8;
};

class S8BrandingSecurity : boost::noncopyable
{
public:
  std::vector<SecurityInfo*>& securityInfoVec() { return _securityInfoVec; }
  const std::vector<SecurityInfo*>& securityInfoVec() const { return _securityInfoVec; }

private:
  std::vector<SecurityInfo*> _securityInfoVec;
};
} // tse

