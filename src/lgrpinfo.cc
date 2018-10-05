#if !defined(UNICODE)
#define UNICODE
#endif

#pragma comment(lib, "netapi32.lib")

#include <stdio.h>
#include <stdlib.h>
#include <windows.h> 
#include <lm.h>
#include <wchar.h>
#include "lgapi-utils.h"
#include "lgrperrs.h"

char* getLocalGroupComment(
  const wchar_t* pGroupNameW,
  const wchar_t* pServerNameW
)
{
  char* pComment = NULL;

  NET_API_STATUS status; // Result code from NetLocalGroupGetInfo()
  LPLOCALGROUP_INFO_1 pBuffer; // NetLocalGroupGetInfo() will put results here

  status = NetLocalGroupGetInfo(
    pServerNameW,
    pGroupNameW,
    1,   // for struct LOCALGROUP_INFO_1 results
    (LPBYTE*) &pBuffer
  );

  if (status != NERR_Success) throw APIError(status);
  else
  {
    if (pBuffer && pBuffer->lgrpi1_comment)
    {
      try {
        pComment = getMultibyteStrCopy(pBuffer->lgrpi1_comment);
      }
      catch (WinLGrpsError& er) {
        NetApiBufferFree(pBuffer);
        throw er;
      }
    }

    NetApiBufferFree(pBuffer);
  }

  return pComment;
}

