//----------------------------------------------------------------------------
//
//  Copyright Sabre 2003
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

#include "Common/SmallBitSet.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseEnums.h"

#include <set>
#include <vector>

namespace tse
{
class FareMarket;
class Itin;

/** @class GvoerningCarrier
 * This class selects the governing carrier for each FareMarket.
 * Each fare market calls selectGoverningCarrier() to determine
 * the governing carrier.
 * Governing Carrier selection logic varies with the travel
 * boundary of the FareMaket and a FareMarket can have exactly
 * one governing carrier.
 *
 */
class PricingTrx;
class AirSeg;
class TravelSeg;

typedef SmallBitSet<uint8_t, FMTravelBoundary> TravelBoundarySet;

class GoverningCarrier
{
  friend class GoverningCarrierTest;

public:
  // this constructor needs for the Mileage request (WN)
  GoverningCarrier();
  GoverningCarrier(PricingTrx* trx);
  virtual ~GoverningCarrier() {}

  /**
   * Main method to instantiate FareMarket object.
   * @param fareMarket  A reference to the FareMarket object.
   * @return bool
   *
   */
  bool process(FareMarket& fareMarket);

  bool processRtw(FareMarket& fareMarket);

  /**
   * This method is for other classes to call GovCxr selection class.
   * The major difference is, govCxr called from FareCollectorOrchestrator
   * returns at most one GovCxr per FareMarket.
   * But this method takes a reference of TavelSeg and determines all
   * possible governing Carriers, especially when the travel boundary is
   * within Europe or within same country.
   *
   */
  bool getGoverningCarrier(const std::vector<TravelSeg*>& tvlsegs,
                           std::set<CarrierCode>& govCxrSet,
                           FMDirection fmDirection = FMDirection::UNKNOWN);

  bool getGoverningCarrier(const std::vector<TravelSeg*>& segments,
                           std::set<CarrierCode>& result,
                           FMDirection direction,
                           Boundary boundary);

  bool getGoverningCarrierRtw(const std::vector<TravelSeg*>& segments,
                              const Boundary boundary,
                              std::set<CarrierCode>& govCxrSet);

  /**
   * Determines GovCxr when travel is within USA/CA.
   * This method selects Carrier of the first AirSeg as the governing carrier.
   * @param faremkt FareMarket A reference to the FareMarket object.
   * @return bool
   *        -true, if GovCxr is selected.
   *        -false otherwise.
   */
  bool processCarrierPreference(FareMarket& fareMarket);

  /**
   * Determines GovCxr.
   * This method takes a reference of TavelSeg and determines all
   * possible governing Carriers, based on High TPM when travel is
   * within Sub-Area 21.
   */
  CarrierCode getHighestTPMCarrierOld(const std::vector<TravelSeg*>& tvlSegs);

  /**
   * Determines GovCxr - IATA Fare Selection project Sep. 2014
   * This method takes a reference of TavelSegs and determines all
   * possible governing Carriers, based on High TPM when travel on 
   * international Fare Market. The first one carrier with the highest TMP 
   * is a winner.
   */
  CarrierCode getHighestTPMCarrier(const std::vector<TravelSeg*>& tvlSegs,
                                      FMDirection direction,
                                      TravelSeg*& primarySector );

  uint32_t getHighestTPMByCarrier(const CarrierCode& cxr,
                                  const std::vector<TravelSeg*>& tvlSegs);

  bool getGovCxrSpecialCases(FareMarket&);

  virtual uint32_t getTPM(const AirSeg& airSeg);

  bool selectFirstCrossingGovCxr(const std::vector<TravelSeg*>& tvlsegs,
                                 std::set<CarrierCode>& govCxrSet,
                                 FMDirection fmDirection,
                                 TravelSeg*& primarySector);

  CarrierCode
  getForeignDomHighestTPMCarrier(const std::vector<TravelSeg*>& tvlSegs,
                                 FMDirection direction, TravelSeg*& primarySector );

