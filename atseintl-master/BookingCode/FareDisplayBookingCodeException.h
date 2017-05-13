//-------------------------------------------------------------------
//
//	Copyright Sabre 2004
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

#include "BookingCode/RBData.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/BookingCodeExceptionSegment.h"
#include "DBAccess/BookingCodeExceptionSequence.h"
#include "DBAccess/DAOUtils.h"
#include "Rules/RuleUtil.h"

namespace tse
{
class FareDisplayTrx;
class PaxTypeFare;
class BookingCodeExceptionSequenceList;
class FarePath;
class PricingUnit;

using TravelSegVectorCI = std::vector<TravelSeg*>::const_iterator;

class FareDisplayBookingCodeException
{
  class segmentStatus
  {
  public:
    segmentStatus() = delete;
    segmentStatus(BookingCodeExceptionSequence* seq, bool conditional, bool forceCon)
      : _sequence(seq), _conditionalSegmentFound(conditional), _isForcedConditionalSeg(forceCon)
    {
    }

    BookingCodeExceptionSequence* _sequence = nullptr;
    bool _conditionalSegmentFound;
    bool _isForcedConditionalSeg;
  };

public:
  FareDisplayBookingCodeException();
  FareDisplayBookingCodeException(FareDisplayTrx* trx, PaxTypeFare* ptf, RBData* rbData = nullptr);
  virtual ~FareDisplayBookingCodeException() = default;

  void getBookingCodeException(FareDisplayTrx* trx,
                               PaxTypeFare& paxTypeFare,
                               std::vector<BookingCode>& bkgCodes);

  bool getBookingCodeException(std::vector<BookingCode>& bkgCodes);
  bool getBookingCodeException(const VendorCode& vendorCode,
                               std::vector<BookingCode>& bkgCodes,
                               const AirSeg* airSeg,
                               bool convention2);

  const static CarrierCode BCE_DOLLARDOLLARCARRIER;
  const static CarrierCode BCE_XDOLLARCARRIER;
  const static CarrierCode BCE_ANYCARRIER;

  static constexpr char BCE_PRIME = 'X';
  static constexpr char BCE_FARE_CONSTRUCTED = 'C';
  static constexpr char BCE_FARE_SPECIFIED = 'S';

  static constexpr char BCE_CHAR_BLANK = ' ';
  static constexpr char BCE_IF_FARECOMPONENT = '1';
  static constexpr char BCE_IF_ANY_TVLSEG = '2';

  static constexpr char BCE_PERMITTED = 'P';
  static constexpr char BCE_REQUIRED = 'R';
  static constexpr char BCE_REQUIRED_WHEN_OFFERED = 'W';
  static constexpr char BCE_REQUIRED_WHEN_AVAIL = 'V';
  static constexpr char BCE_PERMITTED_WHEN_PRIME_NOT_OFFERED = 'O';
  static constexpr char BCE_PERMITTED_WHEN_PRIME_NOT_AVAIL = 'A';
  static constexpr char BCE_REQUIRED_WHEN_PRIME_NOT_OFFERED = 'G';
  static constexpr char BCE_REQUIRED_WHEN_PRIME_NOT_AVAIL = 'H';
  static constexpr char BCE_RBD2_PERMITTED_IF_RBD1_AVAILABLE = 'B';
  static constexpr char BCE_RBD2_REQUIRED_IF_RBD1_AVAILABLE = 'D';
  static constexpr char BCE_ADDITIONAL_DATA_APPLIES = 'U';
  static constexpr char BCE_STANDBY = 'S';
  static constexpr char BCE_NOT_PERMITTED = 'X';
  static constexpr char BCE_DOES_NOT_EXIST = 'N';

  const static EquipmentType BCE_EQUIPBLANK;

  static constexpr char BCE_SEG_PRIMARY = 'P';
  static constexpr char BCE_SEG_SECONDARY = 'S';
  static constexpr char BCE_SEG_FROMTO_PRIMARY = 'T';

private:
  friend class FareDisplayBookingCodeExceptionTest;

  bool isValidSequence(const BookingCodeExceptionSequence& sequence);

  bool isValidSegment(const BookingCodeExceptionSegment& segment,
                      const BookingCodeExceptionSequence& sequence);

  bool validateCarrier(const BookingCodeExceptionSegment& segment,
                       const BookingCodeExceptionSequence& sequence,
                       const AirSeg* airSeg);

  bool validatePortionOfTravel(const BookingCodeExceptionSegment& segment,
                               const BookingCodeExceptionSequence& sequence,
                               const AirSeg* airSeg);

  bool validatePrimarySecondary(const BookingCodeExceptionSegment& segment,
                                const BookingCodeExceptionSequence& sequence,
                                const AirSeg* airSeg);

  bool validateTSI(const BookingCodeExceptionSegment& segment,
                   const BookingCodeExceptionSequence& sequence,
                   const AirSeg* airSeg);

  bool validateLocation(const BookingCodeExceptionSegment& segment,
                        const BookingCodeExceptionSequence& sequence);

