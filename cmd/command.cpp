#include <ctime>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/iostreams/concepts.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include "cmd/command.hpp"
#include "ftp/client.hpp"
#include "fs/directory.hpp"
#include "fs/file.hpp"
#include "util/error.hpp"
#include "fs/status.hpp"
#include "fs/path.hpp"
#include "cmd/dirlist.hpp"
#include "cmd/factory.hpp"
#include "util/misc.hpp"

#include <iostream>

namespace cmd
{

void ABORCommand::Execute()
{
  client.Reply(ftp::DataClosedOkay, "ABOR command successful."); 
}

void ACCTCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "ACCT Command not implemented."); 
}

void ADATCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "ADAT Command not implemented."); 
}

void ALLOCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "ALLO Command not implemented."); 
}

void APPECommand::Execute()
{
  client.Reply(ftp::NotImplemented, "APPE Command not implemented."); 
}

void AUTHCommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  if (argStr != "TLS")
  {
    client.Reply(ftp::ParameterNotImplemented,
                 "AUTH " + argStr + " is unsupported.");
    return;
  }
  
  client.Reply(ftp::SecurityExchangeOkay, "AUTH TLS successful."); 
  client.NegotiateTLS();  
}

void CCCCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "CCC Command not implemented."); 
}

void CDUPCommand::Execute()
{
  if (!argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  util::Error e = fs::ChangeDirectory(client,  "..");
  if (!e) client.Reply(ftp::ActionNotOkay, "CDUP failed: " + e.Message());
  else client.Reply(ftp::FileActionOkay, "CDUP command successful."); 
}

void CONFCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "CONF Command not implemented."); 
}

void CWDCommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  util::Error e = fs::ChangeDirectory(client,  argStr);
  if (!e) client.Reply(ftp::ActionNotOkay, "CWD failed: " + e.Message());
  else client.Reply(ftp::FileActionOkay, "CWD command successful."); 
}

void DELECommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  util::Error e = fs::DeleteFile(client,  argStr);
  if (!e) client.Reply(ftp::ActionNotOkay, "DELE failed: " + e.Message());
  else client.Reply(ftp::FileActionOkay, "DELE command successful."); 
}

void ENCCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "ENC Command not implemented."); 
}

void EPRTCommand::Execute()
{
  if (args.size() != 2)
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }

  std::string delim = args[1].substr(0, 1);
  std::vector<std::string> fields;
  boost::split(fields, args[1], boost::is_any_of(delim));
  if (fields.size() != 5)
  {
    client.Reply(ftp::SyntaxError, "Invalid port string.");
    return;
  }
  
  if (fields[1] != "1" && fields[1] != "2")
  {
    client.Reply(ftp::SyntaxError, "Uknown address family.");
    return;
  }
  
  uint16_t port;
  try
  {
    port = boost::lexical_cast<uint16_t>(fields[3]);
  }
  catch (const boost::bad_lexical_cast&)
  {
    client.Reply(ftp::SyntaxError, "Invalid port string.");
    return;
  }
  
  util::net::Endpoint ep;
  try
  {
    ep = util::net::Endpoint(fields[2], port);
  }
  catch (const util::net::InvalidIPAddressError&)
  {
    client.Reply(ftp::SyntaxError, "Invalid port string.");
    return;
  }
  
  try
  {
    client.DataInitialise(ep, false);
  }
  catch (const util::net::NetworkError& e)
  {
    client.Reply(ftp::CantOpenDataConnection,
                 "Unable to open data connection to " + ep.ToString());
    return;
  }
  
  client.Reply(ftp::CommandOkay, "EPRT command successful.");
}

