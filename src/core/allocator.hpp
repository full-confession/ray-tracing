#pragma once
#include <memory>
#include <vector>

namespace Fc
{
    class Allocator
    {
    public:
        explicit Allocator(std::size_t defaultPageSize)
            : defaultPageSize_{defaultPageSize}
        { }


        std::byte* Allocate(std::size_t count)
        {
            count = (count + 15) & (~15);

            if(activePageOffset_ + count > activePage_.Size())
            {
                activePageOffset_ = 0;
                if(activePage_.Size() != 0)
                {
                    usedPages_.push_back(std::move(activePage_));
                }


                std::size_t freePageIndex{};
                for(; freePageIndex < freePages_.size(); ++freePageIndex)
                {
                    if(freePages_[freePageIndex].Size() >= count)
                    {
                        break;
                    }
                }

                if(freePageIndex != freePages_.size())
                {
                    activePage_ = std::move(freePages_[freePageIndex]);
                    std::swap(freePages_.back(), freePages_[freePageIndex]);
                    freePages_.pop_back();
                }
                else
                {
                    activePage_ = Page{std::max(count, defaultPageSize_)};
                }
            }

            std::byte* p{activePage_.Bytes() + activePageOffset_};
            activePageOffset_ += count;
            return p;
        }

        /*template <typename T>
        T* Allocate()
        {
            std::byte* bytes{Allocate(sizeof(T))};
            return reinterpret_cast<T*>(bytes);
        }

        template <typename T, std::enable_if_t<std::is_array_v<T>&& std::extent_v<T> == 0, bool> = true>
        std::remove_extent_t<T>* Allocate(std::size_t size)
        {
            std::byte* bytes{Allocate(sizeof(std::remove_extent_t<T>) * size)};
            return reinterpret_cast<std::remove_extent_t<T>*>(bytes);
        }*/

        template <typename T, typename... Args>
        T* Emplace(Args&& ...args)
        {
            std::byte* bytes{Allocate(sizeof(T))};
            T* object{new(bytes) T{std::forward<Args>(args)...}};
            return object;
        }


        void Clear()
        {
            activePageOffset_ = 0;
            if(activePage_.Size() != 0)
            {
                freePages_.push_back(std::move(activePage_));
            }

            for(auto& usedPage : usedPages_)
            {
                freePages_.emplace_back(std::move(usedPage));
            }
            usedPages_.clear();
        }


    private:
        std::size_t defaultPageSize_{};
        class Page
        {
        public:
            Page() = default;
            explicit Page(std::size_t size) : size_{size}, memory_{std::make_unique<std::byte[]>(size)}
            { }

            Page(Page&& other) noexcept : size_{other.size_}, memory_{std::move(other.memory_)}
            {
                other.size_ = 0;
            }

            Page& operator=(Page&& other) noexcept
            {
                size_ = other.size_;
                other.size_ = 0;
                memory_ = std::move(other.memory_);
                return *this;
            }

            std::size_t Size() const { return size_; }
            std::byte* Bytes() { return memory_.get(); }

        private:
            std::size_t size_{};
            std::unique_ptr<std::byte[]> memory_{};
        };

        std::vector<Page> usedPages_{};
        std::vector<Page> freePages_{};

        Page activePage_{};
        std::size_t activePageOffset_{};
    };
}