//----------------------------------------------------------------------------
//
//  Description: Diagnostic 187 formatter
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

#include "Diagnostic/DiagCollector.h"

namespace tse
{
class PricingTrx;
class BookingCodeExceptionSegment;

class Diag187Collector : public DiagCollector
{
public:
  friend class Diag187CollectorTest;

  explicit Diag187Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag187Collector() {}

  void printHeader(const std::string& carrierReq,
                   const std::string& cityPair,
                   const DateTime& tktDate);
  void printHeader(const RBDByCabinCall& call);
  void printAirSegData(const AirSeg& seg, const DateTime& date, const DateTime& TktDate, bool& open);

  void printAirSegData(const AirSeg& seg,
                       const DateTime& date,
                       const std::vector<BookingCode>& bks,
                       const DateTime& tktDate);
  void printAirSegData(const PaxTypeFare& ptf,
                       const DateTime& date,
                       const std::vector<BookingCode>& bks,
                       const DateTime& tktDate);

  void printAirSegData(const AirSeg& seg,
                       const CarrierCode& carrier,
                       const BookingCode& bookingCode,
                       const DateTime& date,
                       const DateTime& tktDate,
                       bool& open,
                       const std::vector<BookingCode>& bks);
  void printAirSegData(const AirSeg& seg,
                       const BookingCode& bookingCode,
                       const DateTime& date,
                       const PaxTypeFare& paxTypeFare,
                       const uint32_t& itemNo,
                       const BookingCodeExceptionSegment& bceSegment,
                       const DateTime& tktDate,
                       bool& open);
  void printRecordHeader();
  void printRecordHeaderShort();
  void printBookingCode(const BookingCode& bookingCode,
                        const std::vector<BookingCode>& bks);
  void printCurrentRBD(const BookingCode& bookingCode);

  void printRbdByCabinInfo(const PricingTrx& trx,
                           const RBDByCabinInfo* rbdByCabin,
                           bool skip = false);

  void printRbdByCabinInfoShort(const RBDByCabinInfo* rbdByCabin);

  void printReqCabinNotMatch(const Indicator& cabin,
                             const BookingCode& bookingCode);

  void printCabinNotFoundForRBD(const BookingCode& bookingCode);
  void printDefaultEconomyCabin();
  void printCabinFoundForRBD(const BookingCode& bookingCode,
                             const Indicator& cabin,
                             const CarrierCode& carrier,
                             bool defaultCabin = false);
  void printFailFoundCabinForRBD(const BookingCode& bookingCode,
                                 const CarrierCode& carrier);
  void printRBDByCabinNotFoundForCarrier(const CarrierCode& carrier);
  void printRBDinRQNotMatchRBDinSeg(const BookingCode& bkgRQ, const BookingCode& bkgSeg);
  void printAncBagRBDNotFiledInCxrRbdByCabin(const RBDByCabinInfo& rbdInfo,
                                             const BookingCode& bookingCode);

  void printFailStatus(const RBDByCabinStatus& status);
  void printFailStatusShort(const RBDByCabinStatus& status);
  void printStatus(const RBDByCabinStatus& status);

  void printFooter();

private:
  void displayCabin(Indicator cab, std::vector<BookingCode>& rbdVector);
  void displayZoneDetail(const PricingTrx& trx,
                         const VendorCode& vendor,
                         const Zone& zone);
  virtual const ZoneInfo* getZone(const PricingTrx& trx,
                                  const VendorCode& vendor,
                                  const Zone& zoneNo);
  void displayLocType(const Indicator& locType);
  void displayDirectionalInd(const Indicator& dir);
  void displayExclIncl(const Indicator& ei);
};

} // namespace tse

