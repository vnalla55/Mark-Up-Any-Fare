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

#include <boost/date_time/date_facet.hpp>
#include <boost/optional.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/type_traits/has_dereference.hpp>
#include <boost/mpl/or.hpp>

#include "Common/MoneyUtil.h"
#include "Common/SafeEnumToString.h"
#include "DataModel/Common/CodeIO.h"
#include "TestServer/Xform/XmlWriter.h"
#include "TestServer/Xform/XmlTagsList.h"
#include "TestServer/Server/InputRequestWithCache.h"

#include "AtpcoTaxes/DataModel/RequestResponse/BCHOutputResponse.h"
#include "AtpcoTaxes/DataModel/RequestResponse/OutputResponse.h"
#include "AtpcoTaxes/DomainDataObjects/XmlCache.h"

// for request processing:
#include "AtpcoTaxes/DataModel/RequestResponse/InputRequest.h"

#include <cassert>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <vector>

namespace tax
{

namespace
{

template <typename>
  struct is_container : boost::mpl::false_ { };

template <typename T>
  struct is_container< std::vector<T> > : boost::mpl::true_ { };

template <typename T>
  struct is_container< boost::ptr_vector<T> > : boost::mpl::true_ { };

std::string
truncateZeroes(std::string amount)
{
  size_t i = 0;
  std::string::const_reverse_iterator it = amount.rbegin();
  std::string::const_reverse_iterator itEnd = amount.rend();

  for (; it != itEnd; it++)
  {
    if (*it != '0')
    {
      if (*it == '.')
        i++;
      break;
    }
    i++;
  }
  return amount.substr(0, amount.size() - i);
}

void
writeAttribute(std::ostream& out,
               type::MoneyAmount const& attribute,
               const std::string& name)
{
  std::ostringstream str;
  if (attribute.denominator() == 1)
    str << attribute.numerator() << ".";
  else
    str << std::fixed << std::setprecision(8) << amountToDouble(attribute);
  out << " " << name << "=\"" << truncateZeroes(str.str()) << "\"";
}

template <typename T>
void
writeAttribute(std::ostream& out, const T& attribute, const std::string& name)
{
  out << " " << name << "=\"" << attribute << "\"";
}

template <typename T>
void
writeAttribute(std::ostream& out,
               const boost::optional<T>& attribute,
               const std::string& name)
{
  if (attribute)
    writeAttribute(out, *attribute, name);
}

template <typename T>
void
writeAttribute(std::ostream& out,
               const CompactOptional<T>& attribute,
               const std::string& name)
{
  if (attribute.has_value())
    writeAttribute(out, attribute.value(), name);
}

template <typename Tag, int L,int H>
void
writeAttribute(std::ostream& out, Code<Tag, L, H> code, const std::string& name)
{
  if (!code.empty())
    writeAttribute(out, code.asString(), name);
}

template <typename T>
void
writeAttribute(std::ostream& out, const std::vector<T>& vec, const std::string& name)
{
  out << ' ' << name << "=\"";
  for (size_t i = 0; i < vec.size(); ++i)
  {
    if (i > 0)
      out << ' ';
    out << vec[i];
  }
  out << '\"';
}

void
writeAttribute(std::ostream& out,
               const std::vector<type::TaxableUnit>& vec,
               const std::string& name)
{
  out << ' ' << name << "=\"";
  for (size_t i = 0; i < vec.size(); ++i)
  {
    if (i > 0)
      out << ' ';
    out << static_cast<uint16_t>(vec[i]);
  }
  out << '\"';
}

} // namespace

class XmlWriter
{
public:
  XmlWriter(const XmlTagsList& xmlTagslist);

  void writeXml(std::ostream& out, const OutputResponse& response);
  void writeXml(std::ostream& out, const BCHOutputResponse& response);
  void writeXml(std::ostream& out, const InputRequest& request);
  void writeXml(std::ostream& out, const InputRequestWithStringCache& request);

  template <typename T> void writeAllChildren(std::ostream& out, const T& source);

  template <typename T> void writeAllAttributes(std::ostream& out, const T& source);

  template <typename T>
  typename boost::enable_if<boost::has_dereference<T>, void>::type
  writeXml(std::ostream& out, const T& source);

  template <typename T>
  typename boost::enable_if<is_container<T>, void>::type
  writeXml(std::ostream& out, const T& source);