void EPSVCommand::Execute()
{
  if (!argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  util::net::Endpoint ep;
  try
  {
    client.DataInitialise(ep, true);
  }
  catch (const util::net::NetworkError& e)
  {
    client.Reply(ftp::CantOpenDataConnection,
                 "Unable to listen for data connection.");
    return;
  }
  
  int16_t family = ep.IP().Family() == util::net::IPAddress::IPv4 ? 1 : 2;
  std::ostringstream hostString;
  
  if (client.ExtPasvMode() == ftp::EPSVFull)
    hostString << "|" << family
               << "|" << ep.IP()
               << "|" << ep.Port() << "|";
  else
    hostString << "|||" << ep.Port() << "|";
  
  client.Reply(ftp::ExtendedPassiveMode, "Entering extended passive mode (" + 
               hostString.str() + ")");
}

void FEATCommand::Execute()
{
  client.PartReply(ftp::SystemStatus, "Extended feature support:");
  client.PartReply(ftp::NoCode, " AUTH TLS");
  client.PartReply(ftp::NoCode, " EPRT");
  client.PartReply(ftp::NoCode, " EPSV");
  client.PartReply(ftp::NoCode, " PBSZ");
  client.PartReply(ftp::NoCode, " PROT");
  client.PartReply(ftp::NoCode, " MDTM");
  client.PartReply(ftp::NoCode, " SIZE");
  client.PartReply(ftp::NoCode, " SIZE");
  client.Reply(ftp::SystemStatus, "End.");
}

void HELPCommand::Execute()
{
  static const char* reply =
    "FTP Command listing:\n"
    "------------------------------------------------------------------------\n"
    " ABOR  ACCT  ADAT  ALLO  APPE  AUTH  CCC   CDUP  CONF  CWD   DELE  ENC\n"
    " EPRT  EPSV  FEAT  HELP  LANG  LIST  LPRT  LPSV  MDTM  MIC   MKD   MLSD\n"
    " MLST  MODE  NLST  NOOP  OPTS  PASS  PASV  PBSZ  PORT  PROT  PWD   QUIT\n"
    " REIN  REST  RETR  RMD   RNFR  RNTO  SITE  SIZE  SMNT  STAT  STOR  STOU\n"
    " STRU  SYST  TYPE  USER  XCUP  XMKD  XPWD  XRCP  XRMD  XRSQ  XSEM  XSEN\n"
    "------------------------------------------------------------------------\n"
    "End of list.";
    
  client.MultiReply(ftp::HelpMessage, reply);
}

void LANGCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "LANG Command not implemented."); 
}

void LISTCommand::Execute()
{
  client.Reply(ftp::TransferStatusOkay,
               "Opening data connection for directory listing.");

  try
  {
    client.DataOpen();
  }
  catch (const util::net::NetworkError&e )
  {
    client.Reply(ftp::CantOpenDataConnection,
                 "Unable to accept data connection: " + e.Message());
    return;
  }
  
  std::string options;
  std::string path(".");
  if (args.size() >= 2)
  {
    std::string::size_type optOffset = 0;
    if (args[1][0] == '-')
    {
      options = args[1].substr(1);
      optOffset += args[1].length();
    }
    
    std::string path(argStr, optOffset);
    boost::trim(path);
  }
  
  DirectoryList dirList(client, path, ListOptions(options, "l"), true);

  try
  {
    dirList.Execute();    
  }
  catch (const util::net::NetworkError& e)
  {
    client.DataClose();
    client.Reply(ftp::DataCloseAborted,
                "Error whiling writing to data connection: " + e.Message());
    return;
  }
  
  client.DataClose();
  client.Reply(ftp::DataClosedOkay, "End of directory listing."); 
}

void LPRTCommand::Execute()
{
  if (args.size() != 2)
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  std::vector<int16_t> fields;
  util::SplitToType(args[1], ',', fields);

  if (fields.size() < 2)
  {
    client.Reply(ftp::SyntaxError, "Invalid port string.");
    return;
  }
  
  for (size_t i = 0; i < fields.size(); ++i)
  {
    if (fields[i] < 0 || fields[i] > 255)
    {
      client.Reply(ftp::SyntaxError, "Invalid port string.");
      return;
    }
  }
  
  int16_t family = fields[0];
  if (family != 4 && family != 6)
  {
    client.Reply(ftp::SyntaxError, "Unsupported address family.");
    return;
  }
  
  uint16_t addrLen = fields[1];
  char addr[addrLen];
  
  if (fields.size() != addrLen + 5 || 
      fields[addrLen + 2] != 2)
  {
    client.Reply(ftp::SyntaxError, "Invalid port string.");
    return;
  }
  
  for (size_t i = 2, j = 0; j < addrLen; ++i, ++j)
    addr[j] = fields[i];

  uint16_t port = (fields[addrLen + 3] << 8) | fields[addrLen + 4];
  
  util::net::Endpoint ep(addr, port);
    
}

void LPSVCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "LPSV Command not implemented."); 
}

void MDTMCommand::Execute()
{
  static const char* dummySiteRoot = "/home/bioboy/ftpd/site";
  
  if (argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  fs::Path absolute = client.WorkDir() / args[1];
  
  // ACL check
  
  fs::Path real = fs::Path(dummySiteRoot) + absolute;
  
  fs::Status status;
  try
  {
    status.Reset(real);
  }
  catch (const util::SystemError& e)
  {
    client.Reply(ftp::ActionNotOkay, "MDTM failed: " + e.Message());
    return;
  }
  
  char timestamp[15];
  strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%S",
           localtime(&status.Native().st_mtime));
  client.Reply(ftp::FileStatus, timestamp);
}

void MICCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "MIC Command not implemented."); 
}

void MKDCommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  util::Error e = fs::CreateDirectory(client,  argStr);
  if (!e) client.Reply(ftp::ActionNotOkay, "MKD failed: " + e.Message());
  else client.Reply(ftp::PathCreated, "MKD command successful."); 
}

void MLSDCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "MLSD Command not implemented."); 
}

void MLSTCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "MLST Command not implemented."); 
}

void MODECommand::Execute()
{
  if (args.size() != 2)
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  if (args[1] == "S")
    client.Reply(ftp::CommandOkay, "Transfer mode set to 'stream'.");
  else if (args[1] == "B")
    client.Reply(ftp::ParameterNotImplemented,
                 "Transfer mode 'block' not implemented.");
  else if (args[1] == "C")
    client.Reply(ftp::ParameterNotImplemented,
                 "Transfer mode 'compressed' not implemented.");
  else
    client.Reply(ftp::SyntaxError, "Unrecognised transfer mode.");
}

void NLSTCommand::Execute()
{
  client.Reply(ftp::TransferStatusOkay,
               "Opening data connection for directory listing.");

  try
  {
    client.DataOpen();
  }
  catch (const util::net::NetworkError&e )
  {
    client.Reply(ftp::CantOpenDataConnection,
                 "Unable to accept data connection: " + e.Message());
    return;
  }
  
  std::string options;
  std::string path(".");
  if (args.size() >= 2)
  {
    std::string::size_type optOffset = 0;
    if (args[1][0] == '-')
    {
      options = args[1].substr(1);
      optOffset += args[1].length();
    }
    
    path = argStr.substr(optOffset);
    boost::trim(path);
  }
  
  DirectoryList dirList(client, path, ListOptions(options, ""), true);

  try
  {
    dirList.Execute();    
  }
  catch (const util::net::NetworkError& e)
  {
    client.Reply(ftp::DataCloseAborted,
                "Error whiling writing to data connection: " + e.Message());
  }
  
  client.DataClose();
  client.Reply(ftp::DataClosedOkay, "End of directory listing."); 
}

void NOOPCommand::Execute()
{
  client.Reply(ftp::CommandOkay, "NOOP command successful."); 
}

void OPTSCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "OPTS Command not implemented."); 
}

void PASSCommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;    
  }
  
  if (!client.VerifyPassword(argStr))
  {
    if (client.PasswordAttemptsExceeded())
    {
      client.Reply(ftp::NotLoggedIn,
                  "Password attempts exceeded, disconnecting.");
      client.SetFinished();
    }
    else
    {
      client.Reply(ftp::NotLoggedIn, "Login incorrect.");
      client.SetLoggedOut();
    }
    return;
  }
  
  client.Reply(ftp::UserLoggedIn, "User " + client.User().Name() + " logged in.");
  client.SetLoggedIn();
}

