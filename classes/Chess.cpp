#include "Chess.h"
#include <limits>
#include <cmath>
#include "MagicBitBoards.h"


Chess::Chess()
{
    _grid = new Grid(8, 8);

    initMagicBitboards();   

    // bit board lookup table to remove branching when finding bitboards
    for (int i = 0; i < 128; i++) {
        _bitboardLookup[i] = 0;
    }

    _bitboardLookup['P'] = WHITE_PAWNS;
    _bitboardLookup['N'] = WHITE_KNIGHTS;
    _bitboardLookup['B'] = WHITE_BISHOPS;
    _bitboardLookup['R'] = WHITE_ROOKS;
    _bitboardLookup['Q'] = WHITE_QUEENS;
    _bitboardLookup['K'] = WHITE_KINGS;
    _bitboardLookup['p'] = BLACK_PAWNS;
    _bitboardLookup['n'] = BLACK_KNIGHTS;
    _bitboardLookup['b'] = BLACK_BISHOPS;
    _bitboardLookup['r'] = BLACK_ROOKS;
    _bitboardLookup['q'] = BLACK_QUEENS;
    _bitboardLookup['k'] = BLACK_KINGS;
    _bitboardLookup['0'] = EMPTY_SQUARES;
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
    
    // TEST CAPTURE
    //FENtoBoard("8/8/3N4/8/1K2n3/3P4/5P2/k7");

    _moves = generateAllMoves(stateString(), currentPlayer);

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
    std::cout << "clear" << std::endl;
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

Player* Chess::checkForWinner()
{
    _moves = generateAllMoves(stateString(), currentPlayer);
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

#pragma region Chess AI

int Chess::evaluateBoard(std::string state) 
{
    int values[128];
    values['P'] = 100;
    values['N'] = 300;
    values['B'] = 400;
    values['R'] = 500;
    values['Q'] = 1000;
    values['K'] = 2000;
    values['p'] = -100;
    values['n'] = -300;
    values['b'] = -400;
    values['r'] = -500;
    values['q'] = -1000;
    values['k'] = -2000;

    values['0'] = 0;
    int score = 0;
    for (char ch : state) {
        score += values[ch];
    }

    return score;
}

void Chess::updateAI() {
    // NEGAMAX ALGORITHM
    int bestVal = -1000000;
    BitMove bestMove;
    std::string state = stateString();

    for (auto move: _moves) {
        int srcSquare = move.from;
        int dstSquare = move.to;

        // modify state string to "move" piece
        char oldDst = state[dstSquare];
        char srcPce = state[srcSquare];
        state[dstSquare] = srcPce;
        state[srcSquare] = '0';

        int moveVal = -negamax(state, 3, -currentPlayer, -1000, 1000);
        // Undo move
        state[dstSquare] = oldDst;
        state[srcSquare] = srcPce;

        if (moveVal > bestVal) {
            bestMove = move;
            bestVal = moveVal;
        }
    };

    // Confirm the move

    
}

int Chess::negamax(std::string& state, int depth, int playerColor, int alpha, int beta) {
    
    
    //max depth
    if (depth == 0) {
        return evaluateBoard(state) * playerColor;
    }
    
    // get new moves based on new state
    auto newMoves = generateAllMoves(state, playerColor);

    int bestVal = -1000000;
    // branch out into the next possible board outcomes 
    for (auto move: newMoves) {
        int srcSquare = move.from;
        int dstSquare = move.to;

        // modify state string to "move" piece
        char oldDst = state[dstSquare];
        char srcPce = state[srcSquare];
        state[dstSquare] = srcPce;
        state[srcSquare] = '0';

        int moveVal = -negamax(state, 3, -playerColor, -alpha, -beta);
        // Undo move
        state[dstSquare] = oldDst;
        state[srcSquare] = srcPce;

        if (moveVal > bestVal) {
            bestVal = moveVal;
        }
    };
    return bestVal;
}

#pragma endregion

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

std::vector<BitMove> Chess::generateAllMoves(const std::string state, int playerColor)
{
    std::vector<BitMove> moves;
    moves.reserve(32);
    // need to implement friendly/unfriendly in bit so for now this hack
    currentPlayer = getCurrentPlayer()->playerNumber() == 0 ? WHITE : BLACK;
    std::cout << currentPlayer << std::endl;

    for (int i = 0; i < e_numBitboards; i++) {
        _bitboards[i] = 0;
    }

    for (int i = 0; i < 64; i++) {
        int bitIndex = _bitboardLookup[state[i]];
        _bitboards[bitIndex] |= 1ULL << i;
    }

    _bitboards[WHITE_ALL_PIECES] = _bitboards[WHITE_PAWNS].getData() | 
    _bitboards[WHITE_KNIGHTS].getData() | 
    _bitboards[WHITE_BISHOPS].getData() | 
    _bitboards[WHITE_ROOKS].getData() | 
    _bitboards[WHITE_QUEENS].getData() | 
    _bitboards[WHITE_KINGS].getData();

    _bitboards[BLACK_ALL_PIECES] = _bitboards[BLACK_PAWNS].getData() | 
    _bitboards[BLACK_KNIGHTS].getData() | 
    _bitboards[BLACK_BISHOPS].getData() | 
    _bitboards[BLACK_ROOKS].getData() | 
    _bitboards[BLACK_QUEENS].getData() | 
    _bitboards[BLACK_KINGS].getData();
    
    _bitboards[OCCUPANCY] = _bitboards[WHITE_ALL_PIECES].getData() | _bitboards[BLACK_ALL_PIECES].getData();

    int bitIndex = currentPlayer == WHITE ? WHITE_PAWNS : BLACK_PAWNS; // can be used as offset to the _bitboards lookup index
    int oppBitIndex = currentPlayer == WHITE ? BLACK_PAWNS : WHITE_PAWNS;

    generateKnightMoves(moves, _bitboards[WHITE_KNIGHTS + bitIndex], ~_bitboards[WHITE_ALL_PIECES + bitIndex].getData());
    generatePawnMoves(moves, _bitboards[WHITE_PAWNS + bitIndex], ~_bitboards[OCCUPANCY].getData(), _bitboards[WHITE_ALL_PIECES + oppBitIndex], currentPlayer);
    generateKingMoves(moves, _bitboards[WHITE_KINGS + bitIndex], ~_bitboards[WHITE_ALL_PIECES + bitIndex].getData());
    generateBishopMoves(moves, _bitboards[WHITE_BISHOPS + bitIndex], _bitboards[OCCUPANCY].getData(), _bitboards[WHITE_ALL_PIECES + bitIndex].getData());
    generateRookMoves(moves, _bitboards[WHITE_ROOKS + bitIndex], _bitboards[OCCUPANCY].getData(), _bitboards[WHITE_ALL_PIECES + bitIndex].getData());
    generateQueenMoves(moves, _bitboards[WHITE_QUEENS + bitIndex], _bitboards[OCCUPANCY].getData(), _bitboards[WHITE_ALL_PIECES + bitIndex].getData());


    return moves;
}

#pragma region Knight FX

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
        BitBoard moveBitboard = BitBoard(KnightAttacks[fromSquare] & empty_squares);
        // Efficiently iterate through only the set bits
        moveBitboard.forEachBit([&](int toSquare) {
           moves.emplace_back(fromSquare, toSquare, Knight);
        });
    });
}

