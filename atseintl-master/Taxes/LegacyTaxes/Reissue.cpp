// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
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

#include <sstream>
#include "DataModel/TaxTrx.h"
#include "DBAccess/TaxReissue.h"
#include "Taxes/LegacyTaxes/Reissue.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/LocationDescriptionUtil.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "DBAccess/Nation.h"
#include "Common/Money.h"
#include "DBAccess/DataHandle.h"
#include "Common/Money.h"
#include <boost/lexical_cast.hpp>
#include <string>
#include <algorithm>

using namespace tse;
using namespace std;

log4cxx::LoggerPtr
Reissue::_logger(log4cxx::Logger::getLogger("atseintl.Taxes.Reissue"));

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxResponse::TaxResponse
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------

Reissue::Reissue()
  : _subCat0(EMPTY_STRING()),
    _subCat1(EMPTY_STRING()),
    _subCat2(EMPTY_STRING()),
    _subCat3(EMPTY_STRING()),
    _subCat4(EMPTY_STRING())

{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxResponse::~TaxResponse
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------

Reissue::~Reissue() {}
// ----------------------------------------------------------------------------
// <PRE>
//
// @function buildReissue
//
// Description:  Complete all Reissue layouts
//
// </PRE>
// ----------------------------------------------------------------------------

void
Reissue::build(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{

  _subCat0 = "* " + taxDisplayItem.taxCodeReg()->taxCode();
  _subCat0 +=
      "   SEQ: " + boost::lexical_cast<std::string>(taxDisplayItem.taxReissue()->seqNo()) + "\n";

  if (taxDisplayItem.taxReissue()->refundInd() == YES)
    _subCat1 = "* TAX IS REFUNDABLE.\n";
  else if (taxDisplayItem.taxReissue()->refundInd() == NO)
    _subCat1 = "* TAX IS NON REFUNDABLE.\n";
  else
    _subCat1 = "     TAX IS NON REFUNDABLE.\n";

  if (!taxDisplayItem.taxReissue()->reissueLoc().empty())
  {
    _subCat2 = "* APPLIES FOR REISSUES IN ";

    if (taxDisplayItem.taxReissue()->reissueExclLocInd() == YES)
      _subCat2 = "* DOES NOT APPLY FOR REISSUES IN ";

    _subCat2 += LocationDescriptionUtil::description(taxTrx,
                                                     taxDisplayItem.taxReissue()->reissueLocType(),
                                                     taxDisplayItem.taxReissue()->reissueLoc());
    _subCat2 += ".\n";
  }
  else
    _subCat2 = "     NO SALE-REISSUE RESTRICTION.\n";

  if (!taxDisplayItem.taxReissue()->validationCxr().empty())
  {
    _subCat3 = "* TICKETING REISSUE CARRIER/S ";

    if (taxDisplayItem.taxReissue()->tktlCxrExclInd() == YES)
    {
      _subCat3 = "* EXCEPT TICKETING REISSUE CARRIER/S ";
    }

    bool carrierAdded = false;
    std::vector<CarrierCode>::iterator carrierCodeIter =
        taxDisplayItem.taxReissue()->validationCxr().begin();
    std::vector<CarrierCode>::iterator carrierCodeEndIter =
        taxDisplayItem.taxReissue()->validationCxr().end();

    for (; carrierCodeIter != carrierCodeEndIter; carrierCodeIter++)
    {
      if (carrierAdded)
        _subCat3 += ", ";

      _subCat3 += (*carrierCodeIter);
      carrierAdded = true;
    }
    _subCat3 += ".\n";
  }
  else
    _subCat3 = "     NO VALIDATING CARRIER-TICKETING REISSUE CARRIER RESTRICTION.\n";

  if (!taxDisplayItem.taxReissue()->currencyCode().empty() &&
      taxDisplayItem.taxReissue()->taxAmt() != 0)
  {
    _subCat4 = "* TAX EXEMPT IF REISSUE IS LESS THAN ";
    _subCat4 += taxDisplayItem.taxReissue()->currencyCode();

    std::ostringstream tempStr;
    tempStr.setf(std::ios::fixed, std::ios::floatfield);
    tempStr.setf(std::ios::left, std::ios::adjustfield);

    Money moneyPublished(taxDisplayItem.taxCodeReg()->taxCur());
    tempStr.precision(moneyPublished.noDec(taxTrx.ticketingDate()));

    tempStr << " " << taxDisplayItem.taxReissue()->taxAmt();
    tempStr << "." << endl;

    _subCat4 += tempStr.str();
  }
  else
    _subCat4 = "     NO CURRENCY RESTRICTION.\n";

  return;
}
