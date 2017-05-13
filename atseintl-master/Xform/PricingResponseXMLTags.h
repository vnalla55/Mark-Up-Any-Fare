//-------------------------------------------------------------------
//
//  File:        PricingResponseXMLTags.h
//  Created:     May 5, 2005
//  Authors:     Andrea Yang
//
//  Description: All known WP XML2 Response tags
//
//  Updates:
//
//  Copyright Sabre 2005
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

#include <string>

namespace tse
{
namespace xml2
{
// !!!!----------------------------------------------------------------
// lines commented out are still with XML1 value, find the XML2 value
// before uncommented out
// !!!!----------------------------------------------------------------

// Combinations Tags
// static const std::string NumberOfCombinations = "NBC";
// static const std::string Combos = "COM";
// static const std::string TotalNumberDecimals = "TND";

//
extern const std::string RequestType;
extern const std::string RequestTypeAdvancedReservation;
extern const std::string SecondaryRequestType;

extern const std::string LastTicketTime;
// static const std::string PricesCurrentItin = "PCT";
// static const std::string ChangeClassRebook = "RBK";
// static const std::string TotalTimeVariance = "TTV";
// static const std::string TotalServiceLevel = "TSL";
// static const std::string OpaqueOption = "FOO";
// static const std::string DCATimedOut = "DTO";
// static const std::string DCAThrottled = "DCT";
// static const std::string AltCityOption = "ACO";
// static const std::string PublishedOption = "POO";                     // Use for Hotwire
// static const std::string WebBasedFare = "WEB";
// static const std::string WebCommissionFare = "WCF";
// static const std::string WebNonCommissionFare = "WNC";
// static const std::string BulkFare = "BLK";                                    // Mojito
// static const std::string ETicketable = "ETK";                         // Mojito phase II
// static const std::string DivideParty = "DIV";

// Ticketing Agent Tags
extern const std::string TicketingAgentInfo;
extern const std::string AgentCity;
extern const std::string TvlAgencyPCC;
extern const std::string MainTvlAgencyPCC;
extern const std::string TvlAgencyIATA;
extern const std::string HomeAgencyIATA;
extern const std::string AgentFunctions;
extern const std::string AgentDuty;
extern const std::string AirlineDept;
extern const std::string CxrCode;
extern const std::string CurrencyCodeAgent;
extern const std::string CoHostID;
extern const std::string AgentCommissionType;
extern const std::string AgentCommissionAmount;
extern const std::string VendorCrsCode;
extern const std::string OfficeDesignator;
extern const std::string OfficeCode;
extern const std::string AirlineChannelId;

// Billing Information Tags
extern const std::string BillingInformation;
extern const std::string UserPseudoCityCode;
extern const std::string UserStation;
extern const std::string UserBranch;
extern const std::string PartitionID;
extern const std::string UserSetAddress;
extern const std::string ServiceName;
extern const std::string AaaCity;
extern const std::string AaaSine;
extern const std::string ActionCode;
extern const std::string NodeId;
extern const std::string TrxStartTime;
extern const std::string TrxEndTime;
extern const std::string TrxCPU;
extern const std::string TrxRC;
extern const std::string TrxID;
extern const std::string ClientTrxID;
extern const std::string ParentTrxID;
extern const std::string SolutionsRequested;
extern const std::string SolutionsProduced;
extern const std::string ClientSvcName;
extern const std::string ParentSvcName;
extern const std::string RequestPath;

// Summary Tags
extern const std::string SummaryInfo;
extern const std::string TotalPriceAll;
extern const std::string TotalCurrencyCode;
extern const std::string NetRemitTotalPriceAll;
extern const std::string TicketingDate;
extern const std::string TicketingTime;
extern const std::string IATASalesCode;
extern const std::string SalesLocation;
extern const std::string TicketPointOverride;
extern const std::string AdvancedPurchaseDate;
extern const std::string ValidatingCarrier;
extern const std::string LastTicketDay;
extern const std::string PurchaseByDate;
extern const std::string SimultaneousRes;
extern const std::string PrivateFareIndication;
extern const std::string ServerHostname;
extern const std::string FareCalcTooLong;
extern const std::string ConsolidatorPlusUpCurrencyCode;
extern const std::string ConsolidatorPlusUpAmount;
extern const std::string ExchangeReissueIndicator;
extern const std::string RoeBSRDate;
extern const std::string RoeBSRTime;
extern const std::string SegmentFeeMessage;
extern const std::string AncillariesNonGuarantee;
extern const std::string BrandSoldout;
extern const std::string NoBrandsOffered;
extern const std::string AdjustedSellingLevel;
extern const std::string DataView;

// Validating Carrier List

extern const std::string VcxrInfo;
extern const std::string InhibitPq;
extern const std::string SettlementMethod;
extern const std::string AtseProcess;
extern const std::string MultiNeutralValCxr;
extern const std::string IETCntryCode;


// Default Validating Carrier
extern const std::string DefaultValidatingCxr;
extern const std::string ValidatingCxrCode;
extern const std::string TicketType;

// Alternate Validating Carrier
extern const std::string AlternateValidatingCxr;

// Participating Carrier
extern const std::string ParticipatingCxr;
extern const std::string ParticipatingCxrCode;
extern const std::string AgreementTypeCode;

// Flight Tags
// static const std::string FlightSegmentInfo = "FSI";
// static const std::string FlightSegmentOrderNumber = "FSN";
// static const std::string AirlineCode = "CXR";
// static const std::string AFlightNumber = "FLN";
// static const std::string DepartureDate = "DDT";
// static const std::string DepartureTime = "DTM";                                       // in
// minutes
// static const std::string DepartureTimeZone = "DTZ";
// static const std::string DepartureDayOfWeek = "DDW";
// static const std::string ArrivalDate = "ADT";
// static const std::string ArrivalTime = "ATM";                                 // in minutes
// static const std::string ArrivalTimeZone = "ATZ";
// static const std::string ArrivalDayOfWeek = "ADW";
// static const std::string BoardAirport = "BRD";
// static const std::string OffAirport = "OFF";
// static const std::string ClassOfService = "COS";
// static const std::string CabinCode = "CAB";
// static const std::string NumberFlightStops = "STP";
// static const std::string FlightEquipmentType = "EQC";
// static const std::string OperatingCarrier = "CRO";
// static const std::string MealCode = "FMC";
// static const std::string TotalElapsedTime = "TET";                            // in minutes
// static const std::string TrafficRestrictionCode = "TRC";
// static const std::string DOTRating = "FDR";
// static const std::string FlightIsConnection = "CNX";
// static const std::string FlightIsMarriedConnection = "MNX";
// static const std::string InterlineConnection = "INT";
// static const std::string CallDirectCarrier = "CDC";
// static const std::string OneSegmentCnx = "SG1";
// static const std::string TwoSegmentCnx = "SG2";
// static const std::string ThreeSegmentCnx = "SG3";
// static const std::string FourSegmentCnx = "SG4";
// static const std::string TACarrier = "TAC";
// static const std::string SeamlessCarrier = "SMS";
// static const std::string ElectronicTicket = "ETI";
// static const std::string CodeShareCarrier = "OAL";
// static const std::string DirectConnectParticipant = "DCX";
// static const std::string BBRCarrierItinerary = "BBR";
// static const std::string MidPoint = "MID";
// static const std::string DOTDisclosure = "DOT";
// static const std::string DOTDisclosureText = "DTX";
// static const std::string AvailabilitySeamlessCarrier = "ASM";
// static const std::string AvailabilityType = "FAT";
// static const std::string AvailabilityWarningMessage = "FWM";
// static const std::string OutboundConnection = "OBC";
// static const std::string InboundConnection = "IBC";
// static const std::string MarriedOutboundConnection = "MOC";
// static const std::string MarriedInboundConnection = "MIC";
// static const std::string TimeZoneDifference = "TZD";
// static const std::string AltCityDepartureMiles = "ADM";
// static const std::string AltCityArrivalMiles = "AAM";

// Intermediate Point Tags
// static const std::string IntermediateInformation = "POI";
// static const std::string IntermediateAirportCode = "PAP";
// static const std::string IntermediateArrivalDate = "PAD";
// static const std::string IntermediateArrivalTime = "PAT";
// static const std::string AirMiles = "PAM";
// static const std::string ElapsedTime = "PET";
// static const std::string ElapsedLayover = "PEL";
// static const std::string IntermediateDepartureDate = "PDD";
// static const std::string IntermediateDepartureTime = "PDT";
// static const std::string TimeZone = "PTZ";
// static const std::string IntermediateEquipmentType = "PEQ";

// Pax Fare Tags
extern const std::string PassengerInfo;
extern const std::string PassengerType;
extern const std::string RequestedPassengerType;
extern const std::string NetRemitPublishedFareRetrieved;
extern const std::string ConstructionCurrencyCode;
extern const std::string ConstructionTotalAmount;
extern const std::string NetRemitConstructionCurrencyCode;
extern const std::string NetRemitConstructionTotalAmount;
extern const std::string BaseCurrencyCode;
extern const std::string BaseFareAmount;
extern const std::string NetRemitBaseCurrencyCode;
extern const std::string NetRemitBaseFareAmount;
extern const std::string EquivalentCurrencyCode;
extern const std::string EquivalentAmount;
extern const std::string NetRemitEquivalentAmount;
extern const std::string CurrencyCodeMinimum;
extern const std::string CurrencyCodeMinimumAmount;
extern const std::string StopOverCount;
extern const std::string StopOverCharges;
extern const std::string StopOverPublishedCurrencyCode;
extern const std::string TransferCount;
extern const std::string TransferCharges;
extern const std::string TransferPublishedCurrencyCode;
extern const std::string PaxFareCurrencyCode;
extern const std::string TotalPerPassenger;
extern const std::string TotalPerPassengerPlusImposed;
extern const std::string NetRemitTotalPerPassenger;
extern const std::string TotalPerPassengerPlusAbsorbtion;
extern const std::string TotalTaxes;
extern const std::string TotalTaxesPlusImposed;
extern const std::string NetRemitTotalTaxes;
extern const std::string CommissionPercentage;
extern const std::string CommissionAmount;
extern const std::string CommissionCap;
extern const std::string TravelAgencyTax;
extern const std::string FareCalculation;
extern const std::string NonRefundable;
extern const std::string NonRefundableAmount;
extern const std::string NonRefundableIndicator;
extern const std::string NegotiatedWithCorp;
extern const std::string NegotiatedWithoutCorp;
extern const std::string LocalCurrency;
extern const std::string PaxFarePassengerNumber;
extern const std::string TicketFareVendorSource;
extern const std::string TourCodeDescription;
extern const std::string ValueCode;
extern const std::string NetFareAmount; // CAT35
extern const std::string NetFareAmountPlusCharges; // CAT35
extern const std::string NetFareBaseAmount; // CAT35 Net Total Amt in Base
extern const std::string TourIndicator; // T-Tour, V-Value, C-Car
extern const std::string TextBox;
extern const std::string NetGross;
extern const std::string Cat35CommissionPercentage;
extern const std::string Cat35CommissionAmount;
extern const std::string Cat35RegularNet;

// Commission Management Tags
extern const std::string Cat35MarkupAmount;

// Common tag for both Cat35 and non-Cat35 (calculated from COS) commission.
extern const std::string TotalCommissionAmount;

// Indicate (T) for different commission amount for different validating carriers
extern const std::string DifferentCommissionAmount;
extern const std::string CommissionSourceIndicator; // A-AMC, C-CAT35, M-Manual

extern const std::string BSPMethodType;
extern const std::string Cat35Warning;
extern const std::string Cat35Used;
extern const std::string OverrideCat35;
extern const std::string paperTicketSurchargeMayApply;
extern const std::string paperTicketSurchargeIncluded;
extern const std::string WpnDetails;
extern const std::string BaggageResponse;
extern const std::string WpnOptionNumber;
extern const std::string TicketingRestricted;
extern const std::string TfrRestricted;
extern const std::string PrintOption;
extern const std::string NetRemitTicketIndicator;
extern const std::string TicketingSegmentFeeMsg;
extern const std::string AbacusBillingData;
extern const std::string ContractIssueDate;
extern const std::string ContractNumber;
extern const std::string FareTariff;
extern const std::string FareRule;
extern const std::string DBSource;
extern const std::string FareSourceCarrier;
extern const std::string FareSourcePcc;
extern const std::string FareBasisTktDesig;
extern const std::string FareOrig;
extern const std::string FareDest;
extern const std::string NetConstructionTotalAmount; // CAT35 NET
extern const std::string NetTotalPerPassenger; // CAT35 NET
extern const std::string NetTotalTaxes; // CAT35 NET
extern const std::string SpanishDiscountIndicator;
extern const std::string UsDotItinIndicator;
extern const std::string FopBINNumberApplied;
extern const std::string Cat35TFSFWithNet;
extern const std::string SurfaceRestrictedInd;
extern const std::string TotalFareAmountWithTTypeFees;
extern const std::string TotalTTypeOBFeeAmount;
extern const std::string Cat35BlankCommission;
extern const std::string CommissionBaseAmount;

// Passenger Fare REX Tags
extern const std::string ReissueExchange;
extern const std::string ResidualIndicator;
extern const std::string FormOfRefundIndicator;
extern const std::string UnflownItinPriceInfo;
extern const std::string Tag7PricingSolution;
extern const std::string ElectronicTicketRequired;
extern const std::string ElectronicTicketNotAllowed;
// Passenger Fare Change Fee Tags
extern const std::string ChangeFee;
extern const std::string HighestChangeFeeIndicator;
extern const std::string ChangeFeeAmount;
extern const std::string ChangeFeeCurrency;
extern const std::string ChangeFeeWaived;
extern const std::string ChangeFeeNotApplicable;
extern const std::string ChangeFeeAmountInPaymentCurrency;
extern const std::string ChangeFeeAgentCurrency;

// Segment Tags
extern const std::string SegmentInformation;
extern const std::string SegmentDepartureCity;
extern const std::string SegmentDepartureAirport;
extern const std::string SegmentDepartureDate;
extern const std::string SegmentDepartureTime;
extern const std::string SegmentArrivalDate;
extern const std::string SegmentArrivalTime;
extern const std::string SegmentArrivalCity;
extern const std::string SegmentArrivalAirport;
extern const std::string FareClass;
extern const std::string CityStopoverCharge;
extern const std::string TransferCharge;
extern const std::string ItinSegmentNumber;
extern const std::string RouteTravel;
extern const std::string NotValidBeforeDate;
extern const std::string NotValidAfterDate;
extern const std::string SegmentPassengerNumber; // for baggage
extern const std::string BaggageIndicator; // P/K/L
extern const std::string BaggageValue;
extern const std::string AvailabilityBreak;
extern const std::string SideTripIndicator;
extern const std::string ExtraMileageAllowance;
extern const std::string MileageExclusion;
extern const std::string MileageReduction;
extern const std::string MileageEqualization;
extern const std::string StopoverSegment;
extern const std::string ConnectionSegment;
extern const std::string TransferSegment;
extern const std::string FareBreakPoint;
extern const std::string TurnaroundPoint;
extern const std::string OffPointPartMainComponent;
extern const std::string SideTripEnd_Obsolete;
extern const std::string SideTripEndComponent_Obsolete;
extern const std::string SideTripStart;
extern const std::string SideTripEnd;
extern const std::string UnchargeableSurface;
extern const std::string PureSurfaceSegment;
extern const std::string JourneyType;
extern const std::string SegmentCabinCode;
extern const std::string Cat35FareSegment;
extern const std::string Cat5RequiresRebook;
extern const std::string SegDepartureCountry;
extern const std::string SegArrivalCountry;
extern const std::string LocationType;
extern const std::string FareVendor;
extern const std::string FareCarrier;
extern const std::string FareVendorSource;
extern const std::string SEGEquipmentCode;
extern const std::string OAndDMarketBeginSegOrder;
extern const std::string OAndDMarketEndSegOrder;
extern const std::string MarketingCarrier;
extern const std::string OperatingCarrier;

// Mileage Tags
extern const std::string MileageDisplayInformation;
extern const std::string MileageDisplayType;
extern const std::string MileageDisplayCity;

// Surcharge Tags
extern const std::string SurchargeInformation;
extern const std::string SurchargeType;
extern const std::string SurchargeAmount;
extern const std::string SurchargeCurrencyCode;
extern const std::string SurchargeDescription;
extern const std::string SurchargeDepartureCity;
extern const std::string SurchargeArrivalCity;
extern const std::string SurchargeSurchargeType;

// Fare Calc Tags
extern const std::string FareCalcInformation;
extern const std::string FareCalcDepartureCity;
extern const std::string FareCalcDepartureAirport;
extern const std::string GoverningCarrier;
extern const std::string SecondGoverningCarrier;
extern const std::string TrueGoverningCarrier;
extern const std::string FareCalcArrivalCity;
extern const std::string FareCalcArrivalAirport;
extern const std::string FareAmount;
extern const std::string FareBasisCode;
extern const std::string FareBasisCodeLength;
extern const std::string NetRemitPubFareAmount;
extern const std::string NetRemitPubFareBasisCode;
extern const std::string NetRemitPubFareBasisCodeLength;
extern const std::string DifferentialFareBasisCode;
extern const std::string TicketDisignator;
extern const std::string SnapDesignatorDiscount;
extern const std::string FareCalcCabinCode;
extern const std::string DepartureCountry;
extern const std::string DepartureIATA;
extern const std::string DepartureStateOrProvince;
extern const std::string ArrivalCountry;
extern const std::string ArrivalIATA;
extern const std::string ArrivalStateOrProvince;
extern const std::string PublishedFareAmount;
extern const std::string FareComponentCurrencyCode;
extern const std::string RoundTripFare;
extern const std::string OneWayFare;
extern const std::string OneWayDirectionalFare;
extern const std::string IATAAuthorizedCarrier;
extern const std::string VerifyGeographicalRestrictions;
extern const std::string FailCat15;
extern const std::string FareByRuleSpouseHead;
extern const std::string FareByRuleSpouseAccompany;
extern const std::string FareByRuleSeamanAdult;
extern const std::string FareByRuleSeamanChild;
extern const std::string FareByRuleSeamanInfant;
extern const std::string FareByRuleBTS;
extern const std::string FareByRuleNegotiated;
extern const std::string FareByRuleNegotiatedChild;
extern const std::string FareByRuleNegotiatedInfant;
extern const std::string CommencementDate;
// static const std::string Cat5RequiresRebook = "PAX";
extern const std::string TypeOfFare;
extern const std::string DiscountCode;
extern const std::string DiscountPercentage;
extern const std::string IsMileageRouting;
extern const std::string MileageSurchargePctg;
extern const std::string HIPOrigCity;
extern const std::string HIPDestCity;
extern const std::string ConstructedHIPCity;
extern const std::string SpecialFare;
extern const std::string PricingUnitType;
extern const std::string GlobalDirectionInd;
extern const std::string FareCalcDirectionality;
extern const std::string PricingUnitCount;
extern const std::string CorpId;
extern const std::string CategoryList;
extern const std::string SegmentsCount;
extern const std::string PrivateFareInd;
extern const std::string NetTktFareAmount; // cat35 net
extern const std::string IbfInternalPath;
extern const std::string PublicPrivateFare;
extern const std::string TotalTaxesPerFareComponent;
extern const std::string TotalSurchargesPerFareComponent;
extern const std::string FareComponentPlusTaxesPlusSurcharges;
extern const std::string FareComponentInEquivalentCurrency;
extern const std::string FareRetailerCodeNet;
extern const std::string FareRetailerCodeAdjusted;

// The next sections added for the Enhanced Rule Display project -- command WPRD.
extern const std::string ERDInformation; // Section to be included in CAL for EnhancedRuleDisplay
extern const std::string Link;
extern const std::string Sequence;
extern const std::string CreateDate;
extern const std::string CreateTime;
extern const std::string FCAFareType;
extern const std::string ERDFareClass;
extern const std::string ERDFareClassLength;
extern const std::string ERDFareAmount;
extern const std::string ERDBookingCode;
extern const std::string ERDFareBasisCode;
extern const std::string ERDTicketDesignator;
extern const std::string ERDAccountCode;
extern const std::string ERDFareDepartDate; // Departure date of first seg of this fare.
extern const std::string ERDGoverningCarrier;
extern const std::string ERDCommandPricing;

extern const std::string ERDC25Information; // Section to be included in ERD for Cat25
extern const std::string ERDC25Vendor;
extern const std::string ERDC25ItemNo;
extern const std::string ERDC25Directionality;

extern const std::string ERDC35Information; // Section to be included in ERD for Cat35
extern const std::string ERDC35Vendor;
extern const std::string ERDC35ItemNo;
extern const std::string ERDC35FareDisplayType;
extern const std::string ERDC35Directionality;

extern const std::string ERDDiscountInformation; // Section to be included in ERD for ERDDiscounts
extern const std::string ERDDiscountVendor;
extern const std::string ERDDiscountItemNo;
extern const std::string ERDDiscountDirectionality;

extern const std::string ERDContructedInformation; // Section to be included in ERD for Contructed
extern const std::string ERDGateway1;
extern const std::string ERDGateway2;
extern const std::string ERDConstructionType;
extern const std::string ERDSpecifiedFareAmount;
extern const std::string ERDConstructedNucAmount;

extern const std::string OAOInformation; // Section to be included in ERD for Origin Add on
extern const std::string OAOFootnote1;
extern const std::string OAOFootnote2;
extern const std::string OAOFareClass;
extern const std::string OAOTariff;
extern const std::string OAORouting;
extern const std::string OAOAmount;
extern const std::string OAOCurrency;
extern const std::string OAOOWRT;

extern const std::string DAOInformation; // Section to be included in ERD for Origin Add on
extern const std::string DAOFootnote1;
extern const std::string DAOFootnote2;
extern const std::string DAOFareClass;
extern const std::string DAOTariff;
extern const std::string DAORouting;
extern const std::string DAOAmount;
extern const std::string DAOCurrency;
extern const std::string DAOOWRT;

// END ERD Information.
// Tax Tags
extern const std::string TaxInformation;
extern const std::string TaxBreakdown;
extern const std::string TaxExempt;
extern const std::string TaxAmount;
extern const std::string ATaxCode;
extern const std::string TaxCurrencyCode;
extern const std::string StationCode;
extern const std::string EMUCurrency;
extern const std::string ATaxDescription;
extern const std::string AmountPublished;
extern const std::string PublishedCurrency;
extern const std::string GSTTax;
extern const std::string TaxCountryCode;
extern const std::string TaxAirlineCode;
extern const std::string TaxType;
extern const std::string TbdTaxType;
extern const std::string ReissueRestrictionApply;
extern const std::string TaxApplyToReissue;
extern const std::string ReissueTaxRefundable;
extern const std::string ReissueTaxCurrency;
extern const std::string ReissueTaxAmount;
extern const std::string MinTaxAmount;
extern const std::string MaxTaxAmount;
extern const std::string MinMaxTaxCurrency;
extern const std::string TaxRateUsed;
extern const std::string TaxPercentage;
extern const std::string PreviousTaxInformation;
extern const std::string TaxCarrier;
extern const std::string RefundableTaxTag;

// Handling/Markup Tags
extern const std::string MarkupDetail;
extern const std::string MarkupFeeAppId;
extern const std::string MarkupTypeCode;
extern const std::string FareAmountAfterMarkup;
extern const std::string MarkupAmount;
extern const std::string AmountCurrency;
extern const std::string MarkupRuleSourcePCC;
extern const std::string MarkupRuleItemNumber;

// Currency Coversion Tags
extern const std::string FareIATARate;
extern const std::string FareBankerSellRate;
extern const std::string TaxBankerSellRate;
extern const std::string CurrencyConversionInformation;
extern const std::string From;
extern const std::string To;
extern const std::string IntermediateCurrency;
extern const std::string Amount;
extern const std::string NumberDecimalPlaces;
extern const std::string ConvertedAmount;
extern const std::string NumberDecimalPlacesConvertedAmount;
extern const std::string ExchangeRateOne;
extern const std::string NumberDecimalPlacesExchangeRateOne;
extern const std::string ExchangeRateTwo;
extern const std::string NumberDecimalPlacesExchangeRateTwo;
extern const std::string CurrencyConversionCarrierCode;
extern const std::string CountryCode;
extern const std::string TravelDate;
extern const std::string CurrencyCoversionFareBasisCode;
extern const std::string ConversionType;
extern const std::string ApplicationType;
extern const std::string EffectiveDate;
extern const std::string DiscontinueDate;

// Dynamic Price Deviation Tags
extern const std::string EffectivePriceDeviation;

// Plus Up Tags
extern const std::string PlusUps;
extern const std::string PlusUpAmount;
extern const std::string PlusUpOrigCity;
extern const std::string PlusUpDestCity;
extern const std::string PlusUpFareOrigCity;
extern const std::string PlusUpFareDestCity;
extern const std::string PlusUpMessage;
extern const std::string PlusUpViaCity;
extern const std::string PlusUpCountryOfPmt;

// Message Tags
extern const std::string MessageInformation;
extern const std::string MessageType;
extern const std::string MessageFailCode;
extern const std::string MessageAirlineCode;
extern const std::string MessageText;
extern const std::string MessageFreeText;
extern const std::string MessageWarningText;

// Higher Intermediate Point Tags
extern const std::string HigherIntermediatePoint;
extern const std::string ZeroDifferentialItem;
extern const std::string OrigCityHIP;
extern const std::string OrigCityHIPWPDF;
extern const std::string DestCityHIP;
extern const std::string DestCityHIPWPDF;
extern const std::string LowOrigHIP;
extern const std::string LowDestHIP;
extern const std::string HighOrigHIP;
extern const std::string HighDestHIP;
extern const std::string FareClassHigh;
extern const std::string FareClassLow;
extern const std::string CabinLowHIP;
extern const std::string CabinHighHIP;
extern const std::string AmountHIP;
extern const std::string OrigSegOrder;
extern const std::string DestSegOrder;

extern const std::string NetRemitInfo;
extern const std::string AccTvlData;
extern const std::string ReqAccTvl;

extern const std::string ValidationResult;
extern const std::string TicketGuaranteed;
extern const std::string WpaWpNoMatch;

extern const std::string ItinNumber;

// WPA Tags
extern const std::string WPAOptionInformation;
extern const std::string WPANoMatchOptions;
extern const std::string WPALastOption;

// Service Fee detailed info (OB)
extern const std::string ServiceFeeDetailedInfo;
extern const std::string ServiceTypecode;
extern const std::string ServiceFeeAmount;
extern const std::string FopBINNumber;
extern const std::string IATAindCombined;
extern const std::string NoChargeInd;
extern const std::string ServiceFeePercent;
extern const std::string MaxServiceFeeAmt;
extern const std::string ShowNoObFees;
extern const std::string ServiceFeeAmountTotal;
extern const std::string ServiceDescription;
extern const std::string RequestedBin;
extern const std::string CardChargeAmount;
extern const std::string FormsOfPayment;

// Service Fee detailed info for WPA (OB)
extern const std::string FormOfPayment;
extern const std::string SecondFormOfPayment;
extern const std::string ResidualSpecifiedChargeAmount;
extern const std::string ObFeesFailStatus;
extern const std::string AccountCodes;
extern const std::string CorporateIDs;
extern const std::string ForbidCreditCard;

// OC Fees Display Info tags (OC)
extern const std::string OCFeesDisplayInfo;
extern const std::string OCGroupInfo;
extern const std::string OCFeeGroupCode;
extern const std::string AttnInd;
extern const std::string OCGenericMsg;
extern const std::string OCGroupHeader;
extern const std::string OCGroupTrailer;
extern const std::string OCGroupStatusCode;
extern const std::string OCGroupStatusInd;
extern const std::string OCFareStat;
extern const std::string OCPreformattedMsg;

// OC Fees Tags to PNR
extern const std::string OCFeesReference; // Used to reference an OCFee or as a container for the
                                          // referenced OC
extern const std::string OCFeesReferenceNumber; // Uniquely identifies an OCFee
extern const std::string OCFeesPNRInfo; // OC PNR Data Main Tag
extern const std::string ServiceLineNumber; // Table 170 Bytes 20-26
extern const std::string ServiceBaseCurPrice; // Table 170 Bytes 20-26
extern const std::string ServiceBaseCurCode; // Table 170 Bytes 27-29
extern const std::string ServiceEquiCurPrice; // Converted amount in Selling Currency
extern const std::string ServiceEquiCurCode; // Currency Code in Selling Currency or override entry
extern const std::string ServiceTaxIndicator; // S7 byte 312
extern const std::string ServiceTaxAmount; // Place Holder
extern const std::string ServiceTaxCode; // Place Holder
extern const std::string ServiceTotalPrice; // Price Plus Taxes [Base/Equi]
extern const std::string ServiceTotalPrice4OC; // Price Plus Taxes
extern const std::string ServiceCommercialName; // S5 Bytes 88-117
extern const std::string ServiceRFICCode; // S5 Byte 56
extern const std::string ServiceRFICSubCode; // S5 Sub Code Bytes 8-10
extern const std::string ServiceSSRCode; // S5 Byte 57-60
extern const std::string ServiceSegment; // PNR Segment number; '/' used if more than one segment
extern const std::string ServiceEMDType; // S5 Byte 85
extern const std::string ServiceNChrgNAvlb; // S7 Byte 288
extern const std::string ServicePaxTypeCode; // S7 Byte 62-64 or Input can be ALL
extern const std::string ServiceDisplayOnly; // OCFees
extern const std::string ServiceOwningCxr; // S5 Carrier - Bytes 3-5
extern const std::string ServiceIATAApplInd; // S7 IATA Ind - Bytes 284, 285, 286
                                             // RefundReIssue/Commision/Interline
extern const std::string ServiceSecPorInd; // S7 Byte 133
extern const std::string ServiceBooking; // S5 Bytes 86-87 Booking Indicator
extern const std::string ServiceFeeApplInd; // Fee Indicator Byte 311
extern const std::string ServiceFeeRefund; // S7 Byte 287
extern const std::string ServiceFeeGuarInd; // OCFee Fee Guarantee Indicator
extern const std::string ServiceFeeNameNum;
extern const std::string ServiceFeeTktNum;
extern const std::string ServiceFeeGroupCd; // S5 Bytes 43-45
extern const std::string ServiceSimResTkt; // S7 Byte 256 - AdvPurTktIssue
extern const std::string ServiceSSIMCode; // S5 Byte 84
extern const std::string ServiceVendorCode; // S5
extern const std::string ServiceTravelDateEff; // S7 based on S7 byte 254-256
                                               // AdvPurUnit/AdvPurTktIssue
extern const std::string ServiceTravelDateDisc; // S7 Eff/Disc Date Byte 38-43, 44-49
extern const std::string ServiceTierStatus; // S7 Byte 76 FreqFlr Status
extern const std::string ServicePurchaseByDate; // Calculate based on S7 filing
extern const std::string ServiceTourCode; // S7 Byte 101-115 Tour Code
extern const std::string ServiceOrigDest; // Populated based on filing
extern const std::string EmdChargeInd; // Emd SoftPass Charge Indicator

// M70 Tags
extern const std::string ItinInfo; // Itinerary Information
extern const std::string GenId; // Itinerary Id
extern const std::string OCS5Info; // Ancillary Fee sub code information - S5
extern const std::string S5CarrierCode; // Owning Carrier Code - S5
extern const std::string S5RFICCode; // RFIC Code - S5
extern const std::string S5EMDType; // EMD Type - S5
extern const std::string S5SSIMCode; // SSIM Code - S5
extern const std::string S5Consumption; // CONSUMPTION Ind - S5  (M70 path)
extern const std::string OosCollection;
extern const std::string OOSS7Info; // Optional Services information - S7
extern const std::string OOSSecPorInd; // Sector Portion Indicator
extern const std::string OOSOrigAirportCode; // Service Origin Airport Code
extern const std::string OOSDestAirportCode; // Service Destination Airport Code
extern const std::string OOSSoftMatchInd; // Soft Match Indicator
extern const std::string OOSTaxExemptInd; // Tax Exempt ind - M70 only
extern const std::string OOSSectorNumber; // Sector Numbers for Portion od Travel
extern const std::string OosPadisNumber; // PADIS - Upgrade to RBD table 198 item number
extern const std::string OosFirstDisclosureOccurrence; // First Disclosure Occurrence
extern const std::string OosSecondDisclosureOccurrence; // Second Disclosure Occurrence
extern const std::string PspPadisInfo; // PADIS - all sequences for a single item number
extern const std::string UpcPadisInfo; // PADIS - data related to a single T198 sequence
extern const std::string UpcPadisSequence; // PADIS - t198 sequence number
extern const std::string PdsPadisItem; // PADIS - data related to a single PADIS code
extern const std::string PdsPadisCode; // PADIS - PADIS code
extern const std::string PdsPadisAbbreviation; // PADIS - abbreviation
extern const std::string PdsPadisDescription; // PADIS - description
extern const std::string AncillaryRespOptions; // Response Options
extern const std::string TruncRespIndicator; // Truncated response Indicator

extern const std::string BaggageTravelIndex;
extern const std::string SegIdInBaggageTravel;
extern const std::string OptionalServicesSeqNo;
extern const std::string InterlineIndicator;

extern const std::string AncillaryIdentifier;
extern const std::string AncillaryQuantity;
extern const std::string AncillaryPriceModificationIdentifier;

// Build Passenger and Fare Information
extern const std::string SUMPsgrFareInfo; // Passenger and Fare Info
extern const std::string PsgrTypeCode; // Passenfer Type Code
extern const std::string TotalPrice; // Total Price
extern const std::string SUMBasePrice; // Base Price
extern const std::string SUMBaseCurrencyCode; // Base Currency Code
extern const std::string SUMEquiBasePrice; // Equivalent Base Price
extern const std::string SUMEquiCurCode; // Equivalent Currency Code
extern const std::string TaxInd; // Tax Indicator
extern const std::string AncillaryServiceSubGroup; // Ancillary Services SubGroup
extern const std::string AncillaryServiceSubGroupDescription; // Ancillary Services SubGroup
                                                              // Description
extern const std::string AncillaryServiceType; // Ancillary Services Type
extern const std::string GroupedOCFeesReferences; // Container for OCFee references

// Build SFQ - STO information
extern const std::string FeeQualifyInfo; // Fee Qualifying Info
extern const std::string SFQCabin; // SFQ Cabin
extern const std::string SFQRuleTariff; // SFQ Rule Tariff
extern const std::string SFQRule; // Rule
extern const std::string SFQDayOfWeek; // Day of the week
extern const std::string SFQEquipmentCode; // Equipment Code
extern const std::string StopOverInfo; // Stop Over Info
extern const std::string StpOvrConxDestInd; // Stopover - Connection - Destination Indicator
extern const std::string StvrTime; // Stopover Time
extern const std::string StvrTimeUnit; // Stopover Time Unit

// Build Date and Time Information
extern const std::string DateTimeInfo; // Date Time Info
extern const std::string TvlDtEffDt; // Travel Date Effective Date
extern const std::string LtstTvlDtPermt; // Latest Travel Date Permitted
extern const std::string PurByDate; // Purchase By Date
extern const std::string StartTime; // Start Time
extern const std::string StopTime; // Stop Time

// Fee Application And Ticketing Information
extern const std::string FeeApplTktInfo; // Fee Application and Tkt Info
extern const std::string FeeApplInd; // Fee Application Indicator
extern const std::string FeeGuranInd; // Fee Gurantee Indicator
extern const std::string SimulTktInd; // Simultaneous Tkt Indicator
extern const std::string NoChrgNotAvailInd; // No Charge Not Available Indicator
extern const std::string FormOfRefund; // Form Of Refund
extern const std::string FQTVCxrFiledTierLvl; // FQTV Carrier Filed Tier Level
extern const std::string RefundReissueInd; // Refund Reissue Indicator
extern const std::string CommisionInd; // Commision Indicator
extern const std::string InterlineInd; // Interline Indicator

// RBD information - Table 198
extern const std::string RBDInfo; // RBD Info - Table 198
extern const std::string MktOptInd; // Marketing Operating Indicator
extern const std::string RBDCarrier; // RBD Carrier
extern const std::string BookingCodeType; // Booking Code Type

// Carrier Resulting Fare Class Information - Table 171
extern const std::string CarrierFareClassInfo; // Carrier Fare Class Info - Table 171
extern const std::string FCLCarrier; // FCL Carrier
extern const std::string ResultFCLOrTKTDesig; // Resulting Fare Class or Ticket Designator
extern const std::string FCLFareType; // FCL Fare Type

// Fare Component information
extern const std::string FareComponentLevelData;
extern const std::string FareComponentBreakDown;
extern const std::string FareComponentNumber;
extern const std::string Location;
extern const std::string MostRestrictiveJourneyData;
extern const std::string CommissionRuleId;
extern const std::string CommissionProgramId;
extern const std::string CommissionContractId;

// Pricing Unit information
extern const std::string PricingUnitLevelData;
extern const std::string PricingUnitNumber;

// Cat 5 Advance Reservation/Ticketing
extern const std::string AdvanceReservationData;
extern const std::string LastDayToBook;
extern const std::string LastTimeToBook;
extern const std::string LastDayToPurchaseTicket;
extern const std::string LastTimeToPurchaseTicket;

// Cat 6 Minimum Stay
extern const std::string MinimumStay;
extern const std::string MinimumStayDate;
extern const std::string MinimumStayTime;

// Cat 7 Maximum Stay
extern const std::string MaximumStay;
extern const std::string MaximumStayDate;
extern const std::string MaximumStayTime;

extern const std::string LastDayReturnMustBeCompleted;
extern const std::string LastTimeReturnMustBeCompleted;

// Tax Carrier Flight Information - Table 186
extern const std::string CarrierFlightInfo; // Carrier Flight Info - Table 186
extern const std::string MKtCarrier; // Marketing Carrier
extern const std::string OprtCarrier; // Operating Carrier
extern const std::string FlightNum1; // Flight Number 1
extern const std::string FlightNum2; // Flight Number 2

// BGD section
extern const std::string BaggageBGD;
extern const std::string BaggageOC1;
extern const std::string BaggageOC2;
extern const std::string BaggageSizeWeightLimit; //
extern const std::string BaggageSizeWeightUnitType; //
extern const std::string BaggageSizeWeightLimitType;

// BGA section
extern const std::string BaggageBGA; // Baggage Allowance Data
extern const std::string BaggageBPC;
extern const std::string BaggageITR; // Baggage Item Limit
extern const std::string BaggageITT;
extern const std::string BaggageBIR; // Baggage Item Size and Weight Restrictions
extern const std::string BaggageSDC; // Service Description from S5

// Baggage Disclosure

extern const std::string BaggageBDI;
extern const std::string PriceAndFulfillmentInf; // Price and Fulfillment Information

extern const std::string ExtendedSubCodeKey;
extern const std::string FrequentFlyerWarning;

extern const std::string ServiceGroup;
extern const std::string BaggageProvision;
extern const std::string Desc1Code;
extern const std::string Desc1Text;
extern const std::string Desc2Code;
extern const std::string Desc2Text;
extern const std::string MinimumWeightW01; // xs:short  short int optional  Minimum weight
extern const std::string MinimumWeightWU1; // WeightUnitType  1 optional  Minimum weight - units
extern const std::string MinimumWeightW02; // xs:short  short int optional  Minimum weight in
                                           // alternate units
extern const std::string MinimumWeightWU2; // WeightUnitType  1 optional  Minimum weight in
                                           // alternate units - units
extern const std::string MaximumWeightW03; // xs:short  short int optional  MaximumWeight
extern const std::string MaximumWeightWU3; // WeightUnitType  1 optional  Maximum weight - units
extern const std::string MaximumWeightW04; // xs:short  short int optional  Maximum weight in
                                           // alternate units
extern const std::string MaximumWeightWU4; // WeightUnitType  1 optional  Maximum weight in
                                           // alternate units - units
extern const std::string MinimumSizeS01; // xs:short  short int optional  MinimumSize
extern const std::string MinimumSizeSU1; // LengthUnitType  1 optional  Minimum size - units
extern const std::string MinimumSizeS02; // xs:short  short int optional  Minimum size in alternate
                                         // units
extern const std::string MinimumSizeSU2; // LengthUnitType  1 optional  Minimum size in alternate
                                         // units - units
extern const std::string MaximumSizeS03; // xs:short  short int optional  MaximumSize
extern const std::string MaximumSizeSU3; // LengthUnitType  1 optional  Maximum size - units
extern const std::string MaximumSizeS04; // xs:short  short int optional  Maximum size in alternate
                                         // units
extern const std::string MaximumSizeSU4; // LengthUnitType  1 optional  Maximum size in alternate
                                         // units - units
extern const std::string TravelSegId;

// SDC section
extern const std::string ServiceDescriptionDC1;
extern const std::string ServiceDescriptionD00;

// R7 Tags
extern const std::string TicketPsgrInfo; // Ticket Passenger Order Number
extern const std::string TicketPsgrNum; // Ticket Passenger Number
extern const std::string PsgrNameInfo; // PNR Passenger Name Information
extern const std::string NameNum; // Name Number
extern const std::string TicketNum; // Ticket Number
extern const std::string TicketRefNum; // Ticket Reference number
extern const std::string AvailService; // Availability Service
extern const std::string OOSTicketCouponAssoc; // Ticket Coupon Association
extern const std::string OOSFlightCoupon; // ticket coupon number
extern const std::string AncillaryGroupCode; // AncillaryGroupCode in APR element

// Baggage Tags
extern const std::string PassengerCount; // Passenger Count for the Type
extern const std::string BaggageDisclosure; // Baggage Rules Disclosure

// Metrics Instrument
extern const std::string ExtraLatencyInfo;
extern const std::string LatencyDetail;
extern const std::string LatencyDescription;
extern const std::string LatencyNCalls; // Number of calls
extern const std::string LatencyWallTime; // Elapsed time
extern const std::string LatencyCpuTime;

// Multi-ticketing
extern const std::string MultiTicketExt;
extern const std::string MultiTicketResp;
extern const std::string MultiTicketNum;
extern const std::string GroupNumber;
extern const std::string PricingCriteria;

// Specify Maximum Penalty
extern const std::string PenaltyInformation;
extern const std::string ChangePenaltyBefore;
extern const std::string ChangePenaltyAfter;
extern const std::string RefundPenaltyBefore;
extern const std::string RefundPenaltyAfter;
extern const std::string NonChangeable;
extern const std::string MaximumPenaltyAmount;
extern const std::string MaximumPenaltyCurrency;
extern const std::string IsCategory16;
extern const std::string PenaltyMissingData;

// Commission Management
extern const std::string ValidatingCarrierCommissionInfo;

// Adjusting Selling Level
extern const std::string ASLSellingFareData;
extern const std::string ASLLayerTypeName;
extern const std::string ASLMarkupSummary;
extern const std::string ASLTypeCode;
extern const std::string ASLDescription;
} // end namespace xml2
} // end namespace tse
