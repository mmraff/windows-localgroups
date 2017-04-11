#ifndef __LGRPLIST_H__
#define __LGRPLIST_H__

struct LocalGroupInfo
{
  char *name;
  char *comment;
};

class LocalGroupList
{
  static const size_t recordSz = sizeof(struct LocalGroupInfo);

  public:
    LocalGroupList() : _pList(NULL), _count(0) {}
    ~LocalGroupList();
    inline size_t Length() { return _count; }
    void Expand(size_t);
    void AddGroup(const wchar_t*, const wchar_t*);
    const struct LocalGroupInfo& operator[] (int);

  private:
    struct LocalGroupInfo* _pList;
    size_t _count;
};

LocalGroupList* getLocalGroupList(const wchar_t*);

#endif
