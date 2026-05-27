#include "board.h"
#include "constants.h"
#include "tt.h"
#include "uci.h"
#include "evaluation.h"
#include "search.h"
#include <iostream>
#include <thread>

void info() {
  std::cout << std::endl;
  std::cout << "   .---.  " << std::endl;
  std::cout << "  (     ` " << std::endl;
  std::cout << "   `---.  .--.  .--.  .--. .--. " << std::endl;
  std::cout << "        ) |   | |  |  |    |--' " << std::endl;
  std::cout << "  `.___.` |__.` '__|_ '__' '__' " << std::endl;
  std::cout << "          | " << std::endl;
  std::cout << "          | " << std::endl;
  std::cout << std::endl;
  std::cout << "AUTHOR: rustam-cpp" << std::endl;
  std::cout << "SOURCE CODE: https://github.com/rustam-cpp/Space-Engine" << std::endl;
  std::cout << std::endl;
}

int main(int argc, char* argv[]) {

  rt* RT = new rt;

  // ./space bench
  if (argc == 2) {
    std::string cmd = argv[1];
    processBenchCommand();
    return 0;
  }

  info();

  // board
  Position pos;
  pos.convertFromFen(StartFen, RT);

  // transposition table
  tt* TT = new tt(256);

  // freopen("log.txt", "w", stderr);

  while (true) {
    std::string command;
    // getline helps to enter string with spaces
    getline(std::cin, command);

    // std::cerr << command << '\n';

    if (command == "uci") {
      std::cout << "id name Space\n";
      std::cout << "id author rustam-cpp\n";
      std::cout << "option name Hash type spin default 256 min 1 max 1024\n";
      std::cout << "option name Threads type spin default 1 min 1 max 1\n";
      std::cout << "uciok" << std::endl;
    } else if (command == "isready") {
      std::cout << "readyok" << std::endl;
    } else if (command == "ucinewgame") {
      // after each ucinewgame we can clear our TT
      TT->clear();
      RT->clear();
    } else if (command.substr(0, 8) == "position") {
      processPositionCommand(pos, RT, command);
    } else if (command.substr(0, 2) == "go") {
      std::thread go(processGoCommand, std::ref(pos), TT, RT, command);
      go.detach();
    } else if (command == "stop") {
      stopCommand();
    } else if (command == "bench") {
      processBenchCommand();
    } else if (command == "perfttest") {
      processPerftTestCommand();
    } else if (command.substr(0, 25) == "setoption name Hash value") {
      std::vector<std::string> cmd = split(command);
      // don't forget to free the allocated memory
      delete TT;
      // and create a new TT
      TT = new tt(std::stoi(cmd[4]));
    } else if (command.substr(0, 28) == "setoption name Threads value") {
      std::cout << "option name Threads type spin default 1 min 1 max 1" << std::endl;
    } else if (command == "quit") {
      break;
    } else if (command == "eval") {
      std::cout << evaluation(pos) << '\n';
    } else if (command == "d") {
      // debug command
      std::cout << std::endl << pos << std::endl;
    } else {
      std::cout << "Unsupported command / invalid UCI command" << std::endl;
    }
  }

}