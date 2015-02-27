/*
 * Copyright (c) 2015 Falltergeist developers
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

// C++ standard includes
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// ini2json includes

// Third party includes

std::string tab(unsigned level)
{
    std::string result;

    for (unsigned int i = 0; i != level; ++i)
    {
        result += "    ";
    }

    return result;
}

std::vector<std::string> explode(const std::string& text, const char symbol)
{
    std::string line = text;
    std::vector<std::string> lines;

    while(line.find(symbol) != std::string::npos)
    {
        lines.push_back(line.substr(0, line.find(symbol)));
        line = line.substr(line.find(symbol) + 1);
    }

    lines.push_back(line);
    return lines;
}

std::string trim(const std::string& line, const std::string& symbols = " \n\r\t")
{
    auto begin = line.find_first_not_of(symbols);
    auto end = line.find_last_not_of(symbols);

    // empty string
    if (begin == std::string::npos) return "";

    return line.substr(begin, end - begin + 1);
}

void usage(std::string binaryName)
{
    std::cout << "INI to JSON converter v0.0.1" << std::endl;
    std::cout << "Usage: " << binaryName << " <INI filename>" << std::endl;
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        usage(argv[0]);
        return 1;
    }

    std::string filename = argv[1];
    std::ifstream in;

    in.open(filename.c_str(), std::ios_base::binary | std::ios_base::in);
    if (!in.is_open())
    {
        std::cout << "Can't open file: " << filename << std::endl;
        return 1;
    }

    std::string line;
    bool sectionOpened = false;
    bool hasAttributes = false;


    std::cout << "{" << std::endl;

    while(std::getline(in, line))
    {
        // remove comment
        if (line.find(';') != std::string::npos)
        {
            line = line.substr(0, line.find(';'));
        }

        // trim spaces
        line = trim(line);

        // skip empty lines
        if (line.length() == 0) continue;

        if (line[0] == '[') // section name
        {
            line = trim(line, "[ ]");
            if (hasAttributes)
            {
                hasAttributes = false;
                std::cout << std::endl;
            }

            if (sectionOpened)
            {
                std::cout << tab(1) << "}," << std::endl;
            }
            else
            {
                sectionOpened = true;
            }

            std::cout << tab(1) << "\"" << line << "\": {" << std::endl;

        }
        else
        {
            // corrupted attributes line
            if (line.find('=') == std::string::npos) continue;

            std::string attribute = line.substr(0, line.find('='));
            std::string value = line.substr(line.find('=') + 1);

            if (hasAttributes)
            {
                std::cout << "," << std::endl;
            }
            else
            {
                hasAttributes = true;
            }

            std::cout << tab(3) << "\"" << attribute << "\": ";

            // associative array
            if (value.find(':') != std::string::npos)
            {
                std::cout << "{" << std::endl;
                auto items = explode(value, ',');
                for (unsigned i = 0; i != items.size(); ++i)
                {
                    auto item = items.at(i);
                    auto key = item.substr(0, item.find(':'));
                    auto value = item.substr(item.find(':') + 1);
                    std::cout << tab(4) << "\"" << trim(key) << "\": \"" << trim(value) << "\"";
                    if (i < items.size() - 1)
                    {
                        std::cout << ",";
                    }

                    std::cout << std::endl;
                }
                std::cout << tab(3) << "}";
            }
            // array
            else if (value.find(',') != std::string::npos)
            {
                std::cout << "[" << std::endl;
                auto items = explode(value, ',');
                for (unsigned i = 0; i != items.size(); ++i)
                {
                    auto item = items.at(i);
                    std::cout << tab(4) << "\"" << trim(item) << "\"";

                    if (i < items.size() - 1)
                    {
                        std::cout << ",";
                    }

                    std::cout << std::endl;
                }


                std::cout << tab(3) << "]";
            }
            // simple value
            else
            {
                std::cout << "\"" << value << "\"";
            }

        }
    }

    if (hasAttributes)
    {
        std::cout << std::endl;
    }

    if (sectionOpened)
    {
        std::cout << tab(1) << "}" << std::endl;
    }
    std::cout << "}" << std::endl;

    in.close();
    return 0;
}
