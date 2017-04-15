#include <string.h>
#include <errno.h>
#include <lmerr.h>
#include "lgrperrs.h"

const char* APISnag::message()
{
  switch (_code)
  {
    case ERROR_ACCESS_DENIED:
      return "access is denied to current user";
    case RPC_S_SERVER_UNAVAILABLE:
      return "no RPC server available - can't query the named host";
    case NERR_InvalidComputer:
      return "the given hostname is unrecognized";
    case NERR_BufTooSmall:
      return "the API result buffer is too small [DEVEL ERROR]";
    case NERR_GroupNotFound:
      return "the group name could not be found";

    // The following status value has never occurred in my tests, and I suspect
    // that the documentation that includes it is wrong, because it doesn't
    // mention NERR_GroupNotFound, which is easily provoked; but I'm leaving it
    // here just in case.
    case ERROR_NO_SUCH_ALIAS:
      return "the specified local group does not exist";

    // These are possible errors from ConvertSidToStringSid:
    case ERROR_NOT_ENOUGH_MEMORY:
      return "not enough memory to perform the API request";
    case ERROR_INVALID_SID:
      return "an invalid SID was passed to an API function";
    case ERROR_INVALID_PARAMETER:
      return "invalid parameter (hint: check destination pointer)";
  }
  return NULL;
}

const char* SystemSnag::message()
{
  switch (_code)
  {
    case EILSEQ:
      return "system gave a wide character that could not be converted to multibyte";
    case ENOMEM:
      return "failed to allocate memory";
  }
  return NULL;
}

const char* UsageSnag::message()
{
  switch (_code)
  {
    case EILSEQ:
      return "invalid multibyte character";
    case EMSGSIZE:
      return "given name is too long";
  }
  return NULL;
}

