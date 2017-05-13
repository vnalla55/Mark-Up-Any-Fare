#pragma once

#include "Common/TseBoostStringTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/MinFareFareTypeGrpSeg.h"

#include <vector>

namespace tse
{

class MinFareFareTypeGrp
{
public:
  MinFareFareTypeGrp() {}

  ~MinFareFareTypeGrp()
  {
    std::vector<MinFareFareTypeGrpSeg*>::iterator SegIt;
    for (SegIt = _segs.begin(); SegIt != _segs.end(); SegIt++)
    { // Get Children ... First the ExcptCxrs ...
      delete *SegIt;
    }
  }

  std::string& specialProcessName() { return _specialProcessName; }
  const std::string& specialProcessName() const { return _specialProcessName; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  std::vector<MinFareFareTypeGrpSeg*>& segs() { return _segs; }
  const std::vector<MinFareFareTypeGrpSeg*>& segs() const { return _segs; }

  bool operator==(const MinFareFareTypeGrp& rhs) const
  {
    bool eq =
        ((_specialProcessName == rhs._specialProcessName) && (_createDate == rhs._createDate) &&
         (_expireDate == rhs._expireDate) && (_effDate == rhs._effDate) &&
         (_discDate == rhs._discDate) && (_segs.size() == rhs._segs.size()));

    for (size_t i = 0; (eq && (i < _segs.size())); ++i)
    {
      eq = (*(_segs[i]) == *(rhs._segs[i]));
    }

    return eq;
  }

  static void dummyData(MinFareFareTypeGrp& obj)
  {
    obj._specialProcessName = "aaaaaaaa";
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);

    MinFareFareTypeGrpSeg* mfftgs1 = new MinFareFareTypeGrpSeg;
    MinFareFareTypeGrpSeg* mfftgs2 = new MinFareFareTypeGrpSeg;

    MinFareFareTypeGrpSeg::dummyData(*mfftgs1);
    MinFareFareTypeGrpSeg::dummyData(*mfftgs2);

    obj._segs.push_back(mfftgs1);
    obj._segs.push_back(mfftgs2);
  }

  WBuffer& write(WBuffer& os) const
  {
    return convert(os, this);
  }

  RBuffer& read(RBuffer& is)
  {
    return convert(is, this);
  }

protected:
  std::string _specialProcessName;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  std::vector<MinFareFareTypeGrpSeg*> _segs;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _specialProcessName);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _segs);
  }

protected:
private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    return buffer
           & ptr->_specialProcessName
           & ptr->_createDate
           & ptr->_expireDate
           & ptr->_effDate
           & ptr->_discDate
           & ptr->_segs;
  }

  MinFareFareTypeGrp(const MinFareFareTypeGrp&);
  MinFareFareTypeGrp& operator=(const MinFareFareTypeGrp&);
};

} // namespace tse

