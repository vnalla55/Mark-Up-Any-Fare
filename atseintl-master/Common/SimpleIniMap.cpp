#include "Common/SimpleIniMap.h"

std::string
rtrim(std::string node, std::string substr)
{
  std::string s = node;

  s.erase(s.find_last_not_of(substr) + 1, s.length() - (s.find_last_not_of(substr) + 1));

  return s;
}

std::string
trim(std::string node, std::string substr)
{
  std::string s = node;

  s.erase(0, s.find_first_not_of(substr));
  s.erase(s.find_last_not_of(substr) + 1, s.length() - (s.find_last_not_of(substr) + 1));

  return s;
}

std::string
trim(std::string node, std::string substr1, std::string substr2)
{
  std::string s = node;

  s.erase(0, s.find_first_not_of(substr1));
  s.erase(s.find_last_not_of(substr2) + 1, s.length() - (s.find_last_not_of(substr2) + 1));

  return s;
}

bool
SimpleIniMap::GetParams(std::string fileName)
{
  char line[1024] = { 0 };

  Param param;
  std::ifstream ifs(fileName.c_str());
  if (ifs)
  {
    std::string section = "";
    std::string node;
    std::string sval;

    do
    {
      ifs.getline(line, 10000);
      node = line;
      node = trim(node, " ");

      if ((node[0] == '[' && node[node.length() - 1] == ']'))
      {
        if (param.size() > 0)
        {
          m_sections.insert(SectionMap::value_type(std::string(section), param));
          param.clear();
        }

        section = trim(trim(node, "[", "]"), " ");
      }
      else if (node.find_first_of("=") != std::string::npos)
      {
        sval = trim(node.substr(node.find_first_of("=") + 1), " ");
        node = trim(node.substr(0, node.find_first_of("=")), " ");

        param.insert(Param::value_type(node, sval));
      }
    } while (!ifs.eof());

    if (param.size() > 0)
    {
      m_sections.insert(SectionMap::value_type(std::string(section), param));
      param.clear();
    }

    return true;
  }
  else
    return false;
}

bool
SimpleIniMap::exist(std::string section, std::string key)
{
  iterator pos;
  pos = find(section);

  if (pos == end())
  {
    return false;
  }

  if (key.empty())
  {
    return false;
  }

  Param::iterator pos2;
  pos2 = pos->second.find(key);

  return pos2 != pos->second.end();
}
