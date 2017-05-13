/*---------------------------------------------------------------------------
 *  Copyright Sabre 2015
 *    The copyright to the computer program(s) herein
 *    is the property of Sabre.
 *    The program(s) may be used and/or copied only with
 *    the written permission of Sabre or in accordance
 *    with the terms and conditions stipulated in the
 *    agreement/contract under which the program(s)
 *    have been supplied.
 *-------------------------------------------------------------------------*/
#include "Pricing/SimilarItin/DiagnosticWrapper.h"

#include "DataModel/Itin.h"
#include "DataModel/TravelSeg.h"
#include "Pricing/FPPQItem.h"
#include "Pricing/GroupFarePath.h"
#include "Pricing/PaxFarePathFactoryBase.h"

#include <map>
#include <vector>

namespace tse
{
typedef std::map<TravelSeg*, unsigned int> TvlSegToItinIdxMap;

namespace similaritin
{
DiagnosticWrapper::DiagnosticWrapper(PricingTrx& trx, DiagCollector& diag) : _trx(trx), _diag(diag)
{
  if (_trx.diagnostic().diagnosticType() == Diagnostic990)
    _diag990 = dynamic_cast<Diag990Collector*>(&_diag);
}

void
DiagnosticWrapper::itinNotApplicableRule(Itin& itin, const uint16_t rule)
{
  if (!_diag990)
    return;
  Diag990Collector::Diag990NotAppReason reason(
      Diag990Collector::Diag990NotAppReason::DefaultReason);
  switch (rule)
  {
  case RuleConst::FLIGHT_APPLICATION_RULE:
    reason = Diag990Collector::Diag990NotAppReason::FlightApplicationRule;
    break;
  case RuleConst::STOPOVER_RULE:
    reason = Diag990Collector::Diag990NotAppReason::StopoverRule;
    break;
  default:
    break;
  }
  _diag990->addItinNotApplicable(&itin, reason);
}

void
DiagnosticWrapper::itinNotApplicable(Itin& itin, const Diag990Collector::Diag990NotAppReason reason)
{
  if (!_diag990)
    return;
  _diag990->addItinNotApplicable(&itin, reason);
}

void
DiagnosticWrapper::printNotApplicable()
{
  if (!_diag990)
    return;
  _diag990->printItinNotApplicable();
  _diag990->clearItinNotApplicable();
}

void
DiagnosticWrapper::itinPopulated(Itin& itin, const std::vector<FarePath*>& farePathVec)
{
  if (!_diag990)
    return;
  *_diag990 << "POPULATED SIMILAR ITIN " << itin.itinNum() << ":\n";
  *_diag990 << itin;

  _diag990->printBookingClassChildItinNew(&itin);
  _diag990->printSimilarItinFarePaths(farePathVec);
}

void
DiagnosticWrapper::printAttempt(const uint16_t attemptNumber, const GroupFarePath& gfp)
{
  if (!_diag990)
    return;
  static_cast<std::ostringstream&>(*_diag990) << attemptNumber << " NEXT GFP:" << std::endl;
  *_diag990 << gfp.groupFPPQItem();
}

void
DiagnosticWrapper::printFamily(const Itin& motherItin, const std::vector<FPPQItem*>& groupFPath)
{
  if (!_diag990)
    return;
  _diag990->printFamily(_trx, groupFPath, motherItin);
}

void
DiagnosticWrapper::printSurchargesNotApplied()
{
  if (!_diag990)
    return;
  *_diag990 << "FAILED: APPLY SURCHARGES. USE MOTHER SURCHARGES.\n";
}

void
DiagnosticWrapper::clearChildBookingClass(const Itin* itin)
{
  if (!_diag990)
    return;
  _diag990->clearChildBookingClassNew(itin);
}

void
DiagnosticWrapper::updateChildBookingClass(const Itin* child,
                                           const std::vector<std::vector<ClassOfService*>*>& cosVec)
{
  if (!_diag990)
    return;
  _diag990->updateChildBookingClassNew(child, cosVec);
}

void
DiagnosticWrapper::processingSavedFPPQItems(const IntIndex& itinNum)
{
  if (_diag.diagnosticType() == Diagnostic991)
  {
    _diag << " PROCESSING SAVED FPPQITEMS FOR MOTHER ITIN NO " << itinNum << std::endl;
    _diag.printLine();
  }
}

void
DiagnosticWrapper::processingSimilarItinNo(const IntIndex& itinNum)
{
  if (_diag.diagnosticType() == Diagnostic991)
  {
    _diag << " PROCESSING SIMILAR ITIN NO " << itinNum << std::endl;
    _diag.printLine();
  }
}

void
DiagnosticWrapper::savedItems(const PaxType& paxType,
                              const std::vector<SavedFPPQItem>& items)
{
  if (_diag.diagnosticType() == Diagnostic991)
  {
    _diag << " PAXTYPE: " << paxType.paxType() << std::endl;
    _diag << " SAVED ITEMS COUNT " << items.size() << std::endl;
    listItems(items);
    _diag.printLine();
    _diag.lineSkip(1);
  }
}

void
DiagnosticWrapper::processingNewFPPQItem(const FPPQItem& fppqItem, const uint16_t itemIndex)
{
  if (_diag.diagnosticType() == Diagnostic991)
  {
    _diag.printLine();
    _diag << " PROCESSING NEW FPPQITEM: " << std::to_string(itemIndex) << std::endl;
    _diag << " PAXTYPE: " << fppqItem.farePath()->paxType()->paxType() << std::endl;
    _diag << " FARE PATH:" << std::endl;
    _diag << *fppqItem.farePath();
  }
}
void
DiagnosticWrapper::savedFPPQItemFailed()
{
  if (_diag.diagnosticType() == Diagnostic991)
  {
    _diag << "  VALIDATION FAILED";
    _diag.lineSkip(2);
  }
}

void
DiagnosticWrapper::savedFPPQItemPassed()
{
  if (_diag.diagnosticType() == Diagnostic991)
  {
    _diag << "  VALIDATION PASSED";
    _diag.lineSkip(2);
  }
}

void
DiagnosticWrapper::returningItem(const uint16_t itemIndex)
{
  if (_diag.diagnosticType() == Diagnostic991)
  {
    _diag << "   RETURNING ITEM NO " << itemIndex << std::endl;
    _diag.lineSkip(1);
  }
}

void
DiagnosticWrapper::pushingBack(const uint16_t itemIndex, const MoneyAmount amount)
{
  if (_diag.diagnosticType() == Diagnostic991)
  {
    _diag << "   PUSHING BACK ITEM NO " << itemIndex << " WITH AMOUNT " << amount << std::endl;
    _diag.lineSkip(1);
  }
}

void
DiagnosticWrapper::gfpPassed()
{
  if (_diag.diagnosticType() == Diagnostic991)
  {
    _diag << "  GROUP FARE PATH VALIDATION PASSED";
    _diag.lineSkip(2);
  }
}

void
DiagnosticWrapper::gfpFailed()
{
  if (_diag.diagnosticType() == Diagnostic991)
  {
    _diag << "  GROUP FARE PATH VALIDATION FAILED";
    _diag.lineSkip(2);
  }
}

void
DiagnosticWrapper::printHeader()
{
  if (_diag.diagnosticType() == Diagnostic991)
    _diag.printHeader();
}

void
DiagnosticWrapper::revalidationPassed()
{
  if (_diag.diagnosticType() == Diagnostic991)
  {
    _diag << "  REVALIDATION PASSED";
    _diag.lineSkip(1);
  }
}
void
DiagnosticWrapper::revalidationFailed()
{
  if (_diag.diagnosticType() == Diagnostic991)
  {
    _diag << "  REVALIDATION FAILED";
    _diag.lineSkip(1);
  }
}

void
DiagnosticWrapper::listItems(const std::vector<SavedFPPQItem>& items)
{
  if (!_trx.diagnostic().diagParamIsSet("DD", "SHOWITEMS"))
    return;

  for (const SavedFPPQItem& item : items)
  {
    _diag << "  FARE PATH:" << std::endl << *item._fppqItem->farePath();
    _diag << "  FAIL REASON FOR MOTHER: "
          << "  CATEGORY " << std::to_string(item._failedCategory) << std::endl;
  }
}
}
}
