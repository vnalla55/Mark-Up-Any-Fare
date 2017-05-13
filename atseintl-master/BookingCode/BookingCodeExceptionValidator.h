//-----------------------------------------------------------------------------
//
//  File:     BookingCodeExceptionValidator.h
//            **This is NOT a C++ Exception **
//
//  Author :  Kul Shekhar
//            Linda Dillahunty
//
//  Copyright Sabre 2004
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the agreement/contract
//          under which the program(s) have been supplied.
//
//-----------------------------------------------------------------------------

#pragma once

#include "DataModel/PaxTypeFare.h"
#include "DBAccess/BookingCodeExceptionSequence.h"

namespace tse
{
// The first element is the sequence number and
// the second element is the segment number that passed.
// first can also contain BCE_FLT_NOMATCH      = -1;
// BCE_FLT_FAILED       = -2;

using TravelSegResult = std::pair<int32_t, int16_t>;
// a vector the same size as TravelSegVector
// to hold passing BCE segment numbers.
using TravelSegmentResultVector = std::vector<TravelSegResult>;

class PricingTrx;
class DiagCollector;
class Diag405Collector;
class AirSeg;
class TravelSeg;
class Fare;
class FarePath;
class FareUsage;
class PricingUnit;
class ClassOfService;
class BCETuning;
class Itin;
class BookingCodeExceptionIndex;

using TravelSegVectorCI = std::vector<TravelSeg*>::const_iterator;

enum BCEReturnTypes
{ BCE_PASS = 1,
  BCE_NEXT_FLT,
  BCE_NEXT_SEQUENCE,
  BCE_FROMTO_PRI };

/**
 * BookingCodeExceptionValidator --
 * Reservation Booking Designator (RBD) Table999 --
 * BookingCodeExceptionValidator
 *
 * This class determines the validity of the booking code in flight
 * segments against RBD Table 999. Validity can be checked against
 * the FareComponent, the PricingUnit or the Journey depending on flags
 * in the Table 999 records. This class uses Diagnostic 405 to output
 * all of its information.
 */

class BookingCodeExceptionValidator
{
  friend class BookingCodeExceptionValidatorTest;
  friend class BookingCodeExceptionValidatorRtwTest;

public:
  BookingCodeExceptionValidator(const Itin& itin,
                                bool partOfLocalJny,
                                bool skipFlownSegCat31,
                                bool useBKGExceptionIndex)
    : _partOfLocalJny(partOfLocalJny),
      _itin(itin),
      _skipFlownSegCat31(skipFlownSegCat31),
      _useBKGExceptionIndex(useBKGExceptionIndex)
  {
  }

  virtual ~BookingCodeExceptionValidator() = default;

  const static int16_t BCE_FLT_NOMATCH;
  const static int16_t BCE_FLT_FAILED = -2;
  const static int16_t BCE_FLT_SKIPPED = -3;
  const static int16_t BCE_SEG_IFTAG = -4;
  const static int16_t BCE_SEC_ARNK = -5;

  const static uint32_t BCE_ALL_SEQUENCES = 0;

  static constexpr char BCE_CHAR_BLANK = ' ';
  static constexpr char BCE_IF_FARECOMPONENT = '1';
  static constexpr char BCE_IF_ANY_TVLSEG = '2';

  static constexpr char BCE_PRIME = 'X';
  static constexpr char BCE_FARE_CONSTRUCTED = 'C';
  static constexpr char BCE_FARE_SPECIFIED = 'S';

  static constexpr char BCE_FLTINDIVIDUAL = 'I';
  static constexpr char BCE_FLTRANGE = 'R';

  const static CarrierCode BCE_DOLLARDOLLARCARRIER;
  const static CarrierCode BCE_XDOLLARCARRIER;
  const static CarrierCode BCE_ANYCARRIER;

  static constexpr char BCE_SEG_PRIMARY = 'P';
  static constexpr char BCE_SEG_SECONDARY = 'S';
  static constexpr char BCE_SEG_FROMTO_PRIMARY = 'T';

  const static EquipmentType BCE_EQUIPBLANK;
  const static TSICode BCE_NO_TSI;
  const static TSICode BCE_POS_TSI33;

  static constexpr char BCE_PERMITTED = 'P';
  static constexpr char BCE_REQUIRED = 'R';
  static constexpr char BCE_PERMITTED_IF_PRIME_NOT_OFFER = 'O';
  static constexpr char BCE_PERMITTED_IF_PRIME_NOT_AVAIL = 'A';
  static constexpr char BCE_REQUIRED_IF_PRIME_NOT_OFFER = 'G';
  static constexpr char BCE_REQUIRED_IF_PRIME_NOT_AVAIL = 'H';
  static constexpr char BCE_REQUIRED_WHEN_OFFERED = 'W';
  static constexpr char BCE_REQUIRED_WHEN_AVAIL = 'V';
  static constexpr char BCE_RBD2_PERMITTED_IF_RBD1_AVAILABLE = 'B';
  static constexpr char BCE_RBD2_REQUIRED_IF_RBD1_AVAILABLE = 'D';
  static constexpr char BCE_ADDITIONAL_DATA_APPLIES = 'U';
  static constexpr char BCE_STANDBY = 'S';
  static constexpr char BCE_NOT_PERMITTED = 'X';
  static constexpr char BCE_DOES_NOT_EXIST = 'N';

