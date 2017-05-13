// ----------------------------------------------------------------
//
//   Copyright Sabre 2011
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
#include "Pricing/Shopping/Diversity/DiversityModelPriceOnly.h"

#include "Common/Config/ConfigManUtils.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Diagnostic/DiagCollector.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"

namespace tse
{
namespace
{
ConfigurableValue<float>
altDateItinPriceJumpFactor("PRICING_SVC", "ALT_DATE_ITIN_PRICE_JUMP_FACTOR", 100000.0);
}
Logger
DiversityModelPriceOnly::_logger("atseintl.ShoppingPQ");

DiversityModelPriceOnly::DiversityModelPriceOnly(ShoppingTrx& trx,
                                                 ItinStatistic& stats,
                                                 DiagCollector* dc)
  : _stats(stats),
    _diversityParams(trx.diversity()),
    _dc(dc),
    _newFarePath(true),
    _diagPQ(false),
    _numberOfSolutionsRequired(trx.diversity().getNumberOfOptionsToGenerate()),
    _isFareCutOffUsed(_diversityParams.getFareCutoffCoefDefined()),
    _altDates(trx.isAltDates())
{
  initializeDiagnostic(trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL));
  calculateFareCutOffCoef();
  int32_t statEnabled = 0;
  if (_isFareCutOffUsed)
  {
    _diversityParams.setFareCutoffAmount(std::numeric_limits<MoneyAmount>::max());
    statEnabled = ItinStatistic::STAT_MIN_PRICE;
  }
  if (_diversityParams.getNumOfCustomSolutions())
    statEnabled |= ItinStatistic::STAT_CUSTOM_SOLUTION;
  _stats.setEnabledStatistics(statEnabled, this);
}

bool
DiversityModelPriceOnly::continueProcessing(MoneyAmount pqScore)
{
  bool result = false;
  bool isCutoffReached = false;

  // Check for fare amount cut-off hit
  if (_isFareCutOffUsed && pqScore > _diversityParams.getFareCutoffAmount())
    isCutoffReached = true;
  else
    result = (_stats.getTotalOptionsCount() < _numberOfSolutionsRequired);

  printContinueProcessing(result, isCutoffReached);
  return result;
}

DiversityModel::PQItemAction
DiversityModelPriceOnly::getPQItemAction(const shpq::SoloPQItem* pqItem)
{
  PQItemAction result = USE;
  if (!continueProcessing(pqItem->getScore()))
  {
    result = STOP;
  }

  printPQItemAction(result, pqItem);
  _newFarePath = false;
  return result;
}

DiversityModel::SOPCombinationList::iterator
DiversityModelPriceOnly::getDesiredSOPCombination(SOPCombinationList& combinations,
                                                  MoneyAmount score,
                                                  size_t fpKey)
{
  _newFarePath = true;

  if (_stats.getTotalOptionsCount() >= _numberOfSolutionsRequired)
    return combinations.end();

  return combinations.begin();
}

bool
DiversityModelPriceOnly::getIsNewCombinationDesired(const SOPCombination& combination,
                                                    MoneyAmount score)
{
  return true;
}

bool
DiversityModelPriceOnly::addSolution(const ShoppingTrx::FlightMatrix::value_type& solution,
                                     ShoppingTrx::FlightMatrix& flightMatrix,
                                     size_t,
                                     const DatePair* datePair)
{
  _stats.addSolution(*flightMatrix.insert(solution).first, datePair);

  // Got first solution, let's setup cut-off amount
  if (_isFareCutOffUsed && _stats.getTotalOptionsCount() == 1)
  {
    _diversityParams.setFareCutoffAmount(
        round(_diversityParams.getFareCutoffCoef() * _stats.getMinPrice() * 100.0) / 100.0);

    // flaky approach to remove subscription on statistic, PriceModel depends on
    _stats.setEnabledStatistics(_stats.getEnabledStatistics(this) & ~ItinStatistic::STAT_MIN_PRICE,
                                this);
    printParameters();
  }

  return true;
}

void
DiversityModelPriceOnly::initializeDiagnostic(const std::string& diagArg)
{
  if (!_dc)
    return;

  if (diagArg == "PQ" || diagArg == "ALL")
    _diagPQ = true;
}

void
DiversityModelPriceOnly::printContinueProcessing(bool result, bool isCutoffReached) const
{
  if (!_newFarePath || !_dc)
    return;

  *_dc << "DM Continue Processing: " << (result ? "TRUE" : "FALSE") << " ";
  if (!result)
  {
    if (isCutoffReached)
      *_dc << "Fare cut-off amount reached\n";
    else
      *_dc << "All requirements met\n";
  }
  *_dc << "\n\tSolutions generated: " << std::setw(3) << _stats.getTotalOptionsCount() << "/"
       << _numberOfSolutionsRequired << "\n";
}

void
DiversityModelPriceOnly::printPQItemAction(DiversityModel::PQItemAction result,
                                           const shpq::SoloPQItem* pqItem) const
{
  if (!_diagPQ)
    return;

  *_dc << "PQ Item Action: ";
  switch (result)
  {
  case DiversityModel::USE:
    *_dc << "USE ";
    break;
  case DiversityModel::STOP:
    *_dc << "STOP";
    break;
  default:
    break;
  }
  *_dc << " " << pqItem->str(shpq::SoloPQItem::SVL_HEADINGONLY);
  if (result == DiversityModel::STOP)
    *_dc << " Fare cut-off amount reached";
  *_dc << "\n";
}

void
DiversityModelPriceOnly::printParameters() const
{
  if (!_dc)
    return;

  *_dc << "Setting up diversity parameters:\n"
       << "\tFare cut-off amount coefficient: " << _diversityParams.getFareCutoffCoef()
       << " Fare cut-off amount: " << std::fixed << std::setprecision(2)
       << _diversityParams.getFareCutoffAmount() << std::resetiosflags(std::ios_base::floatfield)
       << std::setprecision(6) << "\n";
}

void
DiversityModelPriceOnly::calculateFareCutOffCoef()
{
  if (_altDates && _diversityParams.getFareCutoffCoef() < 1.0)
  {
    _diversityParams.setFareCutoffCoef(altDateItinPriceJumpFactor.getValue());
  }
}
}
