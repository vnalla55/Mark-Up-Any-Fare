
// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2007
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

#include "Taxes/LegacyTaxes/Category12.h"
#include "Taxes/LegacyTaxes/TaxDisplayCommonText.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/Common/TaxUtility.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"

#include <string>
#include <map>
#include <set>
#include <algorithm>

using namespace tse;

// ----------------------------------------------------------------------------
// <PRE>
//
// @function Category12::Category12
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------

Category12::Category12() : _subCat1(EMPTY_STRING())
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function Category12::~Category12
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------

Category12::~Category12() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function Category12::build
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------

void
Category12::build(TaxTrx& trx, TaxDisplayItem& taxDisplayItem)
{

  _subCat2 = TaxDisplayCommonText::getCommonText(*taxDisplayItem.taxCodeReg(),
                                                 TaxDisplayCommonText::TICKET_DESIGNATOR);

  if (taxDisplayItem.taxCodeReg()->taxRestrTktDsgs().empty() && _subCat2.empty())
  {
    _subCat1 = "     NO TICKET DESIGNATOR RESTRICTIONS APPLY.\n";
    return;
  }

  typedef std::set<std::string> TktDsgs;
  typedef std::map<CarrierCode, TktDsgs> TktDsgs4CarrierCodes;

  TktDsgs4CarrierCodes tktDsgs4CarrierCodes;

  for (const auto tktDsgsPtr : taxDisplayItem.taxCodeReg()->taxRestrTktDsgs())
  {
    if (tktDsgsPtr->carrier().size())
      tktDsgs4CarrierCodes[tktDsgsPtr->carrier()].insert(tktDsgsPtr->tktDesignator());
  }

  _subCat1 = "* TICKET DESIGNATOR -\n";

  for (const auto& tktDsgs4CarrierCode : tktDsgs4CarrierCodes)
  {
    _subCat1 += "  ON CARRIER " + tktDsgs4CarrierCode.first + " - ";

    std::ostringstream carriersStream;
    std::copy(tktDsgs4CarrierCode.second.begin(),
              tktDsgs4CarrierCode.second.end(),
              std::ostream_iterator<std::string>(carriersStream, ", "));

    _subCat1 += carriersStream.str();

    std::string::size_type replacepos = _subCat1.rfind(", ");

    if (replacepos != std::string::npos)
      _subCat1.replace(replacepos, 2, "\n");
  }

  std::string::size_type replacepos = _subCat1.rfind("\n");

  if (replacepos != std::string::npos)
    _subCat1.replace(replacepos, 1, ".\n");
}
