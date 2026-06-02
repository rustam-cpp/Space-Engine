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
      
      std::cout << "option name knight type spin default 300 min 200 max 400\n";
      std::cout << "option name bishop type spin default 310 min 250 max 400\n";
      std::cout << "option name rook type spin default 500 min 400 max 600\n";
      std::cout << "option name queen type spin default 900 min 800 max 1200\n";

      std::cout << "option name bishopPairBonus type spin default 30 min 0 max 80\n";
      std::cout << "option name doubledPawnPenalty type spin default 40 min 0 max 100\n";
      std::cout << "option name isolatedPawnPenalty type spin default 25 min 0 max 100\n";
      std::cout << "option name protectedPassedPawnBonus type spin default 30 min 0 max 120\n";

      std::cout << "option name noLuftPenalty type spin default 20 min 0 max 80\n";
      std::cout << "option name kingNearbyOpenFilePenalty type spin default 70 min 0 max 150\n";

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
    } else if (command.substr(0, 9) == "setoption") {
      if (command.substr(0, 25) == "setoption name Hash value") {
        std::vector<std::string> cmd = split(command);
        // don't forget to free the allocated memory
        delete TT;
        // and create a new TT
        TT = new tt(std::stoi(cmd[4]));
      } else if (command.substr(0, 28) == "setoption name Threads value") {
        std::cout << "option name Threads type spin default 1 min 1 max 1" << std::endl;
      } else {
        processSetoptionCommand(command);
      }
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