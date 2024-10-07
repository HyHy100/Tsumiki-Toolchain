#include "parser.h"

namespace kate::sc {
    void Parser::parse(const std::string_view& source) 
    {
        m_lexer.tokenize(source);

        
    }

    template<typename T>
    Result<T>::Result(Failure failure) {
        if (failure == Failure::kNoMatch) {
            errored = false;
            matched = false;
        } else {
            errored = true;
            matched = false;
        }
    }

    template<typename T>
    template<typename U>
    Result<T>::Result(Result<U>&& rhs) 
    {
        value = std::move(rhs.value);
        matched = rhs.matched;
        errored = rhs.errored;
    }

    template<typename T>
    Result<T>::Result() : 
        errored{ false }, 
        matched { false }
    {
    }

    template<typename T>
    Result<T>::Result(T ptr) : 
        errored { false }, 
        matched { true }, 
        value { ptr } 
    {
    }

    template<typename T>
    Result<T>::Result(bool _erroed, bool _matched) : 
        errored { _erroed }, 
        matched { _matched }
    {
    }

    template<typename T>
    Result<T>::Result(bool _erroed, bool _matched, T value) : 
        errored { _erroed }, 
        matched { _matched }, 
        value { std::move(value) }
    {
    }

    template<typename T>
    T&& Result<T>::unwrap() 
    {
        return std::move(value);
    }

    template<typename T>
    T& Result<T>::operator->()
    {
        return value;
    }

    template<typename T>
    bool Result<T>::ok()
    {
        return !errored;
    }
}