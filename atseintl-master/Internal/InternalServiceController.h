//----------------------------------------------------------------
//
//  File:		InternalServiceController
//  Authors:	Mike Carroll
//
//  Copyright Sabre 2004
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

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class Trx;
class PricingTrx;
class DCFactory;

class InternalServiceController
{
public:
  InternalServiceController();
  virtual ~InternalServiceController();

  bool process(PricingTrx& trx);

private:
  void handleDiagnostic187(PricingTrx& trx);
  void handleDiagnostic188(PricingTrx& trx);
  void handleDiagnostic193(PricingTrx& trx);
  void handleDiagnostic194(PricingTrx& trx);
  void handleDiagnostic198(PricingTrx& trx);
  void handleDiagnostic199(PricingTrx& trx);
  void handleDiagnostic997(PricingTrx& trx);

  DCFactory* _dcFactory;
  static log4cxx::LoggerPtr _logger;
};

} /* end namespace tse */

