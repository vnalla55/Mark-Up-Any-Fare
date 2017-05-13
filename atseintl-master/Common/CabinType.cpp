#include "Common/CabinType.h"

namespace tse
{

std::ostream& operator<<(std::ostream& os, const CabinType& x)
{
  os << x.getCabinIndicator();
  return os;
}
void
CabinType::setClass(Indicator ch)
{
  switch (ch)
  {
  case UNDEFINED_CLASS:
    _cabin = UNDEFINED_CLASS;
    break;

  case FIRST_CLASS_PREMIUM:
    _cabin = FIRST_CLASS_PREMIUM;
    break;

  case FIRST_CLASS:
    _cabin = FIRST_CLASS;
    break;

  case BUSINESS_CLASS_PREMIUM:
    _cabin = BUSINESS_CLASS_PREMIUM;
    break;

  case BUSINESS_CLASS:
    _cabin = BUSINESS_CLASS;
    break;

  case ECONOMY_CLASS_PREMIUM:
    _cabin = ECONOMY_CLASS_PREMIUM;
    break;

  case ECONOMY_CLASS:
    _cabin = ECONOMY_CLASS;
    break;

  case UNKNOWN_CLASS:
    _cabin = UNKNOWN_CLASS;
    break;

  default:
    _cabin = INVALID_CLASS;
  }
}

void
CabinType::setClassFromAlphaNum(char alphaNum)
{
  switch (alphaNum)
  {
  case FIRST_CLASS_PREMIUM_ALPHA:
    _cabin = FIRST_CLASS_PREMIUM;
    break;

  case FIRST_CLASS_ALPHA:
    _cabin = FIRST_CLASS;
    break;

  case BUSINESS_CLASS_PREMIUM_ALPHA:
    _cabin = BUSINESS_CLASS_PREMIUM;
    break;

  case BUSINESS_CLASS_ALPHA:
    _cabin = BUSINESS_CLASS;
    break;

  case ECONOMY_CLASS_PREMIUM_ALPHA:
    _cabin = ECONOMY_CLASS_PREMIUM;
    break;

  case ECONOMY_CLASS_ALPHA:
    _cabin = ECONOMY_CLASS;
    break;

  default:
    _cabin = INVALID_CLASS;
  }
}
void
CabinType::setClassFromAlphaNumAnswer(char alphaNum)
{
  switch (alphaNum)
  {
  case FIRST_CLASS_PREMIUM_ALPHA_ANSWER:
    _cabin = FIRST_CLASS_PREMIUM;
    break;

  case FIRST_CLASS_ALPHA_ANSWER:
    _cabin = FIRST_CLASS;
    break;

  case BUSINESS_CLASS_PREMIUM_ALPHA_ANSWER:
    _cabin = BUSINESS_CLASS_PREMIUM;
    break;

  case BUSINESS_CLASS_ALPHA_ANSWER:
    _cabin = BUSINESS_CLASS;
    break;

  case ECONOMY_CLASS_PREMIUM_ALPHA_ANSWER:
    _cabin = ECONOMY_CLASS_PREMIUM;
    break;

  case ECONOMY_CLASS_ALPHA_ANSWER:
    _cabin = ECONOMY_CLASS;
    break;

  default:
    _cabin = INVALID_CLASS;
  }
}
Indicator
CabinType::getCabinNumberFromAlphaAnswer(char alphaNum)
{
  switch (alphaNum)
  {
  case FIRST_CLASS_PREMIUM_ALPHA_ANSWER:
    _cabin = FIRST_CLASS_PREMIUM;
    break;

  case FIRST_CLASS_ALPHA_ANSWER:
    _cabin = FIRST_CLASS;
    break;

  case BUSINESS_CLASS_PREMIUM_ALPHA_ANSWER:
    _cabin = BUSINESS_CLASS_PREMIUM;
    break;

  case BUSINESS_CLASS_ALPHA_ANSWER:
    _cabin = BUSINESS_CLASS;
    break;

  case ECONOMY_CLASS_PREMIUM_ALPHA_ANSWER:
    _cabin = ECONOMY_CLASS_PREMIUM;
    break;

  case ECONOMY_CLASS_ALPHA_ANSWER:
    _cabin = ECONOMY_CLASS;
    break;

  default:
    _cabin = INVALID_CLASS;
  }
  return _cabin;
}

char
CabinType::getClassAlphaNum(bool cabinFromAnswer) const
{
  if(cabinFromAnswer)
    return getClassAlphaNumAnswer();

  switch (_cabin)
  {
  case FIRST_CLASS_PREMIUM:
    return FIRST_CLASS_PREMIUM_ALPHA;
  case FIRST_CLASS:
    return FIRST_CLASS_ALPHA;
  case BUSINESS_CLASS_PREMIUM:
    return BUSINESS_CLASS_PREMIUM_ALPHA;
  case BUSINESS_CLASS:
    return BUSINESS_CLASS_ALPHA;
  case ECONOMY_CLASS_PREMIUM:
    return ECONOMY_CLASS_PREMIUM_ALPHA;
  case ECONOMY_CLASS:
    return ECONOMY_CLASS_ALPHA;
  default:
    return ' ';
  }
}

char
CabinType::getClassAlphaNumAnswer() const
{
  switch (_cabin)
  {
  case FIRST_CLASS_PREMIUM:
    return FIRST_CLASS_PREMIUM_ALPHA_ANSWER;
  case FIRST_CLASS:
    return FIRST_CLASS_ALPHA_ANSWER;
  case BUSINESS_CLASS_PREMIUM:
    return BUSINESS_CLASS_PREMIUM_ALPHA_ANSWER;
  case BUSINESS_CLASS:
    return BUSINESS_CLASS_ALPHA_ANSWER;
  case ECONOMY_CLASS_PREMIUM:
    return ECONOMY_CLASS_PREMIUM_ALPHA_ANSWER;
  case ECONOMY_CLASS:
    return ECONOMY_CLASS_ALPHA_ANSWER;
  default:
    return ' ';
  }
}

std::string
CabinType::printNameAnswer(char cabin) const
{
  std::string name = "";
  switch (cabin)
  {
  case FIRST_CLASS_PREMIUM_ALPHA_ANSWER:
    name = "PREMIUM FIRST CABIN";
    break;
  case FIRST_CLASS_ALPHA_ANSWER:
    name = "FIRST CABIN";
    break;
  case BUSINESS_CLASS_PREMIUM_ALPHA_ANSWER:
    name = "PREMIUM BUSINESS CABIN";
    break;
  case BUSINESS_CLASS_ALPHA_ANSWER:
    name = "BUSINESS CABIN";
    break;
  case ECONOMY_CLASS_PREMIUM_ALPHA_ANSWER:
    name = "PREMIUM ECONOMY CABIN";
    break;
  case ECONOMY_CLASS_ALPHA_ANSWER:
    name = "ECONOMY CABIN";
    break;
  default:
    name = "INVALID CABIN";
  }
  return name;
}
std::string
CabinType::printName() const
{
  std::string name = "";
  switch (_cabin)
  {
  case FIRST_CLASS_PREMIUM:
    name = "PREMIUM FIRST CABIN";
    break;
  case FIRST_CLASS:
    name = "FIRST CABIN";
    break;
  case BUSINESS_CLASS_PREMIUM:
    name = "PREMIUM BUSINESS CABIN";
    break;
  case BUSINESS_CLASS:
    name = "BUSINESS CABIN";
    break;
  case ECONOMY_CLASS_PREMIUM:
    name = "PREMIUM ECONOMY CABIN";
    break;
  case ECONOMY_CLASS:
    name = "ECONOMY CABIN";
    break;
  default:
    name = "INVALID CABIN";
  }
  return name;
}
Indicator
CabinType::lowerCabin(Indicator cabin) const
{
  switch (cabin)
  {
  case SUPER_SONIC:
    return FIRST_CLASS_PREMIUM;
  case FIRST_CLASS_PREMIUM:
    return FIRST_CLASS;
  case FIRST_CLASS:
    return BUSINESS_CLASS_PREMIUM;
  case BUSINESS_CLASS_PREMIUM:
    return BUSINESS_CLASS;
  case BUSINESS_CLASS:
    return ECONOMY_CLASS_PREMIUM;
  case ECONOMY_CLASS_PREMIUM:
    return ECONOMY_CLASS;
  case ECONOMY_CLASS:
    return UNKNOWN_CLASS;
  default:
    return UNKNOWN_CLASS;
  }
}
Indicator
CabinType::higherCabin(Indicator cabin) const
{
  switch (cabin)
  {
  case ECONOMY_CLASS:
    return ECONOMY_CLASS_PREMIUM;
  case ECONOMY_CLASS_PREMIUM:
    return BUSINESS_CLASS;
  case BUSINESS_CLASS:
    return BUSINESS_CLASS_PREMIUM;
  case BUSINESS_CLASS_PREMIUM:
    return FIRST_CLASS;
  case FIRST_CLASS:
    return FIRST_CLASS_PREMIUM;
  case FIRST_CLASS_PREMIUM:
    return SUPER_SONIC;
  case SUPER_SONIC:
    return UNKNOWN_CLASS;
  default:
    return UNKNOWN_CLASS;
  }
}
CabinType
CabinType::addOneLevelToCabinType(CabinType cabin)
{
  CabinType newCabin;
  switch (cabin._cabin)
  {
  case SUPER_SONIC:
    newCabin._cabin = FIRST_CLASS_PREMIUM;
    break;
  case FIRST_CLASS_PREMIUM:
    newCabin._cabin = FIRST_CLASS;
    break;
  case FIRST_CLASS:
    newCabin._cabin = BUSINESS_CLASS_PREMIUM;
    break;
  case BUSINESS_CLASS_PREMIUM:
    newCabin._cabin = BUSINESS_CLASS;
    break;
  case BUSINESS_CLASS:
    newCabin._cabin = ECONOMY_CLASS_PREMIUM;
    break;
  case ECONOMY_CLASS_PREMIUM:
    newCabin._cabin = ECONOMY_CLASS;
    break;
  default:
    newCabin._cabin = INVALID_CLASS;
  }
  return newCabin;
}
std::map<CabinType, bool>
CabinType::createEmptyCabinBoolMap()
{
  std::map<CabinType, bool> cabinMap;
  CabinType economy, economy_p, business, business_p, first, first_p;
  economy.setEconomyClass();
  economy_p.setPremiumEconomyClass();
  business.setBusinessClass();
  business_p.setPremiumBusinessClass();
  first.setFirstClass();
  first_p.setPremiumFirstClass();

  cabinMap[economy] = false;
  cabinMap[economy_p] = false;
  cabinMap[business] = false;
  cabinMap[business_p] = false;
  cabinMap[first] = false;
  cabinMap[first_p] = false;

  return cabinMap;
}

size_t
CabinType::Index(Indicator cabin)
{
  switch (cabin)
  {
  case ECONOMY_CLASS:
    return 0;
  case ECONOMY_CLASS_PREMIUM:
    return 1;
  case BUSINESS_CLASS:
    return 2;
  case BUSINESS_CLASS_PREMIUM:
    return 3;
  case FIRST_CLASS:
    return 4;
  case FIRST_CLASS_PREMIUM:
    return 5;
  default:
    return std::string::npos;
  }
}

size_t
CabinType::generalIndex(Indicator cabin)
{
  switch (cabin)
  {
  case ECONOMY_CLASS:
    return 0;
  case ECONOMY_CLASS_PREMIUM:
    return 0;
  case BUSINESS_CLASS:
    return 2;
  case BUSINESS_CLASS_PREMIUM:
    return 2;
  case FIRST_CLASS:
    return 4;
  case FIRST_CLASS_PREMIUM:
    return 4;
  default:
    return std::string::npos;
  }
}

// see CabinType::Index(Indicator cabin) implementation
char CabinType::getClassAlphaNum(size_t index, bool answer)
{
  switch (index)
  {
  case 5:
    if(answer)
      return FIRST_CLASS_PREMIUM_ALPHA_ANSWER;
    return FIRST_CLASS_PREMIUM_ALPHA;
  case 4:
    return FIRST_CLASS_ALPHA;
  case 3:
    return BUSINESS_CLASS_PREMIUM_ALPHA;
  case 2:
    return BUSINESS_CLASS_ALPHA;
  case 1:
    if(answer)
      return ECONOMY_CLASS_PREMIUM_ALPHA_ANSWER;
    return ECONOMY_CLASS_PREMIUM_ALPHA;
  case 0:
    return ECONOMY_CLASS_ALPHA;
  default:
    return ' ';
  }
}

bool
CabinType::canTreatAsSame(const CabinType& rhs) const
{
  if ((isPremiumFirstClass() || isFirstClass()) &&
      (rhs.isPremiumFirstClass() || rhs.isFirstClass()))
    return true;
  else if ((isPremiumBusinessClass() || isBusinessClass()) &&
           (rhs.isPremiumBusinessClass() || rhs.isBusinessClass()))
    return true;
  else if ((isPremiumEconomyClass() || isEconomyClass()) &&
           (rhs.isPremiumEconomyClass() || rhs.isEconomyClass()))
    return true;
  else
    return false;
}
} // namespace tse
