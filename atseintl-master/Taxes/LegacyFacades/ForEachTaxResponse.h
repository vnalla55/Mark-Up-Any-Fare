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
#pragma once

#include "Common/TrxUtil.h"
#include "DataModel/NetFarePath.h"
#include "DataModel/PricingTrx.h"

using namespace tse;

namespace tse
{

}

namespace tax
{

namespace detail
{

tse::TaxResponse*
findTaxResponse(const tse::Itin* itin, const tse::FarePath* farepath);
tse::TaxResponse*
findTaxResponse(const tse::Itin* itin, const tse::FarePath* farepath, const CarrierCode& valCxr);

}

template <typename F>
void
forEachTaxResponse(tse::PricingTrx& trx, F f)
{
  const bool isCat35TFSFEnabled = tse::TrxUtil::isCat35TFSFEnabled(trx);
  for(tse::Itin * itin : trx.itin())
  {
    for(tse::FarePath * farePath : itin->farePath())
    {
      for(tse::TaxResponse* taxResponse : itin->getTaxResponses())
      {
        if (taxResponse->farePath() == farePath)
          f(*taxResponse, *itin, *farePath);

        tse::FarePath* fp = farePath->findTaggedFarePath(taxResponse->validatingCarrier());
        if (fp && taxResponse->farePath() == fp)
          f(*taxResponse, *itin, *fp);

        if (UNLIKELY(isCat35TFSFEnabled && farePath->netFarePath()))
        {
          tse::TaxResponse* netTaxResponse =
              detail::findTaxResponse(itin, farePath->netFarePath(), taxResponse->validatingCarrier());
          if (netTaxResponse)
            f(*netTaxResponse, *itin, *farePath->netFarePath());
        }

        if (UNLIKELY(farePath->adjustedSellingFarePath()))
        {
          tse::TaxResponse* adjTaxResponse =
            detail::findTaxResponse(itin, farePath->adjustedSellingFarePath(), taxResponse->validatingCarrier());
          if (adjTaxResponse)
            f(*adjTaxResponse, *itin, *farePath->adjustedSellingFarePath());
        }
      }
    }
  }
}

}

