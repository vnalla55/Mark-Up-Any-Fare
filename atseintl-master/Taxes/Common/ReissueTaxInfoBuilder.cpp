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

#include "Common/DateTime.h"
#include "Common/TrxUtil.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxReissue.h"
#include "Diagnostic/DiagCollector.h"
#include "Taxes/Common/ReissueTaxInfoBuilder.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"

#include <iterator>

namespace tse
{

ReissueTaxInfoBuilder::ReissueTaxInfoBuilder() : _reissueTaxInfo(), _diag(nullptr)
{
}

ReissueTaxInfoBuilder::~ReissueTaxInfoBuilder()
{
}

void
ReissueTaxInfoBuilder::ReissueTaxInfo::setDefaultValues()
{
  reissueRestrictionApply = false; // PXF
  taxApplyToReissue = false; // PXG
  reissueTaxRefundable = true; // PXH
  reissueTaxCurrency = ""; // C79
  reissueTaxAmount = 0; // C80
}

void
ReissueTaxInfoBuilder::ReissueTaxInfo::setMatchedValues(const TaxReissue& taxReissue)
{
  if (taxReissue.refundInd() == NO) // PXH
    reissueTaxRefundable = false;

  if (!taxReissue.currencyCode().empty()) // C79
    reissueTaxCurrency = taxReissue.currencyCode();

  if (taxReissue.taxAmt() != 0) // C80
    reissueTaxAmount = taxReissue.taxAmt();

  taxApplyToReissue = true; // PXG
}

const tse::Loc*
ReissueTaxInfoBuilder::getReissueLocation(PricingTrx& trx)
{
  const tse::Loc* exchangeReissueLoc = nullptr;
  BaseExchangeTrx* bExcTrx = dynamic_cast<BaseExchangeTrx*>(&trx);
  if (bExcTrx && !bExcTrx->reissueLocation().empty())
  {
    exchangeReissueLoc =
        trx.dataHandle().getLoc(bExcTrx->reissueLocation(), trx.getRequest()->ticketingDT());
    if (exchangeReissueLoc != nullptr)
      return exchangeReissueLoc;
  }
  exchangeReissueLoc = TrxUtil::ticketingLoc(trx);
  return exchangeReissueLoc;
}

bool
ReissueTaxInfoBuilder::matchReissueLocation(const TaxReissue& taxReissue,
                                            const tse::Loc* exchangeReissueLoc,
                                            const DateTime& ticketingDate)
{
  bool matchLoc = false;
  if (taxReissue.reissueLoc().empty())
  {
    matchLoc = true;
  }
  else
  {
    if (exchangeReissueLoc == nullptr)
      return false;

    if (LocUtil::isInLoc(*exchangeReissueLoc,
                         taxReissue.reissueLocType(),
                         taxReissue.reissueLoc(),
                         SABRE_USER,
                         MANUAL,
                         LocUtil::OTHER,
                         GeoTravelType::International,
                         EMPTY_STRING(),
                         ticketingDate))
      matchLoc = true;
  }

  if (taxReissue.reissueExclLocInd() == YES)
    return !matchLoc;

  return matchLoc;
}

bool
ReissueTaxInfoBuilder::matchTicketingCarrier(const TaxReissue& taxReissue,
                                             const CarrierCode& validatingCarrier)
{
  bool matchTktCxr = false;

  if (taxReissue.validationCxr().empty())
  {
    matchTktCxr = true;
  }
  else
  {
    std::vector<CarrierCode>::const_iterator carrierCodeIter = taxReissue.validationCxr().begin();
    const std::vector<CarrierCode>::const_iterator carrierCodeEndIter =
        taxReissue.validationCxr().end();
    for (; carrierCodeIter != carrierCodeEndIter; carrierCodeIter++)
    {
      if (validatingCarrier == *carrierCodeIter)
      {
        matchTktCxr = true;
        break;
      }
    }
  }

  if (taxReissue.tktlCxrExclInd() == YES)
    return !matchTktCxr;

  return matchTktCxr;
}

const ReissueTaxInfoBuilder::ReissueTaxInfo&
ReissueTaxInfoBuilder::build(PricingTrx& trx, const TaxCode& taxCode, const TaxType& taxType)
{
  _reissueTaxInfo.setDefaultValues();

  ExchangePricingTrx* exTrx = dynamic_cast<ExchangePricingTrx*>(&trx);
  if (exTrx && exTrx->reqType() == AGENT_PRICING_MASK)
    return _reissueTaxInfo;

  const std::vector<TaxReissue*>& unfilteredTaxReissueVec =
      ( trx.dataHandle().getTaxReissue(taxCode, trx.getRequest()->ticketingDT()) );

  std::vector<TaxReissue*> taxReissueVec;
  std::copy_if(unfilteredTaxReissueVec.begin(),
               unfilteredTaxReissueVec.end(),
               std::back_inserter(taxReissueVec),
               [taxType](TaxReissue* reissue) { return reissue->taxType() == taxType; }
  );

  const tse::Loc* exchangeReissueLoc = getReissueLocation(trx);

  diag806Header(trx, taxReissueVec, taxCode, exchangeReissueLoc);
  if (taxReissueVec.empty())
  {
    return _reissueTaxInfo;
  }

  _reissueTaxInfo.reissueRestrictionApply = true;

  std::vector<TaxReissue*>::const_iterator taxReissueIter = taxReissueVec.begin();
  std::vector<TaxReissue*>::const_iterator taxReissueEndIter = taxReissueVec.end();

  for (; taxReissueIter != taxReissueEndIter; taxReissueIter++)
  {
    const TaxReissue& taxReissue = *(*taxReissueIter);

    if (!matchReissueLocation(taxReissue, exchangeReissueLoc, trx.getRequest()->ticketingDT()))
      continue;

    if (!matchTicketingCarrier(taxReissue, trx.getRequest()->validatingCarrier()))
      continue;

    _reissueTaxInfo.setMatchedValues(taxReissue);

    diag806MatchingSeq(*taxReissueIter);
    return _reissueTaxInfo;
  }
  diag806MatchingSeq(nullptr);

  return _reissueTaxInfo;
}

void
ReissueTaxInfoBuilder::diag806Header(PricingTrx& trx,
                                     const std::vector<TaxReissue*>& taxReissueVec,
                                     const TaxCode& taxCode,
                                     const tse::Loc* exchangeReissueLoc)
{
  if (_diag == nullptr)
    return;

  DiagCollector& dc = *_diag;
  dc << std::endl;
  dc << "************************************************************ " << std::endl;
  dc << "TAX CODE: " << taxCode;
  dc << "  REISSUE LOC: ";

  if (exchangeReissueLoc)
    dc << exchangeReissueLoc->loc();
  else
    dc << "INVALID";

  dc << "  VALIDATING CXR: " << trx.getRequest()->validatingCarrier() << std::endl;
  dc << "************************************************************ " << std::endl;
  dc << " SN   EXC/LOC  RFND  AMOUNT    EXC/CXR                       " << std::endl;
  dc << "------------------------------------------------------------ " << std::endl;

  if (taxReissueVec.empty())
  {
    dc << " NO ITEMS FOUND IN TAX REISSUE TABLE " << std::endl;
    dc << "************************************************************ " << std::endl;
    dc << std::endl;
    return;
  }

  std::vector<TaxReissue*>::const_iterator taxReissueIter = taxReissueVec.begin();
  std::vector<TaxReissue*>::const_iterator taxReissueEndIter = taxReissueVec.end();

  for (; taxReissueIter != taxReissueEndIter; taxReissueIter++)
  {
    const TaxReissue& taxReissue = *(*taxReissueIter);
    diag806TaxSeq(taxReissue);
  }

  dc << std::endl;
}

void
ReissueTaxInfoBuilder::diag806TaxSeq(const TaxReissue& taxReissue)
{
  if (_diag == nullptr)
    return;

  DiagCollector& dc = *_diag;
  dc << std::setw(4) << taxReissue.seqNo();

  if (taxReissue.reissueExclLocInd() == YES)
    dc << "  Y/";
  else
    dc << "  N/";

  dc << taxReissue.reissueLocType() << "-" << taxReissue.reissueLoc() << "   "
     << taxReissue.refundInd();

  dc.unsetf(std::ios::right);
  dc.setf(std::ios::left, std::ios::adjustfield);

  MoneyAmount amt = 0.0;
  CurrencyCode cur = "   ";

  if (taxReissue.taxAmt() != 0)
    amt = taxReissue.taxAmt();

  if (!taxReissue.currencyCode().empty())
    cur = taxReissue.currencyCode();

  if (taxReissue.taxAmt() != 0)
    dc << "    " << std::setw(9) << Money(amt, cur);
  else
    dc << "    0 " << cur << "   ";

  if (taxReissue.tktlCxrExclInd() == YES)
    dc << "  Y/";
  else
    dc << "  N/";

  std::vector<CarrierCode>::const_iterator carrierCodeIter = taxReissue.validationCxr().begin();
  const std::vector<CarrierCode>::const_iterator carrierCodeEndIter =
      taxReissue.validationCxr().end();

  for (; carrierCodeIter != carrierCodeEndIter; carrierCodeIter++)
  {
    dc << *carrierCodeIter << " ";
  }

  dc << std::endl;
}

void
ReissueTaxInfoBuilder::diag806MatchingSeq(const TaxReissue* taxReissue)
{
  if (_diag == nullptr)
    return;
  DiagCollector& dc = *_diag;
  dc << "MATCHING SEQUENCE : ";
  if (taxReissue == nullptr)
    dc << " NONE " << std::endl;
  else
    dc << taxReissue->seqNo() << std::endl;
  dc << "************************************************************ " << std::endl;
  dc << std::endl;
}

} // namespace tse
