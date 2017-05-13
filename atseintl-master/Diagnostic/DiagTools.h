//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
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
//-------------------------------------------------------------------

#ifndef DIAG_TOOLS_H
#define DIAG_TOOLS_H

#include <string>

namespace tse
{

class Trx;
class PricingTrx;

namespace utils
{

const std::string STANDARD_TRUNCATE_MESSAGE = " .OUTPUT TRUNCATED DUE TO LENGTH. ";
const size_t STANDARD_MAX_DIAGNOSTIC_LENGTH = 300000;

// Encapsulates obscure rules telling if we should
// include diagnostic text in the shopping response
// for a given transaction
bool
shouldDiagBePrintedForTrx(const Trx& trx);

// Get hostname of the shopping server for the request
std::string
getHostname();

// Get port number of the shopping server for the request
std::string
getPortNumber();

// Get info about database being used by the shopping server
std::string
getDbInfo();

// Get metrics for the transaction
std::string
getMetricsInfo(const Trx& trx);

// Can't be const ShoppingTrx& since toString() method
// of Diagnostic is non-const
std::string
getDiagnosticPrintout(Trx& trx, const std::string& build_label_string);

// Tells if for given transaction the diagnostic
// text can be truncated
bool
canTruncateDiagnostic(const PricingTrx& trx);

// Truncates diagnostic text if its length exceeds maxLen
// In case of truncation, appends a truncateMsg
//
// Returns: truncated diagnostic with truncateMsg or unchanged text
//     depending of initial text length
std::string
truncateDiagnostic(const std::string& str,
                   size_t maxLen = STANDARD_MAX_DIAGNOSTIC_LENGTH,
                   const std::string& truncateMsg = STANDARD_TRUNCATE_MESSAGE);

// Checks if for given trx, diagnostic can be truncated
// and does it if necessary.
// Returns: truncated diagnostic with truncateMsg or unchanged text
std::string
truncateDiagIfNeeded(const std::string& str, const PricingTrx& trx);

} // namespace utils

} // namespace tse

#endif // DIAG_TOOLS_H
