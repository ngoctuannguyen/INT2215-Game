#include <iostream>
#include <SDL_mixer.h>
#include <string>
#include "common.h"
#include "board.h"

using namespace std;

Board::Board() : display(this){

    initializerVars();
    emptyBoard();
    initializePiece();
    placePieceDefault();
    initializeZobrist();
    setSquarePosition();
    initiallizeBitboard();
    display.setSpriteClips();
    display.setButton();
}

Board::Board(string FEN){

    cout << "Loading FEN" << FEN << "\n";
    initiallizeVars();
    emptyBoard();
    initializePiece();
    setSquarePosition();
    display.setSpriteClips();
    display.setButtons();
    placePiece(FEN);
    initiallizeZobrist();
    moveInfo[0].zobrist = zobrist.key;
}

void Board::updateDisplay(const int& mF, const int& mT){
    display.displayBoard(mF, mT);
}

void Board::initializeZobrist() {
	bool color;
	int pos;
	zobrist.key = 0;
	for (int p = 0; p < 32; p++) {
		if (!piece[p].getAlive()) continue;
		color = piece[p].getColor();
		pos = to64(piece[p].getPos());
		switch (piece[p].getValue()) {
			case R_VAL:
				zobrist.key ^= zobrist.piece[0][color][pos-1];
			case N_VAL:
				zobrist.key ^= zobrist.piece[1][color][pos-1];
			case B_VAL:
				zobrist.key ^= zobrist.piece[2][color][pos-1];
			case Q_VAL:
				zobrist.key ^= zobrist.piece[3][color][pos-1];
			case K_VAL:
				zobrist.key ^= zobrist.piece[4][color][pos-1];
			case P_VAL:
				zobrist.key ^= zobrist.piece[5][color][pos-1];
		}
	}

	if (!side)
		zobrist.key ^= zobrist.side;

	if (piece[wK].getMoved() == 0) {
		if (piece[wkR].getMoved() == 0 && piece[wqR].getMoved() == 0) {
			zobrist.key ^= zobrist.castling[1][2];
		}
		else if (piece[wkR].getMoved() == 0) {
			zobrist.key ^= zobrist.castling[1][0];
		}
		else if (piece[wqR].getMoved() == 0) {
			zobrist.key ^= zobrist.castling[1][1];
		}
	}
	else 
		zobrist.key ^= zobrist.castling[1][3];

	if (piece[bK].getMoved() == 0) {
		if (piece[bkR].getMoved() == 0 && piece[bqR].getMoved() == 0) {
			zobrist.key ^= zobrist.castling[0][2];
		}
		else if (piece[bkR].getMoved() == 0) {
			zobrist.key ^= zobrist.castling[0][0];
		}
		else if (piece[bqR].getMoved() == 0) {
			zobrist.key ^= zobrist.castling[0][1];
		}
	}
	else 
		zobrist.key ^= zobrist.castling[0][3];
	
	
	if (moveInfo.size() && moveInfo.back().epSq != 0)
		zobrist.key ^= zobrist.enPassant[to64(moveInfo.back().epSq)%10-1];
}

void Board::setSquarePosition(){

    if (!flipped){
        for (int i = 0;i < 64;++i){
            display.square[i].setPos(display.getBoardXStart() + (display.getSqSize() * (i % 8)), display.getBoardYStart() + display.getBoardSize() - (display.getSqSize() * (i / 8 + 1)));
            display.square[i].setSq(i + 1);  
        }
    }
    else {
        for (int i = 0;i < 64;++i) {
			display.square[i].setPos(display.getBoardXStart()+display.getBoardSize() - (display.getSqSize()*(i%8+1)), display.getBoardYStart()+(display.getSqSize()*(i/8)));
			display.squares[i].setSq(i+1);
		}
    }
}

void Board::setPieceOnSquare(){
    for (inti i = 0;i < 64;++i)
        display.square[i].setPiece(board120[from64(i + 1)]);
}

void Board::initializeVars(){

    moveFrom = 0;
    moveTo = 0;
    ply = 0;
    side = WHITE;
    whiteIsBot = false;
    blackIsBot = true;
    whiteCastled = false;
    blackCastled = false;
    castling = 0;
    sideInCheck = 0;
    sideInCheckmate = 0;
    whiteMaterial = 8 * P_VAL + 2 * (R_VAL + B_VAL + N_VAL) + Q_VAL + K_VAL;
    blackMaterial = whiteMaterial;
}

void Board::initiallizeBitboard(){
    int pos, t;
    for (int i = BLACK;i <= WHITE;++i){
        for (int j = bqR - (i * 16);j <= bPh - (i * 16);++j){
            if (piece[p].getAlive()){
                pos = to64(piece[j].getPos()) - 1;
                t = piece[j].getType();
                bb.pieces[i][j] |= bb.sq[pos];
                bb.allPieces[i][j] |= bb.sq[pos]; 
            }
        }
    }   
}

