#pragma once

#include "includer.h"

class name_mask_filter
{
private:
    std::vector<std::regex> masks;

    static std::regex glob_to_regex(const std::string& mask)
    {
        std::string pattern = "^";
        for (char c : mask)
        {
            switch (c)
            {
                case '*': pattern += ".*"; break;
                case '?': pattern += '.';  break;
                case '.': case '\\': case '^': case '$': case '+':
                case '(': case ')': case '[': case ']': case '{':
                case '}': case '|':
                    pattern += '\\';
                    pattern += c;
                    break;
                default:
                    pattern += c;
            }
        }
        pattern += "$";
        return std::regex(pattern, std::regex::icase); 
    }

public:
    name_mask_filter() = default;

    void add_mask(const std::string& mask)
    {
        masks.push_back(glob_to_regex(mask));
    }

    bool passes(const std::string& filename) const
    {
        if (masks.empty()) return true; 
        for (const auto& re : masks)
        {
            if (std::regex_match(filename, re))
                return true;
        }
        return false;
    }
};