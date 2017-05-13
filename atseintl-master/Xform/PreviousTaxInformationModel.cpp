// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "Common/FallbackUtil.h"
#include "DataModel/Billing.h"
#include "DataModel/NetFarePath.h"
#include "Xform/PreviousTaxInformationModel.h"
#include "Xform/TaxBreakdownModel.h"
#include "Xform/TaxInformationModel.h"

namespace tse
{
FALLBACK_DECL(cat33DisablePssPath);

PreviousTaxInformationModel::PreviousTaxInformationModel(PricingTrx& trx)
{
  if (trx.billing() == nullptr || trx.billing()->requestPath() != "PSS" ||
      fallback::cat33DisablePssPath(&trx))
  {
    for (const TaxResponse* taxResponse : trx.getExcItinTaxResponse())
    {
      if (dynamic_cast<const NetFarePath*>(taxResponse->farePath()) != nullptr)
      {
        for (TaxRecord* taxRecord : taxResponse->taxRecordVector())
          _netTaxInformation.emplace_back(new TaxInformationModel(*taxRecord, trx));

        for (TaxItem* taxItem : taxResponse->taxItemVector())
          _netTaxBreakdown.emplace_back(new TaxBreakdownModel(trx, *taxItem));
      }
      else
      {
        for (TaxRecord* taxRecord : taxResponse->taxRecordVector())
          _taxInformation.emplace_back(new TaxInformationModel(*taxRecord, trx));

        for (TaxItem* taxItem : taxResponse->taxItemVector())
          _taxBreakdown.emplace_back(new TaxBreakdownModel(trx, *taxItem));
      }
    }
  }

  for (const TaxResponse* taxResponse : trx.getTaxInfoResponse())
  {
    for (TaxItem* taxItem : taxResponse->taxItemVector())
      _taxInfoBreakdown.emplace_back(new TaxBreakdownModel(trx, *taxItem));
  }

  const bool isSellEmpty =
      _taxInformation.empty() && _taxBreakdown.empty() && _taxInfoBreakdown.empty();

  _isNetEmpty = !fallback::cat33DisablePssPath(&trx)
                    ? _netTaxInformation.empty() && _netTaxBreakdown.empty()
                    : _netTaxInformation.empty() || _netTaxBreakdown.empty();

  _isEmpty = !fallback::cat33DisablePssPath(&trx) && isSellEmpty && _isNetEmpty;
}

const std::vector<std::unique_ptr<AbstractTaxBreakdownModel>>&
PreviousTaxInformationModel::getTaxInfoBreakdown() const
{
  return _taxInfoBreakdown;
}

const std::vector<std::unique_ptr<AbstractTaxBreakdownModel>>&
PreviousTaxInformationModel::getTaxBreakdown() const
{
  return _taxBreakdown;
}

const std::vector<std::unique_ptr<AbstractTaxInformationModel>>&
PreviousTaxInformationModel::getTaxInformation() const
{
  return _taxInformation;
}

const std::vector<std::unique_ptr<AbstractTaxBreakdownModel>>&
PreviousTaxInformationModel::getNetTaxBreakdown() const
{
  return _netTaxBreakdown;
}

const std::vector<std::unique_ptr<AbstractTaxInformationModel>>&
PreviousTaxInformationModel::getNetTaxInformation() const
{
  return _netTaxInformation;
}

} // end of tse namespace
