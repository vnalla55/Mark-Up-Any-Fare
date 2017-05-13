// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
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

#include "FreeBagService/EmbargoesResultProcessor.h"

#include "DataModel/BaggageTravel.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Loc.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "FreeBagService/BaggageTextFormatter.h"
#include "FreeBagService/EmbargoesBaggageTextFormatter.h"
#include "ServiceFees/OCFees.h"


#include <vector>

namespace tse
{
namespace
{

struct EmbargoesCityPair
{
  LocCode _origin;
  LocCode _destination;
  CarrierCode _carrier;
};

typedef std::vector<EmbargoesCityPair> EmbargoesListPair_first;
typedef std::pair<EmbargoesListPair_first, const BaggageTravel*> EmbargoesListPair;
typedef std::vector<EmbargoesListPair> EmbargoesCompressList;

struct EmbargoesDataEqual : std::binary_function<const OCFees*, const OCFees*, bool>
{
  bool operator()(const OCFees* fstEmbargo, const OCFees* sndEmbargo) const
  {
    const OptionalServicesInfo* fstS7 = fstEmbargo->optFee();
    const OptionalServicesInfo* sndS7 = sndEmbargo->optFee();
    const SubCodeInfo* fstS5 = fstEmbargo->subCodeInfo();
    const SubCodeInfo* sndS5 = sndEmbargo->subCodeInfo();

    if (fstS7 && sndS7 && fstS5 && sndS5)
    {
      return fstS7->baggageOccurrenceFirstPc() == sndS7->baggageOccurrenceFirstPc() &&
             fstS5->description1() == sndS5->description1() &&
             fstS5->description2() == sndS5->description2() &&
             fstS5->serviceSubGroup() == sndS5->serviceSubGroup() &&
             fstS5->serviceGroup() == sndS5->serviceGroup() &&
             fstS5->commercialName() == sndS5->commercialName();
    }
    else if (!fstS7 && !sndS7 && !fstS5 && !sndS5)
      return true;
    return false;
  }
};

class EmbargoesCompressEqual : std::unary_function<const EmbargoesListPair&, bool>
{
  const BaggageTravel* _baggageTravel;

public:
  EmbargoesCompressEqual(const BaggageTravel* baggageTravel) : _baggageTravel(baggageTravel) {}
  bool operator()(const EmbargoesListPair& embargoPair) const
  {
    if (_baggageTravel->_embargoVector.size() != embargoPair.second->_embargoVector.size())
      return false;

    CarrierCode operatingCrx1 = static_cast<AirSeg*>(*_baggageTravel->_MSS)->operatingCarrierCode();
    CarrierCode operatingCrx2 =
        static_cast<AirSeg*>(*(embargoPair.second->_MSS))->operatingCarrierCode();

    if (operatingCrx1 != operatingCrx2)
      return false;

    return std::equal(_baggageTravel->_embargoVector.begin(),
                      _baggageTravel->_embargoVector.end(),
                      embargoPair.second->_embargoVector.begin(),
                      EmbargoesDataEqual());
  }
};
}

EmbargoesResultProcessor::EmbargoesResultProcessor(PricingTrx& trx) : _trx(trx) {}

EmbargoesResultProcessor::~EmbargoesResultProcessor() {}

void
EmbargoesResultProcessor::process(const std::vector<FarePath*>& farePaths)
{
  std::string embargoesTest;
  if (!farePaths.empty())
    embargoesTest = formatEmbargoesText(farePaths.front()->baggageTravelsPerSector());

  for (FarePath* farePath : farePaths)
    farePath->baggageEmbargoesResponse() += embargoesTest;
}

std::string
EmbargoesResultProcessor::formatEmbargoesText(
    const std::vector<const BaggageTravel*>& baggageTravels) const
{
  if (baggageTravels.empty())
    return EMPTY_STRING();

  EmbargoesBaggageTextFormatter embargoesTextFormatter(_trx);

  EmbargoesCompressList embargoesCompressList;

  for (const BaggageTravel* baggageTravel : baggageTravels)
  {
    if (!baggageTravel->_embargoVector.empty() && baggageTravel->shouldAttachToDisclosure())
    {
      EmbargoesCompressList::iterator it = find_if(embargoesCompressList.begin(),
                                                   embargoesCompressList.end(),
                                                   EmbargoesCompressEqual(baggageTravel));

      EmbargoesCityPair embargoesCityPair;
      embargoesCityPair._origin = (*baggageTravel->getTravelSegBegin())->origin()->loc();
      embargoesCityPair._destination = (*baggageTravel->getTravelSegBegin())->destination()->loc();
      embargoesCityPair._carrier = embargoesTextFormatter.getOperatingCarrierText(baggageTravel);

      if (it != embargoesCompressList.end())
      {
        EmbargoesListPair_first& cityPairs = it->first;
        cityPairs.push_back(embargoesCityPair);
      }
      else
      {
        EmbargoesListPair_first cityPairs;
        cityPairs.push_back(embargoesCityPair);
        embargoesCompressList.push_back(make_pair(cityPairs, baggageTravel));
      }
    }
  }

  if (embargoesCompressList.empty())
    return EMPTY_STRING();

  std::string embargoesText = "  \nEMBARGOES-APPLY TO EACH PASSENGER\n";
  // Format response
  for (const EmbargoesCompressList::value_type val : embargoesCompressList)
  {
    uint32_t index = 0;
    CarrierCode carrier;
    const EmbargoesListPair_first& cityPairs = val.first;

    for (const EmbargoesCityPair& cp : cityPairs)
    {
      index++;
      carrier = cp._carrier;
      embargoesText += cp._origin + cp._destination;

      if (index != cityPairs.size())
        embargoesText += " ";
    }
    embargoesText += BaggageTextFormatter::DASH + carrier + BaggageTextFormatter::NEW_LINE;

    for (OCFees* embargo : val.second->_embargoVector)
      embargoesText += embargoesTextFormatter.formatEmbargoesText(embargo);
  }
  return embargoesText;
}
}
