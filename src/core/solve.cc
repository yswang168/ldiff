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

#include <vector>
#include <string>
#include <bitset> 
#include <set>
#include <algorithm>

#include <stdint.h>
#include <string.h>

#include "global.h"
#include "ddimacs.h"
#include "solve.h"

extern bool bEmbed;
extern bool bputfile;
extern bool bout;
using namespace std;
   
int Cnm(int n, int k) { // compute the combination number C_n^k.
	int i;
	int mn;
	int mx;
	int value;

	mn = k;
	if (n - k < mn)	{
		mn = n - k;
	}

	if (mn < 0)	{
		value = 0;
	}
	else if (mn == 0) {
		value = 1;
	}
	else	{
		mx = k;
		if (mx < n - k)		{
			mx = n - k;
		}
		value = mx + 1;

		for (i = 2; i <= mn; i++)		{
			value = (value * (mx + i)) / i;
		}
	}

	return value;
}
/*
int OneBits(uint32_t v) { // compute the number of bit 1 of the variable c 32 bit integer
	// It is from http://thecodeway.com/blog/?p=711
		
	v = v �C ((v >> 1) & 0x55555555);                    
	v = (v & 0x33333333) + ((v >> 2) & 0x33333333);     
	return (((v + (v >> 4) & 0xF0F0F0F) * 0x1010101) >> 24); 	
}
*/
void cEMSet::init(vector<int> &vs) {

	data = vs;
	size = vs.size(); 
	if (buf != NULL) 
		delete buf;
	buf = new int[size];
	return;
}

cEMSet::cEMSet(vector<int> &vs){
    buf = NULL;
	init(vs);	
}

void cEMSet::next(vector<int> &subset) {
	
	ksubset_colex_successor(num, size, buf, rank); 
	subset.clear();
	for (int i = 0; i < num; i++)
		subset.push_back(data[ buf[i] - 1 ]);
	//sort(subset.begin(), subset.end());
	return;
}

void CSolve::init(CDimacs *k1, CDimacs *k2, vector<int> &s, CDimacs *res) {
	kb1 = k1; 
	kb2 = k2; 
	//kb1sat = kb1->sat();
	//kb2sat = kb2->sat();
	diff = res;
	sig = s;
	return;
}

