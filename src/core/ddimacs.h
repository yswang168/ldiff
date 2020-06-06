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

#ifndef _DIMACS__H
#define _DIMACS__H

#include <stdio.h>
#include <vector>
#include <fstream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <zlib.h>

#include "global.h"
#include "Dimacs.h" 
#include "Solver.h"
#include "mtl/Vec.h"
#include "SolverTypes.h"

using namespace Minisat;
using namespace std;
    
extern bool bEmbed;
extern bool bMIN;
extern bool bmin;
extern bool bma;
extern bool bout;

class CDimacs{
public:
	long EndPosition;
	string TmpFileName;
	string TmpSatResult;
	char SatRun[100];
	bool bsave;
	bool bHorn;
	bool b2CNF;
	int initialUNSAT;  // -1: unkown, 0: tautology, 1: falsehood	
	int  diff_min_size;  // the size of minimal difference 
	vector< vector<int> > clausal_diff_clauses; // to keep the clausal difference
	vector< vector<int> > min_diff_clauses; // to keep the minimize difference without the last loop checking
	vector<int> min_diff_removed; // to keep the removed clauses from the minimal diffeence without the last loop checking
	vector<int> num_Kclauses; // num_Kclauses[i] denotes the number of clauses of length i+1;
	vector< vector<int> > diff_unsat; // the clauses for difference which is already unsatisfied.
	vec<Lit> dummy;
public:
	/////////////// the Minisat solver
	// It is a bit a surprising that the member "solver" CANNOT be declared as an object
	// otherwise the call of solver.solve() will take quite a long time
	// Solver solver;  
	Solver *psolver; 

	void minimize(Solver *S); // minimal clausal difference
	//void prime(); // remove subsumed clauses in the files 

	void add_min_diff_clause(vector<int>&v)	{
		min_diff_clauses.push_back(v);
	}
	void MINimize(); // check the MIN difference, i.e., the minimal difference with the last loop checking
	void add_min_clauses(Solver* ps, unsigned int ex) // add the min_diff_clauses into a solver s, exclude the ex-th clauses and the removed
		// clauses whose positions are in min_diff_removed
	{
		vec<Lit> lits;
		vector<int>::iterator first, last;
		first = min_diff_removed.begin();
		last = min_diff_removed.end();

		
		for (unsigned int i = 0; i < min_diff_clauses.size(); i++){			
			// add new var
			for (unsigned int j = 0; j < min_diff_clauses[i].size(); j++) {
				Var var = abs(min_diff_clauses[i][j]) - 1;
				while (var >= ps->nVars()) ps->newVar();
			}
			
			if (i == ex) continue;
			if (find(first,last, i) != last) continue;			
			lits.clear();
			toMinisatClause(min_diff_clauses[i], lits);
			ps->addClause_(lits);
		}
		return;
	}

	void AddClauseVar(vector<int> &cl) {// add the variables in the clause cl into the solver "psolver"
		Var var;
		for (unsigned int i = 0; i < cl.size(); i++){
			var = abs(cl[i]) - 1;
			while (var >= psolver->nVars()) psolver->newVar();
		}
		return;	
	}

	// Add all of the variables in the current min_diff_clauses into the solver "psolver"
	void AddVars(){  
		for (unsigned int i = 0; i < min_diff_clauses.size(); i++)
			AddClauseVar(min_diff_clauses[i]);		
		return;
	}

	// transform the clauses in c to the minisat internal clause form lits
	void toMinisatClause(vector<int> &c, vec<Lit> &lits) 
	{
		unsigned int i;
		int var;
		lits.clear();
		for (i = 0; i < c.size(); i++){
			var = abs(c[i]) - 1;
			while (var >= psolver->nVars()) psolver->newVar();
			lits.push((c[i] > 0) ? mkLit(var) : ~mkLit(var));
		}
		return;
	}

