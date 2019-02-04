#ifndef POSITION_H_INCLUDED
#define POSITION_H_INCLUDED

#include <cassert>
#include <deque>
#include <memory> // For std::unique_ptr
#include <string>

#include "bitboard.h"
#include "types.h"


/// StateInfo struct stores information needed to restore a Position object to
/// its previous state when we retract a move. Whenever a move is made on the
/// board (by calling Position::do_move), a StateInfo object must be passed.

struct StateInfo {
    
    // Copied when making a move
    int    castlingRights;
    int    rule50;
    int    pliesFromNull;
    Square_int epSquare;
    
    // Not copied when making a move (will be recomputed anyhow)
    Key        key;  // TODO
    VectorSquareList checkers;
    Piece      capturedPiece;
    StateInfo* previous;
    VectorSquareList blockersForKing[COLOR_NB];
    //VectorSquareList pinners[COLOR_NB];
    VectorSquareList checkSquares[PIECE_TYPE_NB];
};

/// A list to keep track of the position states along the setup moves (from the
/// start position to the position just before the search starts). Needed by
/// 'draw by repetition' detection. Use a std::deque because pointers to
/// elements are not invalidated upon list resizing.
typedef std::unique_ptr<std::deque<StateInfo>> StateListPtr;


class Position {
public:
    Position() = default;
    Position(const Position&) = delete;
    Position& operator=(const Position&) = delete;
    
    // FEN string input/output
    Position& set(const std::string& fenStr, StateInfo* si);
    void print_position() const;
    
    // Position representation
    Bitboard pieces() const;
    Bitboard pieces(PieceType pt) const;
    Bitboard pieces(Color c) const;
    Bitboard pieces(Color c, PieceType pt) const;
    Piece piece_on(Square s) const;
    Piece piece_on(int file, int rank) const;  // new 2018-12-01
    Piece piece_on(Square_int s) const;
    Square_int ep_square() const;
    template<PieceType Pt> Square_int square(Color c) const;
    
    // Castling
    int can_castle(Color c) const;
    int can_castle(CastlingRight cr) const;
    bool castling_impeded(CastlingRight cr) const;
    Square_int castling_rook_square(CastlingRight cr) const;
    
    // Checking
    VectorSquareList checkers() const;
    VectorSquareList blockers_for_king(Color c) const;
    VectorSquareList check_squares(PieceType pt) const;
    bool in_check() const;  // new 2019-01-07
    
    // Attacks to/from a given square
    VectorSquareList slider_blockers(Square from, Square to) const;
    
    // Properties of moves
    bool legal(Move m) const;
    bool gives_check(Move m) const;
    Piece captured_piece() const;
    
    // Doing moves
    void do_move(Move m, StateInfo& newSt);
    void do_move(Move m, StateInfo& newSt, bool givesCheck);
    
    // Other properties of the position
    Color side_to_move() const;
    int game_ply() const;
    int rule50_count() const;
    
    unsigned char get_square_attackers_count(Color color, int file, int rank) const;
    bool is_king_square_attacked(int file, int rank) const;
    void update();
  
private:
    // Initialization helpers (used while setting up a position)
    void set_castling_right(Color c, Square_int rfrom);
    void set_check_info(StateInfo* si) const;
    void set_state(StateInfo* si) const;
    
    void inc_square_attackers_count(Color color, int file, int rank);
    void update_squares_attackers_count();
    void print_squares_attackers_count();  // for debugging
    void update_attacked_king_squares();
    
    // Other helpers
    void put_piece(Piece pc, /*Square s*/ int file, int rank);  // Optimization: do not create a temporary object 'Square'
    void move_piece(/*Piece pc,*/ Square from, Square to);
    
    // Data members
    Piece board [8][8] = { { NO_PIECE } };
    Bitboard byTypeBB[PIECE_TYPE_NB];
    Bitboard byColorBB[COLOR_NB];
    int pieceCount[PIECE_NB];
    Square_int pieceList[PIECE_NB][16];
    int index[SQUARE_NB];
    int castlingRightsMask[SQUARE_NB];
    Square_int castlingRookSquare[CASTLING_RIGHT_NB];
    Bitboard castlingPath[CASTLING_RIGHT_NB];
    int gamePly;
    Color sideToMove;
    StateInfo* st;
    unsigned char squares_attackers_count [COLOR_NB][8][8] = { { { 0 } } };  // calculated after UCI "go" command
    VectorSquareList attacked_king_squares;  // Attacked squares behind king (by bishop, rook or queen)
};


