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

#pragma once

#include "Common/Logger.h"
#include "DataModel/BrandingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "Xform/XMLWriter.h"

#include <string>

namespace tse
{

// Builds XML containing the response to a branding request N06=N
// Bases mainly on contents of the BrandingResponseType structure
//
// Response format example:
//
// <ShoppingResponse STK="picli304:27036 2013-08-08T05:16:26 3d2" Q0S="8">
//   <ITN NUM="0">                # itin number
//     <GRI SB2="BS">             # brand ID (user friendly code)
//       <PRG SC0="122"/>         # program ID (numeric value)
//       <PRG SC0="123"/>
//       ...
//     </GRI>
//     <GRI ...> ... </GRI>
//     ...
//   </ITN>
//   <ITN ...> ... </ITN>
//   ...
// </ShoppingResponse>
class XMLBrandingResponse
{
public:
  XMLBrandingResponse(const BrandingTrx& trx) : _trx(trx), _diagType(0) {}

  // Sets the transaction token, e.g. picli304:27036 2013-08-08T05:16:26 3d2
  // If the token is set and non-empty, it will be contained in the response
  void setToken(const std::string& token) { _token = token; }

  void setDiagOutput(int diagType, const std::string& diagText);

  // Builds an XML response to a branding request
  // taking the content from the passed BrandingResponseType structure
  void buildXmlBrandingResponse(std::string& outputString,
                                const BrandingResponseType& response);

private:
  typedef XMLWriter::Node Node;

  void responseStructToNodes(XMLWriter& w,
                             const BrandingResponseType& responseStruct);

  static void printProgramsIntoBrands(XMLWriter& w, const ProgramIdSet& programSet);
  static void createDiagnosticNode(XMLWriter& w, int diagType, const std::string& diagText);

  const BrandingTrx& _trx;
  std::string _token;
  int _diagType;
  std::string _diagText;

  static Logger _logger;
};

} // namespace tse

