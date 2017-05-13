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

#include <iosfwd>

namespace tax
{
class XmlTagsList;
class TaxDetailsLevel;
class Response;
class Request;

class ResponseToStringBuilder
{
public:
  void buildFrom(const Request* request,
                 const Response& response,
                 const XmlTagsList& xmlTagsList,
                 const TaxDetailsLevel& detailsLevel,
                 std::ostream& responseStream);

  void buildFromBCH(const Request* request,
                    const Response& response,
                    const XmlTagsList& xmlTagsList,
                    std::ostream& responseStream);
};

} // namespace tax