void Board::emptyBoard(){

    for (int i = 0;i < 120;++i)
        if (i < 21 || i > 98)
            board120[i] = invalid;
        else if (i % 10 == 0 || i % 10 == 9)
            board120[i] = invalid;
        else board120[i] = none;
}

void Board::placePiece(unsigned int p, unsigned int sq){
    piece[p].setPos(sq);
    board120[sq] = p;
}

void Board::placePieceDefault(){

    for (int i = A1;i <= H1;++i)
        placePiece(i - A1, i);
    for (int i = A2;i <= A2;++i)
        placePiece(i - A2 + 8, i);
    for (int i = A8;i <= H8;++i)
        placePiece(i - A8 + 16, i);
    for (int i = A7;i <= H7;++i)
        placePiece(i - A7 + 24, i);
}

void Board::initializePiece(){

    int v;

    for (int i = 0; i <= 16; i += 16) {
		piece[wqR+i].setName("Rook");
		piece[wqR+i].setAbbr('R' + 2*i);
		piece[wqR+i].setValue(R_VAL);
		piece[wqR+i].setType(ROOK);

		piece[wqN+i].setName("Knight");
		piece[wqN+i].setAbbr('N' + 2*i);
		piece[wqN+i].setValue(N_VAL);
		piece[wqN+i].setType(KNIGHT);

		piece[wqB+i].setName("Bishop");
		piece[wqB+i].setAbbr('B' + 2*i);
		piece[wqB+i].setValue(B_VAL);
		piece[wqB+i].setType(BISHOP);

		piece[wQ+i].setName("Queen");
		piece[wQ+i].setAbbr('Q' + 2*i);
		piece[wQ+i].setValue(Q_VAL);
		piece[wQ+i].setType(QUEEN);

		piece[wK+i].setName("King");
		piece[wK+i].setAbbr('K' + 2*i);
		piece[wK+i].setValue(K_VAL);
		piece[wK+i].setType(KING);

		piece[wkB+i].setName("Bishop");
		piece[wkB+i].setAbbr('B' + 2*i);
		piece[wkB+i].setValue(B_VAL);
		piece[wkB+i].setType(BISHOP);

		piece[wkN+i].setName("Knight");
		piece[wkN+i].setAbbr('N' + 2*i);
		piece[wkN+i].setValue(N_VAL);
		piece[wkN+i].setType(KNIGHT);

		piece[wkR+i].setName("Rook");
		piece[wkR+i].setAbbr('R' + 2*i);
		piece[wkR+i].setValue(R_VAL);
		piece[wkR+i].setType(ROOK);
	}
	for (int s = 0; s <= 16; s += 16) 
		for (int i = 0; i < 8; i++) { 
			piece[wPa+i+s].setName("Pawn");
			piece[wPa+i+s].setAbbr('P' + 2*s);
			piece[wPa+i+s].setValue(P_VAL);
			piece[wPa+i+s].setType(PAWN);
		}


    for (int i = wqR;i <= wPh;++i)
        piece[i].setColor(WHITE);
    for (int i = bqR;i <= bPh;++i)
        piece[i].setColor(BLACK);
    
    for (int i = wqR;i <= bPh;++i){
        piece[i].freeMoveList();
        v = piece[i].getValue();

        if (v == K_VAL)
            piece[i].setMoveListSize(10);
        else if (v == Q_VAL)
            piece[i].setMoveListSize(27);
        else if (v == R_VAL)
            piece[i].setMoveListSize(14);
        else if (v == B_VAL)
            piece[i].setMoveListSize(13);
        else if (v == N_VAL)
            piece[i].setMoveListSize(8);
        else if (v == P_VAL)
            piece[i].setMoveListSize(4);
        piece[i].setMoveList(new int[piece[i].getMoveListSize()]);
    }
}

