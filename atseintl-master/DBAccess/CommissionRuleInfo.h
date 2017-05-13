#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/TSEDateInterval.h"

namespace tse
{

class CommissionRuleInfo
{
 public:

  struct OriginDestination
  {
    OriginDestination()
    {
    }

    OriginDestination(const LocCode& origin,
                      const LocCode& destination)
      : _origin(origin)
      , _destination(destination)
    {
    }

    bool operator == (const OriginDestination& other) const
    {
      return _origin == other._origin && _destination == other._destination;
    }

    void flattenize(Flattenizable::Archive& archive)
    {
      FLATTENIZE(archive, _origin);
      FLATTENIZE(archive, _destination);
    }

    LocCode _origin;
    LocCode _destination;
  };

  CommissionRuleInfo()
    : _commissionId(0)
    , _programId(-1)
    , _commissionValue(0)
    , _commissionTypeId(-1)
    , _inhibit(' ')
    , _validityIndicator(' ')
    , _interlineConnectionRequired(' ')
    , _roundTrip(' ')
    , _fareAmountMin(0)
    , _fareAmountMax(0)
  {
  }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  uint64_t& commissionId() { return _commissionId; }
  uint64_t commissionId() const { return _commissionId; }

  TSEDateInterval& effInterval() { return _effInterval; }
  const TSEDateInterval& effInterval() const { return _effInterval; }

  DateTime& createDate() { return _effInterval.createDate(); }
  DateTime createDate() const { return _effInterval.createDate(); }

  DateTime& effDate() { return _effInterval.effDate(); }
  DateTime effDate() const { return _effInterval.effDate(); }

  DateTime& expireDate() { return _effInterval.expireDate(); }
  DateTime expireDate() const { return _effInterval.expireDate(); }

  DateTime& discDate() { return _effInterval.discDate(); }
  DateTime discDate() const { return _effInterval.discDate(); }

  int64_t& programId() { return _programId; }
  int64_t programId() const { return _programId; }

  std::string& description() { return _description; }
  const std::string& description() const { return _description; }

  CurrencyCode& currency() { return _currency; }
  const CurrencyCode& currency() const { return _currency; }

  MoneyAmount& commissionValue() { return _commissionValue; }
  MoneyAmount commissionValue() const { return _commissionValue; }

  int& commissionTypeId() { return _commissionTypeId; }
  int commissionTypeId() const { return _commissionTypeId; }

  Indicator& inhibit() { return _inhibit; }
  Indicator inhibit() const { return _inhibit; }

  Indicator& validityIndicator() { return _validityIndicator; }
  Indicator validityIndicator() const { return _validityIndicator; }

  std::vector<char>& fareBasisCodeIncl() { return _fareBasisCodeIncl; }
  const std::vector<char>& fareBasisCodeIncl() const { return _fareBasisCodeIncl; }

  std::vector<char>& fareBasisCodeExcl() { return _fareBasisCodeExcl; }
  const std::vector<char>& fareBasisCodeExcl() const { return _fareBasisCodeExcl; }

  std::vector<BookingCode>& classOfServiceIncl() { return _classOfServiceIncl; }
  const std::vector<BookingCode>& classOfServiceIncl() const { return _classOfServiceIncl; }

  std::vector<BookingCode>& classOfServiceExcl() { return _classOfServiceExcl; }
  const std::vector<BookingCode>& classOfServiceExcl() const { return _classOfServiceExcl; }

  std::vector<CarrierCode>& operatingCarrierIncl() { return _operatingCarrierIncl; }
  const std::vector<CarrierCode>& operatingCarrierIncl() const { return _operatingCarrierIncl; }

  std::vector<CarrierCode>& operatingCarrierExcl() { return _operatingCarrierExcl; }
  const std::vector<CarrierCode>& operatingCarrierExcl() const { return _operatingCarrierExcl; }

  std::vector<CarrierCode>& marketingCarrierIncl() { return _marketingCarrierIncl; }
  const std::vector<CarrierCode>& marketingCarrierIncl() const { return _marketingCarrierIncl; }

  std::vector<CarrierCode>& marketingCarrierExcl() { return _marketingCarrierExcl; }
  const std::vector<CarrierCode>& marketingCarrierExcl() const { return _marketingCarrierExcl; }

  std::vector<CarrierCode>& ticketingCarrierIncl() { return _ticketingCarrierIncl; }
  const std::vector<CarrierCode>& ticketingCarrierIncl() const { return _ticketingCarrierIncl; }

