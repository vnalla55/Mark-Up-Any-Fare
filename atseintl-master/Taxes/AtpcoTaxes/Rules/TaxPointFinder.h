// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include "Rules/TaxPointUtils.h"

namespace tax
{
class StopoverValidator
{
public:
  StopoverValidator(const TaxPointUtils& utils,
                    const std::vector<std::pair<type::Index, type::Index> >& ranges,
                    bool /* fareBreakMustAlsoBeStopover */);
  void operator()(const Geo& geo, const Geo&);
  bool allValidated() const { return _validatedCount == _validated.size(); }
  const std::vector<const Geo*>& getResult() const { return _result; }

private:
  const TaxPointUtils& _utils;
  const std::vector<std::pair<type::Index, type::Index> >& _ranges;
  std::vector<bool> _validated;
  type::Index _validatedCount;
  std::vector<const Geo*> _result;
};

class ConnectionValidator
{
public:
  ConnectionValidator(const TaxPointUtils& utils,
                      const std::vector<std::pair<type::Index, type::Index> >& ranges,
                      bool /* fareBreakMustAlsoBeStopover */);
  void operator()(const Geo& geo, const Geo&);
  bool allValidated() const { return _validatedCount == _validated.size(); }
  const std::vector<const Geo*>& getResult() const { return _result; }

private:
  const TaxPointUtils& _utils;
  const std::vector<std::pair<type::Index, type::Index> >& _ranges;
  std::vector<bool> _validated;
  type::Index _validatedCount;
  std::vector<const Geo*> _result;
};

class FurthestValidator
{
public:
  FurthestValidator(const TaxPointUtils&,
                    const std::vector<std::pair<type::Index, type::Index> >& ranges,
                    bool /* fareBreakMustAlsoBeStopover */);
  void operator()(const Geo&, const Geo& furthestGeo);
  bool allValidated() const { return _allValidated; }
  const std::vector<const Geo*>& getResult() const { return _result; }

private:
  const std::vector<std::pair<type::Index, type::Index> >& _ranges;
  std::vector<const Geo*> _result;
  bool _allValidated;
};

class FareBreakValidator
{
public:
  FareBreakValidator(const TaxPointUtils& utils,
                     const std::vector<std::pair<type::Index, type::Index> >& ranges,
                     bool fareBreakMustAlsoBeStopover);
  void operator()(const Geo& geo, const Geo&);
  bool allValidated() const { return _validatedCount == _validated.size(); }
  const std::vector<const Geo*>& getResult() const { return _result; }

private:
  const TaxPointUtils& _utils;
  const std::vector<std::pair<type::Index, type::Index> >& _ranges;
  std::vector<bool> _validated;
  type::Index _validatedCount;
  std::vector<const Geo*> _result;
  bool _fareBreakMustAlsoBeStopover;
};

class FeeBreakValidator
{
public:
  FeeBreakValidator(const TaxPointUtils& utils,
                    const std::vector<std::pair<type::Index, type::Index> >& ranges,
                    bool fareBreakMustAlsoBeStopover);
  void operator()(const Geo& geo, const Geo&);
  bool allValidated() const { return _validatedCount == _validated.size(); }
  const std::vector<const Geo*>& getResult() const { return _result; }

private:
  const TaxPointUtils& _utils;
  const std::vector<std::pair<type::Index, type::Index> >& _ranges;
  std::vector<bool> _validated;
  type::Index _validatedCount;
  std::vector<const Geo*> _result;
  bool _fareBreakMustAlsoBeStopover;
};

class TaxPointFinder
{
public:
  TaxPointFinder(const type::Index beginId,
                 const type::Index furthestIndex,
                 const TaxPointsProperties& properties,
                 const bool matchTicketedOnly,
                 const int32_t direction,
                 const std::vector<Geo>& geos);

  std::vector<const Geo*> find(type::Loc2StopoverTag loc2StopoverTag,
                               bool fareBreakMustAlsoBeStopover,
                               const std::vector<std::pair<type::Index, type::Index> >& ranges,
                               bool matchFeeBreaks = true) const;

  template <class Validator>
  std::vector<const Geo*>
  find(const std::vector<std::pair<type::Index, type::Index> >& ranges,
       bool fareBreakMustAlsoBeStopover = false) const
  {
    const Geo& furthestGeo = _geos[_furthestIndex];
    int32_t step = 2 * _direction;
    TaxPointUtils utils(_properties);
    Validator validator(utils, ranges, fareBreakMustAlsoBeStopover);
    for (type::Index i = _beginId + _direction; i < _geos.size() && !validator.allValidated();
         i += step)
    {
      const Geo& geo = _geos[i];
      if (_matchTicketedOnly && geo.unticketedTransfer() == type::UnticketedTransfer::Yes)
        continue;

      validator(geo, furthestGeo);
    }
    return validator.getResult();
  }

private:
  const type::Index _beginId;
  const type::Index _furthestIndex;
  const TaxPointsProperties& _properties;
  const bool _matchTicketedOnly;
  const int32_t _direction;
  const std::vector<Geo>& _geos;
};
}

