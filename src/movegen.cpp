#include "movegen.h"
#include "board.h"
#include "position.h"


namespace {
    
    template<CastlingRight Cr>
    ExtMove* generate_castling(const Position& pos, ExtMove* moveList, Color us)
    {
        constexpr bool KingSide = (Cr == WHITE_OO || Cr == BLACK_OO);
        
        if (pos.castling_impeded(Cr) || !pos.can_castle(Cr))
            return moveList;
        
        // After castling, the rook and king final positions are the same in Chess960
        // as they would be in standard chess.
        Square_int kfrom = pos.square<KING>(us);
        Square_int rfrom = pos.castling_rook_square(Cr);
        Square_int kto = relative_square(us, KingSide ? SQ_G1 : SQ_C1);
        
        //assert(!pos.checkers());
        assert(!pos.in_check());
        
        const Direction step = KingSide ? WEST : EAST;
        
        for (Square_int s = kto; s != kfrom; s += step)
            if (pos.get_square_attackers_count(~us, file_of(s), rank_of(s)))  // new 2019-01-06
                return moveList;
        
        //Move m = make<CASTLING>(kfrom, rfrom);
        /* TODO: decide how to encode castling:
         * "kfrom -> kto"  or  "kfrom -> rfrom" (king captures rook) ? */
        Move m ( Square(file_of(kfrom), rank_of(kfrom)), Square(file_of(kto), rank_of(kto)) );
        
        *moveList++ = m;
        return moveList;
    }
    
    template<Color Us>
    ExtMove* check_generate_pawn_capture(const Position& pos, ExtMove* moveList, int ourRank7, int file, int rank, int to_file, int to_rank)
    {
        Piece pc = pos.piece_on(to_file, to_rank);
        if (    ((pc != NO_PIECE) && (color_of(pc) != Us))      ||      // Standard captures
                ((pc == NO_PIECE) && ( make_square(File(to_file), Rank(to_rank)) == pos.ep_square() ))   )  {   // En-passant captures
            if (rank != ourRank7)  // No promotions
                *moveList++ = Move( Square(file, rank), Square(to_file, to_rank) );
            else {                 // Promotions
                for (PieceType pt : {
                            KNIGHT, BISHOP, ROOK, QUEEN
                        })
                    *moveList++ = Move( Square(file, rank), Square(to_file, to_rank), pt );
            }
        }
        return moveList;
    }
    
    template<Color Us/*, GenType Type*/>
    ExtMove* generate_pawn_moves(const Position& pos, ExtMove* moveList)
    {
        int ourRank2, ourRank7, up;
        Piece ourPawn;
        switch (Us) {
        case WHITE:
            ourRank2 = RANK_2;
            ourRank7 = RANK_7;
            up = 1;
            ourPawn = W_PAWN;
            break;
        case BLACK:
            ourRank2 = RANK_7;
            ourRank7 = RANK_2;
            up = -1;
            ourPawn = B_PAWN;
            break;
        }
        
        for (int f = 0; f < 8; ++f)
            for (int r = 1; r < 7; ++r) {
                if ( pos.piece_on(f, r) == ourPawn ) {
                    
                    if ( pos.piece_on(f, r + up) == NO_PIECE ) {
                        // Single pawn pushes
                        if (r != ourRank7)  // No promotions
                            *moveList++ = Move( Square(f, r), Square(f, r + up) );
                        else {              // Promotions
                            for (PieceType pt : {
                                        KNIGHT, BISHOP, ROOK, QUEEN
                                    })
                                *moveList++ = Move( Square(f, r), Square(f, r + up), pt );
                        }
                        
                        // Double pawn pushes
                        if (( r == ourRank2 ) && ( pos.piece_on(f, r + 2*up) == NO_PIECE ))
                            *moveList++ = Move( Square(f, r), Square(f, r + 2*up) );
                    }
                    
                    // Standard and en-passant captures
                    if (f > FILE_A)
                        moveList = check_generate_pawn_capture<Us>(pos, moveList, ourRank7, f, r, f - 1, r + up);
                    if (f < FILE_H)
                        moveList = check_generate_pawn_capture<Us>(pos, moveList, ourRank7, f, r, f + 1, r + up);
                }
            }
        
        return moveList;
    }
    
    // kbrq = knight, bishop, rook, queen
    ExtMove* check_generate_kbrq_move(const Position& pos, ExtMove* moveList, Color us,  int file, int rank, int to_file, int to_rank)
    {
        Piece pc = pos.piece_on(to_file, to_rank);
        if (    ( pc == NO_PIECE )  ||
                ( color_of(pc) != color_of(pos.piece_on(file, rank)) )    )
            *moveList++ = Move( Square(file, rank), Square(to_file, to_rank) );
        return moveList;
    }
    
    //template<bool Checks>
    ExtMove* generate_knight_moves(const Position& pos, ExtMove* moveList, Color us, int f, int r)
    {
        for ( const Square& sq : knight_attacks_from(f, r) ) {
            moveList = check_generate_kbrq_move(pos, moveList, us, f, r, sq.file, sq.rank);
        }
        return moveList;
    }
    
    //template<bool Checks>
    ExtMove* generate_bishop_moves(const Position& pos, ExtMove* moveList, Color us, int f, int r)
    {
        for ( const Square& sq : bishop_attacks_from(pos, f, r) ) {
            moveList = check_generate_kbrq_move(pos, moveList, us, f, r, sq.file, sq.rank);
        }
        return moveList;
    }
    
