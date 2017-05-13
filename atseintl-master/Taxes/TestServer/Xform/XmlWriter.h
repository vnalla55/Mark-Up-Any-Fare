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

class BCHOutputResponse;
class OutputResponse;
class InputRequest;
struct InputRequestWithCache;
struct InputRequestWithStringCache
{
  const InputRequest& request;
  const std::string& cache;
};
class XmlTagsList;

void writeXmlResponse(std::ostream& out, const OutputResponse& response, const XmlTagsList& xmlTagslist);
void writeXmlResponse(std::ostream& out, const BCHOutputResponse& response, const XmlTagsList& xmlTagslist);
void writeXmlRequest(std::ostream& out, const InputRequest& request, const XmlTagsList& xmlTagslist);
void writeXmlRequest(std::ostream& out, const InputRequest& request, const std::string& cache, const XmlTagsList& xmlTagslist);

} // namespace tax