  std::vector<CarrierCode>& ticketingCarrierExcl() { return _ticketingCarrierExcl; }
  const std::vector<CarrierCode>& ticketingCarrierExcl() const { return _ticketingCarrierExcl; }

  char& interlineConnectionRequired() { return _interlineConnectionRequired; }
  char interlineConnectionRequired() const { return _interlineConnectionRequired; }

  char& roundTrip() { return _roundTrip; }
  char roundTrip() const { return _roundTrip; }

  MoneyAmount& fareAmountMin() { return _fareAmountMin; }
  MoneyAmount fareAmountMin() const { return _fareAmountMin; }

  MoneyAmount& fareAmountMax() { return _fareAmountMax; }
  MoneyAmount fareAmountMax() const { return _fareAmountMax; }

  CurrencyCode& fareAmountCurrency() { return _fareAmountCurrency; }
  const CurrencyCode& fareAmountCurrency() const { return _fareAmountCurrency; }

  std::vector<std::string>& fbcFragmentIncl() { return _fbcFragmentIncl; }
  const std::vector<std::string>& fbcFragmentIncl() const { return _fbcFragmentIncl; }

  std::vector<std::string>& fbcFragmentExcl() { return _fbcFragmentExcl; }
  const std::vector<std::string>& fbcFragmentExcl() const { return _fbcFragmentExcl; }

  std::vector<TktDesignator>& requiredTktDesig() { return _requiredTktDesig; }
  const std::vector<TktDesignator>& requiredTktDesig() const { return _requiredTktDesig; }

  std::vector<TktDesignator>& excludedTktDesig() { return _excludedTktDesig; }
  const std::vector<TktDesignator>& excludedTktDesig() const { return _excludedTktDesig; }

  std::vector<LocCode>& requiredCnxAirPCodes() { return _requiredCnxAirPCodes; }
  const std::vector<LocCode>& requiredCnxAirPCodes() const { return _requiredCnxAirPCodes; }

  std::vector<LocCode>& excludedCnxAirPCodes() { return _excludedCnxAirPCodes; }
  const std::vector<LocCode>& excludedCnxAirPCodes() const { return _excludedCnxAirPCodes; }

  std::vector<CarrierCode>& requiredMktGovCxr() { return _requiredMktGovCxr; }
  const std::vector<CarrierCode>& requiredMktGovCxr() const { return _requiredMktGovCxr; }

  std::vector<CarrierCode>& excludedMktGovCxr() { return _excludedMktGovCxr; }
  const std::vector<CarrierCode>& excludedMktGovCxr() const { return _excludedMktGovCxr; }

  std::vector<PaxTypeCode>& requiredPaxType() { return _requiredPaxType; }
  const std::vector<PaxTypeCode>& requiredPaxType() const { return _requiredPaxType; }

  std::vector<CarrierCode>& requiredOperGovCxr() { return _requiredOperGovCxr; }
  const std::vector<CarrierCode>& requiredOperGovCxr() const { return _requiredOperGovCxr; }

  std::vector<CarrierCode>& excludedOperGovCxr() { return _excludedOperGovCxr; }
  const std::vector<CarrierCode>& excludedOperGovCxr() const { return _excludedOperGovCxr; }

  std::vector<OriginDestination>& requiredNonStop() { return _requiredNonStop; }
  const std::vector<OriginDestination>& requiredNonStop() const { return _requiredNonStop; }

  std::vector<char>& requiredCabinType() { return _requiredCabinType; }
  const std::vector<char>& requiredCabinType() const { return _requiredCabinType; }

  std::vector<char>& excludedCabinType() { return _excludedCabinType; }
  const std::vector<char>& excludedCabinType() const { return _excludedCabinType; }

  std::vector<TourCode>& excludedTourCode() { return _excludedTourCode; }
  const std::vector<TourCode>& excludedTourCode() const { return _excludedTourCode; }

