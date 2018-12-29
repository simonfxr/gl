#define REF_CONCURRENT

#include "data/Ref.hpp"

typedef Ref<defs::uptr> URef;
typedef WeakRef<defs::uptr> WURef;

bool
to_ref(WURef &wref, URef *ref)
{
    return wref.strong(ref);
}

WURef
weak_ref(URef &r)
{
    return WURef(r);
}

void
del_weak(WURef *r)
{
    delete r;
}

void
del_strong(URef *r)
{
    delete r;
}

int
main()
{
    return 0;
}