  enum BCEDiagMsgs
  { IFTAG_FAIL = 1,
    SKIP_FLT,
    PRIME_FAIL,
    SEGMENT_PASS,
    SEGMENT_SKIP,
    XDOLLAR_CXR_FAIL,
    CXR_FAIL_NXTSEQ,
    CXR_FAIL_NXTFLT,
    PRIMRY_SECNDRY_NXTSEQ,
    PRIMRY_SECNDRY_NXTFLT,
    FLT_INDIVIDUAL_FAIL,
    FLT_RANGE_FAIL,
    EQUIPMENT_FAIL,
    PORTION_OF_TVL_FAIL,
    TSI_PU_FAIL,
    TSI_PU_BAD,
    TSI_FC_REVAL,
    TSI_FC_BAD,
    TSI_FC_FAIL,
    TSI_FC_BAD2,
    LOC_FAIL_NXTSEQ,
    LOC_FAIL_NXTFLT,
    POINT_OF_SALE_FAIL,
    SOLD_TAG_FAIL,
    IFTAG_PASS,
    NOT_FIRST_MATCHING_SEQ,
    NO_SEQ_MATCHED_IN_RULE1,
    SEQTYPE_CONSTRSPEC_FARE,
    TUNING_SEQ_FAIL,
    TUNING_SEGM_FAIL,
    SEQ_FAIL_ANALYZE_FM_LOC,
    SEG_FAIL_BCE_FROMTO_PRI,
    FLT_PASS_BCE,
    FLT_FAIL_BCE,
    FLT_NOMATCH_BCE,
    FLT_FAIL_CABIN,
    SKIP_FLT_CAT25,
    SEQ_MISS_DUAL_RBD,
    SKIP_SEQ_REST_B_D_R6C1,
    SKIP_SEQ_REST_B_D_CAT25
    };

  enum BCEErroMsgs
  { SEQTYPE_CONSTR_FARE = 1,
    SEQTYPE_SPEC_FARE,
    SEQ_SEG_DUAL_RBD
  };

  using COSInnerPtrVecIC = std::vector<ClassOfService*>::const_iterator;
  using DiagParamMap = std::map<std::string, std::string>;
  using DiagParamMapVecI = std::map<std::string, std::string>::iterator;
  using DiagParamMapVecIC = std::map<std::string, std::string>::const_iterator;

  enum ErrorCode
  { NO_ERR = 0,
    FARE_LOC_NULL,
    FLT_LOC_NULL,
    FAIL_DATE,
    FAIL_DOW,
    FAIL_TIME };

  /**
    * method - validate() Table 999 processing for following two scenarios :
    * 1. Record 1 and Record 6 Convention 2 when the pointer pAirSeg is NULL.
    * 2. Record 6 Convention 1 when pointer pAirSeg has a valid pointer value
    * This method is called by FareBookingCodeValidator
    * @param trx -- the transaction information.
    * @param bceSequenceList -- the BookingCodeExceptionSequenceList from the DB.
    * @param PaxTypeFare -- the paxTypeFare will provide the Fare and the Travel Segments
    * @return true or false
    */

  bool validate(PricingTrx& trx,
                const BookingCodeExceptionSequenceList& bceSequenceList,
                PaxTypeFare& paxTypeFare,
                AirSeg* pAirSeg,
                BCETuning& bceTuning,
                FareUsage* fu,
                bool r1T999 = false,
                bool cat25PrimeSector = false);

protected:
  virtual bool validateWP(PricingTrx& trx,
                          const BookingCodeExceptionSegment& bceSegment,
                          const TravelSeg* tvlSeg,
                          PaxTypeFare::SegmentStatusVec& segStatVec,
                          const uint16_t airIndex,
                          PaxTypeFare& paxTypeFare,
                          bool rbd2Only = false);

  virtual bool validateWPNC(const BookingCodeExceptionSegment& bceSegment,
                            const TravelSeg* tvlSeg,
                            PaxTypeFare::SegmentStatusVec& segStatVec,
                            const uint16_t airIndex,
                            PaxTypeFare& paxTypeFare,
                            PricingTrx& trx,
                            bool rbd2Only = false);

  virtual BookingCodeValidationStatus
  validateWPNC_NEW(const BookingCodeExceptionSegment& bceSegment,
                   const TravelSeg* tvlSeg,
                   PaxTypeFare::SegmentStatusVec& segStatVec,
                   const uint16_t airIndex,
                   PaxTypeFare& paxTypeFare,
                   PricingTrx& trx,
                   bool rbd2Only = false);

  void setRebookRBD(PricingTrx& trx,
                    PaxTypeFare& paxTypeFare,
                    const TravelSeg* tvlSeg,
                    PaxTypeFare::SegmentStatusVec& segStatVec,
                    uint16_t airIndex,
                    const ClassOfService& cs);

  virtual void rebookSegmentWPNC(PricingTrx& trx,
                                 PaxTypeFare& paxTypeFare,
                                 uint16_t airIndex,
                                 const ClassOfService& cs,
                                 PaxTypeFare::SegmentStatusVec& segStatVec);

private:
  enum StatusType
  { STATUS_RULE1 = 1,
    STATUS_RULE1_AS_BOOKED,
    STATUS_RULE2,
    STATUS_RULE2_AS_BOOKED,
    STATUS_JORNY,
    STATUS_JORNY_AS_BOOKED };

