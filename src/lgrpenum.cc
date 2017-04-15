#if !defined(UNICODE)
#define UNICODE
#endif

#pragma comment(lib, "netapi32.lib")

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <lm.h>
#include <wchar.h>
#include "lgrplist.h"
#include "lgrperrs.h"

// -- getLocalGroupList() ----------------------------------------------------
// Fetch the name and descriptive comment of all local groups defined on this
// system, or from another host if its name is given.
// Uses API function NetLocalGroupEnum.
//
// [in] pServerNameW: name of the system that is to be queried
//      (optional - local machine is the default)
//
LocalGroupList* getLocalGroupList(const wchar_t* pServerNameW)
{
  LocalGroupList* pList = NULL;

  // NetLocalGroupEnum() arg that must persist across multiple calls
  unsigned long resumeHandle = 0;

  NET_API_STATUS status; // NetLocalGroupEnum() result code

  do {
    // NetLocalGroupEnum() args
    LPLOCALGROUP_INFO_1 pBuffer = NULL;
    unsigned long entriesReadArg = 0, totalEntriesArg = 0;
    // Note: totalEntriesArg - documentation is totally wacky on this.

    status = NetLocalGroupEnum(
      pServerNameW,
      1,
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

    if (pList == NULL) pList = new LocalGroupList();

    // A default Windows installation has certain local groups pre-defined.
    // I have not tested on a system that has no local groups defined, but I'm
    // not ready to rule out that an admin can accomplish that (though it
    // would be one weird system with no Administrators or Users group!)
    // By inference from what I've seen happen with NetLocalGroupGetMembers,
    // I assume that NetLocalGroupEnum would set pBuffer to NULL and return a
    // status of NERR_Success if no local groups exist. Therefore, we ensure
    // first that we have pList set to an (empty) instance of LocalGroupList,
    // then simply jump out of the loop:
    if (pBuffer == NULL) break;

    try { pList->Expand(entriesReadArg); }
    catch (...) {
      delete pList;
      NetApiBufferFree(pBuffer);
      throw;
    }

    LPLOCALGROUP_INFO_1 pInEntry = pBuffer; // To index into the inbound list
    unsigned long i;

    for (i = 0; i < entriesReadArg; i++, pInEntry++)
    {
      // MSDN doc does not say that lgrpi1_name can be NULL,
      // so we assume it can't.
      pList->AddGroup(pInEntry->lgrpi1_name, pInEntry->lgrpi1_comment);
    }

    NetApiBufferFree(pBuffer);

  } while (status == ERROR_MORE_DATA);

  //fprintf(stderr, "Exited normally from do-loop\n");  // DEBUG ONLY

  return pList;
}

