#pragma once
#include <cstdint>
#include <algorithm>

namespace Fc
{
    static constexpr float FLOAT_ONE_MINUS_EPSILON{0x1.fffffep-1f};

    class Random
    {
        static constexpr uint64_t DEFAULT_STATE{0x853c49e6748fea9b};
        static constexpr uint64_t DEFAULT_STREAM{0xda3e39cb94b95bdb};

    public:
        Random() = default;
        Random(uint64_t sequenceIndex)
        {
            SetSequence(sequenceIndex);
        }

        void SetSequence(uint64_t sequenceIndex)
        {
            state_ = 0U;
            inc_ = (sequenceIndex << 1u) | 1u;
            UniformUInt32();
            state_ += DEFAULT_STATE;
            UniformUInt32();
        }

        uint32_t UniformUInt32()
        {
            uint64_t oldstate{state_};
            state_ = oldstate * 6364136223846793005ULL + (inc_ | 1);
            uint32_t xorshifted{static_cast<uint32_t>(((oldstate >> 18u) ^ oldstate) >> 27u)};
            uint32_t rot{static_cast<uint32_t>(oldstate >> 59u)};
            return (xorshifted >> rot) | (xorshifted << ((~rot + 1u) & 31));
        }

        uint32_t UniformUInt32(uint32_t b)
        {
            uint32_t value{};
            uint32_t threshold{0xFFFF'FFFF - 0xFFFF'FFFF % b};
            do
            {
                value = UniformUInt32();
            } while(value >= threshold);

            return value % b;
        }

        float UniformFloat()
        {
            return std::min(FLOAT_ONE_MINUS_EPSILON, UniformUInt32() * 0x1p-32f);
        }

    private:
        uint64_t state_{DEFAULT_STATE};
        uint64_t inc_{DEFAULT_STREAM};
    };
}