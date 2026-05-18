#include "board.h"
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
  std::cout << "          |                v1.0 " << std::endl;
  std::cout << std::endl;
  std::cout << "AUTHOR: rustam-cpp" << std::endl;
  std::cout << "SOURCE CODE: https://github.com/rustam-cpp/Space-Engine" << std::endl;
  std::cout << std::endl;
}

int main() {

  info();

  // board
  Position pos;
  pos.convertFromFen(StartFen);

  // transposition table
  tt* TT = new tt(256);

  // freopen("log.txt", "w", stderr);

  while (true) {
    std::string command;
    // getline helps to enter string with spaces
    getline(std::cin, command);

    // std::cerr << command << '\n';

    if (command == "uci") {
      std::cout << "id name Space 1.0\n";
      std::cout << "id author rustam-cpp\n";
      std::cout << "option name Hash type spin default 256 min 1 max 1024\n";
      std::cout << "option name Threads type spin default 1 min 1 max 1\n";
      std::cout << "uciok" << std::endl;
    } else if (command == "isready") {
      std::cout << "readyok" << std::endl;
    } else if (command == "ucinewgame") {
      // after each ucinewgame we can clear our TT
      TT->clear();
    } else if (command.substr(0, 8) == "position") {
      processPositionCommand(pos, command);
    } else if (command.substr(0, 2) == "go") {
      std::thread go(processGoCommand, std::ref(pos), TT, command);
      go.detach();
    } else if (command == "stop") {
      stopCommand();
    } else if (command == "bench") {
      Position startpos;
      startpos.convertFromFen(StartFen);
      tt* temp = new tt(8);
      int64_t nodes = iterative_depening(startpos, temp, 5, BIG_INF, BIG_INF).second;
      std::cout << nodes << std::endl;
      delete temp;
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