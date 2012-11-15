#include "cmd/site/readd.hpp"
#include "acl/usercache.hpp"

namespace cmd { namespace site
{

void READDCommand::Execute()
{
  // needs further checking to ensure
  // gadmins can't exceed their slots
  
  util::Error e = acl::UserCache::Readd(args[1]);
  if (!e)
    control.Reply(ftp::ActionNotOkay, e.Message());
  else
    control.Reply(ftp::CommandOkay, "User " + args[1] + " has been readded.");
  return;
}

} /* site namespace */
} /* cmd namespace */
