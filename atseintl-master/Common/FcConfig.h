#pragma once

#include "DataModel/AltPricingTrx.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/FareCalcConfigText.h"

namespace tse
{

class PricingTrx;
class FareCalcConfig;

class FcConfig
{
  friend class DataHandle;
  friend class NoPNRFcConfigWrapper;

public:
  static const FcConfig* create(const PricingTrx* trx, const FareCalcConfig* fcc);

  FcConfig(const PricingTrx* trx, const FareCalcConfig* fcc);
  virtual ~FcConfig() = default;

  Indicator userApplType() const { return _fcc->userApplType(); }

  const UserApplCode& userAppl() const { return _fcc->userAppl(); }
  const PseudoCityCode& pseudoCity() const { return _fcc->pseudoCity(); }
  const LocKey& loc1() const { return _fcc->loc1(); }

  Indicator itinDisplayInd() const { return _fcc->itinDisplayInd(); }
  Indicator wpPsgTypDisplay() const { return _fcc->wpPsgTypDisplay(); }
  Indicator itinHeaderTextInd() const { return _fcc->itinHeaderTextInd(); }
  Indicator wpConnectionInd() const { return _fcc->wpConnectionInd(); }
  Indicator wpChildInfantFareBasis() const { return _fcc->wpChildInfantFareBasis(); }
  Indicator fareBasisTktDesLng() const { return _fcc->fareBasisTktDesLng(); }
  Indicator truePsgrTypeInd() const { return _fcc->truePsgrTypeInd(); }
  Indicator fareTaxTotalInd() const { return _fcc->fareTaxTotalInd(); }
  Indicator noofTaxBoxes() const { return _fcc->noofTaxBoxes(); }
  int baseTaxEquivTotalLength() const { return _fcc->baseTaxEquivTotalLength(); }
  Indicator taxPlacementInd() const { return _fcc->taxPlacementInd(); }
  Indicator taxCurCodeDisplayInd() const { return _fcc->taxCurCodeDisplayInd(); }
  Indicator zpAmountDisplayInd() const { return _fcc->zpAmountDisplayInd(); }
  Indicator taxExemptionInd() const { return _fcc->taxExemptionInd(); }
  Indicator taxExemptBreakdownInd() const { return _fcc->taxExemptBreakdownInd(); }
  Indicator fcPsgTypDisplay() const { return _fcc->fcPsgTypDisplay(); }
  Indicator fcChildInfantFareBasis() const { return _fcc->fcChildInfantFareBasis(); }
  Indicator fcConnectionInd() const { return _fcc->fcConnectionInd(); }
  Indicator domesticNUC() const { return _fcc->domesticNUC(); }
  Indicator tvlCommencementDate() const { return _fcc->tvlCommencementDate(); }
  Indicator wrapAround() const { return _fcc->wrapAround(); }
  Indicator multiSurchargeSpacing() const { return _fcc->multiSurchargeSpacing(); }
  Indicator domesticISI() const { return _fcc->domesticISI(); }
  Indicator internationalISI() const { return _fcc->internationalISI(); }
  Indicator displayBSR() const { return _fcc->displayBSR(); }
  Indicator endorsements() const { return _fcc->endorsements(); }
  Indicator warningMessages() const { return _fcc->warningMessages(); }
  Indicator lastDayTicketDisplay() const { return _fcc->lastDayTicketDisplay(); }
  Indicator lastDayTicketOutput() const { return _fcc->lastDayTicketOutput(); }
  Indicator reservationOverride() const { return _fcc->reservationOverride(); }
  Indicator fareBasisDisplayOption() const { return _fcc->fareBasisDisplayOption(); }
  Indicator globalSidetripInd() const { return _fcc->globalSidetripInd(); }

