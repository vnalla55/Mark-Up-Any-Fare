//----------------------------------------------------------------------------
//  File:        Diag270Collector.C
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//  Created:     2004-05-20
//
//  Description: Diagnostic 270 formatter
//
//  Updates:
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

#include "Diagnostic/Diag270Collector.h"

#include "Common/FareMarketUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/Loc.h"

#include <iomanip>

namespace tse
{
Diag270Collector& Diag270Collector::operator<<(const FareMarket& fareMarket)
{
  if (_active)
  {
    DiagCollector& dc(*this);

    if (fareMarket.travelSeg().size() == 0)
      return *this;

    std::vector<TravelSeg*>::const_iterator tvlSegItr;
    tvlSegItr = fareMarket.travelSeg().begin();

    dc << " " << std::endl;

    dc << FareMarketUtil::getFullDisplayString(fareMarket) << std::endl;

    dc << " " << std::endl;

    const CarrierPreference* pCarrierPref = fareMarket.governingCarrierPref();

    if (pCarrierPref == nullptr)
    {
      dc << "CARRIER PREFERENCE DIAGNOSTIC - DATA NOT FOUND" << std::endl;
      dc << "GOVERNING CARRIER : " << fareMarket.governingCarrier() << std::endl;
    }
    else
    {

      dc << "CARRIER PREFERENCE DIAGNOSTIC" << std::endl;

      dc << "\nGOVERNING CARRIER                          : ";

      if (pCarrierPref->carrier() == "  " || pCarrierPref->carrier().empty())
      {
        dc << "DEFAULT";
      }
      else
      {
        dc << pCarrierPref->carrier();
      }
      //    dc << "\nFIRST TRAVEL DATE                          : "
      //       << pCarrierPref->firstTvlDate().dateToUsString();
      //    dc << "\nLAST TRAVEL DATE                           : "
      //       << pCarrierPref->lastTvlDate().dateToUsString();
      dc << "\nOVERRIDE BY SECTOR BAGGAGE                 : "
         << pCarrierPref->ovrideFreeBagFareCompLogic();
      dc << "\nBAG-FREE BAGGAGE EXEMPT                    : " << pCarrierPref->freebaggageexempt();
      dc << "\nBAG-FREE BAGGAGE EXEMPT TC13               : "
         << pCarrierPref->noApplyBagExceptOnCombArea1_3();
      dc << "\nAVL-APPLY RULE 2                           : "
         << pCarrierPref->availabilityApplyrul2st();
      dc << "\nAVL-APPLY RULE 3                           : "
         << pCarrierPref->availabilityApplyrul3st();
      dc << "\nMIN-DO NOT APPLY OSC                       : " << pCarrierPref->bypassosc();
      dc << "\nMIN-DO NOT APPLY RSC                       : " << pCarrierPref->bypassrsc();
      dc << "\nCUR-APPLY SAME NUC TO RT FARE              : " << pCarrierPref->applysamenuctort();

      if (!TrxUtil::isFullMapRoutingActivated(*static_cast<PricingTrx*>(trx())))
      {
        dc << "\nRTG-TERMINAL ON/OFF POINTS ONLY            : "
           << pCarrierPref->applyrtevaltoterminalpt();
      }

      dc << "\nRTG-DO NOT APPLY DRV ON NON-US CITIES      : " << pCarrierPref->noApplydrvexceptus();
      dc << "\nRUL-STOPOVER-LEAST RESTRICTIVE PROVISION   : "
         << pCarrierPref->applyleastRestrStOptopu();
      dc << "\nRUL-TRANSFER-LEAST RESTRICTIVE PROVISION   : "
         << pCarrierPref->applyleastRestrtrnsftopu();
      dc << "\nCOM-DO NOT APPLY COMB OF TAG1 AND TAG3     : "
         << pCarrierPref->noApplycombtag1and3();
      dc << "\nADD-APPLY SINGLE ADD-ON OVER DOUBLE ADD-ON : "
         << pCarrierPref->applysingleaddonconstr();
      dc << "\nADD-APPLY SPECIFIED OVER CONSTRUCTED FARE  : " << pCarrierPref->applyspecoveraddon();
      dc << "\nCUR-DO NOT APPLY NIGERIA CURR ADJ          : "
         << pCarrierPref->noApplynigeriaCuradj();
      dc << "\nCOM-NO SURFACE AT FARE BREAK               : "
         << pCarrierPref->noSurfaceAtFareBreak();
      dc << "\nGEO-CARRIER BASE NATION                    : " << pCarrierPref->carrierbasenation();
      dc << "\nAPPLY-PREM-BUS-CABIN-DIFF-CALC             : "
         << pCarrierPref->applyPremBusCabinDiffCalc();
      dc << "\nAPPLY-PREM-ECO-CABIN-DIFF-CALC             : "
         << pCarrierPref->applyPremEconCabinDiffCalc();
      dc << "\nNO-APPLY-NON-PREMIUM-CABIN-FARE            : "
         << pCarrierPref->noApplySlideToNonPremium();
      dc << "\nAPPLY-NORMAL-FARE-OJ-IN-DIFF-CNTRYS        : "
         << pCarrierPref->applyNormalFareOJInDiffCntrys();
      dc << "\nAPPLY-SINGLE-TOJ-BETW-AREAS-SHORTER-FC     : "
         << pCarrierPref->applySingleTOJBetwAreasShorterFC();
      dc << "\nAPPLY-SINGLE-TOJ-BETW-AREAS-LONGER-FC      : "
         << pCarrierPref->applySingleTOJBetwAreasLongerFC();
      dc << "\nAPPLY-SPCL-DOJ-EUROPE                      : " << pCarrierPref->applySpclDOJEurope();
      dc << "\nAPPLY-HIGHEST RT OPEN JAW CHECK            : " << pCarrierPref->applyHigherRTOJ();
      dc << "\nAPPLY-HIGHEST RT CHECK                     : " << pCarrierPref->applyHigherRT();
      dc << "\nDISPLAY-FBC                                : " << pCarrierPref->applyFBCinFC();

      dc << "\nDESCRIPTION : " << pCarrierPref->description() << std::endl;

      dc << "\nPERMITTED FBR VENDOR                       : ";
      const std::vector<VendorCode>& vendorCodes = pCarrierPref->fbrPrefs();
      for (unsigned int i = 0; i < vendorCodes.size(); i++)
      {
        if ((i % 4 == 0) && (i != 0))
        {
          dc << "\n                                             ";
          dc << vendorCodes[i] << " ";
        }
        else
        {
          dc << vendorCodes[i] << " ";
        }
      }

      dc << "\nACTIVATE SOLO PRICING                      : " << pCarrierPref->activateSoloPricing()
         << std::endl;

      dc << "\nACTIVATE SOLO SHOPPING                     : "
         << pCarrierPref->activateSoloShopping() << std::endl;

      dc << "\nACTIVATE JOURNEY PRICING                   : "
         << pCarrierPref->activateJourneyPricing() << std::endl;

      dc << "\nACTIVATE JOURNEY SHOPPING                  : "
         << pCarrierPref->activateJourneyShopping() << std::endl;

      dc << "\nAPPLY US2 TAX ON FREE TICKET               : "
         << pCarrierPref->applyUS2TaxOnFreeTkt() << std::endl;

      dc << "\nFLOW MARKET JOURNEY TYPE                   : " << pCarrierPref->flowMktJourneyType()
         << std::endl;

      dc << "\nLOCAL MARKET JOURNEY TYPE                  : " << pCarrierPref->localMktJourneyType()
         << std::endl;
    }
  }

  return *this;
}
}
