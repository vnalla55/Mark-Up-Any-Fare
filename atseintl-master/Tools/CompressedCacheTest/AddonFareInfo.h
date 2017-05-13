//----------------------------------------------------------------------------
// 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//or transfer of this software/documentation, in any medium, or incorporation of this
//software/documentation into any system or publication, is strictly prohibited
// ----------------------------------------------------------------------------

#pragma once

#include <vector>

#include "TseTypes.h"
#include "CompressedDataUtils.h"
#include "Flattenizable.h"
#include "TSEDateInterval.h"

namespace tse
{
  namespace flattenizer
  {

  template <class AddonFareInfo>
  inline void flatten( Flattenizable::Archive& archive, const std::vector<AddonFareInfo*>& v ) ;
  template <class AddonFareInfo>
  inline void flatten( Flattenizable::Archive& archive, const std::vector<const AddonFareInfo*>& v );
  template <class AddonFareInfo>
  inline void unflatten( Flattenizable::Archive& archive, std::vector<AddonFareInfo*>& v );
  template <class AddonFareInfo>
  inline void unflatten( Flattenizable::Archive& archive, std::vector<const AddonFareInfo*>& v );
  template <class AddonFareInfo>
  inline void calcmem( Flattenizable::Archive& archive, const std::vector<AddonFareInfo*>& v );
  template <class AddonFareInfo>
  inline void calcmem( Flattenizable::Archive& archive, const std::vector<const AddonFareInfo*>& v );


  } // flattenizer

class DataHandle;

class AddonFareInfo
{
public:

  // construction / destruction
  // ============ = ===========

  AddonFareInfo ()
  : _addonTariff(0)
  , _noDec(0)
  , _fareAmt(0)
  , _owrt(' ')
  , _directionality(' ')
  , _arbZone(0)
  , _inhibit(' ')
  , _classFareBasisInd(' ')
  , _globalDir( ZZ )
  , _linkNo(0)
  , _seqNo(0)
  { }
  virtual ~AddonFareInfo() {};

  /**
   * This methods obtains a new FareInfo pointer from
   * the data handle and populates it to be 'equal'
   * to the current object
   *
   * @param dataHandle
   *
   * @return new object
   */
  virtual AddonFareInfo* clone( DataHandle& dataHandle ) const;

  /**
   * This methods populates a given FareInfo to be
   * 'equal' to the current object
   *
   * @param FareInfo - object to populate
   */
  virtual void clone( AddonFareInfo& cloneObj ) const;

  // accessors
  // =========

  TSEDateInterval& effInterval(){ return _effInterval;}
  const TSEDateInterval& effInterval() const { return _effInterval; }

  DateTime& createDate(){ return _effInterval.createDate();}
  const DateTime& createDate() const { return _effInterval.createDate(); }

  DateTime& effDate(){ return _effInterval.effDate();}
  const DateTime& effDate() const { return _effInterval.effDate(); }

  DateTime& expireDate(){ return _effInterval.expireDate(); }
  const DateTime& expireDate() const { return _effInterval.expireDate(); }

  DateTime& discDate(){ return _effInterval.discDate();}
  const DateTime& discDate() const { return _effInterval.discDate(); }

  LocCode& gatewayMarket() { return _gatewayMarket;}
  const LocCode& gatewayMarket() const { return _gatewayMarket; }

  LocCode& interiorMarket() { return _interiorMarket;}
  const LocCode& interiorMarket() const { return _interiorMarket; }

  CarrierCode& carrier() { return _carrier;}
  const CarrierCode& carrier() const { return _carrier; }

  FareClassCode& fareClass() { return _fareClass;}
  const FareClassCode& fareClass() const { return _fareClass; }

  TariffNumber& addonTariff(){ return _addonTariff;}
  const TariffNumber addonTariff() const { return _addonTariff; }

  VendorCode& vendor() { return _vendor;}
  const VendorCode& vendor() const { return _vendor; }

  CurrencyCode& cur() { return _cur;}
  const CurrencyCode& cur() const { return _cur; }

  DateTime& lastModDate(){ return _lastModDate;}
  const DateTime& lastModDate() const { return _lastModDate; }

  int& noDec(){ return _noDec;}
  const int noDec() const { return _noDec; }

  MoneyAmount& fareAmt(){ return _fareAmt;}
  const MoneyAmount fareAmt() const { return _fareAmt; }

  std::string& hashKey(){ return _hashKey;}
  const std::string& hashKey() const { return _hashKey; }

  Footnote& footNote1() { return _footNote1;}
  const Footnote& footNote1() const { return _footNote1; }

