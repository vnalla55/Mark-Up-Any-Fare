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

#include "Common/FarePathCopier.h"
#include "Pricing/SimilarItin/BookingCodeSelector.h"
#include "Pricing/SimilarItin/ValidatingCarrierModule.h"

#include <vector>

namespace tse
{
class FPPQItem;
class FarePath;
class FareUsage;
class Itin;
namespace similaritin
{
struct Context;
class SegmentsBuilder;

template <typename D>
class FarePathBuilder : private FarePathCopier
{
public:
  FarePathBuilder(Context& context, D& diagnostic, bool validateCarrierModule = false);
  std::vector<FarePath*> cloneFarePaths(const std::vector<FPPQItem*>& groupFPath, Itin& est);
  FPPQItem* cloneFPPQItem(const FPPQItem& source, Itin& est);

private:
  std::vector<FarePath*> cloneForEachFPPQItem(const std::vector<FPPQItem*>& groupFPath, Itin& est);
  FarePath* cloneFarePath(FPPQItem& sourceItem, Itin& est);
  FarePath* duplicateAndAdjustFarePath(const FarePath& farePath, Itin& similar);
  bool adjustFareUsage(const SegmentsBuilder& segmentsBuilder,
                       const FareUsage& source,
                       FareUsage& output);
  std::vector<FarePath*> duplicateFarePathsForGSA(const FarePath& farePath, Itin& similar);
  bool selectBookingCode(FarePath&, Itin& est);

  ValidatingCarrierModule _validatingCarrierModule;
  BookingCodeSelector<D> _bookingCodeSelector;
  D& _diagnostic;
  Context& _context;
  bool _validateCarrierModule;
};
}
}
