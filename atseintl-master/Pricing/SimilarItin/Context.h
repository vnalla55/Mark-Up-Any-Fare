/*----------------------------------------------------------------------------
 *  Copyright Sabre 2015
 *
 *     The copyright to the computer program(s) herein
 *     is the property of Sabre.
 *     The program(s) may be used and/or copied only with
 *     the written permission of Sabre or in accordance
 *     with the terms and conditions stipulated in the
 *     agreement/contract under which the program(s)
 *     have been supplied.
 *-------------------------------------------------------------------------*/
#pragma once

namespace tse
{
class DiagCollector;
class Itin;
class PricingTrx;
namespace similaritin
{
struct Context
{
  Context(PricingTrx& pTrx, Itin& pMotherItin, DiagCollector& pDiagnostic)
    : trx(pTrx), motherItin(pMotherItin), diagnostic(pDiagnostic)
  {
  }

  PricingTrx& trx;
  Itin& motherItin;
  DiagCollector& diagnostic;
};
}
}
