#pragma once

#include "Game.h"
#include "Grid.h"
#include "BitBoard.h"

constexpr int pieceSize = 80;

//template <typename TYPE> void plusPlus(TYPE) {TYPE++;}

class Chess : public Game
{
public:
    Chess();
    ~Chess();

    void setUpBoard() override;

    bool canBitMoveFrom(Bit &bit, BitHolder &src) override;
    bool canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;
    bool actionForEmptyHolder(BitHolder &holder) override;

    void stopGame() override;

    Player *checkForWinner() override;
    bool checkForDraw() override;

    std::string initialStateString() override;
    std::string stateString() override;
    void setStateString(const std::string &s) override;

    Grid* getGrid() override { return _grid; }

private:
    Bit* PieceForPlayer(const int playerNumber, ChessPiece piece);
    Player* ownerAt(int x, int y) const;
    void FENtoBoard(const std::string& fen);
    char pieceNotation(int x, int y) const;

    #define WHITE 1
    #define BLACK -1

    // piece movement
    // Knight
    BitBoard generateKnightMoveBitBoard(int square); 
    void generateKnightMoves(std::vector<BitMove>& moves, BitBoard knightBoard, uint64_t empty_squares);
    
    BitBoard  _knightBitBoards[64];
    std::pair<int, int> knightOffsets[8] = {  // all possible moveable positions as a knight
        {2, 1}, {2, -1}, {-2, 1}, {-2, -1},
        {1, 2}, {1, -2}, {-1, 2}, {-1, -2}
    };
    

    //King
    // Only accounts for the position of ONE KING, whose index pos is indicated at kingPos
    BitBoard  _kingBitBoards[64];
    BitBoard generateKingMoveBitBoard(int square); 
    void generateKingMoves(std::vector<BitMove>& moves, unsigned int kingPos, uint64_t empty_squares);
    std::pair<int, int> kingOffsets[8] = {  // all possible moveable positions as a knight
        {1, 0}, {1, 1}, {0, 1}, {-1, 1},
        {-1, 0}, {-1, -1}, {0, -1}, {1, -1}
    };

    // Pawn
    BitBoard  _pawnBitBoards[64];
    void generatePawnMoves(std::vector<BitMove>& moves, BitBoard pawnBoard, BitBoard enemyPieces, BitBoard empty_squares, char color);
    void addPawnBitBoardMoves(std::vector<BitMove>& moves, const BitBoard pawnMove, const int shift);

    Grid* _grid;
    std::vector<BitMove> generateAllMoves();

    std::vector<BitMove>    _moves;
};