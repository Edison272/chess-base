#include "Chess.h"
#include <limits>
#include <cmath>

Chess::Chess()
{
    _grid = new Grid(8, 8);
}

Chess::~Chess()
{
    delete _grid;
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
    //FENtoBoard("8/8/8/4N3/8/8/8/8/8");
    
    // generate moves for each position on the board
    for (int i = 0; i < 64; i++) {
        _knightBitBoards[i] = generateKnightMoveBitBoard(i);
        _kingBitBoards[i] = generateKingMoveBitBoard(i);
    }
    _moves = generateAllMoves();

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
    //     std::cout << _grid->getSquare(x,y)->gameTag() << " at: " << x << ", " << y << std::endl;
    // }


}

bool Chess::actionForEmptyHolder(BitHolder &holder)
{
    return false;
}

bool Chess::canBitMoveFrom(Bit &bit, BitHolder &src)
{
    // need to implement friendly/unfriendly in bit so for now this hack
    int currentPlayer = getCurrentPlayer()->playerNumber() * 128;
    int pieceColor = bit.gameTag() & 128;
    if (pieceColor == currentPlayer) return true;

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
    if (square) {
        // if one of the moves is the destination square, return true
        int squareIndex = square->getSquareIndex();
        for (auto move : _moves) {
            if (move.to == squareIndex) {
                return true;
            }
        }
    }
    return false;
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
    //std::cout << "creating state string: " << s << std::endl;
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

#pragma region Chess Piece Movement

std::vector<BitMove> Chess::generateAllMoves()
{
    std::vector<BitMove> moves;
    moves.reserve(32);
    std::string state = stateString();

    uint64_t whiteKnights = 0LL;
    uint64_t whitePawns = 0LL;
    unsigned int whiteKingPos = 0;  // only save a singular int value for the index of the king
    float fooFloat = 0.0f;

    for (int i = 0; i < 64; i++) {
        if (state[i] == 'N') {
            whiteKnights |= 1ULL << i;
        } else if (state[i] == 'P') {
            whitePawns |= 1ULL << i;
        } else if (state[i] == 'K') {
            whiteKingPos = i;
        }
    }

    uint64_t occupancy = whiteKnights | whitePawns;
    generateKnightMoves(moves, whiteKnights, ~occupancy);
    generateKingMoves(moves, whiteKingPos, ~occupancy);

    std::cout << moves.size() << std::endl;
    return moves;
}

BitBoard Chess::generateKnightMoveBitBoard(int square) {
    // create an empty bitboard
    BitBoard bitboard = 0ULL;
    int column = square / 8;  // y value
    int row = square % 8;     // x value

    constexpr uint64_t oneBit = 1;
    // if the offset position is a valid position on the board, mark it on the bitboard
    for (auto [dx, dy] : knightOffsets) {
        int x = row + dx, y = column + dy;
        if (x >= 0 && x < 8 && y >= 0 && y < 8) {
            // shift the 1 bit onto the respective place on the board relative to its on-board index
            bitboard |= oneBit << (y * 8 + x);
        }
    }

    return bitboard;
}

void Chess::generateKnightMoves(std::vector<BitMove>& moves, BitBoard knightBoard, uint64_t empty_squares) {
    knightBoard.forEachBit([&](int fromSquare) {
        BitBoard moveBitboard = BitBoard(_knightBitBoards[fromSquare].getData() & empty_squares);
        // Efficiently iterate through only the set bits
        moveBitboard.forEachBit([&](int toSquare) {
           moves.emplace_back(fromSquare, toSquare, Knight);
        });
    });
}

BitBoard Chess::generateKingMoveBitBoard(int square) {
    // create an empty bitboard
    BitBoard bitboard = 0ULL;
    int column = square / 8;  // y value
    int row = square % 8;     // x value

    constexpr uint64_t oneBit = 1;
    // if the offset position is a valid position on the board, mark it on the bitboard
    for (auto [dx, dy] : kingOffsets) {
        int x = row + dx, y = column + dy;
        if (x >= 0 && x < 8 && y >= 0 && y < 8) {
            // shift the 1 bit onto the respective place on the board relative to its on-board index
            bitboard |= oneBit << (y * 8 + x);
        }
    }

    return bitboard;
}

void Chess::generateKingMoves(std::vector<BitMove>& moves, unsigned int kingPos, uint64_t empty_squares){
    BitBoard moveBitboard = BitBoard(_kingBitBoards[kingPos].getData() & empty_squares);
    std::cout << kingPos % 8 << " , " << kingPos / 8 << std::endl;
    _kingBitBoards[kingPos].printBitboard();
    // Efficiently iterate through only the set bits
    moveBitboard.forEachBit([&](int toSquare) {
        moves.emplace_back(kingPos, toSquare, Knight);
    });
}


#pragma endregion
