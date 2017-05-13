#include "Xform/PricingResponseXMLTags.h"

#include <string>

namespace tse
{
namespace xml2
{
// Combinations Tags
const std::string RequestType = "S96";
const std::string RequestTypeAdvancedReservation = "AR";
const std::string SecondaryRequestType = "SA8";

const std::string LastTicketTime = "D60";

// Ticketing Agent Tags
const std::string TicketingAgentInfo = "AGI";
const std::string AgentCity = "A10";
const std::string TvlAgencyPCC = "A20";
const std::string MainTvlAgencyPCC = "A21";
const std::string TvlAgencyIATA = "AB0";
const std::string HomeAgencyIATA = "AB1";
const std::string AgentFunctions = "A90";
const std::string AgentDuty = "N0G";
const std::string AirlineDept = "A80";
const std::string CxrCode = "B00";
const std::string CurrencyCodeAgent = "C40";
const std::string CoHostID = "Q01";
const std::string AgentCommissionType = "N0L";
const std::string AgentCommissionAmount = "C6C";
const std::string VendorCrsCode = "AE0";
const std::string OfficeDesignator = "AE1";
const std::string OfficeCode = "AE2";
const std::string AirlineChannelId = "AE4";

// Billing Information Tags
const std::string BillingInformation{"BIL"};
const std::string UserPseudoCityCode{"A20"};
const std::string UserStation{"Q03"};
const std::string UserBranch{"Q02"};
const std::string PartitionID{"AE0"};
const std::string UserSetAddress{"AD0"};
const std::string ServiceName{"C20"};
const std::string AaaCity{"A22"};
const std::string AaaSine{"AA0"};
const std::string ActionCode{"A70"};
const std::string NodeId{"N0S"};
const std::string TrxStartTime{"D10"};
const std::string TrxEndTime{"D11"};
const std::string TrxCPU{"Q32"};
const std::string TrxRC{"Q3M"};
const std::string TrxID{"C00"};
const std::string ClientTrxID{"C01"};
const std::string ParentTrxID{"C02"};
const std::string SolutionsRequested{"Q0S"};
const std::string SolutionsProduced{"Q1L"};
const std::string ClientSvcName{"C21"};
const std::string ParentSvcName{"C22"};
const std::string RequestPath{"S0R"};

// Summary Tags
const std::string SummaryInfo = "SUM";
const std::string TotalPriceAll = "C56";
const std::string TotalCurrencyCode = "C40";
const std::string NetRemitTotalPriceAll = "C51";
const std::string TicketingDate = "D07";
const std::string TicketingTime = "D54";
const std::string IATASalesCode = "S69";
const std::string SalesLocation = "AO0";
const std::string TicketPointOverride = "AF0";
const std::string AdvancedPurchaseDate = "D14";
const std::string ValidatingCarrier = "B00";
const std::string LastTicketDay = "D00";
const std::string PurchaseByDate = "D16";
const std::string SimultaneousRes = "PBC";
const std::string PrivateFareIndication = "PAR";
const std::string ServerHostname = "S79";
const std::string FareCalcTooLong = "P3J";
const std::string ConsolidatorPlusUpCurrencyCode = "C74";
const std::string ConsolidatorPlusUpAmount = "C6Z";
const std::string ExchangeReissueIndicator = "N27";
const std::string RoeBSRDate = "D95";
const std::string RoeBSRTime = "T95";
const std::string SegmentFeeMessage = "SFY";
const std::string AncillariesNonGuarantee = "ANG";
const std::string BrandSoldout = "SBL";
const std::string NoBrandsOffered = "NBO";
const std::string AdjustedSellingLevel = "ADJ";
const std::string DataView = "ONV";

// Validating Carrier List
const std::string VcxrInfo = "VCL";
const std::string InhibitPq = "P3L";
const std::string SettlementMethod = "SM0";
const std::string AtseProcess = "VC0";
const std::string MultiNeutralValCxr = "MNV";
const std::string IETCntryCode = "CTC";

// Default Validating Carrier
const std::string DefaultValidatingCxr = "DCX";
const std::string ValidatingCxrCode = "B00";
const std::string TicketType = "TT0";

// Alternate Validating Carrier
const std::string AlternateValidatingCxr = "ACX";

// Participating Carrier
const std::string ParticipatingCxr = "PCX";
const std::string ParticipatingCxrCode = "B00";
const std::string AgreementTypeCode = "VC1";

// Pax Fare Tags
const std::string PassengerInfo = "PXI";
const std::string PassengerType = "B70";
const std::string RequestedPassengerType = "B71";
const std::string NetRemitPublishedFareRetrieved = "P3K";
const std::string ConstructionCurrencyCode = "C43";
const std::string ConstructionTotalAmount = "C5E";
const std::string NetRemitConstructionCurrencyCode = "C4B";
const std::string NetRemitConstructionTotalAmount = "C52";
const std::string BaseCurrencyCode = "C40";
const std::string BaseFareAmount = "C5A";
const std::string NetRemitBaseCurrencyCode = "C4A";
const std::string NetRemitBaseFareAmount = "C51";
const std::string EquivalentCurrencyCode = "C45";
const std::string EquivalentAmount = "C5F";
const std::string NetRemitEquivalentAmount = "C53";
const std::string CurrencyCodeMinimum = "C44";
const std::string CurrencyCodeMinimumAmount = "C5C";
const std::string StopOverCount = "Q0X";
const std::string StopOverCharges = "C63";
const std::string StopOverPublishedCurrencyCode = "C73";
const std::string TransferCount = "C71";
const std::string TransferCharges = "C70";
const std::string TransferPublishedCurrencyCode = "C72";
const std::string PaxFareCurrencyCode = "C46";
const std::string TotalPerPassenger = "C66";
const std::string TotalPerPassengerPlusImposed = "C56";
const std::string NetRemitTotalPerPassenger = "C6B";
const std::string TotalPerPassengerPlusAbsorbtion = "C50";
const std::string TotalTaxes = "C65";
const std::string TotalTaxesPlusImposed = "C6L";
const std::string NetRemitTotalTaxes = "C6A";
const std::string CommissionPercentage = "C5D";
const std::string CommissionAmount = "C5B";
const std::string CommissionCap = "N09";
const std::string TravelAgencyTax = "C64";
const std::string FareCalculation = "S66";
const std::string NonRefundable = "P27";
const std::string NonRefundableAmount = "NRA";
const std::string NonRefundableIndicator = "NRI";
const std::string NegotiatedWithCorp = "P2C";
const std::string NegotiatedWithoutCorp = "P2B";
const std::string LocalCurrency = "P2A";
const std::string PaxFarePassengerNumber = "Q0W";
const std::string TicketFareVendorSource = "S83";
const std::string TourCodeDescription = "S02";
const std::string ValueCode = "S87";
const std::string NetFareAmount = "C62"; // CAT35
const std::string NetFareAmountPlusCharges = "C5G"; // CAT35
const std::string NetFareBaseAmount = "C5M"; // CAT35 Net Total Amt in Base
const std::string TourIndicator = "N0C"; // T-Tour, V-Value, C-Car
const std::string TextBox = "S01";
const std::string NetGross = "N0B";
const std::string Cat35CommissionPercentage = "C61";
const std::string Cat35CommissionAmount = "C60";
const std::string Cat35RegularNet = "PY9";

// Commission Management Tags
const std::string Cat35MarkupAmount = "C57";

// Common tag for both Cat35 and non-Cat35 (calculated from COS) commission.
const std::string TotalCommissionAmount = "C58";

// Indicate (T) for different commission amount for different validating carriers
const std::string DifferentCommissionAmount = "C5H";
const std::string CommissionSourceIndicator = "CT0"; // A-AMC, C-CAT35, M-Manual

const std::string BSPMethodType = "N0A";
const std::string Cat35Warning = "Q0V";
const std::string Cat35Used = "P26";
const std::string OverrideCat35 = "P28";
const std::string paperTicketSurchargeMayApply = "P2E";
const std::string paperTicketSurchargeIncluded = "P2D";
const std::string WpnDetails = "S84";
const std::string BaggageResponse = "S86";
const std::string WpnOptionNumber = "Q4P";
const std::string TicketingRestricted = "P06";
const std::string TfrRestricted = "P3L";
const std::string PrintOption = "N1S";
const std::string NetRemitTicketIndicator = "PY3";
const std::string TicketingSegmentFeeMsg = "P3J";
const std::string AbacusBillingData = "ABL";
const std::string ContractIssueDate = "D16";
const std::string ContractNumber = "S88";
const std::string FareTariff = "S89";
const std::string FareRule = "S90";
const std::string DBSource = "S83";
const std::string FareSourceCarrier = "B02";
const std::string FareSourcePcc = "A20";
const std::string FareBasisTktDesig = "B50";
const std::string FareOrig = "A11";
const std::string FareDest = "A12";
const std::string NetConstructionTotalAmount = "C5N"; // CAT35 NET
const std::string NetTotalPerPassenger = "C5P"; // CAT35 NET
const std::string NetTotalTaxes = "C5O"; // CAT35 NET
const std::string SpanishDiscountIndicator = "PY5";
const std::string UsDotItinIndicator = "USI";
const std::string FopBINNumberApplied = "BIN";
const std::string Cat35TFSFWithNet = "PY6";
const std::string SurfaceRestrictedInd = "ISR";
const std::string TotalFareAmountWithTTypeFees = "C81";
const std::string TotalTTypeOBFeeAmount = "C82";
const std::string Cat35BlankCommission = "PY7";
const std::string CommissionBaseAmount = "PY8";

// Passenger Fare REX Tags
const std::string ReissueExchange = "REX";
const std::string ResidualIndicator = "N1X";
const std::string FormOfRefundIndicator = "N20";
const std::string UnflownItinPriceInfo = "UFL";
const std::string Tag7PricingSolution = "PXM";
const std::string ElectronicTicketRequired = "PXY";
const std::string ElectronicTicketNotAllowed = "PXZ";

// Passenger Fare Change Fee Tags
const std::string ChangeFee = "CHG";
const std::string HighestChangeFeeIndicator = "PXJ";
const std::string ChangeFeeAmount = "C77";
const std::string ChangeFeeCurrency = "C78";
const std::string ChangeFeeWaived = "PXK";
const std::string ChangeFeeNotApplicable = "PXL";
const std::string ChangeFeeAmountInPaymentCurrency = "C76";
const std::string ChangeFeeAgentCurrency = "C40";

// Segment Tags
const std::string SegmentInformation = "SEG";
const std::string SegmentDepartureCity = "A11";
const std::string SegmentDepartureAirport = "C6I";
const std::string SegmentDepartureDate = "D71";
const std::string SegmentDepartureTime = "D72";
const std::string SegmentArrivalDate = "D73";
const std::string SegmentArrivalTime = "D74";
const std::string SegmentArrivalCity = "A12";
const std::string SegmentArrivalAirport = "A02";
const std::string FareClass = "P72";
const std::string CityStopoverCharge = "C67";
const std::string TransferCharge = "C68";
const std::string ItinSegmentNumber = "Q0Z";
const std::string RouteTravel = "S12";
const std::string NotValidBeforeDate = "D06";
const std::string NotValidAfterDate = "D05";
const std::string SegmentPassengerNumber = "Q0Y"; // for baggage
const std::string BaggageIndicator = "N0D"; // P/K/L
const std::string BaggageValue = "B20";
const std::string AvailabilityBreak = "PAW";
const std::string SideTripIndicator = "P2N";
const std::string ExtraMileageAllowance = "P2I";
const std::string MileageExclusion = "P2K";
const std::string MileageReduction = "P2L";
const std::string MileageEqualization = "P2J";
const std::string StopoverSegment = "P2M";
const std::string ConnectionSegment = "P2H";
const std::string TransferSegment = "P3M";
const std::string FareBreakPoint = "P2F";
const std::string TurnaroundPoint = "P2O";
const std::string OffPointPartMainComponent = "P2G";
const std::string SideTripEnd_Obsolete = "S07";
const std::string SideTripEndComponent_Obsolete = "S08";
const std::string SideTripStart = "S07";
const std::string SideTripEnd = "S08";
const std::string UnchargeableSurface = "S09";
const std::string PureSurfaceSegment = "S10";
const std::string JourneyType = "N1L";
const std::string SegmentCabinCode = "N00";
const std::string Cat35FareSegment = "P26";
const std::string Cat5RequiresRebook = "PAX";
const std::string SegDepartureCountry = "C11";
const std::string SegArrivalCountry = "C12";
const std::string LocationType = "C13";
const std::string FareVendor = "S37";
const std::string FareCarrier = "B09";
const std::string FareVendorSource = "S83";
const std::string SEGEquipmentCode = "SHR";
const std::string OAndDMarketBeginSegOrder = "ODB";
const std::string OAndDMarketEndSegOrder = "ODE";
const std::string MarketingCarrier = "B00";
const std::string OperatingCarrier = "B01";

// Mileage Tags
const std::string MileageDisplayInformation = "MIL";
const std::string MileageDisplayType = "AP2";
const std::string MileageDisplayCity = "AP3";

// Surcharge Tags
const std::string SurchargeInformation = "SUR";
const std::string SurchargeType = "N0F";
const std::string SurchargeAmount = "C69";
const std::string SurchargeCurrencyCode = "C46";
const std::string SurchargeDescription = "S03";
const std::string SurchargeDepartureCity = "A11";
const std::string SurchargeArrivalCity = "A12";
const std::string SurchargeSurchargeType = "N28";

// Fare Calc Tags
const std::string FareCalcInformation = "CAL";
const std::string FareCalcDepartureCity = "A11";
const std::string FareCalcDepartureAirport = "A01";
const std::string GoverningCarrier = "B02";
const std::string SecondGoverningCarrier = "B03";
const std::string TrueGoverningCarrier = "B08";
const std::string FareCalcArrivalCity = "A12";
const std::string FareCalcArrivalAirport = "A02";
const std::string FareAmount = "C50";
const std::string FareBasisCode = "B50";
const std::string FareBasisCodeLength = "Q04";
const std::string NetRemitPubFareAmount = "C52";
const std::string NetRemitPubFareBasisCode = "B51";
const std::string NetRemitPubFareBasisCodeLength = "Q10";
const std::string DifferentialFareBasisCode = "B51";
const std::string TicketDisignator = "BE0";
const std::string SnapDesignatorDiscount = "BD0";
const std::string FareCalcCabinCode = "N00";
const std::string DepartureCountry = "A41";
const std::string DepartureIATA = "A51";
const std::string DepartureStateOrProvince = "A31";
const std::string ArrivalCountry = "A42";
const std::string ArrivalIATA = "A52";
const std::string ArrivalStateOrProvince = "A32";
const std::string PublishedFareAmount = "C51";
const std::string FareComponentCurrencyCode = "C40";
const std::string RoundTripFare = "P05";
const std::string OneWayFare = "P04";
const std::string OneWayDirectionalFare = "P03";
const std::string IATAAuthorizedCarrier = "P02";
const std::string VerifyGeographicalRestrictions = "P0E";
const std::string FailCat15 = "P01";
const std::string FareByRuleSpouseHead = "P0B";
const std::string FareByRuleSpouseAccompany = "P0A";
const std::string FareByRuleSeamanAdult = "P0D";
const std::string FareByRuleSeamanChild = "C63";
const std::string FareByRuleSeamanInfant = "P0C";
const std::string FareByRuleBTS = "P00";
const std::string FareByRuleNegotiated = "P09";
const std::string FareByRuleNegotiatedChild = "P07";
const std::string FareByRuleNegotiatedInfant = "P08";
const std::string CommencementDate = "D00";

const std::string TypeOfFare = "N0K";
const std::string DiscountCode = "S67";
const std::string DiscountPercentage = "Q17";
const std::string IsMileageRouting = "PAY";
const std::string MileageSurchargePctg = "Q48";
const std::string HIPOrigCity = "A13";
const std::string HIPDestCity = "A14";
const std::string ConstructedHIPCity = "A18";
const std::string SpecialFare = "PAZ";
const std::string PricingUnitType = "N1K";
const std::string GlobalDirectionInd = "A60";
const std::string FareCalcDirectionality = "S70";
const std::string PricingUnitCount = "Q4J";
const std::string CorpId = "AC0";
const std::string CategoryList = "CAT";
const std::string SegmentsCount = "Q0U";
const std::string PrivateFareInd = "N1U";
const std::string NetTktFareAmount = "C5R"; // cat35 net
const std::string IbfInternalPath = "PAF";
const std::string PublicPrivateFare = "PPF";
const std::string TotalTaxesPerFareComponent = "TTA";
const std::string TotalSurchargesPerFareComponent = "TSU";
const std::string FareComponentPlusTaxesPlusSurcharges = "FTS";
const std::string FareComponentInEquivalentCurrency = "FAE";
const std::string FareRetailerCodeNet = "RC1";
const std::string FareRetailerCodeAdjusted = "RC2";

// The next sections added for the Enhanced Rule Display project -- command WPRD.
const std::string ERDInformation = "ERD"; // Section to be included in CAL for EnhancedRuleDisplay
const std::string Link = "Q46";
const std::string Sequence = "Q1K";
const std::string CreateDate = "D12";
const std::string CreateTime = "D55";
const std::string FCAFareType = "S53";
const std::string ERDFareClass = "BJ0";
const std::string ERDFareClassLength = "Q04";
const std::string ERDFareAmount = "C5A";
const std::string ERDBookingCode = "P72";
const std::string ERDFareBasisCode = "B50";
const std::string ERDTicketDesignator = "BE0";
const std::string ERDAccountCode = "AC0";
const std::string ERDFareDepartDate = "D08"; // Departure date of first seg of this fare.
const std::string ERDGoverningCarrier = "B00";
const std::string ERDCommandPricing = "CP0";

const std::string ERDC25Information = "C25"; // Section to be included in ERD for Cat25
const std::string ERDC25Vendor = "S37";
const std::string ERDC25ItemNo = "Q41";
const std::string ERDC25Directionality = "S70";

const std::string ERDC35Information = "C35"; // Section to be included in ERD for Cat35
const std::string ERDC35Vendor = "S37";
const std::string ERDC35ItemNo = "Q41";
const std::string ERDC35FareDisplayType = "N1P";
const std::string ERDC35Directionality = "S70";

const std::string ERDDiscountInformation = "DFI"; // Section to be included in ERD for ERDDiscounts
const std::string ERDDiscountVendor = "S37";
const std::string ERDDiscountItemNo = "Q41";
const std::string ERDDiscountDirectionality = "S70";

const std::string ERDContructedInformation = "ECN"; // Section to be included in ERD for Contructed
const std::string ERDGateway1 = "AM0";
const std::string ERDGateway2 = "AN0";
const std::string ERDConstructionType = "N1J";
const std::string ERDSpecifiedFareAmount = "C66";
const std::string ERDConstructedNucAmount = "C6K";

const std::string OAOInformation = "OAO"; // Section to be included in ERD for Origin Add on
const std::string OAOFootnote1 = "S55";
const std::string OAOFootnote2 = "S64";
const std::string OAOFareClass = "BJ0";
const std::string OAOTariff = "Q3W";
const std::string OAORouting = "S65";
const std::string OAOAmount = "C50";
const std::string OAOCurrency = "C40";
const std::string OAOOWRT = "P04";

const std::string DAOInformation = "DAO"; // Section to be included in ERD for Origin Add on
const std::string DAOFootnote1 = "S55";
const std::string DAOFootnote2 = "S64";
const std::string DAOFareClass = "BJ0";
const std::string DAOTariff = "Q3W";
const std::string DAORouting = "S65";
const std::string DAOAmount = "C50";
const std::string DAOCurrency = "C40";
const std::string DAOOWRT = "P04";

// END ERD Information.
// Tax Tags
const std::string TaxInformation = "TAX";
const std::string TaxBreakdown = "TBD";
const std::string TaxExempt = "TBE";
const std::string TaxAmount = "C6B";
const std::string ATaxCode = "BC0";
const std::string TaxCurrencyCode = "C40";
const std::string StationCode = "S05";
const std::string EMUCurrency = "P2P";
const std::string ATaxDescription = "S04";
const std::string AmountPublished = "C6A";
const std::string PublishedCurrency = "C41";
const std::string GSTTax = "P2Q";
const std::string TaxCountryCode = "A40";
const std::string TaxAirlineCode = "A04";
const std::string TaxType = "A05";
const std::string TbdTaxType = "A06";
const std::string ReissueRestrictionApply = "PXF";
const std::string TaxApplyToReissue = "PXG";
const std::string ReissueTaxRefundable = "PXH";
const std::string ReissueTaxCurrency = "C79";
const std::string ReissueTaxAmount = "C80";
const std::string MinTaxAmount = "C6D";
const std::string MaxTaxAmount = "C6E";
const std::string MinMaxTaxCurrency = "C47";
const std::string TaxRateUsed = "C6F";
const std::string TaxPercentage = "A06";
const std::string PreviousTaxInformation = "PTI";
const std::string TaxCarrier = "X20";
const std::string RefundableTaxTag = "X21";

// Handling/Markup Tags
const std::string MarkupDetail = "HPU";
const std::string MarkupFeeAppId= "APP";
const std::string MarkupTypeCode= "T52";
const std::string FareAmountAfterMarkup= "C51";
const std::string MarkupAmount= "C52";
const std::string AmountCurrency= "C53";
const std::string MarkupRuleSourcePCC= "A52";
const std::string MarkupRuleItemNumber= "R52";

// Currency Coversion Tags
const std::string FareIATARate = "FIR";
const std::string FareBankerSellRate = "P45";
const std::string TaxBankerSellRate = "TBR";
const std::string CurrencyConversionInformation = "CCD";
const std::string From = "C41";
const std::string To = "C42";
const std::string IntermediateCurrency = "C46";
const std::string Amount = "C52";
const std::string NumberDecimalPlaces = "Q08";
const std::string ConvertedAmount = "C53";
const std::string NumberDecimalPlacesConvertedAmount = "Q07";
const std::string ExchangeRateOne = "C54";
const std::string NumberDecimalPlacesExchangeRateOne = "Q05";
const std::string ExchangeRateTwo = "C55";
const std::string NumberDecimalPlacesExchangeRateTwo = "Q06";
const std::string CurrencyConversionCarrierCode = "B00";
const std::string CountryCode = "A40";
const std::string TravelDate = "D00";
const std::string CurrencyCoversionFareBasisCode = "B50";
const std::string ConversionType = "N02";
const std::string ApplicationType = "N01";
const std::string EffectiveDate = "D06";
const std::string DiscontinueDate = "D05";

// Dynamic Price Deviation Tags
const std::string EffectivePriceDeviation = "EPD";

// Plus Up Tags
const std::string PlusUps = "PUP";
const std::string PlusUpAmount = "C6L";
const std::string PlusUpOrigCity = "A11";
const std::string PlusUpDestCity = "A12";
const std::string PlusUpFareOrigCity = "A13";
const std::string PlusUpFareDestCity = "A14";
const std::string PlusUpMessage = "S68";
const std::string PlusUpViaCity = "A18";
const std::string PlusUpCountryOfPmt = "A40";

// Message Tags
const std::string MessageInformation = "MSG";
const std::string MessageType = "N06";
const std::string MessageFailCode = "Q0K";
const std::string MessageAirlineCode = "B00";
const std::string MessageText = "S18";
const std::string MessageFreeText = "S19";
const std::string MessageWarningText = "S20";

// Higher Intermediate Point Tags
const std::string HigherIntermediatePoint = "HIP";
const std::string ZeroDifferentialItem = "ZDF";
const std::string OrigCityHIP = "A13";
const std::string OrigCityHIPWPDF = "A11";
const std::string DestCityHIP = "A14";
const std::string DestCityHIPWPDF = "A12";
const std::string LowOrigHIP = "A01";
const std::string LowDestHIP = "A02";
const std::string HighOrigHIP = "A03";
const std::string HighDestHIP = "A04";
const std::string FareClassHigh = "BJ0";
const std::string FareClassLow = "B30";
const std::string CabinLowHIP = "N00";
const std::string CabinHighHIP = "N04";
const std::string AmountHIP = "C50";
const std::string OrigSegOrder = "Q4S";
const std::string DestSegOrder = "Q4T";

const std::string NetRemitInfo = "NET";
const std::string AccTvlData = "S85";
const std::string ReqAccTvl = "PBS";

const std::string ValidationResult = "VLS";
const std::string TicketGuaranteed = "PBT";
const std::string WpaWpNoMatch = "PBV";

const std::string ItinNumber = "Q6F";

// WPA Tags
const std::string WPAOptionInformation = "PXX";
const std::string WPANoMatchOptions = "S81";
const std::string WPALastOption = "Q0S";

// Service Fee detailed info (OB)
const std::string ServiceFeeDetailedInfo = "OBF";
const std::string ServiceTypecode = "SF0";
const std::string ServiceFeeAmount = "SF1";
const std::string FopBINNumber = "SF2";
const std::string IATAindCombined = "SF3";
const std::string NoChargeInd = "SF4";
const std::string ServiceFeePercent = "SF5";
const std::string MaxServiceFeeAmt = "SF6";
const std::string ShowNoObFees = "SF9";
const std::string ServiceFeeAmountTotal = "STA";
const std::string ServiceDescription = "SDD";
const std::string RequestedBin = "BIN";
const std::string CardChargeAmount = "PAT";
const std::string FormsOfPayment = "P6X";

// Service Fee detailed info for WPA (OB)
const std::string FormOfPayment = "FOP";
const std::string SecondFormOfPayment = "FP2";
const std::string ResidualSpecifiedChargeAmount = "PRS";
const std::string ObFeesFailStatus = "OFS";
const std::string AccountCodes = "SMX";
const std::string CorporateIDs = "ACC";
const std::string ForbidCreditCard = "FCC";

// OC Fees Display Info tags (OC)
const std::string OCFeesDisplayInfo = "OCM";
const std::string OCGroupInfo = "OCG";
const std::string OCFeeGroupCode = "SF0";
const std::string AttnInd = "SF8";
const std::string OCGenericMsg = "OCF";
const std::string OCGroupHeader = "OCH";
const std::string OCGroupTrailer = "OCT";
const std::string OCGroupStatusCode = "ST0";
const std::string OCGroupStatusInd = "ST1";
const std::string OCFareStat = "FTY";
const std::string OCPreformattedMsg = "APR";

// OC Fees Tags to PNR
const std::string OCFeesReference =
    "OCI"; // Used to reference an OCFee or as a container for the referenced OC
const std::string OCFeesReferenceNumber = "ID0"; // Uniquely identifies an OCFee
const std::string OCFeesPNRInfo = "OCP"; // OC PNR Data Main Tag
const std::string ServiceLineNumber = "SHI"; // Table 170 Bytes 20-26
const std::string ServiceBaseCurPrice = "SFA"; // Table 170 Bytes 20-26
const std::string ServiceBaseCurCode = "SFB"; // Table 170 Bytes 27-29
const std::string ServiceEquiCurPrice = "SFC"; // Converted amount in Selling Currency
const std::string ServiceEquiCurCode = "SFH"; // Currency Code in Selling Currency or override entry
const std::string ServiceTaxIndicator = "SFI"; // S7 byte 312
const std::string ServiceTaxAmount = "SHD"; // Place Holder
const std::string ServiceTaxCode = "SHE"; // Place Holder
const std::string ServiceTotalPrice = "SFE"; // Price Plus Taxes [Base/Equi]
const std::string ServiceTotalPrice4OC = "C50"; // Price Plus Taxes
const std::string ServiceCommercialName = "SFF"; // S5 Bytes 88-117
const std::string ServiceRFICCode = "SFG"; // S5 Byte 56
const std::string ServiceRFICSubCode = "SHK"; // S5 Sub Code Bytes 8-10
const std::string ServiceSSRCode = "SHL"; // S5 Byte 57-60
const std::string ServiceSegment = "SSG"; // PNR Segment number; '/' used if more than one segment
const std::string ServiceEMDType = "SFJ"; // S5 Byte 85
const std::string ServiceNChrgNAvlb = "SNN"; // S7 Byte 288
const std::string ServicePaxTypeCode = "SHF"; // S7 Byte 62-64 or Input can be ALL
const std::string ServiceDisplayOnly = "SFD"; // OCFees
const std::string ServiceOwningCxr = "SFK"; // S5 Carrier - Bytes 3-5
const std::string ServiceIATAApplInd =
    "SFL"; // S7 IATA Ind - Bytes 284, 285, 286 RefundReIssue/Commision/Interline
const std::string ServiceSecPorInd = "SFM"; // S7 Byte 133
const std::string ServiceBooking = "SFN"; // S5 Bytes 86-87 Booking Indicator
const std::string ServiceFeeApplInd = "SHG"; // Fee Indicator Byte 311
const std::string ServiceFeeRefund = "SFP"; // S7 Byte 287
const std::string ServiceFeeGuarInd = "SFR"; // OCFee Fee Guarantee Indicator
const std::string ServiceFeeNameNum = "SFS";
const std::string ServiceFeeTktNum = "SFT";
const std::string ServiceFeeGroupCd = "SFU"; // S5 Bytes 43-45
const std::string ServiceSimResTkt = "SFX"; // S7 Byte 256 - AdvPurTktIssue
const std::string ServiceSSIMCode = "SFW"; // S5 Byte 84
const std::string ServiceVendorCode = "SFV"; // S5
const std::string ServiceTravelDateEff =
    "SFZ"; // S7 based on S7 byte 254-256 AdvPurUnit/AdvPurTktIssue
const std::string ServiceTravelDateDisc = "SHA"; // S7 Eff/Disc Date Byte 38-43, 44-49
const std::string ServiceTierStatus = "SHB"; // S7 Byte 76 FreqFlr Status
const std::string ServicePurchaseByDate = "SHM"; // Calculate based on S7 filing
const std::string ServiceTourCode = "SHC"; // S7 Byte 101-115 Tour Code
const std::string ServiceOrigDest = "SHN"; // Populated based on filing
const std::string EmdChargeInd = "ECI"; // Emd SoftPass Charge Indicator

// M70 Tags
const std::string ItinInfo = "ITN"; // Itinerary Information
const std::string GenId = "Q00"; // Itinerary Id
const std::string OCS5Info = "OSC"; // Ancillary Fee sub code information - S5
const std::string S5CarrierCode = "B01"; // Owning Carrier Code - S5
const std::string S5RFICCode = "N01"; // RFIC Code - S5
const std::string S5EMDType = "N02"; // EMD Type - S5
const std::string S5SSIMCode = "N03"; // SSIM Code - S5
const std::string S5Consumption = "PR0"; // CONSUMPTION Ind - S5  (M70 path)
const std::string OosCollection = "OCC";
const std::string OOSS7Info = "OOS"; // Optional Services information - S7
const std::string OOSSecPorInd = "N11"; // Sector Portion Indicator
const std::string OOSOrigAirportCode = "A01"; // Service Origin Airport Code
const std::string OOSDestAirportCode = "A02"; // Service Destination Airport Code
const std::string OOSSoftMatchInd = "PS1"; // Soft Match Indicator
const std::string OOSTaxExemptInd = "PY0"; // Tax Exempt ind - M70 only
const std::string OOSSectorNumber = "Q00"; // Sector Numbers for Portion od Travel
const std::string OosPadisNumber = "URT"; // PADIS - Upgrade to RBD table 198 item number
const std::string OosFirstDisclosureOccurrence = "FDO"; // First Disclosure Occurrence
const std::string OosSecondDisclosureOccurrence = "SDO"; // Second Disclosure Occurrence
const std::string PspPadisInfo = "PSP"; // PADIS - all sequences for a single item number
const std::string UpcPadisInfo = "UPC"; // PADIS - data related to a single T198 sequence
const std::string UpcPadisSequence = "SEU"; // PADIS - t198 sequence number
const std::string PdsPadisItem = "PDS"; // PADIS - data related to a single PADIS code
const std::string PdsPadisCode = "PDI"; // PADIS - PADIS code
const std::string PdsPadisAbbreviation = "PCA"; // PADIS - abbreviation
const std::string PdsPadisDescription = "SED"; // PADIS - description
const std::string AncillaryRespOptions = "ROP"; // Response Options
const std::string TruncRespIndicator = "PTR"; // Truncated response Indicator

const std::string BaggageTravelIndex = "BTI";
const std::string SegIdInBaggageTravel = "QBT";
const std::string OptionalServicesSeqNo = "SEQ";
const std::string InterlineIndicator = "NII";

const std::string AncillaryIdentifier = "AID";
const std::string AncillaryQuantity = "QTY";
const std::string AncillaryPriceModificationIdentifier = "PMI";

// Build Passenger and Fare Information
const std::string SUMPsgrFareInfo = "SUM"; // Passenger and Fare Info
const std::string PsgrTypeCode = "B70"; // Passenfer Type Code
const std::string TotalPrice = "C50"; // Total Price
const std::string SUMBasePrice = "C51"; // Base Price
const std::string SUMBaseCurrencyCode = "C5A"; // Base Currency Code
const std::string SUMEquiBasePrice = "C52"; // Equivalent Base Price
const std::string SUMEquiCurCode = "C5B"; // Equivalent Currency Code
const std::string TaxInd = "N21"; // Tax Indicator
const std::string AncillaryServiceSubGroup = "ASG"; // Ancillary Services SubGroup
const std::string AncillaryServiceSubGroupDescription =
    "ASD"; // Ancillary Services SubGroup Description
const std::string AncillaryServiceType = "AST"; // Ancillary Services Type
const std::string GroupedOCFeesReferences = "GOC"; // Container for OCFee references

// Build SFQ - STO information
const std::string FeeQualifyInfo = "SFQ"; // Fee Qualifying Info
const std::string SFQCabin = "N1A"; // SFQ Cabin
const std::string SFQRuleTariff = "Q0W"; // SFQ Rule Tariff
const std::string SFQRule = "SHP"; // Rule
const std::string SFQDayOfWeek = "SHQ"; // Day of the week
const std::string SFQEquipmentCode = "SHR"; // Equipment Code
const std::string StopOverInfo = "STO"; // Stop Over Info
const std::string StpOvrConxDestInd = "N12"; // Stopover - Connection - Destination Indicator
const std::string StvrTime = "Q01"; // Stopover Time
const std::string StvrTimeUnit = "N13"; // Stopover Time Unit

// Build Date and Time Information
const std::string DateTimeInfo = "DTE"; // Date Time Info
const std::string TvlDtEffDt = "D01"; // Travel Date Effective Date
const std::string LtstTvlDtPermt = "D02"; // Latest Travel Date Permitted
const std::string PurByDate = "D03"; // Purchase By Date
const std::string StartTime = "Q11"; // Start Time
const std::string StopTime = "Q12"; // Stop Time

// Fee Application And Ticketing Information
const std::string FeeApplTktInfo = "FAT"; // Fee Application and Tkt Info
const std::string FeeApplInd = "N41"; // Fee Application Indicator
const std::string FeeGuranInd = "P01"; // Fee Gurantee Indicator
const std::string SimulTktInd = "N42"; // Simultaneous Tkt Indicator
const std::string NoChrgNotAvailInd = "N43"; // No Charge Not Available Indicator
const std::string FormOfRefund = "N44"; // Form Of Refund
const std::string FQTVCxrFiledTierLvl = "Q7D"; // FQTV Carrier Filed Tier Level
const std::string RefundReissueInd = "N45"; // Refund Reissue Indicator
const std::string CommisionInd = "P03"; // Commision Indicator
const std::string InterlineInd = "P04"; // Interline Indicator

// RBD information - Table 198
const std::string RBDInfo = "RBD"; // RBD Info - Table 198
const std::string MktOptInd = "N51"; // Marketing Operating Indicator
const std::string RBDCarrier = "B51"; // RBD Carrier
const std::string BookingCodeType = "BKC"; // Booking Code Type

// Carrier Resulting Fare Class Information - Table 171
const std::string CarrierFareClassInfo = "FCL"; // Carrier Fare Class Info - Table 171
const std::string FCLCarrier = "B61"; // FCL Carrier
const std::string ResultFCLOrTKTDesig = "BJ0"; // Resulting Fare Class or Ticket Designator
const std::string FCLFareType = "S53"; // FCL Fare Type

// Fare Component information

const std::string FareComponentLevelData = "FCD";
const std::string FareComponentBreakDown = "FCB";
const std::string FareComponentNumber = "Q6D";
const std::string Location = "LOC";
const std::string MostRestrictiveJourneyData = "MRJ";
const std::string CommissionRuleId = "C3R";
const std::string CommissionProgramId = "C3P";
const std::string CommissionContractId = "C3C";

// Pricing Unit information

const std::string PricingUnitLevelData = "PUD";
const std::string PricingUnitNumber = "PUN";

// Cat 5 Advance Reservation/Ticketing

const std::string AdvanceReservationData = "ADV";
const std::string LastDayToBook = "LDB";
const std::string LastTimeToBook = "LTB";
const std::string LastDayToPurchaseTicket = "LDT";
const std::string LastTimeToPurchaseTicket = "LTT";

// Cat 6 Minimum Stay

const std::string MinimumStay = "MIN";
const std::string MinimumStayDate = "MSD";
const std::string MinimumStayTime = "MST";

// Cat 7 Maximum Stay

const std::string MaximumStay = "MAX";
const std::string MaximumStayDate = "MSD";
const std::string MaximumStayTime = "MST";

const std::string LastDayReturnMustBeCompleted = "LDC";
const std::string LastTimeReturnMustBeCompleted = "LTC";

// Tax Carrier Flight Information - Table 186
const std::string CarrierFlightInfo = "CFT"; // Carrier Flight Info - Table 186
const std::string MKtCarrier = "B71"; // Marketing Carrier
const std::string OprtCarrier = "B72"; // Operating Carrier
const std::string FlightNum1 = "Q0B"; // Flight Number 1
const std::string FlightNum2 = "Q0C"; // Flight Number 2

// BGD section
const std::string BaggageBGD = "BGD";
const std::string BaggageOC1 = "OC1";
const std::string BaggageOC2 = "OC2";
const std::string BaggageSizeWeightLimit = "B20"; //
const std::string BaggageSizeWeightUnitType = "N0D"; //
const std::string BaggageSizeWeightLimitType = "L01";

// BGA section
const std::string BaggageBGA = "BGA"; // Baggage Allowance Data
const std::string BaggageBPC = "BPC";
const std::string BaggageITR = "ITR"; // Baggage Item Limit
const std::string BaggageITT = "ITT";
const std::string BaggageBIR = "BIR"; // Baggage Item Size and Weight Restrictions
const std::string BaggageSDC = "SDC"; // Service Description from S5

// Baggage Disclosure
const std::string BaggageBDI = "BDI";
const std::string PriceAndFulfillmentInf = "PFF"; // Price and Fulfillment Information

const std::string ExtendedSubCodeKey = "SHK";
const std::string FrequentFlyerWarning = "FFW";

const std::string ServiceGroup = "SF0";
const std::string BaggageProvision = "BPT";
const std::string Desc1Code = "DC1";
const std::string Desc1Text = "D01";
const std::string Desc2Code = "DC2";
const std::string Desc2Text = "D02";
const std::string MinimumWeightW01 = "W01"; // xs:short  short int optional  Minimum weight
const std::string MinimumWeightWU1 = "WU1"; // WeightUnitType  1 optional  Minimum weight - units
const std::string MinimumWeightW02 =
    "W02"; // xs:short  short int optional  Minimum weight in alternate units
const std::string MinimumWeightWU2 =
    "WU2"; // WeightUnitType  1 optional  Minimum weight in alternate units - units
const std::string MaximumWeightW03 = "W03"; // xs:short  short int optional  MaximumWeight
const std::string MaximumWeightWU3 = "WU3"; // WeightUnitType  1 optional  Maximum weight - units
const std::string MaximumWeightW04 =
    "W04"; // xs:short  short int optional  Maximum weight in alternate units
const std::string MaximumWeightWU4 =
    "WU4"; // WeightUnitType  1 optional  Maximum weight in alternate units - units
const std::string MinimumSizeS01 = "S01"; // xs:short  short int optional  MinimumSize
const std::string MinimumSizeSU1 = "SU1"; // LengthUnitType  1 optional  Minimum size - units
const std::string MinimumSizeS02 =
    "S02"; // xs:short  short int optional  Minimum size in alternate units
const std::string MinimumSizeSU2 =
    "SU2"; // LengthUnitType  1 optional  Minimum size in alternate units - units
const std::string MaximumSizeS03 = "S03"; // xs:short  short int optional  MaximumSize
const std::string MaximumSizeSU3 = "SU3"; // LengthUnitType  1 optional  Maximum size - units
const std::string MaximumSizeS04 =
    "S04"; // xs:short  short int optional  Maximum size in alternate units
const std::string MaximumSizeSU4 =
    "SU4"; // LengthUnitType  1 optional  Maximum size in alternate units - units
const std::string TravelSegId = "Q00";

// SDC section
const std::string ServiceDescriptionDC1 = "DC1";
const std::string ServiceDescriptionD00 = "D00";

// R7 Tags
const std::string TicketPsgrInfo = "ONT"; // Ticket Passenger Order Number
const std::string TicketPsgrNum = "TPN"; // Ticket Passenger Number
const std::string PsgrNameInfo = "PNM"; // PNR Passenger Name Information
const std::string NameNum = "S0L"; // Name Number
const std::string TicketNum = "Q87"; // Ticket Number
const std::string TicketRefNum = "Q86"; // Ticket Reference number
const std::string AvailService = "AX1"; // Availability Service
const std::string OOSTicketCouponAssoc = "TCN"; // Ticket Coupon Association
const std::string OOSFlightCoupon = "C7A"; // ticket coupon number
const std::string AncillaryGroupCode = "AGC"; // AncillaryGroupCode in APR element

// Baggage Tags
const std::string PassengerCount = "Q0U"; // Passenger Count for the Type
const std::string BaggageDisclosure = "DCL"; // Baggage Rules Disclosure

// Metrics Instrument
const std::string ExtraLatencyInfo = "LTC";
const std::string LatencyDetail = "LDT";
const std::string LatencyDescription = "LDS";
const std::string LatencyNCalls = "LDN"; // Number of calls
const std::string LatencyWallTime = "LDW"; // Elapsed time
const std::string LatencyCpuTime = "LDC";

// Multi-ticketing
const std::string MultiTicketExt = "EXT";
const std::string MultiTicketResp = "MTR";
const std::string MultiTicketNum = "TKN";
const std::string GroupNumber = "GRP";
const std::string PricingCriteria = "PCR";

// Specify Maximum Penalty
const std::string PenaltyInformation = "PEN";
const std::string ChangePenaltyBefore = "CPB";
const std::string ChangePenaltyAfter = "CPA";
const std::string RefundPenaltyBefore = "RPB";
const std::string RefundPenaltyAfter = "RPA";
const std::string NonChangeable = "NON";
const std::string MaximumPenaltyAmount = "MPA";
const std::string MaximumPenaltyCurrency = "MPC";
const std::string IsCategory16 = "C16";
const std::string PenaltyMissingData = "MDT";

// Commission Management
const std::string ValidatingCarrierCommissionInfo = "VCC";

// Adjusting Selling Level
const std::string ASLSellingFareData = "SFD";
const std::string ASLLayerTypeName = "TYP";
const std::string ASLMarkupSummary = "HPS";
const std::string ASLTypeCode = "T52";
const std::string ASLDescription = "N52";
} // end namespace xml2
} // end namespace tse