  static const char* errorCodes[];
  std::set<CarrierCode> collectCarriersForChainsLookup(PaxTypeFare& paxTypeFare, bool chart2);
  /**
    * method - validateSequence()  loops thru sequences and validates its segment vector.
    * @param trx -- the transaction information.
    * @param bceSequenceList -- the BookingCodeExceptionSequenceList from the DB.
    * @param PaxTypeFare -- the paxTypeFare will provide the Fare and the Travel Segments
    * @param diag -- the diagnostic collector
    * @return void
    */
  void validateSequence(PricingTrx& trx,
                        const BookingCodeExceptionSequenceList& bceSequenceList,
                        PaxTypeFare& paxTypeFare,
                        const AirSeg* pAirSeg);
  int validateSequenceList(PricingTrx& trx,
                           const BookingCodeExceptionSequenceList& bceSequenceList,
                           PaxTypeFare& paxTypeFare,
                           const AirSeg* pAirSeg);
  int legacyValidateSequenceList(PricingTrx& trx,
                                 const BookingCodeExceptionSequenceList& bceSequenceList,
                                 PaxTypeFare& paxTypeFare,
                                 const AirSeg* pAirSeg);
  bool validateSingleSequence(PricingTrx& trx,
                              const BookingCodeExceptionSequence& bceSequence,
                              PaxTypeFare& paxTypeFare,
                              const AirSeg* pAirSeg);
  // Analyze sequences and find the 1st one that will be used to change the 1st
  // char of the YY Fare Basis Code
  void analyzeSequencies(PricingTrx& trx,
                         const BookingCodeExceptionSequenceList& bceSequenceList,
                         PaxTypeFare& paxTypeFare,
                         const AirSeg* pAirSeg);

  // Analyze segments and find the 1st one that will be used to change the 1st
  // char of the YY Fare Basis Code
  bool analyzeSeg(PaxTypeFare& paxTypeFare, const BookingCodeExceptionSegment& bceSegment);

  bool analyzeFMLoc(PricingTrx& trx,
                    const PaxTypeFare& paxTypeFare,
                    const BookingCodeExceptionSequence& bceSequence,
                    const BookingCodeExceptionSegment& bceSegment) const;

  bool validateCarrier(const BookingCodeExceptionSequence& bceSequence,
                       const BookingCodeExceptionSegment& bceSegment,
                       const PaxTypeFare& fare,
                       const AirSeg& airSeg);

  bool
  validateFCType(const BookingCodeExceptionSegment& bceSegment, const PaxTypeFare& paxTypeFare);

  // validate sequence for precense of both RBDs if the Restriction tag 'B' and 'D'
  // 'RBD T999 Dual Inventory project'
  bool validateSequenceForBothRBDs(const BookingCodeExceptionSequence& bceSequence);

  /**
    * method - validateSegment()  loops thru segments and validates each object.
    * @param trx -- the transaction information.
    * @param bceSequence -- the BookingCodeExceptionSequence being processed.
    * @param PaxTypeFare -- the paxTypeFare will provide the Fare and the Travel Segments
    * @param iFlt -- is -1 when validSegement is called for fareComponent validation, else
    *                if its PU revalidation time then it contains FlightIndex.
    * @param pfarePath -- pinter to a farePath
    * @param pPU -- pointer to a Pricing Unit object
    * @param diag -- the diagnostic collector
    * @return void
    */
  void validateSegment(PricingTrx& trx,
                       const BookingCodeExceptionSequence& bceSequence,
                       PaxTypeFare& paxTypeFare,
                       int16_t iFlt,
                       FarePath* pfarePath,
                       PricingUnit* pPU);

  /**
    * method - validateCarrier()  validates Carrier
    * @param bceSequence -- the BookingCodeExceptionSequence being processed.
    * @param bceSegment  -- the BookingCodeExceptionSegment being processed.
    * @param fare -- the fare being tested
    * @param airSeg -- the air segment being tested
    * @param diag -- the diagnostic collector
    * @return BCEReturnTypes  BCE_PASS, BCE_NEXT_FLT or BCE_NEXT_SEQUENCE
    */
  BCEReturnTypes validateCarrier(const BookingCodeExceptionSequence& bceSequence,
                                 const BookingCodeExceptionSegment& bceSegment,
                                 const Fare& fare,
                                 const AirSeg& airSeg,
                                 const int16_t iFlt,
                                 const uint16_t fltIndex);

  /**
    * method - validatePrimarySecondary()  validates PrimarySecondary
    * @param bceSequence -- the BookingCodeExceptionSequence being processed.
    * @param bceSegment  -- the BookingCodeExceptionSegment being processed.
    * @param fare -- the fare being tested
    * @param primarySector -- pointer to the primary TravelSeg of this farecomponent
    * @param travelSeg -- pointer to the TravelSeg beig tested
    * @param diag -- the diagnostic collector
    * @return BCEReturnTypes BCE_PASS, BCE_NEXT_FLT or BCE_NEXT_SEQUENCE
    */
  BCEReturnTypes validatePrimarySecondary(const BookingCodeExceptionSequence& bceSequence,
                                          const BookingCodeExceptionSegment& bceSegment,
                                          const Fare& fare,
                                          const TravelSeg* primarySector,
                                          const TravelSeg* travelSeg);

  /**
    * method - validateFlights()  validates Flights
    * @param bceSequence  -- the BookingCodeExceptionSequence being processed.
    * @param bceSegment  -- the BookingCodeExceptionSegment being processed.
    * @param airSeg -- the air segment being tested
    * @param diag -- the diagnostic collector
    * @return BCEReturnTypes  BCE_PASS or BCE_NEXT_FLT
    */
  BCEReturnTypes validateFlights(const BookingCodeExceptionSequence& bceSequence,
                                 const BookingCodeExceptionSegment& bceSegment,
                                 const AirSeg& airSeg);