void Board::placePieces(string FEN){

    unsigned int index = 0, sqCounter = A8, p;
    int wPIndex = wPa, bPIndex = bPa;

    while (FEN[index] != ' '){
        if (FEN[index] == '/')
            sqCounter -= 18;
        else if (isalpha(FEN[index])){
            if (FEN[index] == 'Q')
                p = wQ;
            else if (FEN[index] == 'K')
                p = wK;
            else if (FEN[index] == 'R'){
                if (piece[wqR].getPos())
                    p = wkR;
                else p = wqR;
            }
            else if (FEN[index] == 'N'){
                if (piece[wqN].getPos())
                    p = wkN;
                else p = wqN;
            }
            else if (FEN[index] == 'B')
                if (piece[wqB].getPos())
                    p = wkB;
                else p = wqB;
            else if (FEN[index] == 'P'){
                p = wPIndex;
                ++wPIndex;
            }
            else if (FEN[index] == 'q')
                p = bQ;
            else if (FEN[index] == 'k')
                p = bK;
            else if (FEN[index] == 'r'){
                if (piece[bqR].getPos())
                    p = bkR;
                else p = bqR;
            }
            else if (FEN[index] == 'n'){
                if (piece[bqN].getPos())
                    p = bkN;
                else p = bqN;
            }
            else if (FEN[index] == 'b')
                if (piece[bqB].getPos())
                    p = bkB;
                else p = wqB;
            else if (FEN[index] == 'p'){
                p = bPIndex;
                ++bPIndex;
            } 

            placePiece(p, sqCounter);
            ++sqCounter; 
            
        }
        else sqCounter += FEN[index] - '0';
        ++index;
    }

    whiteMaterial = 0;
    blackMaterial = 0;

    for (int i = wqR;i <= bPh;++i){
        if (!piece[i].getPos())
            piece[i].kill();
        if (!piece[i].getAlive())
            continue;
        
        if (i <= wPh)
            whiteMaterial += piece[i].getValue();
        else blackMaterial += piece[i].getValue();   

        if (piece[i].getValue() == P_VAL){
            if (i <= wPh){
                if (piece[i].getPos() / 10 != 3)
                    piece[i].incrMoved();
            }
            else {
                if (piece[i].getPos() / 10 != 8)
                    piece[i].incrMoved();
            }
        } 
    }

    ++index;
    if (FEN[index] == 'w')
        side = 1;
    else side = 0;

    index += 2;
    piece[wqR].incrMoved();
    piece[wkR].incrMoved();
    piece[bqR].incrMoved();
    piece[bkR].incrMoved();

    while (FEN[index] != ' '){
        if (FEN[index] == 'K')
            piece[wkR].decrMoved();
        else if (FEN[index] == 'Q')
            piece[wqR].decrMoved();
        else if (FEN[index] == 'k')
            piece[bkR].decrMoved();
        else if (FEN[index] == 'q')
            piece[bqR].decrMoved();
        ++index;       
    }

    int epSq = 0;

    ++index; 
    if (FEN[index] != '-'){
        int row, file;
        file = FEN[index] - 'a' + 1;
        ++index;
        row = FEN[index] - '0' + 1;
        epSq = row * 10 + file;
    }
    ++index;

    ++index;
    int halfMoveClock = FEN[index] - '0';
    ++index;
    while (FEN[index] != ' '){
        halfMoveClock = halfMoveClock * 10;
        ++index;
        halfMoveClock += FEN[index] - '0';
        ++index;
    }

    ++index;
    int moves;
    moves = FEN[index] - '0';
    while (index + 1 != FEN.size() && FEN[index + 1] != ' '){
        moves = movés * 10;
        ++index;
        moves += FEN[index] - '0';
    }

    ply = (move - 1) * 2;
    if (!side) 
        ++ply;
    
    moveInfo.push_back({0, epSq, -1, -1, halfMoveClock});

}

void Board::handleInput(int& mF, int& mT, SDL_Event* e){

    display.handleButtons(e);

    int sound = 0;
    for (int i = 0;i < 64;++i){
        display.squares[i].handleEvent(e, mF, mT, side, sound);
        if (sound == 1)
            Mix_PlayChannel(-1, display.mFSound, 0);
        else if (sound == 2)
            Mix_PlayChannel(-1, display.mTSound, 0);
    }

    if (mF != -1 && mT != -1){
        mF = from64(mF);
        mT = from64(mT);
        if (legalMove(mF, mT, getSide(), 1)){
            setMove(mF, mT);
            movePiece();
            changeTurn();
            genOrderMoveList();
            checkCheck(getSide());
            std::cout << "Current FEN (ply " << ply << ", move " << (ply-1)/2+1 << "): " << getFEN() << '\n';
			std::cout << "Current Zobrist: " << zobrist.key << '\n';
			if (drawCheck() == 2)
				std::cout << "Threefold repetition.\n";
			std::cout << "-----------------------------------------------------------------------------\n\n";
            
        }
        mT = -1;
        mF = -1;
    }
}

void Board::changeTurn(){
    if (side)
        side = BLACK;
    else side = WHITE;
}

