//----------------------------------------------------------------------------
//  File:        Diag853Collector.C
//  Authors:
//  Created:
//
//  Description: Diagnostic 853 to display content of FareCalcConfig data
//
//  Updates:
//          05/06/2004  BT - create.
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

#include "Diagnostic/Diag853Collector.h"

#include "Common/FareCalcUtil.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/FareCalcConfig.h"

#include <iomanip>

using namespace std;

namespace tse
{
void
Diag853Collector::writeFareCalcConfigHeader()
{
  ((DiagCollector&)*this) << "**************  FARE-CALC-CONFIG  ****************" << '\n';
}

void
Diag853Collector::writeFareCalcConfigSegHeader()
{
  ((DiagCollector&)*this) << "*************  FARE-CALC-CONFIG-SEG **************" << '\n';
}

void
Diag853Collector::writeFooter()
{
  ((DiagCollector&)*this) << "*********   END-OF-FARE-CALC-DISPLAY   ***********" << '\n';
}

//----------------------------------------------------------------------------
//
// @function void  Diag853Collector::process
//
// Description:    display FareCalcConfig data from database
//
// @param   trx    PricingTrx
// @return  void
//
//----------------------------------------------------------------------------
void
Diag853Collector::process(const PricingTrx& trx, const FareCalcConfig*& fareCalcConfig)
{
  if (_active)
  {
    writeFareCalcConfigHeader();
    if (buildFareCalcConfigDisplay(trx, fareCalcConfig))
    {
      writeFareCalcConfigSegHeader();
      buildFareCalcConfigSegDisplay(fareCalcConfig);
    }
    writeFooter();
  }
}

bool
Diag853Collector::buildFareCalcConfigDisplay(const PricingTrx& trx,
                                             const FareCalcConfig*& fareCalcConfig)
{
  std::string applMsg[FareCalcConfigText::MAX_TEXT_APPL];
  for (int i = 1; i < FareCalcConfigText::MAX_TEXT_APPL; i++)
  {
    FareCalcUtil::getMsgAppl(
        static_cast<FareCalcConfigText::TextAppl>(i), applMsg[i], const_cast<PricingTrx&>(trx));
  }

  DiagCollector& dc(*this);
  dc.setf(std::ios::right, std::ios::adjustfield);
  if (fareCalcConfig != nullptr)
  {
    dc << "USER-APPL-TYPE:       " << setw(5) << fareCalcConfig->userApplType()
       << "  FC-CONNECTION-IND:          " << fareCalcConfig->fcConnectionInd() << '\n'
       << "USER-APPL:            " << setw(5) << fareCalcConfig->userAppl()
       << "  FC-PSG-TYP-DISPLAY:         " << fareCalcConfig->fcPsgTypDisplay() << '\n'
       << "PSEUDO-CITY:          " << setw(5) << fareCalcConfig->pseudoCity()
       << "  FARE-TAX-TOTAL-IND:         " << fareCalcConfig->fareTaxTotalInd() << '\n'
       << "LOC1TYPE:             " << setw(5) << fareCalcConfig->loc1().locType()
       << "  NO-OF-TAX-BOXES:            " << fareCalcConfig->noofTaxBoxes() << '\n'
       << "LOC1:                 " << setw(5) << fareCalcConfig->loc1().loc()
       << "  BASE-TAX-EQUIV-LENGTH:      " << fareCalcConfig->baseTaxEquivTotalLength() << '\n'
       << "ITIN-DISPLAY-IND:     " << setw(5) << fareCalcConfig->itinDisplayInd()
       << "  TAX-PLACEMENT-IND:          " << fareCalcConfig->taxPlacementInd() << '\n'
       << "ITIN-HEADER-TEXT:     " << setw(5) << fareCalcConfig->itinHeaderTextInd()
       << "  TAX-CUR-CODE-DISPLAY-IND:   " << fareCalcConfig->taxCurCodeDisplayInd() << '\n'
       << "TRUE-PSGR-TYPE-IND:   " << setw(5) << fareCalcConfig->truePsgrTypeInd()
       << "  TAX-EXEMPTION-IND:          " << fareCalcConfig->taxExemptionInd() << '\n'
       << "WP-PSG-DISPLAY  :     " << setw(5) << fareCalcConfig->wpPsgTypDisplay()
       << "  TAX-EXEMPT-BREAKDOWN-IND:   " << fareCalcConfig->taxExemptBreakdownInd() << '\n'
       << "WP-CONNECTION-IND:    " << setw(5) << fareCalcConfig->wpConnectionInd()
       << "  ZP-AMOUNT-DISPLAY-IND:      " << fareCalcConfig->zpAmountDisplayInd() << '\n'
       << "GLOBAL-SIDE-TRIP-IND: " << setw(5) << fareCalcConfig->globalSidetripInd()
       << "  TVL-COMENCEMENT-DATE:       " << fareCalcConfig->tvlCommencementDate() << '\n'
       << "WRAP-AROUND:          " << setw(5) << fareCalcConfig->wrapAround()
       << "  LAST-DAY-TICKET-OUTPUT:     " << fareCalcConfig->lastDayTicketOutput() << '\n'
       << "DOMESTIC-NUC:         " << setw(5) << fareCalcConfig->domesticNUC()
       << "  LAST-DAY-TICKET-DISPLAY:    " << fareCalcConfig->lastDayTicketDisplay() << '\n'
       << "DOMESTIC-ISI:         " << setw(5) << fareCalcConfig->domesticISI()
       << "  MULTI-SURCHARGE-SPACING:    " << fareCalcConfig->multiSurchargeSpacing() << '\n'
       << "INTERNATIONAL-ISI:    " << setw(5) << fareCalcConfig->internationalISI()
       << "  FARE-BASIS-TKT-DESIG:       " << fareCalcConfig->fareBasisTktDesLng() << '\n'
       << "DISPLAY-BSR:          " << setw(5) << fareCalcConfig->displayBSR()
       << "  FARE-BASIS-DISPLAY-OPTION:  " << fareCalcConfig->fareBasisDisplayOption() << '\n'
       << "ENDORSEMENTS:         " << setw(5) << fareCalcConfig->endorsements()
       << "  WP-CHILD-INFANT-FARE-BASIS: " << fareCalcConfig->wpChildInfantFareBasis() << '\n'
       << "WARNING-MESSAGES:     " << setw(5) << fareCalcConfig->warningMessages()
       << "  FC-CHILD-INFANT-FARE-BASIS: " << fareCalcConfig->fcChildInfantFareBasis() << '\n'
       << "RESERVATION-OVERRIDE: " << setw(5) << fareCalcConfig->reservationOverride()
       << "  INTERLINE-TKT-PERMITTED:    " << fareCalcConfig->interlineTktPermitted() << '\n'
       << "IET PRICING ACTIVE:   " << setw(5) << fareCalcConfig->ietPriceInterlineActive()
       << "  VALUE-CODE-BASE:            " << fareCalcConfig->valueCodeBase() << '\n'
       << "WP-WPA-TRAILER-MSG:   " << setw(5) << fareCalcConfig->wpWpaTrailerMsg()
       << "  PARTICIPATING-AGREEMENT:    " << fareCalcConfig->participatingAgreement() << '\n'
       << "VAL-CXR-DISPLAY-OPT:  " << setw(5) << fareCalcConfig->valCxrDisplayOpt()
       << "  APPLY DOM MULTI-CURRENCY:   " << fareCalcConfig->applyDomesticMultiCurrency() << '\n'
       << "NEG-PERMITTED:        " << setw(5) << fareCalcConfig->negPermitted()
       << "  APPLY INTL MULTI-CURRENCY:  " << fareCalcConfig->applyIntlMultiCurrency() << '\n'
       << "SKIP-AVAIL-CHECK:     " << setw(5) << fareCalcConfig->noMatchAvail() << '\n';

    dc << " \n";

    if (fareCalcConfig->wpaPermitted() == 'Y')
    {
      dc << "WPA-PERMITTED:          " << setw(3) << fareCalcConfig->wpaPermitted()
         << "  WPA-RO-IND:                 " << fareCalcConfig->wpaRoInd() << '\n'
         << "WPA-SORT:               " << setw(3) << fareCalcConfig->wpaSort()
         << "  WPA-NO-MATCH-HI-CABIN-FARE: " << fareCalcConfig->wpaNoMatchHigherCabinFare() << '\n'
         << "WPA-PSG-DTL-FORMAT:     " << setw(3) << fareCalcConfig->wpaPsgDtlFormat()
         << "  WPA-STORE-WITHOUT-REBOOK:   " << fareCalcConfig->wpaStoreWithoutRebook() << '\n'
         << "WPA-FARE-LINE-PSG-TYPE: " << setw(3) << fareCalcConfig->wpaFareLinePsgType()
         << "  WPA-PSG-MULTI-LINE-BREAK:   " << fareCalcConfig->wpaPsgMultiLineBreak() << '\n'
         << "WPA-FARE-LINE-HDR:      " << setw(3) << fareCalcConfig->wpaFareLineHdr()
         << "  WPA-ACC-TVL-CAT13:          " << fareCalcConfig->wpaAccTvlCat13() << '\n'
         << "WPA-PRIME-PSG-REF-NO:   " << setw(3) << fareCalcConfig->wpaPrimePsgRefNo()
         << "  WPA-ACC-TVL-OPTION:         " << fareCalcConfig->wpaAccTvlOption() << '\n'
         << "WPA-2ND-PSG-REF-NO:     " << setw(3) << fareCalcConfig->wpa2ndPsgRefNo()
         << "  WPA-FARE-OPTION-MAX-NO:     " << fareCalcConfig->wpaFareOptionMaxNo() << '\n'
         << "WPA-SHOW-DUP-AMOUNTS:   " << setw(3) << fareCalcConfig->wpaShowDupAmounts()
         << "  WPA-PSG-LINE-BREAK:         " << fareCalcConfig->wpaPsgLineBreak() << '\n';

      dc << " \n";
    }

    dc << "WP-NO-MATCH-PERMITTED:  " << setw(3) << fareCalcConfig->wpNoMatchPermitted()
       << "  WP-RO-IND:                  " << fareCalcConfig->wpRoInd() << '\n'
       << "WP-SORT:                " << setw(3) << fareCalcConfig->wpSort()
       << "  WP-NO-MATCH-HI-CABIN-FARE:  " << fareCalcConfig->wpNoMatchHigherCabinFare() << '\n'
       << "WP-PSG-DTL-FORMAT:      " << setw(3) << fareCalcConfig->wpPsgDtlFormat()
       << "  WP-STORE-WITHOUT-REBOOK:    " << fareCalcConfig->wpStoreWithoutRebook() << '\n'
       << "WP-FARE-LINE-PSG-TYPE:  " << setw(3) << fareCalcConfig->wpFareLinePsgType()
       << "  WP-PSG-MULTI-LINE-BREAK:    " << fareCalcConfig->wpPsgMultiLineBreak() << '\n'
       << "WP-FARE-LINE-HDR:       " << setw(3) << fareCalcConfig->wpFareLineHdr()
       << "  WP-ACC-TVL-CAT13:           " << fareCalcConfig->wpAccTvlCat13() << '\n'
       << "WP-PRIME-PSG-REF-NO:    " << setw(3) << fareCalcConfig->wpPrimePsgRefNo()
       << "  WP-ACC-TVL-OPTION:          " << fareCalcConfig->wpAccTvlOption() << '\n'
       << "WP-2ND-PSG-REF-NO:      " << setw(3) << fareCalcConfig->wp2ndPsgRefNo()
       << "  WP-FARE-OPTION-MAX-NO:      " << fareCalcConfig->wpFareOptionMaxNo() << '\n'
       << "WP-SHOW-DUP-AMOUNTS:    " << setw(3) << fareCalcConfig->wpShowDupAmounts()
       << "  WP-PSG-LINE-BREAK:          " << fareCalcConfig->wpPsgLineBreak() << '\n';

    dc << " \n";

    dc << "************ TEXT MESSAGE APPLICATION ************" << '\n';
    if (fareCalcConfig->wpaPermitted() == 'Y')
    {
      dc << "WPA-RO-IND:\n    " << applMsg[FareCalcConfigText::WPA_RO_INDICATOR] << '\n'
         << "WPA-NO-MATCH-NO-FARES:\n    " << applMsg[FareCalcConfigText::WPA_NO_MATCH_NO_FARE]
         << '\n' << "WPA-NO-MATCH-REBOOK:\n    " << applMsg[FareCalcConfigText::WPA_NO_MATCH_REBOOK]
         << '\n' << "WPA-NO-MATCH-VERIFY-BKG-CLASS:\n    "
         << applMsg[FareCalcConfigText::WPA_NO_MATCH_VERIFY_BOOKING_CLASS] << '\n'
         << "WPA-NO-MATCH-BOOKING-CLASS:\n    "
         << applMsg[FareCalcConfigText::WPA_NO_MATCH_BOOKING_CLASS] << '\n' << " \n";
    }

    dc << "WP-RO-IND:\n    " << applMsg[FareCalcConfigText::WP_RO_INDICATOR] << '\n'
       << "WP-NO-MATCH-NO-FARES:\n    " << applMsg[FareCalcConfigText::WP_NO_MATCH_NO_FARE] << '\n'
       << "WP-NO-MATCH-REBOOK:\n    " << applMsg[FareCalcConfigText::WP_NO_MATCH_REBOOK] << '\n'
       << "WP-NO-MATCH-VERIFY-BKG-CLASS:\n    "
       << applMsg[FareCalcConfigText::WP_NO_MATCH_VERIFY_BOOKING_CLASS] << '\n'
       << "WP-NO-MATCH-BOOKING-CLASS:\n    "
       << applMsg[FareCalcConfigText::WP_NO_MATCH_BOOKING_CLASS] << '\n';

    dc << " \n";

    return (true);
  }
  else
  {
    dc << "NO DATA AVAILABLE\n";
    return (false);
  }
}

void
Diag853Collector::buildFareCalcConfigSegDisplay(const FareCalcConfig*& fareCalcConfig)
{
  DiagCollector& dc(*this);

  const std::vector<FareCalcConfigSeg*>& fcSegs = fareCalcConfig->segs();
  std::vector<FareCalcConfigSeg*>::const_iterator iter = fcSegs.begin();

  if (iter != fcSegs.end())
  {
    dc.setf(std::ios::right, std::ios::adjustfield);
    for (; iter != fcSegs.end(); iter++)
    {
      dc << " \n";
      dc << "USER-APPL-TYPE: " << setw(5) << fareCalcConfig->userApplType()
         << "  USER-APPL:      " << setw(5) << fareCalcConfig->userAppl() << '\n'
         << "PSEUDO-CITY:    " << setw(5) << fareCalcConfig->pseudoCity()
         << "  MARKET-LOC:     " << setw(5) << (*iter)->marketLoc() << '\n'
         << "LOC1TYPE:       " << setw(5) << fareCalcConfig->loc1().locType()
         << "  DISPLAY-LOC:    " << setw(5) << (*iter)->displayLoc() << '\n'
         << "LOC1:           " << setw(5) << fareCalcConfig->loc1().loc() << '\n';
    }
    dc << " \n";
  }
  else
  {
    dc << "NO DATA AVAILABLE\n";
  }
}
}
