//----------------------------------------------------------------------------
//
//      File: PfcDisplayResponseFormatter.h
//      Description: Class to format Tax Display responses back to sending client
//      Created: September, 2006
//      Authors: Hitha Alex
//
//  Copyright Sabre 2006
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
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"


namespace tse
{

class PfcDisplayResponseFormatter
{
public:
  //--------------------------------------------------------------------------
  // @function PfcDisplayResponseFormatter::PfcDisplayResponseFormatter
  //
  // Description: constructor
  //
  // @param none
  //--------------------------------------------------------------------------
  PfcDisplayResponseFormatter();

  //--------------------------------------------------------------------------
  // @function PfcDisplayResponseFormatter::~PfcDisplayResponseFormatter
  //
  // Description: destructor
  //--------------------------------------------------------------------------
  virtual ~PfcDisplayResponseFormatter();

  //--------------------------------------------------------------------------
  // @function PfcDisplayResponseFormatter::formatResponse
  //
  // Description: Prepare a TaxRequest response tagged suitably for client
  //
  // @param taxTrx - a valid TaxTrx
  //--------------------------------------------------------------------------
  void formatResponse(TaxTrx& taxTrx);
  void formatResponse(TaxTrx& taxTrx, ErrorResponseException& ere);
  static void formatResponse(const ErrorResponseException& ere, std::string& response);

protected:
  // Nothing

private:

  static void buildMessage(const std::string& response, XMLConstruct& construct, std::string msgType = "X");

  static const std::string readConfigXMLNamespace(const std::string& configName);
}; // End class PfcDisplayResponseFormatter

} // End namespace tse