void CSolve::emsolve()
{
	unsigned int i, j, k, ti, clause_len = sig.size(), positive_lit = clause_len, plit;
	cEMSet empos(sig), emneg(sig);
	vector<int> vpos, vneg, tv, vclause; 
	//int pset[MAXSIGLENGTH], nset[MAXSIGLENGTH];
	bool bpset[MAXSIGLENGTH]; // , bnset[MAXSIGLENGTH];
	int *subset;
	string clause;

	if (bma && kb1->IsHorn() && kb2->IsHorn()) positive_lit = 1;
	if (clause_len > 2 &&  bma && kb1->Is2CNF() && kb2->Is2CNF()) clause_len = 2; 
	/* initial the number of k-clauses to be 0*/
	for (i = 0; i <= sig.size(); diff->num_Kclauses.push_back(0), i++); 
	if (kb1->initialUNSAT == 1 && bout) {
		diff->num_clauses++;
		diff->num_Kclauses[0] ++;
		clause.append("0\n");
		if (bputfile || !bEmbed) diff->add_clause(clause);
		if (bout && bDetailed) 	cout << "0" << endl;
	}

	for (i = 1; i <= clause_len; i++) // for each i: the length of the clause  
	{
		plit = (positive_lit == 1 ? positive_lit : i);
		emneg.init(sig);
		for (j = 0; j <= plit; j++)  // for each j: the number of positive literal  
		{
			///////////////////////////////////////////////////
			if (j == 0)	{ // no positive literals		 
				emneg.setnum(i);
				emneg.next(vneg);
				do {
					// construct the clause from vpos and vneg and checking satisfiability
					if (0 != kb1->can_derive(vneg, false))
					{
						emneg.next(vneg); continue;
					}  // kb1 cannot derive the clauses not_vneg, or unknown
					if (1 != kb2->can_derive(vneg, false))
					{
						emneg.next(vneg); continue;
					} // kb2 can derive the clauses  not_vneg, or unknown
					ti = 0;
					vclause.clear();
					while (ti < vneg.size()) {
						vclause.push_back(-vneg[ti]); 
						ti++;
					} 
					if (diff->get_num_clause() == 0
						|| bout // for logical difference
						|| (bma && !diff->subsum_(vclause)) // for prime difference
						|| (bmin && (diff->can_derive(vneg, false) == 1)) ) // for a minimal difference						
						//if (!bma || diff->get_num_clause() == 0 || !diff->subsum(vclause)){ //diff->can_derive(vneg, false) != 0){						
					{
						// construct the clause
						if (bout || !bEmbed) { 						
							clause.clear();
							BuildClause(vneg, false, clause);	
							clause.append("0 \n");
							if (bputfile || !bEmbed) diff->add_clause(clause);
							if ((bout || bma) && bDetailed) 	cout << clause.c_str(); 
							diff->num_clauses ++;
							diff->num_Kclauses[vneg.size()] ++;							
						}else{
							diff->add_clause(vneg, false);
						}
					}
					emneg.next(vneg);
				} while (!emneg.last());
			}
			///////////////////////////////////////////////////
			if (j == i) { // no negative literals
				//empos.init(sig);
				empos.setnum(i);
				empos.next(vpos);
				do {					
					// construct the clause from vpos and vneg and checking satisfiability
					if (0 != kb1->can_derive(vpos, true))
						{empos.next(vpos); continue;}  // kb1 cannot derive the clauses not_vneg, or unknown
					if (1 != kb2->can_derive(vpos, true))
						{empos.next(vpos); continue;} // kb can derive the clauses  not_vneg, or unknown				 

					if (diff->get_num_clause() == 0
						|| !(bma || bmin)  // for logical difference
						|| (bma && !diff->subsum_(vpos)) // for prime difference
						|| (bmin && (diff->can_derive(vpos, true) == 1))) // for a minimal difference
						//if (!bma || diff->get_num_clause() == 0 || !diff->subsum(vpos) != 0){
					{
						// construct the clause 
						if (bout || !bEmbed) { 						
							clause.clear();
							BuildClause(vpos, true, clause);	
							clause.append("0 \n");
							if (bputfile || !bEmbed) diff->add_clause(clause);
							if ((bout || bma) && bDetailed) 	cout << clause.c_str();
							diff->num_clauses ++;
							diff->num_Kclauses[vpos.size()] ++;							
						}else{
							diff->add_clause(vpos, true);
						}
					}

					empos.next(vpos);
				} while (!empos.last());
			}
			///////////////////////////////////////////////////
			if (0 < j && j < i) {
				empos.setnum(j);
				empos.next(vpos); // the first call for enumerate the subset
				do {					
					// construct the negative part
					bzero(bpset, MAXSIGLENGTH / 8);
					subset = empos.get_buf();
					for (k = 0; k < j; k++) 
						bpset[subset[k] - 1] = true;
					tv.clear();
					for (k = 0; k < sig.size(); k++){
						if (!bpset[k]) 
							tv.push_back(sig[k]);
					}
					emneg.init(tv);
					emneg.setnum(i - j);
					emneg.next(vneg);
					do {						
						// construct the clause from vpos and vneg and checking satisfiability
						if (0 != kb1->can_derive(vpos, vneg))
							{emneg.next(vneg); continue;}  // kb1 cannot derive the clauses not_vneg, or unknown
						if (1 != kb2->can_derive(vpos, vneg))
							{emneg.next(vneg); continue;} // kb can derive the clauses  not_vneg, or unknown

						vclause.clear();
						ti = 0;
						while (ti < vpos.size()) {
							vclause.push_back(vpos[ti]);
							ti++;
						}
						ti = 0;						
						while (ti < vneg.size()) {
							vclause.push_back(-vneg[ti]);
							ti++;
						}						

						if (diff->get_num_clause() == 0
							|| (!bma && !bmin)  // for logical difference
							|| (bma && !diff->subsum_(vclause)) // for prime difference
							|| (bmin && (diff->can_derive(vpos, vneg) == 1))) // for a minimal difference
							//if (!bma || diff->get_num_clause() == 0 || !diff->subsum(vclause) ){//diff->can_derive(vpos, vneg) != 0){
						{
							// construct the clause 
						if (bout || !bEmbed) {// print the clause							
							clause.clear();
							BuildClause(vpos, true, clause);
							BuildClause(vneg, false, clause);
							clause.append("0 \n");
							if (bputfile || !bEmbed) diff->add_clause(clause);
							if ((bout || bma) && bDetailed) 	cout << clause.c_str();
							diff->num_clauses ++;
							diff->num_Kclauses[vneg.size() + vpos.size()] ++;
							
						}else diff->add_clause(vpos, vneg);						
						}
						emneg.next(vneg); // get the next subset of negativ literals
					} while (!emneg.last());
					empos.next(vpos);  // get the next subset of positive literals
				} while (!empos.last()); 
			} // end of the case with both positive and negative clauses within the length i
		} // end of the clauses in length i
	} // end of all the clauses 
	return;
}
