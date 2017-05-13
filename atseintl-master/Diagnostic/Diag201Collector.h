//----------------------------------------------------------------------------
//  File:         Diag201Collector.h
//  Description:  Overrides the virtual methods to follow Diagnostic 201 format
//  Authors:      Mohammad Hossan
//  Created:      Dec 2003
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2003
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

#include "Diagnostic/DiagCollector.h"

// Each Sub class will override the virtual methods to meet the format of
// presenting the information

namespace tse
{

class FareMarket;
class TariffCrossRefInfo;

// This class is to display failed fare tariff reference inhibits information.
class Diag201Collector : public DiagCollector
{
public:
  explicit Diag201Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag201Collector() {}

  Diag201Collector& operator<<(const FareMarket& fareMarket) override;
  Diag201Collector& operator<<(const TariffCrossRefInfo& tariffCrossRef);
};

} // namespace tse

