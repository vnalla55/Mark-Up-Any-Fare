//----------------------------------------------------------------------------
//  File:           TaxInfoXmlBuilder.h
//  Authors:        Piotr Lach
//  Created:        12/12/2008
//  Description:    TaxInfoXmlBuilder header file for ATSE V2 PFC Display Project.
//                  TaxInfoXmlFormatter and TaxInfoXmlBuilder construct TaxInfo XML response.
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

#include <string>

namespace tse
{
class TaxInfoResponse;

class TaxInfoXmlBuilder final
{
public:
  std::string build(TaxInfoResponse& response);
  std::string response() { return _response; }

private:
  std::string _response;
};

} // namespace tse
