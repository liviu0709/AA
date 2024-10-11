# Turing Machine Simulator

## Implementation
    - The data is read in a structure, that is later processed into a more efficient structure:
        - One structure for each state, containing a vector of another strucure with the transitions for that state.
        - This way, the transitions are stored in a more efficient way, and the program can access them faster.
    - The tape is not infinite. Once an end is reach the tape triples in size.
    - After the input is read, the program enters a loop where it reads the tape, and based on the current state and the symbol read, it changes the state, writes a new symbol and moves the tape.
    - The program stops when the current state is qReject, qAccept, H or the state given by the user.
    - The program also stops if no transition is found corresponding the current state and tape value.

## Limitations
    - This implementation supports just 1 accepted state added by user.
    However, qReject, qAccept, Y and H are hard coded.
    - Comments can only be added at the start of the input file with //