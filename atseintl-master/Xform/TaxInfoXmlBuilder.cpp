// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2009
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

#include "DataModel/TaxInfoResponse.h"
#include "Xform/TaxInfoXmlBuilder.h"

#include <tuple>
#include <utility>

namespace tse
{
namespace
{
template <class Tuple, size_t... Indices>
inline std::string
addAllAttrImpl(const Tuple& t1, const Tuple& t2, std::index_sequence<Indices...>)
{
  using NameValuePair = std::pair<const std::string&, const std::string&>;
  std::string response;
  response.reserve(128);

  for (const auto& nv : {NameValuePair(std::get<Indices>(t1), std::get<Indices>(t2))...})
    if (!nv.second.empty())
      response += nv.first + "=\"" + nv.second + "\" ";

  return response;
}

template <class Tuple>
inline std::string
addAllAttr(const Tuple& t1, const Tuple& t2)
{
  return addAllAttrImpl(t1, t2, std::make_index_sequence<std::tuple_size<Tuple>::value>{});
}
} // ns

std::string
TaxInfoXmlBuilder::build(TaxInfoResponse& response)
{
  std::string xmlResponse;
  xmlResponse.reserve(256);
  xmlResponse += "<TAX ";
  xmlResponse += addAllAttr(response.taxAttrNames(), response.taxAttrValues());

  if (response.aptAttrValues().empty())
    return xmlResponse + "/>";
  else
    xmlResponse += ">";

  for (const TaxInfoResponse::AptItem& aptItem : response.aptAttrValues())
  {
    xmlResponse += "<APT ";
    xmlResponse += addAllAttr(response.aptAttrNames(), aptItem);
    xmlResponse += "/>";
  }

  if (response.aptAttrValues().empty())
    xmlResponse += "/>";
  else
    xmlResponse += "</TAX>";

  return xmlResponse;
}

} // ns tse
