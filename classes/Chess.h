#pragma once

#include "Game.h"
#include "Grid.h"
#include "PieceSquare.h"
#include "GameState.h"

constexpr int pieceSize = 80;

constexpr int MILLY = 1000000;

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
    void bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst);

    std::string initialStateString() override;
    std::string stateString() override;
    void setStateString(const std::string &s) override;

    Grid* getGrid() override { return _grid; }

private:
    Bit* PieceForPlayer(const int playerNumber, ChessPiece piece);
    Player* ownerAt(int x, int y) const;
    void FENtoBoard(const std::string& fen);
    char pieceNotation(int x, int y) const;

    int currentPlayer = WHITE;

    // AI
    int evaluateBoard(const GameState& gameState);
	void updateAI() override;
    int negamax(GameState& gamestate, int depth, int alpha, int beta);

    Grid* _grid;
    GameState _gameState;


    std::vector<BitMove>    _moves;
    BitBoard _bitboards[e_numBitboards];
    int _bitboardLookup[128];

    // Debug
    int _countMoves = 0;
};