  template <typename T>
  typename boost::disable_if<boost::mpl::or_<boost::has_dereference<T>,
                                             is_container<T> >, void>::type
  writeXml(std::ostream& out, const T& source);

private:
  const XmlTagsList& _l;
};

XmlWriter::XmlWriter(const XmlTagsList& xmlTagsList) : _l(xmlTagsList)
{
}


template <typename T>
typename boost::enable_if<boost::has_dereference<T>, void>::type
XmlWriter::writeXml(std::ostream& out, const T& source)
{
  if (source)
    writeXml(out, *source);
}

template <typename T>
typename boost::enable_if<is_container<T>, void>::type
XmlWriter::writeXml(std::ostream& out, const T& source)
{
  for (typename T::const_iterator element = source.begin(); element != source.end(); ++element)
  {
    writeXml(out, *element);
  }
}

template <typename T>
typename boost::disable_if<boost::mpl::or_<boost::has_dereference<T>, is_container<T> >,
                           void>::type
XmlWriter::writeXml(std::ostream& out, const T& source)
{
  out << "<" << _l.getTagName<T>();
  writeAllAttributes(out, source);
  out << ">";
  writeAllChildren(out, source);
  out << "</" << _l.getTagName<T>() << ">";
}

// ************************************
// TaxRq - Request
void
XmlWriter::writeXml(std::ostream& out, const InputRequest& request)
{
  typedef boost::date_time::date_facet<boost::gregorian::date, char> DateFacet;
  std::locale initialLocale = out.getloc();
  out.imbue(std::locale(initialLocale, new DateFacet("%Y-%m-%d")));
  writeXml<InputRequest>(out, request);
  out.imbue(initialLocale);
}

void
XmlWriter::writeXml(std::ostream& out, const InputRequestWithStringCache& request)
{
  typedef boost::date_time::date_facet<boost::gregorian::date, char> DateFacet;
  std::locale initialLocale = out.getloc();
  out.imbue(std::locale(initialLocale, new DateFacet("%Y-%m-%d")));
  writeXml<InputRequestWithStringCache>(out, request);
  out.imbue(initialLocale);
}

template <>
void
XmlWriter::writeAllChildren<InputRequest>(std::ostream& out, const InputRequest& request)
{
  writeXml(out, request.processing());
  writeXml(out, request.ticketingOptions());
  writeXml(out, request.pointsOfSale());
  writeXml(out, request.passengers());
  writeXml(out, request.geoPaths());
  writeXml(out, request.prevTicketGeoPath());
  writeXml(out, request.flights());
  writeXml(out, request.flightPaths());
  writeXml(out, request.geoPathMappings());
  writeXml(out, request.fares());
  writeXml(out, request.farePaths());
  writeXml(out, request.yqYrs());
  writeXml(out, request.yqYrPaths());
  writeXml(out, request.optionalServices());
  writeXml(out, request.optionalServicePaths());
  writeXml(out, request.itins());
  writeXml(out, request.ticketingFees());
  writeXml(out, request.ticketingFeePaths());
  writeXml(out, request.changeFees());
  writeXml(out, request.diagnostic());
}

// *************************************
// WithCache variant

template <>
void
XmlWriter::writeAllChildren<InputRequestWithStringCache>(std::ostream& out, const InputRequestWithStringCache& rqWithCache)
{
  writeAllChildren(out, rqWithCache.request);
  //writeXml(out, rqWithCache.xmlCache);
  out << "<TestData>" << rqWithCache.cache << "</TestData>";
}

template <>
void
XmlWriter::writeAllAttributes<InputRequestWithStringCache>(std::ostream& , const InputRequestWithStringCache& )
{
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<XmlCache>(std::ostream& out, const XmlCache& cache)
{
  writeXml(out, cache.nations());
  // TODO: writeXml(out, cache.members);
}

template <>
void
XmlWriter::writeAllAttributes<XmlCache>(std::ostream& , const XmlCache& )
{
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<Nation>(std::ostream& , const Nation& )
{
}

template <>
void
XmlWriter::writeAllAttributes<Nation>(std::ostream& out, const Nation& nation)
{
  writeAttribute(out, nation.locCode(), _l.getAttributeName_LocCode());
  writeAttribute(out, nation.nationCode(), _l.getAttributeName_NationCode());
  writeAttribute(out, nation.cityCode(), _l.getAttributeName_CityCode());
  writeAttribute(out, nation.alaskaZone(), _l.getAttributeName_AlaskaZone());
  writeAttribute(out, nation.state(), _l.getAttributeName_State());
  writeAttribute(out, nation.currency(), _l.getAttributeName_Currency());
}

// *************************************
//  end WithCache variant

template <>
void
XmlWriter::writeAllAttributes<InputRequest>(std::ostream& out, const InputRequest& request)
{
  if (!request.echoToken().empty())
    writeAttribute(out, request.echoToken(), _l.getAttributeName_EchoToken());
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<InputDiagnosticCommand>(std::ostream& out, const InputDiagnosticCommand& diagnostic)
{
  writeXml(out, diagnostic._parameters);
}

template <>
void
XmlWriter::writeAllAttributes<InputDiagnosticCommand>(std::ostream& out, const InputDiagnosticCommand& diagnostic)
{
  writeAttribute(out, diagnostic._number, _l.getAttributeName_Id());
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<InputParameter>(std::ostream&, const InputParameter&)
{
}

template <>
void
XmlWriter::writeAllAttributes<InputParameter>(std::ostream& out, const InputParameter& parameter)
{
  writeAttribute(out, parameter.name(), _l.getAttributeName_Name());
  writeAttribute(out, parameter.value(), _l.getAttributeName_Value());
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<InputChangeFee>(std::ostream&, const InputChangeFee&)
{
}

template <>
void
XmlWriter::writeAllAttributes<InputChangeFee>(std::ostream& out, const InputChangeFee& changeFee)
{
  writeAttribute(out, changeFee._id, _l.getAttributeName_Id());
  writeAttribute(out, changeFee._amount, _l.getAttributeName_Amount());
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<InputItin>(std::ostream&, const InputItin&)
{
}

template <>
void
XmlWriter::writeAllAttributes<InputItin>(std::ostream& out, const InputItin& itin)
{
  writeAttribute(out, itin._id, _l.getAttributeName_Id());
  writeAttribute(out, itin._geoPathRefId, _l.getAttributeName_GeoPathRefId());
  writeAttribute(out, itin._farePathRefId, _l.getAttributeName_FarePathRefId());
  writeAttribute(out, itin._flightPathRefId, _l.getAttributeName_FlightPathRefId());
  writeAttribute(out, itin._yqYrPathRefId, _l.getAttributeName_YqYrPathRefId());
  writeAttribute(out, itin._optionalServicePathRefId, _l.getAttributeName_OptionalServicePathRefId());
  writeAttribute(out, itin._farePathGeoPathMappingRefId, _l.getAttributeName_FarePathGeoPathMappingRefId());
  writeAttribute(out, itin._yqYrPathGeoPathMappingRefId, _l.getAttributeName_YqYrPathGeoPathMappingRefId());
  writeAttribute(out, itin._optionalServicePathGeoPathMappingRefId, _l.getAttributeName_OptionalServicePathGeoPathMappingRefId());
  writeAttribute(out, itin._travelOriginDate, _l.getAttributeName_TravelOriginDate());
  writeAttribute(out, itin._passengerRefId, _l.getAttributeName_PassengerRefId());
  writeAttribute(out, itin._pointOfSaleRefId, _l.getAttributeName_PointOfSaleRefId());
  writeAttribute(out, itin._changeFeeRefId, _l.getAttributeName_ChangeFeeRefId());
  writeAttribute(out, itin._ticketingFeeRefId, _l.getAttributeName_TicketingFeeRefId());
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<InputTicketingFeePath>(std::ostream& out, const InputTicketingFeePath& ticketingFeePath)
{
  writeXml(out, ticketingFeePath._ticketingFeeUsages);
}

template <>
void
XmlWriter::writeAllAttributes<InputTicketingFeePath>(std::ostream& out, const InputTicketingFeePath& ticketingFeePath)
{
  writeAttribute(out, ticketingFeePath._id, _l.getAttributeName_Id());
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<InputTicketingFeeUsage>(std::ostream&, const InputTicketingFeeUsage&)
{
}

template <>
void
XmlWriter::writeAllAttributes<InputTicketingFeeUsage>(std::ostream& out, const InputTicketingFeeUsage& ticketingFeeUsage)
{
  writeAttribute(out, ticketingFeeUsage._ticketingFeeRefId, _l.getAttributeName_TicketingFeeRefId());
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<InputTicketingFee>(std::ostream&, const InputTicketingFee&)
{
}

template <>
void
XmlWriter::writeAllAttributes<InputTicketingFee>(std::ostream& out, const InputTicketingFee& ticketingFee)
{
  writeAttribute(out, ticketingFee._id, _l.getAttributeName_Id());
  writeAttribute(out, ticketingFee._amount, _l.getAttributeName_Amount());
  writeAttribute(out, ticketingFee._taxAmount, _l.getAttributeName_TaxAmount());
  writeAttribute(out, ticketingFee._subCode, _l.getAttributeName_ServiceSubTypeCode());
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<InputYqYrPath>(std::ostream& out, const InputYqYrPath& yqYrPath)
{
  writeXml(out, yqYrPath._yqYrUsages);
}

template <>
void
XmlWriter::writeAllAttributes<InputYqYrPath>(std::ostream& out, const InputYqYrPath& yqYrPath)
{
  writeAttribute(out, yqYrPath._totalAmount, _l.getAttributeName_TotalAmount());
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<InputYqYrUsage>(std::ostream&, const InputYqYrUsage&)
{
}

template <>
void
XmlWriter::writeAllAttributes<InputYqYrUsage>(std::ostream& out, const InputYqYrUsage& yqYrUsage)
{
  writeAttribute(out, yqYrUsage._yqYrRefId, _l.getAttributeName_YqYrRefId());
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<InputYqYr>(std::ostream&, const InputYqYr&)
{
}

template <>
void
XmlWriter::writeAllAttributes<InputYqYr>(std::ostream& out, const InputYqYr& yqYr)
{
  writeAttribute(out, yqYr._id, _l.getAttributeName_Id());
  writeAttribute(out, yqYr._code, _l.getAttributeName_Code());
  writeAttribute(out, yqYr._type, _l.getAttributeName_Type());
  writeAttribute(out, yqYr._amount, _l.getAttributeName_Amount());
  writeAttribute(out, yqYr._taxIncluded, _l.getAttributeName_TaxIncluded());
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<InputFare>(std::ostream&, const InputFare&)
{
}

template <>
void
XmlWriter::writeAllAttributes<InputFare>(std::ostream& out, const InputFare& fare)
{
  writeAttribute(out, fare._id, _l.getAttributeName_Id());
  writeAttribute(out, fare._basis, _l.getAttributeName_BasisCode());
  writeAttribute(out, fare._type, _l.getAttributeName_TypeCode());
  writeAttribute(out, fare._oneWayRoundTrip, _l.getAttributeName_OneWayRoundTrip());
  writeAttribute(out, fare._directionality, _l.getAttributeName_Directionality());
  writeAttribute(out, fare._amount, _l.getAttributeName_Amount());
  writeAttribute(out, fare._sellAmount, _l.getAttributeName_SellAmount());
  writeAttribute(out, fare._rule, _l.getAttributeName_Rule());
  writeAttribute(out, fare._isNetRemitAvailable, _l.getAttributeName_IsNetRemitAvailable());
  writeAttribute(out, fare._tariff, _l.getAttributeName_Tariff());
  writeAttribute(out, fare._tariffInd, _l.getAttributeName_TariffInd());
  writeAttribute(out, fare._outputPtc, _l.getAttributeName_OutputPassengerCode());
  writeAttribute(out, fare._markupAmount, _l.getAttributeName_MarkupAmount());
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<InputOptionalService>(std::ostream&, const InputOptionalService&)
{
}

template <>
void
XmlWriter::writeAllAttributes<InputOptionalService>(std::ostream& out, const InputOptionalService& optionalService)
{
  writeAttribute(out, optionalService._id, _l.getAttributeName_Id());
  writeAttribute(out, optionalService._subCode, _l.getAttributeName_ServiceSubTypeCode());
  writeAttribute(out, optionalService._type, _l.getAttributeName_Type());
  writeAttribute(out, optionalService._serviceGroup, _l.getAttributeName_SvcGroup());
  writeAttribute(out, optionalService._serviceSubGroup, _l.getAttributeName_SvcSubGroup());
  writeAttribute(out, optionalService._amount, _l.getAttributeName_Amount());
  writeAttribute(out, optionalService._ownerCarrier, _l.getAttributeName_OwnerCarrier());
  writeAttribute(out, optionalService._pointOfDeliveryLoc, _l.getAttributeName_PointOfDeliveryLoc());
  writeAttribute(out, optionalService._outputPtc, _l.getAttributeName_OutputPassengerCode());
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<InputOptionalServicePath>(std::ostream& out, const InputOptionalServicePath& optionalServicePath)
{
  writeXml(out, optionalServicePath._optionalServiceUsages);
}

template <>
void
XmlWriter::writeAllAttributes<InputOptionalServicePath>(std::ostream& out, const InputOptionalServicePath& optionalServicePath)
{
  writeAttribute(out, optionalServicePath._id, _l.getAttributeName_Id());
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<InputOptionalServiceUsage>(std::ostream&, const InputOptionalServiceUsage&)
{
}

template <>
void
XmlWriter::writeAllAttributes<InputOptionalServiceUsage>(std::ostream& out, const InputOptionalServiceUsage& optionalServiceUsage)
{
  writeAttribute(out, optionalServiceUsage._optionalServiceRefId, _l.getAttributeName_OptionalServiceRefId());
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<InputFarePath>(std::ostream& out, const InputFarePath& farePath)
{
  writeXml(out, farePath._fareUsages);
}

template <>
void
XmlWriter::writeAllAttributes<InputFarePath>(std::ostream& out, const InputFarePath& farePath)
{
  writeAttribute(out, farePath._id, _l.getAttributeName_Id());
  writeAttribute(out, farePath._totalAmount, _l.getAttributeName_TotalAmount());
  writeAttribute(out, farePath._totalAmountBeforeDiscount, _l.getAttributeName_TotalAmountBeforeDiscount());
  writeAttribute(out, farePath._validatingCarrier, _l.getAttributeName_ValidatingCarrier());
  writeAttribute(out, farePath._outputPtc, _l.getAttributeName_OutputPassengerCode());
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<InputFareUsage>(std::ostream&, const InputFareUsage&)
{
}

template <>
void
XmlWriter::writeAllAttributes<InputFareUsage>(std::ostream& out, const InputFareUsage& fareUsage)
{
  writeAttribute(out, fareUsage._fareRefId, _l.getAttributeName_FareRefId());
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<InputFlightPath>(std::ostream& out, const InputFlightPath& flightPath)
{
  writeXml(out, flightPath._flightUsages);
}

template <>
void
XmlWriter::writeAllAttributes<InputFlightPath>(std::ostream& out, const InputFlightPath& flightPath)
{
  writeAttribute(out, flightPath._id, _l.getAttributeName_Id());
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<InputFlightUsage>(std::ostream&, const InputFlightUsage&)
{
}

template <>
void
XmlWriter::writeAllAttributes<InputFlightUsage>(std::ostream& out, const InputFlightUsage& flightUsage)
{
  writeAttribute(out, flightUsage._flightRefId, _l.getAttributeName_FlightRefId());
  writeAttribute(out, flightUsage._bookingCodeType, _l.getAttributeName_BookingCodeType());
  writeAttribute(out, flightUsage._connectionDateShift, _l.getAttributeName_ConnectionDateShift());
  writeAttribute(out, flightUsage._openSegmentIndicator, _l.getAttributeName_SkipDateTime());
  writeAttribute(out, flightUsage._forcedConnection, _l.getAttributeName_ForcedConnection());
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<InputFlight>(std::ostream&, const InputFlight&)
{
}

template <>
void
XmlWriter::writeAllAttributes<InputFlight>(std::ostream& out, const InputFlight& flight)
{
  writeAttribute(out, flight._id, _l.getAttributeName_Id());
  writeAttribute(out, flight._departureTime, _l.getAttributeName_DepartureTime());
  writeAttribute(out, flight._arrivalTime, _l.getAttributeName_ArrivalTime());
  writeAttribute(out, flight._arrivalDateShift, _l.getAttributeName_ArrivalDateShift());
  writeAttribute(out, flight._marketingCarrierFlightNumber, _l.getAttributeName_MarketingCarrierFlightNumber());
  writeAttribute(out, flight._marketingCarrier, _l.getAttributeName_MarketingCarrier());
  writeAttribute(out, flight._operatingCarrier, _l.getAttributeName_OperatingCarrier());
  writeAttribute(out, flight._equipment, _l.getAttributeName_Equipment());
  writeAttribute(out, flight._cabinCode, _l.getAttributeName_CabinCode());
  writeAttribute(out, flight._reservationDesignator, _l.getAttributeName_ReservationDesignator());
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<InputGeoPathMapping>(std::ostream& out, const InputGeoPathMapping& geoPathMapping)
{
  writeXml(out, geoPathMapping._mappings);
}

template <>
void
XmlWriter::writeAllAttributes<InputGeoPathMapping>(std::ostream& out, const InputGeoPathMapping& geoPathMapping)
{
  writeAttribute(out, geoPathMapping._id, _l.getAttributeName_Id());
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<InputMapping>(std::ostream& out, const InputMapping& mapping)
{
  writeXml(out, mapping.maps());
}

template <>
void
XmlWriter::writeAllAttributes<InputMapping>(std::ostream&, const InputMapping&)
{
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<InputMap>(std::ostream&, const InputMap&)
{
}

template <>
void
XmlWriter::writeAllAttributes<InputMap>(std::ostream& out, const InputMap& map)
{
  writeAttribute(out, map._geoRefId, _l.getAttributeName_GeoRefId());
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<InputGeoPath>(std::ostream& out, const InputGeoPath& geoPath)
{
  writeXml(out, geoPath._geos);
}

template <>
void
XmlWriter::writeAllAttributes<InputGeoPath>(std::ostream& out, const InputGeoPath& geoPath)
{
  writeAttribute(out, geoPath._id, _l.getAttributeName_Id());
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<InputPrevTicketGeoPath>(std::ostream& out, const InputPrevTicketGeoPath& geoPath)
{
  writeXml(out, geoPath._geos);
}

template <>
void
XmlWriter::writeAllAttributes<InputPrevTicketGeoPath>(std::ostream& out, const InputPrevTicketGeoPath& geoPath)
{
  writeAttribute(out, geoPath._id, _l.getAttributeName_Id());
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<InputGeo>(std::ostream&, const InputGeo&)
{
}

template <>
void
XmlWriter::writeAllAttributes<InputGeo>(std::ostream& out, const InputGeo& geo)
{
  writeAttribute(out, geo._loc.tag(), _l.getAttributeName_Type());
  writeAttribute(out, geo._loc.code(), _l.getAttributeName_Loc());
  writeAttribute(out, geo._unticketedTransfer, _l.getAttributeName_UnticketedTransfer());
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<InputPassenger>(std::ostream&, const InputPassenger&)
{
}

template <>
void
XmlWriter::writeAllAttributes<InputPassenger>(std::ostream& out, const InputPassenger& passenger)
{
  writeAttribute(out, passenger._id, _l.getAttributeName_Id());
  writeAttribute(out, passenger._code, _l.getAttributeName_PassengerCode());
  writeAttribute(out, passenger._birthDate, _l.getAttributeName_DateOfBirth());
  writeAttribute(out, passenger._stateCode, _l.getAttributeName_StateCode());
  writeAttribute(out, passenger._passengerStatus._nationality, _l.getAttributeName_Nationality());
  writeAttribute(out, passenger._passengerStatus._residency, _l.getAttributeName_Residency());
  writeAttribute(out, passenger._passengerStatus._employment, _l.getAttributeName_Employment());
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<InputPointOfSale>(std::ostream&, const InputPointOfSale&)
{
}

template <>
void
XmlWriter::writeAllAttributes<InputPointOfSale>(std::ostream& out, const InputPointOfSale& pos)
{
  writeAttribute(out, pos.id(), _l.getAttributeName_Id());
  writeAttribute(out, pos.loc(), _l.getAttributeName_PointOfSaleLoc());

  if (!pos.agentPcc().empty())
    writeAttribute(out, pos.agentPcc(), _l.getAttributeName_AgentPCC());

  writeAttribute(out, pos.vendorCrsCode(), _l.getAttributeName_VendorCrsCode());
  writeAttribute(out, pos.carrierCode(), _l.getAttributeName_CarrierCode());
  writeAttribute(out, pos.agentDuty(), _l.getAttributeName_DutyCode());
  writeAttribute(out, pos.agentFunction(), _l.getAttributeName_FunctionCode());
  writeAttribute(out, pos.agentCity(), _l.getAttributeName_AgentCity());
  writeAttribute(out, pos.iataNumber(), _l.getAttributeName_IATANumber());
  writeAttribute(out, pos.ersp(), _l.getAttributeName_ERSP());
  writeAttribute(out, pos.agentAirlineDept(), _l.getAttributeName_AgentAirlineDept());
  writeAttribute(out, pos.agentOfficeDesignator(), _l.getAttributeName_AgentOfficeDesignator());
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<InputTicketingOptions>(std::ostream&, const InputTicketingOptions&)
{
}

template <>
void
XmlWriter::writeAllAttributes<InputTicketingOptions>(std::ostream& out, const InputTicketingOptions& options)
{
  writeAttribute(out, options.ticketingDate(), _l.getAttributeName_TicketingDate());
  writeAttribute(out, options.ticketingTime(), _l.getAttributeName_TicketingTime());
  writeAttribute(out, options.ticketingPoint(), _l.getAttributeName_TicketingPoint());
  writeAttribute(out, options.paymentCurrency(), _l.getAttributeName_PaymentCurrency());
  writeAttribute(out, options.formOfPayment(), _l.getAttributeName_FormOfPayment());
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<InputProcessingOptions>(std::ostream& out, const InputProcessingOptions& options)
{
  writeXml(out, options.taxDetailsLevel());
  writeXml(out, options.exemptedRules());
  writeXml(out, options.getApplicableGroups());
}

template <>
void
XmlWriter::writeAllAttributes<InputProcessingOptions>(std::ostream& out, const InputProcessingOptions& options)
{
  writeAttribute(out, options._geoPathsSize, _l.getAttributeName_GeoPathsSize());
  writeAttribute(out, options._geoPathMappingsSize, _l.getAttributeName_GeoPathMappingsSize());
  writeAttribute(out, options._faresSize, _l.getAttributeName_FaresSize());
  writeAttribute(out, options._farePathsSize, _l.getAttributeName_FarePathsSize());
  writeAttribute(out, options._flightsSize, _l.getAttributeName_FlightsSize());
  writeAttribute(out, options._flightPathsSize, _l.getAttributeName_FlightPathsSize());
  writeAttribute(out, options._yqYrsSize, _l.getAttributeName_YqYrsSize());
  writeAttribute(out, options._yqYrPathsSize, _l.getAttributeName_YqYrPathsSize());
  writeAttribute(out, options._itinsSize, _l.getAttributeName_ItinsSize());

  writeAttribute(out, options._tch, _l.getAttributeName_TCH());
  writeAttribute(out, options._rtw, _l.getAttributeName_RoundTheWorld());
  writeAttribute(out, options._applyUSCAgrouping, _l.getAttributeName_ApplyUSCAGrouping());
  // TODO: other attrs, but they are not read from TaxRq.
}

// ...................................
template <>
void
XmlWriter::writeAllChildren<InputTaxDetailsLevel>(std::ostream&, const InputTaxDetailsLevel&)
{
}

template <>
void
XmlWriter::writeAllAttributes<InputTaxDetailsLevel>(std::ostream& out, const InputTaxDetailsLevel& level)
{
  writeAttribute(out, level.allDetails(), _l.getAttributeName_AllDetails());
  writeAttribute(out, level.geoDetails(), _l.getAttributeName_GeoDetails());
  writeAttribute(out, level.calcDetails(), _l.getAttributeName_CalcDetails());
  writeAttribute(out, level.taxOnTaxDetails(), _l.getAttributeName_TaxOnTaxDetails());
  writeAttribute(out, level.taxOnFaresDetails(), _l.getAttributeName_TaxOnFaresDetails());
  writeAttribute(out, level.taxOnYQYRDetails(), _l.getAttributeName_TaxOnYQYRDetails());
  writeAttribute(out, level.taxOnOptionalServiceDetails(), _l.getAttributeName_TaxOnOptionalServicesDetails());
  writeAttribute(out, level.taxOnExchangeReissueDetails(), _l.getAttributeName_TaxOnExchangeReissueDetails());
  writeAttribute(out, level.exchangeReissueDetails(), _l.getAttributeName_ExchangeReissueDetails());
}

// ......................................
template <>
void
XmlWriter::writeAllChildren<InputExemptedRule>(std::ostream&, const InputExemptedRule&)
{
}

template <>
void
XmlWriter::writeAllAttributes<InputExemptedRule>(std::ostream& out, const InputExemptedRule& level)
{
  writeAttribute(out, level._ruleRefId, _l.getAttributeName_RuleRefId());
}

// ......................................
template <>
void
XmlWriter::writeAllChildren<InputApplyOn>(std::ostream&, const InputApplyOn&)
{
}

template <>
void
XmlWriter::writeAllAttributes<InputApplyOn>(std::ostream& out, const InputApplyOn& applyOn)
{
  writeAttribute(out, applyOn._group, _l.getAttributeName_ProcessingGroup());
}

// ************************************
// BCH Response
void
XmlWriter::writeXml(std::ostream& out, const BCHOutputResponse& response)
{
  writeXml<BCHOutputResponse>(out, response);
}

template <>
void
XmlWriter::writeAllAttributes<BCHOutputResponse>(std::ostream&, const BCHOutputResponse&)
{
}

template <>
void
XmlWriter::writeAllChildren<BCHOutputResponse>(std::ostream& out, const BCHOutputResponse& response)
{
  for (auto detail : response.constTaxDetails())
    writeXml(out, detail);

  for (auto itin : response.constItins())
    writeXml(out, itin);

  writeXml(out, response.constDiagnostic());
  writeXml(out, response.constError());
}

template <>
void
XmlWriter::writeAllAttributes<BCHOutputTaxDetail>(std::ostream& out, const BCHOutputTaxDetail& detail)
{
  writeAttribute(out, detail.getId(), _l.getOutputAttributeName_BCHTaxId());
  writeAttribute(out, detail.getSabreCode(), _l.getOutputAttributeName_BCHTaxCode());
  writeAttribute(out, detail.getPaymentAmt(), _l.getOutputAttributeName_BCHTaxAmount());
  writeAttribute(out, detail.getPaymentAmt(), _l.getOutputAttributeName_BCHTaxAmountAdjusted());
  writeAttribute(out, detail.getName(), _l.getOutputAttributeName_BCHTaxDescription());
}

template <>
void
XmlWriter::writeAllChildren<BCHOutputTaxDetail>(std::ostream&, const BCHOutputTaxDetail&)
{
}

template <>
void
XmlWriter::writeAllAttributes<BCHOutputItin>(std::ostream& out, const BCHOutputItin& itin)
{
  writeAttribute(out, itin.getId(), _l.getOutputAttributeName_BCHItinId());
}

template <>
void
XmlWriter::writeAllChildren<BCHOutputItin>(std::ostream& out, const BCHOutputItin& itin)
{
  writeXml(out, itin.constPaxDetail());
}

template <>
void
XmlWriter::writeAllAttributes<BCHOutputItinPaxDetail>(std::ostream& out, const BCHOutputItinPaxDetail& paxDetail)
{
  writeAttribute(out, paxDetail.getPtc(), _l.getOutputAttributeName_BCHPaxType());
  writeAttribute(out, paxDetail.getPtcNumber(), _l.getOutputAttributeName_BCHPaxCount());
  writeAttribute(out, paxDetail.getTotalAmount(), _l.getOutputAttributeName_BCHPaxTotalAmount());
  std::ostringstream taxList;
  for (auto taxId : paxDetail.constTaxIds())
  {
    taxList << taxId << "|";
  }
  std::string taxListStr = taxList.str();
  taxListStr.pop_back();
  writeAttribute(out, taxListStr, _l.getOutputAttributeName_BCHPaxTaxes());
}

template <>
void
XmlWriter::writeAllChildren<BCHOutputItinPaxDetail>(std::ostream&, const BCHOutputItinPaxDetail&)
{
}

// ************************************
// TaxRs - Response
void
XmlWriter::writeXml(std::ostream& out, const OutputResponse& response)
{
  out << std::boolalpha;
  writeXml<OutputResponse>(out, response);
}

template <>
void
XmlWriter::writeAllAttributes<OutputResponse>(std::ostream& out, const OutputResponse& response)
{
  if (!response.echoToken().empty())
    writeAttribute(out, response.echoToken(), _l.getOutputAttributeName_EchoToken());
}

template <>
void
XmlWriter::writeAllChildren<OutputResponse>(std::ostream& out, const OutputResponse& response)
{
  writeXml(out, response.itins());
  writeXml(out, response.diagnostic());
  writeXml(out, response.error());
}

// ************************************
// Itins (OutputItins)
template <>
void
XmlWriter::writeAllChildren<OutputItins>(std::ostream& out, OutputItins const& itins)
{
  writeXml(out, itins.taxDetailsSeq());
  writeXml(out, itins.taxGroupSeq());
  writeXml(out, itins.itinSeq());
}

template <>
void
XmlWriter::writeAllAttributes<OutputItins>(std::ostream& out, OutputItins const& itins)
{
  writeAttribute(out, itins.paymentCur(), _l.getOutputAttributeName_PaymentCur());
}

// ************************************
// Diagnostic (OutputDiagnostic)
template <>
void
XmlWriter::writeAllChildren<OutputDiagnostic>(std::ostream& out,
                                              const OutputDiagnostic& diagnosticRsp)
{
  writeXml(out, diagnosticRsp.messages());
}

template <>
void
XmlWriter::writeAllAttributes<OutputDiagnostic>(std::ostream& /*out*/, const OutputDiagnostic&)
{
}

// ************************************
// Error (OutputError)
template <>
void
XmlWriter::writeAllChildren<OutputError>(std::ostream& out, const OutputError& errorRsp)
{
  writeXml(out, errorRsp.messages());
}

template <>
void
XmlWriter::writeAllAttributes<OutputError>(std::ostream& /*out*/, const OutputError&)
{
}

// ************************************
// Messeage (OutputMessage)
template <>
void
XmlWriter::writeAllChildren<OutputMessage>(std::ostream& /*out*/, const OutputMessage& /*message*/)
{
}

template <>
void
XmlWriter::writeAllAttributes<OutputMessage>(std::ostream& out, const OutputMessage& message)
{
  writeAttribute(out, message.content(), _l.getOutputAttributeName_Content());
}

// ************************************
// Itin (OutputItin)
template <>
void
XmlWriter::writeAllChildren<OutputItin>(std::ostream&, OutputItin const&)
{
}

template <>
void
XmlWriter::writeAllAttributes<OutputItin>(std::ostream& out, OutputItin const& outputItin)
{
  writeAttribute(out, outputItin.id(), _l.getOutputAttributeName_Id());
  writeAttribute(out, outputItin.taxGroupId(), _l.getOutputAttributeName_TaxGroupId());
  writeAttribute(out,
                 outputItin.taxOnOptionalServiceGroupRefId(),
                 _l.getOutputAttributeName_TaxOnOptionalServiceGroupRefId());
  writeAttribute(out,
                 outputItin.getTaxOnBaggageGroupRefId(),
                 _l.getOutputAttributeName_TaxOnBaggageGroupRefId());
  writeAttribute(out,
                 outputItin.taxOnChangeFeeGroupRefId(),
                 _l.getOutputAttributeName_TaxOnChangeFeeGroupRefId());
}

// ************************************
// TaxGroup (OutputTaxGroup)
template <>
void
XmlWriter::writeAllChildren<OutputTaxGroup>(std::ostream& out, OutputTaxGroup const& outputTaxGroup)
{
  writeXml(out, outputTaxGroup.taxSeq());
}
template <>
void
XmlWriter::writeAllAttributes<OutputTaxGroup>(std::ostream& out,
                                              OutputTaxGroup const& outputTaxGroup)
{
  writeAttribute(out, outputTaxGroup.id(), _l.getOutputAttributeName_Id());
  writeAttribute(out, outputTaxGroup.type(), _l.getOutputAttributeName_TaxGroupType());
  writeAttribute(out, outputTaxGroup.totalAmt(), _l.getOutputAttributeName_TotalAmt());
}

// ************************************
// Tax (OutputTax)
template <>
void
XmlWriter::writeAllChildren<OutputTax>(std::ostream& out, OutputTax const& tax)
{
  writeXml(out, tax.taxDetailsRef());
}

template <>
void
XmlWriter::writeAllAttributes<OutputTax>(std::ostream& out, OutputTax const& tax)
{
  if (tax.taxName())
  {
    writeAttribute(out, tax.taxName()->nation(), _l.getOutputAttributeName_Nation());
    writeAttribute(out, tax.taxName()->taxCode(), _l.getOutputAttributeName_Code());
    writeAttribute(out, tax.taxName()->taxType(), _l.getOutputAttributeName_Type());
    writeAttribute(out, tax.taxName()->taxPointTag(), _l.getOutputAttributeName_TaxPointTag());
    writeAttribute(
        out, tax.taxName()->percentFlatTag(), _l.getOutputAttributeName_PercentageFlatTag());
  }
  writeAttribute(out, tax.totalAmt(), _l.getOutputAttributeName_TotalAmt());
}

// ************************************
// TaxDetailsRef (OutputTaxDetailsRef)
template <>
void
XmlWriter::writeAllChildren<OutputTaxDetailsRef>(std::ostream&, const OutputTaxDetailsRef&)
{
}

template <>
void
XmlWriter::writeAllAttributes<OutputTaxDetailsRef>(std::ostream& out,
                                                   const OutputTaxDetailsRef& taxDetailsRef)
{
  writeAttribute(out, taxDetailsRef.id(), _l.getOutputAttributeName_ElementRefId());
}

// ************************************
// CalcDetails (OutputCalcDetails)
template <>
void
XmlWriter::writeAllChildren<OutputCalcDetails>(std::ostream& /*out*/,
                                               OutputCalcDetails const& /*calcDetails*/)
{
}

template <>
void
XmlWriter::writeAllAttributes<OutputCalcDetails>(std::ostream& out,
                                                 OutputCalcDetails const& calcDetails)
{
  writeAttribute(
      out, calcDetails.taxCurToPaymentCurBSR, _l.getOutputAttributeName_TaxCurToPaymentCurBSR());
  writeAttribute(out, calcDetails.roundingUnit, _l.getOutputAttributeName_TaxRoundingUnit());
  writeAttribute(out, calcDetails.roundingDir, _l.getOutputAttributeName_TaxRoundingDir());
}

// ************************************
// FaresDetails (OutputFaresDetails)
template <>
void
XmlWriter::writeAllChildren<OutputFaresDetails>(std::ostream&, const OutputFaresDetails&)
{
}

template <>
void
XmlWriter::writeAllAttributes<OutputFaresDetails>(std::ostream& out,
                                                  const OutputFaresDetails& details)
{
  writeAttribute(out, details.totalAmt, _l.getOutputAttributeName_TotalAmt());
}

// ************************************
// YQYRDetails (OutputYQYRDetails)
template <>
void
XmlWriter::writeAllChildren<OutputYQYRDetails>(std::ostream&, const OutputYQYRDetails&)
{
}

template <>
void
XmlWriter::writeAllAttributes<OutputYQYRDetails>(std::ostream& out,
                                                 const OutputYQYRDetails& details)
{
  writeAttribute(out, details.totalAmt, _l.getOutputAttributeName_TotalAmt());
}

// ************************************
// OCDetails (OutputOCDetails)
template <>
void
XmlWriter::writeAllChildren<OutputOCDetails>(std::ostream&, const OutputOCDetails&)
{
}

template <>
void
XmlWriter::writeAllAttributes<OutputOCDetails>(std::ostream& out, const OutputOCDetails& details)
{
  writeAttribute(out, details.totalAmt, _l.getOutputAttributeName_TotalAmt());
}

// ************************************
// BaggageDetails (OutputBaggageDetails)
template <>
void
XmlWriter::writeAllChildren<OutputBaggageDetails>(std::ostream&, const OutputBaggageDetails&)
{
}

template <>
void
XmlWriter::writeAllAttributes<OutputBaggageDetails>(std::ostream& out,
                                                    const OutputBaggageDetails& details)
{
  writeAttribute(out, details.totalAmt, _l.getOutputAttributeName_TotalAmt());
}

// ************************************
// OBDetails (OutputOBDetails)
template <>
void
XmlWriter::writeAllChildren<OutputOBDetails>(std::ostream&, const OutputOBDetails&)
{
}

template <>
void
XmlWriter::writeAllAttributes<OutputOBDetails>(std::ostream& out, const OutputOBDetails& details)
{
  writeAttribute(out, details.totalAmt, _l.getOutputAttributeName_TotalAmt());
}

// ************************************
// TaxOnTaxDetails (OutputTaxOnTaxDetails)
template <>
void
XmlWriter::writeAllChildren<OutputTaxOnTaxDetails>(std::ostream& out,
                                                   OutputTaxOnTaxDetails const& taxOnTaxDetails)
{
  writeXml(out, taxOnTaxDetails.taxDetailsRef);
}

template <>
void
XmlWriter::writeAllAttributes<OutputTaxOnTaxDetails>(std::ostream& out,
                                                     const OutputTaxOnTaxDetails& details)
{
  writeAttribute(out, details.totalAmt, _l.getOutputAttributeName_TotalAmt());
}

// ************************************
// GeoDetails (OutputGeoDetails)
template <>
void
XmlWriter::writeAllChildren<OutputGeoDetails>(std::ostream&, const OutputGeoDetails&)
{
}

template <>
void
XmlWriter::writeAllAttributes<OutputGeoDetails>(std::ostream& out, const OutputGeoDetails& d)
{
  writeAttribute(out, d.unticketedPoint(), _l.getOutputAttributeName_UnticketedPoint());
  writeAttribute(out, d.taxPointIndexBegin(), _l.getOutputAttributeName_TaxPointIndexBegin());
  writeAttribute(out, d.taxPointIndexEnd(), _l.getOutputAttributeName_TaxPointIndexEnd());
  writeAttribute(out, d.taxPointLoc1(), _l.getOutputAttributeName_TaxPointLoc1());
  writeAttribute(out, d.taxPointLoc2(), _l.getOutputAttributeName_TaxPointLoc2());
  writeAttribute(out, d.taxPointLoc3(), _l.getOutputAttributeName_TaxPointLoc3());
  writeAttribute(out, d.journeyLoc1(), _l.getOutputAttributeName_JourneyLoc1());
  writeAttribute(out, d.journeyLoc2(), _l.getOutputAttributeName_JourneyLoc2());
  writeAttribute(out, d.pointOfSaleLoc(), _l.getOutputAttributeName_PointOfSaleLoc());
  writeAttribute(out, d.pointOfTicketingLoc(), _l.getOutputAttributeName_PointOfTicketingLoc());
}

// OptionalServiceDetails (OutputOptionalServiceDetails)
template <>
void
XmlWriter::writeAllChildren<OutputOptionalServiceDetails>(std::ostream&,
                                                          const OutputOptionalServiceDetails&)
{
}

template <>
void
XmlWriter::writeAllAttributes<OutputOptionalServiceDetails>(std::ostream& out,
                                                            const OutputOptionalServiceDetails& d)
{
  writeAttribute(out, d.amount, _l.getOutputAttributeName_PaymentAmt());
  writeAttribute(out, d.subCode, _l.getOutputAttributeName_ServiceSubTypeCode());
  writeAttribute(out, d.serviceGroup, _l.getOutputAttributeName_SvcGroup());
  writeAttribute(out, d.serviceSubGroup, _l.getOutputAttributeName_SvcSubGroup());
  writeAttribute(out, d.optionalServiceType, _l.getOutputAttributeName_OptionalServiceType());
  writeAttribute(out, d.ownerCarrier, _l.getOutputAttributeName_Carrier());
  writeAttribute(out, d.pointOfDeliveryLoc, _l.getOutputAttributeName_PointOfDeliveryLoc());
}

// TaxDetail (OutputTaxDetail)
template <>
void
XmlWriter::writeAllChildren<OutputTaxDetails>(std::ostream& out, const OutputTaxDetails& d)
{
  writeXml(out, d.taxOnFaresDetails);
  writeXml(out, d.taxOnTaxDetails);
  writeXml(out, d.taxOnYqYrDetails);
  writeXml(out, d.taxOnOcDetails);
  writeXml(out, d.taxOnBaggageDetails);
  writeXml(out, d.obDetails);
  writeXml(out, d.calcDetails());
  writeXml(out, d.geoDetails());
  writeXml(out, d.optionalServiceDetails());
}

template <>
void
XmlWriter::writeAllAttributes<OutputTaxDetails>(std::ostream& out, const OutputTaxDetails& d)
{
  assert(d.id());
  writeAttribute(out, d.id(), _l.getOutputAttributeName_Id());
  writeAttribute(out, d.seqNo(), _l.getOutputAttributeName_SeqNo());
  writeAttribute(out, d.nation(), _l.getOutputAttributeName_Nation());
  writeAttribute(out, d.code(), _l.getOutputAttributeName_Code());
  writeAttribute(out, d.type(), _l.getOutputAttributeName_Type());
  writeAttribute(out, d.sabreCode(), _l.getOutputAttributeName_SabreCode());
  writeAttribute(out, d.paymentAmt(), _l.getOutputAttributeName_PaymentAmt());
  writeAttribute(out, d.publishedAmt(), _l.getOutputAttributeName_PublishedAmt());
  writeAttribute(out, d.publishedCur(), _l.getOutputAttributeName_PublishedCur());
  writeAttribute(out, d.taxPointLocBegin(), _l.getOutputAttributeName_TaxPointLocBegin());
  writeAttribute(out, d.taxPointLocEnd(), _l.getOutputAttributeName_TaxPointLocEnd());
  writeAttribute(out, d.name(), _l.getOutputAttributeName_Name());
  writeAttribute(out, d.carrier(), _l.getOutputAttributeName_Carrier());
  writeAttribute(out, d.gst(), _l.getOutputAttributeName_GST());
  writeAttribute(out, d.taxableUnitTags(), _l.getOutputAttributeName_TaxableUnitTags());
}

void writeXmlResponse(std::ostream& out, const OutputResponse& response, const XmlTagsList& xmlTagslist)
{
  XmlWriter(xmlTagslist).writeXml(out, response);
}

void writeXmlResponse(std::ostream& out, const BCHOutputResponse& response, const XmlTagsList& xmlTagslist)
{
  XmlWriter(xmlTagslist).writeXml(out, response);
}

void writeXmlRequest(std::ostream& out, const InputRequest& request, const XmlTagsList& xmlTagslist)
{
  XmlWriter(xmlTagslist).writeXml(out, request);
}

void writeXmlRequest(std::ostream& out, const InputRequest& request, const std::string& cache, const XmlTagsList& xmlTagslist)
{
  XmlWriter(xmlTagslist).writeXml(out, InputRequestWithStringCache{request, cache});
}

} // namespace tax
