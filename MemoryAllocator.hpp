#pragma once
#include <vector>
#include <memory>
#include <cassert>

namespace Fc
{
    class MemoryAllocator
    {
    public:
        MemoryAllocator(std::size_t pageSize)
            : pageSize_{pageSize}
        {
            activePage_ = std::unique_ptr<std::byte[]>(new std::byte[pageSize_]);
            activePageOffset_ = activePage_.get();
            activePageFreeSize_ = pageSize_;
        }

        template <typename T, typename... Args>
        T* Emplace(Args&& ...args)
        {
            assert(sizeof(T) <= pageSize_);

            if(std::align(alignof(T), sizeof(T), activePageOffset_, activePageFreeSize_))
            {
                T* allocation{new(activePageOffset_) T{std::forward<Args>(args)...}};
                activePageOffset_ = static_cast<std::byte*>(activePageOffset_) + sizeof(T);
                activePageFreeSize_ -= sizeof(T);

                return allocation;
            }
            else
            {
                usedPages_.emplace_back(activePage_.release());

                if(!freePages_.empty())
                {
                    activePage_.reset(freePages_.back().release());
                    freePages_.pop_back();
                }
                else
                {
                    activePage_ = std::unique_ptr<std::byte[]>(new std::byte[pageSize_]);
                }
                activePageOffset_ = activePage_.get();
                activePageFreeSize_ = pageSize_;

                // -------

                T* allocation{new(activePageOffset_) T{std::forward<Args>(args)...}};
                activePageOffset_ = static_cast<std::byte*>(activePageOffset_) + sizeof(T);
                activePageFreeSize_ -= sizeof(T);

                return allocation;
            }
        }


        void Clear()
        {
            activePageOffset_ = activePage_.get();
            activePageFreeSize_ = pageSize_;

            for(auto& usedPage : usedPages_)
            {
                freePages_.emplace_back(usedPage.release());
            }
            usedPages_.clear();
        }


    private:
        std::vector<std::unique_ptr<std::byte[]>> freePages_{};
        std::vector<std::unique_ptr<std::byte[]>> usedPages_{};


        std::unique_ptr<std::byte[]> activePage_{};

        void* activePageOffset_{};
        std::size_t activePageFreeSize_{};

        std::size_t pageSize_{};
    };
}