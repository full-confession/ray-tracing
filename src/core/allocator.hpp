#pragma once
#include <cstddef>

namespace fc
{
    class allocator
    {
    public:
        virtual ~allocator() = default;

        virtual std::byte* allocate(std::size_t count) = 0;
        virtual void clear() = 0;
    };

    class allocator_wrapper
    {
    public:
        explicit allocator_wrapper(allocator* allocator)
            : allocator_{allocator}
        { }

        std::byte* allocate(std::size_t count)
        {
            return allocator_->allocate(count);
        }

        template<typename T, typename... Args>
        T* emplace(Args&& ...args)
        {
            std::byte* bytes{allocator_->allocate(sizeof(T))};
            T* object{new(bytes) T{std::forward<Args>(args)...}};
            return object;
        }

        void clear()
        {
            allocator_->clear();
        }

    private:
        allocator* allocator_{};
    };
}