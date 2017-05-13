// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#ifndef DAO_DIAGNOSTICS_H
#define DAO_DIAGNOSTICS_H

namespace tse
{

class PricingTrx;

class DaoDiagnostics
{
public:
  static const char* PARAM_TABLE_NAME;

  static void printDBDiagnostic(PricingTrx& trx);

private:
  DaoDiagnostics();
  ~DaoDiagnostics();
  DaoDiagnostics(const DaoDiagnostics&);
  DaoDiagnostics& operator=(const DaoDiagnostics&);
};

} // namespace tse

#endif // DAO_DIAGNOSTICS