	// transform the clause in Minisat to explicit presentation
	void fromMinisatClause(vec<Lit> &lits, vector<int> &c) 
	{
		c.clear();
		for (int i = 0; i < lits.size(); i++)	{
			c.push_back(sign(lits[i]) ? -(var(lits[i]) + 1) : var(lits[i]) + 1);
		}
		return;
	}
	

	void fromMinisatClause(Clause &lits, vector<int> &c) 
	{
		c.clear();
		for (int i = 0; i < lits.size(); i++)	{
			c.push_back(sign(lits[i]) ? -(var(lits[i]) + 1): var(lits[i]) + 1);
		}
		return;
	}

	int can_derive(vector<int> &tv);

	void PrintClause(vector<int> clause){
		sort(clause.begin(), clause.end());
		for (unsigned int k = 0; k < clause.size(); k++)
			cout << clause[k] << " ";
		cout << "0\n";
	}

	void neg_clause(vector<int>&t){
		for (unsigned int i = 0; i < t.size(); i++)
			t[i] = t[i] * -1;
		return;
	}
	///////////////////////////////////////////////////////////////////
	long num_clauses, min_clauses; // the latter is for the number of removed clauses for minimizing
	bool CheckHorn();
	bool Check2CNF();
	bool IsHorn() { return bHorn; }
	bool Is2CNF() { return b2CNF; }

	//    bool subsum(vector<int> *s, vector<int> *t);
	// check if the clause *s subsum the clause *t;

	bool subsum(vector<int> &v);
	// check if the clause [positive]v is subsumed in the minisat solver "psolver"
	bool subsum_(vector<int> &v);
	// check if the clause [positive]v is subsumed in the clauses clausal_diff_clauses

	void minimize(); // minimal clausal difference
	//void prime(); // remove subsumed clauses in the files TmpFileName
	
	int can_derive(vector<int> &vi, bool positive);
	// return if the clauses can derive the POSITIVE/NEGATIVE clause 
	// -1: unkown, 0: false, 1: true

	int can_derive(vector<int> &pos, vector<int> &neg);
	// return if the clauses can derive the POSITIVE + NEGATIVE clause 
	// -1: unkown, 0: false, 1: true


	CDimacs() { 
		bHorn = false;
		b2CNF = false;
		num_clauses = 0;
		min_clauses = 0;
		diff_min_size = 0; // for minisat solver approach of minimal difference
		initialUNSAT = -1; //unknown
		bsave = false;		
		min_diff_removed.clear();
		min_diff_clauses.clear();
		diff_unsat.clear();
		clausal_diff_clauses.clear();
		num_Kclauses.clear();
	}

	int initial_unsat() { return initialUNSAT; }

	int get_num_clause() {
		unsigned int num;
		if (!bEmbed || bout) return num_clauses;  // with minisat solver as an external call or for clausal difference
    	if (bMIN)	return (num = min_diff_clauses.size() - min_diff_removed.size());
		else if (bmin)   return (num = min_diff_clauses.size());
		else return clausal_diff_clauses.size(); // for clausal difference and prime difference
/*
		if (initialUNSAT != -1) {// satisfiable
			if (bMIN)	return (num = min_diff_clauses.size() - min_diff_removed.size());
			if (bmin)   return (num = psolver->clauses.size() + psolver->trail.size());
		}else{ // unsatisfiable
			if (bMIN)	return 2; // just a pair of complementary literals
			if (bmin)   return (num = diff_unsat.size() + psolver->clauses.size() + psolver->trail.size());
		}	   
*/			
	}

	void init(char *);	  // set the CNFs in dimacs into the temporary file and set the EndPosition
	~CDimacs();       // remove the temporary file

	void setSave(char *file){   // set the file to be save, not removed finally 
		int fd;
		remove(TmpFileName.c_str());
		fd = open(file, O_WRONLY | O_CREAT, 0600);
		ftruncate(fd, 0);
		close(fd);
		TmpFileName = file;
		bsave = true;
		sprintf(SatRun, "minisat -verb=0 %s %s 2>/dev/null 1>/dev/null", TmpFileName.c_str(), TmpSatResult.c_str());
	}

