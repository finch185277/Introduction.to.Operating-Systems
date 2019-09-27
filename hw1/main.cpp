/*
Student No.: 0616214
Student Name: YU-AN CHEN
Email: 59487andy@gmail.com
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not
suppposed to be posted to a public server, such as a
public GitHub repostiory or a public web page.
*/

#include <cstring> // std::strtok()
#include <iostream>
#include <sys/types.h> // wait()
#include <sys/wait.h>  // wait()
#include <unistd.h>    // fork()
#include <vector>

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
    char *cv[100];  // command in char* array
    int cv_idx = 0; // cv current index
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
      cv[cv_idx - 1] = nullptr; // change "&" to nullptr
    } else {
      cv[cv_idx++] = nullptr; // put a nullptr to the last place
    }

    // end the program when input "exit"
    if (strcmp(cv[0], "exit") == 0) {
      is_continue = false;
      break;
    }

    // ignore background child return signal to prevent Zombie Process
    signal(SIGCHLD, SIG_IGN);

    pid_t pid;
    pid = fork();
    if (pid == 0) { // child process
      // execute the command
      if (execvp(cv[0], cv) < 0)
        std::cout << "Command \"" << cv[0] << "\" not found!" << std::endl;
    } else if (pid > 0) { // parent process
      // child process run in foreground, parent process should wait
      if (!is_background)
        wait(nullptr);
    } else {
      std::cout << "Error!" << std::endl;
    }
  }
  return 0;
}
