# --coding=utf-8--
import sys
import os 
import operator 
import random

from functools import reduce

# randomly generated Horn clauses

def combination(n,k):  
    return  reduce(operator.mul, range(n - k + 1, n + 1)) /reduce(operator.mul, range(1, k +1))  

# compute the number of clauses with length 0, 1, ..., n in n variables
def clauses_bounds(n):    
    bounds=list()
    bounds.append(1)
    for i in range(1,n+1):
        bounds.append(int(combination(n,i)*(1+i)))
        
    return bounds
    
# generate a Horn clause with length k and n variables
def generate_one_Horn_clause(n,k):
    assert(k<=n)
    clause = random.sample(range(-n,0),k)
    if (random.randint(0,1)==1): # one literal is positive
        pos = random.randint(0,len(clause)-1)
        clause[pos ] = -clause[pos]
    return sorted(clause)
    
def print_clause(cl, file=sys.stdout):
    for i in range(len(cl)):
        print("%d "%cl[i], end="", file=file)
    print("0", file=file)
    
# checking if the clause cl is in clauses    
def clause_in_clauses(cl, clauses):
    # assert cl and the clauses in 'clauses' is sorted
    for i in range(len(clauses)): 
        if cl == clauses[i]: return True
    return False

# generate m Horn clauses with length k and n variables    
def generate_Horn_clauses_k(n, k, m, repeatable=False, file=sys.stdout):
    
    if k==0: print("0", file=file); return
    
    #assert(m <= combination(n,k))    
    if repeatable:
        for i in range(m):
            print_clause(generate_one_Horn_clause(n,k), file)
        return
    
    HCNF = list()
    i=0
    while i<m:
        clause = generate_one_Horn_clause(n,k) 
        if  clause_in_clauses(clause, HCNF):  continue
        HCNF.append(clause)
        print_clause(clause, file)
        i += 1
            
    return HCNF

# generate m Horn clauses with n variables    
def generate_Horn_clauses(n, m, repeatable=False, fs=None):
    
    # initialize the number of clauses of length k by 0
    CNF_ks = [0]*(n+1)   
    
    CNF_ts = [1]*(n+1)
    Bounds = clauses_bounds(n)
    for i in range(1,n+1):
        CNF_ts[i] = reduce(operator.add, Bounds[0:i+1])
        
    # get random m intergers between [0, 2**n(n+2)-1]
    CNF_ms =  random.sample(range(1,CNF_ts[n]), m) 
    
    for i in CNF_ms:
        for j in range(n+1):
            if  CNF_ts[j] >= i: break;
        CNF_ks[j] += 1
                
    if fs != None:
        File = open(fs, "w+")
    else:
        File = sys.stdout

    print("p cnf %d %d"%(n,m), file=File)
    for k in range(n+1):
        if CNF_ks[k] > 0:
            generate_Horn_clauses_k(n, k, CNF_ks[k], repeatable, File)
        
    File.close()
    return

# generate Horn CNFs at the director $root/nfor difference in terms of variable-length-clause model
# C=range([0.1, 1, 0.045]) , range([1,5,0.2])
# Prob[SAT]=0.998 (c=0.1), Prob[SAT]=0.499 (c=1), Prob[SAT]=0.993 (c=5)
'''
c       F(e^{-c})
0.1	0.002
0.2	0.025
0.3	0.07
0.4	0.13
0.5	0.21
0.6	0.28
0.7	0.35
0.8	0.42
0.9	0.48
1	0.49963
2	0.84
3	0.94
4	0.98
5	0.993
'''
def gen_Horn_CNFs(n, lenA, root, repeatable):
    if os.path.exists(root+"/"+str(n)):
        os.system("rm -fr "+root+"/"+str(n))
    os.mkdir(root+"/"+str(n))
    os.chdir(root+"/"+str(n))
    tn = 2**n;
    mclauses = list() 
    F_head=str(lenA)+"-"

    for i in range(0,21):
        c=0.1+0.045*i
        m=int(c*tn)
        mclauses.append(m)
        if os.path.exists(root+"/"+str(n)+"/"+str(m)): continue
        os.mkdir(str(m))
        os.chdir(str(m))
        for j in range(1,101):
            # generate the two random Horn theories
            generate_Horn_clauses(n, m, repeatable, F_head + str(j)+"-1")
            generate_Horn_clauses(n, m, repeatable, F_head + str(j)+"-2")

            # build the relative signature
            relativeA = random.sample(range(1,n+1), lenA)
            rAs = " ".join([str(na) for na in relativeA])
            print(rAs, file=open(str(lenA)+"-"+str(j), "w+"))

        os.chdir("..")

    for i in range(1,21):
        c=1+0.2*i
        m=int(c*tn)
        mclauses.append(m)
        if os.path.exists(root+"/"+str(n)+"/"+str(m)): continue
        os.mkdir(str(m))
        os.chdir(str(m))
        for j in range(1,101):
            # generate the two random Horn theories
            generate_Horn_clauses(n, m, repeatable, F_head + str(j)+"-1")
            generate_Horn_clauses(n, m, repeatable, F_head + str(j)+"-2")

            # build the relative signature
            relativeA = random.sample(range(1,n+1), lenA)
            rAs = " ".join([str(na) for na in relativeA])
            print(rAs, file=open(str(lenA)+"-"+str(j), "w+"))

        os.chdir("..")
    # print the clauses
    print(" ".join([str(nc) for nc in mclauses]), file=open("clauses","w+"))


if __name__ == "__main__":
    if len(sys.argv) != 5:
        print("%s n  lenA ROOT repeatable")
        sys.exit(0) 
    rep = True;
    if int(sys.argv[4]==0): rep=False
    gen_Horn_CNFs(int(sys.argv[1]), int(sys.argv[2]), sys.argv[3], rep)

