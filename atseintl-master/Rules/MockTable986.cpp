#include "Rules/MockTable986.h"

using namespace std;

namespace tse
{

map<unsigned, MockTable986*> MockTable986::tables;

unsigned
MockTable986::putTable(MockTable986* table)
{
  if (tables.size() == 0)
    tables[0] = nullptr; // Dummy since 0 as a table number means "No value"
  tables[tables.size()] = table;
  return tables.size() - 1;
}

MockTable986*
MockTable986::getTable(unsigned number)
{
  return tables[number];
}

} // namespace tse
