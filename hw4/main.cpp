/*
Student No.: 0616214
Student Name: YU-AN CHEN
Email: 59487andy@gmail.com
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not
supposed to be posted to a public server, such as a
public GitHub repository or a public web page.
*/

#include <fstream>
#include <iostream>
#include <limits>
#include <pthread.h>
#include <semaphore.h>
#include <sstream>
#include <sys/time.h>
#include <vector>

std::vector<sem_t> sem_up(16);
std::vector<sem_t> sem_down(16);

struct args {
  std::vector<int> *nums;
  int lb;
  int mid;
  int hb;
  int id;
};

void print_nums(std::ofstream &fst, std::vector<int> nums) {
  bool is_first = true;
  for (int i = 0; i < nums.size(); i++) {
    if (is_first) {
      fst << nums.at(i);
      is_first = false;
    } else {
      fst << ' ' << nums.at(i);
    }
  }
}

void print_nums(std::ofstream &fst, std::vector<int> *nums) {
  bool is_first = true;
  for (int i = 0; i < nums->size(); i++) {
    if (is_first) {
      fst << nums->at(i);
      is_first = false;
    } else {
      fst << ' ' << nums->at(i);
    }
  }
}

void bubble_sort(std::vector<int> &nums, int lb, int ub) {
  for (int i = ub; i > 0; i--)
    for (int j = lb; j < i; j++)
      if (nums.at(j) > nums.at(j + 1))
        std::swap(nums.at(j), nums.at(j + 1));
}

void bubble_sort(std::vector<int> *nums, int lb, int ub) {
  for (int i = ub; i > 0; i--)
    for (int j = lb; j < i; j++)
      if (nums->at(j) > nums->at(j + 1))
        std::swap(nums->at(j), nums->at(j + 1));
}

void merge(std::vector<int> &nums, int lb, int mid, int ub) {
  int left_idx = 0, right_idx = 0;
  std::vector<int> left(nums.begin() + lb, nums.begin() + mid + 1),
      right(nums.begin() + mid + 1, nums.begin() + ub + 1);

  left.insert(left.end(), std::numeric_limits<int>::max());
  right.insert(right.end(), std::numeric_limits<int>::max());

  for (int i = lb; i <= ub; i++) {
    if (left.at(left_idx) < right.at(right_idx)) {
      nums.at(i) = left.at(left_idx++);
    } else {
      nums.at(i) = right.at(right_idx++);
    }
  }
}

void merge(std::vector<int> *nums, int lb, int mid, int ub) {
  int left_idx = 0, right_idx = 0;
  std::vector<int> left(nums->begin() + lb, nums->begin() + mid + 1),
      right(nums->begin() + mid + 1, nums->begin() + ub + 1);

  left.insert(left.end(), std::numeric_limits<int>::max());
  right.insert(right.end(), std::numeric_limits<int>::max());

  for (int i = lb; i <= ub; i++) {
    if (left.at(left_idx) < right.at(right_idx)) {
      nums->at(i) = left.at(left_idx++);
    } else {
      nums->at(i) = right.at(right_idx++);
    }
  }
}

void job_dispatcher() { ; }

void sort_worker() { ; }

void sort_with_n_thread(std::vector<int> &nums, n) {
  // start of count the time
  struct timeval st_start, st_end;
  gettimeofday(&st_start, 0);

  sem_post(&sem_down.at(0));
  sem_wait(&sem_up.at(0));

  // end of count the time
  gettimeofday(&st_end, 0);
  double st_sec = st_end.tv_sec - st_start.tv_sec;
  double st_usec = st_end.tv_usec - st_start.tv_usec;
  std::cout << "The sorting with " << n
            << " thread(s) used: " << st_sec + (st_usec / 1000000) << " secs\n";
}

int main(int argc, char **argv) {
  // get file name
  std::string file_name;
  std::cout << "Enter input file name: ";
  std::cin >> file_name;

  // open the file
  std::ifstream infile(file_name);
  if (infile.good()) { // if file exist
    int cnt, idx = 0;
    std::string line;

    // get # of nums
    std::getline(infile, line);
    cnt = std::stoi(line);
    if (cnt == 0) {
      std::cout << "Input Error: # of nums is 0" << '\n';
      exit(0);
    }

    // build nums
    std::vector<int> nums(cnt);
    std::getline(infile, line);
    std::stringstream ss(line);
    int num;
    while (ss >> num)
      nums.at(idx++) = num;

    // init all semaphore
    for (int i = 0; i < 16; i++) {
      sem_init(&sem_up.at(i), 0, 0);
      sem_init(&sem_down.at(i), 0, 0);
    }

    // build all thread id
    std::vector<pthread_t> tid(8);

    for (int i = 0; i < 8; i++)
      pthread_create(&tid.at(i), nullptr, sort_worker, &st_args);

    for (int i = 1; i <= 8; i++)
      sort_with_n_thread(nums, i);

  } else { // if file not exist
    std::cout << "File: " << file_name << " does not exist!" << '\n';
  }

  return 0;
}
