#include "Common/DiagMonitor.h"

namespace tse
{

DiagMonitor::DiagMonitor(PricingTrx& trx, DiagnosticTypes diagType)
{
  _diag = DCFactory::instance()->create(trx);
  if (UNLIKELY(_diag && trx.diagnostic().diagnosticType() == diagType))
    _diag->enable(diagType);

  if (UNLIKELY(_diag && (diagType == DiagnosticNone)))
    _diag->clear();
}

DiagMonitor::DiagMonitor(PricingTrx& trx, DiagnosticTypes diagType1, DiagnosticTypes diagType2)
{
  _diag = DCFactory::instance()->create(trx);
  if (_diag && (trx.diagnostic().diagnosticType() == diagType1 ||
                trx.diagnostic().diagnosticType() == diagType2))
    _diag->enable(diagType1, diagType2);

  if (_diag && ((diagType1 == DiagnosticNone) || (diagType2 == DiagnosticNone)))
    _diag->clear();
}

DiagMonitor::DiagMonitor(PricingTrx& trx,
                         DiagnosticTypes diagType1,
                         DiagnosticTypes diagType2,
                         DiagnosticTypes diagType3)
{
  _diag = DCFactory::instance()->create(trx);
  if (LIKELY(_diag && (trx.diagnostic().diagnosticType() == diagType1 ||
                trx.diagnostic().diagnosticType() == diagType2 ||
                trx.diagnostic().diagnosticType() == diagType3)))
    _diag->enable(diagType1, diagType2, diagType3);

  if (LIKELY(_diag && ((diagType1 == DiagnosticNone) || (diagType2 == DiagnosticNone) ||
                (diagType3 == DiagnosticNone))))
    _diag->clear();
}

DiagMonitor::DiagMonitor(DCFactory* diagFactory, PricingTrx& trx, DiagnosticTypes diagType)
{
  _diag = diagFactory->create(trx);
  if (_diag && trx.diagnostic().diagnosticType() == diagType)
    _diag->enable(diagType);

  if (_diag && (diagType == DiagnosticNone))
    _diag->clear();
}

DiagMonitor::~DiagMonitor()
{
  if (LIKELY(_diag))
  {
    _diag->flushMsg();
  }
}

} // tse
