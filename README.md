# qnx-scheduling
Experiments with Real-Time Scheduling Algorithms

# Adding new task sets
Add new task sets within fixt.c. You create a new fixt_set structure with
fixt_set_new then append the set using DL_APPEND.

# Adding new scheduling algorithms
Create a new subdirectory of fixt/impl for your algorithm. Implement the
four AlgoHooks required of a new fixt_algo. Then, instantiate the algorithm
within fixt.c and append it with DL_APPEND.
