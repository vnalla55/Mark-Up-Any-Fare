// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include "Common/Logger.h"
#include "DBAccess/DataHandle.h"
#include "Taxes/AtpcoTaxes/DataModel/Services/CarrierApplication.h"
#include "Taxes/LegacyFacades/CarrierApplicationServiceV2.h"
#include "Taxes/LegacyFacades/DaoDataFormatConverter.h"
#include "Taxes/LegacyFacades/ConvertCode.h"
#include "Taxes/AtpcoTaxes/DataModel/Common/CodeIO.h"
#include "Taxes/AtpcoTaxes/ServiceInterfaces/AtpcoDataError.h"

#include <boost/format.hpp>

namespace tse
{

namespace
{
typedef CarrierApplicationServiceV2::Key Key;
typedef CarrierApplicationServiceV2::SharedConstValue SharedConstValue;

Logger logger("atseintl.Taxes.LegacyFacades");

class ReadData
{
  DateTime _ticketingDate;

public:
  explicit ReadData(const DateTime& ticketingDate) : _ticketingDate{ticketingDate} {}

  SharedConstValue
  operator()(const Key& key)
  {
    DataHandle dataHandle {_ticketingDate};
    std::unique_ptr<tax::CarrierApplication> mutableResult {new tax::CarrierApplication};

    if (const TaxCarrierAppl* rawResult =
            dataHandle.getTaxCarrierAppl(toTseVendorCode(key.first), static_cast<int>(key.second)))
    {
      DaoDataFormatConverter::convert(*rawResult, *mutableResult);
    }
    else
    {
      static const char* const message = "No Carrier Application record found with Vendor=\"%1%\" and ItemNo=\"%2%\". ";
      LOG4CXX_ERROR(logger, boost::format(message) % key.first % key.second);
    }

    return std::move(mutableResult);
  }
};

} // anonymous namespace

CarrierApplicationServiceV2::CarrierApplicationServiceV2(const DateTime& ticketingDate)
  : _cache(ReadData{ticketingDate})
{
}

SharedConstValue
CarrierApplicationServiceV2::getCarrierApplication(const tax::type::Vendor& vendor,
                                                   const tax::type::Index& itemNo) const
{
  return _cache.get(Key(vendor, itemNo));
}
}
