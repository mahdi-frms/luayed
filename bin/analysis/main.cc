#include <string>
#include <unistd.h>
#include <iostream>
#include <lexer.h>
#include <parser.h>
#include <lstrep.h>
#include <resolve.h>
#include <compiler.h>
#include "generator.h"

namespace cli
{
    enum class Step
    {
        None,
        Lex,
        Parse,
        Compile,
    };

    void error_exit(string error, int status_code = 1)
    {
        std::cerr << "error: " << error << "\n";
        exit(status_code);
    }

    struct Args
    {
        Step step = Step::None;
        string path;

        static Args parse(int argc, char **argv)
        {
            opterr = 0;
            int c;
            Args args;
            while ((c = getopt(argc, argv, "lpc")) != -1)
            {
                string error;
                switch (c)
                {
                case 'l':
                    args.step = Step::Lex;
                    break;

                case 'p':
                    args.step = Step::Parse;
                    break;

                case 'c':
                    args.step = Step::Compile;
                    break;

                case '?':
                    error.append("unknown option");
                    error.append(" '");
                    error.push_back(optopt);
                    error.push_back('\'');
                    error_exit(error);
                    break;

                default:
                    exit(1);
                }
            }
            if (optind >= argc)
                error_exit("no path provided");
            args.path = argv[optind];
            return args;
        }
    };
}

string read_file(string path)
{
    string str = "";
    int bsize = 1024;
    FILE *file = fopen(path.c_str(), "r");
    if (!file)
        cli::error_exit(string("failed to open file '") + path + string("'"));
    char buffer[bsize + 1];
    while (true)
    {
        int rsl = fread(buffer, 1, bsize, file);
        buffer[rsl] = '\0';
        str += string(buffer);
        if (rsl < bsize)
            break;
    }
    fclose(file);
    return str;
}

ast::Ast parse_file(string &code)
{
    Lexer lexer(code.c_str());
    Parser parser(&lexer);
    ast::Ast tree = parser.parse();
    if (!tree.root())
        std::cerr << parser.get_error() << "\n";
    return tree;
}

void command_read_file(string path)
{
    std::cout << read_file(path);
}

void command_lex_file(string path)
{
    string code = read_file(path);
    Lexer lexer(code.c_str());
    vector<Token> tokens = lexer.drain();
    for (size_t i = 0; i < tokens.size(); i++)
        std::cout << tokens[i] << "\n";
}

void command_parse_file(string path)
{
    string code = read_file(path);
    ast::Ast tree = parse_file(code);
    if (!tree.root())
        exit(1);
    std::cout << tree.root();
}

void command_compile_file(string path)
{
    string code = read_file(path);
    ast::Ast tree = parse_file(code);
    if (!tree.root())
        exit(1);
    Resolver resolver(tree);
    vector<Lerror> errors = resolver.analyze();
    if (errors.size())
    {
        for (size_t i = 0; i < errors.size(); i++)
            std::cerr << errors[i];
        exit(1);
    }
    AnalysisGenerator gen;
    Compiler compiler(&gen);
    compiler.compile(tree, path.c_str());
    std::cout << gen.stringify();
}

int main(int argc, char **argv)
{
    cli::Args args = cli::Args::parse(argc, argv);

    if (args.step == cli::Step::Lex)
        command_lex_file(args.path);
    else if (args.step == cli::Step::Parse)
        command_parse_file(args.path);
    else if (args.step == cli::Step::Compile)
        command_compile_file(args.path);
    else
        command_read_file(args.path);

    return 0;
}