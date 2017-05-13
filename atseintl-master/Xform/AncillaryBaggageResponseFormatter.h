//----------------------------------------------------------------------------
//
//  Copyright Sabre 2012
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

#include "Common/XMLConstruct.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "ServiceFees/OCFees.h"
#include "Xform/AncillaryPricingResponseFormatter.h"

namespace tse
{
class AncRequest;

class AncillaryBaggageResponseFormatter : public AncillaryPricingResponseFormatter
{
  friend class AncillaryBaggageResponseFormatterTest;

public:
  AncillaryBaggageResponseFormatter(AncillaryPricingTrx& ancTrx);

  std::string formatResponse(
      const std::string& responseString,
      ErrorResponseException::ErrorResponseCode errCode = ErrorResponseException::NO_ERROR);

private:
  void constructXMLResponse(XMLConstruct& construct, const std::string& response) const;

  void buildResponse(std::ostringstream& output);
  void buildFootnotesLegend(std::ostringstream& output);
  void buildAllowanceHeader(std::ostringstream& output,
                            const OptionalServicesInfo* s7,
                            const SubCodeInfo* s5) const;

  void buildAllowance(std::ostringstream& output,
                      const BaggageTravel* baggageTravel,
                      const OCFees* ocFees) const;

  void buildAllowance(std::ostringstream& output,
                      const std::vector<OCFees::BaggageItemProperty>& itemProperties) const;

  void buildCharges(std::ostringstream& output,
                    const BaggageTravel* baggageTravel,
                    const std::vector<OCFees*>& chargesOcFees);

  void buildChargeLine(std::ostringstream& output,
                       uint32_t index,
                       const OCFeesUsage* usage,
                       const CarrierCode& carrier,
                       const std::vector<std::string>& descriptionLines,
                       bool usDot);

  bool formatSubCodeDescription(const SubCodeInfo* subCode,
                                uint32_t maxLineLen,
                                std::vector<std::string>& output) const;

  std::string formatPaxType(const PaxType* paxType, bool wpbgPostTicket) const;
  void getFees(std::vector<OCFees*>& ocFees, const FarePath* farePath,
               const std::vector<ServiceFeesGroup*>& serviceFeesGroupVector,
               const std::vector<ServiceGroup>& requestedGroupCodes) const;
  std::vector<OCFees*> getFees(const Itin* itin) const;
  std::string formatFootnotes(const OptionalServicesInfo& s7, bool usDot);
  bool validateRequestedBTIndices(const std::set<uint32_t>& reqBTIndices) const;
  std::string formatWeight(const OptionalServicesInfo& s7) const;

  void split(const std::string& str, uint32_t maxLen, std::vector<std::string>& output) const;

  MoneyAmount getMoneyAmount(const OCFeesUsage* usage) const;

  AncillaryPricingTrx& _trx;
  const AncRequest* _request;
  std::map<const char, bool> _showFootnoteLegend;

  static const char BAG_FEE_APPLIES_PER_KILO;
  static const char BAG_FEE_APPLIES_PER_5KILOS;
  static const char BAG_FEE_APPLIES_AT_EACH_LOC;
  static const char SERVICE_NONREFUNDABLE;
  static const char FREQ_FLYER;
};
}

