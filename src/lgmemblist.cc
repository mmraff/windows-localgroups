#include <stdlib.h>
#include <malloc.h> // TODO: any redundancy here? For malloc & free,
                    // docs say "Required header: <stdlib.h> and <malloc.h>"
#include <memory.h>
#include <assert.h>
#include <errno.h>
#include "lgmemblist.h"
#include "lgrperrs.h"
#include "lgapi-utils.h"

LGMemberList::~LGMemberList()
{
  if (_pList != NULL)
  {
    for (size_t i = 0; i < _count; i++) {
      free(_pList[i].sid);
      free(_pList[i].domainAndName);
    }
    free(_pList);
  }
}

void LGMemberList::Expand(size_t addCount)
{
  assert(0 < addCount && "Invalid value to Expand()");

  if (_pList == NULL) // This is the initial allocation of memory
  {
    size_t sz = addCount * LGMemberList::recordSz;
    _pList = (struct LocGrpMemberInfo*)malloc(sz);
    if (_pList == NULL) throw SysError(ENOMEM);

    memset(_pList, NULL, sz);
  }
  else // This is not the first Expand() call
  {
    size_t diffSize = addCount * LGMemberList::recordSz;
    size_t newSize = (_count + addCount) * LGMemberList::recordSz;
    struct LocGrpMemberInfo* p = (struct LocGrpMemberInfo*)realloc(_pList, newSize);
    if (p == NULL) throw SysError(ENOMEM);

    _pList = p;
    memset(_pList + _count, NULL, diffSize);
  }
}

void LGMemberList::AddMember(
  const wchar_t* pSIDW,
  const char* pType,
  const wchar_t* pNameW
)
{
  LocGrpMemberInfo* pCurr = _pList + _count;

  pCurr->sid = getMultibyteStrCopy(pSIDW);
  pCurr->accountType = (char*)pType;
  pCurr->domainAndName = getMultibyteStrCopy(pNameW);

  _count++;
}

const struct LocGrpMemberInfo& LGMemberList::operator[] (int index)
{
  assert(0 <= index && index < (int)_count && "LGMemberList[]: invalid index");
  assert(_pList != NULL && "LGMemberList[]: count is set but pList is NULL!");

  return _pList[index];
}

