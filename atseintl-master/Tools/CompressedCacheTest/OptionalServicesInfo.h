//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "OptionalServicesSeg.h"
#include "Flattenizable.h"
#include "LocKey.h"

namespace tse
{
  class OptionalServicesInfo
  {
  public:
    OptionalServicesInfo()
      : _fltTktMerchInd(' ')
      , _seqNo(0)
      , _publicPrivateInd(' ')
      , _minAge(0)
      , _maxAge(0)
      , _firstOccurence(0)
      , _lastOccurence(0)
      , _frequentFlyerStatus(0)
      , _serviceFeesAccountCodeTblItemNo(0)
      , _serviceFeesTktDesigTblItemNo(0)
      , _serviceFeesSecurityTblItemNo(0)
      , _sectorPortionInd(' ')
      , _fromToWithinInd(' ')
      , _stopCnxDestInd(' ')
      , _stopoverUnit(' ')
      , _cabin(' ')
      , _serviceFeesResBkgDesigTblItemNo(0)
      , _upgradeCabin(' ')
      , _upgrdServiceFeesResBkgDesigTblItemNo(0)
      , _serviceFeesCxrResultingFclTblItemNo(0)
      , _ruleTariff(0)
      , _fareInd(' ')
      , _startTime(0)
      , _stopTime(0)
      , _timeApplication(' ')
      , _taxCarrierFltTblItemNo(0)
      , _advPurchTktIssue(' ')
      , _refundReissueInd(' ')
      , _commissionInd(' ')
      , _interlineInd(' ')
      , _formOfFeeRefundInd(' ')
      , _notAvailNoChargeInd(' ')
      , _collectSubtractInd(' ')
      , _netSellingInd(' ')
      , _andOrInd(' ')
      , _serviceFeesCurrencyTblItemNo(0)
      , _frequentFlyerMileageFee(0)
      , _frequentFlyerMileageAppl(' ')
      , _taxInclInd(' ')
      , _availabilityInd(' ')
      , _taxTextTblItemNo(0)
      , _segCount(0)
      , _resultServiceFeesTktDesigTblItemNo(0)
      , _tvlStartYear(0)
      , _tvlStartMonth(0)
      , _tvlStartDay(0)
      , _tvlStopYear(0)
      , _tvlStopMonth(0)
      , _tvlStopDay(0)
      , _freeBaggagePcs(-1)
      , _baggageOccurrenceFirstPc(-1)
      , _baggageOccurrenceLastPc(-1)
      , _baggageWeight(-1)
      , _baggageWeightUnit(' ')
      , _taxExemptInd(' ')
    {
    }

    ~OptionalServicesInfo()
    {
      std::vector<OptionalServicesSeg *>::iterator segIt;
      for (segIt = _segs.begin(); segIt != _segs.end(); segIt++)
      {
        delete *segIt;
      }
      std::vector<OptionalServicesSeg*> empty;
      _segs.swap(empty);
    }

    VendorCode&                   vendor() { return _vendor; }
    const VendorCode&             vendor() const { return _vendor; }

    CarrierCode&                  carrier() { return _carrier; }
    const CarrierCode&            carrier() const { return _carrier; }

    ServiceTypeCode&              serviceTypeCode() { return _serviceTypeCode; }
    const ServiceTypeCode&        serviceTypeCode() const { return _serviceTypeCode; }

    ServiceSubTypeCode&           serviceSubTypeCode() { return _serviceSubTypeCode; }
    const ServiceSubTypeCode&     serviceSubTypeCode() const { return _serviceSubTypeCode; }

    Indicator&                    fltTktMerchInd() { return _fltTktMerchInd; }
    const Indicator&              fltTktMerchInd() const { return _fltTktMerchInd; }

    uint32_t&                     seqNo() { return _seqNo; }
    const uint32_t&               seqNo() const { return _seqNo; }

    DateTime&                     createDate() { return _createDate; }
    const DateTime&               createDate() const { return _createDate; }

    DateTime&                     expireDate() { return _expireDate; }
    const DateTime&               expireDate() const { return _expireDate; }

    Indicator&                    publicPrivateInd() { return _publicPrivateInd; }
    const Indicator&              publicPrivateInd() const { return _publicPrivateInd; }

    DateTime&                     effDate() { return _effDate; }
    const DateTime&               effDate() const { return _effDate; }

    DateTime&                     discDate() { return _discDate; }
    const DateTime&               discDate() const { return _discDate; }

    DateTime&                     ticketEffDate() { return _ticketEffDate; }
    const DateTime&               ticketEffDate() const { return _ticketEffDate; }

    DateTime&                     ticketDiscDate() { return _ticketDiscDate; }
    const DateTime&               ticketDiscDate() const { return _ticketDiscDate; }

    PaxTypeCode&                  psgType() { return _psgType; }
    const PaxTypeCode&            psgType() const { return _psgType; }

