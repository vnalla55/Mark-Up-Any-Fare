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

#include "SITAFareInfo.h"

using namespace tse;

SITAFareInfo::SITAFareInfo()
  : FareInfo(),
    _fareQualCode(),
    _tariffFamily(),
    _cabotageInd(),
    _govtAppvlInd(),
    _multiLateralInd(),
    _viaCityInd()
{
}

void SITAFareInfo::clone( FareInfo& cloneObj ) const
{
  SITAFareInfo& sitaClone = static_cast<SITAFareInfo&>( cloneObj );

  sitaClone._routeCode       = _routeCode;
  sitaClone._dbeClass        = _dbeClass;
  sitaClone._fareQualCode    = _fareQualCode;
  sitaClone._tariffFamily    = _tariffFamily;
  sitaClone._cabotageInd     = _cabotageInd;
  sitaClone._govtAppvlInd    = _govtAppvlInd;
  sitaClone._multiLateralInd = _multiLateralInd;
  sitaClone._airport1        = _airport1;
  sitaClone._airport2        = _airport2;
  sitaClone._viaCityInd      = _viaCityInd;
  sitaClone._viaCity         = _viaCity;

  FareInfo::clone( cloneObj );
}

void SITAFareInfo::dummyData( )
{
  FareInfo::dummyData( ) ;

  _routeCode       = "AB"       ;
  _dbeClass        = "CDE"      ;
  _fareQualCode    = 'F'        ;
  _tariffFamily    = 'G'        ;
  _cabotageInd     = 'H'        ;
  _govtAppvlInd    = 'I'        ;
  _multiLateralInd = 'J'        ;
  _viaCityInd      = 'K'        ;
  _viaCity         = "LMNOP"    ;
  _airport1        = "QRSTU"    ;
  _airport2        = "VWXYZ"    ;
}
