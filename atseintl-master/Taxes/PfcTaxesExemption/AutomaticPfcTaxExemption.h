//----------------------------------------------------------------------------
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

#ifndef AUTOMATIC_PFC_TAX_EXEMTPION_H
#define AUTOMATIC_PFC_TAX_EXEMTPION_H

#include "DataModel/AutomaticPfcTaxExemptionData.h"

namespace tse
{
class PricingTrx;
class TaxResponse;
class FarePath;

class AutomaticPfcTaxExemption
{
public:
  static void analyzePfcExemtpion(PricingTrx& trx, TaxResponse& rsp);
  static bool isAutomaticPfcExemtpionEnabled(PricingTrx& trx, TaxResponse& rsp);
  static bool matchTktDes(const std::string& rule, const std::string& fare);
  static bool isAAItinOnly(const FarePath& farePath);
  static bool bypassTktDes(const FarePath& farePath);
  static bool isFreeTicketByPass(PricingTrx& trx, const TaxResponse& rsp);

private:
  AutomaticPfcTaxExemption(const AutomaticPfcTaxExemption&);
  AutomaticPfcTaxExemption& operator=(const AutomaticPfcTaxExemption&);

  static AutomaticPfcTaxExemptionData createPfcExemptionData(PricingTrx& trx, TaxResponse& rsp);
};

} // namespace tse

#endif // AUTOMATIC_PFC_TAX_EXEMTPION_H