#pragma endregion

#pragma region Pawn FX

void Chess::generatePawnMoves(std::vector<BitMove>& moves, BitBoard pawnBoard, BitBoard empty_squares, BitBoard enemyPieces, char color) 
{
    if (pawnBoard.getData() == 0) {  // no pawns
        return;
    }

    // Calculate single moves
    // shift bits LEFT to push white up the board. otherwise, shift bits RIGHT to push black down the board
    BitBoard singleMoves = color == WHITE ? 
    (pawnBoard.getData() << 8) & empty_squares.getData(): 
    (pawnBoard.getData() >> 8) & empty_squares.getData();
    // BitBoard(empty_squares.getData()).printBitboard();
    // BitBoard(enemyPieces.getData()).printBitboard();

    // Calculate double moves
    /*only let pawns move forward if:
    - (after a single move) pawns are on row 3 for white, row 6 for black
    - after a single move, the next square is empty
    */
    BitBoard doubleMoves = color == WHITE ? 
    ((singleMoves.getData() & Row3) << 8) & empty_squares.getData(): 
    ((singleMoves.getData() & Row6) >> 8) & empty_squares.getData();

    // Calculate left & right capturing
    // can only capture when an enemy piece is present
    // check left column. Ignore for pawns on column 1
    BitBoard captureLeft = color == WHITE ? 
    ((pawnBoard.getData() & NotCol1) << 7) & enemyPieces.getData():
    ((pawnBoard.getData() & NotCol1) >> 9) & enemyPieces.getData();
    // check right column. Ignore for pawns on column 8
    BitBoard captureRight = color == WHITE ? 
    ((pawnBoard.getData() & NotCol8) << 9) & enemyPieces.getData(): 
    ((pawnBoard.getData() & NotCol8) >> 7) & enemyPieces.getData();

    int shiftForward = (color == WHITE) ? 8 : -8;
    int doubleShift = (color == WHITE) ? 16 : -16;
    int captureLeftShift = (color == WHITE) ? 7 : -9;
    int captureRightShift = (color == WHITE) ? 9 : -7;

    // add single moves to list
    addPawnBitBoardMoves(moves, singleMoves, shiftForward);
    // add double moves to list
    addPawnBitBoardMoves(moves, doubleMoves, doubleShift);
    // add left captures to list
    addPawnBitBoardMoves(moves, captureLeft, captureLeftShift);
    // add right captures to list
    addPawnBitBoardMoves(moves, captureRight, captureRightShift);
}

