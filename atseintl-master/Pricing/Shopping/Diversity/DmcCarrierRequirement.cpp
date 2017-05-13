// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "Pricing/Shopping/Diversity/DmcCarrierRequirement.h"
#include "DataModel/Diversity.h"
#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/DiagCollector.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/PQ/SoloPQItem.h"
#include "Pricing/Shopping/PQ/SopCombinationUtil.h"

#include <algorithm>

namespace tse
{

DmcCarrierRequirement::DmcCarrierRequirement(DmcRequirementsSharedContext& sharedCtx)
  : _isFareCutoffReached(false), _carriersWereAdjusted(false), _sharedCtx(sharedCtx)
{
}

DmcCarrierRequirement::Value
DmcCarrierRequirement::getStatus() const
{
  if (_isFareCutoffReached)
    return 0;

  return (!isAllCarriersSatisfied() ? NEED_CARRIERS : 0);
}

DmcCarrierRequirement::Value
DmcCarrierRequirement::getPQItemCouldSatisfy(const shpq::SoloPQItem* pqItem) const
{
  if (_isFareCutoffReached)
    return 0;

  Value result = 0;

  // We always "could" satisfy C on SP and CR levels, so we need to check only on CRC and FPF
  if (pqItem->getLevel() == shpq::SoloPQItem::CRC_LEVEL ||
      pqItem->getLevel() == shpq::SoloPQItem::FPF_LEVEL)
  {
    for (const CarrierCode& cxr : pqItem->getApplicableCxrs())
    {
      if (cxr != Diversity::INTERLINE_CARRIER && isCarrierOptionsNeeded(cxr))
        return NEED_CARRIERS;
    }
  }
  else if (!isAllCarriersSatisfied())
  {
    result = NEED_CARRIERS;
  }

  return result;
}

DmcCarrierRequirement::Value
DmcCarrierRequirement::getCombinationCouldSatisfy(shpq::SopIdxVecArg comb, MoneyAmount price) const
{
  if (_isFareCutoffReached)
    return 0;

  const ShoppingTrx::SchedulingOption* outbound = nullptr, *inbound = nullptr;
  SopCombinationUtil::getSops(_sharedCtx._trx, comb, &outbound, &inbound);

  CarrierCode cxr = SopCombinationUtil::detectCarrier(outbound, inbound);

  bool isInterline = cxr.empty();
  if (isInterline || !isCarrierOptionsNeeded(cxr))
    return 0;

  return NEED_CARRIERS;
}

void
DmcCarrierRequirement::adjustCarrierRequirementsIfNeeded(size_t numberOfSolutionsRequired,
                                                         size_t numberOfSolutionsCollected)
{
  if (!_sharedCtx._trx.getRequest()->isBrandedFaresRequest() && !_carriersWereAdjusted &&
      ((numberOfSolutionsCollected > numberOfSolutionsRequired) ||
       isCarrierRequirementsAdjustmentNeeded()))
  {
    adjustCarrierRequirements();
  }
}

bool
DmcCarrierRequirement::isAllCarriersSatisfied() const
{
  const std::map<CarrierCode, float>& optionsPerCarrier =
      _sharedCtx._diversity.getOptionsPerCarrierMap();

  std::map<CarrierCode, float>::const_iterator it(optionsPerCarrier.begin());
  for (; it != optionsPerCarrier.end(); ++it)
  {
    if (_sharedCtx._stats.getNumOfItinsForCarrier(it->first) < it->second)
      return false;
  }

  return true;
}

void
DmcCarrierRequirement::print() const
{
  DiagCollector& dc = *_sharedCtx._dc;

  const std::map<CarrierCode, float>& optionsPerCarrier(
      _sharedCtx._diversity.getOptionsPerCarrierMap());
  if (optionsPerCarrier.empty())
  {
    dc << " no online carriers present\n";
    return;
  }

  for (const auto& elem : optionsPerCarrier)
  {
    size_t count = _sharedCtx._stats.getNumOfItinsForCarrier(elem.first);
    size_t required = (size_t)(elem.second);
    dc << " " << elem.first << "[" << count << "/" << required << "]";
  }

  dc << "\n";
}

void
DmcCarrierRequirement::printOCOIfSet() const
{
  DiagCollector& dc = *_sharedCtx._dc;
  if (!_sharedCtx._diversity.isOCO())
    return;

  const std::map<CarrierCode, size_t>& carrierOptionsOnline(
      _sharedCtx._stats.getCarrierOptionsOnline());
  if (carrierOptionsOnline.empty())
  {
    dc << " no carriers online present\n";
    return;
  }

  const std::map<CarrierCode, float>& optionsPerCarrier(
          _sharedCtx._diversity.getOptionsPerCarrierMap());

  for (const auto& element: carrierOptionsOnline)
  {
    size_t OnlineCount = element.second;
    size_t required = optionsPerCarrier.find(element.first)->second;
    dc << " " << element.first << "[" << OnlineCount << "/" << required << "]";
  }

  dc << "\n";
}

bool
DmcCarrierRequirement::getSopCouldSatisfy(unsigned legId,
                                          const SOPInfo& sopInfo,
                                          const SopInfosStatistics& statistics) const
{
  if (UNLIKELY(_isFareCutoffReached))
    return false;

  const ShoppingTrx::SchedulingOption& sop =
      SopCombinationUtil::getSop(_sharedCtx._trx, legId, sopInfo._sopIndex);

  const CarrierCode& carrierCode = SopCombinationUtil::detectCarrier(&sop, nullptr);
  return !carrierCode.empty() && isCarrierOptionsNeeded(carrierCode);
}

bool
DmcCarrierRequirement::isCarrierOptionsNeeded(const CarrierCode& carrier) const
{
  const std::map<CarrierCode, size_t>& carrierOptionsOnline(
      _sharedCtx._stats.getCarrierOptionsOnline());
  if (!_sharedCtx._diversity.isOCO() ||
      carrierOptionsOnline.empty())
  {
    return (_sharedCtx._stats.getNumOfItinsForCarrier(carrier) <
            _sharedCtx._diversity.getOptionsPerCarrier(carrier));
  }

  size_t totalOptionsNeeded = _sharedCtx._diversity.getOptionsPerCarrier(carrier);
  size_t onlineCount = 0;
  std::map<CarrierCode, size_t>::const_iterator itr;

  if ((itr=carrierOptionsOnline.find(carrier)) != carrierOptionsOnline.end())
  {
    onlineCount = itr->second;
    return onlineCount < totalOptionsNeeded;
  }
  else
  {
    return (_sharedCtx._stats.getNumOfItinsForCarrier(carrier) <
            _sharedCtx._diversity.getOptionsPerCarrier(carrier));
  }
}

bool
DmcCarrierRequirement::isCarrierRequirementsAdjustmentNeeded() const
{
  const std::map<CarrierCode, float>& optionsPerCarrier(
      _sharedCtx._diversity.getOptionsPerCarrierMap());

  for (const auto& elem : optionsPerCarrier)
  {
    if (_sharedCtx._stats.getNumOfItinsForCarrier(elem.first) > elem.second)
      return false;
  }

  return true;
}

void
DmcCarrierRequirement::adjustCarrierRequirements()
{
  Diversity& diversity = _sharedCtx._diversity;

  const std::map<CarrierCode, float>& cxrReq = diversity.getOptionsPerCarrierMap();

  for (const auto& elem : cxrReq)
  {
    size_t cnt = _sharedCtx._stats.getNumOfItinsForCarrier(elem.first);
    if (cnt == 0)
      cnt = 1;

    if (!diversity.hasDCL())
      diversity.setOptionsPerCarrier(elem.first, std::min((float)cnt, elem.second));
    else
      diversity.setOptionsPerCarrier(elem.first, elem.second);
  }

  _carriersWereAdjusted = true;

  if (DiagCollector* dc = _sharedCtx._dc)
  {
    *dc << "Carrier requirements adjustment:\n";
    _sharedCtx.printCarriersRequirements();
  }
}

} // ns tse
