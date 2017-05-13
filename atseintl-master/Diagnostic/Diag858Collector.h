//----------------------------------------------------------------------------
//
//  Copyright Sabre 2011
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
#include "Rules/TicketingEndorsement.h"

namespace tse
{
class Diag858Collector : public DiagCollector
{
public:
  virtual void printHeader() override;
  void print(const PricingUnit& prU);
  void printEndorsement(const PaxTypeFare* ptf,
                        const std::vector<TicketEndorseItem>& endos,
                        size_t validCounter);
  void printEndorsement(const FarePath& fp);
  void printGluedMsgs(const TicketingEndorsement::TicketEndoLines& msgs, const CarrierCode& cxr);

  void printGluedMsgs(const TicketingEndorsement::TicketEndoLines& msgs);

private:
  void printSingleEndorsement(unsigned short counter, const TicketEndorseItem& tei);
  void printAllEndoLines(const TicketingEndorsement::TicketEndoLines& msgs);
  bool isFop(const TicketEndorseItem& tei)
  {
    return tei.tktLocInd == TicketingEndorsement::TKTLOCIND_FOP_1 ||
           tei.tktLocInd == TicketingEndorsement::TKTLOCIND_FOP_3 ||
           tei.tktLocInd == TicketingEndorsement::TKTLOCIND_FOP_5;
  }
};
}

