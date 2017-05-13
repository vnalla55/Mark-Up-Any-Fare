//-------------------------------------------------------------------
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

#pragma once

#include "DBAccess/FareTypeTable.h"
#include "DataModel/RexBaseTrx.h"
#include "Diagnostic/DiagCollector.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"

namespace tse
{

class FareTypeTablePresenter : std::unary_function<const FareTypeTable*, void>
{
public:
  FareTypeTablePresenter(DiagCollector& dc, RexBaseTrx& trx) : _dc(dc), _trx(trx) {}

  void printFareType(const VendorCode& vendor, uint32_t tblItemNo, const DateTime& applicationDate)
  {
    if (tblItemNo)
    {
      const std::vector<FareTypeTable*>& fareTypes =
          _trx.getFareTypeTables(vendor, tblItemNo, applicationDate);

      std::for_each(fareTypes.begin(), fareTypes.end(), *this);
    }
    else
      printEmptyFareType();
  }

  void operator()(const FareTypeTable* ftt)
  {
    _dc << "FARE TYPE TBL: " << std::setw(5) << ftt->itemNo() << "FARE TYPE IND: " << std::setw(3)
        << static_cast<char>(ftt->fareTypeAppl()) << "FARE TYPES: " << ftt->fareType() << "\n";
  }

private:
  void printEmptyFareType()
  {
    FareTypeTable empty;
    empty.itemNo() = 0;
    empty.fareType() = "";
    empty.fareTypeAppl() = ' ';
    (*this)(&empty);
  }

  DiagCollector& _dc;
  RexBaseTrx& _trx;
};
}

