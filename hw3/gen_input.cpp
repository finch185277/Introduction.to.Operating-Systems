#include <cstdlib>
#include <ctime>
#include <iostream>

int main() {
  int range;
  std::srand(std::time(0));
  std::cout << "Range: ";
  std::cin >> range;
  for (int i = 0; i < range; i++)
    std::cout << std::rand() << ' ';
  return 0;
}
