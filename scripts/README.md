### What is this?

This folder contains some scripts that help us in testing or debugging the output of the chess engine. The scripts are written in Python, because of its simplicity and readability. The scripts are not part of the engine itself, but they are useful for testing and debugging purposes.

### Where do we need this?

So, currently the best performing chess engine is Stockfish. and we often need to compare our engine's output with Stockfish's output.

Say, we have a position and we run a perft test on it. Perft test is a test that counts the number of possible moves at a given depth. So, we run a perft test on our engine and we get the number of possible moves at a given depth. Now, we want to compare this number with Stockfish's number. So, we run the same perft test on Stockfish and get the number of possible moves at a given depth. Now, we can compare our engine's output with Stockfish's output.

The prbolem here is that order of moves in the output of the engine and Stockfish is not same. So, it is very difficult to compare them manually.

Now, we could have done this in C itself but it would have been little complex and difficult. So stockfish returns output in a format like this:

```
e2e3: 11427551
g2g3: 4190119
a5a6: 16022983
e2e4: 8853383
g2g4: 13629805
b4b1: 19481757
b4b2: 12755330
b4b3: 15482610
b4a4: 11996400
b4c4: 17400108
b4d4: 15996777
b4e4: 14187097
b4f4: 3069955
a5a4: 14139786
```

So, we need to parse this output as dictionary and find out where our engine's output is different from Stockfish's output. This is where Python scripts come in handy. We can easily parse this output and compare it with our engine's output.

### How to use this?

1. load whatever position you want to test in the engine. (Preferably, in FEN format)
2. Run the perft test on the engine and get the output. engine will return output like this:

```
"a3a4": 3878463,
"d3d4": 4380579,
"b2b3": 3419939,
"b2b4": 3711088,
"g2g3": 3878092,
"h2h3": 4197369,
"h2h4": 3743675,
"c3d5": 3699930,
"c3b5": 3665630,
"c3d1": 2638406,
"c3b1": 2773444,
"c3a4": 3477406,
"c3a2": 3313599,
"f3e5": 5187884,
"f3h4": 3800952,
"f3e1": 2861872,
"f3d4": 4645393,
"f3d2": 3640148,
"g5h6": 3822473,
"g5f6": 3337594,
"g5h4": 3278233,
```

we can just copy this and paste it inside curly braces to be used as dictionary in Python.
But, stockfish's output is not in this format. Without script we have to manually insert commas and colons to make it a dictionary. and we are too lazy to do that. ;) 

3. Copy the output of the engine and paste it in the `user_moves` dictionary in [`find_mismatch.py`](/tests/find_mismatch.py). 

4. Launch stockfish in your terminal.
```
$ stockfish
```

5. Run the perft test on stockfish.

```bash
position fen <fen_string>
go perft <depth>
```

6. Copy the output of stockfish and paste it in [`stockfish.txt`](/tests/stockfish.txt) file.
7. Finally, run the script.

```bash
$ python3 find_mismatch.py
```

Make sure you have Python installed on your system. Also, you need to have stockfish installed on your system. 

### Note
There could have been lot of better ways to do this. But, this is what I did and this documentation is just to help you understand what this folder is about and how to use it. cheers! :)
