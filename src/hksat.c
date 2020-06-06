/*
  The is modified from ksat.c 
  http://research.microsoft.com/en-us/um/people/dbwilson/ksat/ksat.c
  for compute random k-Horn clauses

  It makes use of SPRNG
  M. Mascagni and A. Srinivasan (2000), "Algorithm 806: SPRNG: A Scalable Library for Pseudorandom Number Generation," ACM Transactions on Mathematical Software, 26: 436-461.
  http://www.sprng.org/

  To compile:
  g++ hksat.c -lsprng

  Yisong Wang
  2015.8.8
*/
const char *synopsis =
"ksat <k> <n> <m> <#>\n\
Produces a random CNF formula in DIMACS format.\n\
 Parameters are k,n,m,number.\n\
 k := number k of literals per clause\n\
 if k<0 then compute the clauses whose length is no more than k+1 \
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
#include <stdlib.h>
#include <string.h>
#define CHECK_POINTERS
#include "sprng_cpp.h" 

int GetMaxHornClauses(int k, int n);

main (int argc, char *argv[])
{
  int k,n,m,id,gtype;
  bool bk = false; // for horn clauses of length from 1 to k if bk is true
  Sprng *str;
  unsigned long long int max;

  int *clauses;
  int i, j, l;
  int var, mask, shift;
  int hashmask, hashshift, *hashfn, *phash, *hashtbl, hash;
  int *HornLength=NULL;
  int rr, kk;
  /* Use &mask for low bits, >>shift for high bits.
   * Recommendation is to use high bits.
   */
  
  /* Parse arguments, and set up random stream
   */
  if (argc != 5) {fprintf(stderr,synopsis); exit(-1);}
  k = atoi(argv[1]);
  n = atoi(argv[2]);
  m = atoi(argv[3]);
  id= atoi(argv[4]);
  if (k < 0) {
	  k = -k;  bk = true;
  }
  if (k<=0 || n<=0 || m<0 || id<0 || k>n) {fprintf(stderr,synopsis); exit(-1);}

  max = 0;
  if (!bk) // only for Horn k-clauses
	  max = GetMaxHornClauses(k, n);
  else {
	  HornLength = (int *)malloc(k*sizeof(int));
	  if (!HornLength) { fprintf(stderr, "Out of memory.\n"); exit(-1); }
	  for (i = 1; i <= k; i++) {
		  if (i == 1)
			  HornLength[i - 1] = GetMaxKClauses(i, n);
		  else
			  HornLength[i - 1] = HornLength[i - 2] + GetMaxKClauses(i, n);
	  }
	  max = HornLength[i - 1];
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
		  rr = str->isprng(); // the high bits
		  rr %= max;
		  rr++;
		  kk = 0;
		  while (rr > HornLength[kk]) kk++;
		  kk++;
	  }
    /* Choose kk variables */
    for (j=0; j<kk;) {
      while ((var=(str->isprng()>>shift))>=n); 
	  ++var;
      for (l=0; l<j && clauses[i*k+l] != -var; ++l);
      if (l==j) { /* variable new to the clause */
		for (l=j; l>0 && clauses[i*k+l-1] > -var; --l) // insert the var in order
			clauses[i*k+l] = clauses[i*k+l-1]; // the l-th literal of i-th k-clause
		clauses[i*k+l] = -var; //negative the variable
		++j;
      }
    }
    /* Assign signs */
	if (str->isprng() % 0x40000000) // one positive literal
	{
		rr = (str->isprng()) % kk; // the high bits
		clauses[i*k + rr] *= -1;
	}
    // for (j=0; j<kk; ++j) 
	//	if (str->isprng()&0x40000000) clauses[i*kk+j] *= -1;
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
      for (j=0; j<k; ++j) {
		  if (clauses[i*k+j] != 0)
			printf("%d ",clauses[i*k+j]);		
      }
		  printf("0\n");
    }
  

  // free memory
  str->free_sprng();
  free(hashtbl);
  free(clauses);
  if (bk) free(HornLength);
  free(phash);

  exit(0);
}


// the number of maximum horn k-clause with n variables
int GetMaxHornClauses(int k, int n)
{
	int max, j;

	for (max = 1, j = 0; j<k; ++j)
		max = max * (n - j) / (j + 1); // max number of Horn k-CNF clauses

	return max*(1+k);
}
 
