//----------------------------------------------------------------------------
//
//  File:         EquipmentValidator.cpp
//  Description:  Tax Validator Class for ATSE International Project
//  Created:      3/25/2004
//  Authors:      Sommapan Lathitham
//
//  Description: This routine will validate tax equipment restriction vector
//               against the travelseg index that pass the Carrier Exemption
//               the method function call before this method.
//
//  Updates:
//          3/25/2004 - SL - Create EquipmentValidator for Tax for ATSE International.
//
//  Copyright Sabre 2004
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

#include "Taxes/LegacyTaxes/EquipmentValidator.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/TaxCodeReg.h"

using namespace tse;

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

EquipmentValidator::EquipmentValidator() {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

EquipmentValidator::~EquipmentValidator() {}

// ----------------------------------------------------------------------------
//
// bool EquipmentValidator::validateEquipment
//
// Description:  This routine check if tax code has any equipment exemption.
//
// ----------------------------------------------------------------------------

bool
EquipmentValidator::validateEquipment(PricingTrx& trx,
                                      TaxResponse& taxResponse,
                                      TaxCodeReg& taxCodeReg,
                                      uint16_t travelSegIndex)
{
  std::vector<TravelSeg*>::const_iterator tvlSegI;
  tvlSegI = taxResponse.farePath()->itin()->travelSeg().begin() + travelSegIndex;

  return validateEquipment(trx, taxResponse, taxCodeReg, (*tvlSegI));
}

bool
EquipmentValidator::validateEquipment(PricingTrx& trx,
                                      TaxResponse& taxResponse,
                                      TaxCodeReg& taxCodeReg,
                                      TravelSeg* travelSeg)
{

  bool match = false;

  if (taxCodeReg.equipmentCode().empty())
  {
    return true;
  }

  // Now validate check if this index match with any of that Tax Equipment
  // Restriction List.

  std::vector<std::string>::iterator equipmentCodeI;

  for (equipmentCodeI = taxCodeReg.equipmentCode().begin();
       equipmentCodeI != taxCodeReg.equipmentCode().end();
       equipmentCodeI++)
  {
    if ((*equipmentCodeI) == (travelSeg->equipmentType()))
    {
      match = true;
      break;
    }
  }
  if ((match && taxCodeReg.exempequipExclInd() == TAX_EXCLUDE) ||
      (!match && taxCodeReg.exempequipExclInd() == TAX_NOT_EXCLUDE))
  {
    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::EQUIPMENT_TYPE, Diagnostic818);

    return false; // reject/fail this tax
  }
  return true;
}
// EquipmentValidator::EquipmentValidator