  uint32_t getForeignDomHighestTPMByCarrier(const CarrierCode& cxr,
                                            const std::vector<TravelSeg*>& tvlSegs);

protected:
  enum RtwStep
  {
    TRANSOCEANIC = 0x01,
    AREA_CROSSING = 0x02,
    SUBAREA_CROSSING = 0x04,
    INTERNATIONAL = 0x08
  };

  typedef SmallBitSet<uint8_t, RtwStep> RtwSteps;

  void setupRtw(PricingTrx& trx);
  bool processRtwSteps(const std::vector<TravelSeg*>& travelSegs,
                       const RtwSteps& steps,
                       std::set<CarrierCode>& govCxrSet,
                       TravelSeg** primarySector = nullptr);

  bool selectWithinUSCA(const std::vector<TravelSeg*>& tvlsegs,
                        std::set<CarrierCode>& govCxrSet,
                        FMDirection fmDirection,
                        TravelSeg** primarySector = nullptr);

  /**
   * Determines GovCxr when travel is within Same country Except USA/CA
   * This method selects carrier with the lowest applicable fare as the
   * governing carrier. In some special cases, this selection process can
   * involve booking code validation.
   * @param faremkt FareMarket A reference to the FareMarket object.
   * @return bool
   *         -true, if GovCxr is selected.
   *	    -false otherwise.
   */
  bool selectWithinCountryExceptUSCA(const std::vector<TravelSeg*>& tvlsegs,
                                     std::set<CarrierCode>& govCxrSet,
                                     TravelSeg** primarySector = nullptr);

  /**
   * Determines the GovCxr when travel boundary is within one IATA Area.
   * First carrier providing service accross two different SubIATA is
   * the governing carrier.
   * @param faremkt FareMarket A reference to the FareMarket object.
   * @return bool
   *        -true, if GovCxr is selected.
   *        -false otherwise.
   */
  bool selectWithinSameIATA(const std::vector<TravelSeg*>& tvlsegs,
                            std::set<CarrierCode>& govCxrSet,
                            FMDirection fmDirection,
                            TravelSeg** primarySector = nullptr);

  /**
   * Selects the GovCxr when travel boundary is within two IATA Areas.
   * Carrier for the first change in IATA area is the governing carrier.
   * @param faremkt FareMarket A reference to the FareMarket object.
   * @return bool
   *        -true, if GovCxr is selected.
   *        -false otherwise.
   */
  bool selectWithinMultiIATA(const std::vector<TravelSeg*>& tvlsegs,
                             std::set<CarrierCode>& govCxrSet,
                             FMDirection fmDirection,
                             TravelSeg** primarySector = nullptr);

  /**
   * Selects the GovCxr when travel boundary involves all three IATA Areas.
   * First carrier providing service into or from Area1 is the governing carrier.
   * @param faremkt FareMarket A reference to the FareMarket object.
   * @return bool
   -true, if GovCxr is selected.
   -false otherwise.
   */
  bool selectWithinAllIATA(const std::vector<TravelSeg*>& tvlsegs,
                           std::set<CarrierCode>& govCxrSet,
                           FMDirection fmDirection,
                           TravelSeg** primarySector = nullptr);

  /**
   * Selects the governing carrier when travel boundary is within same
   * SubIATAArea except Europe.
   * First carrier providing an international flight is the governign carrier.
   */
  bool selectWithinSameSubIATA(const std::vector<TravelSeg*>& tvlsegs,
                               std::set<CarrierCode>& govCxrSet,
                               FMDirection fmDirection,
                               TravelSeg** primarySector = nullptr);

  /**
   *  Selects the governing carrier when travel boundary is within North America.
   *  First carrier providing service to/from US/CA is the governing carrier.
   */
  bool selectWithinSubIATA11(const std::vector<TravelSeg*>& tvlsegs,
                             std::set<CarrierCode>& govCxrSet,
                             FMDirection fmDirection,
                             TravelSeg** primarySector = nullptr);

