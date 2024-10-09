# Monitors

* How difficult do you consider the chosen problem?

        I consider that the problem is of medium difficulty. Personally, I chose the one that I liked the most because I like the graphs.

* What do you feel made it difficult (or easy)?

        Not everyone enjoys using this particular type of data structures, which requires quite a bit of understanding.
        On the other hand, solving the problem on paper wasn't so hard.

* Is your solution general, or only partial, working just for some inputs?

        I'd love to think it is general, but with just 2 tests I can't be sure.
        The general solution is the inneficient one, that relies on checking every single permutation until one is found.
        In conclusion, my solution works on the given tests, but I don't think these count all the edge cases.

* How does your solution scale with the size of the input? For example, on small inputs your solution might
finish in under a second; how long do you think it would take on an input 10 times as large?

        My solution has an exponential complexity due to the fact that it has for in for...
        so for an input 10 times larger it could take 100 times longer.

## Explained solution
    After reading the graph from the input file(the data structure I used has been designed for oriented graphs, so I just doubled the arches),
    I go through each node to find the one with the highest number of unmarked arches.
    Once I find the node I was looking for, I mark all the links from it as visited and I count the node towards the solution.
    When all the arches are marked, the problem is solved and the solution is printed in the terminal.

### Note
    This implementation is very efficient because it is using greedy.
    However, it is not the most efficient solution.
    The most efficient solution would be to use a backtracking algorithm that would check all the permutations of the graph and find the one with the smallest number of arches.
    This solution is not efficient because it has an exponential complexity, but it is the most general one.
