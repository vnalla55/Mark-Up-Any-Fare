// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include "TestServer/Xform/XmlWriter.h"
#include "TestServer/Xform/ResponseToStringBuilder.h"
#include "AtpcoTaxes/Factories/OutputConverter.h"
#include "AtpcoTaxes/DataModel/RequestResponse/BCHOutputResponse.h"
#include "AtpcoTaxes/DataModel/RequestResponse/OutputResponse.h"
#include "AtpcoTaxes/DomainDataObjects/Response.h"
#include "AtpcoTaxes/DomainDataObjects/Request.h"

namespace tax
{

void
ResponseToStringBuilder::buildFrom(const Request* request,
                                   const Response& response,
                                   const XmlTagsList& xmlTagsList,
                                   const TaxDetailsLevel& detailsLevel,
                                   std::ostream& responseStream)
{
  writeXmlResponse(responseStream,
                   OutputConverter::convertToTaxRs(response, detailsLevel, request),
                   xmlTagsList);
}

void
ResponseToStringBuilder::buildFromBCH(const Request* request,
                                      const Response& response,
                                      const XmlTagsList& xmlTagsList,
                                      std::ostream& responseStream)
{
  writeXmlResponse(responseStream,
                   OutputConverter::convertToBCHTaxRs(response, request),
                   xmlTagsList);
}

} // namespace tax
