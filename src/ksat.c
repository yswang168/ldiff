/*
 To compile as follows:
 g++ ksat.c -lsprng
 */
const char *synopsis =
"ksat <k> <n> <m> <#>\n\
Produces a random CNF formula in DIMACS format.\n\
 Parameters are k,n,m,number.\n\
 k := number of literals per clause\n\
   if k<0 then compute the clauses whose length is no longer than k \n\
 n := number of variables\n\
 m := number of clauses -- sampled without replacement\n\
 id# := formula id -- specifying how the random\n\
        number generator should start\n\
 If m<m', then phi_{k,n,m,#} subset phi_{k,n,m',#}.\n\
 Clauses are chosen uniformly at random and independently\n\
 of one another (without replacement), and choosing different\n\
 id#'s gives independent formulas.\n\
 It is required that k>0, n>0, m>=0, id#>=0,\n\
 and that n>=k.\n\
 If m>[n choose k]*2^k (the number of possible clauses),\n\
 it will be reduced to this number.\n\
";
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#define CHECK_POINTERS
#include "sprng_cpp.h"

#define MINI_SAT "SAT"
#define MINI_UNSAT "UNSAT"

#include <fstream>
#include <string>

using namespace std;

int GetMaxKClauses(int k, int n);
bool Satisfaible(int *clause);

main (int argc, char *argv[])
{
  int k,n,m,id,gtype;
  Sprng *str;
  unsigned long long int max;

  int *clauses;
  int i, j, l;
  int var, mask, shift;
  int hashmask, hashshift, *hashfn, *hashtbl, *phash, hash;
  /* Use &mask for low bits, >>shift for high bits.
   * Recommendation is to use high bits.
   */
  
  /* Parse arguments, and set up random stream
   */
  bool bk = false; // for clauses of length from 1 to k if bk is true
  bool bsat = false; // generate only satisfiable theories.
  int *Length = NULL; // to record the number of j-clauses (j=1,2,...,k)
  int rr, kk;

  if (argc != 5) {fprintf(stderr,synopsis); exit(-1);}
  k = atoi(argv[1]);
  n = atoi(argv[2]);
  m = atoi(argv[3]);
  id= atoi(argv[4]);

  if (k < 0) {
	  k = -k;  bk = true;
  }

  if (k<=0 || n<=0 || m<0 || id<0 || k>n) {fprintf(stderr,synopsis); exit(-1);}
  
  if (!bk) // only for Horn k-clauses
	  max = GetMaxKClauses(k, n);
  else {
	  Length = (int *)malloc(k*sizeof(int));
	  if (!Length) { fprintf(stderr, "Out of memory.\n"); exit(-1); }
	  for (i = 1; i <= k; i++) {
		  if (i==1) 
			  Length[i - 1] = GetMaxKClauses(i, n);
		  else
			  Length[i - 1] = Length[i - 2] + GetMaxKClauses(i, n);
	  }
	  max = Length[k - 1];
  }
  
  if (m>max) m=max;
  gtype = SPRNG_MLFG;
  str = SelectType(gtype);
  str->init_sprng(id,/*nstreams*/ 8,make_sprng_seed(),SPRNG_DEFAULT);

  /* Set up hash table and hash function to ensure sampling without replacement
   */
  hashfn = (int*)malloc((2*n+1)*sizeof(int));

  if (!hashfn) {fprintf(stderr,"Out of memory.\n"); exit(-1);}

  for (hashmask=1, hashshift=31; hashmask<2*m; hashmask<<=1, --hashshift);

  hashtbl = (int*)calloc(hashmask,sizeof(int));

  if (!hashtbl) {fprintf(stderr,"Out of memory.\n"); exit(-1);}

  --hashmask;

  for (i=0; i<=2*n; ++i)
    hashfn[i] = (str->isprng()>>hashshift);

  phash = hashfn;
  hashfn = &(hashfn[n]);
  
  /* Generate random k-clauses
   */
  clauses = (int*)malloc(k*m*sizeof(int));

  if (!clauses) {fprintf(stderr,"Out of memory.\n"); exit(-1);}

  for (mask=1, shift=31; mask<n; mask<<=1, --shift);

  --mask;
  for (i=0; i<m;) {
    /* Generate a random k-clause with literals sorted by variable # */
	  if (!bk) kk = k;
	  else
	  {
		  rr = str->isprng(); // the high bits, to choose a clause length of kk
		  rr %= max;
		  rr++;
		  kk = 0;
		  while (rr > Length[kk]) kk++;
		  kk++;
	  }
	  
    /* Choose kk variables */
    for (j=0; j<kk;) {
      while ((var=(str->isprng()>>shift))>=n); 
	  ++var;
      for (l=0; l<j && clauses[i*k+l] != var; ++l);
      if (l==j) { /* variable new to the clause */
		for (l=j; l>0 && clauses[i*k+l-1] > var; --l) // insert the var in order
			clauses[i*k+l] = clauses[i*k+l-1]; // the l-th literal of i-th k-clause
		clauses[i*k+l] = var;
		++j;
      }
    }
    /* Assign signs */
    for (j=0; j<kk; ++j) 
		if (str->isprng()&0x40000000) clauses[i*k+j] *= -1;

	// the other partes are assigned to 0
	for (j = kk; j < k; j++)
		clauses[i*k + j] = 0;

    /* Check to see if we generated this i-th clause before */
    for (hash=0,j=0; j<k; ++j) 
		hash ^= hashfn[clauses[i*k+j]];

    while (hashtbl[hash] &&  bcmp(&(clauses[i*k]), &(clauses[(hashtbl[hash]-1)*k]), k*sizeof(int)))
      hash = (hash+1) & hashmask;

    if (!hashtbl[hash]) { /* new clause */
      hashtbl[hash] = i+1;
      ++i;
    }
  }


   /* Output the k-clauses in DIMACS format */
    printf("p cnf %d %d\n",n,m);
    for (i=0; i<m; ++i) {
		for (j = 0; j<k; ++j) {
			if (clauses[i*k + j] != 0)
				printf("%d ", clauses[i*k + j]);
		}
		printf("0\n"); 
    }

	// free memory
	str->free_sprng();
	free(hashtbl);
	free(clauses);
	if (bk) free(Length);
	free(phash);

  exit(0);
}



// the number of maximum  k-clause with n variables
int GetMaxKClauses(int k, int n)
{
	int max, j;

	for (max = 1, j = 0; j<k; ++j)
		max = max * 2 * (n - j) / (j + 1); // max number of k-CNF clauses

	return max;
}
/*
bool Satisfaible(int *clause, int m){

	char SatRun[100];
	char buf[11] = ".tmpXXXXXX";
	char resbuf[11] = ".resXXXXXX";
	int fd;
	FILE *pf;

	fd = mkstemp(resbuf);
	close(fd);

	fd = mkstemp(buf); // the temporary file name is in buf	
	pf = fdopen(fd, "w+");
	close(fd);

	fprintf(pf, "p cnf %d %d\n", n, m);
	for (i = 0; i<m; ++i) {
		for (j = 0; j<k; ++j) {
			if (clauses[i*k + j] != 0)
				printf(pf, "%d ", clauses[i*k + j]);
		}
		fprintf(pf, "0\n");
	}
	fclose(pf);
	sprintf(SatRun, "minisat -verb=0 %s %s 2>1 1>/dev/null", buf, resbuf);
	
	system(SatRun);
	ifstream ifs(resbuf);

	string res;
	ifs >> res;
	ifs.close();

	remove(buf);
	remove(resbuf);

	if (0 == res.compare(MINI_SAT))
		return true;
	else 
		return false;
}*/
