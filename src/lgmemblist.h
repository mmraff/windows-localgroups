#ifndef __LGMEMBLIST_H__
#define __LGMEMBLIST_H__

struct LocGrpMemberInfo
{
  char *sid;
  char *accountType;
  char *domainAndName;
};

class LGMemberList
{
  static const size_t recordSz = sizeof(struct LocGrpMemberInfo);

  public:
    LGMemberList() : _pList(NULL), _count(0) {}
    ~LGMemberList();
    inline size_t Length() { return _count; }
    void Expand(size_t);
    void AddMember(const wchar_t*, const char*, const wchar_t*);
    const struct LocGrpMemberInfo& operator[] (int);

  private:
    struct LocGrpMemberInfo* _pList;
    size_t _count;
};

LGMemberList* getMemberList(const wchar_t*, const wchar_t*);

#endif

