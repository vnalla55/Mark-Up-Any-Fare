//----------------------------------------------------------------------------
//
//      File: CurrencyResponseFormatter.h
//      Description: Class to format Currency responses back to sending client
//      Created: February 17, 2005
//      Authors: Mike Carroll
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
//----------------------------------------------------------------------------

#pragma once

#include "DataModel/CurrencyTrx.h"
#include "Xform/ResponseFormatter.h"

namespace tse
{
class CurrencyResponseFormatter : public ResponseFormatter
{
public:
  //--------------------------------------------------------------------------
  // @function CurrencyResponseFormatter::formatResponse
  //
  // Description: Prepare a CurrencyConversionRequest response tagged
  //              suitably for client
  //
  // @param currencyTrx - a valid CurrencyTrx
  //--------------------------------------------------------------------------
  std::string formatResponse(CurrencyTrx& currencyTrx);

  //--------------------------------------------------------------------------
  // @function CurrencyResponseFormatter::formatResponse
  //
  // Description: Prepare an error response
  //
  // @param ere - error type
  // @param response - formatted response
  //--------------------------------------------------------------------------
  static void formatResponse(const ErrorResponseException& ere, std::string& response);

private:
  int recNum = 2;
}; // End class CurrencyResponseFormatter

} // End namespace tse