void PASVCommand::Execute()
{
  if (!argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  util::net::Endpoint ep;
  try
  {
    client.DataInitialise(ep, true);
  }
  catch (const util::net::NetworkError& e)
  {
    client.Reply(ftp::CantOpenDataConnection,
                 "Unable to listen for data connection.");
    return;
  }
  
  std::ostringstream hostString;
  hostString << boost::replace_all_copy(ep.IP().ToString(), ".", ",")
             << "," << ((ep.Port() >> 8) & 255)
             << "," << (ep.Port() & 255);
  
  client.Reply(ftp::PassiveMode, "Entering passive mode (" + hostString.str() + ")");
}

void PBSZCommand::Execute()
{
  if (args.size() != 2)
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  if (args[1] != "0")
    client.Reply(ftp::ParameterNotImplemented, "Only protection buffer size 0 supported.");
  else
    client.Reply(ftp::CommandOkay, "Protection buffer size set to 0.");
}

void PORTCommand::Execute()
{
  if (args.size() != 2)
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }

  std::vector<std::string> octets;
  boost::split(octets, args[1], boost::is_any_of(","));
  if (octets.size() != 6)
  {
    client.Reply(ftp::SyntaxError, "Invalid port string.");
    return;
  }

  int16_t intOctets[6];
  try
  {
    for (unsigned i = 0; i < octets.size(); ++i)
      intOctets[i] = boost::lexical_cast<int16_t>(octets[i]);
  }
  catch (const boost::bad_lexical_cast& e)
  {
    client.Reply(ftp::SyntaxError, "Invalid port string.");
    return;
  }
  
  for (unsigned i = 0; i < octets.size(); ++i)
    if (intOctets[i] < 0 || intOctets[i] > 255)
    {
      client.Reply(ftp::SyntaxError, "Invalid port string.");
      return;
    }
  
  int32_t port = (intOctets[4] << 8) | intOctets[5];

  std::ostringstream ip;
  ip << intOctets[0] << "."
     << intOctets[1] << "."
     << intOctets[2] << "."
     << intOctets[3];
  
  util::net::Endpoint ep;
  try
  {
    ep = util::net::Endpoint(ip.str(), port);
  }
  catch (const util::net::InvalidIPAddressError&)
  {
    client.Reply(ftp::SyntaxError, "Invalid port string.");
    return;
  }
  
  try
  {
    client.DataInitialise(ep, false);
  }
  catch (const util::net::NetworkError& e)
  {
    client.Reply(ftp::CantOpenDataConnection,
                 "Unable to open data connection to " + ep.ToString());
    return;
  }
  
  client.Reply(ftp::CommandOkay, "PORT command successful.");
}

void PROTCommand::Execute()
{
  if (args.size() != 2)
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  if (args[1] == "P")
  {
    client.SetDataProtected(true);
    client.Reply(ftp::CommandOkay, "Protection type set to 'private'.");
  }
  else if (args[1] == "C")
  {
    client.SetDataProtected(false);
    client.Reply(ftp::CommandOkay, "Protection type set to 'clear'.");
  }
  else if (args[1] == "S")
    client.Reply(ftp::ParameterNotImplemented,
                 "Protection type 'secure' not implemented.");
  else if (args[1] == "E")
    client.Reply(ftp::ParameterNotImplemented,
                 "Protection type 'confidential' not implemented.");
  else
    client.Reply(ftp::SyntaxError, "Unrecognised protection type.");
}

void PWDCommand::Execute()
{
  client.Reply(ftp::PathCreated, "\"" + client.WorkDir().ToString() +
               "\" is your working directory.");
}

void QUITCommand::Execute()
{
  client.Reply(ftp::ClosingControl, "Bye bye"); 
  client.SetFinished();
}

void REINCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "REIN Command not implemented."); 
}

void RESTCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "REST Command not implemented."); 
}

void RETRCommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
    
  fs::InStreamPtr fin;
  try
  {
    fin = fs::OpenFile(client, argStr);
  }
  catch (const util::SystemError& e)
  {
    client.Reply(ftp::ActionNotOkay,
                 "Unable to open file: " + e.Message());
    return;
  }

  client.Reply(ftp::TransferStatusOkay,
               "Opening data connection for download of " + 
               fs::Path(argStr).Basename().ToString() + ".");

  try
  {
    client.DataOpen();
  }
  catch (const util::net::NetworkError&e )
  {
    client.Reply(ftp::CantOpenDataConnection,
                 "Unable to accept data connection: " +
                 e.Message());
    return;
  }

  try
  {
    char buffer[16384];
    while (true)
    {
      ssize_t len = boost::iostreams::read(*fin,buffer, sizeof(buffer));
      if (len < 0) break;
      client.data.Write(buffer, len);
    }
  }
  catch (const std::ios_base::failure&)
  {
    fin->close();
    client.DataClose();
    client.Reply(ftp::DataCloseAborted,
                 "Error while reading from disk.");
    return;
  }
  catch (const util::net::NetworkError& e)
  {
    client.DataClose();
    client.Reply(ftp::DataCloseAborted,
                 "Error while writing to data connection: " +
                 e.Message());
    return;
  }
  
  client.DataClose();
  client.Reply(ftp::DataClosedOkay, "Transfer finished."); 
}

void RMDCommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  util::Error e = fs::RemoveDirectory(client,  argStr);
  if (!e) client.Reply(ftp::ActionNotOkay, "RMD failed: " + e.Message());
  else client.Reply(ftp::FileActionOkay, "RMD command successful."); 
}

void RNFRCommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }

  fs::Path absolute = (client.WorkDir() / argStr).Expand();
  
  try
  {
    fs::Status status(absolute);
  }
  catch (const util::SystemError& e)
  {
    client.Reply(ftp::ActionNotOkay, "RNFR failed: " + e.Message());
    return;
  }
  
  client.PartReply(ftp::PendingMoreInfo, std::string(absolute));
  client.SetRenameFrom(absolute);
  client.Reply("File exists, ready for destination name."); 
}

void RNTOCommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  util::Error e = fs::RenameFile(client, client.RenameFrom(), argStr);
  if (!e) client.Reply(ftp::ActionNotOkay, "RNTO failed: " + e.Message());
  else client.Reply(ftp::FileActionOkay, "RNTO command successful.");
}

void SITECommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  boost::to_upper(args[1]);
  if (args[1] == "EPSV")
  {
    static const char* syntax = "Syntax: SITE EPSV normal|full";
    if (args.size() == 2)
      client.Reply(ftp::CommandOkay, "Extended passive mode is currently '" +
                   std::string(client.ExtPasvMode() == ftp::EPSVNormal ?
                   "normal" : "full") + "'.");
                    
    else if (args.size() != 3) client.Reply(ftp::SyntaxError, syntax);
    else
    {
      boost::to_upper(args[2]);
      if (args[2] == "NORMAL")
      {
        client.SetExtPasvMode(ftp::EPSVNormal);
        client.Reply(ftp::SyntaxError, "Extended passive mode now set to 'normal'.");
      }
      else if (args[2] == "FULL")
      {
        client.SetExtPasvMode(ftp::EPSVFull);
        client.Reply(ftp::SyntaxError, "Extended passive mode now set to 'full'.");
      }
      else
        client.Reply(ftp::SyntaxError, syntax);
    }
    return;
  }
  
  client.Reply(ftp::CommandUnrecognised, "SITE " + args[1] + 
               " command unrecognised."); 
}

void SIZECommand::Execute()
{
  fs::Path absolute = (client.WorkDir() / argStr).Expand();
  
  // check ACLs
  
  fs::Status status;
  try
  {
    status.Reset(absolute);
  }
  catch (const util::SystemError& e)
  {
    client.Reply(ftp::ActionNotOkay, "SIZE failed: " + e.Message());
    return;
  }
  
  client.Reply(ftp::FileStatus, boost::lexical_cast<std::string>(status.Size())); 
}

void SMNTCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "SMNT Command not implemented."); 
}

void STATCommand::Execute()
{
  if (args.size() == 1)
  {
    client.PartReply(ftp::SystemStatus, "FTPD status:");
    client.PartReply("< Insert status info here >");
    client.Reply("End of status.");
    return;
  }

  std::string options;
  std::string::size_type optOffset = 0;
  if (args[1][0] == '-')
  {
    options = args[1].substr(1);
    optOffset += args[1].length();
  }
  
  std::string path(argStr, optOffset);
  boost::trim(path);
  
  if (path.empty()) path = ".";
  
  client.PartReply(ftp::DirectoryStatus, "Status of " + path + ":");
  DirectoryList dirList(client, path, ListOptions(options, "l"), false);
  dirList.Execute();
  
  client.Reply("End of status.");
}

void STORCommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }

  fs::OutStreamPtr fout;
  try
  {
    fout = fs::CreateFile(client, argStr);
  }
  catch (const util::SystemError& e)
  {
    client.Reply(ftp::ActionNotOkay,
                 "Unable to create file: " + e.Message());
    return;
  }

  client.Reply(ftp::TransferStatusOkay,
               "Opening data connection for upload of " +
               fs::Path(argStr).Basename().ToString() + ".");

  try
  {
    client.DataOpen();
  }
  catch (const util::net::NetworkError&e )
  {
    client.Reply(ftp::CantOpenDataConnection,
                 "Unable to accept data connection: " + e.Message());
    return;
  }

  try
  {
    char buffer[16384];
    while (true)
    {
      size_t len = client.data.Read(buffer, sizeof(buffer));
      fout->write(buffer, len);
    }
  }
  catch (const util::net::EndOfStream&) { }
  catch (const util::net::NetworkError& e)
  {
    client.DataClose();
    client.Reply(ftp::DataCloseAborted,
                 "Error while reading from data connection: " +
                 e.Message());
    return;
  }
  
  client.DataClose();
  client.Reply(ftp::DataClosedOkay, "Transfer finished."); 
}

void STOUCommand::Execute()
{
  static size_t filenameLength = 10;

  if (!argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  fs::Path uniquePath;
  if (!fs::UniqueFile(client, client.WorkDir(),
                      filenameLength, uniquePath))
  {
    client.Reply(ftp::ActionNotOkay,
                 "Unable to generate a unique filename.");
    return;
  }
  
  argStr = uniquePath.ToString();
  args.clear();
  args.push_back("STOR");
  args.push_back(argStr);
  
  ftp::ClientState reqdState;

  std::auto_ptr<cmd::Command>
    command(cmd::Factory::Create(client, argStr, args, reqdState));
  assert(command.get());
  command->Execute();
}

void STRUCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "STRU Command not implemented."); 
}

void SYSTCommand::Execute()
{
  client.Reply(ftp::SystemType, "UNIX Type: L8"); 
}

void TYPECommand::Execute()
{
  if (args.size() != 2)
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  if (args[1] != "I" && args[1] != "A")
  {
    client.Reply(ftp::ParameterNotImplemented,
                 "TYPE " + args[1] + " not supported.");
    return;
  }
  
  client.Reply(ftp::CommandOkay, "TYPE command successful."); 
}

void USERCommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  if (argStr != client.User().Name())
  {
    client.Reply(ftp::NotLoggedIn, "User " + argStr + " access denied.");
    return;
  }
  
  client.Reply(ftp::NeedPassword, "Password required for " + argStr + "."); 
  client.SetWaitingPassword();
}

void XCUPCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "XCUP Command not implemented."); 
}

void XMKDCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "XMKD Command not implemented."); 
}

void XPWDCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "XPWD Command not implemented."); 
}

void XRCPCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "XRCP Command not implemented."); 
}

void XRMDCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "XRMD Command not implemented."); 
}

void XRSQCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "XRSQ Command not implemented."); 
}

void XSEMCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "XSEM Command not implemented."); 
}

void XSENCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "XSEN Command not implemented."); 
}

} /* cmd namespace */
