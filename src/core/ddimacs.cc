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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <inttypes.h>
#include <fstream> 
#include <vector>
#include <algorithm>
#include <zlib.h>
/*
#include "Dimacs.h"
include "solver.h"
#include "mtl/Vec.h"
#include "SolverTypes.h"
*/
#include "ddimacs.h"
#include "Solver.h"
//using namespace std;
//using namespace Minisat;


bool FindLit(vec<Lit> &lits, Lit &lt)
{
	for (int i = 0; i<lits.size(); i++)
	if (lits[i] == lt) return true;
	return false;
}

bool subsume(vector<int> &s, vector<int>&t) {
	// if s subsumes t return true, return false otherwise
	vector<int>::iterator b = t.begin(), e = t.end();
	for (unsigned int i = 0; i < s.size(); i++)
	if (find(b, e, s[i]) == e) return false;

	return true;
}

void CDimacs::init(char *fn)
{  

	if (!bEmbed){
		char buf[11] = ".tmpXXXXXX";
		int fd;

		fd = mkstemp(buf);
		TmpFileName = buf; //tmpnam(NULL);
		close(fd);
		strncpy(buf, ".res", 4);
		TmpSatResult = buf; //tmpnam(NULL); 
		close(fd);		
        sprintf(SatRun, "minisat -verb=0 %s %s 2>/dev/null 1>/dev/null", TmpFileName.c_str(), TmpSatResult.c_str());
		read(fn);
		if (sat() == 0) initialUNSAT = 1;
	} 	
	CheckHorn();
	Check2CNF(); 
	return;
}

CDimacs::~CDimacs()
{
	if (!bsave && !bEmbed) 
		remove(TmpFileName.c_str());
	
	remove(TmpSatResult.c_str());
}

void CDimacs::read(char *file) // read the CNFs in dimacs into the temporary file
{  
	FILE *fd1, *fd2;
	char buf[512];
	int size;
	fd1 = fopen(file, "r");
	assert(fd1 != NULL);
	fd2 = fopen(TmpFileName.c_str(), "w"); 
	while ((size = fread(buf, 1, 512, fd1)))
		fwrite(buf, 1, size, fd2);

	EndPosition = ftell(fd2);

	fclose(fd1);
	fclose(fd2);
	return;
}


int  CDimacs::sat() // check whether the CNFs is satisfiable
// -1: error;  0: unsat;	1: sat
{
	if (initialUNSAT == 1) return 0; // it is initially unsatisfiable

	if (bEmbed){
		vec<Lit> dummy; 
		lbool ret = psolver->solveLimited(dummy);
		if (ret == l_True) return 1;
		if (ret == l_False)	return 0;
		return -1;
	}

	system(SatRun);
	ifstream ifs(TmpSatResult.c_str());

	string res;
	ifs >> res;
	ifs.close();

	if (0 == res.compare(MINI_SAT))
		return 1;
	else if (0 == res.compare(MINI_UNSAT))
		return 0;
	else
		return -1;
}

void skipWhitespace(char *&in) {
	while ((*in >= 9 && *in <= 13) || *in == 32)
		++in;
}
static int parseInt(char *&in) {
	int     val = 0;
	bool    neg = false;
	skipWhitespace(in);
	if (*in == '-') neg = true, ++in;
	else if (*in == '+') ++in;
	if (*in < '0' || *in > '9') fprintf(stderr, "PARSE ERROR! Unexpected char: %c\n", *in), exit(3);
	while (*in >= '0' && *in <= '9')
		val = val * 10 + (*in - '0'),
		++in;
	return neg ? -val : val;
}

bool CDimacs::CheckHorn() { // return true if it is a Horn theory
	char buf[MAX_CLAUSE_LENGTH], *p; 
	int pnum;
	int lit;

	ifstream ifs(TmpFileName.c_str());

	while (ifs.getline(buf, MAX_CLAUSE_LENGTH)){
		if (buf[0] == 'c' || buf[0] == 'p') continue;
		p = buf;
		pnum = 0; 
		for (;;){
			lit = parseInt(p);
			if (lit == 0) break; 
			if (lit > 0) pnum++;
			if (pnum > 1) { ifs.close();  return false; }
		}
	}
	ifs.close();
	bHorn = true;
	return true;
}

