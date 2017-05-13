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



namespace tse
{
/**
 * @class MileageValidator.
 * Implemenst the mileage validation process of a Fare.
 * This is invoked when the fare is a mileage fare or when the restriction
 * is 12/16.
 * It instantiate the TPMCollector and MPMCollector and also applies all
 * exclusion logic for Mileage. It also populates the MileageInfo for
 * diagnostic 450 and 455
 */

class PricingTrx;
class TravelRoute;
class MileageInfo;
class MileageRoute;
class FareCalcVisitor;
class PaxTypeFare;
class DiagCollector;

class MileageValidator
{

public:
  friend class MileageValidatorTest;
  /**
   * Validates if the total accumlated TPM is within the limit of MPM or not.
   * @param trx A reference to the PricingTrx to use dataHandle.
   * @param MileageInfo A reference to the MileageInfo to store diagnostic infos.
   * @TravelRoute A reference to the travelRoute to construct the MileageRoute.
   * @returns true if the TPM is within MPM or the mileage Route qualifies for a
   * surcharge exception.
   */

  void validate(PricingTrx&,
                MileageInfo&,
                const TravelRoute&,
                PaxTypeFare* ptf = nullptr,
                DiagCollector* diag = nullptr) const;
  MileageValidator();
  virtual ~MileageValidator();

private:
  /**
   * Populates MileageInfo using the validation results stored in MileageRoute.
   * @param MileageInfo a reference to the MileageInfo.
   * @param MileageRoute a reference to the mileage route constructed from travelRoute.
   * @returns true/false.
   */

  bool fillInDiagnosticInfo(MileageInfo&, MileageRoute&, FareCalcVisitor&) const;

  bool fillInDiagnosticInfoForPSR(MileageInfo& mInfo, MileageRoute& mRoute) const;

  /**
   * Calculates total TPM by applying all exclusion and exceptions.
   * Iterates over the collection of MileageRouteItems and apply exclusion
   * like SouthAtlantic/TPD/SurfaceSectorExemption etc to get the total
   * applicable TPM of the market. Then the surcharge amount is caclulated
   * by using total applicable TPM and applicable MPM( after applying MPM deductions
   * if applicable.
   * @param MileageInfo a reference to the MileageInfo.
   * @param MileageRoute a reference to the mileage route constructed from travelRoute.
   * @returns true/false.
   */

  bool calculateMileage(MileageInfo&, MileageRoute&, FareCalcVisitor&, PricingTrx&) const;

  virtual bool getTPD(MileageRoute&, FareCalcVisitor&) const;

  virtual bool getPSR(MileageRoute&) const;
  virtual bool applyMileageEqualization(MileageRoute&, MileageInfo&) const;
};

} // namespace tse

