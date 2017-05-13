#pragma once

#include "Common/TseConsts.h"
#include "DBAccess/Flattenizable.h"

#include <string>

namespace tse
{

class CabinType
{
  friend class CabinTypeTest;

public:
  enum CabinTypeNew
  { SUPER_SONIC = '0',
    FIRST_CLASS_PREMIUM = '1',
    FIRST_CLASS = '2',
    BUSINESS_CLASS_PREMIUM = '4',
    BUSINESS_CLASS = '5',
    ECONOMY_CLASS_PREMIUM = '7',
    ECONOMY_CLASS = '8',
    UNKNOWN_CLASS = '-', // prevoiusly '4'
    INVALID_CLASS = '9',
    UNDEFINED_CLASS = ' ',
    ALL_CABIN = '*'};

  enum CabinAlphaNum
  { FIRST_CLASS_PREMIUM_ALPHA = 'P',
    FIRST_CLASS_ALPHA = 'F',
    BUSINESS_CLASS_PREMIUM_ALPHA = 'J',
    BUSINESS_CLASS_ALPHA = 'C',
    ECONOMY_CLASS_PREMIUM_ALPHA = 'S',
    ECONOMY_CLASS_ALPHA = 'Y' };

  enum CabinAlphaNumAnswer
  { FIRST_CLASS_PREMIUM_ALPHA_ANSWER = 'R',
    FIRST_CLASS_ALPHA_ANSWER = 'F',
    BUSINESS_CLASS_PREMIUM_ALPHA_ANSWER = 'J',
    BUSINESS_CLASS_ALPHA_ANSWER = 'C',
    ECONOMY_CLASS_PREMIUM_ALPHA_ANSWER = 'W',
    ECONOMY_CLASS_ALPHA_ANSWER = 'Y' };

  // ctors
  explicit CabinType() : _cabin(UNDEFINED_CLASS) {}
  CabinType(const CabinType& src) : _cabin(src._cabin) {}
  CabinType& operator=(const CabinType& src)
  {
    _cabin = src._cabin;
    return *this;
  }
  // operators
  bool operator==(const CabinType& rhs) const { return _cabin == rhs._cabin; }
  bool operator!=(const CabinType& rhs) const { return _cabin != rhs._cabin; }
  bool operator>(const CabinType& rhs) const { return _cabin > rhs._cabin; }
  bool operator>=(const CabinType& rhs) const { return _cabin >= rhs._cabin; }
  bool operator<(const CabinType& rhs) const { return _cabin < rhs._cabin; }
  bool operator<=(const CabinType& rhs) const { return _cabin <= rhs._cabin; }

  // serialization

  void flattenize(Flattenizable::Archive& archive) { FLATTENIZE(archive, _cabin); }

  // is methods
  bool isFirstClass() const { return _cabin == FIRST_CLASS; }
  bool isPremiumFirstClass() const { return _cabin == FIRST_CLASS_PREMIUM; }
  bool isBusinessClass() const { return _cabin == BUSINESS_CLASS; }
  bool isPremiumBusinessClass() const { return _cabin == BUSINESS_CLASS_PREMIUM; }
  bool isEconomyClass() const { return _cabin == ECONOMY_CLASS; }
  bool isPremiumEconomyClass() const { return _cabin == ECONOMY_CLASS_PREMIUM; }
  bool isUndefinedClass() const { return _cabin == UNDEFINED_CLASS; }
  bool isUnknownClass() const { return _cabin == UNKNOWN_CLASS; }
  bool isInvalidClass() const { return _cabin == INVALID_CLASS; }
  bool isAllCabin() const { return _cabin == ALL_CABIN; }

  bool canTreatAsSame(const CabinType& rhs) const;

  // set methods

  void setFirstClass() { _cabin = FIRST_CLASS; }
  void setPremiumFirstClass() { _cabin = FIRST_CLASS_PREMIUM; }
  void setBusinessClass() { _cabin = BUSINESS_CLASS; }
  void setPremiumBusinessClass() { _cabin = BUSINESS_CLASS_PREMIUM; }
  void setEconomyClass() { _cabin = ECONOMY_CLASS; }
  void setPremiumEconomyClass() { _cabin = ECONOMY_CLASS_PREMIUM; }
  void setUndefinedClass() { _cabin = UNDEFINED_CLASS; }
  void setUnknownClass() { _cabin = UNKNOWN_CLASS; }
  void setInvalidClass() { _cabin = INVALID_CLASS; }
  void setAllCabin() { _cabin = ALL_CABIN; }
  void setClass(Indicator ch);
  void setClassFromAlphaNum(char alphaNum);
  char getClassAlphaNum(bool cabinFromAnswer = false) const;
  static char getClassAlphaNum(size_t index, bool cabinFromAnswer = false);
  std::string printName() const;
  void setClassFromAlphaNumAnswer(char alphaNum);
  char getClassAlphaNumAnswer() const;
  std::string printNameAnswer(char alphaNum) const;
  Indicator getCabinNumberFromAlphaAnswer(char alphaNum);
  // helper funtions

  static void dummyData(CabinType& obj) { obj._cabin = SUPER_SONIC; }

  CabinType minFaresLowerCabin() const
  {
    CabinType ret;
    ret._cabin = lowerCabin(_cabin);
    return ret;
  }
  CabinType minFaresHigherCabin() const
  {
    CabinType ret;
    ret._cabin = higherCabin(_cabin);
    return ret;
  }
  Indicator getCabinIndicator() const { return _cabin; }
  bool isValidCabin() const
  {
    return (_cabin == FIRST_CLASS_PREMIUM || _cabin == FIRST_CLASS ||
            _cabin == BUSINESS_CLASS_PREMIUM || _cabin == BUSINESS_CLASS ||
            _cabin == ECONOMY_CLASS_PREMIUM || _cabin == ECONOMY_CLASS);
  }
  static size_t size() { return Index(FIRST_CLASS_PREMIUM) + 1; }
  size_t index() const { return Index(_cabin); }
  size_t generalIndex() const { return generalIndex(_cabin); }
  static size_t generalIndex(Indicator cabin);

  static CabinType addOneLevelToCabinType(CabinType cabin);
  static std::map<CabinType, bool> createEmptyCabinBoolMap();

private:
  static size_t Index(Indicator cabin);
  Indicator lowerCabin(Indicator cabin) const;
  Indicator higherCabin(Indicator cabin) const;
  ////////////////////////////////////////////////
  Indicator _cabin;
};

std::ostream& operator<<(std::ostream& os, const CabinType& x);

} // namespace tse
