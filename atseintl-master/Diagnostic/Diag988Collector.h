//----------------------------------------------------------------------------
//          File:           Diag988Collector.h
//          Description:    Diag988Collector
//          Created:        11/10/2010
//          Authors:        Anna Kulig
//
//          Updates:
//
//     (c)2010, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseStringTypes.h"
#include "Diagnostic/DiagCollector.h"

#include <iosfwd>
#include <map>
#include <string>

namespace tse
{
class Itin;
class PricingTrx;

class Diag988Collector : public DiagCollector
{
public:
  void outputHeader();
  void outputVITAData(const PricingTrx& trx,
                      const Itin& itin,
                      bool intTicketValidationResult,
                      const std::string& validationMessage = std::string());
};

} // namespace tse