  /**
    * method - validateEquipment()  validate Equipment
    * @param bceSequence  -- the BookingCodeExceptionSequence being processed.
    * @param bceSegment  -- the BookingCodeExceptionSegment being processed.
    * @param airSeg -- the air segment being tested
    * @param diag -- the diagnostic collector
    * @return BCEReturnTypes  BCE_PASS or BCE_NEXT_FLT
    */
  BCEReturnTypes validateEquipment(const BookingCodeExceptionSequence& bceSequence,
                                   const BookingCodeExceptionSegment& bceSegment,
                                   const AirSeg& airSeg);

  /**
    * method - validatePortionOfTravel()  validate PortionOfTravel
    * @param bceSequence -- the BookingCodeExceptionSequence being processed.
    * @param bceSegment  -- the BookingCodeExceptionSegment being processed.
    * @param fare -- the fare being tested
    * @param airSeg -- the air segment being tested
    * @param diag -- the diagnostic collector
    * @return BCEReturnTypes  BCE_PASS or BCE_NEXT_FLT
    */
  BCEReturnTypes validatePortionOfTravel(const BookingCodeExceptionSequence& bceSequence,
                                         const BookingCodeExceptionSegment& bceSegment,
                                         const Fare& fare,
                                         const AirSeg& airSeg);

  /**
    * method - validateTSI()  validate TSI
    * @param bceSequence -- the BookingCodeExceptionSequence being processed.
    * @param bceSegment  -- the BookingCodeExceptionSegment being processed.
    * @param trx -- the transaction information.
    * @param travelSeg -- pointer to the current travelseg
    * @param diag -- the diagnostic collector
    * @return BCEReturnTypes BCE_PASS or BCE_NEXT_FLT
    */
  BCEReturnTypes validateTSI(const BookingCodeExceptionSequence& bceSequence,
                             const BookingCodeExceptionSegment& bceSegment,
                             PricingTrx& trx,
                             PaxTypeFare& paxTypeFare,
                             const TravelSeg* travelSeg,
                             const FarePath* pfarePath,
                             const PricingUnit* pPU);

  /**
    * method - validateLocation()  validateLocation
    * @param bceSequence -- the BookingCodeExceptionSequence being processed.
    * @param bceSegment  -- the BookingCodeExceptionSegment being processed.
    * @param paxTypeFare -- the fare being tested
    * @param airSeg -- the air segment being tested
    * @param diag -- the diagnostic collector
    * @return BCEReturnTypes  BCE_PASS, BCE_NEXT_FLT or BCE_NEXT_SEQUENCE
    */
  BCEReturnTypes validateLocation(PricingTrx& trx,
                                  const BookingCodeExceptionSequence& bceSequence,
                                  const BookingCodeExceptionSegment& bceSegment,
                                  const PaxTypeFare& paxTypeFare,
                                  const AirSeg& airSeg);

  /**
    * method - validatePointOfSale()  validatePointOfSale
    * @param bceSequence  -- the BookingCodeExceptionSequence being processed.
    * @param bceSegment  -- the BookingCodeExceptionSegment being processed.
    * @param trx -- the transaction information.
    * @param airSeg -- the air segment being tested
    * @param diag -- the diagnostic collector
    * @return BCEReturnTypes  BCE_PASS or BCE_NEXT_FLT
    */
  BCEReturnTypes validatePointOfSale(const BookingCodeExceptionSequence& bceSequence,
                                     const BookingCodeExceptionSegment& bceSegment,
                                     PricingTrx& trx,
                                     const AirSeg& airSeg);

  /**
    * method - validateSoldTag()  validateSoldTag
    * @param bceSequence  -- the BookingCodeExceptionSequence being processed.
    * @param bceSegment  -- the BookingCodeExceptionSegment being processed.
    * @param trx -- the transaction information.
    * @param diag -- the diagnostic collector
    * @return BCEReturnTypes  BCE_PASS or BCE_NEXT_FLT
    */
  BCEReturnTypes validateSoldTag(const BookingCodeExceptionSequence& bceSequence,
                                 const BookingCodeExceptionSegment& bceSegment,
                                 PricingTrx& trx);

  /**
    * method - validateDateTimeDOW()  validateDateTimeDOW
    * @param bceSequence -- the BookingCodeExceptionSequence being processed.
    * @param bceSegment  -- the BookingCodeExceptionSegment being processed.
    * @param trx -- the transaction information.
    * @param paxTypeFare -- the fare being tested
    * @param airSeg -- the air segment being tested
    * @param diag -- the diagnostic collector
    * @return BCEReturnTypes  BCE_PASS, BCE_NEXT_FLT or BCE_NEXT_SEQUENCE
    */
  BCEReturnTypes validateDateTimeDOW(const BookingCodeExceptionSequence& bceSequence,
                                     const BookingCodeExceptionSegment& bceSegment,
                                     PricingTrx& trx,
                                     const PaxTypeFare& paxTypeFare,
                                     const AirSeg& airSeg);

  /**
    * method - validateFareclassType(...)  validateFareclassType
    * @param bceSequence -- the BookingCodeExceptionSequence being processed.
    * @param bceSegment  -- the BookingCodeExceptionSegment being processed.
    * @param fare -- the fare being tested
    * @param airSeg -- the air segment being tested
    * @param diag -- the diagnostic collector
    * @return BCEReturnTypes  BCE_PASS, BCE_NEXT_FLT or BCE_NEXT_SEQUENCE
    */
  BCEReturnTypes validateFareclassType(const BookingCodeExceptionSequence& bceSequence,
                                       const BookingCodeExceptionSegment& bceSegment,
                                       const PaxTypeFare& paxTypeFare,
                                       const AirSeg& airSeg);