void Board::botMove() {
	
    if (sideInCheckmate)
		return;
	std::cout << "Thinking for ";
	if (side)
        std::cout << "White...";
    else std::cout << "Black...";

	std::cout << " (ply " << ply+1 << ", move " << ply/2 + 1 << ")\n";

	display.displayBotText();

	int move = 0;
	if (side)
		move = whiteBot.think(*this, whiteBot.getLevel());
	else
		move = blackBot.think(*this, blackBot.getLevel());

	if (move == 0) {
		std::cout << "Stalemate!\n";
		sideInCheckmate = 3;
		return;
	}

	setMove(move / 100, move % 100);
	movePiece();
	if (!muted)
		Mix_PlayChannel(-1, display.mFSound, 0);

	changeTurn();
	genOrderedMoveList();
	checkCheck(side);
	std::cout << "White material: " << whiteMaterial << " Black material: " << blackMaterial << '\n';
	std::cout << "Current FEN (ply " << ply << ", move " << (ply-1)/2+1 << "): " << getFEN() << '\n';
	std::cout << "Current Zobrist: " << zobrist.key << '\n';
	
    if (drawCheck() == 2) {
		std::cout << "Threefold repetition.\n";
		sideInCheckmate = 3;
	}
	std::cout << "-----------------------------------------------------------------------------\n\n";
	
}

void Board::undoMove(){
    if (!movesMade.size())
        return;

    if (movesMade.back() == 0){
        movesMade.pop_back();
        moveInfo.pop_back();
        --ply;
        changeTurn();
        checkCheck(side);
        return;
    }

    changeTurn();
    unmovePiece();
    genOrderedMoveList();
    checkCheck(side);

    moveFrom = movesMade.back() / 100;
    moveTo = movesMade.back() % 100;

}

void Board::restart(){

    if (!start)
        return;

    if (ply == (int)movesMade.size()){
        while (movesMade.size())
            undoMove();
        start = false;
        sdt::cout << "Restarted game\n";
    }
    else {
        while(movesMade.size())
            undoMove();
        moveTo = 0;moveFrom = 0;
        cout << "Reloaded FEN\n";
    }

}

string Board::getFen(){

    string FEN;
    int emptyCount = 0;

    for (int j = 0;j < 8;++j){
        emptyCount = 0;
        for (int i = A8 - j * 10;i <= H8 - j * 10;++i){
            if (board120[i] == none)
                ++emptyCount;
            else {
                if (emptyCount > 0){
                    FEN += to_string(emptyCount);
                    emptyCount = 0;
                }
                FEN += piece[board120[i]].getAbbr();
            }
        }
        if (emptyCount > 0){
            FEN += to_string(emptyCount);
            emptyCount = 0;
        }
        if (j != 7)
            FEN += '/';
    }

    if (side)
        FEN += "w";
    else FEN += "b";

    if (piece[wK].getMoved() == 0){

        if (piece[wkR].getMoved() == 0)
            FEN += 'K';
        if (piece[wqR].getMoved() == 0)
            FEN += 'Q';
    }
    if (piece[bK].getMoved() == 0){
        if (piece[bkR].getMoved() == 0)
            FEN += 'k';
        if (piece[bqR].getMoved() == 0)
            FEN += 'q';
    }

    if (FEN.back() == ' ')
        FEN += '-';

    if (moveInfo.size() && moveInfo.back().epSq != 0)
        FEN += " " + intToSquare(moveInfo.back().epSq) + " ";
    else FEN += "-";

    if (!moveInfo.size())
        FEN += "0";
    else FEN += to_string(moveInfo.back().halfMoveClock);

    FEN += " " + to_string(ply / 2 + 1);

    return FEN;
}

int Board::getFromMoveList(bool s, int i) const{

    if (s) 
        return whiteMoveList[i];
    else return blackMoveList[i];
    
}

int Board::getMoveListSize(bool s) const{
    if (s)
        return int(whiteMoveList.size());
    else return int(blackMoveList.size());
}

int Board::getEpSq(int i) const{
    //
    return moveInfo[i].epSq;
}

int Board::getPmSq(int i) const{
    return moveInfo[i].pmSq;
}

int Board::getPrevOnMoveTo(int i) const {
	assert(i > -1 && i < (int)moveInfo.size());
	return moveInfo[i].prevOnMoveTo;
}

int Board::getPieceMoved(int i) const {
	assert(i > -1 && i < (int)moveInfo.size());
	return moveInfo[i].pieceMoved;
}

int Board::getMoveMade(int i) const{

    return movesMade[i];
}

int Board::getNumMovesmade() const{
    return int(movesMade.size());
}

int Board::getLastMove() const{

    if (movesMade.size())
        return MovesMade.back();
    
    return 0;
}

void Board::addToMoveList(bool s, int v){

    if (s)
        whiteMoveList.push_back(v);
    else blackMoveList.push_back(v);
}

void Board::clearMoveList(bool s){

    if (s)
        whiteMoveList.clear();
    else blackMoveList.clear();

}

const int& Board::operator [](const int index) const {
	assert(index > -1 && index < 120);
	return board120[index];
}

int& Board::operator [](const int index) {
	assert(index > -1 && index < 120);
	return board120[index];
}



