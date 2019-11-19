#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <file name> <# of nums>" << '\n';
    return -1;
  }
  std::ofstream outfile(argv[1]);
  outfile << argv[2] << '\n';
  for (int i = std::atoi(argv[2]); i > 0; i--) {
    outfile << i << ' ';
  }
  outfile << '\n';
  outfile.close();
  return 0;
}
