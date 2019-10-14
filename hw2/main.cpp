/*
Student No.: 0616214
Student Name: YU-AN CHEN
Email: 59487andy@gmail.com
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not
supposed to be posted to a public server, such as a
public GitHub repository or a public web page.
*/

#include <cstring>
#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

int main(int argc, char *argv[]) {
  // create the shared memory
  int shmid = shmget(IPC_PRIVATE, 1024, IPC_CREAT | 0600);
  int *checksum = (int *)shmat(shmid, (void *)0, 0);

  int dimension;
  std::cin >> dimension;

  // create raw matrix
  std::vector<std::vector<int>> raw(dimension, std::vector<int>(dimension));
  for (int x = 0; x < dimension; x++)
    for (int y = 0; y < dimension; y++)
      raw.at(x).at(y) = x * dimension + y;

  for (int fork_counter = 1; fork_counter <= 6; fork_counter++) {
    std::vector<std::vector<int>> final(dimension, std::vector<int>(dimension));
    *checksum = 0;

    // start of count the time
    struct timeval start, end;
    gettimeofday(&start, 0);

    for (int cur_p = 0; cur_p < fork_counter; cur_p++) {
      pid_t pid = fork();
      if (pid == 0) {
        for (int x = 0; x < dimension; x++) {
          for (int y = 0; y < dimension; y++) {
            if ((x * dimension + y) % fork_counter == cur_p) {
              for (int z = 0; z < dimension; z++)
                final.at(x).at(y) = raw.at(x).at(z) * raw.at(z).at(y);
            }
          }
        }
        exit(0);
      }
    }
    for (int i = fork_counter; i > 0; i--) {
      wait(nullptr);
    }

    for (int x = 0; x < dimension; x++) {
      for (int y = 0; y < dimension; y++) {
        *checksum += final.at(x).at(y);
      }
    }

    // end of count the time
    gettimeofday(&end, 0);
    int sec = end.tv_sec - start.tv_sec;
    int usec = end.tv_usec - start.tv_usec;
    std::cout << "Multiply matrices using " << fork_counter << " process"
              << '\n';
    std::cout << "elapsed " << sec * 1000 + (usec / 1000.0)
              << " sec, Checksum: " << *checksum << '\n';
  }

  // detach and detroy the shared memory
  shmdt(checksum);
  shmctl(shmid, IPC_RMID, nullptr);

  return 0;
}
