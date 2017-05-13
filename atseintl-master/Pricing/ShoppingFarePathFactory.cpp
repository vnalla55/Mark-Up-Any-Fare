#include "Pricing/ShoppingFarePathFactory.h"

#include "Common/Global.h"
#include "Common/ShoppingAltDateUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TSELatencyData.h"
#include "Fares/BitmapOpOrderer.h"
#include "Fares/FareByRuleValidator.h"
#include "Pricing/FarePathValidator.h"
#include "Pricing/PricingOrchestrator.h"
#include "Pricing/ShoppingPQ.h"
#include "Rules/PricingUnitRuleController.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "Server/TseServer.h"

#include <cstdlib>
#include <vector>

namespace tse
{
FarePathFactory*
ShoppingFarePathFactoryCreator::create(const FactoriesConfig& factoriesConfig,
                                       const PricingTrx& trx) const
{
  ShoppingFarePathFactory* res =
      &trx.dataHandle().safe_create<ShoppingFarePathFactory>(factoriesConfig);
  res->setShoppingTrx(&_trx);
  res->setShoppingPQ(&_pq);
  res->setBitmapOpOrderer(&_op);
  return res;
}

bool
ShoppingFarePathFactory::isFarePathValid(FPPQItem& fppqItem,
    DiagCollector& diag,
    std::list<FPPQItem*>& fppqItems)
{
  // TODO use validator instead

  const TSELatencyData metrics(*_trx, "SHOPPING FPF VALID");
  FarePath& fpath = *fppqItem.farePath();
  const char* result = "PASS";
  const char* resultFVO = "";
  int fareIndex = 1;
  bool resFVO = callbackFVO(fpath, diag, resultFVO, fareIndex);

  if (UNLIKELY(!resFVO))
  {
    result = "CALLBACK FVO - FAIL";
    _pq->farePathValidationResult(fpath, result, resultFVO, fareIndex);
    return resFVO;
  }

  FarePathValidator validator(*_paxFPFBaseData);
  validator.setSettings(FarePathSettings{
      &_allPUF, &_failedPricingUnits, &_eoeFailedFare, _yqyrPreCalc, _externalLowerBoundAmount});
  validator.setEoeCombinabilityEnabled(_eoeCombinabilityEnabled);
  validator.setPuPath(_puPath);
  validator.setItin(_itin);

  fareIndex = 0;
  bool res =
      (result = "FAIL BRAND PARITY") &&
      (!(_trx->getRequest()->isParityBrandsPath()) || hasBrandParity(fpath, diag)) &&
      (result = "FAIL ALT DATES") && checkAltDates(fpath, diag) && (result = "FAIL PU BUSRULE") &&
      validator.checkFinalPUBusinessRule(fpath, diag) && (result = "FAIL COMBINABILITY") &&
      validator.checkCombinability(fppqItem, diag) && (result = "FAIL CHECK FBR TARIFF") &&
      (isEoeCombinabilityEnabled() ? FareByRuleValidator::checkSameTariffRule(fpath) : true) &&
      (result = "FAIL CHECK CXR PREF") && checkCarrierPreference(fpath, diag);

  farepathutils::copyReusablePUToMutablePU(_storage, fppqItem);

  res = res && (result = "FAIL PRE VALIDATE") && _pq->preValidateFarePath(fpath) &&        
        (result = "PUSH BACK");

  if (res)
  {
    res = validator.penaltyValidation(fpath);

    if(!res)
    {
      result = validator.getResults().c_str();
    }
  }

  _pq->farePathValidationResult(fpath, result, resultFVO, fareIndex);
  return res;
}

bool
ShoppingFarePathFactory::callbackFVO(FarePath& fpath,
                                     DiagCollector& diag,
                                     const char*& resultFVO,
                                     int& fareIndex)
{
  if (!_trx->isAltDates())
    return true;

  ShoppingTrx& shoppingTrx = dynamic_cast<ShoppingTrx&>(*_trx);
  std::vector<PricingUnit*>::iterator puIt = fpath.pricingUnit().begin();

  for (; puIt != fpath.pricingUnit().end(); ++puIt)
  {
    std::vector<FareUsage*>::iterator fuIt = (**puIt).fareUsage().begin();

    for (; fuIt != (**puIt).fareUsage().end(); ++fuIt)
    {
      PaxTypeFare& ptf = *(*fuIt)->paxTypeFare();
      FareMarket*& fM = ptf.fareMarket();

      if (!ptf.isFltIndependentValidationFVO())
      {
        PricingOrchestrator* po = _pq->ppo();
        tse::TseServer& server = po->server();
        FareValidatorOrchestrator* fvo = (FareValidatorOrchestrator*)server.service("FARESV_SVC");
        fvo->callback(shoppingTrx, fM, &ptf);
        resultFVO = "CALLBACK FVO";

        if (!ptf.isValid() || ptf.flightBitmap().empty())
        {
          return false;
        }
      }

      ++fareIndex;
    }
  }

  return true;
}

bool
ShoppingFarePathFactory::checkAltDates(FarePath& fpath, DiagCollector& diag)
{
  if (!_trx->isAltDates())
  {
    return true;
  }

  const TSELatencyData metrics(*_trx, "CHECK ALT DATES");
  ShoppingTrx::AltDatePairs::const_iterator aIt = _trx->altDatePairs().begin();
  ShoppingTrx::AltDatePairs::const_iterator aItEnd = _trx->altDatePairs().end();

  for (; aIt != aItEnd; ++aIt)
  {
    if (LIKELY(aIt->second->numOfSolutionNeeded > 0))
    {
      bool commonValidDate = true;

      for (const auto pu : fpath.pricingUnit())
      {
        for (const auto fu : pu->fareUsage())
        {
          PaxTypeFare& ptf = *fu->paxTypeFare();

          if (!ptf.getAltDatePass(aIt->first))
          {
            commonValidDate = false;
            break;
          }
        }

        if (!commonValidDate)
        {
          break;
        }
      }

      if (commonValidDate)
      {
        return true;
      }
    }
  }

  return false;
}

} // end namespace tse