  bool operator==(const CommissionRuleInfo& rhs) const
  {
    return _vendor == rhs._vendor
           && _commissionId == rhs._commissionId
           && _effInterval == rhs._effInterval
           && _programId == rhs._programId
           && _description == rhs._description
           && _currency == rhs._currency
           && _commissionValue == rhs._commissionValue
           && _commissionTypeId == rhs._commissionTypeId
           && _inhibit == rhs._inhibit
           && _validityIndicator == rhs._validityIndicator
           && _fareBasisCodeIncl == rhs._fareBasisCodeIncl
           && _fareBasisCodeExcl == rhs._fareBasisCodeExcl
           && _classOfServiceIncl == rhs._classOfServiceIncl
           && _classOfServiceExcl == rhs._classOfServiceExcl
           && _operatingCarrierIncl == rhs._operatingCarrierIncl
           && _operatingCarrierExcl == rhs._operatingCarrierExcl
           && _marketingCarrierIncl == rhs._marketingCarrierIncl
           && _marketingCarrierExcl == rhs._marketingCarrierExcl
           && _ticketingCarrierIncl == rhs._ticketingCarrierIncl
           && _ticketingCarrierExcl == rhs._ticketingCarrierExcl
           && _interlineConnectionRequired == rhs._interlineConnectionRequired
           && _roundTrip == rhs._roundTrip
           && _fareAmountMin == rhs._fareAmountMin
           && _fareAmountMax == rhs._fareAmountMax
           && _fareAmountCurrency == rhs._fareAmountCurrency
           && _fbcFragmentIncl == rhs._fbcFragmentIncl
           && _fbcFragmentExcl == rhs._fbcFragmentExcl
           && _requiredTktDesig == rhs._requiredTktDesig
           && _excludedTktDesig == rhs._excludedTktDesig
           && _requiredCnxAirPCodes == rhs._requiredCnxAirPCodes
           && _excludedCnxAirPCodes == rhs._excludedCnxAirPCodes
           && _requiredMktGovCxr == rhs._requiredMktGovCxr
           && _excludedMktGovCxr == rhs._excludedMktGovCxr
           && _requiredPaxType == rhs._requiredPaxType
           && _requiredOperGovCxr == rhs._requiredOperGovCxr
           && _excludedOperGovCxr == rhs._excludedOperGovCxr
           && _requiredNonStop == rhs._requiredNonStop
           && _requiredCabinType == rhs._requiredCabinType
           && _excludedCabinType == rhs._excludedCabinType
           && _excludedTourCode == rhs._excludedTourCode;
  }

