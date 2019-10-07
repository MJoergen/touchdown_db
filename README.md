# touchdown_db
This project creates a database of all legal TouchDown positions, and their
corresponding game-theoretic value.

This database can then be used e.g. to test Machine Learning and Neural Networks.

## The TouchDown Game
TouchDown is a board game where players take turns moving a piece (called
"pawn"). A pawn may move in one of two ways:
* Directly forward one square, but only if that square is empty.
* Diagonally forward one square, but only if that square is occupied by an enemy pawn.
  In that case the enemy pawn is removed from the board.

The game ends in one of two possible ways:
* If a player moves a pawn to the opponents back rank, then the player has won.
* If a player has no legal moves (or no pawns left), then the player has lost.

The initial position (on a 4x4 board) is a follows:
```
OOOO
....
....
XXXX
```
where X is the player to move.

This game may be played on a larger board too, e.g. 6x4:
```
OOOOOO
......
......
XXXXXX
```
or 8x6
```
OOOOOOOO
........
........
........
........
XXXXXXXX
```

### Examples
For instance, the following board (with X to move):
```
...O
..O.
....
.XX.
```
is a win (for X), and the winning move is b1b2.

On the other hand, the following board (again with X to move):
```
...O
..O.
..X.
.X..
```
is a loss (for X) after the only move b1b2.

So any Neural Network must be able to capture the subtle difference between the
two above board positions.

## Algorithm to generate the database
In order to build up the database, I use an algorithm where the entire database
of all legal positions is sweeped over, multiple times, gradually populating the
entries with the correct values. So while the computation is in progress, each
position may be in three possible states: "Win", "Loss", or "Unknown".

Initially, all legal positions are assigned the "Unknown" state, and then the
program repeatedly loops over all legal positions until no more changes are
being made to the database.

When considering a particular "Unknown" position the following actions are taken:
* If the position is lost (because the player has no legal moves or the first
  rank is occupied by an opponent pawn), then mark the position as known.
* Otherwise, loop over all legal moves and examine the values (in the database)
  of the successor positions. If all successor positions are known, then the current
  position is known too.

## Enumerating all the positions
In the above it is assumed that it is possible to loop over all positions. In other words,
each position is assigned an integer index into the database. For the above algorithm
to be efficient the following operations must be easy:
* For a given index construct the position on the board.
* For a given board position, determine the corresponding index.

So how should this index be constructed? Some possible approaches are:
1. There are sixteen squares (either occupied or not) for the first player "X",
   and there are (the same) sixteen squares (either occupied or not) for
   the second player "O". Therefore, the board can be represented using two
   16-bit values. The total number of values is 2^32. The conversion between
   board position and index is trivial.
2. There are sixteen squares in total, and each square may be in one of three
   states (player "X", player "O", or empty).  The total number of values is
   3^16 ~= 2^26. The conversion between board position and index is no longer
   trivial.  However, since 3^5 = 243 ~= 2^8 it is possible to map 5 squares to
   8 bits using a simple lookup table.
3. The positions of the "X" pieces are numbered by selecting four places out of
   sixteen possible (i.e. using C(16,4)) and the positions of the "O" pieces
   are numbered by selecting another four places out of the remaining twelve
   possible (i.e. using C(12,4)). The total number here is 900900 ~= 2^20, but
   the number is actually a bit larger, because we have not accounted for the
   positions where either player has fewer than four pieces.  The conversion
   between board position and index is far from trivial.

The difference in number of board positions is because some illegal positions
are allowed in these index representations. For instance, in the first
representation there is nothing to prevent both players occupying the same
square.  Furthermore, both representations 1 and 2 allow players to have more
than four pawns on the board.

The above analysis suggests that limiting the number of illegal positions
reduces the size of the database, but leads to increased complexity when
converting between board position and index representation. So there is a
tradeoff.

The scheme I've chosen in this program is the following.  The index consists of
a 24-bit number, split into two fields:
* Bits 15-0 indicate which positions on the board are occupied.
* Bits 23-16 indicate which of the occupied positions belong to the
  current player "X".

This representation uses 2^24 values. It still contains illegal positions where
e.g. more than 8 pawns are on the board. The conversion between board position
and index representation is reasonably efficient.

## Skipping over illegal positions
In the algorithm above it was assumed that the loop is over all legal positions.
So we need a way to quickly determine whether a given index corresponds to a
legal position.  This is done by introducing the following shorthands:
* "cp" is number of "1" bits in p, i.e. within bits 15-0.
* "cx" is number of "1" bits in x, i.e. within bits 23-16.

and then examining the following requirements:
1.    cx <= 4.  Reason: Player "X" may have no more than four pieces.
2. cp-cx <= 4.  Reason: Player "O" may have no more than four pieces.

Finally, one more restriction is necessary:
3. "x" < 2^cp.  Reason: Plauer "X" may not have pieces on empty squares on the
   board.

The above restrictions together reduce the number of positions as follows:
1. Reduces the count from 2^24 = 16777216 to 10682368.
2. Reduces the count from 10682368 to 4693459.
3. Reduces the count from 4693459 to 2267253.

These numbers are found by running the command
```
touchdown_rb -i
```

## Generating list of legal moves
Part of the algorithm is to generate a list of all legal moves in a given position.

For this, the index value is not very useful, so the index value is converted to
an internal board representation packed into a 32-bit number:
* Bits 15-0 indicate which squares are non-empty, i.e. contain either a player or an opponent.
* Bits 31-16 indicate which squares are occupied by the player.

Using this representation it is easy to find all the players pawns, and it is
easy to determine, if a given square is empty.

This representation must at all times satisfy the following invariant, which states that a
player pawn must be on a non-empty square.
```
(~board & (board >> 16)) == 0
```



## Swapping player to move
Another part of the algorithm is when examining a legal move and performing a
lookup in the database, the player to move has to be changed, i.e. after "X" has moved,
it is now player "O"'s turn.

### Example
The position
```
..O.
.OX.
O..X
X...
```
is represented by the index value 0x341964.

The swapped position will look like
```
...O
O..X
.OX.
.X..
```
and is represented by the index value 0x344698.

## Output results
Running the command
```
touchdown_db -s touchdown.tb
```

gives the following result:

```
Dump of statistics
Invalid Index     : 14509963
Illegal position  :  1511662
Win               :   220104
Loss              :   535487

Starting position : Win
```

So only a total of 755591 positions are actually valid. Most of the database
thus corresponds to invalid positions.

## TODO
The list of improvements and next steps is quite large:
* Generalize to larger boards.
* Allow user to query the database by inputing a specific position
* Show a best line of play, rather than just the value.

