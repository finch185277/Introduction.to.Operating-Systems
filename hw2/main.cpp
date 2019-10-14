/*
Student No.: 0616214
Student Name: YU-AN CHEN
Email: 59487andy@gmail.com
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not
supposed to be posted to a public server, such as a
public GitHub repository or a public web page.
*/

#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int dimension;
  std::cin >> dimension;

  // using shared memory to create three matrix
  unsigned int *matrix_a;
  unsigned int *matrix_b;
  unsigned int *matrix_c;
  int matrix_a_id =
      shmget(IPC_PRIVATE, dimension * dimension * sizeof(unsigned int),
             IPC_CREAT | 0600);
  int matrix_b_id =
      shmget(IPC_PRIVATE, dimension * dimension * sizeof(unsigned int),
             IPC_CREAT | 0600);
  int matrix_c_id =
      shmget(IPC_PRIVATE, dimension * dimension * sizeof(unsigned int),
             IPC_CREAT | 0600);
  matrix_a = (unsigned int *)shmat(matrix_a_id, nullptr, 0);
  matrix_b = (unsigned int *)shmat(matrix_b_id, nullptr, 0);
  matrix_c = (unsigned int *)shmat(matrix_c_id, nullptr, 0);

  // create raw matrix
  for (int x = 0; x < dimension; x++) {
    for (int y = 0; y < dimension; y++) {
      matrix_a[x * dimension + y] = x * dimension + y;
      matrix_b[x * dimension + y] = x * dimension + y;
    }
  }

  for (int fork_counter = 1; fork_counter <= 16; fork_counter++) {
    unsigned int checksum = 0;

    // turn matrix c to zero
    for (int x = 0; x < dimension; x++)
      for (int y = 0; y < dimension; y++)
        matrix_c[x * dimension + y] = 0;

    // start of count the time
    struct timeval start, end;
    gettimeofday(&start, 0);

    // fork the child process and continue
    for (int cur_p = 0; cur_p < fork_counter; cur_p++) {
      pid_t pid = fork();
      if (pid == 0) {
        for (int x = 0; x < dimension; x++) {
          for (int y = 0; y < dimension; y++) {
            if ((x * dimension + y) % fork_counter == cur_p) {
              for (int z = 0; z < dimension; z++) {
                matrix_c[x * dimension + y] +=
                    matrix_a[x * dimension + z] * matrix_b[z * dimension + y];
              }
            }
          }
        }
        exit(0);
      }
    }

    // wait all child process
    for (int i = fork_counter; i > 0; i--) {
      wait(nullptr);
    }

    // calculate the checksum
    for (int x = 0; x < dimension; x++) {
      for (int y = 0; y < dimension; y++) {
        checksum += matrix_c[x * dimension + y];
      }
    }

    // end of count the time
    gettimeofday(&end, 0);
    int sec = end.tv_sec - start.tv_sec;
    int usec = end.tv_usec - start.tv_usec;
    std::cout << "Multiply matrices using " << fork_counter << " process"
              << '\n';
    std::cout << "elapsed " << sec * 1000 + (usec / 1000.0)
              << " sec, Checksum: " << checksum << '\n';
  }

  // detach and detroy the shared memory
  shmdt(matrix_a);
  shmdt(matrix_b);
  shmdt(matrix_c);
  shmctl(matrix_a_id, IPC_RMID, nullptr);
  shmctl(matrix_b_id, IPC_RMID, nullptr);
  shmctl(matrix_c_id, IPC_RMID, nullptr);
  return 0;
}
