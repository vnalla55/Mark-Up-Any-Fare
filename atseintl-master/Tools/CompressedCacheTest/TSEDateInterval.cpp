//-------------------------------------------------------------------
// 2005, Sabre Inc.  All rights reserved.
//
// This software/documentation is the confidential and proprietary
// product of Sabre Inc. Any unauthorized use, reproduction, or
// transfer of this software/documentation, in any medium, or
// incorporation of this software/documentation into any system
// or publication, is strictly prohibited
//-------------------------------------------------------------------

// legend:
//
//             |-----------|=================|-----------|
//          createDate  effDate          expireDate   discDate
//
//
// createDate  source: Sabre Content. date and time when the record was
//             placed to DB. has date and time portion.
//
// effDate     source: vendor. first day when DB record is applicable.
//             has date and no time portion. the record is applicable
//             starting from 00:00:00 of effDate.
//
// discDate    source: vendor. last day when DB record is applicable. 
//             has date and no time portion. the record is applicable
//             up to 23:59:59 of discDate.
//
// expireDate  source: Sabre Content. date and time fare when DB record
//             is no longer applicable. has date and time portion.
//
// notes:
//
// 1) When any DB record appears in DB for the very first time it 
//    normally has createDate < effDate and expireDate == discDate
//
//    It is possible however that sometimes createDate > effDate
//    (example: ATPCO creates the fare effective from today but the
//    fare delivered to DB after noon)
//
// 2) When Sabre Content expires the record it modifies expireDate;
//    discDate remains the same for the rest of the record life.
//
// 3) expireDate is always less then or equal to discDate.
//    if it is not then this is the bug on content side.
//
//    It is possible that expireDate and discDate can have the same
//    date portion. In this case direct comparison indicates that
//    expireDate > discDate. It's not a correct comparison though.
//    In real you need to compare expireDate with expression like
//    (discDate + 23:59:59)

#include "TSEDateInterval.h"

using namespace tse;

TSEDateInterval::TSEDateInterval()
{
}

TSEDateInterval::TSEDateInterval( const TSEDateInterval& rhs )
{
  rhs.cloneDateInterval( *this );
}

const TSEDateInterval&
TSEDateInterval::operator = ( const TSEDateInterval& rhs )
{
  if( &rhs != this )
  {
    rhs.cloneDateInterval( *this );
  }

  return *this;
}

void
TSEDateInterval::cloneDateInterval( TSEDateInterval& cloneObj ) const
{
  cloneObj.createDate() = _createDate;
  cloneObj.effDate()    = _effDate;
  cloneObj.expireDate() = _expireDate;
  cloneObj.discDate()   = _discDate;
}

bool
TSEDateInterval::isEffective( const DateTime travelDT ) const
{

// for regular pricing:
//
//                               ! travelDT
//             |-----------|=================|-----------|

  return    travelDT >= _effDate
         &&
            travelDT <= _expireDate
         &&
            travelDT <= _discDate;
}

bool
TSEDateInterval::isEffective( std::pair< DateTime , DateTime > travelDateRange ) const
{
  return    travelDateRange.second >= _effDate
         &&
            travelDateRange.first <= _expireDate
         &&
            travelDateRange.first <= _discDate;
}

bool
TSEDateInterval::isEffective( const DateTime ticketingDT,
                              const DateTime travelDT     ) const
{

// for historical pricing:
//                  ! ticketingDT
//                  !              ! travelDT
//             |-----------|=================|-----------|
//
// or
//                         ! ticketingDT
//                         !              ! travelDT
//             |--------|====================|-----------|

  return    (ticketingDT.date() >= _createDate.date() || ticketingDT.date() >= _effDate.date())
         &&
            ticketingDT.date() <= _expireDate.date()
         &&
            ticketingDT.date() <= _discDate.date()
         &&
            travelDT >= _effDate
         &&
            travelDT.date()<= _discDate.date();
}

