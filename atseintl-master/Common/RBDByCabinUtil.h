//-------------------------------------------------------------------------------
// Copyright 2015, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

namespace tse
{
class PricingTrx;
class AirSeg;
class PaxTypeFare;
class ClassOfService;
class Diag187Collector;
class RBDByCabinInfo;
class Cabin;
class CabinType;
class LocKey;
class Loc;
class DateTime;
class BookingCodeExceptionSegment;

class RBDByCabinUtil
{
public:
  friend class RBDByCabinUtilTest;

  RBDByCabinUtil(PricingTrx& trx, RBDByCabinCall call, Diag187Collector* diag = nullptr);
  RBDByCabinUtil(const RBDByCabinUtil&) = delete;
  RBDByCabinUtil& operator=(const RBDByCabinUtil&) = delete;
  virtual ~RBDByCabinUtil() = default;

  void invoke187Diagnostic();
  // XformClientShoppingXML
  const Cabin* getCabinForAirseg(const AirSeg& seg);

  const Cabin* getCabinByRBD(const CarrierCode& carrier,
                             const BookingCode& bookingCode,
                             const AirSeg& seg,
                             bool ancilBagServices = false,
                             const DateTime date = DateTime());

  const Cabin* getCabinByRBD(const CarrierCode& carrier,
                             const BookingCode& bookingCode,
                             const DateTime& date);

  // calls from FBCV for the Cat31
  void getCabinsByRbd(const PaxTypeFare& ptf, std::vector<ClassOfService>& classOfService);

  // calls from ItinAnalyzer for fake TS
  void getCabinsByRbd(const AirSeg& airSeg,
                      std::vector<ClassOfService*>& classOfService);

  // PricingDssResponseHandler or AtaeResponseHandler
  void getCabinsByRbd(AirSeg& airSeg,
                      const std::vector<BookingCode>& bks,
                      std::vector<ClassOfService*>* cosVec = nullptr);

  // BCEV WPNCS
  CabinType getCabinByRbdByType(const AirSeg& airSeg,
                                const BookingCode& bc,
                                const PaxTypeFare& paxTypeFare,
                                const uint32_t& itemNo,
                                const BookingCodeExceptionSegment& bceSegment);

  // PricingModelMap
  void getCabinByRBD(AirSeg& seg);

  virtual const std::vector<RBDByCabinInfo*>&
                                  getRBDByCabin(const VendorCode& vendor,
                                                const CarrierCode& carrier,
                                                DateTime& tvlDate);
private:
  static constexpr Indicator CHAR_BLANK = ' ';

  void storeRbdByCabinForDiag(const RBDByCabinInfo* rbdByCabin);
  bool isProgramCallSelected() const;
  bool isCpaSelected() const;
  bool isCabinSelected();
  bool isSequenceSelected(const RBDByCabinInfo* rbdByCabin) const;
  bool isFlightNumberOrEquipmentCoded(const RBDByCabinInfo* rbdByCabin) const;
  bool isCxrSelected(const CarrierCode& carrier) const;
  bool isBookingCodeSelected(const BookingCode& bookCode) const;
  bool isFareClassOrBasisSelected(const PaxTypeFare& ptf) const;

  void createDiag187();
  bool needToPrintDiag(const BookingCode& bookingCode,
                       const CarrierCode carrier = CarrierCode()) const;
  bool needToPrintDiagNoRBD() const;
  void printHeader() const;
  void printRecordHeader() const;
  void printAirSegSearchData(const AirSeg& seg, 
                             const DateTime& tvlDate,
                             bool open=false);
  void printAirSegSearchData(const DateTime& tvlDate,
                             bool open=false);

  void printAirSegSearchData(const CarrierCode& carrier,
                             const BookingCode& bookingCode,
                             const DateTime& tvlDate,
                             bool open = false);
  void printAirSegSearchData(const std::vector<BookingCode>& bks,
                             const DateTime& tvlDate);
  void printAirSegSearchData(std::vector<ClassOfService*>& classOfService,
                             const DateTime& tvlDate);
  void printAirSegSearchData(const PaxTypeFare& ptf,
                             std::vector<ClassOfService>& classOfService);
  void printAirSegSearchData(const BookingCode& bookingCode,
                             const PaxTypeFare& paxTypeFare,
                             const uint32_t& itemNo,
                             const BookingCodeExceptionSegment& bceSegment,
                             const DateTime& tvlDate,
                             bool open = false);
  void printCurrentRBD(const BookingCode& bookingCode);
  void printRbdByCabinInfo(const RBDByCabinInfo* rbdByCabin) const;
  void printReqCabinNotMatch(const Indicator& cabin, const BookingCode& bookingCode) const;

