#include <algorithm> // For std::fill_n, std::max, std::min
#include <cassert>
#include <cstddef> // For offsetof()
#include <cstring> // For std::memset, std::memcmp
#include <sstream>

#include "position.h"
#include "board.h"
#include "log.h"

using std::string;


namespace {

    const string PieceToChar(" PNBRQK  pnbrqk");

} // namespace


/// Position::set() initializes the position object with the given FEN string.
/// This function is not very robust - make sure that input FENs are correct,
/// this is assumed to be the responsibility of the GUI.

Position& Position::set(const string& fenStr, StateInfo* si)
{
/*
   A FEN string defines a particular position using only the ASCII character set.

   A FEN string contains six fields separated by a space. The fields are:

   1) Piece placement (from white's perspective). Each rank is described, starting
      with rank 8 and ending with rank 1. Within each rank, the contents of each
      square are described from file A through file H. Following the Standard
      Algebraic Notation (SAN), each piece is identified by a single letter taken
      from the standard English names. White pieces are designated using upper-case
      letters ("PNBRQK") whilst Black uses lowercase ("pnbrqk"). Blank squares are
      noted using digits 1 through 8 (the number of blank squares), and "/"
      separates ranks.

   2) Active color. "w" means white moves next, "b" means black.

   3) Castling availability. If neither side can castle, this is "-". Otherwise,
      this has one or more letters: "K" (White can castle kingside), "Q" (White
      can castle queenside), "k" (Black can castle kingside), and/or "q" (Black
      can castle queenside).

   4) En passant target square (in algebraic notation). If there's no en passant
      target square, this is "-". If a pawn has just made a 2-square move, this
      is the position "behind" the pawn. This is recorded only if there is a pawn
      in position to make an en passant capture, and if there really is a pawn
      that might have advanced two squares.

   5) Halfmove clock. This is the number of halfmoves since the last pawn advance
      or capture. This is used to determine if a draw can be claimed under the
      fifty-move rule.

   6) Fullmove number. The number of the full move. It starts at 1, and is
      incremented after Black's move.
*/
    
    unsigned char col = FILE_A, row = RANK_8, token;  // col = 0, row = 7
    size_t idx;
    std::istringstream ss(fenStr);
    
    std::memset(this, 0, sizeof(Position));
    std::memset(si, 0, sizeof(StateInfo));
    std::fill_n(&pieceList[0][0], sizeof(pieceList) / sizeof(Square_int), SQ_NONE);
    st = si;
    
    ss >> std::noskipws;
    
    // 1. Piece placement
    while ((ss >> token) && !isspace(token)) {
        if (isdigit(token))
            col += token - '0'; // Advance the given number of files
        
        else if (token == '/') {
            row -= 1;
            col = FILE_A;  // = 0
        }
        
        else if ((idx = PieceToChar.find(token)) != string::npos) {
            put_piece(Piece(idx), col, row);
            ++col;
        }
    }
    
    // 2. Active color
    ss >> token;
    sideToMove = (token == 'w' ? WHITE : BLACK);
    ss >> token;
    
    // 3. Castling availability. Compatible with 3 standards: Normal FEN standard,
    // Shredder-FEN that uses the letters of the columns on which the rooks began
    // the game instead of KQkq and also X-FEN standard that, in case of Chess960,
    // if an inner rook is associated with the castling right, the castling tag is
    // replaced by the file letter of the involved rook, as for the Shredder-FEN.
    while ((ss >> token) && !isspace(token)) {
        Square_int rsq;
        Color c = islower(token) ? BLACK : WHITE;
        Piece rook = make_piece(c, ROOK);
        
        token = char(toupper(token));
        
        if (token == 'K')
            for (rsq = relative_square(c, SQ_H1); piece_on(rsq) != rook; --rsq) {}
            
        else if (token == 'Q')
            for (rsq = relative_square(c, SQ_A1); piece_on(rsq) != rook; ++rsq) {}
        
        else if (token >= 'A' && token <= 'H')
            rsq = make_square(File(token - 'A'), relative_rank(c, RANK_1));
        
        else
            continue;
        
        set_castling_right(c, rsq);
    }
    
    // 4. En passant square. (Ignore if no pawn capture is possible)
    if (   ((ss >> col) && (col >= 'a' && col <= 'h')) 
        && ((ss >> row) && (row == '3' || row == '6'))) {
        
        st->epSquare = make_square(File(col - 'a'), Rank(row - '1'));
    }
    else
        st->epSquare = SQ_NONE;
    
    // 5-6. Halfmove clock and fullmove number
    ss >> std::skipws >> st->rule50 >> gamePly;

    // Convert from fullmove starting from 1 to gamePly starting from 0,
    // handle also common incorrect FEN with fullmove = 0.
    gamePly = std::max(2 * (gamePly - 1), 0) + (sideToMove == BLACK);
    
    set_state(st);
    
    //assert(pos_is_ok());  TODO
    
    return *this;
}

