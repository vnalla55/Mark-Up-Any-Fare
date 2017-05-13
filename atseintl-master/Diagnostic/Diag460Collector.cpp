//----------------------------------------------------------------------------
//  File:        Diag460Collector.C
//  Authors:     LeAnn Perez
//  Created:     Aug 2004
//
//  Description: Display paxTypeFares and results of routing validation
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

#include "Diagnostic/Diag460Collector.h"

#include "Common/Money.h"
#include "Common/RoutingUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Routing.h"
#include "Diagnostic/RoutingDiagCollector.h"


#include <iomanip>

namespace tse
{
void
Diag460Collector::displayPaxTypeFares(PricingTrx& trx,
                                      FareMarket& fareMarket,
                                      const TravelRoute& tvlRoute)
{
  if (!_active)
  {
    return;
  }

  std::map<std::string, std::string>::const_iterator e = trx.diagnostic().diagParamMap().end();
  std::map<std::string, std::string>::const_iterator i =
      trx.diagnostic().diagParamMap().find(Diagnostic::FARE_MARKET);
  if (i != e)
  {
    LocCode boardCity = i->second.substr(0, 3);
    LocCode offCity = i->second.substr(3, 3);
    if ((fareMarket.origin()->loc() != boardCity && fareMarket.boardMultiCity() != boardCity) ||
        (fareMarket.destination()->loc() != offCity && fareMarket.offMultiCity() != offCity))
    {
      return;
    }
  }

  Diag460Collector& diag = dynamic_cast<Diag460Collector&>(*this);

  FareClassCode fareClass, fareBasis;
  i = trx.diagnostic().diagParamMap().find(Diagnostic::FARE_CLASS_CODE);
  if (i != e)
    fareClass = i->second;
  i = trx.diagnostic().diagParamMap().find(Diagnostic::FARE_BASIS_CODE);
  if (i != e)
    fareBasis = i->second;
  PaxTypeFareFilter filter(fareClass, fareBasis);
  diag.displayRoutingStatus(fareMarket, tvlRoute, filter);
}

//----------------------------------------------------------------------------
// buildHeader
//----------------------------------------------------------------------------
void
Diag460Collector::displayHeader()
{
  if (_active)
  {
    ((DiagCollector&)*this) << "\nFARES VALIDATED BY ROUTINGS IN FARE MARKET                  \n"
                            << "--------------------------------------------------------------\n"
                            << " \n"
                            << " FARE BASIS  V T  RTG  RTG  RTG O O      AMT CUR PAX  PASS SUR\n"
                            << "             N P  NUM TRF1 TRF2 R I              TPE  FAIL    \n"
                            << "------------ - - ---- ---- ---- - - -------- --- --- ---------\n";
    //  KE7NR        A R    5   99      R O   500.00 USD ADT P     25M
  }
}

//----------------------------------------------------------------------------
// Display PaxTypeFares with Routing Results
//----------------------------------------------------------------------------
void
Diag460Collector::displayRoutingStatus(FareMarket& fareMarket,
                                       const TravelRoute& tvlRoute,
                                       const PaxTypeFareFilter& filter)

{
  if (_active)
  {
    Diag460Collector& dc = dynamic_cast<Diag460Collector&>(*this);

    std::vector<PaxTypeFare*> filteredPaxTypeFares;
    std::remove_copy_if(fareMarket.allPaxTypeFare().begin(),
                        fareMarket.allPaxTypeFare().end(),
                        std::back_inserter(filteredPaxTypeFares),
                        std::not1(filter));

    // Iterate through paxTypeFareVector
    // if((fareMarket.allPaxTypeFare().empty()))
    if (filteredPaxTypeFares.empty())
    {
      dc.displayHeader();
      dc.displayNoFaresMessage();
    }
    else
    {
      int displayException = 0;
      // dc.displayTravelRoute(tvlRoute);
      dc.displayCityCarriers(tvlRoute.govCxr(), tvlRoute.travelRoute());
      dc.displayHeader();
      // std::vector<PaxTypeFare*>::iterator itr = fareMarket.allPaxTypeFare().begin();
      std::vector<PaxTypeFare*>::iterator itr = filteredPaxTypeFares.begin();

      // for(; itr != fareMarket.allPaxTypeFare().end(); itr++)
      for (; itr != filteredPaxTypeFares.end(); itr++)
      {
        PaxTypeFare& paxTypeFare = **itr;
        PricingTrx& pricingTrx = static_cast<PricingTrx&>(*_trx);

        const Routing* routing =
            RoutingUtil::getRoutingData(pricingTrx, paxTypeFare, paxTypeFare.routingNumber());

        if (TrxUtil::isFullMapRoutingActivated(*static_cast<PricingTrx*>(_trx)))
        {
          if (RoutingUtil::isTicketedPointOnly(routing, tvlRoute.flightTrackingCxr()))
          {
            if (tvlRoute.unticketedPointInd() == TKTPTS_ANY)
            {
              continue;
            }
          }
          else if (tvlRoute.unticketedPointInd() == TKTPTS_TKTONLY)
          {
            continue;
          }
        }

        if ((*itr)->isRoutingProcessed())
        {
          dc.setf(std::ios::left, std::ios::adjustfield);
          // dc << std::setw( 13) << (*itr)->fareClass()
          dc << std::setw(13) << (*itr)->createFareBasis(nullptr) << std::setw(2)
             << ((*itr)->vendor() == Vendor::ATPCO
                     ? "A"
                     : ((*itr)->vendor() == Vendor::SITA ? "S" : "?"));

          // Set Type of Routing:  Routing or Mileage
          if ((*itr)->isRouting())
          {
            dc << "R ";
          }
          else
          {
            dc << "M ";
          }

          dc << std::setw(4) << (*itr)->routingNumber() << " ";
          dc.setf(std::ios::right, std::ios::adjustfield);

          if ((*itr)->tcrRoutingTariff1() < 0)
          {
            dc << std::setw(4) << "    ";
          }
          else
          {
            dc << std::setw(4) << (*itr)->tcrRoutingTariff1() << " ";
          }

          dc.setf(std::ios::right, std::ios::adjustfield);
          if ((*itr)->tcrRoutingTariff2() < 0)
          {
            dc << "    ";
          }
          else
          {
            dc << std::setw(4) << (*itr)->tcrRoutingTariff2();
          }

          dc << std::setw(2) << ((*itr)->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED ? "R" : "O");

          dc << std::setw(2);
          if ((*itr)->directionality() == FROM)
            dc << "O";

          else if ((*itr)->directionality() == TO)
            dc << "I";

          else
            dc << " ";

          dc << std::setw(9) << Money((*itr)->fareAmount(), (*itr)->currency());
          dc << std::setw(4) << (*itr)->fcasPaxType();

          // Display Routing Status
          if ((*itr)->isRoutingValid())
          {
            dc << "  P ";
          }
          else
          {
            dc << "  F ";
          }
          if ((*itr)->mileageSurchargePctg() > 25)
          {
            dc << std::setw(4) << (*itr)->mileageSurchargePctg() << "EX";
            if (!((*itr)->surchargeExceptionApplies()))
              dc << "C";
          }
          else
          {
            dc << std::setw(4) << (*itr)->mileageSurchargePctg() << "M";
          }

          if ((*itr)->surchargeExceptionApplies())
          {
            dc << "*";
            if (!displayException)
              displayException++;
          }

          dc << '\n';
        }
      }
      dc << " \n";
      if (displayException)
        dc << "\n* SURCHARGE EXCEPTION APPLIED" << std::endl;
    }
  }
}
}
