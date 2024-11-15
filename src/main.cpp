#include "../include/simulator.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Assume the Instruction class and other code from previous examples are
// already defined

int main(int argc, char *argv[]) {
  // Check if the correct number of arguments are provided
  if (argc != 4) {
    std::cerr << "Usage: ./ooosim <N> <S> <trace_file>" << std::endl;
    return 1;
  }

  // Parse the command-line arguments
  int N = std::stoi(argv[1]);       // Superscalar bandwidth
  int S = std::stoi(argv[2]);       // Schedule queue size
  std::string trace_file = argv[3]; // Trace file path

  OOOE my_sim(N, S, trace_file);

  do {
    my_sim.retire();
    my_sim.execute();
    my_sim.issue();
    my_sim.dispatch();
    my_sim.fetch();
  } while (my_sim.advance_cycle());
  my_sim.print_output();

  // Print the parsed values (for debugging purposes)
  std::cout << "Superscalar Bandwidth (N): " << N << std::endl;
  std::cout << "Schedule Queue Size (S): " << S << std::endl;
  std::cout << "Trace File: " << trace_file << std::endl;

  return 0;
}