    //template<bool Checks>
    ExtMove* generate_rook_moves(const Position& pos, ExtMove* moveList, Color us, int f, int r)
    {
        for ( const Square& sq : rook_attacks_from(pos, f, r) ) {
            moveList = check_generate_kbrq_move(pos, moveList, us, f, r, sq.file, sq.rank);
        }
        return moveList;
    }
    
    //template<bool Checks>
    ExtMove* generate_queen_moves(const Position& pos, ExtMove* moveList, Color us, int f, int r)
    {
        moveList = generate_rook_moves(pos, moveList, us, f, r);
        moveList = generate_bishop_moves(pos, moveList, us, f, r);
        return moveList;
    }
    
    ExtMove* check_generate_king_move(const Position& pos, ExtMove* moveList, Color us,  int file, int rank, int to_file, int to_rank)
    {
        Piece pc = pos.piece_on(to_file, to_rank);
        if (    ( pos.get_square_attackers_count(~us, to_file, to_rank) == 0 )  &&
                ( !pos.is_king_square_attacked(to_file, to_rank) )  &&
                ( ( pc == NO_PIECE )  ||  ( color_of(pc) != color_of(pos.piece_on(file, rank)) ) )    )
            *moveList++ = Move( Square(file, rank), Square(to_file, to_rank) );
        return moveList;
    }
    
    template<Color Us/*, bool Checks*/>
    ExtMove* generate_king_moves(const Position& pos, ExtMove* moveList, /*Color us,*/ int f, int r)
    {
        for ( const Square& sq : king_attacks_from(f, r) ) {
            moveList = check_generate_king_move(pos, moveList, Us, f, r, sq.file, sq.rank);
        }
        moveList = generate_castling< make_castling<Us, KING_SIDE>() >(pos, moveList, Us);
        moveList = generate_castling< make_castling<Us, QUEEN_SIDE>() >(pos, moveList, Us);
        return moveList;
    }
    
    template<Color Us, PieceType Pt/*, bool Checks*/>
    ExtMove* generate_moves(const Position& pos, ExtMove* moveList)
    {
        constexpr Piece ourKnight   = (Us == WHITE  ? W_KNIGHT  : B_KNIGHT);
        constexpr Piece ourBishop   = (Us == WHITE  ? W_BISHOP  : B_BISHOP);
        constexpr Piece ourRook     = (Us == WHITE  ? W_ROOK    : B_ROOK);
        constexpr Piece ourQueen    = (Us == WHITE  ? W_QUEEN   : B_QUEEN);
        constexpr Piece ourKing     = (Us == WHITE  ? W_KING    : B_KING);
        for (int f = 0; f < 8; ++f)
            for (int r = 0; r < 8; ++r) {
                switch (pos.piece_on(f, r)) {
                case ourKnight:
                    if (Pt == ALL_PIECES || Pt == KNIGHT)
                        moveList = generate_knight_moves(pos, moveList, Us, f, r);
                    break;
                case ourBishop:
                    if (Pt == ALL_PIECES || Pt == BISHOP)
                        moveList = generate_bishop_moves(pos, moveList, Us, f, r);
                    break;
                case ourRook:
                    if (Pt == ALL_PIECES || Pt == ROOK)
                        moveList = generate_rook_moves(pos, moveList, Us, f, r);
                    break;
                case ourQueen:
                    if (Pt == ALL_PIECES || Pt == QUEEN)
                        moveList = generate_queen_moves(pos, moveList, Us, f, r);
                    break;
                case ourKing:
                    if (Pt == ALL_PIECES || Pt == KING)
                        moveList = generate_king_moves<Us>(pos, moveList, /*Us,*/ f, r);
                    break;
                default:
                    break;
                }
            }
        
        return moveList;
    }
    
    template<Color Us, GenType Type>
    ExtMove* generate_all(const Position& pos, ExtMove* moveList)
    {
        //constexpr bool Checks = Type == QUIET_CHECKS;
        
        moveList = generate_pawn_moves<Us>(pos, moveList);
        moveList = generate_moves<Us, ALL_PIECES>(pos, moveList);
        //moveList = generate_moves<KNIGHT, Checks>(pos, moveList, Us);
        //moveList = generate_moves<BISHOP, Checks>(pos, moveList, Us);
        //moveList = generate_moves<  ROOK, Checks>(pos, moveList, Us);
        //moveList = generate_moves< QUEEN, Checks>(pos, moveList, Us);
        
        return moveList;
    }
    
} // namespace


bool MOVEGEN::check_move(const Position& pos, Move move)
{
    Square from = move.from;
    //Square to = move.to;

    Piece piece = pos.piece_on(from);

    if (piece != NO_PIECE)
        return true;

    return false;
}


template<GenType Type>
ExtMove* generate(const Position& pos, ExtMove* moveList)
{
    Color us = pos.side_to_move();
    
    return us == WHITE  ? generate_all<WHITE, Type>(pos, moveList)
                        : generate_all<BLACK, Type>(pos, moveList);
}

template<>
ExtMove* generate<EVASIONS>(const Position& pos, ExtMove* moveList)
{
    Color us = pos.side_to_move();
    
    return us == WHITE  ? generate_moves<WHITE, KING>(pos, moveList)
                        : generate_moves<BLACK, KING>(pos, moveList);
}


/// generate<LEGAL> generates all the legal moves in the given position

template<>
ExtMove* generate<LEGAL>(const Position& pos, ExtMove* moveList)
{
    Color us = pos.side_to_move();
    
    ExtMove* cur = moveList;
    
    moveList = pos.in_check() ? generate<EVASIONS    >(pos, moveList)
                              : generate<NON_EVASIONS>(pos, moveList);
    
    return moveList;
}
