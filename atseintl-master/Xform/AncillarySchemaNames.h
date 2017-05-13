#pragma once

namespace tse
{
namespace ancillary
{
enum _AncillaryElementIdx_
{ _AncillaryPricingRequest,
  _BaggageRequest,
  _ACI, // Account Code Info
  _AGI, // Agent Information
  _AST, // Ancillary Service Type
  _BIL, // Billing Information
  _BTS, // Baggage Travel Selection for WP*BG
  _CII, // Corporate ID Information
  _DIG, // Diagnostic Parameters
  _EQP, // Equipment Information
  _FBA, // Segment Fare Break Association
  _FBI, // Fare Break Info
  _FFY, // Frequent Flyer Status Information
  _FLI, // Flight Information
  _HSP, // Hidden Stop Information
  _IRO, // Itinerary Request Options
  _ITN, // Itinerary
  _PRO, // Processing Options
  _PXI, // Passenger Information
  _PNM, // PNR Passenger Name information
  _RFG, // Requested ancillary fee Group code
  _SGI, // Segment Info
  _TAG, // Ticketing Agent Information
  _GroupCode, // Group Code
  _DynamicConfig, // Dynamic Configuration Override
  _OSC, // Itinerary Optional Services
  _NumberOfElementNames_ };

enum _AncillaryAttributeIdx_
{ _A01, // Departure Airport Code
  _A02, // Arrival Airport Code
  _A03, // Hidden City Code
  _A10, // Agent City
  _A20, // Pseudo City Code
  _A21, // Main Travel Agency PCC
  _A22, // AAA City
  _A70, // Action Code
  _A80, // Airline Department
  _A90, // Agent Function
  _AA0, // Agent Sine In
  _AB0, // Travel Agency IATA ARC Number
  _AB1, // Home Agency IATA ARC Number
  _ACC, // Account Codes (WPAC*TEST1YAC*TEST2YAC*TEST3YAC*TEST4)
  _AD0, // User Set Address
  _AE0, // Partition ID
  _AE1, // Office Designator
  _AF0, // Ticketing Point Override (WPTTPA)
  _AG0, // Sale Point Override(WPSPAR)
  _ASN, // Ancillary Service Type
  _B00, // Airline Carrier Code/Marketing carrier code
  _B01, // Operating carrier code
  _B02, // Governing Carrier
  _B05, // Validating carrier
  _B30, // BookingCodeType
  _B50, // Fare basis code with slashes and resulting ticket designator
  _B70, // Passenger Type Code
  _B71, // Requested Pax Type Code (In WP/WPA response)
  _BB0, // Reservation status
  _BE0, // Ticket Desginator (Input)
  _C00, // Parent transaction ID (for sender: my transaction id, for receiver: my parent transaction
        // id)
  _C01, // Client transaction ID (The highest level client transaction id)
  _C10, // Diagnostic Number
  _C20, // Parent service name (for sender: my name, for receiver: my parent name)
  _C21, // Business function (The highest level client service name)
  _C40, // Agency Currency Code
  _C45, // Default Currency Override (WPMGBP)
  _C50, // Fare Amount
  _C6C, // Agent Commission Amount
  _C7A, // ticket coupon number
  _C7B, // Checked Portion of Travel Indicator (Baggage)
  _CID, // Corporate Code(WPITEST1)
  _D01, // Departure Date
  _D02, // Arrival Date
  _D07, // Ticketing date
  _D31, // Departure Time
  _D32, // Arrival Time
  _FTY, // OC Fare type bitmap
  _L00, // (Deprecated - use C01) Client transaction ID
  _L01, // Select first for occurrence
  _N0E, // Cabin
  _N0G, // Agent Duty
  _N0L, // Agent Commission Type
  _N1O, // FF VIP type
  _P72, // Forced connection
  _P73, // Forced stopover
  _PCC, //
  _Q00, // Itinerary ID/ Segment ID
  _Q01, // CoHost Identifier
  _Q02, // User Branch
  _Q03, // User Station
  _Q0A, // Number of Items
  _Q0B, // Flight Number
  _Q0U, // Passenger Count for the Type
  _Q3W, // Fare Tariff
  _Q3V, // Fare Indiicator (19-22,25,35)
  _Q5X, // number of hours before departure
  _Q6D, // FareComponent ID
  _Q6E, // Side Trip ID
  _Q7D, // Tier Number
  _P1Z, // PrivateTariff Indicator (Public by default)
  _PS1, // Hard match Indicator
  _S01, // Group Code
  _S07, // Side Trip Start Indicator
  _S08, // Side Trip End Indicator
  _S0R, // Source of Request
  _S14, // Original Pricing command
  _S15, // EPR KEyword
  _S53, // Fare Type
  _S90, // Fare Rule
  _S95, // Equipment Type
  _SAT, // Specific Agency Text
  _SHC, // Tour Code
  _S0L, // Passenger Name Number
  _TKI, // Ticketing Indicator
  _Q86, // Ticket Number
  _Q87, // Ticket reference number
  _ANG, // Ancillaries Not guarantee
  _S37, // Vendor Code
  _P53, // Tax Exempt
  _P54, // All Taxes exempt
  _BH0, // Tax code exempted
  _GRP, // Group Number - for group of solutions that results will be summarized together
  _TKN, // Ticket Number - for multi-ticketing response display
  _AE2, // Office/Station Code
  _GBA, // Baggage and Ancillary informations
  _RCP, // Reservation or CheckIn Path
  _AE3, // Default Ticketing Carrier
  _Version, // Ancillary Pricing Request schema version
  _Name, // Configuration name
  _Value, // Configuration value
  _Substitute, // Configuration name substitute enable
  _Optional, // Configuration override is optional
  _QTY, // Quantity of additional ancillary
  _AID, // Ancillary Identifier
  _PMI, // Price Modifier Index
  _DRP, // Percentage discount
  _DRM, // Monetary discount - value
  _DRC, // Monetary discount - currency
  _DRT, // Price Modification Type
  _NumberOfAttributeNames_ };

extern const char* _AncillaryElementNames[];
extern const char* _AncillaryAttributeNames[];
}
}
