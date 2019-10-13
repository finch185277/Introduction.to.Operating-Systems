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
  int *shmaddr = (int *)shmat(shmid, (void *)0, 0);

  int dimension, counter = 1;
  std::cin >> dimension;

  // create raw matrix
  std::vector<std::vector<int>> raw(dimension, std::vector<int>(dimension));
  for (int i = 0; i < dimension; i++)
    for (int j = 0; j < dimension; j++)
      raw.at(i).at(j) = i * dimension + j;

  for (; counter <= 6; counter++) {
    int checksum = 0;

    // start of count the time
    struct timeval start, end;
    gettimeofday(&start, 0);

    pid_t pid = fork();
    if (pid == 0) {
      *shmaddr = 20;
      std::cout << *shmaddr << '\n';
      exit(0);
    } else if (pid > 0) {
      wait(nullptr);
      std::cout << *shmaddr + 30 << '\n';
    } else {
      std::cout << "Fork Error!" << '\n';
    }

    // end of count the time
    gettimeofday(&end, 0);
    int sec = end.tv_sec - start.tv_sec;
    int usec = end.tv_usec - start.tv_usec;
    std::cout << "Multiply matrices using " << counter << " process" << '\n';
    std::cout << "elapsed " << sec * 1000 + (usec / 1000.0)
              << " sec, Checksum: " << checksum << '\n';
  }

  // detach and detroy the shared memory
  shmdt(shmaddr);
  shmctl(shmid, IPC_RMID, nullptr);

  return 0;
}
