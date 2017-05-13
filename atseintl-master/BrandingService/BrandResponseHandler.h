//-------------------------------------------------------------------
//
//  File:        BrandResponseHandler.h
//  Created:     Feb 03, 2008
//  Authors:     Mauricio Dantas
//
//  Description: Members and methods for Brand response XML content
//
//
//  Updates:
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
//-------------------------------------------------------------------
#pragma once

#include "BrandingService/BrandResponseItem.h"
#include "Common/TseEnums.h"
#include "Common/TseStringTypes.h"

#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>

#include <string>

namespace tse
{

class PricingTrx;

class BrandResponseHandler : public xercesc::DefaultHandler
{

public:
  //--------------------------------------------------------------------------
  // @function BrandResponseHandler::BrandResponseHandler
  //
  // Description: constructor
  //
  // @param textHolder - where to put Brand text
  // @param textMap    - where to put Brand text map
  // @param diagHolder - where to put rtg diagnostic info
  //--------------------------------------------------------------------------
  BrandResponseHandler(const PricingTrx& trx,
                       std::vector<BrandResponseItem*>& brandResponseItemVec);

  virtual ~BrandResponseHandler();

  const bool initialize();

  //--------------------------------------------------------------------------
  // @function BrandResponseHandler::startElement
  //
  // Description: Over-ridden SAX2 method that captures the start tag and
  //              where applicable extracts the attributes where applicable
  //
  // @param uri - The URI of the associated namespace for this element
  // @param localname - The local part of the element name
  // @param qname - The QName of this element
  // @param attrs - The attributes attached to this element, if any
  // @return void
  //--------------------------------------------------------------------------
  void startElement(const XMLCh* const uri,
                    const XMLCh* const localname,
                    const XMLCh* const qname,
                    const xercesc::Attributes& attrs) override;

  //--------------------------------------------------------------------------
  // @function BrandResponseHandler::parse
  //
  // Description: Entry point to set content and error handlers.  Initializes
  //              all required XML utilities.  Calls the reader parse routine
  //              to handle the document content.
  //
  // @param chars - The characters from the XML document
  // @param length - The number of characters to read from the array
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  const bool parse(const char* content);

private:
  xercesc::SAX2XMLReader* _reader = nullptr;
  const PricingTrx& _trx;
  std::vector<BrandResponseItem*>& _brandResponseItemVec;
  std::string _content;
  bool _inDts = false;
  std::string _carrierCodeTMP;
  std::string _campaignCodeTMP;
  BrandResponseItem* _brp = nullptr;

  //--------------------------------------------------------------------------
  // @function BrandResponseHandler::processCBRAtts
  //
  // Description: This is to process the attributes of CBR tag
  //              Request Source Information
  // @param attrs - attributes to be processed
  // @return void
  //--------------------------------------------------------------------------
  void processCBRAtts(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // Description: This is to process the attributes of FBC tag
  //               FBC  Fare Basis Codes
  // @param attrs - attributes to be processed
  // @return void
  //--------------------------------------------------------------------------
  void processFBCAtts(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // Description: This is to process the attributes of MSG tag
  //               MSG  Message informatin
  // @param attrs - attributes to be processed
  // @return void
  //--------------------------------------------------------------------------
  void processMSGAtts(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function BrandResponseHandler::endElement
  //
  // Description: Over-ridden SAX2 method that captures the end tag event
  //
  // @param uri - The URI of the associated namespace for this element
  // @param localname - The local part of the element name
  // @param qname - The QName of this element
  // @return void
  //--------------------------------------------------------------------------
  void endElement(const XMLCh* const uri,
                  const XMLCh* const localname,
                  const XMLCh* const qname) override;

  //--------------------------------------------------------------------------
  // @function BrandResponseHandler::characters
  //
  // Description: Over-ridden SAX2 method that captures characters defined by
  //              the current element.
  //
  // @param chars  - The characters from the XML document
  // @param length - The number of characters to read from the array
  // @return void
  //--------------------------------------------------------------------------
  void characters(const XMLCh* const chars, const unsigned int length) override;
};
} // end namespace tse
