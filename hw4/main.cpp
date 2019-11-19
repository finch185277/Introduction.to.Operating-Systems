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

sem_t is_job_ready, is_job_done;
sem_t mutux_job_list, mutux_check_list;

struct Args {
  std::vector<int> *nums;
};

struct Job {
  bool is_taken;
  int lb;
  int mid;
  int hb;
  int id;
  int sort_type; // 0: bubble sort 1: merge sort
};

struct Range {
  int lb;
  int mid;
  int hb;
};

std::vector<struct Job> job_list;
std::vector<bool> check_list(16, false);

void print_nums(std::ofstream &fst, std::vector<int> &nums) {
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

void bubble_sort(std::vector<int> *nums, int lb, int ub) {
  for (int i = ub; i > 0; i--)
    for (int j = lb; j < i; j++)
      if (nums->at(j) > nums->at(j + 1))
        std::swap(nums->at(j), nums->at(j + 1));
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

void sort_worker(std::vector<int> *nums) {
  int idx = 0;
  for (; idx < job_list.size(); idx++)
    if (job_list.at(idx).is_taken == false)
      break;

  if (job_list.at(idx).sort_type == 0)
    bubble_sort(nums, job_list.at(idx).lb, job_list.at(idx).hb);
  else if (job_list.at(idx).sort_type == 1)
    merge(nums, job_list.at(idx).lb, job_list.at(idx).mid, job_list.at(idx).hb);

  sem_wait(&mutux_check_list);
  check_list.at(job_list.at(idx).id) = true;
  std::cout << "get job " << job_list.at(idx).id << " done" << '\n';
  sem_post(&mutux_check_list);

  job_list.at(idx).is_taken = true;
}

void *thread_pool_maintainer(void *void_args) {
  Args *args = (Args *)void_args;
  for (;;) {
    sem_wait(&is_job_ready);
    sem_wait(&mutux_job_list);
    sort_worker(args->nums);
    sem_post(&mutux_job_list);
    sem_post(&is_job_done);
  }
}

void job_dispatcher(std::vector<int> &nums, int n) {
  job_list.resize(0);
  struct Job job;

  std::vector<struct Range> range_list(16);
  range_list.at(1).lb = 0;
  range_list.at(1).hb = nums.size() - 1;
  range_list.at(1).mid = (range_list.at(1).lb + range_list.at(1).hb) / 2;
  for (int i = 2; i <= 15; i++) {
    if (i % 2) {
      range_list.at(i).lb = range_list.at(i / 2).lb;
      range_list.at(i).hb = range_list.at(i / 2).mid;
    } else {
      range_list.at(i).lb = range_list.at(i / 2).mid + 1;
      range_list.at(i).hb = range_list.at(i / 2).hb;
    }
    range_list.at(i).mid = (range_list.at(i).lb + range_list.at(i).hb) / 2;
  }

  for (int i = 8; i <= 15; i++) {
    job.is_taken = false;
    job.lb = range_list.at(i).lb;
    job.hb = range_list.at(i).hb;
    job.id = i;
    job.sort_type = 0;
    sem_wait(&mutux_job_list);
    job_list.push_back(job);
    sem_post(&mutux_job_list);
    sem_post(&is_job_ready);
  }

  for (int remain_jobs = 15; remain_jobs >= 1; remain_jobs--) {
    sem_wait(&is_job_done);
    sem_wait(&mutux_check_list);
    for (int i = 7; i >= 1; i--) {
      if (check_list.at(i * 2) && check_list.at(i * 2 + 1)) {
        job.is_taken = false;
        job.lb = range_list.at(i).lb;
        job.mid = range_list.at(i).mid;
        job.hb = range_list.at(i).hb;
        job.id = i;
        job.sort_type = 1;

        sem_wait(&mutux_job_list);
        job_list.push_back(job);
        sem_post(&mutux_job_list);
        sem_post(&is_job_ready);

        check_list.at(i * 2) = false;
        check_list.at(i * 2 + 1) = false;
      }
    }
    sem_post(&mutux_check_list);
  }

  std::stringstream ss;
  ss << "output" << n << ".txt";
  std::string file_name = ss.str();
  std::ofstream outfile(file_name);
  print_nums(outfile, nums);
  outfile.close();
}

void sort_with_n_thread(std::vector<int> &nums, int n) {
  // start of count the time
  struct timeval st_start, st_end;
  gettimeofday(&st_start, 0);

  job_dispatcher(nums, n);

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

    sem_init(&is_job_ready, 0, 0);
    sem_init(&is_job_done, 0, 0);
    sem_init(&mutux_job_list, 0, 1);
    sem_init(&mutux_check_list, 0, 1);

    pthread_t tid;
    struct Args args;
    std::vector<int> thread_nums(nums.begin(), nums.end());
    args.nums = &thread_nums;
    pthread_create(&tid, nullptr, thread_pool_maintainer, &args);

    sort_with_n_thread(thread_nums, 1);

  } else { // if file not exist
    std::cout << "File: " << file_name << " does not exist!" << '\n';
  }

  return 0;
}
