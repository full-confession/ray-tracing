#pragma once
#include "../core/allocator.hpp"

#include <memory>
#include <vector>

namespace fc
{
    class paged_allocator : public allocator
    {
    public:
        explicit paged_allocator(std::size_t default_page_size)
            : default_page_size_{default_page_size}
        { }

        std::byte* allocate(std::size_t count)
        {
            count = (count + 15) & (~15);

            if(active_page_offset_ + count > active_page_.size())
            {
                active_page_offset_ = 0;
                if(active_page_.size() != 0)
                {
                    used_pages_.push_back(std::move(active_page_));
                }


                std::size_t free_page_index{};
                for(; free_page_index < free_pages_.size(); ++free_page_index)
                {
                    if(free_pages_[free_page_index].size() >= count)
                    {
                        break;
                    }
                }

                if(free_page_index != free_pages_.size())
                {
                    active_page_ = std::move(free_pages_[free_page_index]);
                    std::swap(free_pages_.back(), free_pages_[free_page_index]);
                    free_pages_.pop_back();
                }
                else
                {
                    active_page_ = page{std::max(count, default_page_size_)};
                }
            }

            std::byte* p{active_page_.bytes() + active_page_offset_};
            active_page_offset_ += count;
            return p;
        }

        void clear()
        {
            active_page_offset_ = 0;
            if(active_page_.size() != 0)
            {
                free_pages_.push_back(std::move(active_page_));
            }

            for(auto& used_page : used_pages_)
            {
                free_pages_.push_back(std::move(used_page));
            }
            used_pages_.clear();
        }
    private:
        std::size_t default_page_size_{};

        class page
        {
        public:
            page() = default;
            explicit page(std::size_t size) : size_{size}, bytes_{std::make_unique<std::byte[]>(size)}
            { }

            page(page&& other) noexcept : size_{other.size_}, bytes_{std::move(other.bytes_)}
            {
                other.size_ = 0;
            }

            page& operator=(page&& other) noexcept
            {
                size_ = other.size_;
                other.size_ = 0;
                bytes_ = std::move(other.bytes_);
                return *this;
            }

            std::size_t size() const { return size_; }
            std::byte* bytes() { return bytes_.get(); }

        private:
            std::size_t size_{};
            std::unique_ptr<std::byte[]> bytes_{};
        };

        std::vector<page> used_pages_{};
        std::vector<page> free_pages_{};

        page active_page_{};
        std::size_t active_page_offset_{};
    };
}