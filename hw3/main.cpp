#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

void signal_handler(int sig) {
  int stat;
  while (true) {
    pid_t pid = waitpid(-1, &stat, WNOHANG);
  }
  return;
}

void print_nums(std::ofstream &fst, std::vector<int> nums, int cnt) {
  bool is_first = true;
  for (int i = 0; i < cnt; i++) {
    if (is_first) {
      fst << nums.at(i);
      is_first = false;
    } else {
      fst << ' ' << nums.at(i);
    }
  }
}

void bubble_sort(std::vector<int> &nums, int lb, int ub) {
  int cnt = nums.size();
  for (int i = ub - 1; i > 0; i--)
    for (int j = lb; j < i; j++)
      if (nums.at(j) > nums.at(j + 1))
        std::swap(nums.at(j), nums.at(j + 1));
}

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " <file name>" << '\n';
    return -1;
  }

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

    signal(SIGCHLD, signal_handler);

    pid_t pid = fork();
    if (pid > 0) {
      pid_t thread_pid = fork();
      if (thread_pid > 0) {
        std::ofstream outfile("output1.txt");
        bubble_sort(nums, 0, nums.size());
        print_nums(outfile, nums, cnt);
      } else if (thread_pid == 0) {
        std::ofstream outfile("output2.txt");
        bubble_sort(nums, 0, nums.size());
        print_nums(outfile, nums, cnt);
      } else {
        std::cout << "Fork error" << '\n';
      }
    } else if (pid == 0) {
    } else {
      std::cout << "Fork error" << '\n';
    }

  } else { // if file not exist
    std::cout << "File: " << argv[1] << " does not exist!" << std::endl;
  }

  return 0;
}
