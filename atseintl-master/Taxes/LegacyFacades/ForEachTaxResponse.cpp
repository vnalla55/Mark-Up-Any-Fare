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
#include "Taxes/LegacyFacades/ForEachTaxResponse.h"
#include "DataModel/TaxResponse.h"

namespace tax
{

namespace detail
{

tse::TaxResponse*
findTaxResponse(const tse::Itin* itin, const tse::FarePath* farepath)
{
  for(tse::TaxResponse *response : itin->getTaxResponses())
  {
    if (response->farePath() == farepath)
      return response;
  }

  return nullptr;
}

tse::TaxResponse*
findTaxResponse(const tse::Itin* itin, const tse::FarePath* farepath, const CarrierCode& valCxr)
{
  for(tse::TaxResponse *response : itin->getTaxResponses())
  {
    if (response->farePath() == farepath && response->validatingCarrier() == valCxr)
      return response;
  }

  return nullptr;
}

}

}

