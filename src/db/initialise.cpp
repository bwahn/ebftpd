#include "db/initialise.hpp"
#include "db/connection.hpp"
#include "db/replicator.hpp"
#include "db/user/usercache.hpp"
#include "db/group/groupcache.hpp"
#include "db/user/util.hpp"
#include "db/group/util.hpp"

namespace db
{

bool CreateUpdateLog()
{
  try
  {
    SafeConnection conn;
    mongo::BSONObj info;
    conn.RunCommand(BSON("create" << "updatelog" << 
                         "capped" << true << 
                         "size" << 102400 << 
                         "max" << 100), info);
    return true;
  }
  catch (const mongo::DBException&)
  { }
  catch (const DBError&)
  { }
  
  return false;
}

bool EnsureIndexes()
{
  try
  {
    SafeConnection conn;
    conn.EnsureIndex("users", BSON("uid" << 1), true);
    conn.EnsureIndex("users", BSON("name" << 1), true);
    conn.EnsureIndex("groups", BSON("gid" << 1), true);
    conn.EnsureIndex("groups", BSON("name" << 1), true);
    conn.EnsureIndex("index", BSON("path" << 1), true);
    conn.EnsureIndex("dupe", BSON("directory" << 1), true);
    conn.EnsureIndex("updatelog", BSON("timestamp" << 1), false);
    conn.EnsureIndex("transfers", BSON("uid" << 1 << 
                                       "direction" << 1 << 
                                       "section" << 1 << 
                                       "day" << 1 << 
                                       "week" << 1 << 
                                       "month" << 1 << 
                                       "year" << 1), true);
    return true;
  }
  catch (const mongo::DBException&)
  { }
  catch (const DBError&)
  { }
  
  return false;
}

bool RegisterCaches(const std::function<void(acl::UserID)>& userUpdatedCB)
{
  try
  {
    auto& replicator = Replicator::Get();
    auto userCache = std::make_shared<UserCache>(userUpdatedCB);
    if (!replicator.Register(userCache)) return false;
    SetUserCache(userCache);
    
    auto groupCache = std::make_shared<GroupCache>();
    if (!replicator.Register(groupCache)) return false;
    SetGroupCache(groupCache);
    
    return true;
  }
  catch (const mongo::DBException&)
  { }
  catch (const DBError&)
  { }
  
  return false;
}

bool Initialise(const std::function<void(acl::UserID)>& userUpdatedCB)
{
  if (!CreateUpdateLog())
  {
    logs::Database("Error while creating update log");
    return false;
  }

  if (!EnsureIndexes())
  {
    logs::Database("Error while building database indexes");
    return false;
  }

  if (!RegisterCaches(userUpdatedCB))
  {
    logs::Database("Error while initialising database replication");
    return false;
  }
  
  return true;
}

} /* db namespace */
