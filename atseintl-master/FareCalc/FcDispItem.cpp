#include "FareCalc/FcDispItem.h"

#include <iomanip>
#include <iostream>
#include <string>

namespace tse
{
std::string
FcDispItem::convertAmount(const MoneyAmount amount, uint16_t inNoDec, uint16_t outNoDec)
{
  std::ostringstream os;
  os << std::fixed << std::setprecision(inNoDec) << amount;

  if (inNoDec < outNoDec)
  {
    if (!inNoDec)
      os << '.';
    os << std::setfill('0') << std::setw(outNoDec) << 0;
  }

  std::string str = os.str();
  size_t i = str.find('.', 0);
  if (i != std::string::npos)
  {
    if (outNoDec)
    {
      str.erase(i + outNoDec + 1);
    }
    else
    {
      int i = 0;
      for (i = str.length() - 1; str[i] == '0'; --i)
        str.erase(i);
      if (str[i] == '.')
        str.erase(i);
    }
  }
  return str;
}

std::ostream& operator<<(std::ostream& os, const FcDispItem& i) { return (os << i.toString()); }

} // namespace tse