  void applyBookingCodeForSingleFlight(PricingTrx& trx,
                                       const BookingCodeExceptionSequence& bceSequence,
                                       PaxTypeFare& paxTypeFare,
                                       TravelSeg* tvlSeg,
                                       uint16_t airIndex,
                                       int16_t iFlt);

  bool isSegmentNoMatched(int16_t segMatch[], uint16_t segSize) const;
  bool isAllFlightsMatched(int16_t fltMatch[], uint16_t fltSize) const;
  bool isAllFlightsSkipped(int16_t fltMatch[], uint16_t fltSize) const;
  void
  saveSegStatusResults(PaxTypeFare& paxTypeFare, PaxTypeFare::SegmentStatusVec& prevSegStatusVec);
  void restoreSegStatusResults(PaxTypeFare& paxTypeFare,
                               PaxTypeFare::SegmentStatusVec& prevSegStatusVec);

  void processRestrictionTag(PricingTrx& trx,
                             const uint32_t& itemNo,
                             const BookingCodeExceptionSegment& bceSegment,
                             const TravelSeg* tvlSeg,
                             PaxTypeFare::SegmentStatusVec& segStatVec,
                             uint16_t airIndex,
                             PaxTypeFare& paxTypeFare,
                             int16_t iFlt);

  void permittedTagP(PricingTrx& trx,
                     const BookingCodeExceptionSegment& bceSegment,
                     const TravelSeg* tvlSeg,
                     PaxTypeFare::SegmentStatusVec& segStatVec,
                     uint16_t airIndex,
                     PaxTypeFare& paxTypeFare,
                     bool rbd2Only = false);

  void requiredTagR(PricingTrx& trx,
                    const BookingCodeExceptionSegment& bceSegment,
                    const TravelSeg* tvlSeg,
                    PaxTypeFare::SegmentStatusVec& segStatVec,
                    uint16_t airIndex,
                    PaxTypeFare& paxTypeFare);

  void permittedIfPrimeNotOfferTagO(PricingTrx& trx,
                                    const BookingCodeExceptionSegment& bceSegment,
                                    const TravelSeg* tvlSeg,
                                    PaxTypeFare::SegmentStatusVec& segStatVec,
                                    uint16_t airIndex,
                                    PaxTypeFare& paxTypeFare);

  void permittedIfPrimeNotAvailTagA(PricingTrx& trx,
                                    const BookingCodeExceptionSegment& bceSegment,
                                    const TravelSeg* tvlSeg,
                                    PaxTypeFare::SegmentStatusVec& segStatVec,
                                    uint16_t airIndex,
                                    PaxTypeFare& paxTypeFare);

  void requiredIfPrimeNotOfferTagG(PricingTrx& trx,
                                   const BookingCodeExceptionSegment& bceSegment,
                                   const TravelSeg* tvlSeg,
                                   PaxTypeFare::SegmentStatusVec& segStatVec,
                                   uint16_t airIndex,
                                   PaxTypeFare& paxTypeFare);

  void requiredIfPrimeNotAvailTagH(PricingTrx& trx,
                                   const BookingCodeExceptionSegment& bceSegment,
                                   const TravelSeg* tvlSeg,
                                   PaxTypeFare::SegmentStatusVec& segStatVec,
                                   uint16_t airIndex,
                                   PaxTypeFare& paxTypeFare);

  void requiredWhenOfferTagW(PricingTrx& trx,
                             const BookingCodeExceptionSegment& bceSegment,
                             const TravelSeg* tvlSeg,
                             PaxTypeFare::SegmentStatusVec& segStatVec,
                             uint16_t airIndex,
                             PaxTypeFare& paxTypeFare);

  void requiredWhenAvailTagV(PricingTrx& trx,
                             const BookingCodeExceptionSegment& bceSegment,
                             const TravelSeg* tvlSeg,
                             PaxTypeFare::SegmentStatusVec& segStatVec,
                             uint16_t airIndex,
                             PaxTypeFare& paxTypeFare);

  void rbd2PermittedWhenRbd1AvailTagB(PricingTrx& trx,
                             const BookingCodeExceptionSegment& bceSegment,
                             const TravelSeg* tvlSeg,
                             PaxTypeFare::SegmentStatusVec& segStatVec,
                             uint16_t airIndex,
                             PaxTypeFare& paxTypeFare);

  void rbd2RequiredWhenRbd1AvailTagD(PricingTrx& trx,
                             const BookingCodeExceptionSegment& bceSegment,
                             const TravelSeg* tvlSeg,
                             PaxTypeFare::SegmentStatusVec& segStatVec,
                             uint16_t airIndex,
                             PaxTypeFare& paxTypeFare);

  void additionalDataApplyTagU(PricingTrx& trx,
                               const BookingCodeExceptionSegment& bceSegment,
                               const TravelSeg* tvlSeg,
                               PaxTypeFare::SegmentStatusVec& segStatVec,
                               uint16_t airIndex,
                               PaxTypeFare& paxTypeFare);

  void standbyTagS(PricingTrx& trx,
                   const BookingCodeExceptionSegment& bceSegment,
                   const TravelSeg* tvlSeg,
                   PaxTypeFare::SegmentStatusVec& segStatVec,
                   uint16_t airIndex,
                   PaxTypeFare& paxTypeFare);

  void notPermittedTagX(PricingTrx& trx,
                        const BookingCodeExceptionSegment& bceSegment,
                        const TravelSeg* tvlSeg,
                        PaxTypeFare::SegmentStatusVec& segStatVec,
                        uint16_t airIndex,
                        PaxTypeFare& paxTypeFare);

