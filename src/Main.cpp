#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <math.h>

#include "source_handler.h"
#include "destination_handler.h"

const std::vector<char> size_lookup = {'B', 'K', 'M', 'G', 'P'};

void print_size(long long int file_size) {
	int step = 0;

	while(file_size > 1024) {
		file_size /= 1024;
		step++;
	}

	file_size = floor(file_size * 100.0) / 100.0;

	std::cout << file_size << size_lookup[step];
}

void print_files(std::vector<file_entry>& files, int& depth, long long int& total_size, bool print) {
	unsigned char L = 195;
	unsigned char N = 192;

	for(auto& i : files) {
		if(print) {
			std::cout << "[  LOG  ] ";
		
			for(int j = 0; j < depth; j++) {
				std::cout << " ";
			}

			if(depth > 0) {
				if(files[files.size()-1].name == i.name || i.child_files.size() > 0) {
					std::cout << N;
				}else{
					std::cout << L;
				}
			}

			std::cout << i.name;
		}

		if(i.is_directory) {
			if(print) {
				std::cout << "/\n";
			}	
			depth++;
			print_files(i.child_files, depth, total_size, print);
			depth--;
		}else{
			total_size += i.size;

			if(print) {
				std::cout << " - ";
				print_size(i.size);
				std::cout << '\n';
			}	
		}
	}
}

int main(int argc, char** argv) {

	if(argc < 2) {
		std::cout << "[ERROR] Not enough arguments provides.\n";
		std::cout << "        Example: BVC.exe config_file\n";
		return -1;
	}

	config_data cd = read_config(argv[1]);

	std::vector<file_entry> files;
	int dir_num = 0;
	int file_num = 0;

	for(auto& i : cd.source_paths) {
		bool exists = false;

		for(auto& j : cd.source_paths) {
			if(i != j) {
				if(i.find(j) != std::string::npos) {
					std::cout << "[WARNING] Skipping source path: " << i << ", already subpath of:\n";
					std::cout << "                                " << j << '\n';
					exists = true;
					break;
				}
			}
		}

		if(!exists) {
			std::cout << "[  LOG  ] Scanning path: " << i << '\n';
			if(argv[2] == "-l") {
				check_root(i, files, dir_num, file_num, cd.ignored_types, true);
			}else{
				check_root(i, files, dir_num, file_num, cd.ignored_types, false);
			}	
		}	
	}

	std::cout << "[  LOG  ] Finished scanning source paths.\n";
	std::cout << "[  LOG  ] ------------------------------------------------------" << '\n';
	int depth = 0;
	long long int total_size = 0;
	if(argv[2] == "-l") {
		print_files(files, depth, total_size, true);	
		std::cout << "[  LOG  ] ------------------------------------------------------" << '\n';
	}else{
		print_files(files, depth, total_size, false);
	}
	std::cout << "[  LOG  ] Total file count is: " << file_num << ".\n";
	std::cout << "[  LOG  ] Total directory count is: " << dir_num << ".\n";
	std::cout << "[  LOG  ] Total size is: ";
	print_size(total_size);
	std::cout << ".\n";

	std::cout << "[  LOG  ] ------------------------------------------------------" << '\n';
	int fileu_num = 0; 
	int file_num2 = 0; 
	int dir_num2 = 0;
	std::ios::sync_with_stdio(false);
	if(argv[2] == "-l") {
		check_destination_root(files[0], cd, file_num2, fileu_num, dir_num2, true, dir_num+file_num);
	}else{
		check_destination_root(files[0], cd, file_num2, fileu_num, dir_num2, false, dir_num+file_num);
	}
	std::cout << "[  LOG  ] ------------------------------------------------------" << '\n';
	std::cout << "[  LOG  ] Finished!\n";
	std::cout << "[  LOG  ] Directories created: " << dir_num2 << ".\n";
	std::cout << "[  LOG  ] Files created: " << file_num2 << ".\n";
	std::cout << "[  LOG  ] Files updated: " << fileu_num << ".\n";

	return 0;
}