  static void dummyData(CommissionRuleInfo& obj)
  {
    obj._vendor = "COS";
    obj._commissionId = 1234567;
    TSEDateInterval::dummyData(obj._effInterval);
    obj._programId = 5432109876;
    obj._description = "Abcdefgh ijkl";
    obj._currency = "USD";
    obj._commissionValue = 321543.21;
    obj._commissionTypeId = 987;
    obj._inhibit = 'N';
    obj._validityIndicator = 'Y';
    obj._fareBasisCodeIncl.push_back('A');
    obj._fareBasisCodeIncl.push_back('B');
    obj._fareBasisCodeExcl.push_back('C');
    obj._fareBasisCodeExcl.push_back('D');
    obj._classOfServiceIncl.push_back("X");
    obj._classOfServiceIncl.push_back("Y");
    obj._classOfServiceExcl.push_back("U");
    obj._classOfServiceExcl.push_back("V");
    obj._operatingCarrierIncl.push_back("AA");
    obj._operatingCarrierExcl.push_back("UA");
    obj._marketingCarrierIncl.push_back("F9");
    obj._marketingCarrierExcl.push_back("AS");
    obj._ticketingCarrierIncl.push_back("N0S");
    obj._ticketingCarrierExcl.push_back("W0A");
    obj._interlineConnectionRequired = 'Y';
    obj._roundTrip = 'R';
    obj._fareAmountMin = 11.00;
    obj._fareAmountMax = 12.00;
    obj._fareAmountCurrency = "AUD";
    obj._fbcFragmentIncl.push_back("AB%");
    obj._fbcFragmentExcl.push_back("C%D");
    obj._requiredTktDesig.emplace_back("AbCdEfGh");
    obj._excludedTktDesig.emplace_back("IjKlMnOpQ");;
    obj._requiredCnxAirPCodes.emplace_back("JFK");
    obj._excludedCnxAirPCodes.emplace_back("BRU");
    obj._requiredMktGovCxr.emplace_back("AA");
    obj._excludedMktGovCxr.emplace_back("F9");
    obj._requiredPaxType.push_back("ADT");
    obj._requiredOperGovCxr.push_back("AA");
    obj._excludedOperGovCxr.push_back("UA");
    obj._requiredNonStop.emplace_back("JFK", "BRU");
    obj._requiredCabinType.push_back('A');
    obj._requiredCabinType.push_back('B');
    obj._excludedCabinType.push_back('C');
    obj._excludedCabinType.push_back('D');
    obj._excludedTourCode.push_back("ABCD");
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _commissionId);
    FLATTENIZE(archive, _effInterval);
    FLATTENIZE(archive, _programId);
    FLATTENIZE(archive, _description);
    FLATTENIZE(archive, _currency);
    FLATTENIZE(archive, _commissionValue);
    FLATTENIZE(archive, _commissionTypeId);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _validityIndicator);
    FLATTENIZE(archive, _fareBasisCodeIncl);
    FLATTENIZE(archive, _fareBasisCodeExcl);
    FLATTENIZE(archive, _classOfServiceIncl);
    FLATTENIZE(archive, _classOfServiceExcl);
    FLATTENIZE(archive, _operatingCarrierIncl);
    FLATTENIZE(archive, _operatingCarrierExcl);
    FLATTENIZE(archive, _marketingCarrierIncl);
    FLATTENIZE(archive, _marketingCarrierExcl);
    FLATTENIZE(archive, _ticketingCarrierIncl);
    FLATTENIZE(archive, _ticketingCarrierExcl);
    FLATTENIZE(archive, _interlineConnectionRequired);
    FLATTENIZE(archive, _roundTrip);
    FLATTENIZE(archive, _fareAmountMin);
    FLATTENIZE(archive, _fareAmountMax);
    FLATTENIZE(archive, _fareAmountCurrency);
    FLATTENIZE(archive, _fbcFragmentIncl);
    FLATTENIZE(archive, _fbcFragmentExcl);
    FLATTENIZE(archive, _requiredTktDesig);
    FLATTENIZE(archive, _excludedTktDesig);
    FLATTENIZE(archive, _requiredCnxAirPCodes);
    FLATTENIZE(archive, _excludedCnxAirPCodes);
    FLATTENIZE(archive, _requiredMktGovCxr);
    FLATTENIZE(archive, _excludedMktGovCxr);
    FLATTENIZE(archive, _requiredPaxType);
    FLATTENIZE(archive, _requiredOperGovCxr);
    FLATTENIZE(archive, _excludedOperGovCxr);
    FLATTENIZE(archive, _requiredNonStop);
    FLATTENIZE(archive, _requiredCabinType);
    FLATTENIZE(archive, _excludedCabinType);
    FLATTENIZE(archive, _excludedTourCode);
  }

 private:

  VendorCode _vendor;
  uint64_t _commissionId; 
  TSEDateInterval _effInterval;
  int64_t _programId;
  std::string _description;
  CurrencyCode _currency;
  MoneyAmount _commissionValue;
  int _commissionTypeId;
  Indicator _inhibit;
  Indicator _validityIndicator;
  std::vector<char> _fareBasisCodeIncl;
  std::vector<char> _fareBasisCodeExcl;
  std::vector<BookingCode> _classOfServiceIncl;
  std::vector<BookingCode> _classOfServiceExcl;
  std::vector<CarrierCode> _operatingCarrierIncl;
  std::vector<CarrierCode> _operatingCarrierExcl;
  std::vector<CarrierCode> _marketingCarrierIncl;
  std::vector<CarrierCode> _marketingCarrierExcl;
  std::vector<CarrierCode> _ticketingCarrierIncl;
  std::vector<CarrierCode> _ticketingCarrierExcl;
  char _interlineConnectionRequired;
  char _roundTrip;
  MoneyAmount _fareAmountMin;
  MoneyAmount _fareAmountMax;
  CurrencyCode _fareAmountCurrency;
  std::vector<std::string> _fbcFragmentIncl;
  std::vector<std::string> _fbcFragmentExcl;
  std::vector<TktDesignator> _requiredTktDesig;
  std::vector<TktDesignator> _excludedTktDesig;
  std::vector<LocCode> _requiredCnxAirPCodes;
  std::vector<LocCode> _excludedCnxAirPCodes;
  std::vector<CarrierCode> _requiredMktGovCxr;
  std::vector<CarrierCode> _excludedMktGovCxr;
  std::vector<PaxTypeCode> _requiredPaxType;
  std::vector<CarrierCode> _requiredOperGovCxr;
  std::vector<CarrierCode> _excludedOperGovCxr;
  std::vector<OriginDestination> _requiredNonStop;//JFK|BRU
  std::vector<char> _requiredCabinType;
  std::vector<char> _excludedCabinType;
  std::vector<TourCode> _excludedTourCode;
};

}// tse
