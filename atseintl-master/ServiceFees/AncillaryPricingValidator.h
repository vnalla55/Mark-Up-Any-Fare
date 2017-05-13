//-------------------------------------------------------------------
//  Copyright Sabre 2010
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------
#pragma once

#include "ServiceFees/CommonSoftMatchValidator.h"

namespace tse
{
class AncillaryPricingValidator : public CommonSoftMatchValidator
{
  friend class AncillaryPricingValidatorTest;

public:
  using CommonSoftMatchValidator::CommonSoftMatchValidator;

  bool validate(OCFees& ocFees, bool stopMatch = false) const override;

private:
  bool isMissingDeptArvlTime() const;
  bool checkIntermediatePoint(const OptionalServicesInfo& optSrvInfo,
                              std::vector<TravelSeg*>& passedLoc3Dest,
                              OCFees& ocFees) const override;
  bool checkStopCnxDestInd(const OptionalServicesInfo& optSrvInfo,
                           const std::vector<TravelSeg*>& passedLoc3Dest,
                           OCFees& ocFees) const override;
  bool isStopover(const OptionalServicesInfo& optSrvInfo,
                  const TravelSeg* seg,
                  const TravelSeg* next) const override;
  bool
  checkFrequentFlyerStatus(const OptionalServicesInfo& optSrvInfo, OCFees& ocFees) const override;
  bool checkSecurity(const OptionalServicesInfo& optSrvInfo, OCFees& ocFees) const override;
  bool isValidCabin(SvcFeesResBkgDesigInfo* rbdInfo,
                    AirSeg& seg,
                    OCFees& ocFees,
                    const BookingCode& (SvcFeesResBkgDesigInfo::*mhod)(void) const) const;
  bool isRBDValid(AirSeg* seg,
                  const std::vector<SvcFeesResBkgDesigInfo*>& validRBDInfoForSeg,
                  OCFees& ocFees) const override;
  bool checkCabinData(AirSeg& seg,
                      const CabinType& cabin,
                      const CarrierCode& carrier,
                      OCFees& ocFees) const override;
  bool validateStartAndStopTime(OCFees& ocFees) const override;
  bool validateStartAndStopTime(const OptionalServicesInfo& info, OCFees& ocFees) const override;
  bool checkEquipmentType(const OptionalServicesInfo& optSrvInfo, OCFees& ocFees) const override;
  bool isValidRuleTariffInd(const ServiceRuleTariffInd& ruleTariffInd,
                            TariffCategory tariffCategory) const override;
  StatusT186 isValidCarrierFlightNumber(const AirSeg& air, CarrierFlightSeg& t186) const override;
  bool setPBDate(const OptionalServicesInfo& optSrvInfo,
                 OCFees& ocFees,
                 const DateTime& pbDate) const override;
  short getTimeDiff(const DateTime& time) const override;
  bool
  skipUpgradeCheck(const OptionalServicesInfo& optSrvInfo, const OCFees& ocFees) const override;

  bool validateLocation(const VendorCode& vendor,
                        const LocKey& locKey,
                        const Loc& loc,
                        const LocCode& zone,
                        bool emptyRet,
                        CarrierCode carrier,
                        LocCode* matchLoc = nullptr) const override;
  StatusS7Validation getStatusCode(const OptionalServicesInfo& optSrvInfo, OCFees& ocFees) const;
  virtual const Cabin* getCabin(const CarrierCode& carrier,
                                AirSeg& seg,
                                const BookingCode& bookingCode) const;
};
} // tse namespace

