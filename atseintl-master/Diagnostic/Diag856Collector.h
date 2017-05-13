//----------------------------------------------------------------------------
//  File:        Diag856Collector.h
//
//  Description: Diagnostic 856 formatter
//
//  Copyright Sabre 2008
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

namespace tse
{
class NoPNRPricingTrx;

class Diag856Collector : public DiagCollector
{
public:
  void process(const NoPNRPricingTrx& trx);

private:
  const NoPNROptions* getDBPnrOptions(const NoPNRPricingTrx& trx);
  std::string getDiagHeader();
  std::string getDiagFooter();
  std::string getDiagBody(const NoPNRPricingTrx& trx);
  std::string getSegmentsInfo(const NoPNROptions& options);
  std::string getCommonDiagBodyPart(const NoPNROptions& options);
};
}

