#pragma once

#include <type_traits>

namespace base {
    template<typename Type, typename BitMaskType = std::underlying_type_t<Type>>    
    class Flags {
    public:
        Flags(BitMaskType v)
            : m_bitmask { v }
        {
        }

        Flags(Type v)
            : m_bitmask { static_cast<BitMaskType>(v) }
        {
        }

        operator Type() const
        {
            return static_cast<Type>(m_bitmask);
        }

        operator BitMaskType() const
        {
            return m_bitmask;
        }

        Flags<Type>& operator&(const Flags<Type>& rhs) const
        {
            return m_bitmask & rhs;
        }

        Flags<Type>& operator|(const Flags<Type>& rhs) const
        {
            return m_bitmask | rhs;
        }

        void operator|=(const Flags<Type>& rhs)
        {
            m_bitmask |= rhs.m_bitmask;
        }

        void operator&=(const Flags<Type>& rhs)
        {
            m_bitmask &= rhs.m_bitmask;
        }
    private:
        BitMaskType m_bitmask;
    };
}