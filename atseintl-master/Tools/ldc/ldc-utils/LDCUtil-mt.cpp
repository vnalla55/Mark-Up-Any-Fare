#include "Tools/ldc/ldc-utils/LDCUtil-mt.h"
#include "Tools/ldc/ldc-utils/mytimes.h"

LDCUtil::LDCUtil(std::string& dirname, bool verboseFlag)
  : dbEnv(nullptr), directory(dirname), berkeleyDBPageSize(4096), verbose(verboseFlag)
{
  createEnvironment();
}

bool
LDCUtil::createEnvironment()
{
  bool retval = false;

  if (verbose)
    fprintf(stderr, "create environment\n");
  if (db_env_create(&dbEnv, 0) == 0)
  {
    if (verbose)
      fprintf(stderr, "open environment\n");
    if (dbEnv->open(
            dbEnv, directory.c_str(), DB_CREATE | DB_INIT_CDB | DB_INIT_MPOOL | DB_THREAD, 0) == 0)
    {
      retval = true;
    }
    else
    {
      fprintf(stderr, "Unable to open Berkeley DB Environment\n");
    }
  }
  else
  {
    fprintf(stderr, "Unable to create Berkeley DB Environment\n");
  }

  if (!retval)
  {
    closeEnvironment();
  }
  return retval;
}

DB*
LDCUtil::openDB(std::string& filename)
{
  bool retval = false;
  DB* db = nullptr;

  if (dbEnv)
  {
    if (verbose)
      fprintf(stderr, "create db\n");
    if (db_create(&db, dbEnv, 0) == 0)
    {
      if (verbose)
        fprintf(stderr, "set page size\n");
      if (db->set_pagesize(db, berkeleyDBPageSize) == 0)
      {
        if (verbose)
          fprintf(stderr, "open db\n");
        if (db->open(db, nullptr, filename.c_str(), nullptr, DB_BTREE, DB_THREAD, 0) == 0)
        {
        }
        else
        {
          fprintf(stderr, "Unable to open Berkeley DB [%s]\n", filename.c_str());
        }
      }
      else
      {
        fprintf(stderr, "Unable to set page size DB [%s]\n", filename.c_str());
      }
    }
    else
    {
      fprintf(stderr, "Unable to create Berkeley DB [%s]\n", filename.c_str());
    }
  }
  else
  {
    fprintf(stderr, "No Berkeley DB Environment\n");
  }
  return db;
}