void Position::print_position() const
{
    LOG::log("Position:");
    std::ostringstream ss;
    for (int row = RANK_8; row >= RANK_1; --row) {
        for (int col = FILE_A; col <= FILE_H; ++col) {
            ss << PieceToChar[ board[col][row] ] << "\t";
        }
        LOG::log(ss.str());
        ss.str("");
    }
}


/// Position::set_castling_right() is a helper function used to set castling
/// rights given the corresponding color and the rook starting square.

void Position::set_castling_right(Color c, Square_int rfrom)
{
    Square_int kfrom = square<KING>(c);
    CastlingSide cs = kfrom < rfrom ? KING_SIDE : QUEEN_SIDE;
    CastlingRight cr = (c | cs);
    
    st->castlingRights |= cr;
    castlingRightsMask[kfrom] |= cr;
    castlingRightsMask[rfrom] |= cr;
    castlingRookSquare[cr] = rfrom;
    
    Square_int kto = relative_square(c, cs == KING_SIDE ? SQ_G1 : SQ_C1);
    Square_int rto = relative_square(c, cs == KING_SIDE ? SQ_F1 : SQ_D1);
    
    for (Square_int s = std::min(rfrom, rto); s <= std::max(rfrom, rto); ++s)
        if (s != kfrom && s != rfrom)
            castlingPath[cr] |= s;

    for (Square_int s = std::min(kfrom, kto); s <= std::max(kfrom, kto); ++s)
        if (s != kfrom && s != rfrom)
            castlingPath[cr] |= s;
}


/// Position::set_check_info() sets king attacks to detect if a move gives check

void Position::set_check_info(StateInfo* si) const
{
    for (int file = FILE_A; file <= FILE_H; ++file)
        for (int rank = RANK_1; rank <= RANK_8; ++rank) {
            Piece pc = piece_on(file, rank);
            PieceType pt = type_of(pc);
            
            if (pt == BISHOP || pt == ROOK || pt == QUEEN) {
                Color c = color_of(pc);
                VectorSquareList sqList = slider_blockers(Square(file, rank), square<KING>(~c));
                if (sqList.size() == 1)
                    si->blockersForKing[~c].addSquare( sqList.front() );
            }
        }
}


void Position::set_state(StateInfo* si) const
{
    set_check_info(si);
}


