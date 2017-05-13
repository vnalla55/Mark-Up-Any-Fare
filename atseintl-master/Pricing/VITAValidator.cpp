#include "Pricing/VITAValidator.h"

#include "Common/Assert.h"
#include "Common/ItinUtil.h"
#include "Common/TravelSegAnalysis.h"
#include "DataModel/InterlineTicketCarrierData.h"
#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/Diag910Collector.h"
#include "Diagnostic/DiagManager.h"

#include <cassert>

namespace tse
{
VITAValidator::VITAValidator(ShoppingTrx& trx)
  : _trx(trx), _carrier(nullptr), _validatingCarrier(_trx), _interlineTicketCarrierData(nullptr)
{
  createInterlineTicketCarrier();
}

VITAValidator::VITAValidator(ShoppingTrx& trx, const CarrierCode* carrier)
  : _trx(trx), _carrier(carrier), _validatingCarrier(_trx), _interlineTicketCarrierData(nullptr)
{
  createInterlineTicketCarrier();
}

bool
VITAValidator::
operator()(const std::vector<int>& sops, const CarrierCode* carrier)
{
  if (UNLIKELY(_interlineTicketCarrierData == nullptr))
  {
    // LOG4CXX_DEBUG(logger, " _interlineTicketCarrierData is NULL");
    return true;
  }

  bool resultFromCache = false;
  bool validatingCarrierFromCache = false;
  Itin* itin = nullptr;
  SopVITAMap::iterator vitaInfoIt = _sopVITAResultMap.find(sops);

  if (LIKELY(vitaInfoIt == _sopVITAResultMap.end()))
  {
    _trx.dataHandle().get(itin);

    for (uint32_t legNo = 0; legNo < sops.size(); ++legNo)
    {
      const int sopNo = sops[legNo];
      const ShoppingTrx::SchedulingOption& sop = _trx.legs()[legNo].sop()[sopNo];
      const std::vector<TravelSeg*>& curTvlSeg = sop.itin()->travelSeg();

      for (const auto tvSeg : curTvlSeg)
      {
        if (UNLIKELY(tvSeg == nullptr))
        {
          // LOG4CXX_DEBUG(logger, " _tvSeg is NULL");
          continue;
        }

        itin->travelSeg().push_back(tvSeg);
      }
    }

    FirstSopValidatingCarrierMap::iterator firstSopIt = _firstSopValidatingCarrierMap.find(sops[0]);

    CarrierCode validatingCarrier = BAD_CARRIER;
    if (_trx.isSimpleTrip() && (firstSopIt != _firstSopValidatingCarrierMap.end()))
    {
      validatingCarrier = firstSopIt->second;
      validatingCarrierFromCache = true;
    }
    else
    {
      TravelSegAnalysis tvlSegAnalysis;
      Boundary tvlBoundary = tvlSegAnalysis.selectTravelBoundary(itin->travelSeg());
      ItinUtil::setGeoTravelType(tvlSegAnalysis, tvlBoundary, *itin);
      _validatingCarrier.update(*itin);
      validatingCarrier = itin->validatingCarrier();

      if (_trx.isSimpleTrip())
      {
        _firstSopValidatingCarrierMap[sops[0]] = validatingCarrier;
      }
    }

    std::string validationMessage;
    const bool result = _interlineTicketCarrierData->validateInterlineTicketCarrierAgreement(
        _trx, validatingCarrier, itin->travelSeg(), &validationMessage);
    const VITAInfo vitaInfo(result, validatingCarrier, validationMessage);
    vitaInfoIt = _sopVITAResultMap.insert(std::make_pair(sops, vitaInfo)).first;
  }
  else // vitaInfoIt != _sopVITAResultMap.end()
  {
    resultFromCache = true;
  }

  TSE_ASSERT(vitaInfoIt != _sopVITAResultMap.end());

  if (UNLIKELY(showDiag910Vita()))
  {
    const VITAInfo& vitaInfo = vitaInfoIt->second;
    DiagManager diag(_trx, Diagnostic910);
    Diag910Collector* const diag910 = dynamic_cast<Diag910Collector*>(&diag.collector());
    TSE_ASSERT(diag910 != nullptr);
    (*diag910) << _interlineTicketCarrierData->getInterlineCarriersForValidatingCarrier(
                      _trx, vitaInfo.validatingCarrier());
    std::string queueType;

    const CarrierCode* carrierToPrint = (carrier ? carrier : _carrier);

    if (carrierToPrint)
    {
      queueType = *carrierToPrint;
    }
    else
    {
      queueType = "Interline";
    }

    (*diag910) << " (Queue " << queueType << ") ";

    if (resultFromCache)
    {
      diag910->printVITAData(_trx,
                             sops,
                             vitaInfo.validatingCarrier(),
                             vitaInfo.result(),
                             vitaInfo.validationMessage());
    }
    else
    {
      diag910->printVITAData(_trx,
                             sops,
                             itin->travelSeg(),
                             vitaInfo.validatingCarrier(),
                             vitaInfo.result(),
                             validatingCarrierFromCache,
                             vitaInfo.validationMessage());
    }

    diag910->flushMsg();
  }

  return vitaInfoIt->second.result();
}

void
VITAValidator::createInterlineTicketCarrier()
{
  if (_trx.getRequest()->processVITAData() && _trx.getOptions()->validateTicketingAgreement())
  {
    if (InterlineTicketCarrier::isPriceInterlineActivated(_trx))
    {
      _trx.dataHandle().get(_interlineTicketCarrierData);
    }
    else if (showDiag910Vita())
    {
      DiagManager diagnostic(_trx, Diagnostic910);
      diagnostic.collector() << "IET PRICING IS NOT ACTIVE\n";
      diagnostic.collector().flushMsg();
    }
  }
}

bool
VITAValidator::showDiag910Vita() const
{
  return _trx.diagnostic().diagnosticType() == Diagnostic910 &&
         _trx.diagnostic().diagParamMapItem("DD") == "VITA";
}
}
