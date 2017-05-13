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

#include "Rules/TaxPointFinder.h"

namespace tax
{

namespace
{
bool
isInRange(type::Index i, std::pair<type::Index, type::Index> range)
{
  if (range.first < range.second)
    return ((i >= range.first) && (i <= range.second));
  else
    return ((i >= range.second) && (i <= range.first));
}
}

StopoverValidator::StopoverValidator(
    const TaxPointUtils& utils,
    const std::vector<std::pair<type::Index, type::Index> >& ranges,
    bool /* fareBreakMustAlsoBeStopover */)
  : _utils(utils),
    _ranges(ranges),
    _validated(ranges.size(), false),
    _validatedCount(0),
    _result(ranges.size(), nullptr)
{
}

void
StopoverValidator::operator()(const Geo& geo, const Geo&)
{
  const type::Index i = geo.id();
  const type::Index toValidate = _result.size();
  for (type::Index id = 0; id < toValidate && _validatedCount < toValidate; ++id)
  {
    if (_validated[id])
      continue;

    if (!isInRange(i, _ranges[id]) || _utils.isOpen(i, _ranges[id]))
    {
      _validated[id] = true;
      ++_validatedCount;
      continue;
    }

    if (_utils.isStopover(i))
    {
      _validated[id] = true;
      ++_validatedCount;
      _result[id] = &geo;
    }
  }
}

ConnectionValidator::ConnectionValidator(
    const TaxPointUtils& utils,
    const std::vector<std::pair<type::Index, type::Index> >& ranges,
    bool /* fareBreakMustAlsoBeStopover */)
  : _utils(utils),
    _ranges(ranges),
    _validated(ranges.size(), false),
    _validatedCount(0),
    _result(ranges.size(), nullptr)
{
}

void
ConnectionValidator::operator()(const Geo& geo, const Geo&)
{
  const type::Index i = geo.id();
  const type::Index toValidate = _result.size();
  for (type::Index id = 0; id < toValidate && _validatedCount < toValidate; ++id)
  {
    if (_validated[id])
      continue;

    if (!isInRange(i, _ranges[id]) || _utils.isOpen(i, _ranges[id]))
    {
      _validated[id] = true;
      ++_validatedCount;
      continue;
    }

    if (!_utils.isStopover(i))
    {
      _validated[id] = true;
      ++_validatedCount;
      _result[id] = &geo;
    }
  }
}

FurthestValidator::FurthestValidator(
    const TaxPointUtils&,
    const std::vector<std::pair<type::Index, type::Index> >& ranges,
    bool /* fareBreakMustAlsoBeStopover */)
  : _ranges(ranges), _result(ranges.size(), nullptr), _allValidated(false)
{
}

void
FurthestValidator::operator()(const Geo&, const Geo& furthestGeo)
{
  const type::Index i = furthestGeo.id();
  const type::Index toValidate = _result.size();
  for (type::Index id = 0; id < toValidate; ++id)
  {
    if (isInRange(i, _ranges[id]))
    {
      _result[id] = &furthestGeo;
    }
  }
  _allValidated = true;
}

FareBreakValidator::FareBreakValidator(
    const TaxPointUtils& utils,
    const std::vector<std::pair<type::Index, type::Index> >& ranges,
    bool fareBreakMustAlsoBeStopover)
  : _utils(utils),
    _ranges(ranges),
    _validated(ranges.size(), false),
    _validatedCount(0),
    _result(ranges.size(), nullptr),
    _fareBreakMustAlsoBeStopover(fareBreakMustAlsoBeStopover)
{
}

void
FareBreakValidator::operator()(const Geo& geo, const Geo&)
{
  const type::Index i = geo.id();

  const type::Index toValidate = _result.size();
  for (type::Index id = 0; id < toValidate && _validatedCount < toValidate; ++id)
  {
    if (_validated[id])
      continue;

    if (!isInRange(i, _ranges[id]))
    {
      _validated[id] = true;
      ++_validatedCount;
      continue;
    }

    if (_utils.isFareBreak(i, _ranges[id]) &&
        (!_fareBreakMustAlsoBeStopover ||
         _utils.isOpen(i, _ranges[id]) ||
         _utils.isStopover(i)))
    {
      _validated[id] = true;
      ++_validatedCount;
      _result[id] = &geo;
    }
  }
}

FeeBreakValidator::FeeBreakValidator(
    const TaxPointUtils& utils,
    const std::vector<std::pair<type::Index, type::Index> >& ranges,
    bool fareBreakMustAlsoBeStopover)
  : _utils(utils),
    _ranges(ranges),
    _validated(ranges.size(), false),
    _validatedCount(0),
    _result(ranges.size(), nullptr),
    _fareBreakMustAlsoBeStopover(fareBreakMustAlsoBeStopover)
{
}

void
FeeBreakValidator::operator()(const Geo& geo, const Geo&)
{
  const type::Index i = geo.id();

  const type::Index toValidate = _result.size();
  for (type::Index id = 0; id < toValidate && _validatedCount < toValidate; ++id)
  {
    if (_validated[id])
      continue;

    if (!isInRange(i, _ranges[id]))
    {
      _validated[id] = true;
      ++_validatedCount;
      continue;
    }

    if (_utils.isFeeBreak(i, _ranges[id]) &&
        (!_fareBreakMustAlsoBeStopover ||
         _utils.isOpen(i, _ranges[id]) ||
         _utils.isStopover(i)))
    {
      _validated[id] = true;
      ++_validatedCount;
      _result[id] = &geo;
    }
  }
}

TaxPointFinder::TaxPointFinder(const type::Index beginId,
                               const type::Index furthestIndex,
                               const TaxPointsProperties& properties,
                               const bool matchTicketedOnly,
                               const int direction,
                               const std::vector<Geo>& geos)
  : _beginId(beginId),
    _furthestIndex(furthestIndex),
    _properties(properties),
    _matchTicketedOnly(matchTicketedOnly),
    _direction(direction),
    _geos(geos)
{
}

std::vector<const Geo*>
TaxPointFinder::find(type::Loc2StopoverTag loc2StopoverTag,
                     bool fareBreakMustAlsoBeStopover,
                     const std::vector<std::pair<type::Index, type::Index> >& ranges,
                     bool matchFeeBreaks /* = true */) const
{
  switch (loc2StopoverTag)
  {
  case type::Loc2StopoverTag::Stopover:
    return find<StopoverValidator>(ranges);
  case type::Loc2StopoverTag::Furthest:
    return find<FurthestValidator>(ranges);
  case type::Loc2StopoverTag::FareBreak:
    return (matchFeeBreaks)
        ? find<FeeBreakValidator>(ranges, fareBreakMustAlsoBeStopover) :
          find<FareBreakValidator>(ranges, fareBreakMustAlsoBeStopover);
  default:
    return std::vector<const Geo*>(ranges.size(), nullptr);
  }
}

}