  void notExistTagN(PricingTrx& trx,
                    const BookingCodeExceptionSegment& bceSegment,
                    const TravelSeg* tvlSeg,
                    PaxTypeFare::SegmentStatusVec& segStatVec,
                    uint16_t airIndex,
                    PaxTypeFare& paxTypeFare);

  bool validateBC(PricingTrx& trx,
                  const BookingCodeExceptionSegment& bceSegment,
                  const TravelSeg* tvlSeg,
                  PaxTypeFare::SegmentStatusVec& segStatVec,
                  uint16_t airIndex,
                  PaxTypeFare& paxTypeFare,
                  bool rbd2Only=false);

  BookingCodeValidationStatus validateBC_NEW(PricingTrx& trx,
                                             const BookingCodeExceptionSegment& bceSegment,
                                             const TravelSeg* tvlSeg,
                                             PaxTypeFare::SegmentStatusVec& segStatVec,
                                             uint16_t airIndex,
                                             PaxTypeFare& paxTypeFare,
                                             bool rbd2Only=false);

  // virtual bool validateWP(PricingTrx& trx,
  //                const BookingCodeExceptionSegment&   bceSegment,
  //                const TravelSeg* tvlSeg,
  //                PaxTypeFare::SegmentStatusVec& segStatVec,
  //                const uint16_t airIndex);

  // virtual bool validateWPNC(const BookingCodeExceptionSegment&   bceSegment,
  //                  const TravelSeg* tvlSeg,
  //                  PaxTypeFare::SegmentStatusVec& segStatVec,
  //                  const uint16_t airIndex,
  //                  PaxTypeFare& paxTypeFare,
  //                  PricingTrx& trx);

  void tagFltNOMATCH(PaxTypeFare::SegmentStatusVec& segStatVec, const uint16_t airIndex);

  void tagFltNOMATCH_NEW(PaxTypeFare::SegmentStatusVec& segStatVec,
                         const uint16_t airIndex,
                         BookingCodeValidationStatus validationStatus);

  void tagFltFAIL(PaxTypeFare::SegmentStatusVec& segStatVec,
                  const uint16_t airIndex,
                  BookingCodeValidationStatus validationStatus);

  void tagFltFAILTagN(PaxTypeFare& paxTypeFare,
                      PaxTypeFare::SegmentStatusVec& segStatVec,
                      const uint16_t airIndex,
                      PricingTrx& trx);
  /**
    * method - getSegment()   get a pointer to the Segment that passed a flight
    * @param bceSequence  -- the BookingCodeExceptionSequence being processed.
    * @param segmentNo    -- the segment number that passed.
    * @return BookingCodeExceptionSegment* pointer to the segment
    */
  BookingCodeExceptionSegment*
  getSegment(const BookingCodeExceptionSequence& bceSequence, const int16_t segmentNo) const;

  /**
    * method - resetFltResult()   reset the FltResult for a sequence
    * @param seqNo -- the sequence number -- it can be set to 0 for all sequences
    * @param airSegSize -- the number of elements in the TravelSegVector.
    * @return void
    */
  void resetFltResult(const uint32_t seqNo, const uint32_t airSegSize);

  /**
    * method - isPrimeOfferred()  check to see if the Prime RBD from the fare is offered
    * @param paxTypeFare -- the paxTypeFare will provide the Fare and the Travel Segments
    * @param pAirSeg     -- the air segment being tested
    * @return true or false.
    */
  bool isPrimeOffered(PricingTrx& trx,
                      const TravelSeg* tvlSeg,
                      PaxTypeFare::SegmentStatusVec& segStatVec,
                      uint16_t airIndex,
                      PaxTypeFare& paxTypeFare);
  /**
    * method - isPrimeAvailable()  check to see if the Prime RBD from the fare is available
    * @param paxTypeFare -- the paxTypeFare will provide the Fare and the Travel Segments
    * @param pAirSeg     -- the air segment being tested
    * @return true or false.
    */
  bool isPrimeAvailable(PricingTrx& trx,
                        const TravelSeg* tvlSeg,
                        PaxTypeFare::SegmentStatusVec& segStatVec,
                        uint16_t airIndex,
                        PaxTypeFare& paxTypeFare);

  /**
    * method - isBceOffered()   check to see if the bookingCode is offered.
    * @param bookCode -- booking code to check
    * @param pAirSeg  -- the air segment being tested
    * @return true or false.
    */
  bool isBceOffered(const BookingCodeExceptionSegment& bceSegment,
                    PricingTrx& trx,
                    const TravelSeg* tvlSeg,
                    PaxTypeFare::SegmentStatusVec& segStatVec,
                    uint16_t airIndex,
                    PaxTypeFare& paxTypeFare);

  /**
    * method - isBceAvailable()   check to see if the bookingCode is Available.
    * @param bookCode -- booking code to check
    * @param pAirSeg  -- the air segment being tested
    * @return true or false.
    */
  bool isBceAvailable(const BookingCodeExceptionSegment& bceSegment,
                      PricingTrx& trx,
                      const TravelSeg* tvlSeg,
                      PaxTypeFare::SegmentStatusVec& segStatVec,
                      uint16_t airIndex,
                      PaxTypeFare& paxTypeFare,
                      bool checkRbd1Only = false);

  bool allFltsDone(const int16_t iFlt);

  void createDiag(PricingTrx& trx, const PaxTypeFare& paxTfare);
  Diag405Collector* diag405();
  void endDiag();

