//----------------------------------------------------------------------------
//  File:        AirlineShoppingFCUtil.h
//  Created:     2011-01-05
//
//  Description: Airline Shopping Fare Calc utility class
//
//  Updates:
//
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
//----------------------------------------------------------------------------

#include "FareCalc/AirlineShoppingFCUtil.h"

#include "Common/ShoppingUtil.h"
#include "Common/WnSnapUtil.h"
#include "DataModel/TaxResponse.h"

#include <set>
#include <utility>
#include <vector>

namespace tse
{
void
AirlineShoppingFCUtil::copyTaxResponsesAndSwapItinsVec(PricingTrx& trx)
{
  std::vector<Itin*> processedItinsOut;
  std::vector<Itin*> processedItinsIn;
  std::set<std::pair<Itin*, FarePath::FarePathKey> > processedFarePaths;

  std::map<Itin*, PricingTrx::SubItinValue>::iterator iter = trx.primeSubItinMap().begin();
  std::map<Itin*, PricingTrx::SubItinValue>::iterator iterEnd = trx.primeSubItinMap().end();

  for (; iter != iterEnd; ++iter)
  {
    Itin* itin = iter->first;

    Itin* outItin = iter->second.outboundItin;
    Itin* inItin = iter->second.inboundItin;

    std::vector<FarePath*>::iterator farePathIter = itin->farePath().begin();
    std::vector<FarePath*>::iterator farePathIterEnd = itin->farePath().end();

    for (; farePathIter != farePathIterEnd; ++farePathIter)
    {
      FarePath* farePath = (*farePathIter);

      FarePath::FarePathKey farePathKey = FarePath::buildKey(trx, farePath);

      if (false == farePathAlreadyProcessed(outItin, farePathKey, processedFarePaths))
      {
        updateProcessedItins(outItin, processedItinsOut);

        FarePath* outFarePath = ShoppingUtil::getFarePathForKey(trx, outItin, farePathKey);

        if (outFarePath != nullptr)
        {
          WnSnapUtil::copyTaxResponse(trx,
                                      itin,
                                      outItin,
                                      outFarePath,
                                      0,
                                      farePath->brandIndexPair().first,
                                      farePath->brandIndexPair().second,
                                      outItin,
                                      inItin);
        }
      }

      if (false == farePathAlreadyProcessed(inItin, farePathKey, processedFarePaths))
      {
        updateProcessedItins(inItin, processedItinsIn);

        FarePath* inFarePath = ShoppingUtil::getFarePathForKey(trx, inItin, farePathKey);

        if (inFarePath != nullptr)
        {
          WnSnapUtil::copyTaxResponse(trx,
                                      itin,
                                      inItin,
                                      inFarePath,
                                      1,
                                      farePath->brandIndexPair().first,
                                      farePath->brandIndexPair().second,
                                      outItin,
                                      inItin);
        }
      }
    }
  }

  trx.itin().clear();

  trx.itin().insert(trx.itin().end(), processedItinsOut.begin(), processedItinsOut.end());
  trx.itin().insert(trx.itin().end(), processedItinsIn.begin(), processedItinsIn.end());
}

namespace
{

class FarePathToRemove
{
public:
  bool operator()(FarePath* farePath) const { return (true == farePath->duplicate()); }
};

class TaxResponseToRemove
{
public:
  bool operator()(TaxResponse* taxResponse) const
  {
    return (true == taxResponse->farePath()->duplicate());
  }
};
}

void
AirlineShoppingFCUtil::updateTaxResponse(PricingTrx& trx, AbstractTaxSplitter& taxSplitter)
{
  std::set<std::pair<Itin*, FarePath::FarePathKey> > processedFarePaths;

  std::map<Itin*, PricingTrx::SubItinValue>::iterator iter = trx.primeSubItinMap().begin();
  std::map<Itin*, PricingTrx::SubItinValue>::iterator iterEnd = trx.primeSubItinMap().end();

  for (; iter != iterEnd; ++iter)
  {
    Itin* itin = iter->first;


    if (trx.getRequest()->originBasedRTPricing())
    {
      Itin* subItin = nullptr;
      bool const isFakeInbound = trx.outboundDepartureDate().isEmptyDate();
      bool const isFakeOutbound = trx.inboundDepartureDate().isEmptyDate();
      int legId = 0;

      if (isFakeInbound)
      {
        subItin = iter->second.outboundItin;
        legId = 0;
      }
      if (isFakeOutbound)
      {
        subItin = iter->second.inboundItin;
        legId = 1;
      }
      if (subItin)
      {
        std::vector<FarePath*>::iterator farePathIter = subItin->farePath().begin();
        std::vector<FarePath*>::iterator farePathIterEnd = subItin->farePath().end();

        for (; farePathIter != farePathIterEnd; ++farePathIter)
        {
          FarePath* farePath = (*farePathIter);

          FarePath::FarePathKey farePathKey = FarePath::buildKey(trx, farePath);

          if (false == baseItinContainsFarePath(trx, itin, farePathKey))
          {
            continue;
          }

          if (false == farePathAlreadyProcessed(subItin, farePathKey, processedFarePaths))
          {
            taxSplitter.clearTaxMaps();
            taxSplitter.setupTaxesForFarePath(itin, subItin, legId, farePath, farePathKey);
          }
        }
      }
    }
    else
    {
      Itin* outItin = iter->second.outboundItin;
      Itin* inItin = iter->second.inboundItin;

      if (outItin)
      {
        std::vector<FarePath*>::iterator farePathIter = outItin->farePath().begin();
        std::vector<FarePath*>::iterator farePathIterEnd = outItin->farePath().end();

        for (; farePathIter != farePathIterEnd; ++farePathIter)
        {
          FarePath* farePath = (*farePathIter);

          FarePath::FarePathKey farePathKey = FarePath::buildKey(trx, farePath);

          if (false == baseItinContainsFarePath(trx, itin, farePathKey))
          {
            continue;
          }

          if (false == farePathAlreadyProcessed(outItin, farePathKey, processedFarePaths))
          {
            taxSplitter.clearTaxMaps();
            taxSplitter.setupTaxesForFarePath(itin, outItin, 0, farePath, farePathKey);
          }
        }
      }

      if (inItin)
      {
        std::vector<FarePath*>::iterator farePathIter = inItin->farePath().begin();
        std::vector<FarePath*>::iterator farePathIterEnd = inItin->farePath().end();

        for (; farePathIter != farePathIterEnd; ++farePathIter)
        {
          FarePath* farePath = (*farePathIter);

          FarePath::FarePathKey farePathKey = FarePath::buildKey(trx, farePath);

          if (false == baseItinContainsFarePath(trx, itin, farePathKey))
          {
            continue;
          }

          if (false == farePathAlreadyProcessed(inItin, farePathKey, processedFarePaths))
          {
            taxSplitter.clearTaxMaps();
            taxSplitter.setupTaxesForFarePath(itin, inItin, 1, farePath, farePathKey);
          }
        }
      }
    }
  }

  std::vector<Itin*>::iterator itinIter = trx.itin().begin();
  std::vector<Itin*>::iterator itinIterEnd = trx.itin().end();

  for (; itinIter != itinIterEnd; ++itinIter)
  {
    Itin* itin = (*itinIter);

    itin->farePath().erase(
        std::remove_if(itin->farePath().begin(), itin->farePath().end(), FarePathToRemove()),
        itin->farePath().end());
    itin->mutableTaxResponses().erase(std::remove_if(itin->mutableTaxResponses().begin(),
                                                     itin->mutableTaxResponses().end(),
                                                     TaxResponseToRemove()),
                                      itin->mutableTaxResponses().end());
  }
}

void
AirlineShoppingFCUtil::updateProcessedItins(Itin* itin, std::vector<Itin*>& processedItins)
{
  if (std::find(processedItins.begin(), processedItins.end(), itin) == processedItins.end())
  {
    processedItins.push_back(itin);
  }
}

bool
AirlineShoppingFCUtil::farePathAlreadyProcessed(
    Itin* itin,
    FarePath::FarePathKey& farePathKey,
    std::set<std::pair<Itin*, FarePath::FarePathKey> >& processedFarePaths)
{
  std::pair<Itin*, FarePath::FarePathKey> itinFpkPair(itin, farePathKey);

  if (std::find(processedFarePaths.begin(), processedFarePaths.end(), itinFpkPair) !=
      processedFarePaths.end())
  {
    return true;
  }
  else
  {
    processedFarePaths.insert(itinFpkPair);
    return false;
  }
}

bool
AirlineShoppingFCUtil::baseItinContainsFarePath(PricingTrx& trx,
                                                Itin* itin,
                                                FarePath::FarePathKey& farePathKey)
{
  std::vector<FarePath*>::iterator farePathIter = itin->farePath().begin();
  std::vector<FarePath*>::iterator farePathIterEnd = itin->farePath().end();

  for (; farePathIter != farePathIterEnd; ++farePathIter)
  {
    FarePath* farePath = (*farePathIter);

    FarePath::FarePathKey procFarePathKey = FarePath::buildKey(trx, farePath);

    if (procFarePathKey == farePathKey)
    {
      return true;
    }
  }

  return false;
}
}
