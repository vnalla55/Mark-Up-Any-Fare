#include "FareCalc/FormattedFareCalcLine.h"

#include "Common/Assert.h"
#include "Common/TrxUtil.h"
#include "DataModel/FarePath.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/FareCalcCollector.h"
#include "FareCalc/FcDispFarePath.h"
#include "Rules/NegotiatedFareRuleUtil.h"
#include "Rules/RuleConst.h"
#include "Common/FallbackUtil.h"

namespace tse
{

void
FormattedFareCalcLine::compute(std::string& val) const
{
  TSE_ASSERT(_trx != nullptr);
  TSE_ASSERT(_farePath != nullptr);
  TSE_ASSERT(_fcConfig != nullptr);
  TSE_ASSERT(_fcCollector != nullptr);

  if (LIKELY(_calcTotals != nullptr))
  {
    FcDispFarePath fcLine(*_trx, *_farePath, *_fcConfig, *_fcCollector, _calcTotals);
    val = fcLine.toString();
    _calcTotals->fareCalculationLine = fcLine.fareCalculationLine();

    if (_calcTotals->netRemitCalcTotals != nullptr)
    {
      FcDispFarePath netRemitFcLine(*_trx,
                                    *_calcTotals->netRemitCalcTotals->farePath,
                                    *_fcConfig,
                                    *_fcCollector,
                                    _calcTotals->netRemitCalcTotals);

      std::string netRemitFcl = netRemitFcLine.toString();
      _calcTotals->netRemitCalcTotals->fareCalculationLine = netRemitFcLine.fareCalculationLine();
    }
    else if (TrxUtil::isCat35TFSFEnabled(*_trx) && // Cat35 tfsf
             nullptr != _calcTotals->netCalcTotals)
    {
      FcDispFarePath netFcLine(*_trx,
                               *_calcTotals->netCalcTotals->farePath,
                               *_fcConfig,
                               *_fcCollector,
                               _calcTotals->netCalcTotals);

      std::string netFcl = netFcLine.toString();
      _calcTotals->netCalcTotals->fareCalculationLine = netFcLine.fareCalculationLine();
    }
    else
    {
      if ((_fcConfig->fareBasisDisplayOption() == FareCalcConsts::FC_YES ||
           TrxUtil::getValidatingCxrFbcDisplayPref(*_trx, *_farePath)) &&
          _farePath->pricingUnit().size() && _farePath->pricingUnit()[0]->fareUsage().size())
      {
        FareBreakPointInfo& fbpInfo =
            _calcTotals->getFareBreakPointInfo(_farePath->pricingUnit()[0]->fareUsage()[0]);

        Indicator tktFareDataInd = NegotiatedFareRuleUtil::getFareAmountIndicator(
            *_trx, _farePath->pricingUnit()[0]->fareUsage()[0]);

        if ((!fbpInfo.netPubFbc.empty() ||
             NegotiatedFareRuleUtil::isTFDPSC(_farePath->pricingUnit()[0]->fareUsage())) &&
            tktFareDataInd == RuleConst::NR_VALUE_F)
        {
          FcDispFarePath fcLine(*_trx, *_farePath, *_fcConfig, *_fcCollector, _calcTotals, true);

          fcLine.toString();
          _calcTotals->netRemitFareCalcLine = fcLine.fareCalculationLine();
        }
      }
    }

    if (nullptr != _calcTotals->adjustedCalcTotal)
    {
      FcDispFarePath netFcLine(*_trx,
                               *_calcTotals->adjustedCalcTotal->farePath,
                               *_fcConfig,
                               *_fcCollector,
                               _calcTotals->adjustedCalcTotal);

      std::string netFcl = netFcLine.toString();
      _calcTotals->adjustedCalcTotal->fareCalculationLine = netFcLine.fareCalculationLine();
    }

  }
}
}
