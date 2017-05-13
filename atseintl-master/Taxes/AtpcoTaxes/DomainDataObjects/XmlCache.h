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
#include <boost/ptr_container/ptr_vector.hpp>

#include "DataModel/Common/Types.h"
#include "DataModel/Services/AKHIFactor.h"
#include "DataModel/Services/CarrierApplication.h"
#include "DataModel/Services/CarrierFlight.h"
#include "DataModel/Services/Currency.h"
#include "DataModel/Services/CurrencyConversion.h"
#include "DataModel/Services/Customer.h"
#include "DataModel/Services/IsInLoc.h"
#include "DataModel/Services/Nation.h"
#include "DataModel/Services/PassengerTypeCode.h"
#include "DataModel/Services/PaxTypeMapping.h"
#include "DataModel/Services/ReportingRecord.h"
#include "DataModel/Services/RepricingEntry.h"
#include "DataModel/Services/RulesRecord.h"
#include "DataModel/Services/SectorDetail.h"
#include "DataModel/Services/ServiceBaggage.h"
#include "DataModel/Services/ServiceFeeSecurity.h"
#include "DataModel/Services/TaxRounding.h"
#include "DomainDataObjects/MileageGetterServer.h"

namespace tax
{
class XmlCache
{
public:
  XmlCache(void);
  ~XmlCache(void);

  const boost::ptr_vector<RulesRecord>& rulesRecords() const { return _rulesRecords; }

  boost::ptr_vector<RulesRecord>& rulesRecords() { return _rulesRecords; }

  const boost::ptr_vector<RepricingEntry>& repricingEntries() const { return _repricingEntries; }

  boost::ptr_vector<RepricingEntry>& repricingEntries() { return _repricingEntries; }

  const boost::ptr_vector<ReportingRecord>& reportingRecords() const { return _reportingRecords; }

  boost::ptr_vector<ReportingRecord>& reportingRecords() { return _reportingRecords; }

  const boost::ptr_vector<Nation>& nations() const { return _nations; }

  boost::ptr_vector<Nation>& nations() { return _nations; }

  const boost::ptr_vector<IsInLoc>& isInLocs() const { return _isInLocs; }

  boost::ptr_vector<IsInLoc>& isInLocs() { return _isInLocs; }

  const boost::ptr_vector<MileageGetterServer>& mileages() const { return _mileages; }

  boost::ptr_vector<MileageGetterServer>& mileages() { return _mileages; }

  const boost::ptr_vector<CarrierFlight>& carrierFlights() const { return _carrierFlights; }

  boost::ptr_vector<CarrierFlight>& carrierFlights() { return _carrierFlights; }

  const boost::ptr_vector<CarrierApplication>& carrierApplications() const
  {
    return _carrierApplications;
  }

  boost::ptr_vector<CarrierApplication>& carrierApplications() { return _carrierApplications; }

  const boost::ptr_vector<ServiceBaggage>& serviceBaggage() const { return _serviceBaggage; }

  boost::ptr_vector<ServiceBaggage>& serviceBaggage() { return _serviceBaggage; }

  const boost::ptr_vector<SectorDetail>& sectorDetail() const { return _sectorDetail; }

  boost::ptr_vector<SectorDetail>& sectorDetail() { return _sectorDetail; }

  const boost::ptr_vector<Currency>& currencies() const { return _currencies; }

  boost::ptr_vector<Currency>& currencies() { return _currencies; }

  const boost::ptr_vector<Customer>& customers() const { return _customers; }

  boost::ptr_vector<Customer>& customers() { return _customers; }

  const boost::ptr_vector<CurrencyConversion>& currencyConversions() const
  {
    return _currencyConversions;
  }

  boost::ptr_vector<CurrencyConversion>& currencyConversions() { return _currencyConversions; }

  const boost::ptr_vector<PassengerTypeCode>& passengerTypeCodes() const
  {
    return _passengerTypeCodes;
  }

  boost::ptr_vector<PassengerTypeCode>& passengerTypeCodes() { return _passengerTypeCodes; }

  const boost::ptr_vector<AKHIFactor>& aKHIFactor() const { return _AKHIfactor; }

  boost::ptr_vector<AKHIFactor>& aKHIFactor() { return _AKHIfactor; }

  boost::ptr_vector<PaxTypeMapping>& paxTypeMapping() { return _paxTypeMapping; }

  const boost::ptr_vector<PaxTypeMapping>& paxTypeMapping() const { return _paxTypeMapping; }

  boost::ptr_vector<ServiceFeeSecurity>& serviceFeeSecurity() { return _serviceFeeSecurity; }

  const boost::ptr_vector<ServiceFeeSecurity>& serviceFeeSecurity() const
  {
    return _serviceFeeSecurity;
  }

  boost::ptr_vector<TaxRounding>& taxRoundings() { return _taxRoundings; }

  const boost::ptr_vector<TaxRounding>& taxRoundings() const { return _taxRoundings; }

private:
  boost::ptr_vector<RulesRecord> _rulesRecords;
  boost::ptr_vector<RepricingEntry> _repricingEntries;
  boost::ptr_vector<ReportingRecord> _reportingRecords;
  boost::ptr_vector<Nation> _nations;
  boost::ptr_vector<IsInLoc> _isInLocs;
  boost::ptr_vector<MileageGetterServer> _mileages;
  boost::ptr_vector<CarrierFlight> _carrierFlights;
  boost::ptr_vector<CarrierApplication> _carrierApplications;
  boost::ptr_vector<ServiceBaggage> _serviceBaggage;
  boost::ptr_vector<Currency> _currencies;
  boost::ptr_vector<CurrencyConversion> _currencyConversions;
  boost::ptr_vector<Customer> _customers;
  boost::ptr_vector<PassengerTypeCode> _passengerTypeCodes;
  boost::ptr_vector<SectorDetail> _sectorDetail;
  boost::ptr_vector<AKHIFactor> _AKHIfactor;
  boost::ptr_vector<PaxTypeMapping> _paxTypeMapping;
  boost::ptr_vector<ServiceFeeSecurity> _serviceFeeSecurity;
  boost::ptr_vector<TaxRounding> _taxRoundings;
};
}
