//----------------------------------------------------------------------------
//	   (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//	   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//	   or transfer of this software/documentation, in any medium, or incorporation of this
//	   software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{

class AdvResTktInfo : public RuleItemInfo
{
public:
  AdvResTktInfo()
    : _unavailtag(' '),
      _advTktDepart(0),
      _geoTblItemNo(0),
      _firstResTod(0),
      _lastResTod(0),
      _advTktTod(0),
      _advTktExcpTime(0),
      _tktTSI(0),
      _resTSI(0),
      _permitted(' '),
      _ticketed(' '),
      _standby(' '),
      _confirmedSector(' '),
      _eachSector(' '),
      _advTktOpt(' '),
      _advTktDepartUnit(' '),
      _advTktBoth(' '),
      _advTktExcpUnit(' '),
      _inhibit(' ')
  {
  }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  Indicator& unavailtag() { return _unavailtag; }
  const Indicator& unavailtag() const { return _unavailtag; }

  int& advTktdepart() { return _advTktDepart; }
  const int& advTktdepart() const { return _advTktDepart; }

  int& geoTblItemNo() { return _geoTblItemNo; }
  const int& geoTblItemNo() const { return _geoTblItemNo; }

  int& firstResTod() { return _firstResTod; }
  const int& firstResTod() const { return _firstResTod; }

  int& lastResTod() { return _lastResTod; }
  const int& lastResTod() const { return _lastResTod; }

  int& advTktTod() { return _advTktTod; }
  const int& advTktTod() const { return _advTktTod; }

  int& advTktExcpTime() { return _advTktExcpTime; }
  const int& advTktExcpTime() const { return _advTktExcpTime; }

  TSICode& tktTSI() { return _tktTSI; }
  const TSICode& tktTSI() const { return _tktTSI; }

  TSICode& resTSI() { return _resTSI; }
  const TSICode& resTSI() const { return _resTSI; }

  ResPeriod& firstResPeriod() { return _firstResPeriod; }
  const ResPeriod& firstResPeriod() const { return _firstResPeriod; }

  ResUnit& firstResUnit() { return _firstResUnit; }
  const ResUnit& firstResUnit() const { return _firstResUnit; }

  ResPeriod& lastResPeriod() { return _lastResPeriod; }
  const ResPeriod& lastResPeriod() const { return _lastResPeriod; }

  ResUnit& lastResUnit() { return _lastResUnit; }
  const ResUnit& lastResUnit() const { return _lastResUnit; }

  Indicator& permitted() { return _permitted; }
  const Indicator& permitted() const { return _permitted; }

  Indicator& ticketed() { return _ticketed; }
  const Indicator& ticketed() const { return _ticketed; }

  Indicator& standby() { return _standby; }
  const Indicator& standby() const { return _standby; }

  Indicator& confirmedSector() { return _confirmedSector; }
  const Indicator& confirmedSector() const { return _confirmedSector; }

  Indicator& eachSector() { return _eachSector; }
  const Indicator& eachSector() const { return _eachSector; }

  ResPeriod& advTktPeriod() { return _advTktPeriod; }
  const ResPeriod& advTktPeriod() const { return _advTktPeriod; }

  ResUnit& advTktUnit() { return _advTktUnit; }
  const ResUnit& advTktUnit() const { return _advTktUnit; }

  Indicator& advTktOpt() { return _advTktOpt; }
  const Indicator& advTktOpt() const { return _advTktOpt; }

  Indicator& advTktDepartUnit() { return _advTktDepartUnit; }
  const Indicator& advTktDepartUnit() const { return _advTktDepartUnit; }

  Indicator& advTktBoth() { return _advTktBoth; }
  const Indicator& advTktBoth() const { return _advTktBoth; }

  Indicator& advTktExcpUnit() { return _advTktExcpUnit; }
  const Indicator& advTktExcpUnit() const { return _advTktExcpUnit; }

  DateTime& waiverResDate() { return _waiverResDate; }
  const DateTime& waiverResDate() const { return _waiverResDate; }

  DateTime& waiverTktDate() { return _waiverTktDate; }
  const DateTime& waiverTktDate() const { return _waiverTktDate; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  virtual bool operator==(const AdvResTktInfo& rhs) const;

  friend std::ostream& dumpObject(std::ostream& os, const AdvResTktInfo& obj);

  static void dummyData(AdvResTktInfo& obj);

  virtual void flattenize(Flattenizable::Archive& archive) override;

  WBuffer& write(WBuffer& os) const
  {
    return convert(os, this);
  }

  RBuffer& read(RBuffer& is)
  {
    return convert(is, this);
  }

protected:
  DateTime _createDate;
  DateTime _expireDate;
  Indicator _unavailtag;
  int _advTktDepart;
  int _geoTblItemNo;
  int _firstResTod;
  int _lastResTod;
  int _advTktTod;
  int _advTktExcpTime;
  TSICode _tktTSI;
  TSICode _resTSI;
  ResPeriod _firstResPeriod;
  ResUnit _firstResUnit;
  ResPeriod _lastResPeriod;
  ResUnit _lastResUnit;
  Indicator _permitted;
  Indicator _ticketed;
  Indicator _standby;
  Indicator _confirmedSector;
  Indicator _eachSector;
  ResPeriod _advTktPeriod;
  ResUnit _advTktUnit;
  Indicator _advTktOpt;
  Indicator _advTktDepartUnit;
  Indicator _advTktBoth;
  Indicator _advTktExcpUnit;
  DateTime _waiverResDate;
  DateTime _waiverTktDate;
  Indicator _inhibit;

private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    RuleItemInfo::convert(buffer, ptr);
    return buffer
           & ptr->_createDate
           & ptr->_expireDate
           & ptr->_unavailtag
           & ptr->_advTktDepart
           & ptr->_geoTblItemNo
           & ptr->_firstResTod
           & ptr->_lastResTod
           & ptr->_advTktTod
           & ptr->_advTktExcpTime
           & ptr->_tktTSI
           & ptr->_resTSI
           & ptr->_firstResPeriod
           & ptr->_firstResUnit
           & ptr->_lastResPeriod
           & ptr->_lastResUnit
           & ptr->_permitted
           & ptr->_ticketed
           & ptr->_standby
           & ptr->_confirmedSector
           & ptr->_eachSector
           & ptr->_advTktPeriod
           & ptr->_advTktUnit
           & ptr->_advTktOpt
           & ptr->_advTktDepartUnit
           & ptr->_advTktBoth
           & ptr->_advTktExcpUnit
           & ptr->_waiverResDate
           & ptr->_waiverTktDate
           & ptr->_inhibit;
  }

};

} // tse

