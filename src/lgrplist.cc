#include <stdlib.h>
#include <malloc.h> // TODO: any redundancy here? For malloc & free,
                    // docs say "Required header: <stdlib.h> and <malloc.h>"
#include <memory.h>
#include <assert.h>
#include <errno.h>
#include "lgrplist.h"
#include "lgrperrs.h"
#include "lgapi-utils.h"

LocalGroupList::~LocalGroupList()
{
  if (_pList != NULL) {
    for (size_t i = 0; i < _count; i++) {
      free(_pList[i].name);
      if (_pList[i].comment != NULL) free(_pList[i].comment);
    }
    free(_pList);
  }
}

void LocalGroupList::Expand(size_t addCount)
{
  assert(0 < addCount && "Invalid value to Expand()");

  if (_pList == NULL) // This is the initial allocation of memory
  {
    size_t sz = addCount * LocalGroupList::recordSz;
    _pList = (struct LocalGroupInfo*)malloc(sz);
    if (_pList == NULL) throw new SystemSnag(ENOMEM);

    memset(_pList, NULL, sz);
  }
  else // This is not the first Expand() call
  {
    size_t diffSize = addCount * LocalGroupList::recordSz;
    size_t newSize = (_count + addCount) * LocalGroupList::recordSz;
    struct LocalGroupInfo* p = (struct LocalGroupInfo*)realloc(_pList, newSize);
    if (p == NULL) throw new SystemSnag(ENOMEM);

    _pList = p;
    memset(_pList + _count, NULL, diffSize);
  }
}

void LocalGroupList::AddGroup(const wchar_t* pNameW, const wchar_t* pCommentW)
{
  LocalGroupInfo* pCurr = _pList + _count;

  pCurr->name = getMultibyteStrCopy(pNameW);
  if (pCommentW) pCurr->comment = getMultibyteStrCopy(pCommentW);

  _count++;
}

const struct LocalGroupInfo& LocalGroupList::operator[] (int index)
{
  assert(0 <= index && index < (int)_count && "LocalGroupList[]: invalid index");
  assert(_pList != NULL && "LocalGroupList[]: count is set but pList is NULL!");

  return _pList[index];
}

