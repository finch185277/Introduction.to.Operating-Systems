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

// l0
sem_t t1_to_t2, t2_to_t1;
sem_t t1_to_t3, t3_to_t1;

// l1
sem_t t2_to_t4, t4_to_t2;
sem_t t2_to_t5, t5_to_t2;
sem_t t3_to_t6, t6_to_t3;
sem_t t3_to_t7, t7_to_t3;

// l2
sem_t t4_to_t8, t8_to_t4;
sem_t t4_to_t9, t9_to_t4;
sem_t t5_to_t10, t10_to_t5;
sem_t t5_to_t11, t11_to_t5;
sem_t t6_to_t12, t12_to_t6;
sem_t t6_to_t13, t13_to_t6;
sem_t t7_to_t14, t14_to_t7;
sem_t t7_to_t15, t15_to_t7;

// st
sem_t t0;

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

  sem_post(&t1_to_t2);
  sem_post(&t1_to_t3);

  sem_wait(&t2_to_t1);
  sem_wait(&t3_to_t1);
}

void *MT_sort_l1(void *void_args) {
  sem_wait(&t1_to_t2);
  std::cout << "t1_to_t2" << '\n';
  sem_post(&t2_to_t1);

  sem_wait(&t1_to_t3);
  std::cout << "t1_to_t3" << '\n';
  sem_post(&t3_to_t1);

  sem_post(&t2_to_t4);
  sem_post(&t2_to_t5);

  sem_wait(&t4_to_t2);
  sem_wait(&t5_to_t2);

  pthread_exit(nullptr);
}

void *MT_sort_l2(void *void_args) {
  sem_wait(&t2_to_t4);
  std::cout << "t2_to_t4" << '\n';
  sem_post(&t4_to_t2);

  sem_wait(&t2_to_t5);
  std::cout << "t2_to_t5" << '\n';
  sem_post(&t5_to_t2);

  sem_post(&t4_to_t8);
  sem_post(&t4_to_t9);

  sem_wait(&t8_to_t4);
  sem_wait(&t9_to_t4);

  pthread_exit(nullptr);
}

void *MT_sort_l3(void *void_args) {
  sem_wait(&t4_to_t8);
  std::cout << "t4_to_t8" << '\n';
  sem_post(&t8_to_t4);

  sem_wait(&t4_to_t9);
  std::cout << "t4_to_t9" << '\n';
  sem_post(&t9_to_t4);

  pthread_exit(nullptr);
}

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

  sem_post(&t0);

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

    pthread_create(&tid.at(2), nullptr, MT_sort_l1, &mt_args);
    pthread_create(&tid.at(3), nullptr, MT_sort_l1, &mt_args);

    pthread_create(&tid.at(4), nullptr, MT_sort_l2, &mt_args);
    pthread_create(&tid.at(5), nullptr, MT_sort_l2, &mt_args);
    pthread_create(&tid.at(6), nullptr, MT_sort_l2, &mt_args);
    pthread_create(&tid.at(7), nullptr, MT_sort_l2, &mt_args);

    pthread_create(&tid.at(8), nullptr, MT_sort_l3, &mt_args);
    pthread_create(&tid.at(9), nullptr, MT_sort_l3, &mt_args);
    pthread_create(&tid.at(10), nullptr, MT_sort_l3, &mt_args);
    pthread_create(&tid.at(11), nullptr, MT_sort_l3, &mt_args);
    pthread_create(&tid.at(12), nullptr, MT_sort_l3, &mt_args);
    pthread_create(&tid.at(13), nullptr, MT_sort_l3, &mt_args);
    pthread_create(&tid.at(14), nullptr, MT_sort_l3, &mt_args);
    pthread_create(&tid.at(15), nullptr, MT_sort_l3, &mt_args);

    // Single thread
    ST_args st_args;
    st_args.nums = nums;
    st_args.lb = 0;
    st_args.hb = nums.size() - 1;
    st_args.level = 0;
    pthread_create(&tid.at(0), nullptr, ST_helper, &st_args);

    sem_wait(&t0);
    std::cout << "end of prog" << '\n';

  } else { // if file not exist
    std::cout << "File: " << argv[1] << " does not exist!" << std::endl;
  }

  return 0;
}