void Chess::addPawnBitBoardMoves(std::vector<BitMove>& moves, const BitBoard pawnMove, const int shift) {
    if (pawnMove.getData() == 0) {
        return;
    }
    BitBoard new_board(pawnMove);
    pawnMove.forEachBit([&](int toSquare) {
        int fromSquare = toSquare - shift;
        moves.emplace_back(fromSquare, toSquare, Pawn);
    });
}

#pragma endregion

#pragma region King FX

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
void Chess::generateKingMoves(std::vector<BitMove>& moves, BitBoard piecesBoard, uint64_t empty_squares) {
    piecesBoard.forEachBit([&](int fromSquare) {
        BitBoard moveBitboard = BitBoard(KingAttacks[fromSquare] & empty_squares);
        // Efficiently iterate through only the set bits
        moveBitboard.forEachBit([&](int toSquare) {
           moves.emplace_back(fromSquare, toSquare, King);
        });
    });
}

#pragma endregion

#pragma region Queen FX

void Chess::generateQueenMoves(std::vector<BitMove>& moves, BitBoard bishopBoard, uint64_t occupancy, uint64_t friend_tiles){
    bishopBoard.forEachBit([&](int fromSquare) {
        BitBoard moveBitboard = BitBoard(getQueenAttacks(fromSquare, occupancy) & ~friend_tiles);
        // Efficiently iterate through only the set bits
        moveBitboard.forEachBit([&](int toSquare) {
           moves.emplace_back(fromSquare, toSquare, Queen);
        });
    });
}

#pragma endregion

#pragma region Rook FX

void Chess::generateRookMoves(std::vector<BitMove>& moves, BitBoard bishopBoard, uint64_t occupancy, uint64_t friend_tiles){
    bishopBoard.forEachBit([&](int fromSquare) {
        BitBoard moveBitboard = BitBoard(getRookAttacks(fromSquare, occupancy) & ~friend_tiles);
        // Efficiently iterate through only the set bits
        moveBitboard.forEachBit([&](int toSquare) {
           moves.emplace_back(fromSquare, toSquare, Rook);
        });
    });
}

#pragma endregion

#pragma region Bishop FX


void Chess::generateBishopMoves(std::vector<BitMove>& moves, BitBoard bishopBoard, uint64_t occupancy, uint64_t friend_tiles){
    bishopBoard.forEachBit([&](int fromSquare) {
        BitBoard moveBitboard = BitBoard(getBishopAttacks(fromSquare, occupancy) & ~friend_tiles);
        // Efficiently iterate through only the set bits
        moveBitboard.forEachBit([&](int toSquare) {
           moves.emplace_back(fromSquare, toSquare, Bishop);
        });
    });
}

#pragma endregion

#pragma endregion
