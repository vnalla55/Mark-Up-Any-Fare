#include "Tools/ldc/ldc-utils/CacheEvent.h"
#include "Tools/ldc/ldc-utils/mytimes.h"
#include "Tools/ldc/ldc-utils/stringFunc.h"

#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static const char* queryText = "select CREATEDATE , ENTITYTYPE , KEYSTRING from %TABLE%";

CacheEvent::CacheEvent(std::string hostArg,
                       std::string userArg,
                       std::string passwordArg,
                       std::string databaseArg,
                       std::string portArg,
                       std::string tableArg)
  : host(hostArg),
    user(userArg),
    password(passwordArg),
    database(databaseArg),
    table(tableArg),
    query(queryText),
    port((unsigned int)atoi(portArg.c_str())),
    upperCaseEntity(true),
    trimLastPipe(true),
    cutSpaces(true)

{
  if (table.length() > 0)
  {
    std::string::size_type pos = query.find("%TABLE%");
    if (pos != std::string::npos)
    {
      query.replace(pos, 7, table);
    }
  }
}
