#include "lexer.h"
#include "ast.h"

#include <iostream>

namespace kate {
    int start(int argc, char* argv[]) {
        if (argc == 1) {
            std::cerr << "Missing shader source file." << std::endl;
            return 1;
        }

        sc::ast::NodeContext ctx;
        auto func_decl = ctx.make<sc::ast::FuncDecl>(
            "Hello!"
        );

        sc::Lexer lexer;
        lexer.tokenize(argv[1]);

        return 0;
    }
}

int main(int argc, char* argv[]) {
    return kate::start(argc, argv);
}