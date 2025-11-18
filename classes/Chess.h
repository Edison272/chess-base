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

    // piece movement
    // Knight
    BitBoard generateKnightMoveBitBoard(int square); 
    void generateKnightMoves(std::vector<BitMove>& moves, BitBoard knightBoard, uint64_t empty_squares);
    
    BitBoard  _knightBitBoards[64];
    std::pair<int, int> knightOffsets[8] = {  // all possible moveable positions as a knight
        {2, 1}, {2, -1}, {-2, 1}, {-2, -1},
        {1, 2}, {1, -2}, {-1, 2}, {-1, -2}
    };

    // Pawn
    BitBoard generatePawnMoveBitBoard(int square); 
    void generatePawnMoves(std::vector<BitMove>& moves, BitBoard knightBoard, uint64_t empty_squares);

    // constexpr uint64_t NotCol0(0xFEFEFEFEFEFEFEFEULL);
    // constexpr uint64_t NotCol7(0x7F7F7F7F7F7F7F7FULL);
    // constexpr uint64_t Row3(0x0000000000FF0000ULL);
    // constexpr uint64_t Row6(0x0000FF0000000000ULL);

    Grid* _grid;
    std::vector<BitMove> generateAllMoves();

    std::vector<BitMove>    _moves;
};