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
#include "Taxes/Common/PricingTrxOps.h"

#include "Common/FallbackUtil.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"

#include <algorithm>
#include <vector>

namespace tse
{
FALLBACK_DECL(taxResponseDuplicatesFix)

namespace
{

template <typename T>
class ProvisionalPushBack
{
  std::vector<T>& _vec;
  bool _undo;

public:
  explicit ProvisionalPushBack(std::vector<T>& vec, const T& val)
    : _vec(vec), _undo(true)
  {
    _vec.push_back(val);
  }

  void commit() { _undo = false; }

  ~ProvisionalPushBack()
  {
    if (UNLIKELY(_undo))
      _vec.pop_back();
  }
};

void rawAddTaxResponse(tse::TaxResponse& taxResponse, tse::PricingTrx& trx, tse::Itin& itin)
{
  ProvisionalPushBack<tse::TaxResponse*> push_back(itin.mutableTaxResponses(), &taxResponse);
  {
    boost::lock_guard<boost::mutex> g(trx.mutexTaxResponse());
    trx.taxResponse().push_back(&taxResponse);
  }
  push_back.commit();
}

bool isResponseAlreadyAdded(tse::TaxResponse& taxResponse, tse::PricingTrx& trx)
{
  if (tse::fallback::taxResponseDuplicatesFix(&trx))
    return false;

  boost::lock_guard<boost::mutex> g(trx.mutexTaxResponse());
  auto foundResponse = std::find(trx.taxResponse().begin(), trx.taxResponse().end(), &taxResponse);
  return foundResponse != trx.taxResponse().end();
}

} // anonymous namespace

void addTaxResponseInAdvance(tse::TaxResponse& taxResponse, tse::PricingTrx& trx, tse::Itin& itin)
{
  taxResponse.createdInAdvance() = true;
  rawAddTaxResponse(taxResponse, trx, itin);
}

void addUniqueTaxResponse(tse::TaxResponse& taxResponse, tse::PricingTrx& trx)
{
  addUniqueTaxResponse(taxResponse, trx, *taxResponse.farePath());
}

void addUniqueTaxResponse(tse::TaxResponse& taxResponse, tse::PricingTrx& trx, FarePath& farePath)
{
  if (LIKELY(!taxResponse.createdInAdvance()) && !isResponseAlreadyAdded(taxResponse, trx))
    rawAddTaxResponse(taxResponse, trx, *farePath.itin());
}

} // namespace tse

