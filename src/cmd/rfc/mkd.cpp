#include "cmd/rfc/mkd.hpp"
#include "fs/directory.hpp"
#include "acl/path.hpp"

namespace cmd { namespace rfc
{

void MKDCommand::Execute()
{
  std::string messagePath;
  util::Error e(acl::path::Filter(client.User(), fs::Path(argStr).Basename(), messagePath));
  if (!e)
  {
    // should display above messagepath, we'll just reply for now
    control.Reply(ftp::ActionNotOkay, "Directory name contains one or more invalid characters.");
    return;
  }

  e = fs::CreateDirectory(client,  argStr);
  if (!e) control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
  else control.Reply(ftp::PathCreated, "MKD command successful."); 
  return;
}

} /* rfc namespace */
} /* cmd namespace */
