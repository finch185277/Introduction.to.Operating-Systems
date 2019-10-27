#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <file name> <# of nums>";
    return -1;
  }
  std::ofstream outfile(argv[1]);
  std::srand(std::time(0));
  outfile << argv[2] << '\n';
  for (int i = 0; i < std::atoi(argv[2]); i++) {
    outfile << std::rand() << ' ';
  }
  outfile << '\n';
  outfile.close();
  return 0;
}