  void diagHeader(PricingTrx& trx, const AirSeg* pAirSegConv1, PaxTypeFare& paxTypeFare);

  void diagResults(PricingTrx& trx, PaxTypeFare& paxTypeFare, const AirSeg* pAirSegConv1);

  void doDiag(const BookingCodeExceptionSequence& bceSequence,
              const BookingCodeExceptionSegment& bceSegment,
              const Fare& fare,
              const AirSeg* airSeg,
              BCEDiagMsgs diagMsg);

  void doDiag(const BookingCodeExceptionSequence& bceSequence,
              const BookingCodeExceptionSegment& bceSegment,
              PricingTrx& trx,
              BCEDiagMsgs diagMsg);

  void doDiag(const BookingCodeExceptionSequence& bceSequence,
              const BookingCodeExceptionSegment& bceSegment,
              uint16_t fltIndex,
              const AirSeg* airSeg,
              BCEDiagMsgs diagMsg);

  void diagApplyBc(PricingTrx& trx,
                   const BookingCodeExceptionSequence& bceSequence,
                   const BookingCodeExceptionSegment& bceSegment);

  void
  diagBc(const TravelSeg* tvlSeg, const PaxTypeFare::SegmentStatus& segStat, BCEDiagMsgs diagMsg);

  void doDiag(const AirSeg* airSeg, uint16_t fltIndex);

  void doDiag(const BookingCodeExceptionSequence& bceSequence, 
              const PaxTypeFare& fare, BCEErroMsgs errMsg);

  void doDiag(const BookingCodeExceptionSequence& bceSequence);
  void doDiag(uint16_t bceSeqSize, uint16_t seqProcessed);
  void doDiag();

  void otherDiag(BCEDiagMsgs diagMsg);

  void diagIndex(PricingTrx& trx,
                 const BookingCodeExceptionIndex& index,
                 const FareType& ft,
                 const FareClassCode& fcc);

  void setFirstLastAirSeg(PaxTypeFare& paxTypeFare);

  bool tryRule2(PricingTrx& trx, PaxTypeFare& paxTypeFare) const;

  BookingCodeExceptionValidator(const BookingCodeExceptionValidator& ref);
  BookingCodeExceptionValidator& operator=(const BookingCodeExceptionValidator& ref);

  bool flowJourneyCarrier(PricingTrx& trx, const TravelSeg* tvlSeg) const;

  bool localJourneyCarrier(PricingTrx& trx, const TravelSeg* tvlSeg) const;

  std::vector<ClassOfService*>* flowMarketAvail(PricingTrx& trx,
                                                const TravelSeg* tvlSeg,
                                                FareMarket* fm,
                                                uint16_t airIndex);

  std::vector<ClassOfService*>* pricingAvail(PricingTrx& trx, const TravelSeg* tvlSeg);

  std::vector<ClassOfService*>* shoppingAvail(PricingTrx& trx,
                                              const TravelSeg* tvlSeg,
                                              FareMarket* fm,
                                              uint16_t airIndex);

  bool tryLocalWithFlowAvail(PricingTrx& trx, PaxTypeFare& paxTypeFare) const;

  bool validateStartDate(const DateTime& fltDate,
                         const uint16_t bceYear,
                         const uint16_t bceMonth,
                         const uint16_t bceDay) const;

  bool validateStopDate(const DateTime& fltDate,
                        const uint16_t bceYear,
                        const uint16_t bceMonth,
                        const uint16_t bceDay) const;

  bool setISegment(uint16_t segmentIndex, const AirSeg* airSeg);

  bool journeyExistInItin(PricingTrx& trx) const;

  bool partOfJourney(PricingTrx& trx, const TravelSeg* tvlSeg) const;

  void validateWPNCS(PricingTrx& trx,
                     const uint32_t& itemNo,
                     const BookingCodeExceptionSegment& bceSegment,
                     const TravelSeg* tvlSeg,
                     PaxTypeFare::SegmentStatusVec& segStatVec,
                     uint16_t airIndex,
                     PaxTypeFare& paxTypeFare,
                     int16_t iFlt);

  CabinType cabinWpncs(PricingTrx& trx,
                       const AirSeg& airSeg,
                       const BookingCode& bc,
                       const PaxTypeFare& paxTypeFare,
                       const uint32_t& itemNo,
                       const BookingCodeExceptionSegment& bceSegment) const;

  const std::vector<ClassOfService*>* getCosFromFlowCarrierJourneySegment(const TravelSeg* tvlSeg);

  const std::vector<ClassOfService*>* getAvailability(PricingTrx& trx,
                                                      const TravelSeg* travelSeg,
                                                      PaxTypeFare::SegmentStatus& segStat,
                                                      PaxTypeFare& paxTypeFare,
                                                      uint16_t airIndex);

  void resetFirstMatchingSeqs(const PaxTypeFare& paxTypeFare);

  PaxTypeFare::SegmentStatusVec& getSegStatusVec(PaxTypeFare& paxTypeFare);

  void validateAsBooked(PricingTrx& trx,
                        const BookingCodeExceptionSequenceList& bceSequenceList,
                        PaxTypeFare& paxTypeFare,
                        const AirSeg* pAirSeg);

  bool doAsBooked(PricingTrx& trx, PaxTypeFare& paxTypeFare, const AirSeg* pAirSegConv1);
  bool statusToAsBooked();

  bool statusFromAsBooked();
  bool isAsBookedStatus() const
  {
    return _statusType == STATUS_RULE1_AS_BOOKED || _statusType == STATUS_RULE2_AS_BOOKED ||
           _statusType == STATUS_JORNY_AS_BOOKED;
  }

