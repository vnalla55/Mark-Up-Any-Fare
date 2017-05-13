// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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
#pragma once

#include "Common/TaxName.h"
#include "DataModel/Common/Codes.h"
#include "DataModel/Common/Types.h"
#include "TaxDisplay/View.h"

#include <boost/optional.hpp>

#include <set>
#include <tuple>

namespace tax
{
class TaxRoundingInfoService;

namespace display
{

class ViewX1TaxTable : public View
{
public:
  using DataRow = std::tuple<const TaxName&, const type::TaxLabel*, const type::Vendor&>;
  using Comparator = bool(*)(const DataRow&, const DataRow&);
  using DataSet = std::set<DataRow, Comparator>;
  struct SingleNationData
  {
    const type::Nation& code;
    const type::NationName& name;
    const type::NationMessage& message;
  };

  static bool byTaxCodeCompare(const DataRow& lhs, const DataRow& rhs)
  {
    const TaxName& lhsTaxName = std::get<const TaxName&>(lhs);
    const TaxName& rhsTaxName = std::get<const TaxName&>(rhs);
    const int nationCompare = lhsTaxName.nation().compare(rhsTaxName.nation());
    if (nationCompare != 0)
      return nationCompare < 0;

    const int taxCodeCompare = lhsTaxName.taxCode().compare(rhsTaxName.taxCode());
    if (taxCodeCompare != 0)
      return taxCodeCompare < 0;

    const int taxTypeCompare = lhsTaxName.taxType().compare(rhsTaxName.taxType());
    if (taxTypeCompare != 0)
      return taxTypeCompare < 0;

    const type::Vendor& lhsVendor = std::get<const type::Vendor&>(lhs);
    const type::Vendor& rhsVendor = std::get<const type::Vendor&>(rhs);
    const int vendorCompare = lhsVendor.compare(rhsVendor);
    if (vendorCompare != 0)
      return vendorCompare < 0;

    return lhsTaxName.taxPointTag() < rhsTaxName.taxPointTag();

  }

  ViewX1TaxTable(const TaxRoundingInfoService& taxRoundingInfoService,
                 const DataSet& dataSet,
                 ResponseFormatter& responseFormatter) :
                   View(responseFormatter),
                   _dataSet(dataSet),
                   _taxRoundingInfoService(taxRoundingInfoService) {}

  bool header() override;
  bool body() override;
  bool footer() override;

  void setSingleNationData(const SingleNationData& data) { _singleNationData.emplace(data); }

private:
  const DataSet& _dataSet;
  const TaxRoundingInfoService& _taxRoundingInfoService;
  boost::optional<SingleNationData> _singleNationData;
};

} // namespace display
} // namespace tax
