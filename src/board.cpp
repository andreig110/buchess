#include "board.h"
#include "position.h"

SquareList pawn_attacks_from(const Position& pos, int file, int rank)
{
    int up;
    switch (color_of(pos.piece_on(file, rank))) {
        case WHITE:
            up = 1;
            break;
        case BLACK:
            up = -1;
            break;
    }
    SquareList squareList;
    if (file > FILE_A)
        squareList.addSquare( Square(file - 1, rank + up) );
    if (file < FILE_H)
        squareList.addSquare( Square(file + 1, rank + up) );
    return squareList;
}

SquareList knight_attacks_from(int file, int rank)
{
    SquareList squareList;
    
    if (file > FILE_B) {
        if (rank > RANK_1)
            squareList.addSquare( Square(file - 2, rank - 1) );
        if (rank < RANK_8)
            squareList.addSquare( Square(file - 2, rank + 1) );
    }
    if (file > FILE_A) {
        if (rank > RANK_2)
            squareList.addSquare( Square(file - 1, rank - 2) );
        if (rank < RANK_7)
            squareList.addSquare( Square(file - 1, rank + 2) );
    }
    if (file < FILE_H) {
        if (rank > RANK_2)
            squareList.addSquare( Square(file + 1, rank - 2) );
        if (rank < RANK_7)
            squareList.addSquare( Square(file + 1, rank + 2) );
    }
    if (file < FILE_G) {
        if (rank > RANK_1)
            squareList.addSquare( Square(file + 2, rank - 1) );
        if (rank < RANK_8)
            squareList.addSquare( Square(file + 2, rank + 1) );
    }
    
    return squareList;
}

SquareList rook_attacks_from(const Position& pos, int file, int rank)
{
    SquareList squareList;
    
    for (int to_rank = rank + 1; to_rank <= RANK_8; ++to_rank) {
        squareList.addSquare( Square(file, to_rank) );
        if (pos.piece_on(file, to_rank) != NO_PIECE)
            break;
    }
    for (int to_rank = rank - 1; to_rank >= RANK_1; --to_rank) {
        squareList.addSquare( Square(file, to_rank) );
        if (pos.piece_on(file, to_rank) != NO_PIECE)
            break;
    }
    
    for (int to_file = file + 1; to_file <= FILE_H; ++to_file) {
        squareList.addSquare( Square(to_file, rank) );
        if (pos.piece_on(to_file, rank) != NO_PIECE)
            break;
    }
    for (int to_file = file - 1; to_file >= FILE_A; --to_file) {
        squareList.addSquare( Square(to_file, rank) );
        if (pos.piece_on(to_file, rank) != NO_PIECE)
            break;
    }
    
    return squareList;
}

SquareList bishop_attacks_from(const Position& pos, int file, int rank)
{
    SquareList squareList;
    
    for (int to_file = file + 1, to_rank = rank + 1; to_file <= FILE_H && to_rank <= RANK_8; ++to_file, ++to_rank) {
        squareList.addSquare( Square(to_file, to_rank) );
        if (pos.piece_on(to_file, to_rank) != NO_PIECE)
            break;
    }
    for (int to_file = file - 1, to_rank = rank - 1; to_file >= FILE_A && to_rank >= RANK_1; --to_file, --to_rank) {
        squareList.addSquare( Square(to_file, to_rank) );
        if (pos.piece_on(to_file, to_rank) != NO_PIECE)
            break;
    }
    
    for (int to_file = file + 1, to_rank = rank - 1; to_file <= FILE_H && to_rank >= RANK_1; ++to_file, --to_rank) {
        squareList.addSquare( Square(to_file, to_rank) );
        if (pos.piece_on(to_file, to_rank) != NO_PIECE)
            break;
    }
    for (int to_file = file - 1, to_rank = rank + 1; to_file >= FILE_A && to_rank <= RANK_8; --to_file, ++to_rank) {
        squareList.addSquare( Square(to_file, to_rank) );
        if (pos.piece_on(to_file, to_rank) != NO_PIECE)
            break;
    }

    return squareList;
}

