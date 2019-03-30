#pragma once

#include <string>
#include <iostream>
#include <dirent.h>
#include <vector>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctime>

#include "picosha2.h"

struct config_data {
	std::vector<std::string> source_paths;
	std::vector<std::string> ignored_types;
	std::string destination_path = "";
};

struct file_entry {
	std::string name, full_path, root_path, hash;
	long long int creation_time, last_mod_time;
	long long int size;
	bool is_directory;
	std::vector<file_entry> child_files;
};

void check_source(const char* path, std::vector<file_entry>& files, int& dir_num, int& file_num, std::vector<std::string>& ignored_types, std::string root_path, bool print) {
	struct dirent* entry;
	DIR* dir = opendir(path);
   
	if(dir == nullptr) {
		return;
	}

	while((entry = readdir(dir)) != nullptr) {
		std::string file_name = entry->d_name;
		if(file_name != "." && file_name != "..") {
			std::string file_path(path);
			file_path.append(entry->d_name);

			struct stat fileInfo;
			stat(file_path.c_str(), &fileInfo);

			std::ifstream in(file_path, std::ifstream::ate | std::ifstream::binary);
			long long int fs = in.tellg();
			std::string temp = root_path + file_name;

			file_entry fe = {file_name, file_path, temp, "", fileInfo.st_ctime, fileInfo.st_mtime, fs, false};
			
			switch(entry->d_type) {
				case DT_DIR: 
					dir_num++;
					fe.full_path.append("/");
					fe.is_directory = true;

					if(print)
						std::cout << "[  LOG  ] Scanning path: " << fe.full_path << '\n';

					fe.root_path.append("/");
					check_source(fe.full_path.c_str(), fe.child_files, dir_num, file_num, ignored_types, fe.root_path, print);

					files.push_back(fe);		
					break;

				case DT_REG: {
					bool ignore = false;
					for(auto& i : ignored_types) {
						if(fe.name.substr(fe.name.find_last_of(".") + 1) == i) {
							ignore = true;
							break;
						}
					}
					
					if(!ignore) {
						file_num++;

						std::ifstream f(fe.full_path, std::ios::binary);
						std::vector<unsigned char> s(picosha2::k_digest_size);
						picosha2::hash256(f, s.begin(), s.end());
						std::string ss(s.begin(), s.end());
						fe.hash = ss;

						files.push_back(fe);
					}
					break; }

				default:
					std::cout << "[WARNING] Unsupported file type - " << entry->d_name << '\n';
			}
		}
	}

	closedir(dir);
}

void check_root(std::string file_path, std::vector<file_entry>& files, int& dir_num, int& file_num, std::vector<std::string>& ignored_types, bool print) {
	struct stat fileInfo;
	stat(file_path.c_str(), &fileInfo);

	std::ifstream in(file_path, std::ifstream::ate | std::ifstream::binary);
	long long int fs = in.tellg();

	std::string name;
	std::string shortened = file_path.substr(0, file_path.length()-1);
	if(file_path[file_path.length()-1] == '/') {
		name = shortened.substr(shortened.find_last_of("/") + 1);
	}else{
		name = file_path.substr(file_path.find_last_of("/") + 1);
	}

	file_entry fe = {name, file_path, name+"/", "", fileInfo.st_ctime, fileInfo.st_mtime, fs, false};
	dir_num++;
	fe.is_directory = true;

	check_source(fe.full_path.c_str(), fe.child_files, dir_num, file_num, ignored_types, fe.root_path, print);

	files.push_back(fe);
}

config_data read_config(const char* path) {
	config_data cd;

	std::ifstream cFile(path);

	if(cFile.is_open()) {
		std::string line;
		while(getline(cFile, line)) {
			line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());
			
			if(line[0] == '#' || line.empty())
				continue;

			auto delimiterPos = line.find("=");
			auto name = line.substr(0, delimiterPos);
			auto value = line.substr(delimiterPos + 1);

			if(name == "source") {
				cd.source_paths.push_back(value);
			}else if(name == "destination") {
				if(cd.destination_path != "") {
					std::cout << "[WARNING] Destination path already provided, ignoring: " << name << " = " << value << '\n';
				}else{
					cd.destination_path = value;	
				}
			}else if(name == "ignored") {
				cd.ignored_types.push_back(value);
				std::cout << "[  LOG  ] Excluding: ." << value << " types.\n";
			}else{
				std::cout << "[WARNING] Skipping unkwnon parameter: " << name << " = " << value << '\n';
			}
		}	
	}else{
		std::cerr << "[ ERROR ] Couldn't open config file for reading.\n";
	}

	return cd;
}