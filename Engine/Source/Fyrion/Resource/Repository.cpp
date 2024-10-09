#include "Repository.hpp"

#include <mutex>

#include "Fyrion/Core/UUID.hpp"


#define PAGE(value)    u32((value)/FY_REPO_PAGE_SIZE)
#define OFFSET(value)  (u32)((value) & (FY_REPO_PAGE_SIZE - 1))

namespace Fyrion
{
    struct ResourceStorage
    {
        RID              rid{};
        UUID             uuid{};
        std::atomic<u32> version = 1;
    };

    struct ResourcePage
    {
        ResourceStorage elements[FY_REPO_PAGE_SIZE];
    };

    namespace
    {
        Allocator&    allocator = MemoryGlobals::GetDefaultAllocator();
        ResourcePage* pages[FY_REPO_PAGE_SIZE]{};
        std::mutex    pageMutex{};
        usize         pageCount{};

        ResourceStorage* GetOrAllocate(RID rid)
        {
            if (pages[rid.page] == nullptr)
            {
                std::unique_lock lock(pageMutex);
                if (pages[rid.page] == nullptr)
                {
                    pages[rid.page] = static_cast<ResourcePage*>(allocator.MemAlloc(sizeof(ResourcePage), alignof(ResourcePage)));
                    pageCount++;
                }
            }
            return &pages[rid.page]->elements[rid.offset];
        }
    }


    RID Repository::Create(TypeID typeId)
    {
        return {};
    }

    ConstPtr Repository::Read(RID rid)
    {
        return nullptr;
    }

    void Repository::Commit(RID rid, ConstPtr value) {}
}
