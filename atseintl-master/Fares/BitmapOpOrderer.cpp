//----------------------------------------------------------------------------
//  File:        BitmapOpOrderer.cpp
//  Created:     2005-01-01
//
//  Description: Booking code, routing and flight related validation
//
//  Updates:
//
//  Copyright Sabre 2008
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

#include "Fares/BitmapOpOrderer.h"

#include "BookingCode/FareBookingCodeValidator.h"
#include "Common/ClassOfService.h"
#include "Common/ConfigList.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/Logger.h"
#include "Common/MetricsUtil.h"
#include "Common/RoutingUtil.h"
#include "Common/TSELatencyData.h"
#include "DataModel/PaxTypeFare.h"
#include "Fares/FareValidatorOrchestrator.h"
#include "Rules/RuleConst.h"

namespace tse
{
namespace
{
static const char BITMAP_OP_GLOBDIR = 'G';
static const char BITMAP_OP_BITMAPV = 'B';
static const char BITMAP_OP_QUALCAT4 = 'Q';
static const char BITMAP_OP_ROUTING = 'R';
static const char BITMAP_OP_BKGCODE = 'K';

ConfigurableValue<ConfigVector<char>>
bitmapOperationOrder("SHOPPING_OPT", "BITMAP_OPERATION_ORDER", std::string("G|B|Q|R|K"));

Logger
logger("atseintl.Fares.BitmapOpOrderer");

inline GlobalDirection
getGlobalDirection(ShoppingTrx& trx, PaxTypeFare& curFare, ItinIndex::ItinCellInfo& curItinCellInfo)
{
  GlobalDirection globalDirection = curItinCellInfo.globalDirection();

  if (trx.isSumOfLocalsProcessingEnabled() &&
      curFare.fareMarket()->getFmTypeSol() == FareMarket::SOL_FM_LOCAL)
  {
    FareMarket* fm = curFare.fareMarket();
    GlobalDirectionFinderV2Adapter::getGlobalDirection(
        &trx, fm->travelDate(), fm->travelSeg(), globalDirection);
  }

  return globalDirection;
}
} // namespace

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
BitmapOpOrderer::BitmapOpOrderer(const tse::ConfigMan& config)
{
  // Get vector of operations -- start with a default value
  // which will be overridden if it's found in the config
  const auto& vec = bitmapOperationOrder.getValue();
  _operationOrder.assign(vec.begin(), vec.end());
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void
BitmapOpOrderer::performBitmapOperations(ShoppingTrx& trx,
                                         PaxTypeFare& curFare,
                                         FareMarket& fM,
                                         const uint32_t& bitIndex,
                                         Itin& journeyItin,
                                         ItinIndex::ItinCellInfo& curItinCellInfo,
                                         const bool bookingCodeBeforeRule,
                                         FareMarketRuleController& ruleController,
                                         FareMarketRuleController& cat4RuleController,
                                         ShoppingRtgMap& rtMap,
                                         std::vector<BCETuning>& bceTuningData,
                                         bool bkcProcessed,
                                         bool rtgProcessed,
                                         bool glbProcessed)
{
  bool didBookingCodeAlready = false;

  for (const char curOp : _operationOrder)
  {
    // Skip other operations for fares keep for routing validation only
    if (UNLIKELY((curOp != BITMAP_OP_ROUTING) && curFare.isKeepForRoutingValidation()))
    {
      continue;
    }

    //--- Global direction operation
    if (curOp == BITMAP_OP_GLOBDIR)
    {
      TSELatencyData metrics(trx, "BIT OPS GLOBALDIR");

      if (!glbProcessed &&
          !doGlobalDirection(curFare, bitIndex, getGlobalDirection(trx, curFare, curItinCellInfo)))
      {
        break;
      }
      continue;
    }

    //--- Bitmap validation operation
    if (curOp == BITMAP_OP_BITMAPV)
    {
      TSELatencyData metricsO(trx, "BITOPORDERER RULES");

      if (bookingCodeBeforeRule)
      {
        TSELatencyData metrics(trx, "BITOPORDERER BKGCODE");
        didBookingCodeAlready = true;

        if (!bkcProcessed &&
            !doBookingCodeValidation(trx, curFare, fM, bitIndex, journeyItin, bceTuningData))
        {
          break;
        }
      }

      if (!doBitmapCatValidations(
              trx, curFare, fM, ruleController, bitIndex, journeyItin, curItinCellInfo))
      {
        break;
      }
      continue;
    }

    //--- Qualifying category four operation
    if (curOp == BITMAP_OP_QUALCAT4)
    {
      TSELatencyData metrics(trx, "BITOPORDERER QUAL4");

      if (!doQualCat4Validations(trx, curFare, cat4RuleController, bitIndex, journeyItin))
      {
        break;
      }
      continue;
    }

    //--- Routing operation
    if (curOp == BITMAP_OP_ROUTING)
    {
      TSELatencyData metrics(trx, "BITOPORDERER ROUTING");

      if (!rtgProcessed && !doRouting(trx, curFare, fM, curItinCellInfo, bitIndex, rtMap))
      {
        break;
      }
      continue;
    }

    if (LIKELY(curOp == BITMAP_OP_BKGCODE))
    {
      if (!didBookingCodeAlready)
      {
        TSELatencyData metrics(trx, "BITOPORDERER BKGCODE");
        didBookingCodeAlready = true;

        if (!bkcProcessed &&
            !doBookingCodeValidation(trx, curFare, fM, bitIndex, journeyItin, bceTuningData))
        {
          break;
        }
        continue;
      }
    }
  }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool
BitmapOpOrderer::doGlobalDirection(PaxTypeFare& curFare,
                                   const uint32_t& bitIndex,
                                   const GlobalDirection globalDirection)
{
  // if the "travel global direction" and the flight
  // global direction are not
  // equal, then the bit is invalid. NOTE that if the
  // global direction is ZZ, then there is no global
  // direction and we don't validate.
  if ((curFare.fare()->globalDirection() != GlobalDirection::ZZ) &&
      (curFare.fare()->globalDirection() != globalDirection))
  {
    curFare.setFlightInvalid(bitIndex, RuleConst::GLOBALDIR_FAIL);
    return false;
  }
  return true;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool
BitmapOpOrderer::doBookingCodeValidation(ShoppingTrx& trx,
                                         PaxTypeFare& curFare,
                                         FareMarket& fM,
                                         const uint32_t& bitIndex,
                                         Itin& journeyItin,
                                         std::vector<BCETuning>& bceTuningData)
{
  FareBookingCodeValidator fbcv(trx, fM, &journeyItin);
  fbcv.validate(bitIndex, &curFare, bceTuningData);
  return curFare.isFlightValid(bitIndex);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool
BitmapOpOrderer::doBitmapCatValidations(PricingTrx& pTrx,
                                        PaxTypeFare& curFare,
                                        FareMarket& fM,
                                        FareMarketRuleController& ruleController,
                                        const uint32_t& bitIndex,
                                        Itin& journeyItin,
                                        ItinIndex::ItinCellInfo& curItinCellInfo)
{
  FareValidatorOrchestrator::performBitmapCatValidations(
      &curFare, &fM, ruleController, bitIndex, pTrx, &journeyItin, curItinCellInfo);
  return curFare.isFlightValid(bitIndex);
}

bool
BitmapOpOrderer::doQualCat4Validations(PricingTrx& pTrx,
                                       PaxTypeFare& curFare,
                                       FareMarketRuleController& cat4RuleController,
                                       const uint32_t& bitIndex,
                                       Itin& journeyItin)
{
  FareValidatorOrchestrator::performQualCat4Validations(
      &curFare, cat4RuleController, bitIndex, pTrx, &journeyItin);
  return curFare.isFlightValid(bitIndex);
}

bool
BitmapOpOrderer::doRouting(ShoppingTrx& trx,
                           PaxTypeFare& curFare,
                           FareMarket& fM,
                           ItinIndex::ItinCellInfo& curItinCellInfo,
                           const uint32_t& bitIndex,
                           ShoppingRtgMap& rtMap)
{
  fM.setGlobalDirection(getGlobalDirection(trx, curFare, curItinCellInfo));
  PaxTypeFare* ptf = &curFare;

  if (UNLIKELY(PricingTrx::FF_TRX == trx.getTrxType()))
  {
    FlightFinderTrx* ffTrx = static_cast<FlightFinderTrx*>(&trx);

    if (ffTrx->avlInS1S3Request() && curFare.isFareByRule() &&
        RoutingUtil::isSpecialRouting(curFare))
    {
      ptf = curFare.getBaseFare();
    }
    if (!ptf)
      ptf = &curFare;
  }

  if (UNLIKELY(rtMap.empty() && RoutingUtil::isSpecialRouting(*ptf, false)))
  {
    doPublishedRouting(trx, fM, bitIndex, rtMap);
  }

  RoutingController shoppingRoutingController(trx);
  shoppingRoutingController.process(trx, fM, bitIndex, *ptf, rtMap);

  if (UNLIKELY(ptf->isKeepForRoutingValidation()))
  {
    curFare.setFlightInvalid(bitIndex, RuleConst::SKIP);
  }

  return curFare.isFlightValid(bitIndex);
}

void
BitmapOpOrderer::doPublishedRouting(ShoppingTrx& trx,
                                    FareMarket& fM,
                                    const uint32_t& bitIndex,
                                    ShoppingRtgMap& rtMap)
{
  RoutingController shoppingRoutingController(trx);
  std::vector<PaxTypeFare*>::iterator pTFIter = fM.allPaxTypeFare().begin();
  std::vector<PaxTypeFare*>::iterator pTFEndIter = fM.allPaxTypeFare().end();

  for (; pTFIter != pTFEndIter; ++pTFIter)
  {
    if (*pTFIter == nullptr)
      continue;

    PaxTypeFare& curFare = **pTFIter;

    if (curFare.isFlightBitmapInvalid())
      continue;

    if (RoutingUtil::isSpecialRouting(curFare, false))
      continue;

    curFare.setKeepForRoutingValidation(true);

    shoppingRoutingController.process(trx, fM, bitIndex, curFare, rtMap);

    curFare.setKeepForRoutingValidation(false);
    return;
  }
}

void
BitmapOpOrderer::doPublishedRoutingDelayed(ShoppingTrx& trx,
                                           RoutingController& shoppingRoutingController,
                                           FareMarket& fM,
                                           PaxTypeFare& curFare,
                                           ShoppingRtgMap& rtMap)
{
  shoppingRoutingController.processShoppingNonSpecialRouting(fM, curFare, rtMap);
}
}
