// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

namespace tax
{

class BCHOutputResponse;
class OutputResponse;
class Response;
class Request;
class TaxDetailsLevel;

class OutputConverter
{
public:
  static OutputResponse
  convertToTaxRs(const Response& response,
                 const TaxDetailsLevel& detailsLevel,
                 const Request* request = nullptr);

  static BCHOutputResponse
  convertToBCHTaxRs(const Response& response,
                    const Request* request = nullptr);
};

} // namespace tax

