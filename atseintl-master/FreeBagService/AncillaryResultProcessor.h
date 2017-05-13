//-------------------------------------------------------------------
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
//-------------------------------------------------------------------
#pragma once

#include "Common/FallbackUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/Itin.h"
#include "ServiceFees/OCFees.h"

#include <vector>

namespace tse
{
FIXEDFALLBACK_DECL(AB240_DecoupleServiceFeesAndFreeBag);

class AncillaryPricingTrx;
class BaggageTravel;
class PricingTrx;
class ServiceFeesGroup;
class SubCodeInfo;

class AncillaryResultProcessor
{
  friend class AncillaryResultProcessorTest;

  void processAllowanceAndCharges(std::vector<ServiceFeesGroup*>& ocFeesGroup,
                      const BaggageTravel* baggageTravel,
                      uint32_t baggageTravelIndex);
  void processEmbargo(std::vector<ServiceFeesGroup*>& ocFeesGroup,
                      const BaggageTravel* baggageTravel,
                      uint32_t baggageTravelIndex);
  void processCarryOn(std::vector<ServiceFeesGroup*>& ocFeesGroup,
                      const BaggageTravel* baggageTravel,
                      uint32_t baggageTravelIndex);

  template <typename Iterator, typename ProcessBaggageTravelPredicate>
  void process(Iterator begin, Iterator end,
               const ProcessBaggageTravelPredicate& processBaggageTravelPredicate)
  {
    LOG4CXX_DEBUG(_logger, "AncillaryResultProcessor::process() - entering");
    uint32_t baggageTravelIndex = 0;
    for(Iterator it = begin; it != end; ++it)
    {
      ++baggageTravelIndex;
      const BaggageTravel * baggageTravel = *it;
      if (isPartOfBaggageRoute(baggageTravel))
      {
        Itin* itin = baggageTravel->itin();
        std::vector<ServiceFeesGroup*>& ocFeesGroup =
            fallback::fixed::AB240_DecoupleServiceFeesAndFreeBag() ?
                itin->ocFeesGroup(): itin->ocFeesGroupsFreeBag();
        processBaggageTravelPredicate(ocFeesGroup, baggageTravel, baggageTravelIndex);
      }
    }
    if (begin != end)
      addEmptyGroupIfNoDataReturned((*begin)->itin());
    LOG4CXX_DEBUG(_logger, "AncillaryResultProcessor::process() - leaving");
}

public:
  AncillaryResultProcessor(AncillaryPricingTrx& trx);
  ~AncillaryResultProcessor();

  void processAllowanceAndCharges(const std::vector<BaggageTravel*>& baggageTravels);
  void processEmbargoes(const std::vector<const BaggageTravel*>& baggageTravels);
  void processCarryOn(const std::vector<const BaggageTravel*>& baggageTravels);
protected:
  struct OCFeesInitializer : std::unary_function<const OCFees*, bool>
  {
    OCFeesInitializer(PricingTrx& trx,
                      const BaggageTravel& baggageTravel,
                      OCFees& ocFees,
                      uint32_t baggageTravelIndex)
      : _trx(trx),
        _baggageTravel(baggageTravel),
        _ocFees(ocFees),
        _baggageTravelIndex(baggageTravelIndex)
    {
    }

    bool operator()(const OCFees* ocFees) const;

    void addOCFees(ServiceFeesGroup* group) const;
    void addOCFeesSeg(OCFees* ocFees) const;
    void init(ServiceFeesGroup* group) const;
    void add196(OCFees::OCFeesSeg* ocFeesSeg) const;
    void addBaggageItemProperty(OCFees::OCFeesSeg* ocFeesSeg, const std::string& txtMsg) const;
    const SubCodeInfo* selectS5Record(OCFees::OCFeesSeg* ocFeesSeg,
                                      const ServiceSubTypeCode& serviceSubTypeCode) const;

    void retrieveS5Records(const VendorCode&, const CarrierCode&, std::vector<SubCodeInfo*>&) const;

  private:
    PricingTrx& _trx;
    const BaggageTravel& _baggageTravel;
    OCFees& _ocFees;
    uint32_t _baggageTravelIndex;
  };

  ServiceFeesGroup*
  getGroup(std::vector<ServiceFeesGroup*>& ocFeesGroup, const SubCodeInfo* subCode);
  ServiceFeesGroup* createGroup(const ServiceGroup& groupCode);
  void buildOcFees(std::vector<ServiceFeesGroup*>& ocFeesGroup,
                   OCFees* ocFees,
                   const BaggageTravel* baggageTravel,
                   uint32_t baggageTravelIndex);
  void addEmptyGroupIfNoDataReturned(Itin* itin);
  bool isPartOfBaggageRoute(const BaggageTravel* baggageTravel) const;
  bool serviceTypeNeeded(const SubCodeInfo* s5, const Indicator& ancillaryServiceType) const;
  const SubCodeInfo* getS5(const BaggageTravel* baggageTravel) const;

  AncillaryPricingTrx& _trx;
  static log4cxx::LoggerPtr _logger;
};

} // tse
