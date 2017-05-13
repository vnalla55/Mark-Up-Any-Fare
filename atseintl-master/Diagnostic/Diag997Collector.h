//----------------------------------------------------------------------------
// File:        Diag997Collector.h
//
// Authors:     Bartosz Kolpanowicz
//
// Created:     Sep 2011
//
// Description: Diagnostic 997 formatter
//
// Copyright Sabre 2011
//
//              The copyright to the computer program(s) herein
//              is the property of Sabre.
//              The program(s) may be used and/or copied only with
//              the written permission of Sabre or in accordance
//              with the terms and conditions stipulated in the
//              agreement/contract under which the program(s)
//              have been supplied.
//
// Updates:     date  coreID  description.
//----------------------------------------------------------------------------

#pragma once

#include "Diagnostic/DiagCollector.h"

namespace tse
{
class Diag997Collector : public DiagCollector
{
public:
  virtual Diag997Collector& operator<<(const PricingTrx&) override;
};

} // namespace tse

