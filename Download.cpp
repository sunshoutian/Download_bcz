/*
 * Download.cpp
 * Copyright (C) 2015  <@BLUEYI-PC>
 * batch download resource file from text file
 * Distributed under terms of the MIT license.
 */

//#include "Download.h"
#include <cstdlib>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <utility>

const std::string::size_type line_length = 5;
const int total_bar = 80;
const std::vector<char> exclude_char{'*', '\?', '\"', '\\', '/', '|', ':', '<', '>'};

std::string fixed_url = "http://baicizhan.qiniucdn.com";
std::string subname;
int rfind_offset = 6;

//read word and resource url from infile to vector
bool geturl(const std::string &infile, std::multimap<std::string, std::pair<std::string, std::string> > &words_url);

//use wget to download url of words_url to directory dir
bool down(const std::string dir, const std::multimap<std::string, std::pair<std::string, std::string> > &words_url, std::multimap<std::string, std::pair<std::string, std::string> > &failed_words, std::string cur_file);

std::string& replace_ex(std::string &str, const std::vector<char> &exclude = exclude_char)
{
    for (const auto &ex : exclude) {
        while (str.find(ex) != std::string::npos) {
            str.replace(str.find(ex), 1, "_");
        }
    }
    return str;
}

void show_progress(int cur, int total, std::string cur_file)
{
    double rate = static_cast<double>(cur) / static_cast<double>(total);
    int num_equal = rate * total_bar;
    std::cout << "***Downloading " << cur_file << "***" << std::endl << std::endl;
    std::string progress(num_equal, '+');
    std::cout << "Total:[" << progress << "=>" << cur << "/" << total << "]" << std::endl << std::endl;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        std::cout << "None input file!" << std::endl;
        exit(EXIT_FAILURE);
    }
    else {
        std::multimap<std::string, std::pair<std::string, std::string> > words_url, failed_words;
        int file_num_start = 1;
        std::string temp1 = argv[1];

        if (temp1 == "-f") {
            std::string http_temp = argv[2];
            if (http_temp.substr(0, 6) != "http://") {
                std::cout << "There must be an correct url following of \'-f\'" << std::endl;
                exit(EXIT_FAILURE);
            }
            fixed_url = http_temp;
            file_num_start = 3;
        }
        else if (temp1 == "-d") {
            fixed_url.clear();
            file_num_start = 2;
        }

        for (int i = file_num_start; i < argc; ++i) {
            words_url.clear();
            if (!geturl(argv[i], words_url)) {
                std::cout << "get url failure from file: " << argv[i] << std::endl;
                continue;
            }
            std::string dir = argv[i];
            if (dir.find(".") != std::string::npos)
                dir = dir.substr(0, dir.find("."));
            std::string dir_command = "md " + dir;
            subname = dir.substr(0, dir.find("_"));
            if(system(dir_command.c_str()) != 0)
                std::cout << "Directory: " << dir << " created error!" << std::endl;

            bool is_down_ok = down(dir, words_url, failed_words, argv[i]);
            if (is_down_ok)
                std::cout << "All of words download success and stored in \n" << "***" << dir << "***" << std::endl;
            else {
                std::cout << std::endl;
                std::cout << "***The following words downloaded failed!***" << std::endl;
                std::string error_fname = "Error_" + dir + ".txt";
                std::ofstream outwords(error_fname.c_str(), std::ofstream::out | std::ofstream::app);
                for (const auto &fword : failed_words) {
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
        }
    }
    return 0;
}

bool geturl(const std::string &infile, std::multimap<std::string, std::pair<std::string, std::string> > &words_url)
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
            word = line.substr(0, line.find("\t"));
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
            sub_url = line.substr(line.rfind("\t") + 1);
            //std::cout << "***word:" << word << "***sentence:" << sentence << "***sub_url:" << sub_url << std::endl;
            //std::cout << "***" << sentence << "***" << std::endl;
            if (sub_url == previous_url)
                continue;
            previous_url = sub_url;
            if (word.empty() || sub_url.empty())
                continue;
            if (sub_url.find("http://") == std::string::npos)
                sub_url = fixed_url + sub_url;
            //std::cout << "****sub_url: " << sub_url << std::endl;
            words_url.insert(std::make_pair(word, std::make_pair(sentence, sub_url)));
        }
    }
    instream.close();
    return true;
}

bool down(const std::string dir, const std::multimap<std::string, std::pair<std::string, std::string> > &words_url, std::multimap<std::string, std::pair<std::string, std::string> > &failed_words, std::string cur_file)
{
    bool is_all_good = true;
    int cur = 0;
    int total_words = words_url.size();
    for (const auto &word_url : words_url) {
        cur++;
        std::string down_command = "wget -c -T 120 ";
        std::string tword, tsentence, turl, suffix, file_name;
        tword = word_url.first;
        tsentence = word_url.second.first;
        turl = word_url.second.second;
        suffix = turl.substr(turl.rfind("."));
        if (tsentence.empty())
            file_name = tword + "_" + subname + suffix;
        else
            file_name = tword + "_" + tsentence  + "_" + subname + suffix;
        down_command = down_command + "\"" + turl + "\"" + " -O " + "\"./" + dir + "/" + file_name + "\"";
        //std::cout << "****down_command: " << down_command << std::endl;
        if (cur < total_words)
            system("cls");
        show_progress(cur, total_words, cur_file);
        if (system(down_command.c_str()) != 0) {
            failed_words.insert(word_url);
            is_all_good = false;
            std::string del_failed_file = "del ";
            del_failed_file = del_failed_file + "\".\\" + dir + "\\" + file_name + "\"";
            system(del_failed_file.c_str());
        }
    }
    return is_all_good;
}

