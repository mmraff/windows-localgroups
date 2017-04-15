#if !defined(UNICODE)
#define UNICODE
#endif

#pragma comment(lib, "netapi32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "kernel32.lib")

#include <stdio.h>
#include <stdlib.h>
#include <windows.h> 
#include <lm.h>
#include <Sddl.h>
#include <wchar.h>
#include "lgmemblist.h"
#include "lgrperrs.h"

// -- getMemberList() --------------------------------------------------------
// Fetch data on every member of the named group from this system, or from
// another host if its name is given.
// Uses API function NetLocalGroupGetMembers.
//
// [in] pGroupNameW: name of the localgroup of interest
// [in] pServerNameW: name of the system that is to be queried
//      (optional - local machine is the default)
//
LGMemberList* getMemberList(
  const wchar_t *pGroupNameW,
  const wchar_t *pServerNameW
)
{
  LGMemberList* pList = NULL;

  // NetLocalGroupGetMembers() arg that must persist across multiple calls
  unsigned long resumeHandle = 0;

  NET_API_STATUS status; // Result code from NetLocalGroupGetMembers()

  do {
    // NetLocalGroupGetMembers() args
    LPLOCALGROUP_MEMBERS_INFO_2 pBuffer;
    unsigned long entriesReadArg = 0, totalEntriesArg = 0;

    status = NetLocalGroupGetMembers(
      pServerNameW,
      pGroupNameW,
      2,   // to get struct LOCALGROUP_MEMBERS_INFO_2 results
      (LPBYTE*) &pBuffer,
      MAX_PREFERRED_LENGTH,
      &entriesReadArg,
      &totalEntriesArg,
      &resumeHandle
    );

    if (status != NERR_Success && status != ERROR_MORE_DATA)
    {
      if (pList != NULL) delete pList;
      throw new APISnag(status);
    }

    if (pList == NULL) pList = new LGMemberList();

    // NetLocalGroupGetMembers has been seen to set pBuffer to NULL when the
    // group has no members; but then at least we have an empty LGMemberList,
    // so we will never return NULL from this function.
    if (pBuffer == NULL) break;

    try { pList->Expand(entriesReadArg); }
    catch (...) {
      delete pList;
      NetApiBufferFree(pBuffer);
      throw;
    }

    LPLOCALGROUP_MEMBERS_INFO_2 pInEntry = pBuffer; // To index into the inbound list
    Snag* pSnag = NULL;
    unsigned long i;

    for (i = 0; i < entriesReadArg; i++, pInEntry++)
    {
      wchar_t* pSidW = NULL;
      if (!ConvertSidToStringSid(pInEntry->lgrmi2_sid, &pSidW))
      {
        unsigned long err = GetLastError();
        pSnag = new APISnag(err);
        //fprintf(stderr, "ConvertSidToStringSid() failed: %d\n", err);
        break;
      }

      // FOR REFERENCE: SID_NAME_USE enum defined in winnt.h
      char* pSidUsage = NULL;
      switch (pInEntry->lgrmi2_sidusage)
      {
        case SidTypeUser:
          pSidUsage = "user"; break;
        case SidTypeGroup: 
          pSidUsage = "group"; break;
        case SidTypeWellKnownGroup:
          pSidUsage = "well-known group"; break;
        case SidTypeDeletedAccount:
          pSidUsage = "deleted"; break;
        case SidTypeUnknown:
          pSidUsage = "unknown"; break;
      }

      pList->AddMember(pSidW, pSidUsage, pInEntry->lgrmi2_domainandname);

      if (LocalFree(pSidW) != NULL)
      {
        pSnag = new SystemSnag(GetLastError());
        //fprintf(stderr, "LocalFree() had a problem...\n");
        break;
      }
    }

    NetApiBufferFree(pBuffer);

    if (pSnag)
    {
      delete pList;
      throw pSnag;
    }
  } while (status == ERROR_MORE_DATA);

  //fprintf(stderr, "Exited normally from do-loop\n");  // DEBUG ONLY

  return pList;
}