VectorSquareList Position::slider_blockers(Square from, Square to) const
{
    Piece pc = piece_on(from);
    PieceType pt = type_of(pc);
    assert(pt == BISHOP || pt == ROOK || pt == QUEEN);
    assert(from != to);
    
    VectorSquareList blockersList;
    
    if ((from.file == to.file) && (pt == ROOK || pt == QUEEN)) {
        //assert(abs(to.rank - from.rank) > 1);
        for (int r = std::min(from.rank, to.rank) + 1; r < std::max(from.rank, to.rank); ++r) {
            if (piece_on(from.file, r) != NO_PIECE)
                blockersList.addSquare(Square(from.file, r));
        }
        return blockersList;
    }
    else if ((from.rank == to.rank) && (pt == ROOK || pt == QUEEN)) {
        //assert(abs(to.file - from.file) > 1);
        for (int f = std::min(from.file, to.file) + 1; f < std::max(from.file, to.file); ++f) {
            if (piece_on(f, from.rank) != NO_PIECE)
                blockersList.addSquare(Square(f, from.rank));
        }
        return blockersList;
    }
    else if ((abs(to.file - from.file) == abs(to.rank - from.rank))  &&  (pt == BISHOP || pt == QUEEN)) {
        //assert(abs(to.file - from.file) > 1);
        /*for (   int f = std::min(from.file, to.file) + 1, r = std::min(from.rank, to.rank) + 1;
                    f < std::max(from.file, to.file)  &&  r < std::max(from.rank, to.rank);
                ++f, ++r    ) {*/       // Unfortunately, this doesn't work like that :)
        const int df = (from.file < to.file) ? +1 : -1;
        const int dr = (from.rank < to.rank) ? +1 : -1;
        for (   int f = from.file + df, r = from.rank + dr;
                f != to.file && r != to.rank;
                f += df, r += dr    ) {
            if (piece_on(f, r) != NO_PIECE)
                blockersList.addSquare(Square(f, r));
        }
        //return blockersList;
    }
    
    return blockersList;
}


/// Position::legal() tests whether a pseudo-legal move is legal

bool Position::legal(Move m) const
{
    Color us = sideToMove;
    
    // En passant captures
    if (m.to == st->epSquare) {
        // TODO
    }
    
    // A non-king move is legal if and only if it is not pinned or it
    // is moving along the ray towards or away from the king.
    return !blockers_for_king(us).contains(m.from)
        || aligned(m.from, m.to, square<KING>(us));
}


/// Position::gives_check() tests whether a pseudo-legal move gives a check

bool Position::gives_check(Move m) const  // TODO
{
    return 0;
}


/// Position::do_move() makes a move, and saves all information necessary
/// to a StateInfo object. The move is assumed to be legal. Pseudo-legal
/// moves should be filtered out before this function is called.

void Position::do_move(Move m, StateInfo& newSt, bool givesCheck)
{
    assert(&newSt != st);
    
    // Copy some fields of the old state to our new StateInfo object except the
    // ones which are going to be recalculated from scratch anyway and then switch
    // our state pointer to point to the new (ready to be updated) state.
    std::memcpy(&newSt, st, offsetof(StateInfo, key));
    newSt.previous = st;
    st = &newSt;
    
    // Increment ply counters. In particular, rule50 will be reset to zero later on
    // in case of a capture or a pawn move.
    ++gamePly;
    ++st->rule50;
    ++st->pliesFromNull;
    
    Color us = sideToMove;
    Color them = ~us;
    Piece pc = piece_on(m.from);
    bool m_en_passant = m.to == st->epSquare;  // type_of(m.flags) == ENPASSANT
    Piece captured = m_en_passant ? make_piece(them, PAWN) : piece_on(m.to);
    
    if (type_of(m.flags) == CASTLING) {
        do_castling<true>(us, m.from, m.to);
        captured = NO_PIECE;
    }
    
    if (captured) {
        Square capsq = m.to;
        
        if (type_of(captured) == PAWN) {
            if (m_en_passant) {
                capsq = capsq - pawn_push(us);
                board[capsq.file][capsq.rank] = NO_PIECE; // Not done by remove_piece()
            }
        }
        
        // Update board and piece lists
        remove_piece(captured, capsq);
        
        // Reset rule 50 counter
        st->rule50 = 0;
    }
    
    // Reset en passant square
    //if (st->epSquare != SQ_NONE)
        st->epSquare = SQ_NONE;
    
    // Update castling rights if needed
    if (st->castlingRights && (castlingRightsMask[m.from] | castlingRightsMask[m.to])) {
        int cr = castlingRightsMask[m.from] | castlingRightsMask[m.to];
        st->castlingRights &= ~cr;
    }
    
    // Move the piece. The tricky Chess960 castling is handled earlier
    if (type_of(m.flags) != CASTLING)
        move_piece(m.from, m.to);
    
    // If the moving piece is a pawn do some special extra work
    if (type_of(pc) == PAWN) {
        // Set en-passant square
        if ( (int(m.to) ^ int(m.from)) == 16 )
            st->epSquare = m.to - pawn_push(us);
        else if (type_of(m.flags) == PROMOTION) {
            Piece promotion = make_piece(us, promotion_type(m.flags));
            remove_piece(pc, m.to);
            put_piece(promotion, m.to.file, m.to.rank);
        }
        
        // Reset rule 50 draw counter
        st->rule50 = 0;
    }
    
    // Set capture piece
    st->capturedPiece = captured;
    
    sideToMove = ~sideToMove;
    
    // Update king attacks used for fast check detection
    set_check_info(st);
}


