#include "repl.h"
#include "evaluator.h"
#include <memory>
#include <string>
#include <iostream>

using namespace std;

namespace repl {

const char* MONKEY_FACE = R"(
               ',
            .-`-,\__
              ."`   `,
            .'_.  ._  `;.
        __ / `      `  `.\ .--.
       /--,| 0)   0)     )`_.-,)
      |    ;.-----.__ _-');   /
       '--./         `.`/  `"`
          :   '`      |.
          | \     /  //
           \ '---'  /'
            `------' \
             _/       `--...
                  __
)";
const string PROMPT = ">>> "; 
void start(){
    string input;
    auto environment = std::make_unique<object::Environment>();
    while(true){
        cout << PROMPT;
        getline(cin, input);
        lexer::Lexer l(input);
        auto p = parser::Parser(l);
        auto program = p.parse_program();
        if(!p.errors.empty()){
            std::cout << MONKEY_FACE << std::endl;
            std::cerr << "parser errors:" << std::endl;
            for(auto s: p.errors){
                cout << "\t" << s << std::endl;
            }
            continue;
        }
        // std::cout << program->string() << std::endl;
        auto evaluated = 
            eval::eval(std::move(program), environment);
        if(evaluated != nullptr){
            std::cout << evaluated->inspect() << std::endl;
        }

    }
}
} // namespace repl