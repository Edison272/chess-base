#pragma once

#include "Game.h"
#include "Grid.h"
#include "BitBoard.h"
#include "PieceSquare.h"

constexpr int pieceSize = 80;

enum AllBitBoards
{
    WHITE_PAWNS,
    WHITE_KNIGHTS,
    WHITE_BISHOPS,
    WHITE_ROOKS,
    WHITE_QUEENS,
    WHITE_KINGS,
    WHITE_ALL_PIECES,
    BLACK_PAWNS,
    BLACK_KNIGHTS,
    BLACK_BISHOPS,
    BLACK_ROOKS,
    BLACK_QUEENS,
    BLACK_KINGS,
    BLACK_ALL_PIECES,
    OCCUPANCY,
    EMPTY_SQUARES,
    e_numBitboards
};

constexpr int WHITE = 1;
constexpr int BLACK = -1;

// row and column masks (for pawn movement)
constexpr uint64_t NotCol1(0xFEFEFEFEFEFEFEFEULL);  // mask along the first column
constexpr uint64_t NotCol8(0x7F7F7F7F7F7F7F7FULL);  // mask along the last column
constexpr uint64_t Row3(0x0000000000FF0000ULL);     // mask on the 3rd row
constexpr uint64_t Row6(0x0000FF0000000000ULL);     // mask on the 6th row

class Chess : public Game
{
public:
    Chess();
    ~Chess();

    void setUpBoard() override;

    bool canBitMoveFrom(Bit &bit, BitHolder &src) override;
    void cancelMove(Bit &bit, BitHolder &src);
    bool canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;
    bool actionForEmptyHolder(BitHolder &holder) override;

    void stopGame() override;
    bool gameHasAI() override { return true; };

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

    int currentPlayer = 1;

    // AI
    int evaluateBoard(std::string);
	void updateAI() override;
    int negamax(std::string& state, int depth, int playerColor, int alpha, int beta);

    // piece movement
    // Knight
    void generateKnightMoves(std::vector<BitMove>& moves, BitBoard knightBoard, uint64_t empty_squares);

    // King
    void generateKingMoves(std::vector<BitMove>& moves, BitBoard piecesBoard, uint64_t empty_squares);

    void generatePawnMoves(std::vector<BitMove>& moves, BitBoard pawnBoard, BitBoard empty_squares, BitBoard enemyPieces, char color);
    void addPawnBitBoardMoves(std::vector<BitMove>& moves, const BitBoard pawnMove, const int shift);

    // Bishop
    void generateBishopMoves(std::vector<BitMove>& moves, BitBoard bishopBoard, uint64_t occupancy, uint64_t friend_tiles);

    // Rook
    void generateRookMoves(std::vector<BitMove>& moves, BitBoard bishopBoard, uint64_t occupancy, uint64_t friend_tiles);

    // Queen
    void generateQueenMoves(std::vector<BitMove>& moves, BitBoard bishopBoard, uint64_t occupancy, uint64_t friend_tiles);


    Grid* _grid;
    std::vector<BitMove> generateAllMoves(const std::string state, int playerColor);

    std::vector<BitMove>    _moves;
    BitBoard _bitboards[e_numBitboards];
    int _bitboardLookup[128];

    // Debug
    int _countMoves = 0;
};