#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <sstream>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

struct thread_args {
  std::vector<int> nums;
  int cnt;
};

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

void single_thread_merge_sort(std::vector<int> &nums, int cnt) {
  int base = cnt / 8, ext = cnt % 8;
  bubble_sort(nums, 0, nums.size());
  std::cout << "cnt: " << cnt << '\n';
  std::ofstream outfile("output2.txt");
  print_nums(outfile, nums, cnt);
  outfile.close();
}

void *single_thread_helper(void *void_args) {
  std::cout << "mid of single thread helper" << '\n';
  thread_args *args = (thread_args *)void_args;

  // start of count the time
  struct timeval start, end;
  gettimeofday(&start, 0);

  single_thread_merge_sort(args->nums, args->cnt);

  // end of count the time
  gettimeofday(&end, 0);
  double sec = end.tv_sec - start.tv_sec;
  double usec = end.tv_usec - start.tv_usec;
  std::cout << "Single thread time: " << sec * 1000 + (usec / 1000) << " ms"
            << '\n';

  pthread_exit(nullptr);
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

    pid_t pid = fork();
    if (pid > 0) {
      pid_t thread_pid = fork();
      if (thread_pid > 0) {
        // start of count the time
        struct timeval start, end;
        gettimeofday(&start, 0);

        std::ofstream outfile("output1.txt");
        bubble_sort(nums, 0, nums.size());
        print_nums(outfile, nums, cnt);
        outfile.close();

        // end of count the time
        gettimeofday(&end, 0);
        double sec = end.tv_sec - start.tv_sec;
        double usec = end.tv_usec - start.tv_usec;
        std::cout << "Multiple thread time: " << sec * 1000 + (usec / 1000)
                  << " ms" << '\n';

        exit(0);
      } else if (thread_pid == 0) {
        wait(nullptr);
        // single thread
        pthread_t st_id;
        thread_args args;
        args.nums = nums;
        args.cnt = cnt;
        std::cout << "start of single thread" << '\n';
        pthread_create(&st_id, nullptr, single_thread_helper, &args);
        std::cout << "end of single thread" << '\n';
        pthread_join(st_id, nullptr);
        exit(0);
      } else {
        std::cout << "Fork error" << '\n';
      }
    } else if (pid == 0) {
      wait(nullptr);
    } else {
      std::cout << "Fork error" << '\n';
    }

  } else { // if file not exist
    std::cout << "File: " << argv[1] << " does not exist!" << std::endl;
  }

  return 0;
}