  void printCabinNotFoundForRBD(const BookingCode& bookingCode) const;
  void printDefaultEconomyCabin(const BookingCode& bookingCode) const;
  void printCabinFoundForRBD(const BookingCode& bookingCode,
                             const Indicator& cabin,
                             const CarrierCode& carrier,
                             bool defaultCabin = false) const;
  void printFailFoundCabinForRBD(const BookingCode& bookingCode,
                                 const CarrierCode& carrier) const;
  void printRBDByCabinNotFoundForCarrier(const CarrierCode& carrier) const;
  void printRBDinRQNotMatchRBDinSeg(const BookingCode& bookingCode) const;
  void printAncBagRBDNotFiledInCxrRbdByCabin(const RBDByCabinInfo& rbdInfo,
                                             const BookingCode& bookingCode) const;
  void printFailStatus(const RBDByCabinStatus& status, const BookingCode& bookingCode) const;
  void finishDiag();
  const Cabin* findCabinByRBD(const CarrierCode& carrier,
                              const BookingCode& bookingCode,
                              DateTime& tvlDate,
                              const DateTime& ticketIssueDate);

  void printRBDByCabin(CarrierCode carrier);
  bool checkTravelDate(const RBDByCabinInfo& rbdInfo,
                       const DateTime& tvlDate) const;
  bool checkTicketDate(const RBDByCabinInfo& rbdInfo,
                       const DateTime& date) const;
  bool checkGlobalInd(const RBDByCabinInfo& rbdInfo);
  bool checkLocations(const RBDByCabinInfo& rbdInfo);
  bool checkFlights(const RBDByCabinInfo& rbdInfo);
  bool checkEquipment(const RBDByCabinInfo& rbdInfo);
  bool validateLoc1(const RBDByCabinInfo& rbdInfo, const Loc& orig, const Loc& dest);
  bool validateBothLocs(const RBDByCabinInfo& rbdInfo, const Loc& orig, const Loc& dest);
  bool validateLocation(const VendorCode& vendor,
                        const LocKey& locKey,
                        const Loc& loc,
                        CarrierCode carrier);
  virtual bool isInZone(const VendorCode& vendor,
                        const LocCode& zone,
                        const Loc& loc,
                        CarrierCode carrier);
  virtual bool isInLoc(const VendorCode& vendor,
                       const LocKey& locKey,
                       const Loc& loc,
                       CarrierCode carrier);
  const Cabin* validateRBDByCabin(const std::vector<RBDByCabinInfo*>& cabinInfos,
                                  const BookingCode& bookingCode,
                                  const DateTime& tvlDate,
                                  const DateTime& ticketIssueDate);

  Indicator getRBDCabin_old(const RBDByCabinInfo& rbdInfo, const BookingCode& bookingCode) const;
  Indicator getRBDCabin(const RBDByCabinInfo& rbdInfo, const BookingCode& bookingCode) const;
  const Cabin* prepareCabin(const BookingCode& bookingCode, const Indicator RBDcabin=CHAR_BLANK) const;

  GeoTravelType setsUpGeoTravelType();
  void setsUpSegInds(const AirSeg& seg, bool fake = false, bool ancilBagServices = false);
  void setsUpFMInds(const PaxTypeFare& ptf);

  // Data members
  PricingTrx& _trx;
  Diag187Collector* _diag;
  bool _ancilBagServices = false; // Ancillary/baggage services
  bool _segmentVSfareComp = false;
  const AirSeg* _airSeg = nullptr;
  const PaxTypeFare* _ptf = nullptr;
  const RBDByCabinInfo* _cabinInfoForDiag = nullptr;
  RBDByCabinCall _call;
  std::vector<const TravelSeg*> _travelSegs;
};
} // namespace tse
