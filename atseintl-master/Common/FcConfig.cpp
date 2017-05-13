#include "Common/FcConfig.h"

#include "Common/Assert.h"
#include "DataModel/PricingTrx.h"

namespace tse
{
const FcConfig*
FcConfig::create(const PricingTrx* trx, const FareCalcConfig* fcc)
{
  return &trx->dataHandle().safe_create<FcConfig>(trx, fcc);
}

FcConfig::FcConfig(const PricingTrx* trx, const FareCalcConfig* fcc) : _fcc(fcc)
{
  const AltPricingTrx* altTrx = dynamic_cast<const AltPricingTrx*>(trx);
  if (UNLIKELY(altTrx != nullptr))
  {
    _trxType = altTrx->altTrxType();
  }

  _fccText =
      &(trx->dataHandle().getMsgText(fcc->userApplType(), fcc->userAppl(), fcc->pseudoCity()));
}

bool
FcConfig::getMsgAppl(FareCalcConfigText::TextAppl appl, std::string& msg) const
{
  if ((_trxType == AltPricingTrx::WPA && appl >= FareCalcConfigText::WP_RO_INDICATOR) ||
      (_trxType != AltPricingTrx::WPA && appl < FareCalcConfigText::WP_RO_INDICATOR))
  {
    switch (appl)
    {
    case FareCalcConfigText::WPA_RO_INDICATOR:
      appl = FareCalcConfigText::WP_RO_INDICATOR;
      break;
    case FareCalcConfigText::WPA_NO_MATCH_NO_FARE:
      appl = FareCalcConfigText::WP_NO_MATCH_NO_FARE;
      break;
    case FareCalcConfigText::WPA_NO_MATCH_VERIFY_BOOKING_CLASS:
      appl = FareCalcConfigText::WP_NO_MATCH_VERIFY_BOOKING_CLASS;
      break;
    case FareCalcConfigText::WPA_NO_MATCH_REBOOK:
      appl = FareCalcConfigText::WP_NO_MATCH_REBOOK;
      break;
    case FareCalcConfigText::WPA_NO_MATCH_BOOKING_CLASS:
      appl = FareCalcConfigText::WP_NO_MATCH_BOOKING_CLASS;
      break;

    case FareCalcConfigText::WP_RO_INDICATOR:
      appl = FareCalcConfigText::WPA_RO_INDICATOR;
      break;
    case FareCalcConfigText::WP_NO_MATCH_NO_FARE:
      appl = FareCalcConfigText::WPA_NO_MATCH_NO_FARE;
      break;
    case FareCalcConfigText::WP_NO_MATCH_VERIFY_BOOKING_CLASS:
      appl = FareCalcConfigText::WPA_NO_MATCH_VERIFY_BOOKING_CLASS;
      break;
    case FareCalcConfigText::WP_NO_MATCH_REBOOK:
      appl = FareCalcConfigText::WPA_NO_MATCH_REBOOK;
      break;
    case FareCalcConfigText::WP_NO_MATCH_BOOKING_CLASS:
      appl = FareCalcConfigText::WPA_NO_MATCH_BOOKING_CLASS;
      break;
    default:
      break;
    }
  }
  return _fccText->getApplMsg(appl, msg);
}
}