  bool validatePointOfSale(const BookingCodeExceptionSegment& segment,
                           const BookingCodeExceptionSequence& sequence);

  bool validateSoldTag(const BookingCodeExceptionSegment& segment,
                       const BookingCodeExceptionSequence& sequence);

  bool validateDateTimeDOW(const BookingCodeExceptionSegment& segment,
                           const BookingCodeExceptionSequence& sequence);

  bool validateFareClassType(const BookingCodeExceptionSegment& segment,
                             const BookingCodeExceptionSequence& sequence);

  FareDisplayTrx* _trx = nullptr;
  PaxTypeFare* _paxTypeFare = nullptr;
  bool _isInternational = false;
  RBData* _rbData = nullptr; // For RB rule text
  bool _bceBkgUpdated = false;
  bool _convention2 = false;
  bool _forceConditional = false;
  bool processTable999Seq(const BookingCodeExceptionSequenceList& lists,
                          std::vector<BookingCode>& bkgCodes,
                          bool convention2,
                          const VendorCode& vendorCode);

  bool changeFareBasisCode(const BookingCodeExceptionSegment& segment, BookingCode bkg);

  bool isRBRequest();

  bool isRBSegmentConditional(const BookingCodeExceptionSegment& segment);
  /*
      void addRBBCESeqments(const VendorCode& vendorCode,
                            BookingCodeExceptionSequence& sequence,
                            bool& conditionalSegmentFound);
  */
  void addRBBCESeqments(const VendorCode& vendorCode,
                        BookingCodeExceptionSequence& sequence,
                        bool& conditionalSegmentFound,
                        std::vector<BookingCode>& bkgCodes);

  //    bool nonConditionalData(const BookingCodeExceptionSegment& segment);

  void updateTravelSeg(std::vector<TravelSeg*>& travelSeg, const TravelSeg* tvlSeg);

  /*---------------------------------------------------------------------------
   * Return Types for the Travel Segment after Booking code validated
   *-------------------------------------------------------------------------*/
  enum setStatus
  { INIT = '0',
    REC1 = '1',
    CONV_2 = '2' };
  ///////////////////////////////////////////////////////////////////////
  //  For validating directionality with multiple segments             //
  ///////////////////////////////////////////////////////////////////////
  enum BCEDirectionValidationState
  { BCE_NO_VALIDATION = 0,
    BCE_CONDITIONAL_VALIDATION_NO_MATCH = 1,
    BCE_CONDITIONAL_VALIDATION_MATCH = 2 };

  BCEDirectionValidationState _condDirValState = BCEDirectionValidationState::BCE_NO_VALIDATION;
  bool _useBKGExceptionIndex = false;

  bool canRemoveSegment(const BookingCodeExceptionSegment& seg1,
                        const BookingCodeExceptionSegment& seg2,
                        const BookingCodeExceptionSequence& seq1,
                        const BookingCodeExceptionSequence& seq2);

  virtual const BookingCodeExceptionSequenceList&
  getBookingCodeExceptionSequence(const VendorCode& vendor, const int itemNo);

  virtual const BookingCodeExceptionSequenceList&
  getBookingCodeExceptionSequence(const VendorCode& vendor,
                                  const CarrierCode& carrier,
                                  const TariffNumber& ruleTariff,
                                  const RuleNumber& rule,
                                  Indicator conventionNo,
                                  const DateTime& date);

  virtual bool scopeTSIGeo(const TSICode tsi,
                           const LocKey& locKey1,
                           const LocKey& locKey2,
                           const RuleConst::TSIScopeParamType& defaultScope,
                           PricingTrx& trx,
                           const FarePath* farePath,
                           const PricingUnit* pricingUnit,
                           const FareMarket* fareMarket,
                           const DateTime& ticketingDate,
                           RuleUtil::TravelSegWrapperVector& applTravelSegment,
                           const DiagnosticTypes& callerDiag = DiagnosticNone,
                           const VendorCode& vendorCode = "ATP");

  virtual bool isInLoc(const LocCode& loc,
                       const LocTypeCode& locTypeCode,
                       const LocCode& locCode,
                       const VendorCode& vendorCode = EMPTY_VENDOR,
                       const ZoneType zoneType = RESERVED,
                       GeoTravelType geoTvlType = GeoTravelType::International,
                       LocUtil::ApplicationType applType = LocUtil::OTHER,
                       const DateTime& ticketingDate = DateTime::localTime());
  virtual bool isInLoc(const Loc& loc,
                       const LocTypeCode& locTypeCode,
                       const LocCode& locCode,
                       const VendorCode& vendorCode = EMPTY_VENDOR,
                       const ZoneType zoneType = RESERVED,
                       LocUtil::ApplicationType applType = LocUtil::OTHER,
                       GeoTravelType geoTvlType = GeoTravelType::International,
                       const CarrierCode& carrier = "",
                       const DateTime& ticketingDate = DateTime::localTime());
  void setUseBKGExceptionIndex();
};

} // End namespace tse

