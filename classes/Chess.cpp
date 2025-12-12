#include "Chess.h"
#include <limits>
#include <cmath>
#include <chrono>
#include <iomanip>

Chess::Chess()
{
    _grid = new Grid(8, 8);

    // bit board lookup table to remove branching when finding bitboards
    for (int i = 0; i < 128; i++) {
        _bitboardLookup[i] = 0;
    }

    _bitboardLookup['P'] = WHITE_PAWNS;
    _bitboardLookup['N'] = WHITE_KNIGHTS;
    _bitboardLookup['B'] = WHITE_BISHOPS;
    _bitboardLookup['R'] = WHITE_ROOKS;
    _bitboardLookup['Q'] = WHITE_QUEENS;
    _bitboardLookup['K'] = WHITE_KING;
    _bitboardLookup['p'] = BLACK_PAWNS;
    _bitboardLookup['n'] = BLACK_KNIGHTS;
    _bitboardLookup['b'] = BLACK_BISHOPS;
    _bitboardLookup['r'] = BLACK_ROOKS;
    _bitboardLookup['q'] = BLACK_QUEENS;
    _bitboardLookup['k'] = BLACK_KING;
    _bitboardLookup['0'] = EMPTY_SQUARES;
}

Chess::~Chess()
{
    delete _grid;
    _gameState.shutdown();
}

char Chess::pieceNotation(int x, int y) const
{
    const char *wpieces = { "0PNBRQK" };
    const char *bpieces = { "0pnbrqk" };
    Bit *bit = _grid->getSquare(x, y)->bit();
    char notation = '0';
    if (bit) {
        notation = bit->gameTag() < 128 ? wpieces[bit->gameTag()] : bpieces[bit->gameTag()-128];
    }
    return notation;
}

Bit* Chess::PieceForPlayer(const int playerNumber, ChessPiece piece)
{   
    const char* pieces[] = { "pawn.png", "knight.png", "bishop.png", "rook.png", "queen.png", "king.png" };

    Bit* bit = new Bit();
    // should possibly be cached from player class?
    const char* pieceName = pieces[piece - 1];
    std::string spritePath = std::string("") + (playerNumber == 0 ? "w_" : "b_") + pieceName;
    bit->LoadTextureFromFile(spritePath.c_str());
    bit->setOwner(getPlayerAt(playerNumber));
    bit->setSize(pieceSize, pieceSize);
    bit->setGameTag(playerNumber == 0 ? piece : piece + 128);
    return bit;
}

void Chess::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 8;
    _gameOptions.rowY = 8;

    _grid->initializeChessSquares(pieceSize, "boardsquare.png");
    FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    
    // TEST CAPTURE
    //FENtoBoard("8/8/3N4/8/1K2n3/3P4/5P2/k7");
    _gameState = GameState();
    _gameState.init(stateString().c_str(), currentPlayer);
    _moves = _gameState.generateAllMoves();

    startGame();
}

void Chess::FENtoBoard(const std::string& fen) {
    // convert a FEN string to a board

    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->setBit(nullptr);
    });

    int y = _grid->getHeight()-1;
    int x = 0;
    for (char fen_char : fen) {
        // go to next row when reaching
        // - '/' for a new row on the board
        // - ' ' for breaks in notation between boardstate, castling, enpessant, etc.
        if (fen_char == '/' || fen_char == ' ') {
            y--;
            // when y >= 0, the function is searching for spaces on the board
            // when y < 0, the function is searching for castling, enpessant, and other notations
            // y = -1 | set turn for player
            // y = -2 | set castling availability for each player
            // y = -3 | determine enpessant availability
            // y = -4 | half moves
            // y = -5 | full moves
            x = 0;
            continue;
        }

        // create a bit, and assign it to a square at the given x, y
        Bit *bit = nullptr;
        char fen_lower = tolower(fen_char);
        // check if character matches a piece
        // set the correct piece image for the bit
        if (y >= 0) {
            if (!isdigit(fen_char)) {
                ChessPiece piece = Pawn;
                switch(fen_lower) {
                    case 'p':
                        break;
                    case 'r':
                        piece = Rook;
                        break;
                    case 'n':
                        piece = Knight;
                        break;
                    case 'b':
                        piece = Bishop;
                        break;
                    case 'q':
                        piece = Queen;
                        break;
                    case 'k':
                        piece = King;
                        break;   
                } 
                bit = PieceForPlayer(std::isupper(fen_char) ? 0 : 1, piece);
                BitHolder* curr_square = _grid->getSquare(x, y);
                bit->setPosition(curr_square->getPosition());
                curr_square->setBit(bit);

                // move one column to the right after each iteration by default
                x += 1;
            }
            // check for numbers 
            else { 
                x += fen_char - '0';
                // skip columns based on fen_char number
                // -1 to offset for default x move
            }
        } else if (y == -1) {  // check turn
            if (fen_lower == 'w') {
                // white to move
            } else if (fen_lower == 'b') {  
                // black to move
            }
        } else if (y == -2) {
            if (fen_lower == 'q') {
                // set castle rules
            } else if (fen_lower == 'k') {
                // set castle rules
            } else if (fen_lower == '-') {
                // nobody can castle
            }
        } else if (y == -3) {  // en pessant rules
            if (fen_lower == '-') {
                // no enpessant
            }
        }  else if (y == -4) {
            // half move
        } else if (y == -5) {
            // full move
        }
    }

    // for (int i = 0; i < 64; i ++) {
    //     x = i%8;
    //     y = i/8;
    // }


}

