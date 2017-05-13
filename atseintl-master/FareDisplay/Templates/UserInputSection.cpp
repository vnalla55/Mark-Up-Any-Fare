//-------------------------------------------------------------------
//
//  File:        UserInputSection.cpp
//  Authors:     Svetlana Tsarkova
//  Created:     September 29, 2005
//  Description: Display Diagnostic for User Input Request
//
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

#include "FareDisplay/Templates/UserInputSection.h"

#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"

#include <sstream>
#include <string>

namespace tse
{

void
UserInputSection::buildDisplay()
{
  _trx.response().clear();

  displayUserInput(_trx);

  _trx.response() << std::endl;

  _trx.response() << "*************************************************************"
                  << "\n";
  _trx.response() << "                     END DIAGNOSTIC INFO"
                  << "\n";
  _trx.response() << "      NODE: ";

  char hostName[1024];

  if (::gethostname(hostName, 1023) < 0)
  {
    std::cout << "hostName: '" << hostName << "'";
    _trx.response() << "HOST NAME ERROR"
                    << "\n";
  }
  else
  {
    std::string aString(hostName);
    std::transform(aString.begin(), aString.end(), aString.begin(), (int (*)(int))toupper);
    _trx.response() << aString << "\n";
  }

  _trx.response() << "*************************************************************" << std::endl;
}

void
UserInputSection::displayUserInput(FareDisplayTrx& trx) const
{

  trx.response() << " ******************** USER INPUT DIAGNOSTIC *****************" << std::endl;

  displayLocInfo(_trx);

  displayAgentInfo(_trx);

  displayRequestInfo(_trx);

  displayTravelSegs(_trx);

  displayFareDispOptions(_trx);
}

void
UserInputSection::displayLocInfo(FareDisplayTrx& trx) const
{

  trx.response() << "AGENT GEO:  CITY - "
                 << trx.getRequest()->ticketingAgent()->agentLocation()->city();

  trx.response() << "  STATE - " << trx.getRequest()->ticketingAgent()->agentLocation()->state();

  trx.response() << "  COUNTRY - " << trx.getRequest()->ticketingAgent()->agentLocation()->nation()
                 << std::endl;
}
void
UserInputSection::displayAgentInfo(FareDisplayTrx& trx) const
{
  trx.response() << "AGENTCITY/ACD/ - ";
  trx.response() << trx.getRequest()->ticketingAgent()->agentCity();
  trx.response() << "       TVlAGENCYPCC/PCC/ - ";
  trx.response() << trx.getRequest()->ticketingAgent()->tvlAgencyPCC() << std::endl;
  trx.response() << "MAINTVLAGENCYPCC/HTA/ - ";
  trx.response() << trx.getRequest()->ticketingAgent()->mainTvlAgencyPCC() << std::endl;
  trx.response() << "TVLAGENCYIATA/ITA/ - ";
  trx.response() << trx.getRequest()->ticketingAgent()->tvlAgencyIATA();
  trx.response() << "  HOMEAGENCYIATA/HIT/ - ";
  trx.response() << trx.getRequest()->ticketingAgent()->homeAgencyIATA() << std::endl;
  trx.response() << "AIRLINEDEPTCODE/SET/ - ";
  trx.response() << trx.getRequest()->ticketingAgent()->airlineDept();
  trx.response() << "   DUTYCODE/DUT/ - ";
  trx.response() << trx.getRequest()->ticketingAgent()->agentDuty() << std::endl;
  trx.response() << "AGENTFUNCTIONCODE/AGT/ - ";
  trx.response() << trx.getRequest()->ticketingAgent()->agentFunctions() << std::endl;
  trx.response() << "CXRCODE/ALC/ - ";
  trx.response() << trx.getRequest()->ticketingAgent()->cxrCode();
  trx.response() << "    COHOSTID/CHT/ - ";
  trx.response() << trx.getRequest()->ticketingAgent()->coHostID() << std::endl;
  trx.response() << "AGENTCURRENCY/ACC/: ";
  trx.response() << trx.getRequest()->ticketingAgent()->currencyCodeAgent() << std::endl;
}

void
UserInputSection::displayRequestInfo(FareDisplayTrx& trx) const
{
  trx.response() << "             ******** REQUEST  INFORMATION ********"
                 << "\n";
  trx.response() << "DISPLAY CURRENCY/C40/ - ";
  trx.response() << trx.getRequest()->displayCurrency() << "\n";
  trx.response() << "ALTERNATEDISPLAYCURRENCY/C41/ - ";
  trx.response() << trx.getRequest()->alternateDisplayCurrency() << "\n";
  trx.response() << "RETURN DATE/D02/ - " << trx.getRequest()->returnDate() << "\n";
  trx.response() << "DATERANGELOWER/D05/ - " << trx.getRequest()->dateRangeLower() << "\n";
  trx.response() << "DATERANGEUPPER/D06/ - " << trx.getRequest()->dateRangeUpper() << "\n";
  trx.response() << "PREFFEREDTRAVELDATE/D00/ - " << trx.getRequest()->preferredTravelDate()
                 << "\n";
  trx.response() << "BOOKING CODE/B30/ - " << trx.getRequest()->bookingCode() << "\n";
  trx.response() << "NUMBER OF FARE LEVELS/Q17/ - " << trx.getRequest()->numberOfFareLevels()
                 << "\n";
  trx.response() << "FARE BASIS CODE/B50/ - " << trx.getRequest()->fareBasisCode() << "\n";
  trx.response() << "TICKET DESIGNATOR/BE0/ - " << trx.getRequest()->ticketDesignator() << "\n";
  trx.response() << "INCLUSION CODE/BI0/ - " << trx.getRequest()->inclusionCode() << "\n";
  trx.response() << "REQUESTED INCLUSION CODE/BI0/ - ";
  trx.response() << trx.getRequest()->requestedInclusionCode() << "\n";
  trx.response() << "ADDSUBLINENUMBER/Q4N/ - ";
  trx.response() << trx.getRequest()->addSubLineNumber() << "\n";
  trx.response() << "ADDSUBPERCENTAGE/Q17/ - ";
  trx.response() << trx.getRequest()->addSubPercentage() << "\n";
  trx.response() << "CARRIER NOT ENTERED/PBB/ - ";
  displayYesNo(trx, trx.getRequest()->carrierNotEntered());

  displaySecondaryCxr(_trx);

  displaySecondaryCity1(_trx);

  displaySecondaryCity2(_trx);

  displayPsgTypes(_trx);

  displayDispPsgTypes(_trx);

  displayRec8PsgTypes(_trx);

  displayMultiAccountCodeCorpIds(_trx);
}

void
UserInputSection::displaySecondaryCxr(FareDisplayTrx& trx) const
{
  trx.response() << "SECONDARY CARRIER/B03/: ";
  if (!trx.getRequest()->secondaryCarrier().empty())
  {
    std::vector<CarrierCode>::const_iterator iter = trx.getRequest()->secondaryCarrier().begin();
    std::vector<CarrierCode>::const_iterator iterEnd = trx.getRequest()->secondaryCarrier().end();
    int _sizeLeft = trx.getRequest()->secondaryCarrier().size();
    int i;

    while (_sizeLeft > 0)
    {
      if (iter != trx.getRequest()->secondaryCarrier().begin())
      {

        trx.response() << "                      ";
      }
      if (_sizeLeft < 10)
      {
        for (i = 0; i != _sizeLeft; i++)
        {
          trx.response() << *iter << " ";

          if (iter != iterEnd)
          {
            iter++;
          }
        }
        _sizeLeft = 0; // we are done
        trx.response() << std::endl;
      }
      else
      {
        for (i = 0; i < 10; i++)
        {
          trx.response() << *iter << " ";
          if (iter != iterEnd)
          {
            iter++;
          }
        }
        trx.response() << std::endl;
        _sizeLeft = _sizeLeft - 10;
      }
    } // While loop
  }
  else
  {
    trx.response() << std::endl;
  }
}

void
UserInputSection::displaySecondaryCity1(FareDisplayTrx& trx) const
{

  if (!trx.getRequest()->secondaryCity1().empty())
  {
    trx.response() << "SECONDARY CITY 1/A10/: ";
    std::vector<LocCode>::const_iterator iter = trx.getRequest()->secondaryCity1().begin();
    std::vector<LocCode>::const_iterator iterEnd = trx.getRequest()->secondaryCity1().end();
    int _sizeLeft = trx.getRequest()->secondaryCity1().size();
    int i;

    while (_sizeLeft > 0)
    {
      if (iter != trx.getRequest()->secondaryCity1().begin())
      {

        trx.response() << "                     ";
      }
      if (_sizeLeft < 10)
      {
        for (i = 0; i != _sizeLeft; i++)
        {

          trx.response() << *iter << " ";

          if (iter != iterEnd)
          {
            iter++;
          }
        }
        _sizeLeft = 0; // we are done
        trx.response() << std::endl;
      }
      else
      {
        for (i = 0; i < 10; i++)
        {
          trx.response() << *iter << " ";
          if (iter != iterEnd)
          {
            iter++;
          }
        }
        trx.response() << std::endl;
        _sizeLeft = _sizeLeft - 10;
      }
    } // While loop
  }
  else
  {
    trx.response() << "SECONDARY CITY 1/A10/: " << std::endl;
  }
}

void
UserInputSection::displaySecondaryCity2(FareDisplayTrx& trx) const
{
  if (!trx.getRequest()->secondaryCity2().empty())
  {
    trx.response() << "SECONDARY CITY 2/A11/: ";
    std::vector<LocCode>::const_iterator iter = trx.getRequest()->secondaryCity2().begin();
    std::vector<LocCode>::const_iterator iterEnd = trx.getRequest()->secondaryCity2().end();
    int _sizeLeft = trx.getRequest()->secondaryCity2().size();
    int i;

    while (_sizeLeft > 0)
    {
      if (iter != trx.getRequest()->secondaryCity2().begin())
      {

        trx.response() << "                     ";
      }
      if (_sizeLeft < 10)
      {
        for (i = 0; i != _sizeLeft; i++)
        {
          trx.response() << *iter << " ";

          if (iter != iterEnd)
          {
            iter++;
          }
        }
        _sizeLeft = 0; // we are done
        trx.response() << std::endl;
      }
      else
      {
        for (i = 0; i < 10; i++)
        {
          trx.response() << *iter << " ";
          if (iter != iterEnd)
          {
            iter++;
          }
        }
        trx.response() << std::endl;
        _sizeLeft = _sizeLeft - 10;
      }
    } // While loop
  }
  else
  {
    trx.response() << "SECONDARY CITY 2/A11/: " << std::endl;
  }
}

void
UserInputSection::displayPsgTypes(FareDisplayTrx& trx) const
{
  trx.response() << "PASSENGER TYPES/B70/: ";
  if (!trx.getRequest()->passengerTypes().empty())
  {
    std::set<PaxTypeCode>::const_iterator iter = trx.getRequest()->passengerTypes().begin();
    std::set<PaxTypeCode>::const_iterator iterEnd = trx.getRequest()->passengerTypes().end();
    int _sizeLeft = trx.getRequest()->passengerTypes().size();
    int i;

    while (_sizeLeft > 0)
    {
      if (iter != trx.getRequest()->passengerTypes().begin())
      {

        trx.response() << "                      ";
      }
      if (_sizeLeft < 10)
      {
        for (i = 0; i != _sizeLeft; i++)
        {
          trx.response() << *iter << " ";

          if (iter != iterEnd)
          {
            iter++;
          }
        }
        _sizeLeft = 0; // we are done
        trx.response() << std::endl;
      }
      else
      {
        for (i = 0; i < 10; i++)
        {
          trx.response() << *iter << " ";

          if (iter != iterEnd)
          {
            iter++;
          }
        }
        trx.response() << std::endl;
        _sizeLeft = _sizeLeft - 10;
      }
    } // While loop  */
  }
  else
  {
    trx.response() << std::endl;
  }
}

void
UserInputSection::displayDispPsgTypes(FareDisplayTrx& trx) const
{
  trx.response() << "DISPLAY PASSENGER TYPES: ";
  if (!trx.getRequest()->displayPassengerTypes().empty())
  {
    std::vector<PaxTypeCode>::const_iterator iter =
        trx.getRequest()->displayPassengerTypes().begin();
    std::vector<PaxTypeCode>::const_iterator iterEnd =
        trx.getRequest()->displayPassengerTypes().end();
    int _sizeLeft = trx.getRequest()->displayPassengerTypes().size();
    int i;

    while (_sizeLeft > 0)
    {
      if (iter != trx.getRequest()->displayPassengerTypes().begin())
      {

        trx.response() << "                         ";
      }
      if (_sizeLeft < 10)
      {
        for (i = 0; i != _sizeLeft; i++)
        {
          trx.response() << *iter << " ";

          if (iter != iterEnd)
          {
            iter++;
          }
        }
        _sizeLeft = 0; // we are done
        trx.response() << std::endl;
      }
      else
      {
        for (i = 0; i < 10; i++)
        {
          trx.response() << *iter << " ";
          if (iter != iterEnd)
          {
            iter++;
          }
        }
        trx.response() << std::endl;
        _sizeLeft = _sizeLeft - 10;
      }
    } // While loop
  }
  else
  {
    trx.response() << std::endl;
  }
}

void
UserInputSection::displayRec8PsgTypes(FareDisplayTrx& trx) const
{
  trx.response() << "REC 8 PASSENGER TYPES: ";
  if (!trx.getRequest()->rec8PassengerTypes().empty())
  {
    std::set<PaxTypeCode>::const_iterator iter = trx.getRequest()->rec8PassengerTypes().begin();
    std::set<PaxTypeCode>::const_iterator iterEnd = trx.getRequest()->rec8PassengerTypes().end();
    int _sizeLeft = trx.getRequest()->rec8PassengerTypes().size();
    int i;

    while (_sizeLeft > 0)
    {
      if (iter != trx.getRequest()->rec8PassengerTypes().begin())
      {

        trx.response() << "                       ";
      }
      if (_sizeLeft < 10)
      {
        for (i = 0; i != _sizeLeft; i++)
        {
          trx.response() << *iter << " ";

          if (iter != iterEnd)
          {
            iter++;
          }
        }
        _sizeLeft = 0; // we are done
        trx.response() << std::endl;
      }
      else
      {
        for (i = 0; i < 10; i++)
        {
          trx.response() << *iter << " ";
          if (iter != iterEnd)
          {
            iter++;
          }
        }
        trx.response() << std::endl;
        _sizeLeft = _sizeLeft - 10;
      }
    } // While loop
  }
  else
  {
    trx.response() << std::endl;
  }
}

void
UserInputSection::displayFareDispOptions(FareDisplayTrx& trx) const
{
  trx.response() << "         ******** FARE DISPLAY OPTIONS *********" << std::endl;
  trx.response() << "ONEWAY FARE/P04/: ";
  displayYesNo(trx, trx.getOptions()->oneWayFare());
  trx.response() << "ROUNDTRIP FARE/P05/: ";
  displayYesNo(trx, trx.getOptions()->roundTripFare());
  trx.response() << std::endl;
  trx.response() << "HALF ROUNDTRIP FARE/P03/: ";
  displayYesNo(trx, trx.getOptions()->halfRoundTripFare());
  trx.response() << std::endl;
  trx.response() << "SORTASCENDING/P08/: ";
  displayYesNo(trx, trx.getOptions()->sortAscending());
  trx.response() << "SORTDESCENDING/P09/: ";
  displayYesNo(trx, trx.getOptions()->sortDescending());
  trx.response() << std::endl;
  trx.response() << "PRIVATE FARES/P1Z/: ";
  displayYesNo(trx, trx.getOptions()->privateFares());
  trx.response() << "PUBLIC FARES/P1Y/: ";
  displayYesNo(trx, trx.getOptions()->publicFares());
  trx.response() << std::endl;
  trx.response() << "EXCLUDE PENALTY FARES/P84/: ";
  displayYesNo(trx, trx.getOptions()->excludePenaltyFares());
  trx.response() << std::endl;
  trx.response() << "EXLUDE ADV PURCHASE FARES/P85/: ";
  displayYesNo(trx, trx.getOptions()->excludeAdvPurchaseFares());
  trx.response() << std::endl;
  trx.response() << "EXCLUDE RESTRICTED FARES/P86/: ";
  displayYesNo(trx, trx.getOptions()->excludeRestrictedFares());
  trx.response() << std::endl;
  trx.response() << "VALIDATE RULES/P80/: ";
  displayYesNo(trx, trx.getOptions()->validateRules());
  trx.response() << std::endl;
  trx.response() << "REVERSE SORT/P81/: ";
  displayYesNo(trx, trx.getOptions()->reverseSort());
  trx.response() << std::endl;
  trx.response() << "DISPLAY BASE TAX TOTAL AMOUNTS/P82/: ";
  displayYesNo(trx, trx.getOptions()->displayBaseTaxTotalAmounts());
  trx.response() << std::endl;
  trx.response() << "EXLUDE MIN/MAX STAY FARES/P83/: ";
  displayYesNo(trx, trx.getOptions()->excludeMinMaxStayFares());
  trx.response() << std::endl;
  trx.response() << "ALL CARRIERS/P87/: ";
  displayYesNo(trx, trx.getOptions()->allCarriers());
  trx.response() << std::endl;
  trx.response() << "ADULT FARES/P90/: ";
  displayYesNo(trx, trx.getOptions()->adultFares());
  trx.response() << std::endl;
  trx.response() << "CHILD FARES/P91/: ";
  displayYesNo(trx, trx.getOptions()->childFares());
  trx.response() << std::endl;
  trx.response() << "INFANT FARES/P92/: ";
  displayYesNo(trx, trx.getOptions()->infantFares());
  trx.response() << std::endl;
  trx.response() << "UNIQUE ACCOUNT CODE/P93/: ";
  displayYesNo(trx, trx.getOptions()->uniqueAccountCode());
  trx.response() << std::endl;
  trx.response() << "UNIQUE CORPORATE ID/P94/: ";
  displayYesNo(trx, trx.getOptions()->uniqueCorporateID());
  trx.response() << std::endl;
  trx.response() << "VIEW PRIVATE FARES/PBN/: ";
  displayYesNo(trx, trx.getOptions()->viewPrivateFares());
  trx.response() << std::endl;
  trx.response() << "SELLING CURRENCY/PBO/: ";
  displayYesNo(trx, trx.getOptions()->sellingCurrency());
  trx.response() << std::endl;
  trx.response() << "EXCLUDE DAY/TIME VALIDATION/PCC/: ";
  trx.response() << trx.getOptions()->excludeDayTimeValidation() << std::endl;
  trx.response() << "BRAND GROUPING OPT OUT/PCF/: ";
  trx.response() << trx.getOptions()->brandGroupingOptOut() << std::endl;
  trx.response() << "EXCLUDE PERCENTAGE PENALTY FARES/Q16/: ";
  trx.response() << trx.getOptions()->excludePercentagePenaltyFares() << std::endl;
  trx.response() << "TEMPLATE OVERRIDE/Q40/: ";
  trx.response() << trx.getOptions()->templateOverride() << std::endl;

  trx.response() << "        ******* RD SPECIFIC  ************" << std::endl;

  trx.response() << "RULE MENU DISPLAY/PAM/: ";
  displayYesNo(trx, trx.getOptions()->ruleMenuDisplay());
  trx.response() << std::endl;
  trx.response() << "ROUTING DISPLAY/PAN/: ";
  displayYesNo(trx, trx.getOptions()->routingDisplay());
  trx.response() << std::endl;
  trx.response() << "HEADER DISPLAY/PAO/: ";
  displayYesNo(trx, trx.getOptions()->headerDisplay());
  trx.response() << std::endl;
  trx.response() << "INTL CONSTRUCTION DISPLAY/PAP/: ";
  displayYesNo(trx, trx.getOptions()->IntlConstructionDisplay());
  trx.response() << std::endl;
  trx.response() << "FB DISPLAY/PAQ/: ";
  displayYesNo(trx, trx.getOptions()->FBDisplay());
  trx.response() << std::endl;
  trx.response() << "COMBINABILITY SCOREBOARD DISPLAY/PBA/: ";
  displayYesNo(trx, trx.getOptions()->combScoreboardDisplay());
  trx.response() << std::endl;
  trx.response() << "LINE NUMBER/Q46/: " << trx.getOptions()->lineNumber() << std::endl;

  displayRuleCats(_trx);
  displayAlphaCodes(_trx);
  displayCombinCodes(_trx);

  trx.response() << "        ******* CAT 25 INFO  *******"
                 << "\n";

  trx.response() << "CAT25 VENDOR CODE/S37/: " << trx.getOptions()->cat25Values().vendorCode()
                 << "\n";
  trx.response() << "CAT25 ITEM NUMBER/Q41/: " << trx.getOptions()->cat25Values().itemNo() << "\n";
  trx.response() << "CAT25 CREATE DATE/D12/: " << trx.getOptions()->cat25Values().createDate()
                 << "\n";

  trx.response() << "        ******* CAT 35 INFO  *******"
                 << "\n";

  trx.response() << "CAT35 VENDOR CODE/S37/: " << trx.getOptions()->cat35Values().vendorCode()
                 << "\n";
  trx.response() << "CAT35 ITEM NUMBER/Q41/: " << trx.getOptions()->cat35Values().itemNo() << "\n";
  trx.response() << "CAT35 CREATE DATE/D12/: " << trx.getOptions()->cat35Values().createDate()
                 << "\n";

  trx.response() << "        ******* DISCOUNTED FARE  INFO  *******"
                 << "\n";

  trx.response() << "DISCOUNTED VENDOR CODE/S37/: "
                 << trx.getOptions()->discountedValues().vendorCode() << "\n";
  trx.response() << "DISCOUNTED ITEM NUMBER/Q41/: " << trx.getOptions()->discountedValues().itemNo()
                 << "\n";
  trx.response() << "DISCOUNTED CREATE DATE/D12/: "
                 << trx.getOptions()->discountedValues().createDate() << "\n";

  trx.response() << "        ******* SHORT RD SPECIFIC  *******"
                 << "\n";

  trx.response() << "BASE FARE CURRENCY/C46/: " << trx.getOptions()->baseFareCurrency() << "\n";
  trx.response() << "CARRIER CODE/B00/: " << trx.getOptions()->carrierCode() << "\n";
  trx.response() << "FARE CLASS/BJ0/: " << trx.getOptions()->fareClass() << "\n";
  trx.response() << "TARIFF NUMBER/Q3W/: " << trx.getOptions()->fareTariff() << "\n";
  trx.response() << "ROUTING NUMBER/S49/: " << trx.getOptions()->routing() << "\n";
  trx.response() << "VENDOR CODE/S37/: " << trx.getOptions()->vendorCode() << "\n";
  trx.response() << "LINK NUMBER/Q46/: " << trx.getOptions()->linkNumber() << "\n";
  trx.response() << "SEQUENCE NUMBER/Q1K/: " << trx.getOptions()->sequenceNumber() << "\n";
  trx.response() << "CREATE DATE/D12/: " << trx.getOptions()->createDate() << "\n";

  trx.response() << "        ******* SHORT MP SPECIFIC  *******"
                 << "\n";

  trx.response() << "ONE WAY/PBP/: ";
  displayYesNo(trx, trx.getOptions()->oneWay());
  trx.response() << std::endl;
  trx.response() << "HALF ROUND TRIP/PBQ/: ";
  displayYesNo(trx, trx.getOptions()->halfRoundTrip());
  trx.response() << std::endl;
}

void
UserInputSection::displayRuleCats(FareDisplayTrx& trx) const
{
  trx.response() << "RULE CATEGORIES/Q3W/: ";
  if (!trx.getOptions()->ruleCategories().empty())
  {
    std::vector<CatNumber>::const_iterator iter = trx.getOptions()->ruleCategories().begin();
    std::vector<CatNumber>::const_iterator iterEnd = trx.getOptions()->ruleCategories().end();
    int _sizeLeft = trx.getOptions()->ruleCategories().size();
    int i;

    while (_sizeLeft > 0)
    {
      if (iter != trx.getOptions()->ruleCategories().begin())
      {

        trx.response() << "                       ";
      }
      if (_sizeLeft < 10)
      {
        for (i = 0; i != _sizeLeft; i++)
        {
          trx.response() << *iter << " ";

          if (iter != iterEnd)
          {
            iter++;
          }
        }
        _sizeLeft = 0; // we are done
        trx.response() << std::endl;
      }
      else
      {
        for (i = 0; i < 10; i++)
        {
          trx.response() << *iter << " ";
          if (iter != iterEnd)
          {
            iter++;
          }
        }
        trx.response() << std::endl;
        _sizeLeft = _sizeLeft - 10;
      }
    } // While loop
  }
  else
  {
    trx.response() << std::endl;
  }
}

void
UserInputSection::displayAlphaCodes(FareDisplayTrx& trx) const
{
  trx.response() << "ALPHA CODES/S59/: ";
  if (!trx.getOptions()->alphaCodes().empty())
  {
    std::vector<AlphaCode>::const_iterator iter = trx.getOptions()->alphaCodes().begin();
    std::vector<AlphaCode>::const_iterator iterEnd = trx.getOptions()->alphaCodes().end();
    int _sizeLeft = trx.getOptions()->alphaCodes().size();
    int i;

    while (_sizeLeft > 0)
    {
      if (iter != trx.getOptions()->alphaCodes().begin())
      {

        trx.response() << "                       ";
      }
      if (_sizeLeft < 10)
      {
        for (i = 0; i != _sizeLeft; i++)
        {
          trx.response() << *iter << " ";

          if (iter != iterEnd)
          {
            iter++;
          }
        }
        _sizeLeft = 0; // we are done
        trx.response() << std::endl;
      }
      else
      {
        for (i = 0; i < 10; i++)
        {
          trx.response() << *iter << " ";
          if (iter != iterEnd)
          {
            iter++;
          }
        }
        trx.response() << std::endl;
        _sizeLeft = _sizeLeft - 10;
      }
    } // While loop
  }
  else
  {
    trx.response() << std::endl;
  }
}

void
UserInputSection::displayCombinCodes(FareDisplayTrx& trx) const
{
  trx.response() << "COMBINABILITY CODES/S60/: ";
  if (!trx.getOptions()->combinabilityCodes().empty())
  {
    std::vector<CombinabilityCode>::const_iterator iter =
        trx.getOptions()->combinabilityCodes().begin();
    std::vector<CombinabilityCode>::const_iterator iterEnd =
        trx.getOptions()->combinabilityCodes().end();
    int _sizeLeft = trx.getOptions()->combinabilityCodes().size();
    int i;

    while (_sizeLeft > 0)
    {
      if (iter != trx.getOptions()->combinabilityCodes().begin())
      {

        trx.response() << "                       ";
      }
      if (_sizeLeft < 10)
      {
        for (i = 0; i != _sizeLeft; i++)
        {
          trx.response() << *iter << " ";

          if (iter != iterEnd)
          {
            iter++;
          }
        }
        _sizeLeft = 0; // we are done
        trx.response() << std::endl;
      }
      else
      {
        for (i = 0; i < 10; i++)
        {
          trx.response() << *iter << " ";
          if (iter != iterEnd)
          {
            iter++;
          }
        }
        trx.response() << std::endl;
        _sizeLeft = _sizeLeft - 10;
      }
    } // While loop
  }
  else
  {
    trx.response() << std::endl;
  }
}

void
UserInputSection::displayMultiAccountCodeCorpIds(FareDisplayTrx& trx) const
{
  if (trx.getRequest()->isMultiAccCorpId())
  {
    trx.response() << "         ******** MULTI ACCOUNT CODE / CORP ID *********" << std::endl;

    if (!trx.getRequest()->accCodeVec().empty())
    {
      std::vector<std::string>::iterator currIterAC = trx.getRequest()->accCodeVec().begin();
      std::vector<std::string>::iterator endIterAC = trx.getRequest()->accCodeVec().end();

      trx.response() << "ACCOUNT CODES /SM1/SM2/SM3/SM4/: ";
      while (currIterAC != endIterAC)
      {
        trx.response() << (*currIterAC) << std::endl;
        ++currIterAC;

        if (currIterAC != endIterAC)
        {
          trx.response() << "                                 ";
        }
      }

      trx.response() << std::endl;
    }

    if (!trx.getRequest()->corpIdVec().empty())
    {
      std::vector<std::string>::iterator currIterCI = trx.getRequest()->corpIdVec().begin();
      std::vector<std::string>::iterator endIterCI = trx.getRequest()->corpIdVec().end();

      trx.response() << "CORPORATE IDS /AC1/AC2/AC3/AC4/: ";
      while (currIterCI != endIterCI)
      {
        trx.response() << (*currIterCI) << std::endl;
        ++currIterCI;

        if (currIterCI != endIterCI)
        {
          trx.response() << "                                 ";
        }
      }
      trx.response() << std::endl;
    }
  }
  else if (!trx.getRequest()->accountCode().empty() || !trx.getRequest()->corporateID().empty())
  {
    trx.response() << "         ******** SINGLE ACCOUNT CODE / CORP ID *********" << std::endl;

    if (!trx.getRequest()->accountCode().empty())
    {
      trx.response() << "ACCOUNT CODE /S07/: " << trx.getRequest()->accountCode() << std::endl;
    }

    if (!trx.getRequest()->corporateID().empty())
    {
      trx.response() << "CORPORATE ID /AC0/: " << trx.getRequest()->corporateID() << std::endl;
    }
  }
}

void
UserInputSection::displayTravelSegs(FareDisplayTrx& trx) const
{
  trx.response() << "         ******** TRAVEL SEGMENTS INFO *********" << std::endl;
  if (!trx.travelSeg().empty())
  {
    std::vector<TravelSeg*>::const_iterator iter = trx.travelSeg().begin();
    std::vector<TravelSeg*>::const_iterator iterEnd = trx.travelSeg().end();

    for (; iter != iterEnd; ++iter)
    {
      trx.response() << "PNR SEGMENT /FSN/ : " << (*iter)->pnrSegment() << "\n";
      trx.response() << "SEGMENT ORDER /SMN/: " << (*iter)->segmentOrder() << "\n";
      trx.response() << "SEGMENT TYPE /STI/: " << (*iter)->segmentType() << "\n";
      trx.response() << "DEPARTURE DATE /DTE/: " << (*iter)->pssDepartureDate() << "\n";
      trx.response() << "DEPARTURE TIME /DEP/: " << (*iter)->pssDepartureTime() << "\n";
      trx.response() << "BOOKING DATE /DAT/: " << (*iter)->pssBookingDate() << "\n";
      trx.response() << "BOOKING TIME /TIM/: " << (*iter)->pssBookingTime() << "\n";
      trx.response() << "ARRIVAL DATE /ADT/: " << (*iter)->pssArrivalDate() << "\n";
      trx.response() << "ARRIVAL TIME /ATM/: " << (*iter)->pssArrivalTime() << "\n";
      trx.response() << "BOOKING CODE /COS/: " << (*iter)->getBookingCode() << "\n";
      trx.response() << "RES STATUS /AAC/: " << (*iter)->resStatus() << "\n";
      trx.response() << "ORIG AIRPORT /BPC/: " << (*iter)->origAirport() << "\n";
      trx.response() << "DEST AIRPORT /OPC/: " << (*iter)->destAirport() << "\n";
      trx.response() << "FARE BASIS CODE /SBC/: " << (*iter)->fareBasisCode() << "\n";
      trx.response() << "SPECIFIED FBC /FBC/: " << (*iter)->specifiedFbc() << "\n";
      trx.response() << "FORCED CONX /SFC/: ";
      displayYesNo(trx, ((*iter)->forcedConx()));
      trx.response() << "FORCED STOPOVER /SFS/: ";
      displayYesNo(trx, ((*iter)->forcedStopOver()));
      trx.response() << "FORCED FARE BRK /SFB/: ";
      displayYesNo(trx, ((*iter)->forcedFareBrk()));
      trx.response() << "FORCED NO FARE BRK /SNB/: ";
      displayYesNo(trx, ((*iter)->forcedNoFareBrk()));
      trx.response() << "FORCED SIDE TRIP /FST/: ";
      displayYesNo(trx, ((*iter)->forcedSideTrip()));

    } // end of loop
  } //  if
}

void
UserInputSection::displayYesNo(FareDisplayTrx& trx, Indicator ind) const
{
  switch (ind)
  {
  case 'Y':
    trx.response() << "YES"
                   << "\n";
    break;

  case 'N':
    trx.response() << "NO"
                   << "\n";
    break;

  case 'T':
    trx.response() << "T"
                   << "\n";
    break;

  case 'F':
    trx.response() << "F"
                   << "\n";
    break;

  default:
    trx.response() << " "
                   << "\n";
    break;
  }
}
} // tse namespace
