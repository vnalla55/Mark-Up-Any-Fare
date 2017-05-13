//----------------------------------------------------------------------------
//
//        File: FCCommand.h
// Description: FC command class will display standard output header
//              and according to the command type will instantiate FCDisplay
//              or FCConvert objects.
//     Created: 10/11/07
//     Authors: Svetlana Tsarkova
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include <string>

namespace
{
static std::string NUC_TOO_HIGH_VALUE = "ERR!";
}
namespace tse
{
class CurrencyTrx;
class DateTime;
class FCDisplay;
class NUCInfo;

class FCCommand
{
public:
  FCCommand(CurrencyTrx& trx) : _trx(trx) {}

  virtual ~FCCommand() = default;

  virtual void process();
  void displayStandardHeader();

protected:
  CurrencyTrx& _trx;

  // common methods, used by both FCDisplay and FCConvert
  virtual std::string format_date(const DateTime& date);
  virtual std::string format_nuc_rate(const NUCInfo& nuc, bool append_left);
  virtual double truncate(const double& value, unsigned no_dec);
  virtual bool currency_code_exists(const CurrencyCode& curr);

  virtual void validate_historical_date(const DateTime& date);

  virtual bool should_display_fchelp_info();
};
} // tse namespace
