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
#pragma once

#include "Pricing/SimilarItin/FarePathBuilder.h"
#include "Pricing/SimilarItin/Revalidator.h"
#include "Pricing/SimilarItin/ValidatingCarrierModule.h"

#include "Diagnostic/Diag990Collector.h"
#include "Diagnostic/Diagnostic.h"

#include <stdint.h>

namespace tse
{
class FarePath;
class GroupFarePath;
class GroupFarePathFactory;
class Itin;
namespace similaritin
{
class Context;

template <typename D>
class PriceWithMother
{
public:
  PriceWithMother(Context& context, D& diagnostic);
  bool findSolution(GroupFarePathFactory& groupFarePathFactory);

private:
  bool verifySimilarItin(const std::vector<FPPQItem*>& groupFPath);
  bool populateSimilarItin(const std::vector<FPPQItem*>& groupFPath, Itin& est);
  bool processFinalGroupFarePath(const std::vector<FPPQItem*>& groupFPath);
  bool isPriced(const Itin& est, const std::vector<FPPQItem*>& gfp) const;

  ValidatingCarrierModule _validatingCarrierModule;
  Revalidator<D> _revalidator;
  FarePathBuilder<D> _farePathBuilder;
  Context& _context;
  D _diagnostic;
  uint32_t _nextGFPAttemptsNumber;
};
}
}
