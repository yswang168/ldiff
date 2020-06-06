// Copyright (C) 2014-2016 Yisong Wang
// csc.yswang@gzu.edu.cn
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//  

#ifndef SOLVE__H
#define SOLVE__H

#include <set>
#include <bitset>
#include <list>
#include <stdint.h>
#include <assert.h>

#include "global.h"
#include "combo.h"
#include "ddimacs.h"
// to use the function:
// ksubset_colex_successor ( int k, int n, int t[], int &rank );
//    Input, int K, the number of elements each K subset must
//    have.  1 <= K <= N.
// 
//    Input, int N, the number of elements in the master set.
//    N must be positive.
// 
//    Input/output, int T[K], describes a K subset.  T(I) is the
//    I-th element.  The elements must be listed in DESCENDING order.
//    On input, T describes a K subset.
//    On output, T describes the next K subset in the ordering.
//    If the input T was the last in the ordering, then the output T
//    will be the first.
// 
//    Input/output, int &RANK, the rank.
//    If RANK = -1 on input, then the routine understands that this is
//    the first call, and that the user wishes the routine to supply
//    the first element in the ordering, which has RANK = 0.
//    In general, the input value of RANK is increased by 1 for output,
//    unless the very last element of the ordering was input, in which
//    case the output value of RANK is 0.

using namespace std;
 
extern bool bma; 
extern bool bmin;
extern bool bDetailed; // if it is truee, the clausal difference is output as well 



int Cnm(int n, int m); // compute the combination number C_n^m.
//int OneBits(uint32_t c); // compute the number of bit 1 of the variable c

class cEMSet{
	vector<int> data;
	int *buf;  // to keep the subset, which must be globally defined 
	int size; // the number of elements of data
	int rank; // the current value for the considered subset, which corresponds to the parameter randk of
			 // the function  ksubset_colex_successor ( int k, int n, int t[], int &rank )
			 // The intial value of rank is -1 indicating the first call
			 // RANK is 0 showing the return is the last one

	int num; // number of elements in the considered subset 
			 // to enumerate the subset of NUM elements in the SIZE set. 

public:
	int *get_buf() { return buf; }
	void setnum(int n) {  // set the N-elements suset of the set data
		assert(n <= size);
		num = n; 
		rank = -1;   
	} // reset the NUM number of subsets

	cEMSet(){}
	~cEMSet() { delete buf; }
	cEMSet(vector<int> &vs); 

	void init(vector<int> &vs); // set the set, which is considered to choose subsets

	void next(vector<int> &subset );  // get the next subset containing n elements of the set
					// to call the function  ksubset_colex_successor ( int k, int n, int t[], int &rank );
					// return the result in subset
	bool last() { return (rank == 0); }
};
 
class CSolve
{
	int kb1sat, kb2sat;
	CDimacs *kb1, *kb2;
	vector<int> sig;
	CDimacs *diff; // to keep the result 

public:
	void init(CDimacs *k1, CDimacs *k2, vector<int> &s, CDimacs *res = NULL);

	CSolve() { 
		kb1sat = false; 
		kb2sat = false;   
	}

	~CSolve(){ 
	}
	  
	void emsolve();  // compute the logical difference of kb1 and kb2 over the signature by emuneration all the candidate clauses

};

#endif
