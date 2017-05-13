//----------------------------------------------------------------------------
//
//      File: TaxInfoResponseFormatter.h
//      Description: Class to format Tax common  responses back to sending client
//      Created: December, 2008
//      Authors: Jakub Kubica
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

#include "Common/XMLConstruct.h"
#include "DataModel/TaxTrx.h"


namespace tse
{
class TaxInfoResponseFormatter
{
public:
  /// Destructor
  ~TaxInfoResponseFormatter();
  //--------------------------------------------------------------------------
  // @function TaxInfoResponseFormatter::PfcDisplayResponseFormatter
  //
  // Description: constructor
  //
  // @param none
  //--------------------------------------------------------------------------
  TaxInfoResponseFormatter();

  //--------------------------------------------------------------------------
  // @function TaxInfoResponseFormatter::formatResponse
  //
  // Description: Prepare a TaxRequest response tagged suitably for client
  //
  // @param taxTrx - a valid TaxTrx
  //--------------------------------------------------------------------------
  void formatResponse(TaxTrx& taxTrx);
  static void formatResponse(const ErrorResponseException& ere, std::string& response);

  void buildMessage(TaxTrx& taxTrx, XMLConstruct& construct);

};

} // End namespace tse
