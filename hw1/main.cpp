/*
Student No.: 0616214
Student Name: YU-AN CHEN
Email: 59487andy@gmail.com
Statement: I am fully aware that this program is not suppposed to be posted to a
public server, such as a public GitHub repostiory or a public web page.
*/

#include <cstring> // std::strtok()
#include <iostream>
#include <sys/types.h> // wait()
#include <sys/wait.h>  // wait()
#include <unistd.h>    // fork()
#include <vector>

int main(int argc, char *argv[]) {
  while (true) {
    pid_t pid;
    pid = fork();
    if (pid == 0) {
      std::cout << ">";

      // read input line by line
      std::string line, file;
      std::getline(std::cin, line);

      // parse the line
      char *cv[100];                                  // command in char* array
      int cv_idx = 0;                                 // cv current index
      std::vector<char> lv(line.begin(), line.end()); // line in vector
      lv.push_back('\0');
      char *token = std::strtok(&*lv.begin(), " ");
      while (token != nullptr) {
        cv[cv_idx++] = token;
        token = std::strtok(nullptr, " ");
      }

      // execute the command
      execvp(cv[0], cv);

      exit(0);
    } else if (pid > 0) {
      wait(0);
    } else {
      std::cout << "Error!" << std::endl;
    }
  }
  return 0;
}
