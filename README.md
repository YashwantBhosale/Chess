## Chess Engine in C
This project was created as a part of the "Data structures and algorithms" course at COEP. We tried to make a chess program in C, which not only understands the legal moves of the pieces but also has a basic AI to play against the user. The program is still in development and we are working on improving the bot and adding more features to the game. 
We consider this a preliminary version of the engine and are actively working on improving it.
You can find pointer diagram for the project [here](/docs/Pointer_Diagram.png) and detailed report [here](/docs/Chess%20Engine%20Report.pdf).

### Motivation
 Chess is a game that has been around for centuries and has been a subject of study for many mathematicians and computer scientists. The game has a simple set of rules but has a very complex set of possibilities. This makes it a very interesting game to study and implement. It is very amusing to see computer understanding and evaluating millions of positions and spit out the best move. This project was a way for us to understand how chess engines work. 

### Idea
The idea of chess engine has two important parts at its core:
- **Search**: The engine needs to search through all the possible moves (or a subset of them) and evaluate the position after each move. This is done using a search algorithm like minimax or alpha-beta pruning.
- **Evaluation**: The engine should be able to tell how good a position is and personally, this is my favorite part of the engine. We have implemented basic ideas like material count, blocked pawns, etc. to evaluate the position, but there are lot of other ideas that we as humans use to evaluate a position and I really like to test them out on the engine.
For example, 
You may see something called as "encouragement matrix" in evaluation.c file. This is a matrix that tells the engine to encourage specific pieces to move to specific squares. Although this is very standard and basic technique, it is mostly used by humans to evaluate a position, like knights should be in the center to maximize their activity or rooks should be on open files. 
Although this part is still in development and we are working on improving the evaluation function.

### Features
- **Single player mode**: Play against the computer. You can choose the difficulty level of the bot (1-3).
- **Two player mode**: Play against a friend.
- **Perft**: A performance test that counts the number of legal moves in a position.

### Data Structures and Algorithms used
- **Bitboards**: These are used to represent the board and the pieces. This is a very efficient way to represent the board and is used by most of the modern chess engines.
- **1D, 2D arrays, Linked lists**: These are used to store the board, moves, etc.
- **Minimax algorithm**: This is the heart of the engine. This is used to search through all the possible moves and gives us the move that leads to the best position according to the evaluation function.
- **Zobrist hashing**: We have used zobrist hashing to store the hash values of the board positions. This is used to store the transposition table.
- **Opening book**: We have used a simple opening book to store the opening moves. This is used to speed up the search in the opening phase of the game. You can find the opening book [here](/src/book.csv).
- **Evaluation function**: We have used a simple evaluation function to evaluate the position. This is used to evaluate the position after each move.

### How to run
- Clone the repository
- Run the following commands:
```bash
cd src
chmod +x compile.sh
./compile.sh
./chess
```
- You can play against the computer or against a friend.

### How to run tests
- Clone the repository
- You can configure the tests in the [perft.c](/src/perft.c) file.
- Run the following commands:
```bash
cd src
chmod +x test.sh
./test.sh
./tests
```
### Future work
There are things we are working on to improve the engine:
- **Evaluation function**: There are lot of ideas that can be implemented to improve the evaluation function. Like mobility, pawn structure, king safety, etc. 
Reference: [Chess Programming wiki](https://www.chessprogramming.org/Evaluation)
- **Search Time**: The engine is still slow and we are working on improving the search time. We are working on implementing iterative deepening, quiescence search, etc.
- **UCI**: Engine currently does not support UCI protocol. We are working on implementing UCI protocol so that the engine can be used with other GUIs.

### How to contribute?

1. Fork the repository
2. Create a new branch
```bash
git checkout -b new_branch
```
3. Make changes
4. Commit your changes
```bash
git commit -m "Message"
```
5. Push your changes
```bash
git push origin new_branch
```
6. Create a pull request

Contributions are highly appreciated. Please create a pull request if you have any suggestions or improvements.

### Authors
- [Yashwant Bhosale](https://github.com/YashwantBhosale)
- [Aryan Jotshi](https://github.com/AryanJotshi)

Happy Coding! :heart: