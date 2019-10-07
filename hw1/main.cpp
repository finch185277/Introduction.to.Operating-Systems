/*
Student No.: 0616214
Student Name: YU-AN CHEN
Email: 59487andy@gmail.com
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not
supposed to be posted to a public server, such as a
public GitHub repository or a public web page.
*/

#include <cstring> // std::strtok()
#include <fcntl.h> // O_RDONLY creat()
#include <iostream>
#include <sys/types.h> // wait()
#include <sys/wait.h>  // wait()
#include <unistd.h>    // fork()
#include <vector>

void signal_handler(int sig) { wait(nullptr); }

int main(int argc, char *argv[]) {
  bool is_continue = true;
  while (is_continue) {
    // into shell
    std::cout << ">";

    // read input line by line
    std::string line, file;
    std::getline(std::cin, line);

    // parse the line
    // covert line from std::string to std::vector<char>,
    // and use std::strtok to split command and args
    char *cv[1000]; // command in char* array
    int cv_idx = 0; // cv index, point to next char* of cv
    std::vector<char> lv(line.begin(), line.end());
    lv.push_back('\0');
    char *token = std::strtok(&*lv.begin(), " ");
    while (token != nullptr) {
      cv[cv_idx++] = token;
      token = std::strtok(nullptr, " ");
    }

    // check whether run in background
    bool is_background = false;
    if (strcmp(cv[cv_idx - 1], "&") == 0) {
      is_background = true;
      cv[--cv_idx] = nullptr; // change "&" to nullptr
    } else {
      cv[cv_idx] = nullptr; // put a nullptr to the last place
    }

    // io redirect
    bool is_input = false, is_output = false;
    char *filename;
    if (cv_idx >= 3) {
      if (strcmp(cv[cv_idx - 2], "<") == 0) { // input
        is_input = true;
      } else if (strcmp(cv[cv_idx - 2], ">") == 0) { // output
        is_output = true;
      }
      if (is_input || is_output) {
        filename = cv[cv_idx - 1];
        cv[--cv_idx] = nullptr;
        cv[--cv_idx] = nullptr;
      }
    }

    // check "|" exist, if yes, split into two commands
    bool is_pipe = false;
    int cv1_idx = 0, cv2_idx = 0;
    char *cv1[500], *cv2[500]; // command in char* array
    for (int idx = 0; idx < cv_idx; idx++) {
      if (!is_pipe && strcmp(cv[idx], "|") == 0) {
        is_pipe = true;
        continue;
      }
      if (!is_pipe) // "|" not be read yet
        cv1[cv1_idx++] = cv[idx];
      else // after "|" show up
        cv2[cv2_idx++] = cv[idx];
    }
    cv1[cv1_idx] = nullptr;
    cv2[cv2_idx] = nullptr;

    // end the program when input "exit"
    if (strcmp(cv[0], "exit") == 0) {
      is_continue = false;
      break;
    }

    pid_t pid = fork();

    if (pid == 0) {   // child process
      if (!is_pipe) { // if no "|"
        // io redirect
        if (is_input) { // input
          int fd = open(filename, O_RDONLY);
          dup2(fd, STDIN_FILENO);
          close(fd);
        } else if (is_output) { // output
          int fd = creat(filename, 0644);
          dup2(fd, STDOUT_FILENO);
          close(fd);
        }
        // execute the command
        if (execvp(cv[0], cv) < 0)
          std::cout << "Command \"" << cv[0] << "\" not found!" << std::endl;
      } else { // if need "|"
        int fd[2];
        pipe(fd); // create pipe shared by parent and child
        pid_t pipe_pid = fork();
        if (pipe_pid == 0) { // command before "|"
          // output ->  pipe
          close(fd[0]); // close pipe read end
          dup2(fd[1], STDOUT_FILENO);
          close(fd[1]); // close pipe write end
          // exec first command
          if (execvp(cv1[0], cv1) < 0)
            std::cout << "Command \"" << cv1[0] << "\" not found!" << std::endl;
        } else if (pipe_pid > 0) { // command after "|"
          // pipe -> input
          close(fd[1]); // close pipe write end
          dup2(fd[0], STDIN_FILENO);
          close(fd[0]); // close pipe read end
          // exec second command
          if (execvp(cv2[0], cv2) < 0)
            std::cout << "Command \"" << cv2[0] << "\" not found!" << std::endl;
        } else {
          std::cout << "Pipe Error!" << std::endl;
        }
      }

    } else if (pid > 0) { // parent process
      // child process run in foreground, parent process should wait
      if (is_background)
        signal(SIGCHLD, signal_handler);
      else
        wait(nullptr);

    } else {
      std::cout << "Error!" << std::endl;
    }
  }
  return 0;
}
