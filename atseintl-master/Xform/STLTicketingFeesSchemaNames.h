#pragma once

namespace tse
{
namespace stlticketingfees
{
enum _STLTicketingFeesElementIdx_
{
  _AccountCode,
  _Agent,
  _BillingInformation,
  _CorpID,
  _RequestedDiagnostic,
  _FareBreakAssociation,
  _FareBreakInformation,
  _Flight,
  _RequestOptions,
  _PassengerIdentity,
  _PricedSolution,
  _PassengerPaymentInfo,
  _FormOfPayment,
  _TravelSegment,
  _OBTicketingFeeRQ,
  _DifferentialHighClass,
  _Option,
  _Arunk,
  _Diagnostic,
  _NumberOfElementNamesTktFees_
};

enum _STLTicketingFeesAttributeIdx_
{
  _originAirport,
  _destinationAirport,
  _pseudoCityCode,
  _agentCity,
  _billingActionCode,
  _agentSineIn,
  _userSetAddress,
  _partitionID,
  _airline,
  _governingCarrier,
  _fareBasisCode,
  _passengerType,
  _ticketDesignator,
  _segmentOrderNumber,
  _travelSegmentOrderNumber,
  _parentTransactionID,
  _clientTransactionID,
  _number,
  _parentServiceName,
  _businessFunction,
  _paymentCurrency,
  _departureDate,
  _requestDate,
  _requestTimeOfDay,
  _validatingCarrier,
  _departureTime,
  _stopOverInd,
  _compOrderNum,
  _userBranch,
  _userStation,
  _fareComponentOrderNum,
  _sideTripOrderNum,
  _sideTripStartInd,
  _sideTripEndInd,
  _sourceOfRequest,
  _version,
  _totalPriceAmount,
  _originCity,
  _destinationCity,
  _firstNameNumber,
  _surNameNumber,
  _pnrNameNumber,
  _objectID,
  _identity,
  _type,
  _passengerRefObjectID,
  _ticketingLocationOverride,
  _chargeAmount,
  _segmentType,
  _NumberOfAttributeNamesTktFees_
};

extern const char* _STLTicketingFeesElementNames[];
extern const char* _STLTicketingFeesAttributeNames[];
}
}
