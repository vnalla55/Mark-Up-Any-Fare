//----------------------------------------------------------------------------
//
//      File: BrandingRequestFormatter.h
//      Description: Class to format XML Branding Request
//      Created: Feb 24, 2008
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

#include "Common/XMLConstruct.h"
#include "DataModel/FareDisplayRequest.h"

class XMLConstruct;

namespace tse
{
class PricingTrx;
class FareDisplayTrx;

namespace xray
{
class JsonMessage;
}

class BrandingRequestFormatter
{
public:

  //----------------------------------------------------------------------------
  // BrandingRequestFormatter::addRequestHeader
  //-------------------------------------------------------------------------
  // @function BrandingRequestFormatter::addRequestHeader
  //
  // Description: Add request Header Information
  //
  // @param construct - where the formatted agent information goes
  // @return void
  //-------------------------------------------------------------------------
  void addRequestHeader(XMLConstruct& construct);

  void addXRAType(XMLConstruct& construct, FareDisplayTrx& trx);

  //-------------------------------------------------------------------------
  // @function BrandingRequestFormatter::addSRCType
  //
  // Description: Add request Source Information
  //
  // @param construct - where the formatted agent information goes
  // @param trx - use to reference request information
  // @return void
  //-------------------------------------------------------------------------
  void addSRCType(XMLConstruct& construct, FareDisplayTrx& trx);

  //-------------------------------------------------------------------------
  // @function BrandingRequestFormatter::addBCRType
  //
  // Description: Add request Branding Carriers Information
  //
  // @param construct - where the formatted agent information goes
  // @param trx - use to reference request information
  // @return void
  //-------------------------------------------------------------------------
  void addBCRType(XMLConstruct& construct, FareDisplayTrx& trx);

private:
  void addXRAType(XMLConstruct& construct, xray::JsonMessage* jsonMessage);

  void addSRCType(XMLConstruct& construct, PricingTrx& trx, const char* clientId);

  void addBCRType(XMLConstruct& construct,
                  PricingTrx& trx,
                  const char* travelDate,
                  const char* orgCity,
                  const char* destCity,
                  const char* carrierCode);
}; // End class BrandingRequestFormatter
} // End namespace tse
