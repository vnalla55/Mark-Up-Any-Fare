// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once
#include "Util/BranchPrediction.h"

namespace tax
{

namespace type
{

enum class ExemptTag : unsigned char
{
  Exempt = 'X',
  Blank = ' '
};

enum class RtnToOrig : unsigned char
{
  ReturnToOrigin = 'Y',
  NotReturnToOrigin = 'N',
  ContinuousJourney1stPoint = 'C',
  ContinuousJourney2ndPoint = 'D',
  Blank = ' '
};

enum class UnticketedTransfer : unsigned char
{
  Yes = 'Y',
  No = 'N'
};

enum class TicketedPointTag : unsigned char
{
  MatchTicketedAndUnticketedPoints = 'U',
  MatchTicketedPointsOnly = ' '
};

enum class JrnyInd : unsigned char
{
  JnyLoc2DestPointForOWOrTurnAroundForRTOrOJ = ' ',
  JnyLoc2DestPointForOWOrRTOrOJ = 'A'
};

enum class LocType : unsigned char
{
  Blank = ' ',
  Area = 'A',
  City = 'C',
  Airport = 'P',
  Nation = 'N',
  StateProvince = 'S',
  Miscellaneous = 'M',
  ZoneReserved = 'Z',
  UserZone178 = 'U'
};

enum class TaxPointTag : unsigned char
{
  Sale = 'S',
  Departure = 'D',
  Arrival = 'A',
  Delivery = 'X'
};

enum class TransferTypeTag : unsigned char
{
  Blank = ' ',
  Interline = 'A',
  OnlineWithChangeOfFlightNumber = 'B',
  OnlineWithChangeOfGauge = 'C',
  OnlineWithNoChangeOfGauge = 'D'
};

enum class TravelDateAppTag : unsigned char
{
  Blank = ' ',
  Journey = 'J',
  TaxPoint = 'T'
};

enum class StopoverTag : unsigned char
{
  Blank = ' ',
  Connection = 'C',
  Stopover = 'S',
  FareBreak = 'F',
  NotFareBreak = 'N'
};

enum class Loc2StopoverTag : unsigned char
{
  Blank = ' ',
  Stopover = 'S',
  FareBreak = 'F',
  Furthest = 'T'
};

enum class OpenSegmentIndicator : unsigned char
{
  Fixed = ' ',
  DateFixed = 'T',
  Open = 'D'
};

enum class ConnectionsTag : unsigned char
{
  Blank = ' ',
  TurnaroundPointForConnection = 'A',
  TurnaroundPoint = 'B',
  FareBreak = 'C',
  FurthestFareBreak = 'D',
  GroundTransport = 'E',
  DifferentMarketingCarrier = 'F',
  Multiairport = 'G',
  DomesticToInternational = 'H',
  InternationalToDomestic = 'I',
  InternationalToInternational = 'J'
};

enum class IntlDomInd : unsigned char
{
  Blank = ' ',
  Domestic = 'D',
  International = 'I'
};

enum class StopoverTimeUnit : unsigned char
{
  Blank = ' ',
  Minutes = 'N',
  Hours = 'H',
  Days = 'D',
  Months = 'M',
  HoursSameDay = 'P'
};

enum class AdjacentIntlDomInd : unsigned char
{
  Blank = ' ',
  AdjacentStopoverDomestic = 'D',
  AdjacentDomestic = 'E',
  AdjacentStopoverInternational = 'I',
  AdjacentInternational = 'J'
};

enum class TaxPointLoc2Compare : unsigned char
{
  Blank = ' ',
  Point = 'X',
  Stopover = 'Y'
};

enum class TaxPointLoc3GeoType : unsigned char
{
  Blank = ' ',
  Stopover = 'N',
  Point = 'P'
};

enum class FormOfPayment : unsigned char
{
  Blank = ' ',
  Government = 'G',
  Miles = 'M'
};

enum class PercentFlatTag : unsigned char
{
  Flat = 'F',
  Percent = 'P'
};

enum class TaxApplicationLimit : unsigned char
{
  OnceForItin = '1',
  FirstTwoPerContinuousJourney = '2',
  OncePerSingleJourney = '3',
  Unused = '4',
  FirstTwoPerUSRoundTrip = '5',
  Unlimited = ' '
};

enum class CarrierApplicationIndicator : unsigned char
{
  Positive = ' ',
  Negative = 'X'
};

enum class ServiceBaggageAppl : unsigned char
{
  Positive = 'Y',
  Negative = 'X',
  Blank = ' '
};

enum class TaxableUnitTag : unsigned char
{
  Blank = ' ',
  Applies = 'X'
};

enum class TaxableUnit : unsigned char
{
  Blank = 0,
  YqYr = 1,
  TicketingFee = 2,
  OCFlightRelated = 3,
  OCTicketRelated = 4,
  OCMerchandise = 5,
  OCFareRelated = 6,
  BaggageCharge = 7,
  TaxOnTax = 8,
  Itinerary = 9,
  ChangeFee = 10
};

enum class OptionalServiceTag : unsigned char
{
  Blank = ' ',
  PrePaid = 'P',
  FlightRelated = 'F',
  TicketRelated = 'T',
  Merchandise = 'M',
  FareRelated = 'R',
  BaggageCharge = 'C'
};

enum class TktValApplQualifier : unsigned char
{
  Blank = ' ',
  BaseFare = 'A',
  FareWithFees = 'B'
};

enum class PtcApplTag : unsigned char
{
  Blank = ' ',
  Permitted = 'Y',
  NotPermitted = 'X'
};

enum class PassengerStatusTag : unsigned char
{
  Blank = ' ',
  Employee = 'E',
  National = 'N',
  Resident = 'R'
};

enum class LocationType : unsigned char
{
  Blank = ' ',
  Nation = 'N',
  Province = 'P'
};

enum class PtcMatchIndicator : unsigned char
{
  Input = 'I',
  Output = 'O'
};

enum class NetRemitApplTag : unsigned char
{
  Blank = ' ',
  Applies = 'S'
};

enum class SectorDetailAppl : unsigned char
{
  Positive = 'Y',
  Negative = 'X',
  Blank = ' '
};

enum class AlaskaZone : unsigned char
{
  A = 'A',
  B = 'B',
  C = 'C',
  D = 'D',
  Blank = ' '
};

enum class SectorDetailApplTag : unsigned char
{
  Blank = ' ',
  AnySectorLoc1Loc2 = 'A',
  AnySectorOrginDestination = 'B',
  FirstInternationalSector = 'I',
  EverySectorInItin = 'J',
  AllSectorLoc1Loc2 = 'N'
};
enum class CalcRestriction : unsigned char
{
  ExemptAllTaxesAndFees = 'A',
  ExemptAllTaxes = 'T',
  ExemptSpecifiedTaxes = 'E',
  CalculateOnlySpecifiedTaxes = 'C',
  Blank = ' '
};

enum class TaxRemittanceId : unsigned char
{
  Sale = 'S',
  Use = 'U'
};

enum class CabinCode : unsigned char
{
  Blank = ' ',
  PremiumFirst = 'R',
  First = 'F',
  PremiumBusiness = 'J',
  Business = 'C',
  PremiumEconomy = 'P',
  Economy = 'Y'
};
enum class TravelAgencyIndicator : unsigned char
{
  Blank = ' ',
  Agency = 'X'
};

enum class TaxAppliesToTagInd : unsigned char
{
  Blank = ' ',
  AllBaseFare = 'A',
  LowestFromFareList = 'B',
  BetweenFareBreaks = 'C',
  DefinedByTaxProcessingApplTag = 'D',
  USDeduct = 'E',
  FullyApplicableRoundTripFare = 'F',
  BeforeDiscount = 'G',
  MatchingSectorDetail = 'H'
};

enum class ServiceBaggageApplTag : unsigned char
{
  Blank = ' ',
  C = 'C',
  E = 'E'
};

enum class PaidBy3rdPartyTag : unsigned char
{
  Blank = ' ',
  Government = 'G',
  Miles = 'M'
};

enum class CodeType : unsigned char
{
  Blank = ' ',
  AgencyPCC = 'T',
  AgencyNumber = 'I',
  Department = 'X',
  CarrierDept = 'V',
  ElecResServProvider = 'E',
  LNIATA = 'L',
  AirlineSpec = 'A'
};

enum class TaxRoundingUnit : unsigned char
{
  Blank = ' ',
  Million = 'M',
  HundredThousand = 'U',
  TenThousand = 'E',
  Thousand = 'S',
  Hundred = 'H',
  Ten = 'T',
  Ones = '0',
  OneDigit = '1',
  TwoDigits = '2',
  ThreeDigits = '3',
  FourDigits = '4',
  NoRounding = 'N'
};

enum class TaxRoundingDir : unsigned char
{
  Blank = ' ',
  RoundUp = 'U',
  RoundDown = 'D',
  Nearest = 'N',
  NoRounding = 'S'
};

enum class TaxRoundingDefaultDir : unsigned char
{
  Blank = ' ',
  RoundUp = 'U',
  RoundDown = 'D',
  Nearest = 'N',
  NoRounding = 'O'
};

enum class TaxRoundingPrecision : unsigned char
{
  ToUnits = '1',
  ToFives = '5'
};

enum class GlobalDirection : unsigned char
{
  NO_DIR = 0, /* None */
  AF = 1,   /* via Africa */
  AL,       /* FBR - AllFares incl. EH/TS */
  AP,       /* via Atlantic and Pacific */
  AT,       /* via Atlantic */
  CA,       /* Canada */
  CT,       /* circle trip */
  DI,       /* special USSR - TC3 app. British Airways */
  DO,       /* domestic */
  DU,       /* special USSR - TC2 app. British Airways */
  EH,       /* within Eastern Hemisphere */
  EM,       /* via Europe - Middle East */
  EU,       /* via Europe */
  FE,       /* Far East */
  IN,       /* FBR for intl. incl. AT/PA/WH/CT/PV */
  ME,       /* via Middle East (other than Aden) */
  NA,       /* FBR for North America incl. US/CA/TB/PV */
  NP,       /* via North or Central Pacific */
  PA,       /* via South, Central or North Pacific */
  PE,       /* TC1 - Central/Southern Africa via TC3 */
  PN,       /* between TC1 and TC3 via Pacific and via North America */
  PO,       /* via Polar Route */
  PV,       /* PR/VI - US/CA */
  RU,       /* Russia - Area 3 */
  RW,       /* round the world */
  SA,       /* South Atlantic only */
  SN,       /* via South Atlantic (routing permitted in 1 direc. via N./Mid-Atlantic) */
  SP,       /* via South Polar */
  TB,       /* transborder */
  TS,       /* via Siberia */
  TT,       /* Area 2 */
  US,       /* intra U.S. */
  WH,       /* within Western Hemisphere */
  XX,       /* Universal */
  ZZ        /* Any Global */
};

enum class ForcedConnection : unsigned char
{
  Blank = ' ',
  Connection = 'X',
  Stopover = 'O'
};

enum class ProcessingGroup : unsigned char
{
  OC = 0,
  OB,
  ChangeFee,
  Itinerary,
  Baggage,
  GroupsCount
};

enum class ViewBookTktInd : unsigned char
{
  ViewBookTkt = '1',
  ViewOnly = '2'
};

enum class TaxOrChargeTag : unsigned char
{
  Blank = ' ',
  Tax = 'T',
  Charge = 'C'
};

enum class RefundableTag : unsigned char
{
  Blank = ' ',
  Yes = 'Y',
  No = 'N',
  NoButReuseable = 'R',
  Partial = 'P'
};

enum class AccountableDocTag : unsigned char
{
  Blank = ' ',
  ReportBundle = 'Y',
  ReportSeparate = 'Z',
  NotReported = 'N',
  NonTkdOrSale = 'U'
};

enum class Directionality : unsigned char
{
  Blank     = ' ',
  From      = '1',
  To        = '2',
  Between   = '3',
  Within    = '4',
  Both      = '5',
  Origin    = '6',
  Terminate = '7'
};

enum class OutputTypeIndicator : unsigned char
{
  OnlyTTBS           = 'T',
  OnlyRATD           = 'R',
  BothTTBSAndRATD    = 'B',
  NeitherTTBSNorRATD = 'N'
};

enum class VatInclusiveInd : unsigned char
{
  Blank = ' ',
  Includes = 'I',
  Excludes = 'X'
};

enum class LocExceptTag : unsigned char
{
  Blank = ' ',
  Yes = 'Y',
  No = 'N'
};

enum class TicketingExceptTag : unsigned char
{
  Blank = ' ',
  Yes = 'Y',
  No = 'N'
};

enum class ReissueLocTypeTag : unsigned char
{
  Blank = 0,
  IataArea = 'I',
  SubArea = '*',
  Market = 'C',
  Nation = 'N',
  StateProvince = 'S',
  Zone = 'Z'
};

enum class ReissueRefundableTag : unsigned char
{
  Blank = ' ',
  Refundable = 'Y',
  NonRefundable = 'N'
};

} // namespace type

} // namespace tax