  template <class TravelSegPred, class TravelSegIterator>
  bool selectFirstSatisfying(const TravelSegPred& pred,
                             TravelSegIterator tsBegin,
                             TravelSegIterator tsEnd,
                             std::set<CarrierCode>& govCxrSet,
                             TravelSeg** primarySector = nullptr)
  {
    TravelSegIterator tsIt = std::find_if(tsBegin, tsEnd, pred);

    if (tsIt != tsEnd)
    {
      return setGoverningCarrier(*tsIt, govCxrSet, primarySector);
    }
    return false;
  }

  /**
   * Find the governing carrrier when travel is witing Europe.
   * Carrier providing the first international flight and carrier
   * providing service for highest TPM are two candidates for
   * governing carrier. After fare retrieval and fare validation
   * carrier providing the lowest fare will be chosen as a
   * governing carrier.
   * @param tvlSeg std::vector<TravelSeg*> Reference to a vector of pointers.
   * @param govCxrSet std::set<CarrierCode> Reference to set of carrier.
   * @return bool
   */
  bool selectWithinSubIATA21(const std::vector<TravelSeg*>& tvlsegs,
                             std::set<CarrierCode>& govCxrSet,
                             FMDirection direction,
                             TravelSeg** primarySector = nullptr);

  /**
   * Finds the carrier of the First International Flight.
   * @param farMarket FareMarket A referecne to the FareMarket object.
   * @return carrier CarrierCode String of two characer.
   */
  CarrierCode
  getFirstIntlFlt(const std::vector<TravelSeg*>& tvlSegs, TravelSeg** primarySector = nullptr);

  bool getCarrierInIATA1(const std::vector<TravelSeg*>& tvlSegs,
                         std::vector<TravelSeg*>::const_iterator first,
                         std::vector<TravelSeg*>::const_iterator last,
                         std::set<CarrierCode>& govCxrSet,
                         TravelSeg** primarySector = nullptr);

  uint32_t getTPMOld(const AirSeg& airSeg);
  virtual uint32_t getTPMWn(const AirSeg& airSeg);

  bool isDomesticOld(const AirSeg& airSeg);
  bool isDomestic(const AirSeg& airSeg, const bool withinScandinavia);
  bool setUpGovCxrBasedOnFirstCrossing(FareMarket& newFareMarket);

  bool setGoverningCarrier(TravelSeg* travelSeg,
                           std::set<CarrierCode>& govCxrSet,
                           TravelSeg** primarySector = nullptr);

  bool setFirstSubAreaCrossing(const std::vector<TravelSeg*>& tvlsegs,
                                  std::set<CarrierCode>& govCxrSet,
                                  FMDirection fmDirection,
                                  TravelSeg*& primarySector);
  bool setFirstAreaCrossing(const std::vector<TravelSeg*>& tvlsegs,
                                  std::set<CarrierCode>& govCxrSet,
                                  FMDirection fmDirection,
                                  TravelSeg*& primarySector);
  bool setFirstInternationalCrossing(const std::vector<TravelSeg*>& tvlsegs,
                                  std::set<CarrierCode>& govCxrSet,
                                  FMDirection fmDirection,
                                  TravelSeg*& primarySector);


  bool getSetOfGoverningCarriers(const std::vector<TravelSeg*>& tvlSegs,
                                 TravelSeg* travelSeg,
                                 std::set<CarrierCode>& govCxrSet,
                                 FMDirection fmDirection,
                                 TravelSeg** primarySector);

  TravelSeg*
  findTravelSegInAreaOne(const std::vector<TravelSeg*>& tvlSegs, FMDirection fmDirection);

  bool getGovCxrSpecialEuropeOnly(FareMarket&, TravelBoundarySet&);
  bool getGovCxrExceptAllDomestic(FareMarket&, TravelBoundarySet&);
  bool getGovCxrSpecialDomesticNonUSCA(FareMarket&, TravelBoundarySet&);
  bool getGovCxrSpecialDomesticUSCA(FareMarket&, TravelBoundarySet&);

  PricingTrx* trx(void) const { return _trx; }

private:
  PricingTrx* _trx;
  bool _isRtw;
  bool _isIataFareSelectionApplicable;
  const Itin* _itin;
};
}

