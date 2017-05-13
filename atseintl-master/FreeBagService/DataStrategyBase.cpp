//-------------------------------------------------------------------
//  Copyright Sabre 2011
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------
#include "FreeBagService/DataStrategyBase.h"

#include "Common/FreeBaggageUtil.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/SubCodeInfo.h"
#include "Diagnostic/Diag852Collector.h"
#include "FreeBagService/BaggageTextUtil.h"
#include "FreeBagService/BaggageTravelInfo.h"

namespace tse
{
DataStrategyBase::DataStrategyBase(PricingTrx& trx) : _trx(trx) {}

DataStrategyBase::~DataStrategyBase() {}

void
DataStrategyBase::printS7ProcessingContext(BaggageTravel* baggageTravel,
                                           const BaggageTravelInfo& bagInfo,
                                           const SubCodeInfo* s5,
                                           bool isUsDot,
                                           Diag852Collector* dc,
                                           bool defer,
                                           bool isCarrierOverride) const
{
  // baggage travels are cloned for every fare path,
  // we should index them through the original range only

  // TODO: BaggageTextUtil::isMarketingCxrUsed(_trx, isUsDot, defer) should be enough but
  // there was a bug in old logic here and I'll fix it the next delivery.
  const bool useMark = TrxUtil::isIataReso302MandateActivated(_trx)
                           ? BaggageTextUtil::isMarketingCxrUsed(_trx, isUsDot, defer)
                           : isUsDot;
  dc->printS7ProcessingContext(
      _trx, baggageTravel, isUsDot, bagInfo.bagIndex() + 1, useMark, s5, defer, isCarrierOverride);
}

const std::vector<SubCodeInfo*>&
DataStrategyBase::retrieveS5Records(const VendorCode& vendor, const CarrierCode& carrier) const
{
  return FreeBaggageUtil::retrieveS5Records(vendor, carrier, _trx);
}

bool
DataStrategyBase::checkFareLineAndCheckedPortion(const BaggageTravel* baggageTravel,
                                                 const BaggageTravelInfo& bagInfo,
                                                 const Diag852Collector* dc) const
{
  if (UNLIKELY(dc))
  {
    const Itin* itin = baggageTravel->itin();
    const PricingTrx* trx = baggageTravel->_trx;

    bool isValidFareLine;
    if (nullptr == dynamic_cast<const AncillaryPricingTrx*>(trx))
      isValidFareLine = (dc->fareLine() == bagInfo.fareIndex() + 1);
    else
      isValidFareLine = dc->checkFl(*itin);

    if (isValidFareLine)
    {
      if (dc->diagType() >= Diag852Collector::CPACTIVE)
      {
        return dc->checkedPortion() == bagInfo.bagIndex() + 1;
      }
      return true;
    }
  }
  return false;
}

bool
DataStrategyBase::isAllowanceCarrierOverridden() const
{
  return !getAllowanceCarrierOverridden().empty();
}

bool
DataStrategyBase::isChargesCarrierOverridden() const
{
  return !getChargesCarrierOverridden().empty();
}

CarrierCode
DataStrategyBase::getAllowanceCarrierOverridden() const
{
  AncRequest* req = dynamic_cast<AncRequest*>(_trx.getRequest());
  return (req && req->majorSchemaVersion() >= 2) ? req->carrierOverriddenForBaggageAllowance()
                                                 : CarrierCode();
}

CarrierCode
DataStrategyBase::getChargesCarrierOverridden() const
{
  AncRequest* req = dynamic_cast<AncRequest*>(_trx.getRequest());
  return (req && req->majorSchemaVersion() >= 2) ? req->carrierOverriddenForBaggageCharges()
                                                 : CarrierCode();
}
} // tse
