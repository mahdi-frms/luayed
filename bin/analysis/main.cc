#include <string>
#include <unistd.h>
#include <iostream>
#include <lyddef.h>

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
        std::cerr << error << "\n";
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

void command_read_file(string path)
{
    std::cout << read_file(path);
}

void command_lex_file(string code)
{
}

void cammand_parse_file(string code)
{
}

void cammand_compile_file(string code)
{
}

int main(int argc, char **argv)
{
    cli::Args args = cli::Args::parse(argc, argv);

    if (args.step == cli::Step::Lex)
        command_lex_file(args.path);
    else if (args.step == cli::Step::Parse)
        cammand_parse_file(args.path);
    else if (args.step == cli::Step::Compile)
        cammand_compile_file(args.path);
    else
        command_read_file(args.path);

    return 0;
}