bool CDimacs::Check2CNF(){ // return true if it is a 2CNF theory
	char buf[MAX_CLAUSE_LENGTH], *p;
	int size;
	int lit;

	ifstream ifs(TmpFileName.c_str());

	while (ifs.getline(buf, MAX_CLAUSE_LENGTH)){
		if (buf[0] == 'c' || buf[0] == 'p') continue;
		p = buf;
		size = 0;
		for (;;){
			lit = parseInt(p);
			if (lit == 0) break;
			size++;
			if (size > 2) { ifs.close(); return false; }
		}
	}
	ifs.close();
	b2CNF = true;
	return true;
}

void CDimacs::add_clause(vector<int> &tv, bool positive){ 
	// add one clause  to minisat solver, just for prime and minimal difference
	// these clauses must be consistent
	vec<Lit> lits;
	
	num_Kclauses[tv.size()] ++; // record the length k clause

	if (!positive) 	 neg_clause(tv); 

	if (bmin){
		toMinisatClause(tv, lits);
		AddClauseVar(tv); // the variable does depend on its sign
		psolver->addClause_(lits); 
		min_diff_clauses.push_back(tv);
	} 
	if (bma) // prime difference
		clausal_diff_clauses.push_back(tv);
	return;
}

void CDimacs::add_clause(vector<int> &pos, vector<int> &neg){ // add one clause  to minisat solver
	vec<Lit> lits;
	vector<int> tv;
	
	num_Kclauses[pos.size() + neg.size() ] ++; // record the length k clause

	for (unsigned int i = 0; i < pos.size(); i++) tv.push_back(pos[i]);
	for (unsigned int i = 0; i < neg.size(); i++) tv.push_back(-neg[i]);
  
	if (bmin) { // for both min and MIN difference
		toMinisatClause(tv, lits);
		AddClauseVar(tv);  
		psolver->addClause_(lits);
	    min_diff_clauses.push_back(tv);
	} 
	if (bma) // for clausal and prime difference, 
		clausal_diff_clauses.push_back(tv);

	return;
}

void CDimacs::add_clause(string& cl)  
{
	ofstream fs(TmpFileName.c_str(), ios_base::app);

		assert(fs.good());
		fs << cl;
		EndPosition = fs.tellp();

		fs.close();
		//num_clauses++; //yisong 2020.3.21, commented!

	return;
}



bool CDimacs::subsum_(vector<int> &v) {
	// check if the clause [positive]v is subsumed in the clauses clausal_diff_clauses
	unsigned int i; 
	for (i = 0; i < clausal_diff_clauses.size(); i++)
		if (subsume(clausal_diff_clauses[i], v)) return true;

	return false;
}

bool CDimacs::subsum(vector<int> &vt) // need only by the result of difference
{
	if (bEmbed){ // check if the clause vt is subsumed by one clause in the solver's clauses
        //if (psolver->clauses.size() == 0) return false;
		int i, j;
		vector<int> target;
        vec<Lit> lits;
        toMinisatClause(vt, lits); 

		for (i = 0; i < psolver->trail.size(); i++)
		if (FindLit(lits, psolver->trail[i])) return true; // it is subsumed by a single literal clause

		for (i = 0; i < psolver->clauses.size(); i++){
			Clause& c = psolver->ca[psolver->clauses[i]];
			j = 0;
			while (j < c.size()) {
               if (FindLit(lits, c[j])) j++;
			   else break;
            }
			if (j == c.size()) return true;
		}
		return false;
	}

	ifstream fs(TmpFileName.c_str());
	char  buf[MAX_CLAUSE_LENGTH];
	vector<int> vs;
	vector<int>::iterator ite, its;
	unsigned int i;

	bzero(buf, MAX_CLAUSE_LENGTH);
	fs.getline(buf, MAX_CLAUSE_LENGTH);
	while (!fs.eof())
	{	
		vs.clear();
		GetClause(vs, buf); 
		i = 0;
		ite = vt.end();
		its = vt.begin();
		while (i < vs.size() )	{
			if (vs[i] == 0) {
				i++;  continue;
			}
			if (find(its, ite, vs[i]) == ite)	break;
			i++;
		}
		if (i == vs.size()) {
			fs.close();
			return true;
		}
		bzero(buf, MAX_CLAUSE_LENGTH);
		fs.getline(buf, MAX_CLAUSE_LENGTH);
		if (strlen(buf) == 0) break;
	}
	fs.close();
	return false;
}

