#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#include <algorithm>  // For std::find
#include <cassert>
#include <cstddef> // For size_t
#include <cstdint> // For uint64_t
#include <vector>

typedef uint64_t Key;
typedef uint64_t Bitboard;

constexpr int MAX_MOVES = 256;
constexpr int MAX_PLY   = 128;

enum Color {
    WHITE, BLACK, COLOR_NB = 2
};

enum CastlingSide {
    KING_SIDE, QUEEN_SIDE, CASTLING_SIDE_NB = 2
};

enum CastlingRight {
    NO_CASTLING,
    WHITE_OO,
    WHITE_OOO = WHITE_OO << 1,
    BLACK_OO  = WHITE_OO << 2,
    BLACK_OOO = WHITE_OO << 3,
    ANY_CASTLING = WHITE_OO | WHITE_OOO | BLACK_OO | BLACK_OOO,
    CASTLING_RIGHT_NB = 16
};

enum PieceType {
    NO_PIECE_TYPE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
    ALL_PIECES = 0,
    PIECE_TYPE_NB = 8
};

enum Piece {
    NO_PIECE,
    W_PAWN = 1, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
    B_PAWN = 9, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
    PIECE_NB = 16
};

enum Square_int : int {
    SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
    SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
    SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
    SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
    SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
    SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
    SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
    SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
    SQ_NONE,

    SQUARE_NB = 64
};

enum Direction : int {
    NORTH =  8,
    EAST  =  1,
    SOUTH = -NORTH,
    WEST  = -EAST,

    NORTH_EAST = NORTH + EAST,
    SOUTH_EAST = SOUTH + EAST,
    SOUTH_WEST = SOUTH + WEST,
    NORTH_WEST = NORTH + WEST
};

enum File : int {
    FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NB
};

enum Rank : int {
    RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NB
};

struct Square {
    Square() { file = FILE_A; rank = RANK_1; }
    Square(File f, Rank r) { file = f; rank = r; }
    Square(int f, int r) { file = static_cast<File>(f); rank = static_cast<Rank>(r); }
    Square(Square_int sqi) { file = File(sqi & 7); rank = Rank(sqi >> 3); }
    operator Square_int() const { return Square_int((rank << 3) + file); }    // same as make_square()
    File file;
    Rank rank;
};

struct Move {
    Move() = default;
    Move(Square f, Square t)  { from = f; to = t; }
    Move(Square f, Square t, PieceType prom)  { from = f; to = t; pawnPromotion = prom; }
    Square from;
    Square to;
    PieceType pawnPromotion = NO_PIECE_TYPE;
};

struct SquareList {
    void addSquare(Square sq) { *last++ = sq; }
    void push(SquareList& other)
    {
        for (const auto& sq : other)
            *last++ = sq;
    }
    const Square* begin() const { return squareList; }
    const Square* end() const { return last; }
    size_t size() const { return last - squareList; }
private:
    Square squareList[SQUARE_NB], *last = &squareList[0];
};

struct VectorSquareList {
    VectorSquareList() = default;
    VectorSquareList(Square sq) { squareList.push_back(sq); }
    void addSquare(Square sq)
    {
        if (std::find(squareList.cbegin(), squareList.cend(), sq) == squareList.cend())  // Push only unique values
            squareList.push_back(sq);
    }
    bool contains(Square sq) const
    {
        return std::find(squareList.cbegin(), squareList.cend(), sq) != squareList.cend();
    }
    Square front() const { return squareList.front(); }
    size_t size() const { return squareList.size(); }
    void clear() { squareList.clear(); }
private:
    std::vector<Square> squareList;
};


#define ENABLE_INCR_OPERATORS_ON(T)                                \
inline T& operator++(T& d) { return d = T(int(d) + 1); }           \
inline T& operator--(T& d) { return d = T(int(d) - 1); }

ENABLE_INCR_OPERATORS_ON(Square_int)
ENABLE_INCR_OPERATORS_ON(File)
ENABLE_INCR_OPERATORS_ON(Rank)

#undef ENABLE_INCR_OPERATORS_ON


/// Additional operators to add a Direction to a Square_int
constexpr Square_int operator+(Square_int s, Direction d) { return Square_int(int(s) + int(d)); }
constexpr Square_int operator-(Square_int s, Direction d) { return Square_int(int(s) - int(d)); }
inline Square_int& operator+=(Square_int& s, Direction d) { return s = s + d; }
inline Square_int& operator-=(Square_int& s, Direction d) { return s = s - d; }

constexpr Color operator~(Color c) {
    return Color(c ^ BLACK);  // Toggle color
}

constexpr CastlingRight operator|(Color c, CastlingSide s) {
    return CastlingRight(WHITE_OO << ((s == QUEEN_SIDE) + 2 * c));
}

template<Color C, CastlingSide S>
constexpr CastlingRight make_castling() {
    return C == WHITE   ? S == QUEEN_SIDE ? WHITE_OOO : WHITE_OO
                        : S == QUEEN_SIDE ? BLACK_OOO : BLACK_OO;
}

constexpr Square_int make_square(File f, Rank r) {
    return Square_int((r << 3) + f);
}

constexpr Piece make_piece(Color c, PieceType pt) {
    return Piece((c << 3) + pt);
}

constexpr PieceType type_of(Piece pc) {
    return PieceType(pc & 7);
}

inline Color color_of(Piece pc) {
    assert(pc != NO_PIECE);
    return Color(pc >> 3);
}

constexpr File file_of(Square_int s) {
    return File(s & 7);
}

constexpr Rank rank_of(Square_int s) {
    return Rank(s >> 3);
}

constexpr Square_int relative_square(Color c, Square_int s) {
    return Square_int(s ^ (c * 56));
}

constexpr Rank relative_rank(Color c, Rank r) {
    return Rank(r ^ (c * 7));
}

#endif // #ifndef TYPES_H_INCLUDED
