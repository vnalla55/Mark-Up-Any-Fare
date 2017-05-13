#pragma once

#include "Common/TseCodeTypes.h"

#include <boost/container/flat_map.hpp>
#include <vector>

namespace tse
{
class BaggageCharge;
class BaggageTravel;
class PaxType;
class OCFees;
class SubCodeInfo;

namespace PrecalcBaggage
{
using namespace boost::container;

struct CxrPair : public std::pair<CarrierCode, CarrierCode>
{
  using Base = std::pair<CarrierCode, CarrierCode>;
  using Base::Base;

  CarrierCode allowanceCxr() const { return first; }
  CarrierCode deferCxr() const { return second; }
};

struct AllowanceRecords
{
  bool s5Found = false;
  std::vector<OCFees*> s7s;
};

struct ChargeRecords
{
  flat_map<const SubCodeInfo*, std::vector<BaggageCharge*>> s7s;
};

struct BagTravelData
{
  BaggageTravel* bagTravel = nullptr;
  flat_map<CxrPair, AllowanceRecords> allowance;
  flat_map<CarrierCode, ChargeRecords> charges;
};

struct PaxData
{
  std::vector<BagTravelData> bagTravelData;
};

struct ItinData
{
  flat_map<const PaxType*, PaxData> paxData;
};

} // ns PrecalcBaggage
} // tse namespace
