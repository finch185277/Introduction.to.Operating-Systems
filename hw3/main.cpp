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

// MT: multiple thread
// ST: single thread

struct MT_args {
  std::vector<int> nums;
  int lb;
  int hb;
};

struct ST_args {
  std::vector<int> nums;
  int lb;
  int hb;
  int level;
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

void bubble_sort(std::vector<int> &nums, int lb, int ub) {
  int cnt = nums.size();
  for (int i = ub; i > 0; i--)
    for (int j = lb; j < i; j++)
      if (nums.at(j) > nums.at(j + 1))
        std::swap(nums.at(j), nums.at(j + 1));
}

void MT_sort_l0(std::vector<int> &nums, int lb, int ub) {
  bubble_sort(nums, 0, nums.size() - 1);
}

void MT_sort_l1() { ; }

void MT_sort_l2() { ; }

void MT_sort_l3() { ; }

void *MT_helper(void *void_args) {
  MT_args *args = (MT_args *)void_args;

  // start of count the time
  struct timeval start, end;
  gettimeofday(&start, 0);

  MT_sort_l0(args->nums, args->lb, args->hb);

  std::ofstream outfile("output1.txt");
  print_nums(outfile, args->nums);
  outfile.close();

  // end of count the time
  gettimeofday(&end, 0);
  double sec = end.tv_sec - start.tv_sec;
  double usec = end.tv_usec - start.tv_usec;
  std::cout << "MT time: " << sec * 1000 + (usec / 1000) << " ms" << '\n';

  pthread_exit(nullptr);
}

void ST_merge(std::vector<int> &nums, int lb, int mid, int ub) {
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

void ST_sort(std::vector<int> &nums, int lb, int ub, int level) {
  if (level < 3 && lb < ub) {
    int mid = (lb + ub) / 2;
    ST_sort(nums, lb, mid, level + 1);
    ST_sort(nums, mid + 1, ub, level + 1);
    ST_merge(nums, lb, mid, ub);
  } else {
    bubble_sort(nums, lb, ub);
  }
}

void *ST_helper(void *void_args) {
  ST_args *args = (ST_args *)void_args;

  // start of count the time
  struct timeval start, end;
  gettimeofday(&start, 0);

  ST_sort(args->nums, args->lb, args->hb, args->level);

  // end of count the time
  gettimeofday(&end, 0);
  double sec = end.tv_sec - start.tv_sec;
  double usec = end.tv_usec - start.tv_usec;
  std::cout << "ST time: " << sec * 1000 + (usec / 1000) << " ms" << '\n';

  std::ofstream outfile("output2.txt");
  print_nums(outfile, args->nums);
  outfile.close();

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

    // build all thread id
    std::vector<pthread_t> tid(16);

    // Multiple thread
    MT_args mt_args;
    mt_args.nums = nums;
    mt_args.lb = 0;
    mt_args.hb = nums.size() - 1;
    pthread_create(&tid.at(1), nullptr, MT_helper, &mt_args);
    pthread_join(tid.at(1), nullptr);

    // pthread_create(&tid.at(2), nullptr, ST_helper, &args);
    // pthread_create(&tid.at(3), nullptr, ST_helper, &args);
    // pthread_create(&tid.at(4), nullptr, ST_helper, &args);
    // pthread_create(&tid.at(5), nullptr, ST_helper, &args);
    // pthread_create(&tid.at(6), nullptr, ST_helper, &args);
    // pthread_create(&tid.at(7), nullptr, ST_helper, &args);
    // pthread_create(&tid.at(8), nullptr, ST_helper, &args);
    // pthread_create(&tid.at(9), nullptr, ST_helper, &args);
    // pthread_create(&tid.at(10), nullptr, ST_helper, &args);
    // pthread_create(&tid.at(11), nullptr, ST_helper, &args);
    // pthread_create(&tid.at(12), nullptr, ST_helper, &args);
    // pthread_create(&tid.at(13), nullptr, ST_helper, &args);
    // pthread_create(&tid.at(14), nullptr, ST_helper, &args);
    // pthread_create(&tid.at(15), nullptr, ST_helper, &args);

    // Single thread
    ST_args st_args;
    st_args.nums = nums;
    st_args.lb = 0;
    st_args.hb = nums.size() - 1;
    st_args.level = 0;
    pthread_create(&tid.at(0), nullptr, ST_helper, &st_args);
    pthread_join(tid.at(0), nullptr);

  } else { // if file not exist
    std::cout << "File: " << argv[1] << " does not exist!" << std::endl;
  }

  return 0;
}
