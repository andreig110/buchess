#include <iostream>
#include <sstream>
#include <string>
#include <ctime>    // For std::time

#include "uci.h"
#include "log.h"
#include "misc.h"
#include "movegen.h"
#include "position.h"

using namespace std;

namespace {
    
    // FEN string of the initial position, normal chess
    const char* StartFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    
    
    // position() is called when engine receives the "position" UCI command.
    // The function sets up the position described in the given FEN string ("fen")
    // or the starting position ("startpos") and then makes the moves given in the
    // following move list ("moves").
    
    void position(Position& pos, istringstream& is, StateListPtr& states)
    {
        Move m;
        string token, fen;
        
        is >> token;
        
        if (token == "startpos") {
            fen = StartFEN;
            is >> token; // Consume "moves" token if any
        }
        else if (token == "fen")
            while (is >> token && token != "moves")
                fen += token + " ";
        else
            return;
        
        states = StateListPtr(new std::deque<StateInfo>(1)); // Drop old and create a new one
        pos.set(fen, &states->back());
        
        // Parse move list (if any)
        while (is >> token && MOVEGEN::check_move(  &pos,  (m = UCI::to_move(pos, token))  )) {
            states->emplace_back();
            pos.do_move(m, states->back());
        }
    }
    
    
    // go() is called when engine receives the "go" UCI command.
    
    void go(Position& pos, istringstream& is/*, StateListPtr& states*/)
    {
        pos.update_square_attackers_count();
        
        MoveList<LEGAL> move_list = MoveList<LEGAL>(pos);
        
        std::ostringstream moves;
        moves << "moves : ";
        for (const auto& m : move_list)
            moves << UCI::move(m.move) << " ";
        LOG::log(moves.str());
        
        const ExtMove* ext_move = move_list.begin();
        int rv = rand() % move_list.size();
        ext_move += rv;
        cout << "info pv " << UCI::move(ext_move->move) << endl;
        cout << "bestmove " << UCI::move(ext_move->move) << endl;
    }
    
    
    Square get_sq(char file, char rank)
    {
        unsigned char f = file - 'a';
        unsigned char r = rank - '1';
        return Square(f, r);
    }
    
    Square from_sq(string& move)
    {
        return get_sq(move[0], move[1]);
    }
    
    Square to_sq(string& move)
    {
        return get_sq(move[2], move[3]);
    }
    
} // namespace


/// UCI::loop() waits for a command from stdin, parses it and calls the appropriate
/// function.

void UCI::loop()
{
        Position pos;
        string token, cmd;
        StateListPtr states(new std::deque<StateInfo>(1));
        
        pos.set(StartFEN, &states->back());
        
        // Use current time as seed for random generator
        std::srand(std::time(0));  // For go()
        
        do {
            if (!getline(std::cin, cmd)) // Block here waiting for input or EOF
                cmd = "quit";
            
            std::istringstream is(cmd);
            
            token.clear(); // Avoid a stale if getline() returns empty or blank line
            is >> std::skipws >> token;
            
            LOG::log("is = " + is.str());
            LOG::log("token = " + token);
            
            if (token == "quit")
                ;
            else if (token == "uci")
                std::cout << "id name " << engine_info(true)
                          << "\nuciok"  << std::endl;
            else if (token == "isready")    std::cout << "readyok" << std::endl;
            else if (token == "position")   position(pos, is, states);
            else if (token == "go")         go(pos, is/*, states*/);
            else
                std::cout << "Unknown command: " << cmd << std::endl;
        } while (token != "quit");
}


/// UCI::square() converts a Square to a string in algebraic notation (g1, a7, etc.)

std::string UCI::square(Square s)
{
    return std::string{ char('a' + s.file), char('1' + s.rank) };
}


/// UCI::move() converts a Move to a string in coordinate notation (g1f3, a7a8q).

string UCI::move(Move m/*, bool chess960*/)
{
    string move = UCI::square(m.from) + UCI::square(m.to);
    
    if (m.pawnPromotion)  // if (m.pawnPromotion != NO_PIECE_TYPE)
        move += " pnbrqk"[m.pawnPromotion];
    
    return move;
}


/// UCI::to_move() converts a string representing a move in coordinate notation
/// (g1f3, a7a8q) to the corresponding (legal) Move, (if any).

Move UCI::to_move(const Position& pos, string& str)
{
    if (str.length() == 5) // Junior could send promotion piece in uppercase
        str[4] = char(tolower(str[4]));
    
    Move m;
    m.from = from_sq(str);
    m.to = to_sq(str);
    return m;
}