    uint16_t&                     minAge() { return _minAge; }
    const uint16_t&               minAge() const { return _minAge; }

    uint16_t&                     maxAge() { return _maxAge; }
    const uint16_t&               maxAge() const { return _maxAge; }

    uint16_t&                     firstOccurence() { return _firstOccurence; }
    const uint16_t&               firstOccurence() const { return _firstOccurence; }

    uint16_t&                     lastOccurence() { return _lastOccurence; }
    const uint16_t&               lastOccurence() const { return _lastOccurence; }

    uint16_t&                     frequentFlyerStatus() { return _frequentFlyerStatus; }
    const uint16_t&               frequentFlyerStatus() const { return _frequentFlyerStatus; }

    uint32_t&                     serviceFeesAccountCodeTblItemNo() { return _serviceFeesAccountCodeTblItemNo; }
    const uint32_t&               serviceFeesAccountCodeTblItemNo() const { return _serviceFeesAccountCodeTblItemNo; }

    uint32_t&                     serviceFeesTktDesigTblItemNo() { return _serviceFeesTktDesigTblItemNo; }
    const uint32_t&               serviceFeesTktDesigTblItemNo() const { return _serviceFeesTktDesigTblItemNo; }

    TourCode&                  tourCode() { return _tourCode; }
    const TourCode&            tourCode() const { return _tourCode; }

    uint32_t&                     serviceFeesSecurityTblItemNo() { return _serviceFeesSecurityTblItemNo; }
    const uint32_t&               serviceFeesSecurityTblItemNo() const { return _serviceFeesSecurityTblItemNo; }

    Indicator&                    sectorPortionInd() { return _sectorPortionInd; }
    const Indicator&              sectorPortionInd() const { return _sectorPortionInd; }

    Indicator&                    fromToWithinInd() { return _fromToWithinInd; }
    const Indicator&              fromToWithinInd() const { return _fromToWithinInd; }

    LocKey&                       loc1() { return _loc1; }
    const LocKey&                 loc1() const { return _loc1; }

    LocCode&                      loc1ZoneTblItemNo() { return _loc1ZoneTblItemNo; }
    const LocCode&                loc1ZoneTblItemNo() const { return _loc1ZoneTblItemNo; }

    LocKey&                       loc2() { return _loc2; }
    const LocKey&                 loc2() const { return _loc2; }

    LocCode&                      loc2ZoneTblItemNo() { return _loc2ZoneTblItemNo; }
    const LocCode&                loc2ZoneTblItemNo() const { return _loc2ZoneTblItemNo; }

    LocKey&                       viaLoc() { return _viaLoc; }
    const LocKey&                 viaLoc() const { return _viaLoc; }

    LocCode&                      viaLocZoneTblItemNo() { return _viaLocZoneTblItemNo; }
    const LocCode&                viaLocZoneTblItemNo() const { return _viaLocZoneTblItemNo; }

    Indicator&                    stopCnxDestInd() { return _stopCnxDestInd; }
    const Indicator&              stopCnxDestInd() const { return _stopCnxDestInd; }

    StopoverTime&                 stopoverTime() { return _stopoverTime; }
    const StopoverTime&           stopoverTime() const { return _stopoverTime; }

    Indicator&                    stopoverUnit() { return _stopoverUnit; }
    const Indicator&              stopoverUnit() const { return _stopoverUnit; }

    Indicator&                    cabin() { return _cabin; }
    const Indicator&              cabin() const { return _cabin; }

    uint32_t&                     serviceFeesResBkgDesigTblItemNo() { return _serviceFeesResBkgDesigTblItemNo; }
    const uint32_t&               serviceFeesResBkgDesigTblItemNo() const { return _serviceFeesResBkgDesigTblItemNo; }

    Indicator&                    upgradeCabin() { return _upgradeCabin; }
    const Indicator&              upgradeCabin() const { return _upgradeCabin; }

    uint32_t&                     upgrdServiceFeesResBkgDesigTblItemNo() { return _upgrdServiceFeesResBkgDesigTblItemNo; }
    const uint32_t&               upgrdServiceFeesResBkgDesigTblItemNo() const { return _upgrdServiceFeesResBkgDesigTblItemNo; }

    uint32_t&                     serviceFeesCxrResultingFclTblItemNo() { return _serviceFeesCxrResultingFclTblItemNo; }
    const uint32_t&               serviceFeesCxrResultingFclTblItemNo() const { return _serviceFeesCxrResultingFclTblItemNo; }

    uint16_t&                     ruleTariff() { return _ruleTariff; }
    const uint16_t&               ruleTariff() const { return _ruleTariff; }

    RuleNumber&                   rule() { return _rule; }
    const RuleNumber&             rule() const { return _rule; }

    ServiceRuleTariffInd&         ruleTariffInd() { return _ruleTariffInd; }
    const ServiceRuleTariffInd&   ruleTariffInd() const { return _ruleTariffInd; }

