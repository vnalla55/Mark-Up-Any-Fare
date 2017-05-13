#include "Pricing/Shopping/PQ/PUFullType.h"

#include "Common/Logger.h"
#include "DataModel/Trx.h"
#include "DBAccess/DataHandle.h"
#include "Pricing/FareMarketPath.h"
#include "Pricing/PUPath.h"
#include "Pricing/Shopping/PQ/ConxRoutePQItem.h"
#include "Pricing/Shopping/PQ/SoloPUPathCollector.h"
#include "Pricing/Shopping/PQ/SolutionPattern.h"

#include <boost/format.hpp>

#include <algorithm>

namespace tse
{
namespace shpq
{

namespace
{
struct NullDeleter
{
  void operator()(void*) {}
};

bool
addToPUPath(PU* pu, PUPath* puPath)
{
  if (pu && puPath)
  {
    puPath->puPath().push_back(pu);
    puPath->cxrFarePreferred() |= pu->cxrFarePreferred();

    return true;
  }
  return false;
}

const char*
subTypeToStr(PricingUnit::PUSubType& subtype)
{
  switch (subtype)
  {
  case PricingUnit::DEST_OPENJAW:
    return "TOJ";
  case PricingUnit::ORIG_OPENJAW:
    return "OOJ";
  case PricingUnit::DOUBLE_OPENJAW:
    return "DOW";
  default:
    return "";
  }
}

ConxRoutePQItem::CxrFMVector
getFMVector(const ConxRoutePQItem* const pqItem)
{
  ConxRoutePQItem::CxrFMVector fmVector;
  if (LIKELY(pqItem))
    fmVector = pqItem->getFMVector();

  return fmVector;
}
}

static Logger
logger("atseintl.ShoppingPQ.PUFullType");

PUTypeDetails::PUTypeDetails(const char* const idCStr,
                             PricingUnit::Type type,
                             PricingUnit::PUSubType subtype)
  : _idCStr(idCStr), _subtypeStr(subTypeToStr(subtype)), _type(type), _subtype(subtype)
{
}

void
PUFullType::setFMOrderStr(int fmIndex)
{
  _fmOrderStr = (boost::format("(%d)") % fmIndex).str();
}

void
PUFullType::setFMOrderStr(int obIndex, int ibIndex)
{
  _fmOrderStr = (boost::format("(%d-%d)") % obIndex % ibIndex).str();
}

PUFullTypePtr
OWPUType::create(uint16_t fmIndex)
{
  return PUFullTypePtr(new OWPUType(fmIndex));
}

bool
OWPUType::addPU(SoloPUPathCollector& puPathCollector, FareMarketPath* fmPath, PUPath* puPath) const
{
  if (LIKELY(fmPath && fmPath->fareMarketPath().size() > _fmIndex))
  {
    PU* pu = puPathCollector.getOrCreateOWPU(fmPath->fareMarketPath()[_fmIndex]);
    return addToPUPath(pu, puPath);
  }
  return false;
}

PUFullTypePtr
RTPUType::create(uint16_t fmIndex1, uint16_t fmIndex2, bool checkConnectingCities)
{
  return PUFullTypePtr(new RTPUType(fmIndex1, fmIndex2, checkConnectingCities));
}

bool
RTPUType::addPU(SoloPUPathCollector& puPathCollector, FareMarketPath* fmPath, PUPath* puPath) const
{
  if (LIKELY(fmPath && (fmPath->fareMarketPath().size() > std::max(_outboundIndex, _inboundIndex))))
  {
    PU* pu = puPathCollector.getOrCreateRTPU(fmPath->fareMarketPath()[_outboundIndex],
                                             fmPath->fareMarketPath()[_inboundIndex]);
    return addToPUPath(pu, puPath);
  }
  return false;
}

bool
RTPUType::isItemValid(const ConxRoutePQItem* const pqItem) const
{
  ConxRoutePQItem::CxrFMVector fmVector = getFMVector(pqItem);
  if (UNLIKELY(!fmVector.empty() && fmVector.size() <= std::max(_outboundIndex, _inboundIndex)))
    return false;

  CxrFareMarketsPtr obCxrFM = fmVector[_outboundIndex];
  CxrFareMarketsPtr ibCxrFM = fmVector[_inboundIndex];
  if (!_checkConnectingCities &&
      (*obCxrFM->begin())->getFareMarket()->getFmTypeSol() == FareMarket::SOL_FM_THRU &&
      (*ibCxrFM->begin())->getFareMarket()->getFmTypeSol() == FareMarket::SOL_FM_THRU)
    return true;

  return (obCxrFM->getOriginLocation() == ibCxrFM->getDestinationLocation() &&
          obCxrFM->getDestinationLocation() == ibCxrFM->getOriginLocation());
}

PUFullTypePtr
OJPUType::create(const PUTypeDetails& puTypeDetails, uint16_t outboundIndex, uint16_t inboundIndex)
{
  if (puTypeDetails.getType() != PricingUnit::Type::OPENJAW)
  {
    LOG4CXX_DEBUG(logger, "OJPUType::create() - cannot create OJPUType for different pricing unit");
    return PUFullTypePtr();
  }
  return PUFullTypePtr(new OJPUType(puTypeDetails, outboundIndex, inboundIndex));
}

bool
OJPUType::addPU(SoloPUPathCollector& puPathCollector, FareMarketPath* fmPath, PUPath* puPath) const
{
  if (fmPath && (fmPath->fareMarketPath().size() > std::max(_outboundIndex, _inboundIndex)))
  {
    PU* pu = puPathCollector.getOrCreateOJPU(fmPath->fareMarketPath()[_outboundIndex],
                                             fmPath->fareMarketPath()[_inboundIndex]);
    return addToPUPath(pu, puPath);
  }
  return false;
}

bool
OJPUType::isItemValid(const ConxRoutePQItem* const pqItem) const
{
  ConxRoutePQItem::CxrFMVector fmVector = getFMVector(pqItem);
  if (!fmVector.empty() && fmVector.size() <= std::max(_outboundIndex, _inboundIndex))
    return false;

  CxrFareMarketsPtr obCxrFM = fmVector[_outboundIndex];
  CxrFareMarketsPtr ibCxrFM = fmVector[_inboundIndex];
  switch (getSubType())
  {
  case PricingUnit::DEST_OPENJAW:
    return !((obCxrFM->getDestinationLocation()) == (ibCxrFM->getOriginLocation()));

  case PricingUnit::ORIG_OPENJAW:
    return !((obCxrFM->getOriginLocation()) == (ibCxrFM->getDestinationLocation()));

  case PricingUnit::DOUBLE_OPENJAW:
    return !((obCxrFM->getDestinationLocation()) == (ibCxrFM->getOriginLocation())) &&
           !((obCxrFM->getOriginLocation()) == (ibCxrFM->getDestinationLocation()));

  default:
    return false;
  };
}

PUFullTypePtr
CTPUType::create()
{
  return PUFullTypePtr(new CTPUType());
}

bool
CTPUType::addPU(SoloPUPathCollector& puPathCollector, FareMarketPath* fmPath, PUPath* puPath) const
{
  if (fmPath && puPath)
    return puPathCollector.createCTPUPath(*puPath, fmPath->fareMarketPath());
  return false;
}

bool
CTPUType::isItemValid(const ConxRoutePQItem* const pqItem) const
{
  switch (pqItem->getSolPattern()->getSPId())
  {
  case SolutionPattern::SP36:
  case SolutionPattern::SP37:
    return true;
  default:
  {
    // validation for diamond CT item

    ConxRoutePQItem::CxrFMVector fmVector = getFMVector(pqItem);
    if (fmVector.size() < 4)
      return false;

    return !(fmVector[0]->getDestinationLocation() == fmVector[2]->getDestinationLocation());
  }
  }
}
}
} // namespace tse::shpq
