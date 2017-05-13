#include "FareCalc/PrepareFareCalcForShoppingTrx.h"

#include "Common/ShoppingUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/FareUsage.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/GroupFarePath.h"

namespace tse
{

PrepareFareCalcForShoppingTrx::PrepareFareCalcForShoppingTrx(ShoppingTrx& trx)
  : _trx(trx), _ruleController(FPRuleValidation)
{
  _ruleController.addCat(RuleConst::ADVANCE_RESERVATION_RULE);
  _ruleController.addCat(RuleConst::TICKET_ENDORSMENT_RULE);
}

PrepareFareCalcForShoppingTrx::~PrepareFareCalcForShoppingTrx() {}

void
PrepareFareCalcForShoppingTrx::process()
{

  uint16_t itinNumber = 0;

  _trx.getRequest()->lowFareRequested() = 'N';
  _trx.getRequest()->ticketingAgent()->currencyCodeAgent() = NUC;

  for (const ShoppingTrx::FlightMatrix::value_type& i : _trx.flightMatrix())
  {
    if (i.second)
      if (i.second->isShortCutPricing())
      {
        if (!i.second->groupFPPQItem().empty())
        {
          FarePath* farePath = i.second->groupFPPQItem().back()->farePath();
          Itin* itin = prepareItin(farePath, i.first, itinNumber++);

          _trx.itin().push_back(itin);
        }
      }
  }

  for (const ShoppingTrx::EstimateMatrix::value_type& i : _trx.estimateMatrix())
  {
    if (i.second.second)
      if (i.second.second->isShortCutPricing())
      {
        if (!i.second.second->groupFPPQItem().empty())
        {
          FarePath* farePath = i.second.second->groupFPPQItem().back()->farePath();
          Itin* itin = prepareItin(farePath, i.first, itinNumber++);

          _trx.itin().push_back(itin);
        }
      }
  }
}

Itin*
PrepareFareCalcForShoppingTrx::prepareItin(FarePath* farePath,
                                           const SopIdVec& sops,
                                           const uint16_t& numberOfitin)
{
  Itin* itin = farePath->itin();
  itin->itinNum() = numberOfitin;
  itin->farePath().clear();
  itin->farePath().push_back(farePath);

  itin->setTravelDate(_trx.journeyItin()->travelDate());
  itin->calculationCurrency() = farePath->calculationCurrency();

  for (PricingUnit* pIter : itin->farePath().back()->pricingUnit())
  {
    _ruleController.validate(_trx, *itin->farePath().back(), *pIter);
    for (FareUsage* fuIter : pIter->fareUsage())
    {
      for (FareMarket* fm : itin->fareMarket())
      {
        fm->setGlobalDirection(fuIter->paxTypeFare()->globalDirection());
        itin->fareMarket().back()->setGlobalDirection(fuIter->paxTypeFare()->globalDirection());
      }
    }
  }
  return itin;
}
}