bool
TSEDateInterval::defineIntersectionH( const TSEDateInterval& i1,
                                     const TSEDateInterval& i2 )
{
  //      |--|=== i1 ===|--|
  //             |--|=== i2 ===|--|
  //  or
  //      |--|============ i1 ============|--|
  //            |--|=== i2 ===|--|

  if( i2.effDate() >= i1.effDate()    &&
      i2.effDate() <= i1.discDate()
    )
  {
    _effDate = i2.effDate();
  }

  //                |--|=== i1 ===|--|
  //      |--|=== i2 ===|--|
  // or
  //           |--|=== i1 ===|--|
  //      |--|=== i2 ===|-----|

  else if( i1.effDate() >= i2.effDate()    &&
           i1.effDate() <= i2.discDate()
         )
  {
    _effDate = i1.effDate();
  }

  //                   |--|=== i1 ===|--|
  //      |--|=== i2 ===|----|
  //  or
  //      |--|=== i1 ===|----|
  //                   |--|=== i2 ===|--|
  else
    return false;

  if( i1.createDate() < i2.createDate() )
    _createDate = i2.createDate();
  else
    _createDate = i1.createDate();

  if( i1.expireDate() < i2.expireDate() )
    _expireDate = i1.expireDate();
  else
    _expireDate = i2.expireDate();

  if( i1.discDate() < i2.discDate() )
    _discDate = i1.discDate();
  else
    _discDate = i2.discDate();

  return true;
}

bool
TSEDateInterval::defineIntersection( const TSEDateInterval& i1,
                                     const TSEDateInterval& i2 )
{
  //      |--|=== i1 ===|--|
  //             |--|=== i2 ===|--|
  //  or
  //      |--|============ i1 ============|--|
  //            |--|=== i2 ===|--|

  if( i2.effDate() >= i1.effDate()    &&
      i2.effDate() <= i1.expireDate() &&
      i2.effDate() <= i1.discDate()
    )
  {
    _effDate = i2.effDate();
  }

  //                |--|=== i1 ===|--|
  //      |--|=== i2 ===|--|
  // or
  //           |--|=== i1 ===|--|
  //      |--|=== i2 ===|-----|

  else if( i1.effDate() >= i2.effDate()    &&
           i1.effDate() <= i2.expireDate() &&
           i1.effDate() <= i2.discDate()
         )
  {
    _effDate = i1.effDate();
  }

  //                   |--|=== i1 ===|--|
  //      |--|=== i2 ===|----|
  //  or
  //      |--|=== i1 ===|----|
  //                   |--|=== i2 ===|--|

  else
    return false;

  if( i1.createDate() < i2.createDate() )
    _createDate = i2.createDate();
  else
    _createDate = i1.createDate();

  if( i1.expireDate() < i2.expireDate() )
    _expireDate = i1.expireDate();
  else
    _expireDate = i2.expireDate();

  if( i1.discDate() < i2.discDate() )
    _discDate = i1.discDate();
  else
    _discDate = i2.discDate();
  
  return true;
}

bool
TSEDateInterval::defineUnion( const TSEDateInterval& i1,
                              const TSEDateInterval& i2 )
{
  //      |--|=== i1 ===|--|
  //             |--|=== i2 ===|--|
  // or
  //      |--|============ i1 ============|--|
  //            |--|=== i2 ===|--|

  if( i2.effDate() >= i1.effDate()    &&
      i2.effDate() <= i1.expireDate() &&
      i2.effDate() <= i1.discDate()
    )
  {
    _effDate = i1.effDate();
  }

  //                   |=== i1 ===|--|
  //         |=== i2 ===|
  //    or
  //            |=== i1 ===|
  //         |=== i2 ===|-----|

  else if( i1.effDate() >= i2.effDate()    &&
           i1.effDate() <= i2.expireDate() &&
           i1.effDate() <= i2.discDate()
         )
  {
    _effDate = i2.effDate();
  }

  else
    return false;

  if( i1.createDate() < i2.createDate() )
    _createDate = i2.createDate();
  else
    _createDate = i1.createDate();

  if( i1.expireDate() < i2.expireDate() )
    _expireDate = i1.expireDate();
  else
    _expireDate = i2.expireDate();

  if( i1.discDate() < i2.discDate() )
    _discDate = i1.discDate();
  else
    _discDate = i2.discDate();

  return true;
}
