#include <fstream>
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <sstream>
#include <vector>

void print_nums(std::vector<int> nums, int cnt) {
  for (int i = 0; i < cnt; i++) {
    std::cout << nums.at(i) << ' ';
  }
  std::cout << '\n';
}

int main(int argc, char **argv) {
  if (argc != 2)
    std::cout << "Usage: " << argv[0] << " <file name>" << '\n';

  std::ifstream infile(argv[1]); // open the file
  if (infile.good()) {           // if file exist
    int cnt, idx = 0;
    std::string line;

    // get # of nums
    std::getline(infile, line);
    cnt = std::stoi(line);

    // build nums
    std::vector<int> nums(cnt);
    std::getline(infile, line);
    std::stringstream ss(line);
    int num;
    while (ss >> num)
      nums.at(idx++) = num;

    print_nums(nums, cnt);

  } else { // if file not exist
    std::cout << "File: " << argv[1] << " does not exist!" << std::endl;
  }

  return 0;
}
