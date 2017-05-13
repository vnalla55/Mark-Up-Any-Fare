//----------------------------------------------------------------------------
//
//  File:         FareClassValidator.cpp
//  Description:  Tax Validator Class for ATSE International Project
//  Created:      3/25/2004
//  Authors:      Sommapan Lathitham
//
//  Description: This routine will validate tax fare type and/or
//               tax fare class with faretype and/or fareclass
//               against fare data.
//
//  Updates:
//          3/25/2004 - SL - Create FareClassValidator for Tax for ATSE International.
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

#include "Taxes/LegacyTaxes/FareClassValidator.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/Itin.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/Fare.h"
#include "Rules/RuleUtil.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"

#include <boost/logic/tribool.hpp>

using namespace tse;


// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

FareClassValidator::FareClassValidator() {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

FareClassValidator::~FareClassValidator() {}

// ----------------------------------------------------------------------------
//
// bool FareClassValidator::validateFareClassRestriction
//
// Description:  Check if tax code has any faretype or fareclass
//               restriction apply.
//
// ----------------------------------------------------------------------------

bool
FareClassValidator::validateFareClassRestriction(PricingTrx& trx,
                                                 TaxResponse& taxResponse,
                                                 TaxCodeReg& taxCodeReg,
                                                 uint16_t travelSegIndex)
{
  const FarePath* farePath = taxResponse.farePath(); // lint !e530
  std::vector<PricingUnit*>::const_iterator puI;
  std::vector<FareUsage*>::const_iterator fuI;

  // ************************************************************************************
  // If there is multi Pax, there will be multi FarePath
  // Each Pax can have multi FarePath. Each farepath(s) have itin associated.
  // PaxTypeFare is just 1-1 to FareUsage of that PU and FarePath...
  // ************************************************************************************

  //------------Check FareClass Restrictions----------------
  uint16_t numRestrFareClass = taxCodeReg.restrictionFareClass().size();

  //Clear number of classes, because in this case the list contains
  //fares, which we should consider during TaxOnTax calcualtion.
  //It's not a list with fares to validate the sequence!
  if (utc::fareClassToAmount(trx, taxCodeReg))
    numRestrFareClass = 0;

  bool applyToWholeList = utc::fareClassCheckAllFares(trx, taxCodeReg);

  boost::tribool matchFareClass(false);
  if(applyToWholeList)
    matchFareClass = boost::indeterminate;

  if (numRestrFareClass > 0)
  {
    for (puI = farePath->pricingUnit().begin(); puI != farePath->pricingUnit().end(); puI++)
    {
      for (fuI = (*puI)->fareUsage().begin(); fuI != (*puI)->fareUsage().end(); fuI++)
      {
        const char* fareClassFareCStr = (*fuI)->paxTypeFare()->fareClass().c_str();

        auto it = std::find_if(taxCodeReg.restrictionFareClass().begin(),
          taxCodeReg.restrictionFareClass().end(),
          [fareClassFareCStr](const FareClassCode& fareClass) -> bool
            { return RuleUtil::matchFareClass(fareClass.c_str(), fareClassFareCStr); });

        if (it != taxCodeReg.restrictionFareClass().end())
        {
          if (!matchFareClass && applyToWholeList)
          {
            matchFareClass = boost::indeterminate;
            break;
          }

          matchFareClass = true;

          if (!applyToWholeList)
            break;
        }
        else if (applyToWholeList)
        {
          if (matchFareClass)
          {
            matchFareClass = boost::indeterminate;
            break;
          }

          matchFareClass = false;
        }
      } // FU Loop

      if (indeterminate(matchFareClass))
        break;

      if (!applyToWholeList && matchFareClass)
        break;

    } // PU Loop

    // if a match is found and Exclude is set, fail/reject the tax
    // if a match is found and Include is set, pass the tax
    if ((matchFareClass && taxCodeReg.fareclassExclInd() == TAX_EXLUDE) ||
        (!matchFareClass && taxCodeReg.fareclassExclInd() != TAX_EXLUDE))

    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::FARE_CLASS, Diagnostic815);

      return false;
    }
  } // No. of fare class items

  //------------Check Fare Type Restrictions----------------
  uint16_t numRestrFareType = taxCodeReg.restrictionFareType().size();
  bool match = false;
  uint16_t j = 0;

  FareType* taxExemptFareTypeI = nullptr;

  if (UNLIKELY(numRestrFareType > 0))
  {
    for (puI = farePath->pricingUnit().begin(); puI != farePath->pricingUnit().end(); puI++)
    {
      for (fuI = (*puI)->fareUsage().begin(); fuI != (*puI)->fareUsage().end(); fuI++)
      {
        for (; j != numRestrFareType; j++)
        {
          taxExemptFareTypeI = &(taxCodeReg.restrictionFareType()[j]);

          PaxTypeFare* paxTypeFare = (*fuI)->paxTypeFare(); // lint !e530

          if (paxTypeFare->fcaFareType() == *taxExemptFareTypeI)
          {
            match = true;
            break;
          }
        }
      } // FU loop
    } // PU loop

    // if a match is found and Exclude is set, fail/reject the tax
    // if a match is found and Include is set, pass the tax
    if ((match && taxCodeReg.fareTypeExclInd() == TAX_EXLUDE) ||
        (!match && taxCodeReg.fareTypeExclInd() != TAX_EXLUDE))

    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::FARE_CLASS, Diagnostic815);

      return false;
    }
  }
  return true;
}
// FareClassValidator::FareClassValidator
