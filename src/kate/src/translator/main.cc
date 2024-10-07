#include "lexer.h"
#include "ast.h"

#include <iostream>

namespace kate::sc {
    int start(int argc, char* argv[]) {
        if (argc == 1) {
            std::cerr << "Missing shader source file." << std::endl;
            return 1;
        }

        auto func_decl = ast::context().make<sc::ast::FuncDecl>(
            "Hello!"
        );

        sc::Lexer lexer;
        lexer.tokenize(argv[1]);

        return 0;
    }
}

int main(int argc, char* argv[]) {
    return kate::sc::start(argc, argv);
}