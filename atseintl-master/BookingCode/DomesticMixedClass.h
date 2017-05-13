//----------------------------------------------------------------------------
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

#include "Common/TseCodeTypes.h"

#include <vector>


namespace tse
{
class CarrierMixedClass;
class CarrierMixedClassSeg;
class Diag411Collector;
class DiagCollector;
class FareMarket;
class PaxTypeFare;
class PricingTrx;
class TravelSeg;

/**
 * Performs all validations for mixed booking codes within FareMarket.
 * This class determines the validity of the mixed booking codes for the through fare
 * only for domestic US/CA fare component.
 */

class DomesticMixedClass
{

  friend class DomesticMixedClassTest;

public:
  DomesticMixedClass() = default;
  DomesticMixedClass(const DomesticMixedClass&) = delete;
  DomesticMixedClass& operator=(const DomesticMixedClass&) = delete;
  /*  This method is called to conduct the validation on the mixed booking codes on the FareMarket.
      It will gets the MixedClassInfo data from the Cache. Analyze the booking code hierarchy.
      Loop through all flights gathering data and checking if carrier is the same
      on all travel segments. The loop is broken and the validation return FAIL
      if some other is found or if no hierarhy is found for any booking code examined.
      Also a vector of night classes are collected on this loop for futher investigation
      if necessary.
      The result could be pass or fail.
  */
  bool validate(PricingTrx& trx,
                PaxTypeFare& paxTfare,
                FareMarket& mkt,
                BookingCode& bkg,
                DiagCollector* diag);

private:
  std::vector<CarrierMixedClassSeg*> nightClassVector;

  // Search for the Booking code hierarchy in the vector provided by DataHandler.

  char getMixedClassHierarchy(const CarrierCode& cxr,
                              const BookingCode bkg,
                              const std::vector<CarrierMixedClass*>& mixedClassList);

  bool displayMixedClassHierarchy(PricingTrx& trx,
                                  const CarrierCode& cxr,
                                  const std::vector<CarrierMixedClass*>& mixedClassList,
                                  DiagCollector* diag) const;

  bool diagPresent(DiagCollector* diag) const;

  Diag411Collector* diag411(DiagCollector* diag) const;
};

using TravelSegPtrVec = std::vector<tse::TravelSeg*>;
using TravelSegPtrVecI = std::vector<tse::TravelSeg*>::iterator;
using TravelSegPtrVecIC = std::vector<tse::TravelSeg*>::const_iterator;

using CarrierMixCSegPtrVec = std::vector<CarrierMixedClassSeg*>;
using CarrierMixCSegPtrVecI = std::vector<CarrierMixedClassSeg*>::iterator;
using CarrierMixCSegPtrVecIC = std::vector<CarrierMixedClassSeg*>::const_iterator;

using CarrierMixCListPtr = std::vector<CarrierMixedClass*>;
using CarrierMixCListPtrI = std::vector<CarrierMixedClass*>::iterator;
using CarrierMixCListPtrIC = std::vector<CarrierMixedClass*>::const_iterator;

} // end tse namespace
