//----------------------------------------------------------------------------
//
// Copyright Sabre 2004
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseConsts.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"


#include <vector>

namespace tse
{
/**
 *@class MileageSurchrgeExcption
 * Selects Through Fare that qualify for a Mileage Surcharge Exception and apply surcharge as per
 *Category 17.
 * Selection of mileage surcharge exception is based on the route of travel between the
 * board and off point. Mileage Surcharge Exception Applies at the Rule level.
 * Example : If a Fare has as mileage surcharge of amount 15M. If this fare qualify for a surcharge
 * exception( exceptions are filed per Vendor + GovCxr+ RuleTariff + Rule) and exceptions allows you
 *to use surchagre upto 5M.
 * Then the Fare surcharge is reduced to 5M from 15M.
 *
 */

class PaxTypeFare;
class PricingTrx;
class MileageInfo;
class LocKey;
class Loc;
class TravelRoute;
class MileageSurchExcept;

class MileageSurchargeException
{
public:
  /**
   * Validates each PaxTypeFare for mileage surcharge exception.
   * @param tvlRoute  A reference to the TravelRoute.
   * @param pFares    A reference to the collection of PaxTypeFares that are alrady validated for
   * Routing.
   * @param trx  A reference to the PricingTrx.
   */

  friend class MileageSurchargeExceptionTest;

  virtual void validate(const TravelRoute& tvlRoute,
                        std::vector<PaxTypeFare*>& pFares,
                        PricingTrx& trx,
                        MileageInfo& mInfo) const;

  virtual bool validateSingleFare(const TravelRoute& tvlRoute,
                                  PaxTypeFare& pFare,
                                  PricingTrx& trx,
                                  const MileageInfo& mInfo) const;

  // paxTyeIndicator
  static constexpr char ANY_INFANT = 'Y';
  static constexpr char ANY_CHILD = 'Y';

  // surchargeIndicator
  static constexpr char SURCHARGE_DOESNT_APPLY = 'N';
  static constexpr char NO_STOP_OVER_ALLOWED = 'Y';
  static constexpr char CARRIERS_NOT_ALLOWED = 'Y';
  static constexpr char TRAVEL_WITHIN_MPM = 'W';
  static constexpr char MUST_ONLY_VIA = 'Y';
  static constexpr char MUST_VIA_ALL = 'Y';

  virtual ~MileageSurchargeException() = default;

private:
  /**
   * Validates if the MileageSurhExcept retrieved from database are applicable for a given fare or
   *not.
   * It calls all other validations process and returns true iff all of them return true.
   * @param sur       A reference to the MileageSurchExcept object.
   * @param fare      A reference to the PaxTypeFare.
   * @param tvlRoute  A reference to the TravelRoute.
   * @param trx       A reference to the PricingTrx.
   * @returns bool     True if surcharege is applicable for the Fare. False otehrwise.
   **/

  virtual bool isExceptionApplicable(const MileageSurchExcept& sur,
                                     const PaxTypeFare& fare,
                                     const TravelRoute& tvlRoute,
                                     PricingTrx& trx) const;
  /**
   * Applies the applicable surcharge exception to the PaxTypeFare and update the Fare for the new
   *surcharge.
   * @param mpmSurchExcept An indicator that specify the upper limit of surcharge that can be
   *ignored.
   * @param fare A reference to the PaxTypeFare.
   **/
  void apply(Indicator& mpmSurchExcept, PaxTypeFare& fare, const MileageInfo& mInfo) const;
  void update(PaxTypeFare& fare, const MileageInfo& mInfo) const;

  /**
   * auxillary method to compare two FareClass.
   */
  bool
  isValidFareType(const FareClassCode& actualFareType, const FareClassCode& expectedFareType) const;

  virtual std::vector<MileageSurchExcept*> getData(const PaxTypeFare& fare, PricingTrx& trx) const;

  /**
   * auxillary method to compare two PaxTypeCode.
   */
  bool isValidPaxType(const std::vector<PaxTypeCode>& paxTypes, const PaxTypeCode& paxType) const;
  /**
   * validates carrier restriction of a Fare Component.
   * If Rule specifies a vector of carrier to be used for the Fare Component
   * to qualify for the Exception, then it validates if all the carrier used
   * in the itinearry are also present in the Specified Carrier List ( set intersection
   * is zero).
   * It also validates negative carrier restriction which implies any carrier specified
   * can not be present in the route of travel. ( set intersection should be the summtion of
   * specified cxr vector and used cxr vector)
   *
   * @param sur A reference to the MileageSurchExcept.
   * @param tvlRoute A reference to the TravelRoute.
   * @returns bool true/false when restriction is met/not met.
   */
  bool validateCxrRestriction(const MileageSurchExcept& sur, const TravelRoute& tvlRoute) const;
  /**
   * validate stop over restriction for the Mileage surcharge exception.
   * when noStopOver indicator is set, the route of travel can not have a
   * stop over in the location specified in the must be via loc in the table.
   * @param noStopOver A reference to the LocKey.
   * @param tvlRoute A reference to the TravelRoute.
   * @returns bool true/false when restriction is met/not met.
   */

  bool validateStopOverRestriction(const LocKey& noStopOver, const TravelRoute& tvlRoute) const;
  /**
   * validates all GeoRestrictions( must be via/ must not be via) of the FareComponent.
   * @param sur A reference to the MileageSurchExcept.
   * @param tvlRoute A reference to the TravelRoute.
   * @returns bool true/false when restriction is met/not met.
   */
  bool validateGeoRestriction(const MileageSurchExcept& sur, const TravelRoute& tvlRoute) const;
  /**
   * validates resrictions specific to the Fare.
   * It validates the PaxType and the FareClass for the FareCompnent.
   * If any_infant or any_child indicator is set, it validates the PaxType
   * using PaxTypeUtil::isInfant() or isChild() method. Other wise it compares
   * the specified PaxTypeCode wit the used PaxTypeCode.
   * @param sur A reference to the MileageSurchExcept.
   * @param fare A reference to the PaxTypeFare.
   * @param trx A reference to the PricingTrx.
   * @returns bool true/false when restriction is met/not met.
   */

  bool validateFareRestriction(const MileageSurchExcept& sur,
                               const PaxTypeFare& fare,
                               PricingTrx& trx) const;
};

} // namespace tse

