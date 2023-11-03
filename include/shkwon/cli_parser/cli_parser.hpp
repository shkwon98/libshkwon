#include <getopt.h>
#include <iostream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace shkwon
{
class CliParser
{
public:
    CliParser(void)
        : name_("unknown")
        , usage_guide_("Usage:\n")
    {
    }
    CliParser(std::string name)
        : name_(name)
        , usage_guide_("Usage:\n")
    {
    }
    ~CliParser()
    {
    }

    CliParser &AddOption(const char &short_option, const char *long_option, std::string &dest, const std::string &usage)
    {
        short_options_ += short_option;
        short_options_ += ':';

        struct option opt = {long_option, required_argument, nullptr, short_option};
        long_options_.push_back(opt);

        usage_guide_ += usage;
        usage_guide_ += "\n";

        option_map_[short_option] = {opt, &dest, nullptr};

        return *this;
    }
    CliParser &AddOption(const char &short_option, const char *long_option, bool &dest, const std::string usage)
    {
        short_options_ += short_option;

        struct option opt = {long_option, no_argument, nullptr, short_option};
        long_options_.push_back(opt);

        usage_guide_ += usage;
        usage_guide_ += "\n";

        option_map_[short_option] = {opt, nullptr, &dest};

        return *this;
    }

    void Parse(int argc, char **argv)
    {
        auto help_flag = false;
        this->AddOption('h', "help", help_flag, "-h --help  // print USAGE");
        auto next_option = 0;

        do
        {
            next_option = getopt_long(argc, argv, short_options_.c_str(), long_options_.data(), nullptr);

            if (next_option != -1)
            {
                auto it = option_map_.find(next_option);
                if (it != option_map_.end())
                {
                    if (it->first == 'h')
                    {
                        std::cout << usage_guide_ << std::endl;
                        std::exit(0);
                    }
                    if (std::get<0>(it->second).has_arg == required_argument)
                    {
                        *(std::string *)std::get<1>(it->second) = optarg;
                    }
                    else
                    {
                        *(bool *)std::get<2>(it->second) = true;
                    }

                    continue;
                }

                std::cerr << "Invalid option (" << next_option << "," << std::hex << next_option << ")" << std::endl;
                throw std::runtime_error("Invalid option.");
            }

        } while (next_option != -1);
    }

private:
    std::string name_;
    std::string short_options_;
    std::vector<struct option> long_options_;
    std::string usage_guide_;
    std::unordered_map<char, std::tuple<struct option, std::string *, bool *>> option_map_;
};
} // namespace shkwon