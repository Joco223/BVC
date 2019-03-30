#pragma once

#include <string>
#include <iostream>
#include <dirent.h>
#include <vector>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctime>
#include <math.h>
#include <chrono>
#include <iomanip>

#include "picosha2.h"
#include "source_handler.h"

bool check_directory(std::string path) {
	struct stat info;

	if(stat(path.c_str(), &info) != 0) {
		return false;
	}else if(info.st_mode & S_IFDIR) {
		return true;
	}else{
		return false;
	}
}

bool check_file(std::string path) {
	std::ifstream ifile(path.c_str());

	if(!ifile.is_open()) {
		return false;
	}else{
		return true;
	}
}

void check_destination(std::vector<file_entry>& target_files, std::string destination_path, int& file_num, int& fileu_num, int& dir_num, bool print, int& total_file_count, int& curr_file_count) {
	for(auto& i : target_files) {
		curr_file_count++;
		if(!print) {
			std::cout << "[  LOG  ] [";

			int count = ceil(((float)(curr_file_count+1) / (float)total_file_count) * 35);

			for(int j = 0; j < count; j++) {
				std::cout << "#";
			}

			for(int j = count; j < 35; j++) {
				std::cout << "-";
			}

			std::cout << "] - " << (curr_file_count+1) << " / " << total_file_count << '\r';
		}

		std::string new_dest = destination_path+i.name;	
		if(i.is_directory) {
			bool exists = check_directory(new_dest);

			if(exists) {
				if(print)
					std::cout << "[  LOG  ] Directory: " << i.name << " already exists.\n";
			}else{
				if(print)
					std::cout << "[  LOG  ] Directory: " << i.name << "/ doesn't exist. Creating.\n";
				mkdir(new_dest.c_str());
				dir_num++;
			}

			check_destination(i.child_files, new_dest+"/", file_num, fileu_num, dir_num, print, total_file_count, curr_file_count);
		}else{
			bool exists = check_file(new_dest);

			if(exists) {
				if(print)
					std::cout << "[  LOG  ] File: " << i.name << " already exists. Checking hash. ";

				std::ifstream f(new_dest, std::ios::binary);
				std::vector<unsigned char> s(picosha2::k_digest_size);
				picosha2::hash256(f, s.begin(), s.end());
				std::string ss(s.begin(), s.end());

				if(i.hash == ss) {
					if(print)
						std::cout << "Valid hash.\n";
				}else{
					if(print)
						std::cout << "Invalid hash. Updating.\n";

					std::ifstream src(i.full_path, std::ios::binary);
					std::ofstream dst(new_dest, std::ios::binary | std::ios::trunc);

					std::copy(std::istreambuf_iterator<char>(src), std::istreambuf_iterator<char>(),std::ostreambuf_iterator<char>(dst));
					fileu_num++;
				}
			}else{
				if(print)
					std::cout << "[  LOG  ] File: " << i.name << " doesn't exist. Creating.\n";

				std::ifstream src(i.full_path, std::ios::binary);
				std::ofstream dst(new_dest, std::ios::binary);

				std::copy(std::istreambuf_iterator<char>(src), std::istreambuf_iterator<char>(),std::ostreambuf_iterator<char>(dst));
				file_num++;
			}	
		}
	}
}

void create_root_dir(std::string path, bool& done, std::string& end_path, int& dir_num) {
	bool exists = check_directory(path);

	if(path.find('/') == std::string::npos) {return;}

	if(!exists) {
		std::string new_dest = path.substr(0, path.find_last_of("/"));
		end_path = new_dest;
		create_root_dir(new_dest, done, end_path, dir_num);
		done = mkdir(path.c_str());
		dir_num++;
	}
}

void check_destination_root(file_entry& root_dir, config_data& cd, int& file_num, int& fileu_num, int& dir_num, bool print, int total_file_count) {
	if(cd.destination_path[cd.destination_path.length()-1] != '/') {
		cd.destination_path.append("/");
	}

	std::string new_dest = cd.destination_path+root_dir.root_path;
	bool exists = check_directory(new_dest);

	if(exists) {
		std::cout << "[  LOG  ] Destination path already exists.\n";
	}else{
		std::cout << "[  LOG  ] Destination path doesn't exist. Creating.\n";
		bool result = true;
		std::string end_path = "";
		create_root_dir(new_dest.substr(0, new_dest.length()-1), result, end_path, dir_num);
		if(result) {
			std::cout << "[ ERROR ] Couldn't create destination directory.\n";
			std::cout << "          Couldn't find directory: " << end_path << '\n';
			return;
		}
	}
	int curr_file_count = 0;
	check_destination(root_dir.child_files, new_dest, file_num, fileu_num, dir_num, print, total_file_count, curr_file_count);
	std::cout << '\n';
}