/// Position::do_castling() is a helper used to do/undo a castling move. This
/// is a bit tricky in Chess960 where from/to squares can overlap.
template<bool Do>
void Position::do_castling(Color us, Square from, Square& to/*, Square& rfrom, Square& rto*/)
{
    bool kingSide = to > from;
    Square rfrom = to; // Castling is encoded as "king captures friendly rook"
    Square rto = relative_square(us, kingSide ? SQ_F1 : SQ_D1);
    to = relative_square(us, kingSide ? SQ_G1 : SQ_C1);

    // Remove both pieces first since squares could overlap in Chess960
    remove_piece(make_piece(us, KING), Do ? from : to);
    remove_piece(make_piece(us, ROOK), Do ? rfrom : rto);
    board[Do ? from.file : to.file][Do ? from.rank : to.rank] =
        board[Do ? rfrom.file : rto.file][Do ? rfrom.rank : rto.rank] = NO_PIECE; // Since remove_piece doesn't do it for us
    put_piece(make_piece(us, KING), Do ? to : from);
    put_piece(make_piece(us, ROOK), Do ? rto : rfrom);
}

void Position::update()
{
    update_squares_attackers_count();
    update_attacked_king_squares();
}

void Position::update_squares_attackers_count()
{
    Square ksq = square<KING>(sideToMove);
    for (int f = 0; f < 8; ++f)
        for (int r = 0; r < 8; ++r) {
            const Piece pc = board[f][r];
            if (pc != NO_PIECE) {
                const SquareList list = figure_attacks_from(type_of(pc), *this, f, r);
                for (const auto& sq : list) {
                    ++squares_attackers_count[color_of(pc)][sq.file][sq.rank];
                    if ((color_of(pc) != sideToMove) && (sq == ksq))
                        st->checkers.addSquare(Square(f, r));
                }
            }
        }
}

void Position::print_squares_attackers_count()
{
    std::ostringstream ss;
    for (int c = 0; c < COLOR_NB; ++c) {
        ss << "squares_attackers_count[" << c << "] :" << std::endl;
        for (int r = RANK_8; r >= RANK_1; --r) {
            for (int f = FILE_A; f <= FILE_H; ++f) {
                ss << static_cast<unsigned int> (squares_attackers_count[c][f][r]) << "\t";
            }
            LOG::log(ss.str());
            ss.str("");
        }
    }
}

void Position::update_attacked_king_squares()
{
    for (int f = 0; f < 8; ++f)
        for (int r = 0; r < 8; ++r) {
            const Piece pc = board[f][r];
            const PieceType pt = type_of(pc);
            if (pt == BISHOP || pt == ROOK || pt == QUEEN) {
                if (color_of(pc) != sideToMove)
                    figure_attacks_behind_king_from(pt, *this, f, r, &attacked_king_squares);
            }
        }
}
