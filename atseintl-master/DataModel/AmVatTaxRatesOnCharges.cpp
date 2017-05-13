//----------------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#include "DataModel/AmVatTaxRatesOnCharges.h"

#include "Common/Logger.h"

#include <boost/algorithm/cxx11/any_of.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <map>
#include <string>
#include <vector>

namespace tse
{
static Logger
logger("atseintl.DataModel.AmVatTaxRatesOnCharges");

AmVatTaxRatesOnCharges::AmVatTaxRatesOnCharges(const std::string& acmsData)
{
  std::vector<std::string> acmsItems;
  boost::split(acmsItems, acmsData, boost::is_any_of("|"));

  for (auto const &acmsItem : acmsItems)
  {
    std::vector<std::string> attrs;
    boost::split(attrs, acmsItem, boost::is_any_of("/"));

    if (attrs.size() == 3)
    {
      AmVatTaxRate rate;
      try
      {
        unsigned taxRate = boost::lexical_cast<unsigned>(attrs[2]);

        if (taxRate < 100)
        {
          rate.setTaxRate(taxRate);
          rate.setTaxCode(attrs[1]);
          _amVatTaxRates[attrs[0]] = rate;
        }
        else
        {
          LOG4CXX_ERROR(logger, __LOG4CXX_FUNC__ << ": Tax rate not in range \"" << attrs[2] << "\"");
        }
      }
      catch (const boost::bad_lexical_cast &)
      {
        LOG4CXX_ERROR(logger, __LOG4CXX_FUNC__ << ": Invalid tax rate \"" << attrs[2] << "\"");
        continue;
      }

    }
  }
}

const AmVatTaxRatesOnCharges::AmVatTaxRate*
AmVatTaxRatesOnCharges::getAmVatTaxRate(const NationCode& nationCode) const
{
  std::map<NationCode, AmVatTaxRate>::const_iterator it = _amVatTaxRates.find(nationCode);

  if (it != _amVatTaxRates.end())
    return &it->second;

  return nullptr;
}
}
