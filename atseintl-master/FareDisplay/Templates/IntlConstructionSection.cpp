//-------------------------------------------------------------------
//
//  File:        IntlConstructionSection.cpp
//  Authors:     LeAnn Perez
//  Created:     June 6, 2005
//  Description: This class abstracts a section.
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/Templates/IntlConstructionSection.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/DateTime.h"
#include "Common/FareDisplayResponseUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/TseConsts.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/DBEGlobalClass.h"
#include "DBAccess/FDAddOnFareInfo.h"
#include "DBAccess/GlobalDir.h"
#include "DBAccess/SITAAddonFareInfo.h"
#include "DBAccess/TariffCrossRefInfo.h"
#include "FareDisplay/InclusionCodeConsts.h"
#include "FareDisplay/RoutingSequenceGenerator.h"
#include "FareDisplay/Templates/ElementField.h"
#include "FareDisplay/Templates/ElementFilter.h"
#include "Fares/FDFareCurrencySelection.h"
#include "Rules/RuleConst.h"

#include <sstream>
#include <string>

namespace tse
{
namespace
{
ConfigurableValue<bool>
enableNewRDHeader("FAREDISPLAY_SVC", "NEW_RDHEADER", false);

Logger
logger("atseintl.FareDisplay.IntlConstructionSection");

const Indicator TF_G = 'G';
const Indicator TF_H = 'H';
const Indicator TF_J = 'J';
const Indicator TF_K = 'K';

const Indicator GCF_SGC = 'Y'; // specifies Global Classes
const Indicator GCF_DBE = 'N'; // specifies DBE classes
const Indicator GCF_ANY = 'A'; // any fare record - no restrictions

const Indicator FQI_E = 'E'; // exclude fare quality codes
}

// -------------------------------------------------------------------
// <PRE>
//
// @MethodName  IntlConstructionSection::buildDisplay()
//
// This is the main processing method of the IntlConstructionSection class.
// It requires a reference to the Fare Display Transaction. When
// called, it iterates through all Rule categories and
// displays the Category number and description.
//
// </PRE>
// -------------------------------------------------------------------------
void
IntlConstructionSection::buildDisplay()
{
  // Select fare from grouped list based on line number requested
  FareDisplayOptions* options = _trx.getOptions();
  if (options == nullptr)
  {
    return;
  }

  const std::vector<FDAddOnFareInfo*>& addOnFareList = _trx.allFDAddOnFare();

  if (addOnFareList.size() == 0)
  {
    // At this point, the addonFareList is supposed to contain only one fare.
    // If there are no fares, this logger error message can be used to diagnose
    // the root of the problem
    LOG4CXX_ERROR(logger, "No addon Fare to display");
    return;
  }
  if (addOnFareList.size() > 1)
  {
    // At this point, the addonFareList is supposed to contain only one fare.
    // If more than one fare, this logger error message can be used to diagnose
    // the root of the problem
    LOG4CXX_ERROR(logger, "More than one AddonFare to display, displaying the first one");
  }

  // Point to Addon Fare Requested for Display
  FDAddOnFareInfo* addon = addOnFareList.front();

  bool isAddOnFare = (_trx.getRequest()->inclusionCode() == ADDON_FARES);

  // Get Global Description
  std::string global;
  const GlobalDir* globalDir =
      _trx.dataHandle().getGlobalDir(addon->globalDir(), _trx.travelDate());
  if (globalDir != nullptr)
  {
    global = globalDir->description();
  }

  // Get Effective and Discontinue Dates
  std::string eDate = addon->effDate().dateToString(DDMMMYY, "");
  std::string dDate = addon->discDate().dateToString(DDMMMYY, "");

  // Get Travel Date
  std::string travelDate = _trx.getRequest()->requestedDepartureDT().dateToString(DDMMMYY, "");

  // Get Addon Tariff Code from Tariff Cross Reference
  TariffCode addonTariffCode = "";
  FareDisplayResponseUtil::getAddonTariffCode(
      _trx, addon->vendor(), addon->carrier(), addon->addonTariff(), addonTariffCode);

  // Get Display Currency
  CurrencyCode displayCurrency;
  FDFareCurrencySelection::getDisplayCurrency(_trx, displayCurrency);

  // Convert Fare to Currency for Display
  MoneyAmount fare = addon->fareAmt();
  if (addon->cur() != displayCurrency)
  {
    bool isInternational = true;
    CurrencyConversionFacade ccFacade;
    Money targetMoney(displayCurrency);
    Money sourceMoney(fare, addon->cur());

    if (ccFacade.convert(targetMoney,
                         sourceMoney,
                         _trx,
                         isInternational,
                         CurrencyConversionRequest::FAREDISPLAY))
    {
      fare = targetMoney.value();
    }
  }

  std::ostringstream* oss = &_trx.response();
  ElementField field;

  // Header
  if (FareDisplayUtil::isAxessUser(_trx))
    *oss << "      FARE BASIS     BK    FARE    TRAVEL-TICKET     DR    RTG" << std::endl;
  else
    *oss << "    V FARE BASIS       J   FARE   TRAVEL-TICKET DR  MINMAX  RTG" << std::endl;

  // Line Number
  oss->setf(std::ios::right, std::ios::adjustfield);
  *oss << std::setw(3) << _trx.getOptions()->lineNumber() << " ";

  // Vendor
  field.strValue() = addon->vendor();
  ElementFilter::vendorCode(field, _trx);
  oss->setf(std::ios::left, std::ios::adjustfield);
  *oss << std::setw(2) << field.strValue();

  // Fare Class
  *oss << std::setw(8) << addon->fareClass() << "         ";

  // Oneway/Roundtrip Indicator
  ElementFilter::addOnJourneyType(field, *addon);
  *oss << std::setw(2) << field.strValue() << " ";

  // Fare Amount
  Money moneyPayment(displayCurrency);
  oss->precision(moneyPayment.noDec(_trx.ticketingDate()));
  oss->setf(std::ios::fixed, std::ios::floatfield);
  oss->setf(std::ios::right, std::ios::adjustfield);
  *oss << std::setw(7) << fare;

  // TravelTicket
  oss->setf(std::ios::left, std::ios::adjustfield);
  *oss << "  -            ";

  // Directionality
  ElementFilter::addOnDirection(field, *addon);
  *oss << std::setw(2) << field.strValue() << " ";

  // Min/Max
  *oss << "  -/  - ";

  // Routing Global + Sequence Number or Routing Number
  ElementFilter::addOnRouting(field, addon);
  *oss << std::setw(4) << field.strValue() << std::endl;
  if (enableNewRDHeader.getValue())
  {
    // FILED TYO-OSA      CXR-AA    TVL-07MAR05   TARIFF-999/ITETTT
    *oss << "FILED " << addon->gatewayMarket() << "-" << addon->interiorMarket() << "      CXR-"
         << addon->carrier() << "    TVL-" << travelDate;
  }
  else
  {
    // FROM-TYO TO-OSA    CXR-AA    TVL-07MAR05   TARIFF-999/ITETTT
    *oss << "FROM-" << addon->gatewayMarket() << " TO-" << addon->interiorMarket() << "    CXR-"
         << addon->carrier() << "    TVL-" << travelDate;
  }

  if (!isAddOnFare)
    *oss << "  TARIFF-" << addonTariffCode << "/" << addon->addonTariff() << std::endl;
  else
  {
    *oss << "  FARE TAR-";

    // Get Fare Tariff from TariffCrossRef table
    DataHandle dh(_trx.ticketingDate());

    const std::vector<TariffCrossRefInfo*>& tariffRef = dh.getTariffXRefByAddonTariff(
        addon->vendor(), addon->carrier(), INTERNATIONAL, addon->addonTariff(), _trx.travelDate());

    for (const auto tariffXRefInfo : tariffRef)
    {
      *oss << tariffXRefInfo->fareTariffCode() << "/" << tariffXRefInfo->fareTariff() << std::endl;
      break; // display only 1st found fare Tariff
    } // end for
  }

  // FARE BASIS-*******  EFF-01JAN05  DISC-N/A
  *oss << "FARE BASIS-" << addon->fareClass();
  if (!isAddOnFare)
    *oss << " "
         << " EFF-" << eDate << "  DISC-" << dDate;

  // FARE BASIS-******* ADDON ZONE-29     ADDON TARIFF-AUSAXYZ/999
  else
  {
    *oss << " "
         << "ADDON ZONE-" << addon->arbZone() << "    ADDON TARIFF-" << addonTariffCode << "/"
         << addon->addonTariff() << std::endl;
  }
  // TARIFF FAMILY -            FOOTNOTE 1 -      FOOTNOTE 2 -
  if (addon->vendor() == SITA_VENDOR_CODE)
  {
    *oss << "TARIFF FAMILY-" << addon->tariffFamily() << "  "
         << "FOOTNOTE 1-" << addon->footNote1() << "   "
         << "FOOTNOTE 2-" << addon->footNote2() << std::endl;

    // cout<< "TARIFF FAMILY-"<< addon->tariffFamily()<<std::endl;
  }

  //*oss<<std::endl;

  // Is RD Addon Fare? If yes - no need to add AUTO PRICE
  if (!isAddOnFare)
  {
    // AUTO PRICE-YES
    *oss << "AUTO PRICE-";
    if (addon->inhibit() == RuleConst::FARE_FOR_DISPLAY_ONLY)
      *oss << "NO/INHIBITED";
    else
      *oss << "YES";
    *oss << std::endl;
  }

  // Display currency line
  if (isAddOnFare)
    buildCurrencyLine(addon->cur(),
                      addon->fareAmt(),
                      addon->noDec(),
                      addon->routing(),
                      addon->createDate(),
                      addon->expireDate(),
                      addon->effDate(),
                      addon->discDate(),
                      addon->footNote1(),
                      addon->footNote2(),
                      addon->vendor(),
                      false);
  else
    buildCurrencyLine(addon->cur(),
                      addon->fareAmt(),
                      addon->noDec(),
                      addon->routing(),
                      addon->createDate(),
                      addon->expireDate(),
                      addon->effDate(),
                      addon->discDate(),
                      addon->fareClass(),
                      addon->addonTariff(),
                      addonTariffCode,
                      addon->footNote1(),
                      addon->footNote2(),
                      false);

  *oss << " " << std::endl;

  // Format IC Display
  *oss << "IC.INTERNATIONAL CONSTRUCTION" << std::endl;
  *oss << "   ADDON AMOUNT FOR CONSTRUCTION PURPOSES ONLY" << std::endl;
  *oss << "   WHEN TRAVEL IS "
       << "/" << global << "/" << std::endl;
  *oss << "       APPLICABLE TO ZONE " << addon->arbZone() << "." << std::endl;
  *oss << "       MAY BE USED TO CONSTRUCT A THRU INTL FARE TO/FROM" << std::endl;

  *oss << "       "
       << FareDisplayUtil::splitLines(
              FareDisplayUtil::getAddonZoneDescr(
                  addon->vendor(), addon->carrier(), addon->addonTariff(), addon->arbZone(), _trx),
              56,
              "/ ",
              56,
              "       ") << "." << std::endl;

  *oss << "       THIS ADDON FARE MAY BE USED TO CONSTRUCT THRU INTL" << std::endl;
  *oss << "       FARES BY COMBINING WITH THE SAME FARE CLASS." << std::endl;

  if (isAddOnFare && addon->vendor() == Vendor::SITA)
  {
    // Display paragraphs for SITA addons
    buildSITADisplay(addon);
  }
}

// -------------------------------------------------------------------
// <PRE>
//
// @MethodName  IntlConstructionSection::buildSITADisplay()
//
// This is the method to display SITA addon paragraphs.
// It requires a pointer to the addon fare.
//
// </PRE>
// -------------------------------------------------------------------------
void
IntlConstructionSection::buildSITADisplay(const FDAddOnFareInfo* addon)
{
  std::ostringstream* oss = &_trx.response();

  //--------------------------------------------------
  // Paragraph for Tariff Family
  //--------------------------------------------------
  if (addon->tariffFamily() != ' ')
  {
    std::string tariffFamilies = "";
    Indicator tariffFamily = addon->tariffFamily();

    switch (tariffFamily)
    {
    case TF_G:
      tariffFamilies = "G/D/Q/U/V";
      break;
    case TF_H:
      tariffFamilies = "H/R/W/B";
      break;
    case TF_J:
      tariffFamilies = "J/U/D/T/V/Q";
      break;
    case TF_K:
      tariffFamilies = "K/R/W/B";
      break;
    default:
      break;
    }

    *oss << " " << std::endl;
    *oss << "       THIS ADDON FARE MAY BE USED TO CONSTRUCT THROUGH" << std::endl;
    *oss << "       INTERNATIONAL FARES BY COMBINING WITH SPECIFIED" << std::endl;

    if (!tariffFamilies.empty())
      *oss << "       FARES IN TARIFF FAMILIES " << tariffFamilies << "." << std::endl;
    else
      *oss << "       FARES IN TARIFF FAMILY " << tariffFamily << "." << std::endl;
  }

  //--------------------------------------------------
  // Paragraph for DBE Class
  //--------------------------------------------------
  if (!addon->dbeClasses().empty() && addon->globalClassFlag() != GCF_ANY)
  {
    std::set<DBEClass> dbeClasses;

    switch (addon->globalClassFlag())
    {
    case GCF_SGC:
    {
      for (const auto& dbeClass : addon->dbeClasses())
      {
        const std::vector<DBEGlobalClass*>& gdc = _trx.dataHandle().getDBEGlobalClass(dbeClass);
        for (const auto dbeGlobalClass : gdc)
        {
          for (const auto& dbeClassFromGlobal : dbeGlobalClass->dbeClasses())
            dbeClasses.insert(dbeClassFromGlobal);
        }
      }
      break;
    }

    case GCF_DBE:
    // Fall through on purpose
    default:
      dbeClasses = addon->dbeClasses();
      break;
    }

    *oss << " " << std::endl;
    *oss << "       THIS ADDON FARE MAY BE USED TO CONSTRUCT THROUGH" << std::endl;
    *oss << "       INTERNATIONAL FARES BY COMBINING WITH SPECIFIED" << std::endl;

    std::set<DBEClass>::const_iterator i = dbeClasses.begin();
    std::set<DBEClass>::const_iterator ie = dbeClasses.end();

    if (dbeClasses.size() > 1)
    {
      *oss << "       FARES IN DBE CLASSES" << std::endl;

      int cnt = 0;

      // Loop through all DBE classes to build display line
      for (; i != ie; ++i)
      {
        if (cnt > 50)
        {
          *oss << "/" << std::endl;
          cnt = 0;
        }

        if (cnt == 0)
          *oss << "       " << *i;
        else
          *oss << "/" << *i;

        cnt += (*i).size() + 1;
      }

      *oss << "." << std::endl;
    }

    // Only one DBE class
    else
    {
      *oss << "       FARES IN DBE CLASS " << *i << "." << std::endl;
    }
  }

  //--------------------------------------------------
  // Paragraph for Fare Quality Code
  //--------------------------------------------------
  if (!addon->fareQualCodes().empty())
  {
    *oss << " " << std::endl;
    *oss << "       THIS ADDON FARE MAY BE USED TO CONSTRUCT THROUGH" << std::endl;
    *oss << "       INTERNATIONAL FARES BY COMBINING WITH SPECIFIED" << std::endl;

    std::set<Indicator>::const_iterator i = addon->fareQualCodes().begin();
    std::set<Indicator>::const_iterator ie = addon->fareQualCodes().end();

    if (addon->fareQualCodes().size() > 1)
    {
      // Check for Exclude
      if (addon->fareQualInd() == FQI_E)
        *oss << "       FARES IN ANY FARE QUALITY CODE EXCEPT" << std::endl;
      else
        *oss << "       FARES IN FARE QUALITY CODES" << std::endl;

      int cnt = 0;

      // Loop through all Fare Quality codes to build display line
      for (; i != ie; ++i)
      {
        if (cnt > 50)
        {
          *oss << "/" << std::endl;
          cnt = 0;
        }

        if (cnt == 0)
          *oss << "       " << *i;
        else
          *oss << "/" << *i;

        cnt += 2;
      }

      *oss << "." << std::endl;
    }

    // Only one Fare Quality code
    else
    {
      // Check for Exclude
      if (addon->fareQualInd() == FQI_E)
        *oss << "       FARES IN ANY FARE QUALITY CODE EXCEPT " << *i << "." << std::endl;
      else
        *oss << "       FARES IN FARE QUALITY CODE " << *i << "." << std::endl;
    }
  }

  //--------------------------------------------------
  // Paragraph for Route Code
  //--------------------------------------------------
  if (!addon->routeCode().empty())
  {
    *oss << " " << std::endl;
    *oss << "       THIS ADDON FARE MAY BE USED TO CONSTRUCT THROUGH" << std::endl;
    *oss << "       INTERNATIONAL FARES BY COMBINING WITH SPECIFIED" << std::endl;
    *oss << "       FARES IN ROUTE CODE " << addon->routeCode() << "." << std::endl;
  }
}
} // tse namespace
