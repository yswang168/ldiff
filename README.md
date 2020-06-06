# ldiff
compute logical (clausal, prime and mimial) difference between propositional CNF theories over a given signature.

For example, for the below two theories 10-24-1 10-24-2 and the signature 10-24:
==========================================
more 10-24*
::::::::::::::
10-24
::::::::::::::
5 10 2 4 7 8 1 6 3 9
::::::::::::::
10-24-1
::::::::::::::
p cnf 10 4
-1 3 9 0
-1 4 -5 0
-1 -2 -5 0
-1 5 -9 0
::::::::::::::
10-24-2
::::::::::::::
p cnf 10 4
-2 -6 10 0
-2 5 6 0
-3 -5 -6 0
2 4 9 0

==========================================
Executing the following command will get 

ldiff -d -embed -pd 10-24-1  10-24-2 10-24

WARNING! DIMACS header mismatch: wrong number of variables.
-5 -2 -1 0
-9 -2 -1 0
3 -2 -1 0
4 -5 -1 0
4 -9 -1 0
5 -9 -1 0
4 3 -1 0
5 3 -1 0
9 3 -1 0
Memory used (MB) : 6.78906
CPU time (second): 0.130986
The number of difference: 9
The clauses of individual length are: 0:0 1:0 2:0 3:9 4:0 5:0 6:0 7:0 8:0 9:0 10:0

==========================================
Executing the following command will get 
ldiff -d -embed -MIN 10-24-1  10-24-2 10-24
WARNING! DIMACS header mismatch: wrong number of variables.
-5 -2 -1 0
4 -5 -1 0
5 -9 -1 0
9 3 -1 0
Memory used (MB) : 6.78516
CPU time (second): 0.138314
The number of difference: 4 : (5)
The clauses of individual length are: 0:0 1:0 2:0 3:4 4:0 5:0 6:0 7:0 8:0 9:0 10:0

==========================================
ldiff -embed  10-24-1  10-24-2 10-24
WARNING! DIMACS header mismatch: wrong number of variables.
Memory used (MB) : 6.78906
CPU time (second): 0.130681
The number of difference: 2978
The clauses of individual length are: 0:0 1:0 2:0 3:9 4:96 5:372 6:686 7:806 8:646 9:314 10:49

