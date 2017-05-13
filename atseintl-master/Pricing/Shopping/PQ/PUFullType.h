// -------------------------------------------------------------------
//
//  Copyright (C) Sabre 2011
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
// -------------------------------------------------------------------

#pragma once

#include "Common/FallbackUtil.h"
#include "DataModel/PricingUnit.h"

#include <memory>

namespace tse
{
class FareMarketPath;
class PU;
class PUPath;
}

namespace tse
{
namespace shpq
{

class ConxRoutePQItem;
class SoloPUPathCollector;

class PUTypeDetails
{
public:
  PUTypeDetails(const char* const idCStr = "",
                PricingUnit::Type type = PricingUnit::Type::UNKNOWN,
                PricingUnit::PUSubType subtype = PricingUnit::UNKNOWN_SUBTYPE);

  PricingUnit::Type getType() const { return _type; }
  PricingUnit::PUSubType getSubType() const { return _subtype; }
  const char* getPUIdStr() const { return _idCStr; }
  const char* getSubTypeStr() const { return _subtypeStr; }

private:
  const char* _idCStr;
  const char* _subtypeStr;
  PricingUnit::Type _type;
  PricingUnit::PUSubType _subtype;
};

class PUFullType
{
public:
  PricingUnit::Type getType() const { return _puTypeDetails.getType(); }
  PricingUnit::PUSubType getSubType() const { return _puTypeDetails.getSubType(); }
  const char* getPUIdStr() const { return _puTypeDetails.getPUIdStr(); }
  const char* getSubTypeStr() const { return _puTypeDetails.getSubTypeStr(); }

  virtual bool addPU(SoloPUPathCollector&, FareMarketPath*, PUPath*) const = 0;
  /**
   * @param pqItem is expected to be != 0
   */
  virtual bool isItemValid(const ConxRoutePQItem* const pqItem) const = 0;

  const std::string& getFMOrder() const { return _fmOrderStr; }

protected:
  explicit PUFullType(const PUTypeDetails& details) : _puTypeDetails(details), _fmOrderStr("") {}

  virtual ~PUFullType() {}

  void setFMOrderStr(int);
  void setFMOrderStr(int, int);

private:
  PUTypeDetails _puTypeDetails;
  std::string _fmOrderStr;
};

typedef std::shared_ptr<PUFullType> PUFullTypePtr;

class OWPUType : public PUFullType
{
public:
  static PUFullTypePtr create(uint16_t);

  virtual bool addPU(SoloPUPathCollector&, FareMarketPath*, PUPath*) const override;
  virtual bool isItemValid(const ConxRoutePQItem* const) const override { return true; }

private:
  OWPUType(uint16_t fmIndex)
    : PUFullType(PUTypeDetails("OW", PricingUnit::Type::ONEWAY)), _fmIndex(fmIndex)
  {
    setFMOrderStr(fmIndex + 1);
  }

private:
  uint16_t _fmIndex;
};

class RTPUType : public PUFullType
{
public:
  static PUFullTypePtr create(uint16_t, uint16_t, bool checkConnectingCities);

  virtual bool addPU(SoloPUPathCollector&, FareMarketPath*, PUPath*) const override;
  virtual bool isItemValid(const ConxRoutePQItem* const) const override;

private:
  RTPUType(uint16_t outboundIndex, uint16_t inboundIndex, bool checkConnectingCities)
    : PUFullType(PUTypeDetails("RT", PricingUnit::Type::ROUNDTRIP)),
      _outboundIndex(outboundIndex),
      _inboundIndex(inboundIndex),
      _checkConnectingCities(checkConnectingCities)
  {
    setFMOrderStr(_outboundIndex + 1, _inboundIndex + 1);
  }

private:
  uint16_t _outboundIndex;
  uint16_t _inboundIndex;
  bool _checkConnectingCities;
};

class OJPUType : public PUFullType
{
public:
  static PUFullTypePtr create(const PUTypeDetails&, uint16_t, uint16_t);

  virtual bool addPU(SoloPUPathCollector&, FareMarketPath*, PUPath*) const override;
  virtual bool isItemValid(const ConxRoutePQItem* const) const override;

private:
  OJPUType(const PUTypeDetails& details, uint16_t outboundIndex, uint16_t inboundIndex)
    : PUFullType(details), _outboundIndex(outboundIndex), _inboundIndex(inboundIndex)
  {
    setFMOrderStr(_outboundIndex + 1, _inboundIndex + 1);
  }

private:
  uint16_t _outboundIndex;
  uint16_t _inboundIndex;
};

class CTPUType : public PUFullType
{
public:
  static PUFullTypePtr create();

  virtual bool addPU(SoloPUPathCollector&, FareMarketPath*, PUPath*) const override;
  virtual bool isItemValid(const ConxRoutePQItem* const) const override;

private:
  CTPUType() : PUFullType(PUTypeDetails("CT", PricingUnit::Type::CIRCLETRIP)) {}
};
}
} // namespace tse::shpq