  Footnote& footNote2() { return _footNote2;}
  const Footnote& footNote2() const { return _footNote2; }

  Indicator& owrt() { return _owrt;}
  const Indicator owrt() const { return _owrt; }

  RoutingNumber& routing() { return _routing;}
  const RoutingNumber& routing() const { return _routing; }

  Indicator& directionality() { return _directionality;}
  const Indicator directionality() const { return _directionality; }

  AddonZone& arbZone() { return _arbZone;}
  const AddonZone& arbZone() const { return _arbZone; }

  Indicator& inhibit() { return _inhibit;}
  const Indicator& inhibit() const { return _inhibit; }

  Indicator& classFareBasisInd() { return _classFareBasisInd;}
  const Indicator& classFareBasisInd() const { return _classFareBasisInd; }

  GlobalDirection& globalDir() { return _globalDir;}
  const GlobalDirection& globalDir() const { return _globalDir; }

  RouteCode& routeCode() { return _routeCode;}
  const RouteCode& routeCode() const { return _routeCode; }

  LinkNumber& linkNo() { return _linkNo;}
  const LinkNumber& linkNo() const { return _linkNo; }

  SequenceNumber& seqNo() { return _seqNo;}
  const SequenceNumber& seqNo() const { return _seqNo; }

  virtual bool operator==(const AddonFareInfo& rhs) const;

  virtual void dummyData();


  virtual void flattenize(Flattenizable::Archive& archive);

  virtual WBuffer& write(WBuffer& os,
                         size_t* memSize = 0) const
  {
    os.write('A');
    if (memSize)
    {
      *memSize += sizeof(AddonFareInfo);
    }
    return convert(os, this);
  }

  virtual RBuffer& read(RBuffer& is)
  {
    return convert(is, this);
  }

private:

  TSEDateInterval _effInterval;
  LocCode         _gatewayMarket;
  LocCode         _interiorMarket;
  CarrierCode     _carrier;
  FareClassCode   _fareClass;
  TariffNumber    _addonTariff;
  VendorCode      _vendor;
  CurrencyCode    _cur;
  DateTime        _lastModDate;
  int             _noDec;
  MoneyAmount     _fareAmt;
  std::string     _hashKey;
  Footnote        _footNote1;
  Footnote        _footNote2;
  Indicator       _owrt;
  RoutingNumber   _routing;
  Indicator       _directionality;
  AddonZone       _arbZone;
  Indicator       _inhibit;
  Indicator       _classFareBasisInd ;
  GlobalDirection _globalDir ;
  RouteCode   _routeCode ;
  LinkNumber	    _linkNo;
  SequenceNumber  _seqNo;

  template <typename B, typename T> static B& convert (B& buffer,
                                                       T ptr)
  {
    return buffer
           & ptr->_effInterval
           & ptr->_gatewayMarket
           & ptr->_interiorMarket
           & ptr->_carrier
           & ptr->_fareClass
           & ptr->_addonTariff
           & ptr->_vendor
           & ptr->_cur
           & ptr->_lastModDate
           & ptr->_noDec
           & ptr->_fareAmt
           & ptr->_hashKey
           & ptr->_footNote1
           & ptr->_footNote2
           & ptr->_owrt
           & ptr->_routing
           & ptr->_directionality
           & ptr->_arbZone
           & ptr->_inhibit
           & ptr->_classFareBasisInd
           & ptr->_globalDir
           & ptr->_routeCode
           & ptr->_linkNo
           & ptr->_seqNo;
  }
}; // class AddonFareInfo

namespace flattenizer
{
  template <>
  inline void flatten(Flattenizable::Archive& archive, const std::vector<AddonFareInfo*>& v)
  {
  }

  template <>
  inline void unflatten(Flattenizable::Archive& archive, std::vector<AddonFareInfo*>& v)
  {
  }

  template <>
  inline void calcmem(Flattenizable::Archive& archive, const std::vector<AddonFareInfo*>& v)
  {
    size_t item_counter(0);
    archive.addSize(sizeof(size_t));
    for (std::vector<AddonFareInfo*>::const_iterator i = v.begin(); i != v.end(); ++i)
    {
      FLATTENIZE_SUBITEM_BEGIN(archive);
      FLATTENIZE_PUSH(archive, item_counter++);
      archive.addSize(sizeof(size_t));
      calcmem(archive, *(const_cast<AddonFareInfo*>(*i)));
      FLATTENIZE_POP(archive);
      FLATTENIZE_SUBITEM_END(archive);
    }
  }

} // namespace flattenizer

} // namespace tse