    Indicator&                    fareInd() { return _fareInd; }
    const Indicator&              fareInd() const { return _fareInd; }

    uint16_t&                     startTime() { return _startTime; }
    const uint16_t&               startTime() const { return _startTime; }

    uint16_t&                     stopTime() { return _stopTime; }
    const uint16_t&               stopTime() const { return _stopTime; }

    Indicator&                    timeApplication() { return _timeApplication; }
    const Indicator&              timeApplication() const { return _timeApplication; }

    DayOfWeekCode&                  dayOfWeek() { return _dayOfWeek; }
    const DayOfWeekCode&            dayOfWeek() const { return _dayOfWeek; }

    uint32_t&                     carrierFltTblItemNo() { return _taxCarrierFltTblItemNo; }
    const uint32_t&               carrierFltTblItemNo() const { return _taxCarrierFltTblItemNo; }

    EquipmentType&                equipmentCode() { return _equipmentCode; }
    const EquipmentType&          equipmentCode() const { return _equipmentCode; }

    ServicePurchasePeriod&        advPurchPeriod() { return _advPurchPeriod; }
    const ServicePurchasePeriod&  advPurchPeriod() const { return _advPurchPeriod; }

    ServicePurchaseUnit&          advPurchUnit() { return _advPurchUnit; }
    const ServicePurchaseUnit&    advPurchUnit() const { return _advPurchUnit; }

    Indicator&                    advPurchTktIssue() { return _advPurchTktIssue; }
    const Indicator&              advPurchTktIssue() const { return _advPurchTktIssue; }

    Indicator&                    refundReissueInd() { return _refundReissueInd; }
    const Indicator&              refundReissueInd() const { return _refundReissueInd; }

    Indicator&                    commissionInd() { return _commissionInd; }
    const Indicator&              commissionInd() const { return _commissionInd; }

    Indicator&                    interlineInd() { return _interlineInd; }
    const Indicator&              interlineInd() const { return _interlineInd; }

    Indicator&                    formOfFeeRefundInd() { return _formOfFeeRefundInd; }
    const Indicator&              formOfFeeRefundInd() const { return _formOfFeeRefundInd; }

    Indicator&                    notAvailNoChargeInd() { return _notAvailNoChargeInd; }
    const Indicator&              notAvailNoChargeInd() const { return _notAvailNoChargeInd; }

    Indicator&                    collectSubtractInd() { return _collectSubtractInd; }
    const Indicator&              collectSubtractInd() const { return _collectSubtractInd; }

    Indicator&                    netSellingInd() { return _netSellingInd; }
    const Indicator&              netSellingInd() const { return _netSellingInd; }

    Indicator&                    andOrInd() { return _andOrInd; }
    const Indicator&              andOrInd() const { return _andOrInd; }

    uint32_t&                     serviceFeesCurrencyTblItemNo() { return _serviceFeesCurrencyTblItemNo; }
    const uint32_t&               serviceFeesCurrencyTblItemNo() const { return _serviceFeesCurrencyTblItemNo; }

    uint32_t&                     applicationFee() { return _frequentFlyerMileageFee; }
    const uint32_t&               applicationFee() const { return _frequentFlyerMileageFee; }

    Indicator&                    frequentFlyerMileageAppl() { return _frequentFlyerMileageAppl; }
    const Indicator&              frequentFlyerMileageAppl() const { return _frequentFlyerMileageAppl; }

    Indicator&                    taxInclInd() { return _taxInclInd; }
    const Indicator&              taxInclInd() const { return _taxInclInd; }

    Indicator&                    availabilityInd() { return _availabilityInd; }
    const Indicator&              availabilityInd() const { return _availabilityInd; }

    RuleBusterFcl&                  ruleBusterFcl() { return _ruleBusterFcl; }
    const RuleBusterFcl&            ruleBusterFcl() const { return _ruleBusterFcl; }

    uint32_t&                     taxTblItemNo() { return _taxTextTblItemNo; }
    const uint32_t&               taxTblItemNo() const { return _taxTextTblItemNo; }

    uint16_t&                     segCount() { return _segCount; }
    const uint16_t&               segCount() const { return _segCount; }

    uint32_t&                     resultServiceFeesTktDesigTblItemNo() { return _resultServiceFeesTktDesigTblItemNo; }
    const uint32_t&               resultServiceFeesTktDesigTblItemNo() const { return _resultServiceFeesTktDesigTblItemNo; }

    std::vector<OptionalServicesSeg*>&       segs() { return _segs; };
    const std::vector<OptionalServicesSeg*>& segs() const { return _segs; };

    int32_t&                      tvlStartYear() { return _tvlStartYear;}
    const int32_t&                tvlStartYear() const { return _tvlStartYear; }

    int32_t&                      tvlStartMonth() { return _tvlStartMonth;}
    const int32_t&                tvlStartMonth() const { return _tvlStartMonth; }

