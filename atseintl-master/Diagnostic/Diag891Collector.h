//----------------------------------------------------------------------------
//  File:        Diag891Collector.h
//  Authors:
//  Created:
//
//  Description: Diagnostic 891 Branded Fares - response from Branded service
//  Updates:
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
//----------------------------------------------------------------------------

#pragma once

#include "Diagnostic/Diag890Collector.h"

namespace tse
{
class Diag891Collector : public Diag890Collector
{
public:
  explicit Diag891Collector(Diagnostic& root) : Diag890Collector(root) {}
  Diag891Collector() {}
};

} // namespace tse