	void read(char *file); // read the CNFs in dimacs into the temporary file 
	
	void Save(){
		FILE *pfile = fopen(TmpFileName.c_str(), "w");
		assert(pfile != NULL);
		this->write(pfile);
		fclose(pfile);
		return;
	}
	
	void print(vector < vector<int> > cls, FILE *pfile=stdout)
	{
		for (unsigned int i = 0; i < cls.size(); i++){
			if (cls[i].size() == 0) continue;
			for (unsigned int j = 0; j < cls[i].size(); j++)
				fprintf(pfile, "%d ", cls[i][j]);
			fprintf(pfile, "0\n");
		}
	}

	void write(FILE *pfile = stdout) // print the clauses in the temporary file  
	{
		if (!bmin) {// for clausal or prime difference 
			print(clausal_diff_clauses);
			if (!psolver->ok) print(diff_unsat,pfile);
			return;
		}

		if (bEmbed){
			if (bMIN) // just print the clauses in min_diff_clauses excluding the positions of in min_diff_remvoed
			{
				vector<int>::iterator first, last;
				first = min_diff_removed.begin();
				last = min_diff_removed.end();
				for (unsigned int i = 0; i < min_diff_clauses.size(); i++){
					if (find(first, last, i) != last) continue;
					vector<int> &v = min_diff_clauses[i];
					for (unsigned int j = 0; j < v.size(); j++)
						fprintf(pfile, "%d ", v[j]);
					fprintf(pfile, "0\n");
				}
				print(diff_unsat, pfile);
				//return;
			}else{// for minimal difference 
				//int i, j;
				//vector<int> tc;
				// the single literal clauses
				/*for (i = 0; i < psolver->trail.size(); i++){
					Lit tlit = psolver->trail[i];
					fprintf(pfile, "%s%d 0\n", sign(tlit) ? "-" : "", var(tlit) + 1);
				}*/

                print(min_diff_clauses,pfile);
				//start_size = 0; // diff_min_size > 0 ? diff_min_size : 0;
                /*
				for (i = 0; i < psolver->clauses.size(); i++){
					Clause &c = psolver->ca[psolver->clauses[i]];
					tc.clear();
					for (j = 0; j < c.size(); j++) 
						tc.push_back(sign(c[j]) ? -(var(c[j]) + 1) : var(c[j]) + 1);
					sort(tc.begin(), tc.end());
					for (unsigned int k = 0; k < tc.size(); k++)
						fprintf(pfile, "%d ", tc[k]);
					fprintf(pfile, "0\n");
				}
				if (!psolver->ok) print(diff_unsat,pfile);
                */
				return;
			}
		}

		ifstream fs(TmpFileName.c_str());
		string str;
		char buf[MAX_CLAUSE_LENGTH];
		char *p;
		while ( !fs.eof() && fs.good() )
		{
			bzero(buf, MAX_CLAUSE_LENGTH);
			fs.getline(buf, MAX_CLAUSE_LENGTH);
			p = buf;
			while (*p ==' ') p++;
			if (strlen(p) == 0) continue;
			cout << buf << endl;
		}

		fs.close();
		return;
	}
	void add_clause(vector<int> &tv, bool positive); // add one clause  to minisat solver
	void add_clause(vector<int> &pos, vector<int> &neg); // add one clause  to minisat solver

	void add_clause(string &cl); // add one clause  
	//void add_neg_literals(vector<int> plit, vector<int> nlit); // add the negation of literals in plit and nlit
	//void add_neg_literals(vector<int> lit, bool positive); // add the negation of literals in plit 
	int  sat(); // check whether the CNFs is satisfiable
				// -1: error;  0: unsat;	1: sat
};


#endif
