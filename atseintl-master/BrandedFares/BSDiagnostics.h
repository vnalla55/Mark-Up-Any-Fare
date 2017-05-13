//-------------------------------------------------------------------
//
//  File:        BSDiagnostics.h
//  Created:     Oct 2013
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
class RuleExecution;
class BSDiagnostics : boost::noncopyable
{
public:
  std::vector<RuleExecution*>& ruleExecution() { return _ruleExecution; }
  const std::vector<RuleExecution*>& ruleExecution() const { return _ruleExecution; }

private:
  std::vector<RuleExecution*> _ruleExecution;
};
} // tse