    int32_t&                      tvlStartDay() { return _tvlStartDay;}
    const int32_t&                tvlStartDay() const { return _tvlStartDay; }

    int32_t&                      tvlStopYear() { return _tvlStopYear;}
    const int32_t&                tvlStopYear() const { return _tvlStopYear; }

    int32_t&                      tvlStopMonth() { return _tvlStopMonth;}
    const int32_t&                tvlStopMonth() const { return _tvlStopMonth; }

    int32_t&                      tvlStopDay() { return _tvlStopDay;}
    const int32_t&                tvlStopDay() const { return _tvlStopDay; }

    int32_t&                      freeBaggagePcs() { return _freeBaggagePcs; }
    const int32_t&                freeBaggagePcs() const { return _freeBaggagePcs; }

    int32_t&                      baggageOccurrenceFirstPc() { return _baggageOccurrenceFirstPc; }
    const int32_t&                baggageOccurrenceFirstPc() const { return _baggageOccurrenceFirstPc; }

    int32_t&                      baggageOccurrenceLastPc() { return _baggageOccurrenceLastPc; }
    const int32_t&                baggageOccurrenceLastPc() const { return _baggageOccurrenceLastPc; }

    int32_t&                      baggageWeight() { return _baggageWeight; }
    const int32_t&                baggageWeight() const { return _baggageWeight; }

    Indicator&                    baggageWeightUnit() { return _baggageWeightUnit; }
    const Indicator&              baggageWeightUnit() const { return _baggageWeightUnit; }

    bool operator==(const OptionalServicesInfo &second) const
    {
      bool equal = 
        (_vendor                                == second._vendor) &&
        (_carrier                               == second._carrier) &&
        (_serviceTypeCode                       == second._serviceTypeCode) &&
        (_serviceSubTypeCode                    == second._serviceSubTypeCode) &&
        (_fltTktMerchInd                        == second._fltTktMerchInd) &&
        (_seqNo                                 == second._seqNo) &&
        (_createDate                            == second._createDate) &&
        (_expireDate                            == second._expireDate) &&
        (_publicPrivateInd                      == second._publicPrivateInd) &&
        (_effDate                               == second._effDate) &&
        (_discDate                              == second._discDate) &&
        (_ticketEffDate                         == second._ticketEffDate) &&
        (_ticketDiscDate                        == second._ticketDiscDate) &&
        (_psgType                               == second._psgType) &&
        (_minAge                                == second._minAge) &&
        (_maxAge                                == second._maxAge) &&
        (_firstOccurence                        == second._firstOccurence) &&
        (_lastOccurence                         == second._lastOccurence) &&
        (_frequentFlyerStatus                   == second._frequentFlyerStatus) &&
        (_serviceFeesAccountCodeTblItemNo       == second._serviceFeesAccountCodeTblItemNo) &&
        (_serviceFeesTktDesigTblItemNo          == second._serviceFeesTktDesigTblItemNo) &&
        (_tourCode                              == second._tourCode) &&
        (_serviceFeesSecurityTblItemNo          == second._serviceFeesSecurityTblItemNo) &&
        (_sectorPortionInd                      == second._sectorPortionInd) &&
        (_fromToWithinInd                       == second._fromToWithinInd) &&
        (_loc1                                  == second._loc1) &&
        (_loc1ZoneTblItemNo                     == second._loc1ZoneTblItemNo) &&
        (_loc2                                  == second._loc2) &&
        (_loc2ZoneTblItemNo                     == second._loc2ZoneTblItemNo) &&
        (_viaLoc                                == second._viaLoc) &&
        (_viaLocZoneTblItemNo                   == second._viaLocZoneTblItemNo) &&
        (_stopCnxDestInd                        == second._stopCnxDestInd) &&
        (_stopoverTime                          == second._stopoverTime) &&
        (_stopoverUnit                          == second._stopoverUnit) &&
        (_cabin                                 == second._cabin) &&
        (_serviceFeesResBkgDesigTblItemNo       == second._serviceFeesResBkgDesigTblItemNo) &&
        (_upgradeCabin                          == second._upgradeCabin) &&
        (_upgrdServiceFeesResBkgDesigTblItemNo  == second._upgrdServiceFeesResBkgDesigTblItemNo) &&
        (_serviceFeesCxrResultingFclTblItemNo   == second._serviceFeesCxrResultingFclTblItemNo) &&
        (_ruleTariff                            == second._ruleTariff) &&
        (_rule                                  == second._rule) &&
        (_ruleTariffInd                         == second._ruleTariffInd) &&
        (_fareInd                               == second._fareInd) &&
        (_startTime                             == second._startTime) &&
        (_stopTime                              == second._stopTime) &&
        (_timeApplication                       == second._timeApplication) &&
        (_dayOfWeek                             == second._dayOfWeek) &&
        (_taxCarrierFltTblItemNo                == second._taxCarrierFltTblItemNo) &&
        (_equipmentCode                         == second._equipmentCode) &&
        (_advPurchPeriod                        == second._advPurchPeriod) &&
        (_advPurchUnit                          == second._advPurchUnit) &&
        (_advPurchTktIssue                      == second._advPurchTktIssue) &&
        (_refundReissueInd                      == second._refundReissueInd) &&
        (_commissionInd                         == second._commissionInd) &&
        (_interlineInd                          == second._interlineInd) &&
        (_formOfFeeRefundInd                    == second._formOfFeeRefundInd) &&
        (_notAvailNoChargeInd                   == second._notAvailNoChargeInd) &&
        (_collectSubtractInd                    == second._collectSubtractInd) &&
        (_netSellingInd                         == second._netSellingInd) &&
        (_andOrInd                              == second._andOrInd) &&
        (_serviceFeesCurrencyTblItemNo          == second._serviceFeesCurrencyTblItemNo) &&
        (_frequentFlyerMileageFee               == second._frequentFlyerMileageFee) &&
        (_frequentFlyerMileageAppl              == second._frequentFlyerMileageAppl) &&
        (_taxInclInd                            == second._taxInclInd) &&
        (_availabilityInd                       == second._availabilityInd) &&
        (_ruleBusterFcl                         == second._ruleBusterFcl) &&
        (_taxTextTblItemNo                      == second._taxTextTblItemNo) &&
        (_segCount                              == second._segCount) &&
        (_resultServiceFeesTktDesigTblItemNo    == second._resultServiceFeesTktDesigTblItemNo) &&
        (_segs.size()                           == second._segs.size()) &&
        (_tvlStartYear                          == second._tvlStartYear) &&
        (_tvlStartMonth                         == second._tvlStartMonth) &&
        (_tvlStartDay                           == second._tvlStartDay) &&
        (_tvlStopYear                           == second._tvlStopYear) &&
        (_tvlStopMonth                          == second._tvlStopMonth) &&
        (_tvlStopDay                            == second._tvlStopDay) &&
        (_freeBaggagePcs                        == second._freeBaggagePcs) &&
        (_baggageOccurrenceFirstPc               == second._baggageOccurrenceFirstPc) &&
        (_baggageOccurrenceLastPc                == second._baggageOccurrenceLastPc) &&
        (_baggageWeight                         == second._baggageWeight) &&
        (_baggageWeightUnit                     == second._baggageWeightUnit)
        && _taxExemptInd == second._taxExemptInd;

      for (size_t i = 0; equal && (i < _segs.size()); ++i)
      {
        equal = (*_segs[i] == *second._segs[i]);
      }

      return equal;
    }