  Indicator wpaPermitted() const { return _fcc->wpaPermitted(); }
  Indicator wpaPsgDtlFormat() const
  {
    return (_trxType == AltPricingTrx::WPA ? _fcc->wpaPsgDtlFormat() : _fcc->wpPsgDtlFormat());
  }
  Indicator wpaFareLinePsgType() const
  {
    return (_trxType == AltPricingTrx::WPA ? _fcc->wpaFareLinePsgType()
                                           : _fcc->wpFareLinePsgType());
  }
  Indicator wpaFareLineHdr() const
  {
    return (_trxType == AltPricingTrx::WPA ? _fcc->wpaFareLineHdr() : _fcc->wpFareLineHdr());
  }
  Indicator wpaPrimePsgRefNo() const
  {
    return (_trxType == AltPricingTrx::WPA ? _fcc->wpaPrimePsgRefNo() : _fcc->wpPrimePsgRefNo());
  }
  Indicator wpa2ndPsgRefNo() const
  {
    return (_trxType == AltPricingTrx::WPA ? _fcc->wpa2ndPsgRefNo() : _fcc->wp2ndPsgRefNo());
  }
  int wpaFareOptionMaxNo() const
  {
    return (_trxType == AltPricingTrx::WPA ? _fcc->wpaFareOptionMaxNo()
                                           : _fcc->wpFareOptionMaxNo());
  }
  Indicator wpaSort() const
  {
    return (_trxType == AltPricingTrx::WPA ? _fcc->wpaSort() : _fcc->wpSort());
  }
  Indicator wpaShowDupAmounts() const
  {
    return (_trxType == AltPricingTrx::WPA ? _fcc->wpaShowDupAmounts() : _fcc->wpShowDupAmounts());
  }
  Indicator wpaPsgLineBreak() const
  {
    return (_trxType == AltPricingTrx::WPA ? _fcc->wpaPsgLineBreak() : _fcc->wpPsgLineBreak());
  }
  Indicator wpaPsgMultiLineBreak() const
  {
    return (_trxType == AltPricingTrx::WPA ? _fcc->wpaPsgMultiLineBreak()
                                           : _fcc->wpPsgMultiLineBreak());
  }
  Indicator wpaNoMatchHigherCabinFare() const
  {
    return (_trxType == AltPricingTrx::WPA ? _fcc->wpaNoMatchHigherCabinFare()
                                           : _fcc->wpNoMatchHigherCabinFare());
  }
  Indicator wpaStoreWithoutRebook() const
  {
    return (_trxType == AltPricingTrx::WPA ? _fcc->wpaStoreWithoutRebook()
                                           : _fcc->wpStoreWithoutRebook());
  }
  Indicator wpaAccTvlOption() const
  {
    return (_trxType == AltPricingTrx::WPA ? _fcc->wpaAccTvlOption() : _fcc->wpAccTvlOption());
  }
  Indicator wpaAccTvlCat13() const
  {
    return (_trxType == AltPricingTrx::WPA ? _fcc->wpaAccTvlCat13() : _fcc->wpAccTvlCat13());
  }
  Indicator wpaRoInd() const
  {
    return (_trxType == AltPricingTrx::WPA ? _fcc->wpaRoInd() : _fcc->wpRoInd());
  }

  Indicator wpNoMatchPermitted() const { return _fcc->wpNoMatchPermitted(); }
  Indicator wpPsgDtlFormat() const { return _fcc->wpPsgDtlFormat(); }
  Indicator wpFareLinePsgType() const { return _fcc->wpFareLinePsgType(); }
  Indicator wpFareLineHdr() const { return _fcc->wpFareLineHdr(); }
  Indicator wpPrimePsgRefNo() const { return _fcc->wpPrimePsgRefNo(); }
  Indicator wp2ndPsgRefNo() const { return _fcc->wp2ndPsgRefNo(); }
  int wpFareOptionMaxNo() const { return _fcc->wpFareOptionMaxNo(); }
  Indicator wpSort() const { return _fcc->wpSort(); }
  Indicator wpShowDupAmounts() const { return _fcc->wpShowDupAmounts(); }
  Indicator wpPsgLineBreak() const { return _fcc->wpPsgLineBreak(); }
  Indicator wpPsgMultiLineBreak() const { return _fcc->wpPsgMultiLineBreak(); }
  Indicator wpNoMatchHigherCabinFare() const { return _fcc->wpNoMatchHigherCabinFare(); }
  Indicator wpStoreWithoutRebook() const { return _fcc->wpStoreWithoutRebook(); }
  Indicator wpAccTvlOption() const { return _fcc->wpAccTvlOption(); }
  Indicator wpAccTvlCat13() const { return _fcc->wpAccTvlCat13(); }
  Indicator wpRoInd() const { return _fcc->wpRoInd(); }

  Indicator valCxrDisplayOpt() const { return _fcc->valCxrDisplayOpt(); }
  Indicator negPermitted() const { return _fcc->negPermitted(); }
  Indicator noMatchAvail() const { return _fcc->noMatchAvail(); }
  Indicator wpWpaTrailerMsg() const { return _fcc->wpWpaTrailerMsg(); }
  Indicator interlineTktPermitted() const { return _fcc->interlineTktPermitted(); }
  Indicator participatingAgreement() const { return _fcc->participatingAgreement(); }
  Indicator ietPriceInterlineActive() const { return _fcc->ietPriceInterlineActive(); }

  const std::vector<FareCalcConfigSeg*>& segs() const { return _fcc->segs(); }

  const FareCalcConfig* fcc() const { return _fcc; }

  virtual bool getMsgAppl(FareCalcConfigText::TextAppl appl, std::string& msg) const;

private:
  FcConfig() {}
  const FareCalcConfig* _fcc = nullptr;
  const FareCalcConfigText* _fccText = nullptr;
  AltPricingTrx::AltTrxType _trxType = AltPricingTrx::AltTrxType::WP;
};

} // namespace tse

