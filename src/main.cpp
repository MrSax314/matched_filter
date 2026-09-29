#include <config.h>
#include <iostream>

using json = nlohmann::json;

int main() {
  matched_filter::Config config;
  json j = config;
  std::cout << j.dump() << "\n";

  std::cout << "Hello World!" << std::endl;
  return 0;
}