// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#pragma once

#include "Xform/PricingResponseFormatter.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class BaggageTrx;
class Diagnostic;
class OCFees;

class BaggageResponseFormatter : public PricingResponseFormatter
{
  friend class BaggageResponseFormatterTest;

public:
  virtual std::string formatResponse(
      const std::string& errorString,
      BaggageTrx& bgTrx,
      ErrorResponseException::ErrorResponseCode errorCode = ErrorResponseException::NO_ERROR);

  void formatResponse(const ErrorResponseException& ere, std::string& response);

private:
  struct FFInfo
  {
    FFInfo(const BaggageProvisionType& baggageProvision,
           uint32_t tierLevel,
           const CarrierCode& carrier,
           const std::set<int>& segmentNumbers)
      : _baggageProvision(baggageProvision),
        _tierLevel(tierLevel),
        _carrier(carrier),
        _segmentNumbers(segmentNumbers)
    {
    }

    BaggageProvisionType _baggageProvision;
    uint32_t _tierLevel;
    CarrierCode _carrier;
    std::set<int> _segmentNumbers;
  };

  using FFInfoList = std::vector<FFInfo>;

  class FFInfoComparator : std::unary_function<BaggageResponseFormatter::FFInfo&, bool>
  {
  private:
    const BaggageResponseFormatter::FFInfo& _ffInfo;

  public:
    FFInfoComparator(const BaggageResponseFormatter::FFInfo& ffInfo) : _ffInfo(ffInfo) {}

    bool operator()(const BaggageResponseFormatter::FFInfo& ffInfo) const
    {
      return _ffInfo._baggageProvision == ffInfo._baggageProvision &&
             _ffInfo._tierLevel == ffInfo._tierLevel && _ffInfo._carrier == ffInfo._carrier;
    }
  };

  void buildErrorAndDiagnosticElements(const std::string& errorString,
                                       BaggageTrx& bgTrx,
                                       ErrorResponseException::ErrorResponseCode errorCode,
                                       XMLConstruct& construct);

  void buildITN(XMLConstruct& construct,
                uint16_t itinIndex,
                const FarePath* farePath,
                BaggageTrx& bgTrx) const;
  void buildPXI(XMLConstruct& construct, const FarePath* farePath, const BaggageTrx& bgTrx) const;
  void buildFFY(XMLConstruct& construct, const FFInfo& ffInfo) const;
  void buildQ00(XMLConstruct& construct, int id) const;
  void buildDCL(XMLConstruct& construct, const FarePath* farePath, BaggageTrx& trx) const;
  void buildSEG(XMLConstruct& construct, const AirSeg* airSeg, const std::string& allowance) const;

  void getSegmentNumbers(const BaggageTravel* baggageTravel, std::set<int>& segmentNumbers) const;
  void collectFFInfo(const OCFees* ocFees,
                     const BaggageProvisionType& baggageProvision,
                     const BaggageTravel* baggageTravel,
                     BaggageResponseFormatter::FFInfoList& ffInfoList) const;

  template <typename T, T(BaggageTravel::*fee)>
  void collectFFInfos(const BaggageProvisionType& baggageProvision,
                      const std::vector<const BaggageTravel*>& baggageTravels,
                      BaggageResponseFormatter::FFInfoList& ffInfoList) const
  {
    for (const BaggageTravel* baggageTravel : baggageTravels)
      collectFFInfo(baggageTravel->*fee, baggageProvision, baggageTravel, ffInfoList);
  }

  template <typename T, T(BaggageTravel::*fee)>
  void collectFFInfosFromVector(const BaggageProvisionType& baggageProvision,
                                const std::vector<const BaggageTravel*>& baggageTravels,
                                BaggageResponseFormatter::FFInfoList& ffInfoList) const
  {
    for (const BaggageTravel* baggageTravel : baggageTravels)
      for (const OCFees* ocFees : baggageTravel->*fee)
        collectFFInfo(ocFees, baggageProvision, baggageTravel, ffInfoList);
  }

  static const std::string XML_NAMESPACE_TEXT;
  static const unsigned int WINDOW_WIDTH = 63;
};

} // namespace tse

