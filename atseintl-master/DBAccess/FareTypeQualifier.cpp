#include "DBAccess/FareTypeQualifier.h"

namespace tse
{
FareTypeQualifier::FareTypeQualifier(const FareTypeQualifier& rhs)
  : _userApplType(rhs._userApplType),
    _userAppl(rhs._userAppl),
    _fareTypeQualifier(rhs._fareTypeQualifier),
    _journeyTypeDom(rhs._journeyTypeDom),
    _journeyTypeIntl(rhs._journeyTypeIntl),
    _journeyTypeEoe(rhs._journeyTypeEoe),
    _pricingUnitDom(rhs._pricingUnitDom),
    _pricingUnitIntl(rhs._pricingUnitIntl),
    _createDate(rhs._createDate),
    _expireDate(rhs._expireDate),
    _lockDate(rhs._lockDate),
    _effDate(rhs._effDate),
    _discDate(rhs._discDate),
    _memoNo(rhs._memoNo),
    _creatorId(rhs._creatorId),
    _creatorBusinessUnit(rhs._creatorBusinessUnit)
{
}

bool
FareTypeQualifier::
operator!=(const FareTypeQualifier& rhs) const
{
  if (this == &rhs)
    return true;

  return (_userApplType != rhs._userApplType || _userAppl != rhs._userAppl ||
          _fareTypeQualifier != rhs._fareTypeQualifier || _journeyTypeDom != rhs._journeyTypeDom ||
          _journeyTypeIntl != rhs._journeyTypeIntl || _journeyTypeEoe != rhs._journeyTypeEoe ||
          _pricingUnitDom != rhs._pricingUnitDom || _pricingUnitIntl != rhs._pricingUnitIntl ||
          _createDate != rhs._createDate);
}
}
