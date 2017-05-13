/*----------------------------------------------------------------------------
 *  Copyright Sabre 2015
 *
 *     The copyright to the computer program(s) herein
 *     is the property of Sabre.
 *     The program(s) may be used and/or copied only with
 *     the written permission of Sabre or in accordance
 *     with the terms and conditions stipulated in the
 *     agreement/contract under which the program(s)
 *     have been supplied.
 *-------------------------------------------------------------------------*/
#include "Pricing/SimilarItin/Pricing.h"

#include "Diagnostic/Diag990Collector.h"
#include "Pricing/FPPQItem.h"
#include "Pricing/GroupFarePathFactory.h"
#include "Pricing/SimilarItin/Context.h"
#include "Pricing/SimilarItin/DiagnosticWrapper.h"
#include "Pricing/SimilarItin/FarePathValidator.h"
#include "Pricing/SimilarItin/PriceWithMother.h"
#include "Pricing/SimilarItin/PriceWithSavedFPPQItems.h"
#include "Util/BranchPrediction.h"

namespace tse
{
namespace similaritin
{
template <typename D>
void
Pricing<D>::price(GroupFarePathFactory& gfpf, const std::vector<FPPQItem*>& groupFPath)
{
  _diagnostic.printFamily(_context.motherItin, groupFPath);

  if (_context.motherItin.getSimilarItins().empty())
    return;
  _diagnostic.printHeader();
  PricingTrx& trx = _context.trx;

  FarePathValidator validator(trx, _context.motherItin);
  if (PriceWithSavedFPPQItems<D>(_context, _diagnostic, validator).priceAll(gfpf, &groupFPath))
    return;

  priceWithMother(gfpf);
}

template <typename D>
void
Pricing<D>::priceWithMother(GroupFarePathFactory& gfpf)
{
  PriceWithMother<D> priceWithMotherModule(_context, _diagnostic);
  priceWithMotherModule.findSolution(gfpf);
}

template class Pricing<DiagnosticWrapper>;
template class Pricing<NoDiagnostic>;
}
}
