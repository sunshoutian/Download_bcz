/*
 * Download.cpp
 * Copyright (C) 2015  <@BLUEYI-PC>
  * predict url from text file but exclude the exclude_list's url and store them in Ex_url.txt
 * you need pass two argument, first is total word list, second is you need to exclude, but the second is optional
 * Distributed under terms of the MIT license.
 */
#include <cstdlib>
#include <iostream>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <fstream>
#include <sstream>
#include <utility>
#include <cctype>
#include <algorithm>

const std::string::size_type line_length = 5;
const int total_bar = 80;
const std::vector<char> exclude_char{'*', '\?', '\"', '\\', '/', '|', ':', '<', '>'};
const std::vector<std::string> try_str{"real", "animate", "leng", "noun"};

std::string fixed_url = "http://baicizhan.qiniucdn.com";
std::string subname;
int rfind_offset = 6;

//read word and resource url from infile to vector
bool geturl(const std::string &infile, std::multimap<std::string, std::pair<std::string, std::string> > &words_url, const std::set<std::string> &exclude_list);

bool get_exclude(const std::string &infile, std::set<std::string> &exclude_list);

std::string& replace_ex(std::string &str, const std::vector<char> &exclude = exclude_char)
{
    for (const auto &ex : exclude) {
        while (str.find(ex) != std::string::npos) {
            str.replace(str.find(ex), 1, "_");
        }
    }
    return str;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        std::cout << "None input file!" << std::endl;
        exit(EXIT_FAILURE);
    }
    else {
        std::multimap<std::string, std::pair<std::string, std::string> > words_url;
        std::set<std::string> exclude_list;
        int ex_file_num_start = 2;

        for (int i = ex_file_num_start; i < argc; ++i) {
            if (!get_exclude(argv[i], exclude_list)) {
                std::cout << "get exclude list failure from file: " << argv[i] << std::endl;
                continue;
            }
        }
        if (!geturl(argv[1], words_url, exclude_list)) {
            std::cout << "get url failure from file: " << argv[1] << std::endl;
        }
        std::string try_tv_name = "Ex_url.txt";
        std::ofstream outwords(try_tv_name.c_str(), std::ofstream::out | std::ofstream::app);
        for (const auto &fword : words_url) {
            if (fword.second.first.empty()) {
                std::cout << fword.first << ": " << fword.second.second << std::endl;
                outwords << fword.first << "\t" << fword.second.second << std::endl;
            }
            else {
                std::cout << fword.first << ": " << fword.second.first << " " << fword.second.second << std::endl;
                outwords << fword.first << "\t" << fword.second.first << "\t" << fword.second.second << std::endl;
            }
        }
        outwords.close();
    }
    return 0;
}

bool geturl(const std::string &infile, std::multimap<std::string, std::pair<std::string, std::string> > &words_url, const std::set<std::string> &exclude_list)
{
    std::ifstream instream(infile.c_str());
    if (!instream) {
        std::cout << "File: " << infile << " open error!" << std::endl;
        return false;
    }
    else {
        std::string line, previous_url;
        while (getline(instream, line)) {
            if (line.length() < line_length)
              continue;
            std::string word, sentence, sub_url;

            if (line.find("\t") != std::string::npos)
              word = line.substr(0, line.find("\t"));
            else if (line.find(" ") != std::string::npos)
              word = line.substr(0, line.find(" "));
            else
              word = line;
            replace_ex(word);

            if (line.find("http://") != std::string::npos)
              rfind_offset = line.size() - line.find("http://");
            else
              rfind_offset = 6;

            if (line.find("\t") == line.rfind("\t"))
              sentence = "";
            else {
                if (line.find(".\t") == line.rfind("\t") - 1)
                  sentence = line.substr(line.find("\t") + 1, line.rfind("\t") - line.find("\t") - 1);
                else if (line.rfind(".", line.size() - rfind_offset) != std::string::npos && line.find("\t") < line.rfind(".", line.size() - rfind_offset))
                  sentence = line.substr(line.find("\t") + 1, line.rfind(".", line.size() - rfind_offset) - line.find("\t") - 1);
                else
                  sentence = line.substr(line.find("\t") + 1, line.rfind("\t") - line.find("\t") - 1);
            }
            replace_ex(sentence);

            if (line.find("\t") != std::string::npos)
              sub_url = line.substr(line.rfind("\t") + 1);
            //std::cout << "***word:" << word << "***sentence:" << sentence << "***sub_url:" << sub_url << std::endl;
            //std::cout << "***" << sentence << "***" << std::endl;
            if (!previous_url.empty() && sub_url == previous_url)
              continue;
            previous_url = sub_url;
            if (word.empty() || sub_url.empty())
              continue;
            if (sub_url.find("http://") == std::string::npos)
              sub_url = fixed_url + sub_url;

            //std::cout << "---" << sub_url << "---"  << std::endl;
            //system("pause");
            if (exclude_list.find(sub_url) == exclude_list.end())
              words_url.insert(std::make_pair(word, std::make_pair(sentence, sub_url)));
        }
    }
    instream.close();
    return true;
}

bool get_exclude(const std::string &infile, std::set<std::string> &exclude_list)
{
    std::ifstream instream(infile.c_str());
    if (!instream) {
        std::cout << "File: " << infile << " open error!" << std::endl;
        return false;
    }
    else {
        std::string line;
        while (getline(instream, line)) {
            if (line.length() < line_length)
              continue;
            std::string url;
            if (line.find("\t") != std::string::npos)
              url = line.substr(line.rfind("\t") + 1);
            else if (line.find(" ") != std::string::npos)
              url = line.substr(line.find(" ") + 1);
            else
              url = line;
            //std::cout << "****" << url << "****"  << std::endl;
            //system("pause");
            exclude_list.insert(url);
        }
    }
    instream.close();
    return true;
}