int CDimacs::can_derive(vector<int> &tv)
{
	// return 0 if the clauses can derive the clause tv
	// else return 1 if (it cannot derive the clause tv)
	vec<Lit> dummy;
	
	if (initialUNSAT == 1) return 0;
	if (initialUNSAT == 0) return 1;

	neg_clause(tv);
	toMinisatClause(tv, dummy);
	lbool ret = psolver->solveLimited(dummy);
	if (ret == l_True) return 1;
	if (ret == l_False)	return 0;
	return -1;
}

int CDimacs::can_derive(vector<int> &vi, bool positive){
	// return 0  if the clauses can derive the POSITIVE/NEGATIVE clause 
	// else return 1 if it cannot derive the [positive]vi clause
	unsigned int i;
	int result;	
	vec<Lit> dummy;
	

	if (initialUNSAT == 1 ) return 0;
	if (initialUNSAT == 0) return 1; 

	if (bEmbed){	 
		if (!positive) toMinisatClause(vi, dummy);
		else{
			//neg_clause(vi);
			vector<int> tv;
			for (unsigned int i = 0; i < vi.size(); i++) tv.push_back(-vi[i]);
			toMinisatClause(tv, dummy);
		}
		lbool ret = psolver->solveLimited(dummy);
		if (ret == l_True) return 1;
		if (ret == l_False)	return 0; 
		return -1;
	}

	ofstream fs(TmpFileName.c_str(), ios_base::app);

	for (i = 0; i < vi.size(); i++) {
		if (positive)
			fs << -vi[i] << " 0" << endl;
		else
			fs << vi[i] << " 0" << endl;
	}
	fs.close();
	result = sat();
	// truncate the clauses to its orginal position
	truncate(TmpFileName.c_str(), EndPosition);
	return result;
}

int CDimacs::can_derive(vector<int> &pos, vector<int> &neg){
	// return 0 if the clauses can derive the POSITIVE + NEGATIVE clause 
	// otherwise return 1 (it cannot derive the clause)

	unsigned int i;
	int result;

	if (initialUNSAT == 1) return 0;
	if (initialUNSAT == 0) return 1;

	if (bEmbed){
		vec<Lit> dummy;
		vector<int> tv; 
		tv.clear();
		dummy.clear();
		for (i = 0; i < pos.size(); i++) 	tv.push_back(-pos[i]);
		for (i = 0; i < neg.size(); i++)	tv.push_back(neg[i]);
		toMinisatClause(tv, dummy);
		
		lbool ret = psolver->solveLimited(dummy);
		if (ret == l_True) return 1;
		if (ret == l_False)	return 0;
		return -1;
	}

	ofstream fs(TmpFileName.c_str(), ios_base::app);

	for (i = 0; i < pos.size(); i++)  
		fs <<  -pos[i] << " 0" << endl;
	for (i = 0; i < neg.size(); i++)
		fs << neg[i] << " 0" << endl; 
	fs.close();
	result = sat();
	// truncate the clauses to its orginal position
	truncate(TmpFileName.c_str(), EndPosition);
	return result;
}


bool AllBlank(char *buf)
{	
	int len = strlen(buf), i = 0;
	while (i < len)	{
		if (buf[i] != ' ') return false;
		i++;
	}
	return true;
}

void ClauseToVector(Clause &cl, vector<int> &v)
{
	int j;
	Lit lt;

	for (j = 0; j < cl.size(); j++) {
		lt = cl[j]; 
		v.push_back(sign(lt) ? var(lt) : -var(lt));
	}
	return;
}
  
void DetachClauses(Solver*ps, vector<int> &v)
{
	for (unsigned int i = 0; i < v.size(); i++){
		//ps->detachClause(ps->clauses[v[i]]);
		ps->removeClause(ps->clauses[v[i]]);
	}
	return;
}
//extern Solver tsolver;
void CDimacs::minimize(Solver* s){ 
	vector<int> clause;   
	min_diff_removed.clear();

	if (min_diff_clauses.size() <= 1) return;

	for (unsigned i = 0; i < (min_diff_clauses.size() - 1); i++)	{
		vec<Lit> assump; 
		assump.clear();
		Solver xsolver;
		add_min_clauses(&xsolver, i); 
		clause.clear(); 
		clause = min_diff_clauses[i];
		neg_clause(clause);
		toMinisatClause(clause, assump);
		lbool lb = xsolver.solveLimited(assump);
		if (lb == l_False) {// unsatisfied 
			min_diff_removed.push_back(i);
			num_Kclauses[clause.size()] --;
		}		
	} 
	return;
}

