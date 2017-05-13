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

#include "Pricing/SimilarItin/DiagnosticWrapper.h"
#include "Pricing/SimilarItin/Revalidator.h"

#include <map>
#include <vector>

namespace tse
{
class ClassOfService;
class FarePath;
class FareUsage;
class Itin;
class PricingUnit;
class SimilarItinData;
class TravelSeg;

namespace similaritin
{
class Context;

template <typename D>
class BookingCodeSelector
{
public:
  explicit BookingCodeSelector(Context& context, D& diagnostic);

  bool getBookingCode(FarePath*, Itin&);

private:
  using TvlSegToItinIdxMap = std::map<TravelSeg*, unsigned int>;

  FareMarket* similarFareMarket(const SimilarItinData*, FareMarket*);
  bool fillWithSimilarFareMarkets(FarePath&, Itin&);
  bool revalidateFBR(FarePath&);
  bool revalidateBookingCodes(FarePath&, Itin&);
  bool validateMixedClass(FarePath&);

  D& _diagnostic;
  Revalidator<D> _revalidator;
  Context& _context;
};
}
}
