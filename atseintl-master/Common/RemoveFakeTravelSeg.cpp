#include "Common/RemoveFakeTravelSeg.h"

#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"

namespace tse
{

void
RemoveFakeTravelSeg::process(PricingTrx& trx)
{
  std::vector<Itin*>::iterator itItin = trx.itin().begin();
  const std::vector<Itin*>::iterator itItinEnd = trx.itin().end();
  const bool fakeOutbound = !trx.outboundDepartureDate().isEmptyDate();
  const TravelSeg* fakeTS = fakeOutbound ? trx.itin().front()->travelSeg().front()
                                         : trx.itin().front()->travelSeg().back();

  for (; itItin != itItinEnd; ++itItin)
  {
    Itin* itin = *itItin;

    if (fakeOutbound)
      itin->travelSeg().erase(itin->travelSeg().begin());
    else
    {
      itin->travelSeg().erase(itin->travelSeg().end() - 1);
      itin->travelSeg().back()->stopOver() = false;
    }

    std::vector<FarePath*>::const_iterator itFP = itin->farePath().begin();
    std::vector<FarePath*>::const_iterator itFPEnd = itin->farePath().end();

    for (; itFP != itFPEnd; ++itFP)
    {
      FarePath* fp = *itFP;
      removeFakePU(fp, fakeTS);
      removeFakeFU(fp);
    }
  }
}

void
RemoveFakeTravelSeg::removeFakePU(FarePath* fp, const TravelSeg* fakeTS)
{
  std::vector<PricingUnit*>::iterator itPU = findFakePU(fp->pricingUnit());

  if (itPU != fp->pricingUnit().end())
    fp->pricingUnit().erase(itPU);

  itPU = fp->pricingUnit().begin();
  const std::vector<PricingUnit*>::iterator itPUend = fp->pricingUnit().end();

  for (; itPU != itPUend; ++itPU)
  {
    std::vector<TravelSeg*>::iterator tsIter =
        std::find((*itPU)->travelSeg().begin(), (*itPU)->travelSeg().end(), fakeTS);

    if (tsIter != (*itPU)->travelSeg().end())
      (*itPU)->travelSeg().erase(tsIter);
  }
}

std::vector<PricingUnit*>::iterator
RemoveFakeTravelSeg::findFakePU(std::vector<PricingUnit*>& pu)
{
  std::vector<PricingUnit*>::iterator itPU = pu.begin();
  std::vector<PricingUnit*>::iterator itPUEnd = pu.end();

  for (; itPU != itPUEnd; ++itPU)
  {
    PricingUnit* pu = *itPU;

    // OW PU based on single dummy fare
    if (pu->puType() == PricingUnit::Type::ONEWAY && pu->fareUsage().size() == 1 &&
        pu->fareUsage()[0]->paxTypeFare()->fareMarket()->useDummyFare())
    {
      break;
    }
  }

  return itPU;
}

void
RemoveFakeTravelSeg::removeFakeFU(FarePath* fp)
{
  std::vector<PricingUnit*>::const_iterator itPU = fp->pricingUnit().begin();
  std::vector<PricingUnit*>::const_iterator itPUEnd = fp->pricingUnit().end();

  for (; itPU != itPUEnd; ++itPU)
  {
    PricingUnit* pu = *itPU;
    std::vector<FareUsage*>::iterator itFU = findFakeFU(pu->fareUsage());

    if (itFU != pu->fareUsage().end())
    {
      const FareUsage* fu = *itFU;
      fp->decreaseTotalNUCAmount(fu->surchargeAmt());
      pu->fareUsage().erase(itFU);
      break; // there is only single dummy FU per FP
    }
  }
}

std::vector<FareUsage*>::iterator
RemoveFakeTravelSeg::findFakeFU(std::vector<FareUsage*>& fu)
{
  std::vector<FareUsage*>::iterator itFU = fu.begin();
  std::vector<FareUsage*>::iterator itFUEnd = fu.end();

  for (; itFU != itFUEnd; ++itFU)
  {
    const FareUsage* fu = *itFU;

    if (fu->paxTypeFare()->fareMarket()->useDummyFare())
      break;
  }

  return itFU;
}

std::vector<FareUsage*>::iterator
RemoveFakeTravelSeg::findFirstNonFakeFU(std::vector<FareUsage*>& fu)
{
  std::vector<FareUsage*>::iterator itFU = fu.begin();
  std::vector<FareUsage*>::iterator itFUEnd = fu.end();

  for (; itFU != itFUEnd; ++itFU)
  {
    const FareUsage* fu = *itFU;

    if (fu->paxTypeFare()->fareMarket()->useDummyFare() == false)
      return itFU;
  }

  return itFUEnd;
}
}
