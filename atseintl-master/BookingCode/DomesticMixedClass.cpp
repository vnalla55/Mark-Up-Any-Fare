//----------------------------------------------------------------------------
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
#include "BookingCode/DomesticMixedClass.h"

#include "Common/Logger.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/CarrierMixedClass.h"
#include "Diagnostic/Diag411Collector.h"
#include "Diagnostic/DiagCollector.h"
#include "Util/BranchPrediction.h"

#include <ctime>
#include <iostream>
#include <string>
#include <vector>

namespace tse
{
static Logger
logger("atseintl.BookingCode.DomesticMixedClass");

/**
 * Performs all validations for mixed booking codes within FareMarket.
 * This class determines the validity of the mixed booking codes for the through fare
 * only for domestic US/CA fare component.
 */
// validate
/**
 * This method is called to conduct the validation on the mixed booking codes on the FareMarket.
 * It will gets the MixedClassInfo data from the Cache. Analyze the booking code hierarchy.
 * The result could be pass or fail.
 *
 * @param  trx       PricingTrx
 * @param  paxTfare  PaxTypeFare
 * @param  mkt       FareMarket
 * @param  bkg       Booking code
 * @param  diag      DiagCollector
 *
 * @return true if fare pass the validation, false otherwise
 *
 * @todo Main Flow of Events
 -#  gets MixedClassTableInfo data for the marketing carrier from the Cache
   -#  Loop through all flights gathering data and checking if carrier is the same on all travel
 segments.
       The loop is broken and the validation return FAIL if some other is found or if no hierarhy is
 found
       for any booking code examined.
       Also a vector of night classes are collected on this loop for futher investigation if
 necessary.

 @see  BkgCodeValidator.h for complete processing flow
 */

bool
DomesticMixedClass::validate(
    PricingTrx& trx, PaxTypeFare& paxTfare, FareMarket& mkt, BookingCode& bkg, DiagCollector* diag)
{
  LOG4CXX_DEBUG(logger, "Entered DomesticMixedClass::validate()");

  bool mixClassOK = true; // Validation status
  Indicator lowestHierarchy = '9'; // Lowest found hierarchy
  Indicator foundBkgHierarchy = '0'; // Hierarchy for the bkg is found
  Indicator previousHierarchy = '0'; // Hierarchy for the previous bkg code

  BookingCode singleLetterBkgCode; // For Night Class checking
  BookingCode previousBkgCode; // Previous Bkg in the trip
  CarrierCode marketingCxr; // Carrier testing

  Indicator currentMixHierarchy = '0'; // default
  BookingCode currentBkgCode;

  // Travel segment related data  need to display all fare for particular TravelSegment
  TravelSegPtrVecIC itTvl = mkt.travelSeg().begin();

  // initialize Booking Code locations
  previousBkgCode.clear();
  currentBkgCode.clear();

  // Find out the first flight on the FareMarket and retrive the Carrier

  while (!(*itTvl)->isAir())
  {
    ++itTvl;
  }

  const AirSeg* airSeg = static_cast<const AirSeg*>(*itTvl);

  marketingCxr = airSeg->carrier();
  const DateTime& travelDate = trx.adjustedTravelDate(airSeg->departureDT());
  const std::vector<CarrierMixedClass*>& mixedClassList =
      trx.dataHandle().getCarrierMixedClass(marketingCxr, travelDate);

  if (mixedClassList.size() == 0)
  {
    if (UNLIKELY(diagPresent(diag)))
    {
      diag->printMessages(25); // "MIXED CLASS DATA NOT FOUND FOR CXR - '"
      *diag << marketingCxr;
    }

    mixClassOK = false;
    LOG4CXX_DEBUG(
        logger, "Leaving DomesticMixedClass::validate(): " << (mixClassOK ? "success" : "failure"));
    return mixClassOK;
  }

  // Need to loop through all Travel segment and checking if the carrier is the same on all flights
  // The loop is broken and validation return -MIXED CLASS FAIL - if some other carrier is found
  // or if no hierarchy is found for any booking code examinated
  // A vector of night classes are collected on this loop for futher investigation if necessa

  if (UNLIKELY(diagPresent(diag)))
  {
    diag->printMessages(26); // "CXR - "
    *diag << marketingCxr;
    diag->lineSkip(1);
  }

  if (trx.diagnostic().isActive() && (trx.diagnostic().diagnosticType() == Diagnostic411))
  {
    displayMixedClassHierarchy(trx, marketingCxr, mixedClassList, diag);
  }

  PaxTypeFare::SegmentStatusVecI iterSegStat = paxTfare.segmentStatus().begin();
  PaxTypeFare::SegmentStatusVecI iterSegStatEnd = paxTfare.segmentStatus().end();

  for (; itTvl != (mkt.travelSeg().end()) && (iterSegStat != iterSegStatEnd);
       itTvl++, iterSegStat++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*itTvl);
    PaxTypeFare::SegmentStatus& segStat = *iterSegStat;

    if (UNLIKELY(!airSeg))
      continue;

    if (UNLIKELY(marketingCxr != airSeg->carrier()))
    {
      if (UNLIKELY(diagPresent(diag)))
      {
        diag->printMessages(27); // "CARRIER NOT THE SAME ON ALL FLIGHTS"
      }
      mixClassOK = false;
      break;
    }

    currentBkgCode = airSeg->getBookingCode();

    if (trx.getRequest()->isLowFareRequested() && // WPNC
        segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
    {
      currentBkgCode = segStat._bkgCodeReBook;
    }

    currentMixHierarchy = getMixedClassHierarchy(marketingCxr, currentBkgCode, mixedClassList);

    // If no current hierarchy is found FAIL

    if (currentMixHierarchy == '0')
    {
      LOG4CXX_DEBUG(logger, "Leaving DomesticMixedClass::validate(): failure: no hier");
      return false;
    }
    // If current hierarchy same as previous but different on booking code, FAIL it

    //@TODO Need to return back the original code

    if ((currentMixHierarchy == previousHierarchy) &&
        (currentBkgCode.find(previousBkgCode, 0) != 0))

    {
      LOG4CXX_DEBUG(logger, "Leaving DomesticMixedClass::validate(): failure");
      return false;
    }

    previousHierarchy = currentMixHierarchy;
    previousBkgCode = currentBkgCode;

    if (lowestHierarchy != currentMixHierarchy)
    {
      if ((lowestHierarchy < '7') || (currentMixHierarchy < '7'))
      {
        mixClassOK = false;

        if (currentBkgCode == bkg)
        {
          foundBkgHierarchy = currentMixHierarchy;
        }
        if (currentMixHierarchy < lowestHierarchy)
        {
          lowestHierarchy = currentMixHierarchy;
        }
      }
    }
    else if (currentMixHierarchy < lowestHierarchy)
    {
      lowestHierarchy = currentMixHierarchy;
    }
  } // Loop for all travel segment

  // if processed booking code is the Lowest hierarchy, allow this FARE

  if ((lowestHierarchy < '7') && (lowestHierarchy == foundBkgHierarchy))
  {
    LOG4CXX_DEBUG(logger, "Leaving DomesticMixedClass::validate(): success");
    return true;
  }

  if (!mixClassOK)
  {
    LOG4CXX_DEBUG(logger, "Leaving DomesticMixedClass::validate(): failure");
    return false;
  }

  // Check for all night classes found in necessary
  // Each night booking code, its first letter is tested for an hierarchy of 7
  // to see if it matches.

  if (lowestHierarchy > '6')
  {
    if (nightClassVector.size() > 0)
    {
      CarrierMixCSegPtrVecIC nightClassSeg = nightClassVector.begin();

      for (; nightClassSeg != nightClassVector.end(); nightClassSeg++)
      {
        singleLetterBkgCode.clear();
        singleLetterBkgCode += (**nightClassSeg).bkgcd()[0];

        currentMixHierarchy =
            getMixedClassHierarchy(marketingCxr, singleLetterBkgCode, mixedClassList);
        if (currentMixHierarchy != '7')
        {
          mixClassOK = false;
          break;
        }
      } // for loop for night classes
    } // if for night classes
  } // if > 6, possible night classes are present

  // if some test did not pass -> return Fail

  if (!mixClassOK)
  {
    LOG4CXX_DEBUG(logger, "Leaving DomesticMixedClass::validate(): failure");
    return false;
  }

  LOG4CXX_DEBUG(logger,
                "Leaving DomesticMixedClass::validate(): " << (mixClassOK ? "success" : "failure"));
  return mixClassOK;
}

/**
 * Search for the Booking code hierarchy in the MixedClassTableInfo.
 *
 * @param cxr             Carrier code
 * @param bkg             Booking Code is being validated
 * @param mixedClassList  List of all BookingCodes for the carrier
 * @param diag            DiagCollector
 *
 * @return  the hierarchy number
 *
 * @todo Main Flow of Events
 -#
 */

char
DomesticMixedClass::getMixedClassHierarchy(const CarrierCode& cxr,
                                           const BookingCode currentBkgCode,
                                           const std::vector<CarrierMixedClass*>& mixedClassList)
{
  Indicator currentMixHierarchy = '0'; // default

  CarrierMixCListPtrIC mixedClassIter = mixedClassList.begin();

  for (; mixedClassIter != mixedClassList.end(); mixedClassIter++)
  {
    CarrierMixCSegPtrVecIC mixClassSeg = (*mixedClassIter)->segs().begin();

    for (; mixClassSeg != (*mixedClassIter)->segs().end(); mixClassSeg++)
    {
      //@TODO Need to return back the original code

      if (currentBkgCode.find((*mixClassSeg)->bkgcd(), 0) == 0)
      {
        currentMixHierarchy = (*mixClassSeg)->hierarchy();
        // Night classes
        if (UNLIKELY(currentMixHierarchy == '8'))
        {
          nightClassVector.push_back(*mixClassSeg);
        }
        break;
      }
    } // for loop - segments inside of each node of the mixed class vector

    if (currentMixHierarchy != '0') // hierarchy is found
      break;

  } // for carrier Mixed Class List

  return currentMixHierarchy;
}

bool
DomesticMixedClass::displayMixedClassHierarchy(
    PricingTrx& trx,
    const CarrierCode& cxr,
    const std::vector<CarrierMixedClass*>& mixedClassList,
    DiagCollector* diag) const
{

  CarrierMixCListPtrIC mixedClassIter = mixedClassList.begin();

  for (; mixedClassIter != mixedClassList.end(); mixedClassIter++)
  {
    Indicator previousHierarchy = '0'; // Hierarchy for the previous bkg code

    if (diagPresent(diag))
    {
      *diag << (**mixedClassIter);

      diag->printMessages(28); // "HIERARCHY              BOOKING CODE"

      CarrierMixCSegPtrVecIC mixClassSeg = (*mixedClassIter)->segs().begin();

      for (; mixClassSeg != (*mixedClassIter)->segs().end(); mixClassSeg++)
      {
        if (!(previousHierarchy == (*mixClassSeg)->hierarchy()))
        {
          previousHierarchy = (*mixClassSeg)->hierarchy();

          diag->displayHierarchy(previousHierarchy);

          *diag << (*mixClassSeg)->bkgcd() << " ";
        }
        else
          *diag << (*mixClassSeg)->bkgcd() << " ";
      }
    } // for loop - segments inside of each node of the mixed class vector
  } // for carrier Mixed Class List
  return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Diag411Collector*
DomesticMixedClass::diag411(DiagCollector* diag) const
{
  return dynamic_cast<Diag411Collector*>(diag);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool
DomesticMixedClass::diagPresent(DiagCollector* diag) const
{
  if (UNLIKELY(diag != nullptr))
  {
    if (diag411(diag))
    {
      return true;
    }
  }
  return false;
}
} // namespace tse
