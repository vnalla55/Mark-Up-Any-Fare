// ---------------------------------------------------------------------------
// (C) 2007, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "Rules/CarrierApplicationValidator.h"

#include "DataModel/RexBaseRequest.h"
#include "DBAccess/CarrierApplicationInfo.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/VoluntaryRefundsInfo.h"
#include "Diagnostic/DiagCollector.h"
#include "Util/BranchPrediction.h"

namespace tse
{
namespace
{

struct Carrier
{
  Carrier(const CarrierCode& cxr, bool& dollarFound) : _cxr(cxr), _dollarFound(dollarFound) {}

  bool operator()(const CarrierApplicationInfo* row) const
  {
    if (row->carrier() == DOLLAR_CARRIER)
      _dollarFound = true;

    return row->carrier() == _cxr;
  }

  const CarrierCode& _cxr;
  bool& _dollarFound;
};
}

const Indicator CarrierApplicationValidator::ALLOW = ' ';

CarrierApplicationValidator::CarrierApplicationValidator(const RexBaseRequest& request,
                                                         const VoluntaryRefundsInfo& record3,
                                                         DiagCollector* dc,
                                                         DataHandle& dh)
  : _cxr(request.newValidatingCarrier()),
    _excCxr(request.excValidatingCarrier()),
    _record3(record3),
    _dc(dc),
    _dh(dh)
{
}

bool
CarrierApplicationValidator::validate()
{
  bool result = determineValidity();
  composeDiagnostic(result);

  return result;
}

bool
CarrierApplicationValidator::determineValidity()
{
  if (!needToCheckTable())
    return true;

  if (dataError())
    return false;

  const CxrTbl& cxrTable = getCarrierApplTbl();
  bool dollarFound = false;

  CxrTbl::const_iterator matchedRow =
      std::find_if(cxrTable.begin(), cxrTable.end(), Carrier(_cxr, dollarFound));

  if (matchedRow != cxrTable.end())
    return (**matchedRow).applInd() == ALLOW ? true : false;

  return dollarFound;
}

void
CarrierApplicationValidator::composeDiagnostic(bool result)
{
  if (UNLIKELY(_dc))
  {
    *_dc << "\nCXR APPL TBL: " << _record3.carrierApplTblItemNo();
    *_dc << "\nVALIDATING CXR: " << _cxr << "\n";

    if (needToCheckTable() && !_cxr.empty())
    {
      const CxrTbl& cxrTable = getCarrierApplTbl();

      for (const auto cxrTab : cxrTable)
      {
        *_dc << "CXR APPL: ";
        (*cxrTab).applInd() == ' ' ? *_dc << "Y" : *_dc << "N";
        *_dc << "  CXR: " << (*cxrTab).carrier() << "\n";
      }
    }

    if (!result)
      *_dc << "FAILED ITEM " << _record3.itemNo() << " - REFUND NOT ALLOWED ON " << _cxr << "\n";
  }
}

bool
CarrierApplicationValidator::needToCheckTable()
{
  return !_excCxr.empty() && _cxr != _excCxr;
}

bool
CarrierApplicationValidator::dataError()
{
  return !_record3.carrierApplTblItemNo() || _cxr.empty();
}

const CarrierApplicationValidator::CxrTbl&
CarrierApplicationValidator::getCarrierApplTbl()
{
  return _dh.getCarrierApplication(_record3.vendor(), _record3.carrierApplTblItemNo());
}
}
