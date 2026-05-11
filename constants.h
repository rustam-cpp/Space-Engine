#pragma once

#include "stdint.h"
#include "types.h"
#include <string>

const Eval Mate = 32'000;

// the evaluation we can't reach
const Eval INF = 32'767;
const int64_t BIG_INF = 9'000'000'000'000'000'000;

const std::string StartFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";