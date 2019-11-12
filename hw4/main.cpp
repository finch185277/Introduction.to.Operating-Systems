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

// MT: multiple thread
struct MT_args {
  std::vector<int> *nums;
  int lb;
  int mid;
  int hb;
  int id;
};

// ST: single thread
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
  sem_wait(&sem_down.at(1));

  sem_post(&sem_down.at(args->id * 2));
  sem_post(&sem_down.at(args->id * 2 + 1));

  sem_wait(&sem_up.at(args->id * 2));
  sem_wait(&sem_up.at(args->id * 2 + 1));

  merge(args->nums, args->lb, args->mid, args->hb);

  std::ofstream outfile("output1.txt");
  print_nums(outfile, args->nums);
  outfile.close();

  sem_post(&sem_up.at(1));
  pthread_exit(nullptr);
}

void *MT_sort_l1(void *void_args) {
  MT_args *args = (MT_args *)void_args;
  sem_wait(&sem_down.at(args->id));

  sem_post(&sem_down.at(args->id * 2));
  sem_post(&sem_down.at(args->id * 2 + 1));

  sem_wait(&sem_up.at(args->id * 2));
  sem_wait(&sem_up.at(args->id * 2 + 1));

  merge(args->nums, args->lb, args->mid, args->hb);

  sem_post(&sem_up.at(args->id));
  pthread_exit(nullptr);
}

void *MT_sort_l2(void *void_args) {
  MT_args *args = (MT_args *)void_args;
  sem_wait(&sem_down.at(args->id));

  sem_post(&sem_down.at(args->id * 2));
  sem_post(&sem_down.at(args->id * 2 + 1));

  sem_wait(&sem_up.at(args->id * 2));
  sem_wait(&sem_up.at(args->id * 2 + 1));

  merge(args->nums, args->lb, args->mid, args->hb);

  sem_post(&sem_up.at(args->id));
  pthread_exit(nullptr);
}

void *MT_sort_l3(void *void_args) {
  MT_args *args = (MT_args *)void_args;
  sem_wait(&sem_down.at(args->id));

  bubble_sort(args->nums, args->lb, args->hb);

  sem_post(&sem_up.at(args->id));
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
  sem_wait(&sem_down.at(0));

  ST_sort(args->nums, args->lb, args->hb, args->level);

  std::ofstream outfile("output2.txt");
  print_nums(outfile, args->nums);
  outfile.close();

  sem_post(&sem_up.at(0));
  pthread_exit(nullptr);
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

    // build all thread id
    std::vector<pthread_t> tid(16);

    // init all semaphore
    for (int i = 0; i < 16; i++) {
      sem_init(&sem_up.at(i), 0, 0);
      sem_init(&sem_down.at(i), 0, 0);
    }

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

    // start of count the time
    struct timeval mt_start, mt_end;
    gettimeofday(&mt_start, 0);

    sem_post(&sem_down.at(1));
    sem_wait(&sem_up.at(1));

    // end of count the time
    gettimeofday(&mt_end, 0);
    double mt_sec = mt_end.tv_sec - mt_start.tv_sec;
    double mt_usec = mt_end.tv_usec - mt_start.tv_usec;
    std::cout << "MT sorting used " << mt_sec + (mt_usec / 1000000)
              << " secs\n";

    // Single thread
    ST_args st_args;
    st_args.nums = nums;
    st_args.lb = 0;
    st_args.hb = cnt - 1;
    st_args.level = 0;
    pthread_create(&tid.at(0), nullptr, ST_helper, &st_args);

    // start of count the time
    struct timeval st_start, st_end;
    gettimeofday(&st_start, 0);

    sem_post(&sem_down.at(0));
    sem_wait(&sem_up.at(0));

    // end of count the time
    gettimeofday(&st_end, 0);
    double st_sec = st_end.tv_sec - st_start.tv_sec;
    double st_usec = st_end.tv_usec - st_start.tv_usec;
    std::cout << "ST sorting used " << st_sec + (st_usec / 1000000)
              << " secs\n";

  } else { // if file not exist
    std::cout << "File: " << file_name << " does not exist!" << '\n';
  }

  return 0;
}