SquareList queen_attacks_from(const Position& pos, int file, int rank)
{
    SquareList squareList = rook_attacks_from(pos, file, rank);
    SquareList tempList = bishop_attacks_from(pos, file, rank);
    squareList.push(tempList);
    return squareList;
}

SquareList king_attacks_from(int file, int rank)
{
    SquareList squareList;
    
    if (file > FILE_A) {
        if (rank > RANK_1)
            squareList.addSquare( Square(file - 1, rank - 1) );
        squareList.addSquare( Square(file - 1, rank) );
        if (rank < RANK_8)
            squareList.addSquare( Square(file - 1, rank + 1) );
    }
    if (rank > RANK_1)
        squareList.addSquare( Square(file, rank - 1) );
    if (rank < RANK_8)
        squareList.addSquare( Square(file, rank + 1) );
    if (file < FILE_H) {
        if (rank > RANK_1)
            squareList.addSquare( Square(file + 1, rank - 1) );
        squareList.addSquare( Square(file + 1, rank) );
        if (rank < RANK_8)
            squareList.addSquare( Square(file + 1, rank + 1) );
    }
    
    return squareList;
}


//template <PieceType Pt>
SquareList figure_attacks_from(const PieceType Pt, const Position& pos, int file, int rank)
{
    switch (Pt) {
        case PAWN:
            return pawn_attacks_from(pos, file, rank);
            break;
        case KNIGHT:
            return knight_attacks_from(file, rank);
            break;
        case BISHOP:
            return bishop_attacks_from(pos, file, rank);
            break;
        case ROOK:
            return rook_attacks_from(pos, file, rank);
            break;
        case QUEEN:
            return queen_attacks_from(pos, file, rank);
            break;
        case KING:
            return king_attacks_from(file, rank);
            break;
        default:
            break;
    }
}


namespace {
    
    void rook_attacks_behind_king_from(const Position& pos, int file, int rank, VectorSquareList* asbk)
    {
        const Piece ourKing = make_piece(pos.side_to_move(), KING);
        
        for (int to_rank = rank + 1; to_rank <= RANK_8; ++to_rank) {
            const Piece pc = pos.piece_on(file, to_rank);
            if (pc == ourKing  &&  ++to_rank <= RANK_8)  // it does not check if the square (behind king) is occupied
                asbk->addSquare(Square(file, to_rank));
            if (pc != NO_PIECE)
                break;
        }
        for (int to_rank = rank - 1; to_rank >= RANK_1; --to_rank) {
            const Piece pc = pos.piece_on(file, to_rank);
            if (pc == ourKing  &&  --to_rank >= RANK_1)
                asbk->addSquare(Square(file, to_rank));
            if (pc != NO_PIECE)
                break;
        }
        
        for (int to_file = file + 1; to_file <= FILE_H; ++to_file) {
            const Piece pc = pos.piece_on(to_file, rank);
            if (pc == ourKing  &&  ++to_file <= FILE_H)
                asbk->addSquare(Square(to_file, rank));
            if (pc != NO_PIECE)
                break;
        }
        for (int to_file = file - 1; to_file >= FILE_A; --to_file) {
            const Piece pc = pos.piece_on(to_file, rank);
            if (pc == ourKing  &&  --to_file <= FILE_A)
                asbk->addSquare(Square(to_file, rank));
            if (pc != NO_PIECE)
                break;
        }
    }
    
}  // namespace


VectorSquareList* figure_attacks_behind_king_from(const PieceType Pt, const Position& pos, int file, int rank, VectorSquareList* asbk)
{
    switch (Pt) {
    case BISHOP:
        break;
    case ROOK:
        rook_attacks_behind_king_from(pos, file, rank, asbk);
        break;
    case QUEEN:
        break;
    }
    
    return asbk;
}
