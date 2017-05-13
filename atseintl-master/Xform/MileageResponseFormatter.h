//----------------------------------------------------------------------------
//
//      File: MileageResponseFormatter.h
//      Description: Class to format Mileage responses back to sending client
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

#include "Xform/ResponseFormatter.h"

#include <iostream>

namespace tse
{
class MileageTrx;

class MileageResponseFormatter : public ResponseFormatter
{
public:
  //--------------------------------------------------------------------------
  // @function MileageResponseFormatter::formatResponse
  //
  // Description: Prepare a MileageRequest response tagged suitably for client
  //
  // @param mileageTrx - a valid MileageTrx
  //--------------------------------------------------------------------------
  std::string formatResponse(MileageTrx& mileageTrx);

  //--------------------------------------------------------------------------
  // @function MileageResponseFormatter::formatResponse
  //
  // Description: Prepare an error response
  //
  // @param ere - error
  // @param response - formatted response
  //--------------------------------------------------------------------------
  static void formatResponse(const ErrorResponseException& ere, std::string& response);

  //--------------------------------------------------------------------------
  // @function MileageResponseFormatter::updateDiagResponseText
  //
  // Description: Prepare a MileageRequest response tagged suitably for client
  //
  // @param mileageTrx - a valid MileageTrx
  //--------------------------------------------------------------------------
  void updateDiagResponseText(MileageTrx& mileageTrx, std::string& response);

private:
  void mainDiagResponse(std::ostringstream& oss) const;

}; // End class MileageResponseFormatter

} // End namespace tse

