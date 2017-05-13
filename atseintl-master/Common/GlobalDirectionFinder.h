//----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include "Common/Assert.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/Loc.h"

#include <vector>

namespace tse
{
class FareMarket;
class GlobalDirSeg;
class PricingTrx;
class Loc;
class TravelSeg;
class GlobalDirSeg;

using GlobalDirSegVector = std::vector<GlobalDirSeg*>;
/**
 * @class GlobalDirection
 * Selects a global direction for each Fare Market.
 * This class accesses global direction table and
 * selects a set of global direction object for a
 * FareMarket using origin and destination geodata as primary
 * key. Then it process the validation as per the requirement
 * to select the first applicable global direction.
 */

class GlobalDirectionFinder
{
  friend class GlobalDirectionFinderTest;

public:
  class Location
  {
  public:
    Location(const Loc* loc, bool hidden = false) : _loc(loc), _isHidden(hidden)
    {
      TSE_ASSERT(_loc);
    }

    bool operator==(const Location& rhs) const
    {
      return _loc == rhs._loc && _isHidden == rhs._isHidden;
    }

    const Loc& loc() const { return *_loc; }

    const LocCode& locCode() const { return _loc->loc(); }

    const IATAAreaCode& area() const { return _loc->area(); }

    const IATASubAreaCode& subArea() const { return _loc->subarea(); }

    bool isHidden() const { return _isHidden; }

  private:
    const Loc* _loc;
    bool _isHidden;
  };

  GlobalDirectionFinder(const std::vector<Location>& locations) : _locations(locations) {};
  virtual ~GlobalDirectionFinder() {};

  virtual bool getGlobalDirection(const PricingTrx* trx,
                                  DateTime travelDate,
                                  const std::set<CarrierCode>& carriers,
                                  GlobalDirection& globalDir) const;

  static const char WITHIN_ONE_AREA;
  static const char BETWEEN_TWO_AREAS;
  static const char FROM_ONE_AREA;
  static const std::string ATLANTIC_PACIFIC;
  static const std::string ATLANTIC;

protected:
  bool setGlobalDirectionForRW(const PricingTrx* trx, GlobalDirection& globalDir) const;
  bool isRWProcessing(const PricingTrx* trx) const;
  void logGlobalDirection(const GlobalDirection& globalDir) const;

  /**
   * Validate whether a given GlobalDirection Segment is applicable to the given Locs
   *
   * @param origin    Origin Location
   * @param dest      Destination Location
   * @param globalDir GlobalDirection Segment from db
   * @param withinOneAreaOnly
   *                  whether to validate this segment is within one segment or not
   *
   * @return bool if segment is valid and should be processed, false otherwise
   */
  bool validateGlobalDir(const Location& origin,
                         const Location& dest,
                         const GlobalDirSeg& globalDir,
                         const bool withinOneAreaOnly) const;

  /**
   * Process all Travel on Carrier logic.
   *
   * If this data member of the retrieved global direction
   * object is not empty it dominates the global direction
   * selection process over everything else.To use the GlobalDire each travel
   * segment has to have the same carrier that is mentioned
   * in allTvloncarrier field of the global direction table.
   * @param faremarket FareMarket A reference to the FareMarket object.
   * @return  bool
   *         - ture, if all TravelSeg match the carrier
   *         - false, otherwise.
   */
  bool validateAllTrvOnCxrLogic(const std::set<CarrierCode>& carriers,
                                const GlobalDirSeg& globalDir) const;

  /**
   * Processes the must be via routing logic.
   *
   * To use the GlobalDirection, FareMarket must have
   * these points as ticketed/non-ticketed points.
   * @param faremarket FareMarket A reference to the FareMarket object.
   * @return bool
   *            - true, if location found in the travel segment.
   *            - false, otherwise.
   */
  bool validateMustBeViaLoc(const GlobalDirSeg& globalDir) const;

  /**
   * Processes the must Not be via location logic.
   *
   * The location present in the 'notbevialoc' column
   * of the global direction table can not be a ticketed and
   * non-ticeted point of a FareMarket
   * @param faremarket FareMarket A reference to the FareMarket object
   * @return  bool
   *            - ture, if non-stop segment via loc1 and loc2 is found.
   *            - false, otherwise.
   *
   */
  bool validateMustNotBeViaLoc(const GlobalDirSeg& globalDir) const;

  /**
   * Processes the must be via intermediate locations logic.
   *
   * This requirement implies  a FareMarket can  use the global direction
   * only when there is a non-stop service between intermediate
   * location1 and intermediate location2 mentioned in the global
   * direction table.
   * @param fareMarket FareMarket& A reference to the FareMarket.
   * @return bool
   *                 - ture, when a non-stop service  between
   *                   two intermediate locations match a travel
   *                   segment.
   *                 - false, otherwise
   */
  bool validateMustBeViaIntermediateLoc(const GlobalDirSeg& globalDir) const;

  /**
   * Processes must not be via intermediate locations logic.
   *
   * This method validates the requirement that a global direcion
   * can not be used if the FareMarket has a non-stop service
   * between two intermediate locaions corresponding to the
   * global direction mentioned in the global direction table.
   * @param fareMarket FareMarket& A reference to the FareMarket.
   * @return bool
   *           - True, no match is found.
   *            - False otherwise.
   *
   */
  bool validateMustNotBeViaIntermediateLoc(const GlobalDirSeg& globalDir) const;

  bool validateLoc(const LocType& matchLocType,
                   const LocCode& matchLoc,
                   const bool checkHiddenStops,
                   const bool matchRC) const;

  bool validateLoc(const LocType& matchLocType1,
                   const LocCode& matchLoc1,
                   const LocType& matchLocType2,
                   const LocCode& matchLoc2,
                   const bool matchRC) const;

  bool applyRestrictionsforAPglobal() const;
  bool applyRestrictionsforATglobal() const;

  bool withinOneArea(const bool checkHiddenStops = false) const;

private:
  bool findTheBestMatchedGlobalDirectionForJourney(const PricingTrx* trx,
                                                   GlobalDirection& globalDir,
                                                   const std::set<CarrierCode>& carriers,
                                                   const GlobalDirSegVector& globalDirectionList,
                                                   const DateTime& travelDate) const;

  bool traverses3areas() const;

  size_t countTransfers(const IATAAreaCode& firstArea, const IATAAreaCode& secondArea) const;

private:
  const std::vector<Location>& _locations;
};
} // End namespace tse
