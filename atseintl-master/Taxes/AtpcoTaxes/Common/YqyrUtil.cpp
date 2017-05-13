// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include <vector>

#include "Common/ProrateCalculator.h"
#include "Common/RangeUtils.h"
#include "Common/YqyrUtil.h"
#include "DataModel/Common/Types.h"
#include "Rules/PaymentDetail.h"
#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/Request.h"
#include "DomainDataObjects/YqYrPath.h"

namespace tax
{
namespace
{
class AmountGetter
{
public:
  AmountGetter(const ProrateCalculator&) {}
  type::MoneyAmount getAmount(const Range&,
                              const Range&,
                              type::MoneyAmount amount,
                              const GeoPath&,
                              bool) const
  {
    return amount;
  }
};

class ProratedAmountGetter
{
public:
  ProratedAmountGetter(const ProrateCalculator& calculator) : _calculator(calculator) {}
  type::MoneyAmount
  getAmount(const Range& yqyrRange,
            const Range& commonRange,
            type::MoneyAmount amount,
            const GeoPath& geoPath,
            bool skipHiddenPoints) const
  {
    return _calculator.getProratedAmount(yqyrRange, commonRange, amount, geoPath, skipHiddenPoints);
  }

private:
  const ProrateCalculator& _calculator;
};

template <class T>
type::MoneyAmount
calculateTaxableAmount(TaxableYqYrs& yqYrDetails,
                       const ProrateCalculator& calculator,
                       const type::Index paymentBegin,
                       const GeoPath& geoPath,
                       bool skipHiddenPoints)
{
  type::MoneyAmount totalTaxableAmount(0);
  const std::vector<TaxableYqYr>& yqYrs = yqYrDetails._subject;
  T amountGetter(calculator);

  for (size_t id = 0; id < yqYrs.size(); ++id)
  {
    if (yqYrDetails.isFailedRule(id))
      continue;

    const TaxableYqYr& yqYr = yqYrs[id];

    Range yqYrRange(yqYrDetails._ranges[id].first, yqYrDetails._ranges[id].second);
    ProperRange calcRange(paymentBegin, yqYrDetails._data[id]._taxPointEnd->id());
    Range commonRange = calcRange & yqYrRange;

    yqYrDetails._data[id]._taxableAmount =
        amountGetter.getAmount(yqYrRange, commonRange, yqYr._amount, geoPath, skipHiddenPoints);
    totalTaxableAmount += yqYrDetails._data[id]._taxableAmount;
  }
  return totalTaxableAmount;
}
}

type::MoneyAmount
getTaxableAmount(const ProrateCalculator& prorateCalculator,
                 const type::TaxAppliesToTagInd& taxAppliesToTagInd,
                 const type::Index& paymentBegin,
                 TaxableYqYrs& yqYrDetails,
                 const GeoPath& geoPath,
                 bool skipHiddenPoints /* = true */)
{
  type::MoneyAmount taxableAmount =
      ((taxAppliesToTagInd == type::TaxAppliesToTagInd::AllBaseFare) ||
       (taxAppliesToTagInd == type::TaxAppliesToTagInd::DefinedByTaxProcessingApplTag))
          ? calculateTaxableAmount<AmountGetter>(yqYrDetails,
                                                 prorateCalculator,
                                                 paymentBegin,
                                                 geoPath,
                                                 skipHiddenPoints)
          : calculateTaxableAmount<ProratedAmountGetter>(yqYrDetails,
                                                         prorateCalculator,
                                                         paymentBegin,
                                                         geoPath,
                                                         skipHiddenPoints);

  return taxableAmount;
}

type::MoneyAmount
getYqyrAmountForItin(const Itin& itin,
                     const Request& request)
{
  type::MoneyAmount yqyrAmount = 0;

  if (itin.yqYrPathRefId().has_value())
  {
    const YqYrPath& yqYrPath = request.yqYrPaths()[itin.yqYrPathRefId().value()];
    for (const YqYrUsage& yqYrUsage : yqYrPath.yqYrUsages())
    {
      const YqYr& yqYr = request.yqYrs()[yqYrUsage.index()];
      yqyrAmount += yqYr.amount();
    }
  }

  return yqyrAmount;
}

} // end of tax namespace
