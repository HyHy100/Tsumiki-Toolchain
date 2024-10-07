#pragma once

#include "lexer.h"

namespace kate::sc {
    enum class Failure {
        kNoMatch,
        kError
    };

    template<typename T>
    struct Result {
        Result(Failure failure);

        template<typename U>
        Result(Result<U>&& rhs);

        Result();
        
        Result(T ptr);

        Result(bool _erroed, bool _matched);

        Result(bool _erroed, bool _matched, T value);

        T&& unwrap();

        T& operator->();

        bool ok();

        bool errored;
        bool matched;
        T value;
    };

    class Parser {
    public:
        Parser() = default;

        void parse(const std::string_view& source);
    private:
        Lexer m_lexer;
    };
}