    void flattenize(Flattenizable::Archive & archive)
    {
      FLATTENIZE(archive, _vendor);
      FLATTENIZE(archive, _carrier);
      FLATTENIZE(archive, _serviceTypeCode);
      FLATTENIZE(archive, _serviceSubTypeCode);
      FLATTENIZE(archive, _fltTktMerchInd);
      FLATTENIZE(archive, _seqNo);
      FLATTENIZE(archive, _createDate);
      FLATTENIZE(archive, _expireDate);
      FLATTENIZE(archive, _publicPrivateInd);
      FLATTENIZE(archive, _effDate);
      FLATTENIZE(archive, _discDate);
      FLATTENIZE(archive, _ticketEffDate);
      FLATTENIZE(archive, _ticketDiscDate);
      FLATTENIZE(archive, _psgType);
      FLATTENIZE(archive, _minAge);
      FLATTENIZE(archive, _maxAge);
      FLATTENIZE(archive, _firstOccurence);
      FLATTENIZE(archive, _lastOccurence);
      FLATTENIZE(archive, _frequentFlyerStatus);
      FLATTENIZE(archive, _serviceFeesAccountCodeTblItemNo);
      FLATTENIZE(archive, _serviceFeesTktDesigTblItemNo);
      FLATTENIZE(archive, _tourCode);
      FLATTENIZE(archive, _serviceFeesSecurityTblItemNo);
      FLATTENIZE(archive, _sectorPortionInd);
      FLATTENIZE(archive, _fromToWithinInd);
      FLATTENIZE(archive, _loc1);
      FLATTENIZE(archive, _loc1ZoneTblItemNo);
      FLATTENIZE(archive, _loc2);
      FLATTENIZE(archive, _loc2ZoneTblItemNo);
      FLATTENIZE(archive, _viaLoc);
      FLATTENIZE(archive, _viaLocZoneTblItemNo);
      FLATTENIZE(archive, _stopCnxDestInd);
      FLATTENIZE(archive, _stopoverTime);
      FLATTENIZE(archive, _stopoverUnit);
      FLATTENIZE(archive, _cabin);
      FLATTENIZE(archive, _serviceFeesResBkgDesigTblItemNo);
      FLATTENIZE(archive, _upgradeCabin);
      FLATTENIZE(archive, _upgrdServiceFeesResBkgDesigTblItemNo);
      FLATTENIZE(archive, _serviceFeesCxrResultingFclTblItemNo);
      FLATTENIZE(archive, _ruleTariff);
      FLATTENIZE(archive, _rule);
      FLATTENIZE(archive, _ruleTariffInd);
      FLATTENIZE(archive, _fareInd);
      FLATTENIZE(archive, _startTime);
      FLATTENIZE(archive, _stopTime);
      FLATTENIZE(archive, _timeApplication);
      FLATTENIZE(archive, _dayOfWeek);
      FLATTENIZE(archive, _taxCarrierFltTblItemNo);
      FLATTENIZE(archive, _equipmentCode);
      FLATTENIZE(archive, _advPurchPeriod);
      FLATTENIZE(archive, _advPurchUnit);
      FLATTENIZE(archive, _advPurchTktIssue);
      FLATTENIZE(archive, _refundReissueInd);
      FLATTENIZE(archive, _commissionInd);
      FLATTENIZE(archive, _interlineInd);
      FLATTENIZE(archive, _formOfFeeRefundInd);
      FLATTENIZE(archive, _notAvailNoChargeInd);
      FLATTENIZE(archive, _collectSubtractInd);
      FLATTENIZE(archive, _netSellingInd);
      FLATTENIZE(archive, _andOrInd);
      FLATTENIZE(archive, _serviceFeesCurrencyTblItemNo);
      FLATTENIZE(archive, _frequentFlyerMileageFee);
      FLATTENIZE(archive, _frequentFlyerMileageAppl);
      FLATTENIZE(archive, _taxInclInd);
      FLATTENIZE(archive, _availabilityInd);
      FLATTENIZE(archive, _ruleBusterFcl);
      FLATTENIZE(archive, _taxTextTblItemNo);
      FLATTENIZE(archive, _segCount);
      FLATTENIZE(archive, _resultServiceFeesTktDesigTblItemNo);
      FLATTENIZE(archive, _segs);
      FLATTENIZE(archive, _tvlStartYear) ;
      FLATTENIZE(archive, _tvlStartMonth) ;
      FLATTENIZE(archive, _tvlStartDay) ;
      FLATTENIZE(archive, _tvlStopYear) ;
      FLATTENIZE(archive, _tvlStopMonth) ;
      FLATTENIZE(archive, _tvlStopDay) ;
      FLATTENIZE(archive, _freeBaggagePcs) ;
      FLATTENIZE(archive, _baggageOccurrenceFirstPc) ;
      FLATTENIZE(archive, _baggageOccurrenceLastPc) ;
      FLATTENIZE(archive, _baggageWeight) ;
      FLATTENIZE(archive, _baggageWeightUnit) ;
    }

