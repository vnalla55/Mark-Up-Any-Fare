//----------------------------------------------------------------------------
//  File:        Diag315Collector.h
//  Authors:
//  Created:
//
//  Description: Diagnostic 315 formatter
//
//  Updates:
//          date - initials - description.
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

#pragma once

#include "DBAccess/Record2Types.h"
#include "DBAccess/SalesRestriction.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Rules/RuleUtil.h"

#include <string>

namespace tse
{
class PaxTypeFare;
class SalesRestriction;

class Diag315Collector : public DiagCollector
{
public:

  //@TODO will be removed, once the transition is done
  explicit Diag315Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag315Collector() = default;

  template <class T, class SalesRestrictionRuleT>
  void diag315Collector(const PaxTypeFare& paxTypeFare,
                        const T& cri,
                        const typename T::item_info_type* rule,
                        const SalesRestriction* srr,
                        const Record3ReturnTypes statusRule,
                        const typename SalesRestrictionRuleT::Cat15FailReasons failReason)
  {
    using namespace std;
    if (_active)
    {
      writeHeader(paxTypeFare);

      string status;
      if (statusRule == PASS)
      {
        status = "PASS";
      }
      else if (statusRule == SOFTPASS)
      {
        status = "SOFTPASS";
      }
      else if (statusRule == SKIP)
      {
        status = "SKIP";
      }
      else
      {
        status = "FAIL";
      }

      DiagCollector& dc(*this);

      dc.setf(std::ios::left, std::ios::adjustfield);

      dc << "VALIDATION RESULT: ";
      dc << setw(15) << status;
      dc << "R3 ITEM NUMBER: ";
      dc << setw(8) << srr->itemNo();
      dc << '\n';

      dc << "ERROR CODE: ";
      dc << setw(2) << failReason;

      if (failReason != SalesRestrictionRuleT::SALES_RESTR_NO_ERROR)
      {
        string error;
        switch (failReason)
        {
        case SalesRestrictionRuleT::SALES_RESTR_COUNTRY:
        {
          error = " - COUNTRY RESTRICTION";
          break;
        }
        case SalesRestrictionRuleT::SALES_RESTR_SALES_DATE:
        {
          error = " - SALES DATE";
          break;
        }
        case SalesRestrictionRuleT::SALES_RESTR_FORM_OF_PAYMENT:
        {
          error = " - FORM OF PAYMENT";
          break;
        }
        case SalesRestrictionRuleT::SALES_RESTR_CURRENCY:
        {
          error = " - CURRENCY RESTRICTION";
          break;
        }
        case SalesRestrictionRuleT::SALES_RESTR_SOLD_TICKET:
        {
          error = " - SALES/TICKETING POINTS";
          break;
        }
        case SalesRestrictionRuleT::SALES_RESTR_CARRIER:
        {
          error = " - CARRIER RESTRICTION";
          break;
        }
        case SalesRestrictionRuleT::SALES_RESTR_LOCALE:
        {
          error = " - LOCALE RESTRICTION";
          break;
        }
        case SalesRestrictionRuleT::SALES_RESTR_SECURITY:
        {
          error = " - SECURITY RESTRICTION";
          break;
        }
        case SalesRestrictionRuleT::SALES_RESTR_CRS_OTHER_CARRIER:
        {
          error = " - SOLD NOT VIA SABRE";
          break;
        }
        case SalesRestrictionRuleT::SALES_RESTR_DATA_UNAVAILABLE:
        {
          error = " - DATA UNAVAILABLE";
          break;
        }
        case SalesRestrictionRuleT::SALES_RESTR_TICKET:
        {
          error = " - TICKET RESTRICTION";
          break;
        }
        case SalesRestrictionRuleT::SALES_RESTR_PRIVATE_SECURITY:
        {
          error = " - NO SECURITY FOR PRIVATE FARE";
          break;
        }
        case SalesRestrictionRuleT::SALES_RESTR_TEXT_ONLY:
        {
          error = " - TEXT ONLY";
          break;
        }
        case SalesRestrictionRuleT::SALES_RESTR_SKIP:
        {
          error = " - SOLD NOT VIA SABRE FOR QUALIFIED";
          break;
        }
        case SalesRestrictionRuleT::SALES_RESTR_DATE_OVERRIDE:
        {
          error = " - OVERRIDE DATE TABLE";
          break;
        }
        default:
        {
          error = " - UNKNOWN";
          break;
        }
        }
        dc << error;
      }

      dc << '\n';

      dc << "CATEGORY 15 RULE DATA:";
      dc << '\n';
      dc << '\n';
      //
      if (srr->unavailTag() == 'X') // dataUnavailable
      {
        dc << " DATA UNAVAILABLE" << '\n';
      }
      else if (srr->unavailTag() == 'Y') // textOnly
      {
        dc << " TEXT DATA ONLY" << '\n';
      }
      // 994
      dc << " OVERRIDE TABLE 994 NUMBER: ";
      dc << setw(7) << srr->overrideDateTblItemNo() << '\n';
      // 996
      dc << " TEXT TABLE 996 NUMBER: ";
      dc << setw(7) << srr->textTblItemNo() << '\n';

      // date
      dc << " EARLIEST DATE FOR RESERVATIONS: " << srr->earliestResDate().dateToIsoExtendedString()
         << '\n';
      dc << " LATEST   DATE FOR RESERVATIONS: " << srr->latestResDate().dateToIsoExtendedString()
         << '\n';
      dc << " EARLIEST DATE FOR TICKETING   : " << srr->earliestTktDate().dateToIsoExtendedString()
         << '\n';
      dc << " LATEST   DATE FOR TICKETING   : " << srr->latestTktDate().dateToIsoExtendedString()
         << '\n';
      dc << '\n';

      if (paxTypeFare.selectedFareNetRemit())
      {
        // done.
        return;
      }
      // country
      dc << " COUNTRY RESTRICTION:" << '\n';
      if (srr->countryRest() == ' ')
      {
        dc << "  NONE" << '\n';
      }
      else
      {
        string country;
        if (srr->countryRest() == 'O') // countryOfOrigin)
          country = "ORIGIN";
        else if (srr->countryRest() == 'D') // countryOfDestination)
          country = "DESTINATION";
        else
          country = "ORIGIN AND DESTINATION";

        dc << "  SALES ARE RESTRICTED TO COUNTRY OF " << country << '\n';
      }
      dc << '\n';
      // fop
      dc << " FORM OF PAYMENT:" << '\n';
      if (srr->fopCashInd() == ' ' && srr->fopCheckInd() == ' ' && srr->fopCreditInd() == ' ' &&
          srr->fopGtrInd() == ' ')
      {
        dc << "  NONE" << '\n';
      }
      else
      {
        dc << "  FORMS OF PAYMENT NOT ACCEPTED FOR A TICKET:" << '\n';

        if (srr->fopCashInd() != ' ')
          dc << "   CASH" << '\n';
        if (srr->fopCheckInd() != ' ')
          dc << "   CHECK" << '\n';
        if (srr->fopCreditInd() != ' ')
          dc << "   CREDIT CARD" << '\n';
        if (srr->fopGtrInd() != ' ')
          dc << "   GOVERNMENT TRANSPORTATION REQUEST" << '\n';
      }
      dc << '\n';
      // currency
      dc << " CURRENCY RESTRICTION:" << '\n';
      if (srr->currCntryInd() == ' ' && !srr->curr().empty())
      {
        dc << "  NONE" << '\n';
      }
      else
      {
        if (srr->currCntryInd() != ' ')
        {
          string country;
          if (srr->currCntryInd() == 'O') // countryOfOrigin)
            country = "ORIGIN";
          else if (srr->currCntryInd() == 'D') // countryOfDestination)
            country = "DESTINATION";
          else
            country = "ORIGIN AND DESTINATION";

          dc << "  SALES ARE ONLY PERMITTED IN THE CURRENCY " << '\n';
          dc << "  OF THE COUNTRY OF " << country << '\n';
        }
        else if (!srr->curr().empty())
          dc << "  SALES ARE ONLY PERMITTED IN CURRENCY " << srr->curr() << '\n';
      }
      dc << '\n';
      // CARRIER RESTRICTIONS:
      bool anyRestriction = false;

      dc << " CARRIER RESTRICTIONS:" << '\n';
      dc << "  CARRIER CRS IND: " << srr->carrierCrsInd() <<'\n';
      dc << "  OTHER CARRIER  : " << srr->otherCarrier() <<'\n';
      dc << "  VALIDATION IND : " << srr->validationInd() <<'\n';

      CarrierCode cc = paxTypeFare.fare()->carrier();
      if (paxTypeFare.fare()->isIndustry())
        cc = paxTypeFare.fareMarket()->governingCarrier();

      if (srr->carrierCrsInd() != ' ')
      {
        if (srr->carrierCrsInd() == 'X')
        {
          dc << "  FARE MUST BE SOLD VIA CARRIER " << cc <<'\n';
          if (!srr->otherCarrier().empty() && srr->otherCarrier() != cc)
            dc << "  OR CARRIER " << srr->otherCarrier() << '\n';
          anyRestriction = true;
        }
        else if (srr->carrierCrsInd() == 'C') // mustBeSoldViaCRS)
        {
          dc << "  FARE MUST BE SOLD VIA CRS " << srr->otherCarrier() << '\n';
          anyRestriction = true;
        }
        if (!paxTypeFare.validatingCarriers().empty())
        {
          dc << "  VALIDATING CARRIERS: ";
          for (const CarrierCode& vcr : paxTypeFare.validatingCarriers())
          {
            dc << vcr << " ";
          }
          dc << " " << '\n';
        }
      }
      // ticketing restriction
      if (srr->validationInd() != ' ')
      {
        if (srr->validationInd() == RuleConst::VALIDATING_CXR_RESTR_EXCLUDE_PUBLISHING)
        {
          dc << "  TICKET MUST *NOT* BE VALIDATED ON CARRIER " << cc << '\n';
          dc << "  -TICKET MUST BE VALIDATED ON CARRIER " << srr->otherCarrier() << '\n';
        }
        else
        {
          dc << "  TICKET MUST BE VALIDATED ON CARRIER " << cc << '\n';
          if (!srr->otherCarrier().empty())
            dc << "  OR CARRIER " << srr->otherCarrier() << '\n';
        }
        anyRestriction = true;
      }

      // Segment
      if (srr->carrierSegInd() != ' ')
      {
        if (srr->carrierSegInd() == 'N') // noSegmentOfTicket)
        {
          dc << "  NO SEGMENT OF TICKET MAY BE ON ANY CARRIER EXCEPT" << '\n';
        }
        else
        {
          dc << "  NO SEGMENT AT THIS FARE MAY BE ON ANY CARRIER EXCEPT" << '\n';
        }
        dc << "  PUBLISHING CARRIER - OR OTHER CARRIER IF SPECIFIED ABOVE" << '\n';
        anyRestriction = true;
      }

      if (!anyRestriction)
      {
        dc << "  NOT APPLICABLE" << '\n';
      }
      dc << '\n';
      // Travel Agency Reservations

      anyRestriction = false;
      dc << " TRAVEL AGENCY RESTRICTIONS:" << '\n';
      // Sale
      if (srr->tvlAgentSaleInd() != ' ')
      {
        if (srr->tvlAgentSaleInd() == RuleConst::MAY_NOT_BE_SOLD_BY_TA) // mayNotBeSoldByTA)
        {
          dc << "  TICKET MAY NOT BE SOLD BY TRAVEL AGENCIES" << '\n';
        }
        else
        {
          dc << "  TICKET MAY ONLY BE SOLD BY TRAVEL AGENCIES" << '\n';
        }
        anyRestriction = true;
      }

      // Selected
      if (srr->tvlAgentSelectedInd() != ' ')
      {
        dc << "  TICKET MAY NOT BE SOLD BY ALL TRAVEL AGENCIES," << '\n';
        dc << "  BUT MAY BE SOLD BY SELECTED AGENCIES" << '\n';

        anyRestriction = true;
      }

      if (!anyRestriction)
      {
        dc << "  NOT APPLICABLE" << '\n';
      }
      dc << '\n';

      // ticket
      dc << " TICKET ISSUANCE METHODS:" << '\n';
      if (srr->tktIssMail() != ' ')
      {
        // mail
        string mail;
        switch (srr->tktIssMail())
        {
        case 'Y': // Allowed
        {
          mail = "ALLOWED";
          break;
        }
        case 'N': // Not allowed
        {
          mail = "NOT ALLOWED";
          break;
        }
        case 'R': // Required
        {
          mail = "REQUIRED";
          break;
        }
        default:
        {
          mail = "UNKNOWN";
          break;
        }
        }
        dc << "  MAIL -" << mail << '\n';
      }

      if (srr->tktIssPta() != ' ')
      {
        // pta
        string pta;
        switch (srr->tktIssPta())
        {
        case 'Y': // Allowed
        {
          pta = "ALLOWED";
          break;
        }
        case 'N': // Not allowed
        {
          pta = "NOT ALLOWED";
          break;
        }
        case 'R': // Required
        {
          pta = "REQUIRED";
          break;
        }
        default:
        {
          pta = "UNKNOWN";
          break;
        }
        }
        dc << "  PTA  -" << pta << '\n';
      }
      if (srr->tktIssMech() != ' ')
      {
        // Mechanical Means
        string mech;
        switch (srr->tktIssMech())
        {
        case 'Y': // Allowed
        {
          mech = "ALLOWED";
          break;
        }
        case 'N': // Not allowed
        {
          mech = "NOT ALLOWED";
          break;
        }
        case 'R': // Required
        {
          mech = "REQUIRED";
          break;
        }
        default:
        {
          mech = "UNKNOWN";
          break;
        }
        }
        dc << "  MECHANICAL MEANS -" << mech << '\n';
      }
      if (srr->tktIssSelf() != ' ')
      {
        // self
        string self;
        switch (srr->tktIssSelf())
        {
        case 'Y': // Allowed
        {
          self = "ALLOWED";
          break;
        }
        case 'N': // Not allowed
        {
          self = "NOT ALLOWED";
          break;
        }
        case 'R': // Required
        {
          self = "REQUIRED";
          break;
        }
        default:
        {
          self = "UNKNOWN";
          break;
        }
        }
        dc << "  SELF -" << self << '\n';
      }
      if (srr->tktIssPtaTkt() != ' ')
      {
        // pta/tkt
        string pt;
        switch (srr->tktIssPtaTkt())
        {
        case 'Y': // Allowed
        {
          pt = "ALLOWED";
          break;
        }
        case 'N': // Not allowed
        {
          pt = "NOT ALLOWED";
          break;
        }
        case 'R': // Required
        {
          pt = "REQUIRED";
          break;
        }
        default:
        {
          pt = "UNKNOWN";
          break;
        }
        }
        dc << "  PTA/TKT -" << pt << '\n';
      }
      if (srr->tktIssAuto() != ' ')
      {
        // ATM
        string atm;
        switch (srr->tktIssAuto())
        {
        case 'Y': // Allowed
        {
          atm = "ALLOWED";
          break;
        }
        case 'N': // Not allowed
        {
          atm = "NOT ALLOWED";
          break;
        }
        case 'R': // Required
        {
          atm = "REQUIRED";
          break;
        }
        default:
        {
          atm = "UNKNOWN";
          break;
        }
        }
        dc << "  AUTOMATIC TICKETING MACHINES -" << atm << '\n';
      }
      if (srr->tktIssSat() != ' ')
      {
        // SATELLITE TICKET PRINTER
        string sat;
        switch (srr->tktIssSat())
        {
        case 'Y': // Allowed
        {
          sat = "ALLOWED";
          break;
        }
        case 'N': // Not allowed
        {
          sat = "NOT ALLOWED";
          break;
        }
        case 'R': // Required
        {
          sat = "REQUIRED";
          break;
        }
        default:
        {
          sat = "UNKNOWN";
          break;
        }
        }
        dc << "  SATELLITE TICKET PRINTER -" << sat << '\n';
      }
      if (srr->tktIssSatOcAto() != ' ')
      {
        // SATO/CATO -
        string sato;
        switch (srr->tktIssSatOcAto())
        {
        case 'Y': // Allowed
        {
          sato = "ALLOWED";
          break;
        }
        case 'N': // Not allowed
        {
          sato = "NOT ALLOWED";
          break;
        }
        case 'R': // Required
        {
          sato = "REQUIRED";
          break;
        }
        default:
        {
          sato = "UNKNOWN";
          break;
        }
        }
        dc << "  SATO/CATO -" << sato << '\n';
      }
      if (srr->tktIssElectronic() != ' ')
      {
        // ELECTRONIC TICKETING -
        string et;
        switch (srr->tktIssElectronic())
        {
        case 'Y': // Allowed
        {
          et = "ALLOWED";
          break;
        }
        case 'N': // Not allowed
        {
          et = "NOT ALLOWED";
          break;
        }
        case 'R': // Required
        {
          et = "REQUIRED";
          break;
        }
        default:
        {
          et = "UNKNOWN";
          break;
        }
        }
        dc << "  ELECTRONIC TICKETING -" << et << '\n';
      }

      if (srr->tktIssSiti() != ' ')
      {
        string siti;
        // SITI -
        switch (srr->tktIssSiti())
        {
        case 'Y': // Allowed
        {
          siti = "ALLOWED";
          break;
        }
        case 'N': // Not allowed
        {
          siti = "NOT ALLOWED";
          break;
        }
        case 'R': // Required
        {
          siti = "REQUIRED";
          break;
        }
        default:
        {
          siti = "UNKNOWN";
          break;
        }
        }
        dc << "  SITI -" << siti << '\n';
      }
      if (srr->tktIssSoto() != ' ')
      {
        string soto;
        // SOTO -
        switch (srr->tktIssSoto())
        {
        case 'Y': // Allowed
        {
          soto = "ALLOWED";
          break;
        }
        case 'N': // Not allowed
        {
          soto = "NOT ALLOWED";
          break;
        }
        case 'R': // Required
        {
          soto = "REQUIRED";
          break;
        }
        default:
        {
          soto = "UNKNOWN";
          break;
        }
        }
        dc << "  SOTO -" << soto << '\n';
      }
      if (srr->tktIssSito() != ' ')
      {
        string sito;
        // SITO -
        switch (srr->tktIssSito())
        {
        case 'Y': // Allowed
        {
          sito = "ALLOWED";
          break;
        }
        case 'N': // Not allowed
        {
          sito = "NOT ALLOWED";
          break;
        }
        case 'R': // Required
        {
          sito = "REQUIRED";
          break;
        }
        default:
        {
          sito = "UNKNOWN";
          break;
        }
        }
        dc << "  SITO -" << sito << '\n';
      }
      if (srr->tktIssSoti() != ' ')
      {
        string soti;
        // SOTI -
        switch (srr->tktIssSoti())
        {
        case 'Y': // Allowed
        {
          soti = "ALLOWED";
          break;
        }
        case 'N': // Not allowed
        {
          soti = "NOT ALLOWED";
          break;
        }
        case 'R': // Required
        {
          soti = "REQUIRED";
          break;
        }
        default:
        {
          soti = "UNKNOWN";
          break;
        }
        }
        dc << "  SOTI -" << soti << '\n';
      }
      dc << '\n';
      displayLocales(srr);
    }
  }

private:
  void writeHeader(const PaxTypeFare& paxTypeFare);
  void displayLocales(const SalesRestriction* sr);
};

template <class T, class SalesRestrictionRuleT>
void runDiag315(const SalesRestrictionRuleT& salesRestrictionRule,
                PricingTrx& trx,
                const PaxTypeFare& paxTypeFare,
                const T& cri,
                const typename T::item_info_type* rule,
                const SalesRestriction* salesRestriction,
                const Record3ReturnTypes retCode,
                const typename SalesRestrictionRuleT::Cat15FailReasons failReason,
                const bool skipCat15SecurityCheck)
{
  const std::string& diagFareClass = trx.diagnostic().diagParamMapItem(Diagnostic::FARE_CLASS_CODE);
  if (diagFareClass.empty() ||
      RuleUtil::matchFareClass(diagFareClass.c_str(), paxTypeFare.fareClass().c_str()))
  {
    DCFactory* factory = DCFactory::instance();
    Diag315Collector* diag = dynamic_cast<Diag315Collector*>(factory->create(trx));
    diag->enable(Diagnostic315);

    if (DiagnosticUtil::isvalidForCarrierDiagReq(trx, paxTypeFare) ||
        (paxTypeFare.fare()->cat15HasSecurity() && retCode != FAIL && !skipCat15SecurityCheck))
    {
      diag->diag315Collector<T, SalesRestrictionRuleT>(paxTypeFare, cri, rule, salesRestriction, retCode, failReason);
    }

    if (trx.isValidatingCxrGsaApplicable() && salesRestrictionRule.ruleDataAccess() && !salesRestrictionRule.ruleDataAccess()->validatingCxr().empty())
      *diag << " GSA VALIDATING CARRIER: " << salesRestrictionRule.ruleDataAccess()->validatingCxr() << "\n";

    diag->flushMsg();
  }
}

} // namespace tse

