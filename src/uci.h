#pragma once

#include "board.h"
#include "tt.h"
#include "timeman.h"
#include <string>
#include <vector>

// splits a big string into small ones after each space
// example: "position startpos" -> {"position", "startpos"}
std::vector<std::string> split(std::string command);

// position command
void processPositionCommand(Position* pos, rt* RT, std::string command);

// go command
void processGoCommand(Position* pos, tt* TT, rt* RT, std::string command);

void processBenchCommand(Position* pos);

void processPerftTestCommand();