    void dummyData()
    {
      static DateTime currentTime(time(NULL));
      _vendor                               = "ABCD";
      _carrier                              = "EF";
      _serviceTypeCode                      = "GH";
      _serviceSubTypeCode                   = "IJK";
      _fltTktMerchInd                       = 'L';
      _seqNo                                = 1;
      _createDate                           = currentTime;
      _expireDate                           = currentTime;
      _publicPrivateInd                     = 'M';
      _effDate                              = currentTime;
      _discDate                             = currentTime;
      _ticketEffDate                        = currentTime;
      _ticketDiscDate                       = currentTime;
      _psgType                              = "NOP";
      _minAge                               = 3;
      _maxAge                               = 4;
      _firstOccurence                       = 5;
      _lastOccurence                        = 6;
      _frequentFlyerStatus                  = 7;
      _serviceFeesAccountCodeTblItemNo      = 8;
      _serviceFeesTktDesigTblItemNo         = 9;
      _tourCode                             = "123456789012345";
      _serviceFeesSecurityTblItemNo         = 10;
      _sectorPortionInd                     = 'Q';
      _fromToWithinInd                      = 'R';
      _loc1.loc()                           = "STUVW";
      _loc1.locType()                       = 'X';
      _loc1ZoneTblItemNo                    = "1234567";
      _loc2.loc()                           = "YZABC";
      _loc2.locType()                       = 'D';
      _loc2ZoneTblItemNo                    = "1234567";
      _viaLoc.loc()                         = "EFGHI";
      _viaLoc.locType()                     = 'J';
      _viaLocZoneTblItemNo                  = "1234567";
      _stopCnxDestInd                       = 'K';
      _stopoverTime                         = "LMN";
      _stopoverUnit                         = 'O';
      _cabin                                = 'P';
      _serviceFeesResBkgDesigTblItemNo      = 11;
      _upgradeCabin                         = 'Q';
      _upgrdServiceFeesResBkgDesigTblItemNo = 12;
      _serviceFeesCxrResultingFclTblItemNo  = 13;
      _ruleTariff                           = 14;
      _rule                                 = "RSTU";
      _ruleTariffInd                        = "VWX";
      _fareInd                              = 'Y';
      _startTime                            = 15;
      _stopTime                             = 16;
      _timeApplication                      = 'Z';
      _dayOfWeek                            = "123456";
      _taxCarrierFltTblItemNo               = 18;
      _equipmentCode                        = "ABC";
      _advPurchPeriod                       = "DEF";
      _advPurchUnit                         = "GH";
      _advPurchTktIssue                     = 'I';
      _refundReissueInd                     = 'J';
      _commissionInd                        = 'K';
      _interlineInd                         = 'L';
      _formOfFeeRefundInd                   = 'M';
      _notAvailNoChargeInd                  = 'N';
      _collectSubtractInd                   = 'A';
      _netSellingInd                        = 'O';
      _andOrInd                             = 'P';
      _serviceFeesCurrencyTblItemNo         = 19;
      _frequentFlyerMileageFee              = 20;
      _frequentFlyerMileageAppl             = 'Q';
      _taxInclInd                           = 'R';
      _availabilityInd                      = 'S';
      _ruleBusterFcl                        = "STUVWXYZ";
      _taxTextTblItemNo                     = 21;
      _segCount                             = 22;
      _resultServiceFeesTktDesigTblItemNo   = 23;
      _tvlStartYear                         = 24;
      _tvlStartMonth                        = 25;
      _tvlStartDay                          = 26;
      _tvlStopYear                          = 27;
      _tvlStopMonth                         = 28;
      _tvlStopDay                           = 29;
      _freeBaggagePcs                       = 30;
      _baggageOccurrenceFirstPc              = 31;
      _baggageOccurrenceLastPc               = 32;
      _baggageWeight                        = 33;
      _baggageWeightUnit                    = 'T';
      _taxExemptInd = 'T';

      _segs.push_back(new OptionalServicesSeg);
      _segs.push_back(new OptionalServicesSeg);

      OptionalServicesSeg::dummyData(*_segs[0]);
      OptionalServicesSeg::dummyData(*_segs[1]);
    }

