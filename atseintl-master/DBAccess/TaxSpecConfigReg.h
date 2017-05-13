#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/Flattenizable.h"


#include <string>
#include <vector>

namespace tse
{

class TaxSpecConfigReg
{

  friend class TaxSpecConfigRegTest;

public:
  TaxSpecConfigReg()
  {
    _taxSpecConfigName = "";
    // DateTime _createDate;   Dates init themselves
    // DateTime _expireDate;   Dates init themselves
    // DateTime _lockDate;     Dates init themselves
    // DateTime _effDate;      Dates init themselves
    // DateTime _discDate;     Dates init themselves
  }

  ~TaxSpecConfigReg()
  { // Nuke the Kids!
    std::vector<TaxSpecConfigRegSeq*>::iterator it;
    for (it = _seqs.begin(); it != _seqs.end(); ++it)
    {
      delete *it;
    }
  }

  struct TaxSpecConfigRegSeq
  {
    TaxSpecConfigRegSeq()
    {
      _seqNo = 0;
      _paramName = "";
      _paramValue = "";
    }

    int& seqNo() { return _seqNo; }
    const int& seqNo() const { return _seqNo; }

    std::string& paramName() { return _paramName; }
    const std::string& paramName() const { return _paramName; }

    std::string& paramValue() { return _paramValue; }
    const std::string& paramValue() const { return _paramValue; }

    bool operator==(const TaxSpecConfigRegSeq& rhs) const
    {
      bool eq((_seqNo == rhs._seqNo) && (_paramName == rhs._paramName) &&
              (_paramValue == rhs._paramValue));
      return eq;
    }

    static void dummyData(TaxSpecConfigRegSeq& obj)
    {
      obj._seqNo = 2;
      obj._paramName = "A";
      obj._paramValue = "B";
    }

  protected:
    int _seqNo;
    std::string _paramName;
    std::string _paramValue;

  public:
    void flattenize(Flattenizable::Archive& archive)
    {
      FLATTENIZE(archive, _seqNo);
      FLATTENIZE(archive, _paramName);
      FLATTENIZE(archive, _paramValue);
    }

  protected:
  private:
    TaxSpecConfigRegSeq(const TaxSpecConfigRegSeq&);
    TaxSpecConfigRegSeq& operator=(const TaxSpecConfigRegSeq&);
  };

  TaxSpecConfigName& taxSpecConfigName() { return _taxSpecConfigName; }
  const TaxSpecConfigName& taxSpecConfigName() const { return _taxSpecConfigName; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  void setDescription(const std::string& description) { _description = description; }
  const std::string& getDescription() const { return _description; }

  std::vector<TaxSpecConfigRegSeq*>& seqs() { return _seqs; }
  const std::vector<TaxSpecConfigRegSeq*>& seqs() const { return _seqs; }

  bool operator==(const TaxSpecConfigReg& rhs) const
  {
    bool eq((_taxSpecConfigName == rhs._taxSpecConfigName) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_effDate == rhs._effDate) &&
            (_description == rhs._description) && (_discDate == rhs._discDate));
    return eq;
  }

  static void dummyData(TaxSpecConfigReg& obj)
  {
    obj._taxSpecConfigName = "ABC";
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._description = "desc";
  }

protected:
  TaxSpecConfigName _taxSpecConfigName;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  std::string _description;
  std::vector<TaxSpecConfigRegSeq*> _seqs;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _taxSpecConfigName);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _description);
  }

protected:
private:
  TaxSpecConfigReg(const TaxSpecConfigReg&);
  TaxSpecConfigReg& operator=(const TaxSpecConfigReg&);
};
}