inline Color Position::side_to_move() const
{
    return sideToMove;
}

inline Piece Position::piece_on(Square s) const
{
    return board[s.file][s.rank];
}

inline Piece Position::piece_on(int file, int rank) const
{
    return board[file][rank];
}

inline Piece Position::piece_on(Square_int s) const
{
    return board [file_of(s)] [rank_of(s)];
}

inline Bitboard Position::pieces() const
{
    return byTypeBB[ALL_PIECES];
}

inline Bitboard Position::pieces(PieceType pt) const
{
    return byTypeBB[pt];
}

inline Bitboard Position::pieces(Color c) const
{
    return byColorBB[c];
}

inline Bitboard Position::pieces(Color c, PieceType pt) const
{
    return byColorBB[c] & byTypeBB[pt];
}

inline Square_int Position::ep_square() const
{
    return st->epSquare;
}

template<PieceType Pt> inline Square_int Position::square(Color c) const
{
    assert(pieceCount[make_piece(c, Pt)] == 1);
    return pieceList[make_piece(c, Pt)][0];
}

inline int Position::can_castle(CastlingRight cr) const
{
    return st->castlingRights & cr;
}

inline int Position::can_castle(Color c) const
{
    return st->castlingRights & ((WHITE_OO | WHITE_OOO) << (2 * c));
}

inline bool Position::castling_impeded(CastlingRight cr) const
{
    return byTypeBB[ALL_PIECES] & castlingPath[cr];
}

inline Square_int Position::castling_rook_square(CastlingRight cr) const
{
    return castlingRookSquare[cr];
}

inline void Position::inc_square_attackers_count(Color color, int file, int rank)
{
    ++squares_attackers_count[color][file][rank];
}

inline unsigned char Position::get_square_attackers_count(Color color, int file, int rank) const
{
    return squares_attackers_count[color][file][rank];
}

inline bool Position::is_king_square_attacked(int file, int rank) const
{
    return attacked_king_squares.contains(Square(file, rank));
}

inline VectorSquareList Position::checkers() const {
    return st->checkers;
}

inline VectorSquareList Position::blockers_for_king(Color c) const
{
    return st->blockersForKing[c];
}

inline VectorSquareList Position::check_squares(PieceType pt) const
{
    return st->checkSquares[pt];
}

inline bool Position::in_check() const
{
    const Square_int king_sq = square<KING>(sideToMove);
    return squares_attackers_count [~sideToMove] [file_of(king_sq)] [rank_of(king_sq)];
}

inline int Position::game_ply() const
{
    return gamePly;
}

inline int Position::rule50_count() const
{
    return st->rule50;
}

inline Piece Position::captured_piece() const
{
    return st->capturedPiece;
}

inline void Position::put_piece(Piece pc, /*Square s*/ int file, int rank)
{
    board[file][rank] = pc;
    const Square_int s = make_square(static_cast<File>(file), static_cast<Rank>(rank));
    byTypeBB[ALL_PIECES] |= s;
    byTypeBB[type_of(pc)] |= s;
    byColorBB[color_of(pc)] |= s;
    index[s] = pieceCount[pc]++;
    pieceList[pc][index[s]] = s;
    //pieceCount[make_piece(color_of(pc), ALL_PIECES)]++;
}

inline void Position::move_piece(/*Piece pc,*/ Square from, Square to)
{
    const Piece pc = board[from.file][from.rank];
    board[to.file][to.rank] = pc;
    board[from.file][from.rank] = NO_PIECE;
    
    // index[from] is not updated and becomes stale. This works as long as index[]
    // is accessed just by known occupied squares.
    Bitboard fromTo = SquareBB[from] ^ SquareBB[to];
    byTypeBB[ALL_PIECES] ^= fromTo;
    byTypeBB[type_of(pc)] ^= fromTo;
    byColorBB[color_of(pc)] ^= fromTo;
    
    index[to] = index[from];
    pieceList[pc][index[to]] = to;
}

inline void Position::do_move(Move m, StateInfo& newSt)
{
    do_move(m, newSt, gives_check(m));
}

#endif // #ifndef POSITION_H_INCLUDED