bool Chess::actionForEmptyHolder(BitHolder &holder)
{
    return false;
}

bool Chess::canBitMoveFrom(Bit &bit, BitHolder &src)
{
    bool ret = false;
    ChessSquare* square = (ChessSquare *)&src;
    if (square) {
        // highlight each square which the piece can move to
        int squareIndex = square->getSquareIndex();
        for (auto move : _moves) {
            if (move.from == squareIndex) {
                ret = true;
                ChessSquare* dest = _grid->getSquareByIndex(move.to);
                dest->setHighlighted(true);
            }
        }
    }
    return ret;
}

bool Chess::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    ChessSquare* square = (ChessSquare *)&dst;
    int fromIndex = ((ChessSquare *)&src)->getSquareIndex();
    bool canMove = false;
    if (square) {
        // if one of the moves is the destination square, return true
        int squareIndex = square->getSquareIndex();
        for (auto move : _moves) {
            if (move.from == fromIndex && move.to == squareIndex) {
                canMove = true;
            }
        }
    }
    return canMove;
}

void Chess::stopGame()
{
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });
}

Player* Chess::ownerAt(int x, int y) const
{
    if (x < 0 || x >= 8 || y < 0 || y >= 8) {
        return nullptr;
    }

    auto square = _grid->getSquare(x, y);
    if (!square || !square->bit()) {
        return nullptr;
    }
    return square->bit()->getOwner();
}

void Chess::bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst) {
    currentPlayer = currentPlayer == WHITE ? BLACK : WHITE;
    _gameState.init(stateString().c_str(), currentPlayer);
    _moves = _gameState.generateAllMoves();
    std::cout << _moves.size() << std::endl;
    endTurn();
}

Player* Chess::checkForWinner()
{
    return nullptr;
}

bool Chess::checkForDraw()
{
    return false;
}

std::string Chess::initialStateString()
{
    return stateString();
}

std::string Chess::stateString()
{
    std::string s;
    s.reserve(64);
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
            s += pieceNotation( x, y );
        }
    );
    return s;}

void Chess::setStateString(const std::string &s)
{
    std::cout << "setting state string" << std::endl;
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        int index = y * 8 + x;
        char playerNumber = s[index] - '0';
        if (playerNumber) {
            square->setBit(PieceForPlayer(playerNumber - 1, Pawn));
        } else {
            square->setBit(nullptr);
        }
    });
}

#pragma region Chess AI

static std::map<char, int> _pieceValues = {  // scores are mapped to the indexes of AllBitBoards
    {'P', 10}, {'N', 20}, {'B', 20}, {'R', 50}, {'Q', 100}, {'K', 200}, // white
    {'p', -10}, {'n', -20}, {'b', -20}, {'r', -50}, {'q', -100}, {'k', -200},
    {'0', 0}
};

int Chess::evaluateBoard(const GameState& gameState) 
{
    int score = 0;
    for (int square = 0; square < 64; ++square) {
        const unsigned char piece = gameState.state[square];
        const int index = _bitboardLookup[gameState.state[square]];
        score += _pieceValues[piece];
        score += pieceSquareTables[index][square];

    }
    return score;
}

void Chess::updateAI() {
    // NEGAMAX ALGORITHM
    int bestVal = -MILLY;
    BitMove bestMove;

    // Moves Per Second (MPS) counting
    const auto searchStart = std::chrono::steady_clock::now();
    _countMoves = 0;

    for (const auto& move: _moves) {
        // choose a possible move
        _gameState.pushMove(move);
        int moveVal = -negamax(_gameState, 4, -MILLY, MILLY);
        // Undo the move
        _gameState.popState();

        if (moveVal > bestVal) {
            bestMove = move;
            bestVal = moveVal;
        }
    };

    // Confirm the move
    if (bestVal != -MILLY) {
        // get move efficiency
        const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - searchStart).count();
        const double boardsPerSecond = seconds > 0.0 ? static_cast<double>(_countMoves) / seconds : 0.0;
        std::cout << "Moves checked: " << _countMoves
                << " (" << std::fixed << std::setprecision(2) << boardsPerSecond
                << " boards/s)" << std::defaultfloat << std::endl;

        // the actual move
        BitHolder& src = getHolderAt(bestMove.from & 7, bestMove.from / 8);
        BitHolder& dst = getHolderAt(bestMove.to & 7, bestMove.to / 8);
        Bit* bit = src.bit();
        dst.dropBitAtPoint(bit, ImVec2(0,0));
        src.setBit(nullptr);
        bitMovedFromTo(*bit, src, dst);
    }
}

int Chess::negamax(GameState& gamestate, int depth, int alpha, int beta) {
    _countMoves++;
    //max depth
    if (depth == 0) {
        return evaluateBoard(gamestate);
    }
    
    // get new moves based on new state
    auto newMoves = gamestate.generateAllMoves();

    // return board eval if there's no other moves
    if (newMoves.size() == 0) {
        return evaluateBoard(gamestate);
    }

    int bestVal = -MILLY;
    // branch out into the next possible board outcomes 
    for(const auto& move : newMoves) {
        gamestate.pushMove(move);
        bestVal = std::max(bestVal, -negamax(gamestate, depth - 1, -beta, -alpha));
        // Undo the move
        gamestate.popState();
        // alpha beta cut-off
        alpha = std::max(alpha, bestVal);
        if (alpha >= beta) {
            break;
        }
    }
    return bestVal;
}

#pragma endregion