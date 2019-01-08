/*
 * Buchess, a UCI chess engine derived from Stockfish 9
 * Copyright (C) 2018-2019 Andrei Guga
 * File creation date: 2018-10-01
 * 
 * Buchess is free, and distributed under the
 * GNU General Public License Version 3 (GPLv3).
 */

#include <iostream>

#include "bitboard.h"
#include "log.h"
#include "misc.h"
#include "uci.h"

int main(int argc, char* argv[])
{
    std::cout << engine_info() << std::endl;
    
    Bitboards::init();
    
    LOG::openFile();
    LOG::log(argc, argv);
    
    UCI::loop();
    
    LOG::closeFile();
    return 0;
}
