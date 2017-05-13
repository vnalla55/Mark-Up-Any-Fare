//----------------------------------------------------------------------------
//  File:           PfcDisplayDb.h
//  Authors:        Piotr Lach
//  Created:        4/17/2008
//  Description:    PfcDisplayDb header file for ATSE V2 PFC Display Project.
//                  DB Adapter for PFC Display functionality.
//
//  Copyright Sabre 2008
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
#ifndef PFC_DISPLAY_DB_H
#define PFC_DISPLAY_DB_H

#include "DataModel/TaxTrx.h"
#include "Common/DateTime.h"

#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/Customer.h"
#include "DBAccess/PfcPFC.h"
#include "DBAccess/PfcMultiAirport.h"
#include "DBAccess/PfcEssAirSvc.h"
#include "DBAccess/PfcAbsorb.h"
#include "DBAccess/PfcEquipTypeExempt.h"
#include "DBAccess/PfcCollectMeth.h"
#include "DBAccess/Nation.h"

#include <string>
#include <vector>
#include <functional>
#include <algorithm>

namespace tse
{

class PfcDisplayDb
{
public:
  PfcDisplayDb(TaxTrx* trx);
  virtual ~PfcDisplayDb();

  bool isValidDate() const;
  bool isSellingDate() const;
  DateTime oldestAllowedDate() const;

  virtual const Customer* getCustomer(const PseudoCityCode& key) const;
  virtual const Loc* getLoc(const LocCode& locCode) const;
  virtual const Nation* getNation(const NationCode& nationCode) const;
  virtual const std::vector<TaxCodeReg*>& getTaxCode(const TaxCode& taxCode) const;

  virtual const std::vector<PfcPFC*>& getPfcPFC(const LocCode& key) const;
  virtual const std::vector<PfcPFC*>& getAllPfcPFC() const;
  virtual const std::vector<PfcPFC*>& getDateIndependentPfcPFC(const LocCode& key) const;

  virtual const std::vector<PfcEssAirSvc*>& getPfcEssAirSvc(const LocCode& easHubArpt) const;
  virtual const std::vector<PfcEssAirSvc*>& getAllPfcEssAirSvc() const;

  virtual const PfcMultiAirport* getPfcMultiAirport(const LocCode& key) const;
  virtual const std::vector<PfcMultiAirport*>& getAllPfcMultiAirport() const;

  virtual const std::vector<PfcAbsorb*>&
  getPfcAbsorb(const LocCode& pfcAirport, const CarrierCode& localCarrier) const;
  virtual const std::vector<PfcAbsorb*>& getAllPfcAbsorb() const;

  virtual const std::vector<PfcEquipTypeExempt*>& getAllPfcEquipTypeExempt() const;

  virtual const std::vector<PfcCollectMeth*>& getAllPfcCollectMethData() const;
  virtual const std::vector<PfcCollectMeth*>&
  getPfcCollectMethData(const CarrierCode& carrier) const;

  void setDate(DateTime date) { _date = date; }

  const TaxTrx* trx() const { return _trx; }
  TaxTrx* trx() { return _trx; }

  const DateTime& date() const { return _date; }
  const DateTime& localDate() const { return _localDate; }

  // ----------------------------------------------------------------------------
  // <PRE>
  //
  // @function Greater
  //
  // Description:  Operator () returns true if lhs is bigger than rls.
  //
  // </PRE>
  // ----------------------------------------------------------------------------

  typedef int NullType;

  template <typename T,
            typename R1,
            typename R2 = NullType,
            typename R3 = NullType,
            typename R4 = NullType>
  class Greater : public std::binary_function<T*, T*, bool>
  {
    R1& (T::*_f1)(void);
    R2& (T::*_f2)(void);
    R3& (T::*_f3)(void);
    R4& (T::*_f4)(void);

  public:
    Greater(R1& (T::*f1)(void)) : _f1(f1), _f2(nullptr), _f3(nullptr), _f4(nullptr) {}
    Greater(R1& (T::*f1)(void), R2& (T::*f2)(void)) : _f1(f1), _f2(f2), _f3(nullptr), _f4(nullptr) {}
    Greater(R1& (T::*f1)(void), R2& (T::*f2)(void), R3& (T::*f3)(void))
      : _f1(f1), _f2(f2), _f3(f3), _f4(nullptr)
    {
    }
    Greater(R1& (T::*f1)(void), R2& (T::*f2)(void), R3& (T::*f3)(void), R4& (T::*f4)(void))
      : _f1(f1), _f2(f2), _f3(f3), _f4(f4)
    {
    }

