#pragma once
#include "../core/allocator.hpp"

#include <vector>
#include <exception>

namespace fc
{
    class fixed_size_allocator : public allocator
    {
    public:
        explicit fixed_size_allocator(std::size_t size)
            : bytes_{size}
        { }

        virtual std::byte* allocate(std::size_t count) override
        {
            count = (count + 15) & (~15);
            if(offset_ + count > bytes_.size())
            {
                throw std::bad_alloc{};
            }

            std::byte* p{&bytes_[offset_]};
            offset_ += count;
            return p;
        }

        virtual void clear() override
        {
            offset_ = 0;
        }

    private:
        std::vector<std::byte> bytes_{};
        std::size_t offset_{};
    };
}