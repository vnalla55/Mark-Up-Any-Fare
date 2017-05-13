//-------------------------------------------------------------------
//  Copyright Sabre 2004
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

#include "FareInfo.h"

//-------------------------------------------------------------------
// ** NOTICE **
//
// This class has a clone method, if you add member variables
// please make sure you update the clone method as well!
//-------------------------------------------------------------------

namespace tse {
class DataHandle;

class SITAFareInfo : public FareInfo
{
public:

  SITAFareInfo();
  virtual ~SITAFareInfo()
  {
  }


  /**
   * This methods populates a given FareInfo to be
   * 'equal' to the current object
   *
   * @param FareInfo - object to populate
   */
  void clone( FareInfo& cloneObj ) const;

  const bool isSITA() const { return true; };

  // accessors
  // =========

  RouteCodeC& routeCode() { return _routeCode;}
  const RouteCodeC& routeCode() const { return _routeCode; }

  DBEClass& dbeClass() { return _dbeClass;}
  const DBEClass& dbeClass() const { return _dbeClass; }

  Indicator& fareQualCode() { return _fareQualCode; }
  const Indicator fareQualCode() const { return _fareQualCode; }

  Indicator& tariffFamily() { return _tariffFamily; }
  const Indicator tariffFamily() const { return _tariffFamily; }

  Indicator& cabotageInd() { return _cabotageInd; }
  const Indicator cabotageInd() const { return _cabotageInd; }

  Indicator& govtAppvlInd() { return _govtAppvlInd; }
  const Indicator govtAppvlInd() const { return _govtAppvlInd; }

  Indicator& multiLateralInd() { return _multiLateralInd; }
  const Indicator multiLateralInd() const { return _multiLateralInd; }

  Indicator& viaCityInd() { return _viaCityInd; }
  const Indicator viaCityInd() const { return _viaCityInd; }

  LocCode& viaCity() { return _viaCity; }
  const LocCode& viaCity() const { return _viaCity; }

  LocCode& airport1() { return _airport1; }
  const LocCode& airport1() const { return _airport1; }

  LocCode& airport2() { return _airport2; }
  const LocCode& airport2() const { return _airport2; }

  virtual bool operator==( const SITAFareInfo & rhs ) const
  {
    return(    (  FareInfo::operator==( rhs )                 )
            && ( _routeCode         == rhs._routeCode         )
            && ( _dbeClass          == rhs._dbeClass          )
            && ( _fareQualCode      == rhs._fareQualCode      )
            && ( _tariffFamily      == rhs._tariffFamily      )
            && ( _cabotageInd       == rhs._cabotageInd       )
            && ( _govtAppvlInd      == rhs._govtAppvlInd      )
            && ( _multiLateralInd   == rhs._multiLateralInd   )
            && ( _viaCityInd        == rhs._viaCityInd        )
            && ( _viaCity           == rhs._viaCity           )
            && ( _airport1          == rhs._airport1          )
            && ( _airport2          == rhs._airport2          )
          ) ;
  }

  virtual WBuffer& write(WBuffer& os,
                         size_t* memSize) const
  {
    os.write('S');
    if (memSize)
    {
      *memSize += sizeof(SITAFareInfo);
    }
    return convert(os, this);
  }

  virtual RBuffer& read(RBuffer& is)
  {
    return convert(is, this);
  }

protected:

  RouteCodeC        _routeCode       ;
  DBEClass         _dbeClass        ;
  Indicator        _fareQualCode    ;
  Indicator        _tariffFamily    ;
  Indicator        _cabotageInd     ;
  Indicator        _govtAppvlInd    ;
  Indicator        _multiLateralInd ;
  Indicator        _viaCityInd      ;
  LocCode          _viaCity         ;
  LocCode          _airport1        ;
  LocCode          _airport2        ;

public:

  virtual void dummyData( );

private:
  template <typename B, typename T> static B& convert(B& buffer,
                                                      T ptr)
  {
    return FareInfo::convert(buffer, ptr)
           & ptr->_routeCode
           & ptr->_dbeClass
           & ptr->_fareQualCode
           & ptr->_tariffFamily
           & ptr->_cabotageInd
           & ptr->_govtAppvlInd
           & ptr->_multiLateralInd
           & ptr->_viaCityInd
           & ptr->_viaCity
           & ptr->_airport1
           & ptr->_airport2;
  }
  // Placed here so the clone methods must be used
  SITAFareInfo(const SITAFareInfo& rhs);
  SITAFareInfo& operator=(const SITAFareInfo& rhs);
};
} // namespace tse
