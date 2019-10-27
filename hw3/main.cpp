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
sem_t mt;
std::vector<sem_t> mt_sem_up(16);
std::vector<sem_t> mt_sem_down(16);

struct MT_args {
  std::vector<int> *nums;
  int lb;
  int mid;
  int hb;
  int id;
};

// ST: single thread
sem_t st;

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

void *MT_sort_l0(void *void_args) {
  MT_args *args = (MT_args *)void_args;

  // start of count the time
  struct timeval start, end;
  gettimeofday(&start, 0);

  sem_post(&mt_sem_down.at(args->id * 2));
  sem_post(&mt_sem_down.at(args->id * 2 + 1));

  sem_wait(&mt_sem_up.at(args->id * 2));
  sem_wait(&mt_sem_up.at(args->id * 2 + 1));

  merge(args->nums, args->lb, args->mid, args->hb);

  // end of count the time
  gettimeofday(&end, 0);
  double sec = end.tv_sec - start.tv_sec;
  double usec = end.tv_usec - start.tv_usec;
  std::cout << "MT time: " << sec * 1000 + (usec / 1000) << " ms" << '\n';

  std::ofstream outfile("output1.txt");
  print_nums(outfile, args->nums);
  outfile.close();

  sem_post(&mt);
  pthread_exit(nullptr);
}

void *MT_sort_l1(void *void_args) {
  MT_args *args = (MT_args *)void_args;

  sem_wait(&mt_sem_down.at(args->id));

  sem_post(&mt_sem_down.at(args->id * 2));
  sem_post(&mt_sem_down.at(args->id * 2 + 1));

  sem_wait(&mt_sem_up.at(args->id * 2));
  sem_wait(&mt_sem_up.at(args->id * 2 + 1));

  merge(args->nums, args->lb, args->mid, args->hb);

  sem_post(&mt_sem_up.at(args->id));
  pthread_exit(nullptr);
}

void *MT_sort_l2(void *void_args) {
  MT_args *args = (MT_args *)void_args;

  sem_wait(&mt_sem_down.at(args->id));

  sem_post(&mt_sem_down.at(args->id * 2));
  sem_post(&mt_sem_down.at(args->id * 2 + 1));

  sem_wait(&mt_sem_up.at(args->id * 2));
  sem_wait(&mt_sem_up.at(args->id * 2 + 1));

  merge(args->nums, args->lb, args->mid, args->hb);

  sem_post(&mt_sem_up.at(args->id));
  pthread_exit(nullptr);
}

void *MT_sort_l3(void *void_args) {
  MT_args *args = (MT_args *)void_args;

  sem_wait(&mt_sem_down.at(args->id));

  bubble_sort(args->nums, args->lb, args->hb);

  sem_post(&mt_sem_up.at(args->id));
  pthread_exit(nullptr);
}

void ST_sort(std::vector<int> &nums, int lb, int ub, int level) {
  if (level < 3 && lb < ub) {
    int mid = (lb + ub) / 2;
    ST_sort(nums, lb, mid, level + 1);
    ST_sort(nums, mid + 1, ub, level + 1);
    merge(nums, lb, mid, ub);
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
  sleep(1);
  std::cout << "ST time: " << sec * 1000 + (usec / 1000) << " ms" << '\n';

  std::ofstream outfile("output2.txt");
  print_nums(outfile, args->nums);
  outfile.close();

  sem_post(&st);
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
    std::vector<MT_args> mt_args(16);
    std::vector<int> mt_nums(nums.begin(), nums.end());

    mt_args.at(1).nums = &mt_nums;
    mt_args.at(1).lb = 0;
    mt_args.at(1).hb = cnt - 1;
    mt_args.at(1).mid = (mt_args.at(1).lb + mt_args.at(1).hb) / 2;
    mt_args.at(1).id = 1;

    for (int i = 2; i <= 15; i++) {
      mt_args.at(i).nums = &mt_nums;
      if (i % 2) {
        mt_args.at(i).lb = mt_args.at(i / 2).lb;
        mt_args.at(i).hb = mt_args.at(i / 2).mid;
      } else {
        mt_args.at(i).lb = mt_args.at(i / 2).mid + 1;
        mt_args.at(i).hb = mt_args.at(i / 2).hb;
      }
      mt_args.at(i).mid = (mt_args.at(i).lb + mt_args.at(i).hb) / 2;
      mt_args.at(i).id = i;

      sem_init(&mt_sem_up.at(i), 0, 0);
      sem_init(&mt_sem_down.at(i), 0, 0);
    }

    for (int i = 1; i <= 15; i++) {
      if (i == 1)
        pthread_create(&tid.at(i), nullptr, MT_sort_l0, &mt_args.at(i));
      else if (i == 2 || i == 3)
        pthread_create(&tid.at(i), nullptr, MT_sort_l1, &mt_args.at(i));
      else if (i >= 4 && i <= 7)
        pthread_create(&tid.at(i), nullptr, MT_sort_l2, &mt_args.at(i));
      else if (i >= 8 && i <= 15)
        pthread_create(&tid.at(i), nullptr, MT_sort_l3, &mt_args.at(i));
    }

    sem_wait(&mt);

    // Single thread
    ST_args st_args;
    st_args.nums = nums;
    st_args.lb = 0;
    st_args.hb = cnt - 1;
    st_args.level = 0;
    pthread_create(&tid.at(0), nullptr, ST_helper, &st_args);

    sem_wait(&st);

  } else { // if file not exist
    std::cout << "File: " << argv[1] << " does not exist!" << std::endl;
  }

  return 0;
}
