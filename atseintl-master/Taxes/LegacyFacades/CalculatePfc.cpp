// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#include "LegacyFacades/CalculatePfc.h"
#include "Pfc/PfcItem.h"
#include "LegacyFacades/ForEachTaxResponse.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"

namespace tax
{

namespace
{

class ComputePfc
{
  tse::PricingTrx& _trx;
public:
  explicit ComputePfc(tse::PricingTrx& trx) : _trx(trx) {}
  void operator()(tse::TaxResponse& taxResponse, tse::Itin& tseItin, tse::FarePath&) const
  {
    tseItin.validatingCarrier() = taxResponse.validatingCarrier();
    tse::PfcItem pfcItem;
    pfcItem.build(_trx, taxResponse);
  }
};

} // anonymous namespace

PfcTask::PfcTask(tse::PricingTrx& trx)
{
  this->trx(&trx);
}

void PfcTask::performTask()
try
{
  if (trx()->getOptions()->getCalcPfc())
    forEachTaxResponse(*trx(), ComputePfc(*trx()));
}
catch (const std::logic_error& err)
{
  _failureMessage = std::string("atpcoProcess buildRequest - ") + err.what();
}

} // namespace tax