    bool operator()(T* lhs, T* rhs) const
    {
      if ((lhs->*_f1)() != (rhs->*_f1)())
      {
        return (lhs->*_f1)() > (rhs->*_f1)();
      }
      else if (_f2)
      {
        if ((lhs->*_f2)() != (rhs->*_f2)())
          return (lhs->*_f2)() > (rhs->*_f2)();
      }
      else if (_f3)
      {
        if ((lhs->*_f3)() != (rhs->*_f3)())
          return (lhs->*_f3)() > (rhs->*_f3)();
      }
      else if (_f4)
      {
        return (lhs->*_f4)() > (rhs->*_f4)();
      }

      return true;
    }
  };

  // ----------------------------------------------------------------------------
  // <PRE>
  //
  // @function Greater
  //
  // Description:  Operator () returns true if lhs is bigger than rls.
  //
  // </PRE>
  // ----------------------------------------------------------------------------
  class PfcMultiArptGreater : public std::binary_function<PfcMultiAirport*, PfcMultiAirport*, bool>
  {
  public:
    PfcMultiArptGreater() {}
    bool operator()(PfcMultiAirport* lhs, PfcMultiAirport* rhs) const
    {
      return lhs->loc().loc() > rhs->loc().loc();
    }
  };

  // ----------------------------------------------------------------------------
  // <PRE>
  //
  // @function PfcPFCDateIndependent_IsNotValidLoc
  //
  // Description:  Operator () returns true if loc ( and carrier) is found in T struct.
  //
  // </PRE>
  // ----------------------------------------------------------------------------

  template <typename T>
  class IsValidLocCarrier : public std::unary_function<T*, bool>
  {
    LocCode _loc;
    CarrierCode _carrier;
    LocCode& (T::*_f1)(void);
    CarrierCode& (T::*_f2)(void);

  public:
    IsValidLocCarrier(const LocCode& loc, LocCode& (T::*f)(void)) : _loc(loc), _f1(f), _f2(nullptr) {}

    IsValidLocCarrier(const LocCode& loc,
                      const CarrierCode& carrier,
                      LocCode& (T::*f1)(void),
                      CarrierCode& (T::*f2)(void))
      : _loc(loc), _carrier(carrier), _f1(f1), _f2(f2)
    {
    }

    bool operator()(T* rec) const
    {
      if (!_f2)
      {
        return ((rec->*_f1)() == _loc);
      }
      else
      {
        return ((rec->*_f1)() == _loc && (rec->*_f2)() == _carrier);
      }
    }
  };

  // ----------------------------------------------------------------------------
  // <PRE>
  //
  // @function  IsNotValidDate
  //
  // Description:  Date filter for T struct.
  //               Returns true if data exists for requested date.
  //
  // </PRE>
  // ----------------------------------------------------------------------------
  template <typename T>
  class IsValidDate : public std::unary_function<T*, bool>
  {
    DateTime _date;
    bool _isSellingDate;

  public:
    explicit IsValidDate(const DateTime& date, bool isSellingDate)
      : _date(date), _isSellingDate(isSellingDate)
    {
    }

    bool operator()(const T* rec) const
    {
      return (
          (_isSellingDate && rec->expireDate() >= _date && rec->effDate() < rec->expireDate()) ||
          (!_isSellingDate && rec->effDate() <= _date && rec->expireDate() >= _date));
    }
  };

private:
  TaxTrx* _trx;
  DateTime _date;
  DateTime _localDate;
  bool _sellingDate;

  mutable std::vector<PfcPFC*>* _lazyAcquiredPfcPFC;
  mutable std::vector<PfcPFC*>* _lazyAcquiredDateIndependentPfcPFC;
  mutable std::vector<PfcAbsorb*>* _lazyAcquiredPfcAbsorb;
  mutable std::vector<PfcEssAirSvc*>* _lazyAcquiredPfcEssAirSvc;
  mutable std::vector<PfcMultiAirport*>* _lazyAcquiredPfcMultiAirport;
  mutable std::vector<PfcEquipTypeExempt*>* _lazyAcquiredPfcEquipExempt;
  mutable std::vector<PfcCollectMeth*>* _lazyAcquiredPfcCollectMethData;
};

} // namespace tse
#endif