    WBuffer &write (WBuffer &os) const
    {
      return convert(os, this);
    }

    RBuffer &read (RBuffer &is)
    {
      return convert(is, this);
    }
  protected:
    VendorCode                        _vendor;
    CarrierCode                       _carrier;
    ServiceTypeCode                   _serviceTypeCode;
    ServiceSubTypeCode                _serviceSubTypeCode;
    Indicator                         _fltTktMerchInd;
    uint32_t                          _seqNo;
    DateTime                          _createDate;
    DateTime                          _expireDate;
    Indicator                         _publicPrivateInd;
    DateTime                          _effDate;
    DateTime                          _discDate;
    DateTime                          _ticketEffDate;
    DateTime                          _ticketDiscDate;
    PaxTypeCode                       _psgType;
    uint16_t                          _minAge;
    uint16_t                          _maxAge;
    uint16_t                          _firstOccurence;
    uint16_t                          _lastOccurence;
    uint16_t                          _frequentFlyerStatus;
    uint32_t                          _serviceFeesAccountCodeTblItemNo;
    uint32_t                          _serviceFeesTktDesigTblItemNo;
    TourCode                       _tourCode;
    uint32_t                          _serviceFeesSecurityTblItemNo;
    Indicator                         _sectorPortionInd;
    Indicator                         _fromToWithinInd;
    LocKey                            _loc1;
    LocCode                           _loc1ZoneTblItemNo;
    LocKey                            _loc2;
    LocCode                           _loc2ZoneTblItemNo;
    LocKey                            _viaLoc;
    LocCode                           _viaLocZoneTblItemNo;
    Indicator                         _stopCnxDestInd;
    StopoverTime                      _stopoverTime;
    Indicator                         _stopoverUnit;
    Indicator                         _cabin;
    uint32_t                          _serviceFeesResBkgDesigTblItemNo;
    Indicator                         _upgradeCabin;
    uint32_t                          _upgrdServiceFeesResBkgDesigTblItemNo;
    uint32_t                          _serviceFeesCxrResultingFclTblItemNo;
    uint16_t                          _ruleTariff;
    RuleNumber                        _rule;
    ServiceRuleTariffInd              _ruleTariffInd;
    Indicator                         _fareInd;
    uint16_t                          _startTime;
    uint16_t                          _stopTime;
    Indicator                         _timeApplication;
    DayOfWeekCode                       _dayOfWeek;
    uint32_t                          _taxCarrierFltTblItemNo;
    EquipmentType                     _equipmentCode;
    ServicePurchasePeriod             _advPurchPeriod;
    ServicePurchaseUnit               _advPurchUnit;
    Indicator                         _advPurchTktIssue;
    Indicator                         _refundReissueInd;
    Indicator                         _commissionInd;
    Indicator                         _interlineInd;
    Indicator                         _formOfFeeRefundInd;
    Indicator                         _notAvailNoChargeInd;
    Indicator                         _collectSubtractInd;
    Indicator                         _netSellingInd;
    Indicator                         _andOrInd;
    uint32_t                          _serviceFeesCurrencyTblItemNo;
    uint32_t                          _frequentFlyerMileageFee;
    Indicator                         _frequentFlyerMileageAppl;
    Indicator                         _taxInclInd;
    Indicator                         _availabilityInd;
    RuleBusterFcl                       _ruleBusterFcl;
    uint32_t                          _taxTextTblItemNo;
    uint16_t                          _segCount;
    uint32_t                          _resultServiceFeesTktDesigTblItemNo;
    int32_t                           _tvlStartYear;
    int32_t                           _tvlStartMonth;
    int32_t                           _tvlStartDay;
    int32_t                           _tvlStopYear;
    int32_t                           _tvlStopMonth;
    int32_t                           _tvlStopDay;
    int32_t                           _freeBaggagePcs;
    int32_t                           _baggageOccurrenceFirstPc;
    int32_t                           _baggageOccurrenceLastPc;
    int32_t                           _baggageWeight;
    Indicator                         _baggageWeightUnit;
    Indicator                         _taxExemptInd;
    std::vector<OptionalServicesSeg*> _segs;
  private:
    template <typename B, typename T> static B& convert (B& buffer,
                                                         T ptr)
    {
    return buffer
           & ptr->_vendor
           & ptr->_carrier
           & ptr->_serviceTypeCode
           & ptr->_serviceSubTypeCode
           & ptr->_fltTktMerchInd
           & ptr->_seqNo
           & ptr->_createDate
           & ptr->_expireDate
           & ptr->_publicPrivateInd
           & ptr->_effDate
           & ptr->_discDate
           & ptr->_ticketEffDate
           & ptr->_ticketDiscDate
           & ptr->_psgType
           & ptr->_minAge
           & ptr->_maxAge
           & ptr->_firstOccurence
           & ptr->_lastOccurence
           & ptr->_frequentFlyerStatus
           & ptr->_serviceFeesAccountCodeTblItemNo
           & ptr->_serviceFeesTktDesigTblItemNo
           & ptr->_tourCode
           & ptr->_serviceFeesSecurityTblItemNo
           & ptr->_sectorPortionInd
           & ptr->_fromToWithinInd
           & ptr->_loc1
           & ptr->_loc1ZoneTblItemNo
           & ptr->_loc2
           & ptr->_loc2ZoneTblItemNo
           & ptr->_viaLoc
           & ptr->_viaLocZoneTblItemNo
           & ptr->_stopCnxDestInd
           & ptr->_stopoverTime
           & ptr->_stopoverUnit
           & ptr->_cabin
           & ptr->_serviceFeesResBkgDesigTblItemNo
           & ptr->_upgradeCabin
           & ptr->_upgrdServiceFeesResBkgDesigTblItemNo
           & ptr->_serviceFeesCxrResultingFclTblItemNo
           & ptr->_ruleTariff
           & ptr->_rule
           & ptr->_ruleTariffInd
           & ptr->_fareInd
           & ptr->_startTime
           & ptr->_stopTime
           & ptr->_timeApplication
           & ptr->_dayOfWeek
           & ptr->_taxCarrierFltTblItemNo
           & ptr->_equipmentCode
           & ptr->_advPurchPeriod
           & ptr->_advPurchUnit
           & ptr->_advPurchTktIssue
           & ptr->_refundReissueInd
           & ptr->_commissionInd
           & ptr->_interlineInd
           & ptr->_formOfFeeRefundInd
           & ptr->_notAvailNoChargeInd
           & ptr->_collectSubtractInd
           & ptr->_netSellingInd
           & ptr->_andOrInd
           & ptr->_serviceFeesCurrencyTblItemNo
           & ptr->_frequentFlyerMileageFee
           & ptr->_frequentFlyerMileageAppl
           & ptr->_taxInclInd
           & ptr->_availabilityInd
           & ptr->_ruleBusterFcl
           & ptr->_taxTextTblItemNo
           & ptr->_segCount
           & ptr->_resultServiceFeesTktDesigTblItemNo
           & ptr->_tvlStartYear
           & ptr->_tvlStartMonth
           & ptr->_tvlStartDay
           & ptr->_tvlStopYear
           & ptr->_tvlStopMonth
           & ptr->_tvlStopDay
           & ptr->_freeBaggagePcs
           & ptr->_baggageOccurrenceFirstPc
           & ptr->_baggageOccurrenceLastPc
           & ptr->_baggageWeight
           & ptr->_baggageWeightUnit
           & ptr->_taxExemptInd
           & ptr->_segs;
    }
  };
}