  void resetAsBooked(const PaxTypeFare& paxTypeFare);

  void adjustStatus(PricingTrx& trx, PaxTypeFare& paxTypeFare, const AirSeg* pAirSegConv1);

  bool checkBookedClassAvail(PricingTrx& trx, const TravelSeg* tvlSeg, bool dual=false) const;

  bool checkBookedClassOffer(PricingTrx& trx, const TravelSeg* tvlSeg) const;

  bool validateWPNCSFromPrimeRBD(const BookingCodeExceptionSegment& bceSegment) const;

  bool allSegsStatusQF(PricingTrx& trx, PaxTypeFare& paxTypeFare, const AirSeg* pAirSegConv1) const;

  bool skipCat31Flown(PricingTrx& trx, const TravelSeg* tvlSeg) const;

  void validateCabinForDifferential(PricingTrx& trx,
                                    const TravelSeg* tvlSeg,
                                    PaxTypeFare& paxTypeFare) const;

  void setFlagForBcvStatus(PaxTypeFare::BkgCodeSegStatus& segStatus,
                           BookingCodeValidationStatus validationStatus);

  void BcValidation(PricingTrx& trx,
                    const BookingCodeExceptionSegment& bceSegment,
                    const TravelSeg* tvlSeg,
                    PaxTypeFare::SegmentStatusVec& segStatVec,
                    uint16_t airIndex,
                    PaxTypeFare& paxTypeFare,
                    BCEDiagMsgs diagMsg,
                    bool rbd2Only = false,
                    BookingCodeValidationStatus forceBcvs = BOOKING_CODE_STATUS_NOT_SET);
  bool isWaitlist(const TravelSeg& tvlSeg) const
  {
    return (tvlSeg.realResStatus() == LL_RES_STATUS || tvlSeg.realResStatus() == BL_RES_STATUS ||
            tvlSeg.realResStatus() == DL_RES_STATUS || tvlSeg.realResStatus() == GL_RES_STATUS ||
            tvlSeg.realResStatus() == HL_RES_STATUS || tvlSeg.realResStatus() == HN_RES_STATUS ||
            tvlSeg.realResStatus() == PN_RES_STATUS || tvlSeg.realResStatus() == TL_RES_STATUS ||
            tvlSeg.realResStatus() == WL_RES_STATUS || tvlSeg.realResStatus() == DS_REAL_RES_STATUS);
  }

  struct RtwPreferredCabinContext
  {
    TravelSegmentResultVector prevFltResults;
    PaxTypeFare::SegmentStatusVec prevSegStats;
    std::vector<CabinType> prevRtwPC;
  };

  bool isRtwPreferredCabinApplicable(PricingTrx& trx, const PaxTypeFare& ptf) const;
  void initRtwPreferredCabin(PricingTrx& trx, const PaxTypeFare& ptf);
  void prepareSequenceValidationRtw(RtwPreferredCabinContext& context, PaxTypeFare& ptf);
  void checkSequenceValidationRtw(const RtwPreferredCabinContext& context,
                                  const BookingCodeExceptionSequence& bceSeq,
                                  PaxTypeFare& ptf);
  void updateRtwPreferredCabin(const BookingCodeExceptionSegment& matchedBceSegment,
                               PaxTypeFare& ptf,
                               uint32_t fltIndex);
  bool shouldEnterRtwPrefCabin(const BookingCodeExceptionSegment* matchedBceSegment,
                               const PaxTypeFare::SegmentStatus& segStat,
                               CabinType preferredCabin) const;
  CabinType getPassedCabin(const PaxTypeFare::SegmentStatus& segStat, const TravelSeg& ts) const;

  void checkPriceByCabin(PricingTrx& trx,
                         PaxTypeFare::SegmentStatusVec& segStatVec,
                         uint16_t airIndex,
                         const TravelSeg* tvlSeg,
                         PaxTypeFare& paxTypeFare);

  void validateRBDRec1PriceByCabin(PricingTrx& trx,
                                   const TravelSeg* tvlSeg,
                                   uint16_t airIndex,
                                   std::vector<BookingCode>& primeBookingCodeVec,
                                   PaxTypeFare& paxTypeFare,
                                   PaxTypeFare::SegmentStatusVec& segStatVec);

  // for each TravelSegment, an entry in this vector will
  // contain  first element is the sequence number and
  // the second element is the  segment number passed.
  // first can also contain BCE_FLT_NOMATCH      = -1;
  // BCE_FLT_FAILED       = -2;
  TravelSegmentResultVector _fltResultVector;
  std::vector<CabinType> _rtwPreferredCabin;
  DiagCollector* _diag = nullptr;
  AirSeg* _firstAirsegInMkt = nullptr;
  AirSeg* _lastAirsegInMkt = nullptr;
  std::vector<int32_t> _firstMatchingSeqs;

  bool _bkgYYupdate = false;
  bool _shortDiag = false;
  BCETuning* _bceTuning = nullptr;
  uint16_t _iSequence = 0;
  uint16_t _iSegment = 0;
  bool _stopTuning = false;
  bool _partOfLocalJny;
  const Itin& _itin;
  FareUsage* _fu = nullptr;
  bool _cat25PrimeSector = false;
  bool _rec1T999Cat25R3 = false;
  StatusType _statusType = StatusType::STATUS_RULE1;
  PaxTypeFare::SegmentStatusVec _asBookedStatus;
  char _restTagProcessed = 0;
  bool _skipFlownSegCat31;
  bool _useBKGExceptionIndex;
};
} // end tse namespace