// This functin does not work yet
// ............................
void CDimacs::MINimize(){ // check the MIN difference, i.e., the minimal difference with the last loop checking
	// This is done on the psolver.
	int i, sz = psolver->clauses.size();
	lbool ret;
	vec<Lit> lits, blits;  
	
	for (i = 0; i < sz - 1; i++)	{  
		 
		CRef &cur = psolver->clauses[i];
		lits.clear();
		blits.clear();
		Clause& cl = psolver->ca[cur];
		for (int j = 0; j < cl.size(); j++){
			lits.push(~cl[j]);
			blits.push(cl[j]);
		}

		psolver->detachClause(cur, true);
		//psolver->removeClause(cur);
		ret = psolver->solveLimited(lits); // this will leed to SEGMENT FAULT
		if (ret == l_False)
			min_diff_removed.push_back(i);
		else
			//psolver->addClause(blits);
			psolver->attachClause(cur);		
	} 
	
	return;
}

void CDimacs::minimize()
{
	fstream fs(TmpFileName.c_str(), ios_base::in | ios_base::out);
	char buf[MAX_CLAUSE_LENGTH], blank[MAX_CLAUSE_LENGTH];
	long end_of_file, cur_pos, next_pos;
	vector<int> clause;
	unsigned int i;
	int isat;

	memset(blank, ' ', MAX_CLAUSE_LENGTH);
	fs.seekg(0,ios_base::end);
	end_of_file = fs.tellg();

	fs.seekg(0, ios_base::beg);
	cur_pos = fs.tellg();
	bzero(buf, MAX_CLAUSE_LENGTH);
	fs.getline(buf, MAX_CLAUSE_LENGTH);
	next_pos = fs.tellg();
	while ( !fs.eof() && fs.good() && !AllBlank(buf) )
	{			
		fs.seekg(cur_pos, ios_base::beg);
		fs.write(blank, strlen(buf));
		clause.clear();
		GetClause(clause, buf);
		fs.seekg(end_of_file, ios_base::beg);
		i = 0;
		while (i < clause.size() - 1) { // the last one is 0
			fs << -clause[i] << " 0" << endl;
			i++;
		}
		fs.close();
		isat = sat();
		fs.open(TmpFileName.c_str(), ios_base::in | ios_base::out);
		if (isat) {
			// recover the tested clause
			fs.seekg(cur_pos, ios_base::beg);
			fs.write(buf, strlen(buf));
		} else
			min_clauses ++;
		// recover the original end position of the file 
		fs.seekg(end_of_file, ios_base::beg);
		i = 0;
		while (i < clause.size() -1 ) {
			fs << "          " << endl;
			i++;
		}
		fs.seekg(next_pos, ios_base::beg);
		cur_pos = next_pos;
		bzero(buf, MAX_CLAUSE_LENGTH);
		fs.getline(buf, MAX_CLAUSE_LENGTH);
		next_pos = fs.tellg();
	}
	return;
}
/*
void CDimacs::prime()
{
	list<vector<int>*> theory;
	string str;
	char buf[MAX_CLAUSE_LENGTH];
	vector<int> *cl, tcl;
	int *pval;
	ifstream fs(TmpFileName.c_str());

	while (!fs.eof())
	{
		fs.getline(buf, MAX_CLAUSE_LENGTH);
		tcl.clear();
		GetClause(tcl, buf);
		cl = new vector<int>(tcl.size());
		*cl = tcl;
		theory.push_back(cl);
	}
	fs.close();

	list<vector<int>*>::iterator il, nl, tl;

	il = theory.begin();
	nl = il;
	nl++;
	while (il != theory.end())
	{
		while (nl != theory.end()) {
			if (_subsum(*nl, *il)) {
				delete *il;
				il = theory.erase(il);
				nl = il;
				nl++;
				il--;
				break;
			}
			nl++;
		}
		il++;
	}

	ofstream ofs(TmpFileName.c_str(), fstream::out | fstream::trunc);
	il = theory.begin();
	while (il != theory.end())
	{
		for (int i = 0; i< (*il)->size(); i++)  {
			//tcl = *(*il);
			ofs << (*il)->at(i) << " ";
		}
		ofs << endl;
		il++;
	}
	ofs.close();

	// free theory
	il = theory.begin();
	while (il != theory.end())
	{
		delete *il;
		il = theory.erase(il);
	}

	return;
}
*/
