#include <string>
#include <unistd.h>
#include <iostream>

namespace cli
{
    enum class Step
    {
        Lex,
        Parse,
        Compile,
    };

    void error_exit(std::string error, int status_code = 1)
    {
        std::cerr << error << "\n";
        exit(status_code);
    }

    struct Args
    {
        Step step;
        std::string path;

        static Args parse(int argc, char **argv)
        {
            opterr = 0;
            int c;
            Args args;
            while ((c = getopt(argc, argv, "lpc")) != -1)
            {
                std::string error;
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

int main(int argc, char **argv)
{
    cli::Args args = cli::Args::parse(argc, argv);
    std::cout << "file: " << args.path << "\n";
    return 0;
}