// Copyright (C) 2014-2016 Yisong Wang
// csc.yswang@gzu.edu.cn

//  ---------------------------------------------------- 
//  MINISAT Solver cannot be declared as a global object
//  ----------------------------------------------------

/*
 Updated 2016/10/09
 Dealing with the case the KB2 entails KB1, there has no difference between KB1 and KB2
*/
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

#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>

#include <fpu_control.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <zlib.h>

#include "global.h"
//#include "ddimacs.h"
//#include "Dimacs.h"
#include "solve.h"
//#include "combo.h" 

using namespace std;

CDimacs kb1, kb2, diff;
vector<int> sig;
CSolve diff_solve;


string OutPutFile;
string KB1Name;
string KB2Name;
string SigName;

bool bma = false;
bool bcd = true;
bool bmin = false;
bool bMIN = false;
bool bEmbed = false;
bool bout = false;
bool bDetailed = false; // if it is truee, the clausal difference is output as well
bool bputfile = false; // if it is truee, the clausal difference is saved to a file 

void usage(char *argv[]);
void ProcessArgument(int argc, char *argv[], Solver *s1, Solver *s2);
void statistics();



int main(int argc, char* argv[])
{
	switch (argc)
	{
		case 2: if (strcmp(argv[1], "-v") == 0) { cout << argv[0] << " " << VERSION << endl; exit(0); }
				if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) { usage(argv); exit(0); }
				break;
		case 1:usage(argv); return 0;
		case 3:usage(argv); return 0;
		//case 4: usage(argv); return 0; 
	} 
	/*
	if (argc < 4) {
		usage(argv); return 0;
	}*/
	 
#if defined(__linux__)
	fpu_control_t oldcw, newcw;
	_FPU_GETCW(oldcw); newcw = (oldcw & ~_FPU_EXTENDED) | _FPU_DOUBLE; _FPU_SETCW(newcw);
	//printf("WARNING: for repeatability, setting FPU to use double precision\n");
#endif
	// It is a bit of surprising that the following declaration cannot be moved out of the main function as global, 
	// otherwise the call of solve() will take quite a long time
	// 
	Solver kb1_solver, kb2_solver, diff_solver;

	kb1.psolver = &kb1_solver;
	kb2.psolver = &kb2_solver;	
	kb1_solver.verbosity = 0;
	kb2_solver.verbosity = 0;
	diff_solver.verbosity = 0;

	ProcessArgument(argc, argv, &kb1_solver, &kb2_solver);

	if (kb1.Is2CNF() && kb2.Is2CNF())
		cout << "KB1: " << KB1Name.c_str() << " and " << "KB2: " << KB2Name.c_str() << " are in 2CNF." << endl;
	if (kb1.IsHorn() && kb2.IsHorn())
		cout << "KB1: " << KB1Name.c_str() << " and " << "KB2: " << KB2Name.c_str() << " are in Horn." << endl;
	
	if (kb2.initialUNSAT == 1) { // if kb2 is unsat, then there is no clausal difference
		statistics();
		cout << "KB2 " << KB2Name.c_str() <<" is unsat. The number of difference: 0" << endl;
		cout << "The clauses of individual length are: ";
		for (unsigned int i = 0; i <= sig.size(); i++)	{ // print the clauses of different length
		cout << i  << ":0 ";
		}
		cout << endl;
		return 0;
	}	
	
	if (kb1.initialUNSAT == 1  && (bma || bmin)){ // The difference is the empty clause, which means falsehood
		if (bputfile) { ofstream of(OutPutFile.c_str()); of << "0 " << endl; /* the empty clause*/ of.close(); }
		statistics();		 
		if (bDetailed) cout << "0 " << endl;
		cout << "KB1 " << KB1Name.c_str() << " is unsat. " << "The number of difference: 1" << endl;
		cout << "The clauses of individual length are: 0:1 ";
		for (unsigned int i = 1; i <= sig.size(); i++)	{ // print the clauses of different length
		cout << i  << ":0 ";
		}
		cout << endl;
		return 0;
	}
	/* This will be handled latter

	if (kb2.entails(&kb1)) { // KB2 entails KB1, there is no difference
		if (bputfile) { ofstream of(OutPutFile.c_str()); of << "0 " << endl; of.close(); }
		statistics();
		if (bDetailed) cout << "0 " << endl;
		cout << "KB2 " << KB2Name.c_str() << " entails KB1: " << KB2Name.c_str() << ". The number of difference: 0" << endl;
		cout << "The clauses of individual length are: 0:0 ";		 
		cout << endl;
		return 0;
	}
	*/
	diff.psolver = &diff_solver;
		
	diff_solve.init(&kb1, &kb2, sig, &diff);

    diff_solve.emsolve();		
 
	if (bMIN)  // to compute the minimal difference result
	if (bEmbed) //diff.MINimize();
		diff.minimize(diff.psolver);
	else diff.minimize();

	if (bDetailed && !bout) diff.write();
	if (diff.bsave && bEmbed) diff.Save();

	statistics();
	cout << "The number of difference: ";
	if (!bMIN) cout << diff.get_num_clause();
	else
		if (bEmbed) cout << diff.get_num_clause() << " : (" << diff.min_diff_removed.size() << ")";				
		else  cout << diff.num_clauses - diff.min_clauses << " : (" << diff.min_clauses << ")";
	
	cout << endl << "The clauses of individual length are: ";
	for (unsigned int i = 0; i < diff.num_Kclauses.size(); i++)	{ // print the clauses of different length
		cout << i  << ":" << diff.num_Kclauses[i] << " ";
	}
	cout << endl;
	 
	return 0;
}

void usage(char *argv[])
{
	cerr << "Usage: " << argv[0] << "[options] <kb1> <kb2> <sig> [out-put]" << endl
		<< "options:" << endl
		<< "-v                Print the version number and exit." << endl
		<< "-h, --help        Print this message and exit." << endl
		<< "-d                Print the detailed information." << endl
		<< "-pd               Compute prime difference." << endl
		<< "-cd               Compute clausal difference (by default)." << endl
		<< "-min              Coumpute a minimal approximation dont caring about the alrealy existing clauses." << endl
		<< "-MIN              Compute a minimal approximation for the clausal difference." << endl
		<< "-embed            Using embeded minisat instead of external minisat."<<endl;
}
void ProcessArgument(int argc, char *argv[], Solver *s1, Solver *s2)
{
	int base = 1;
	int i;

	for (i = 1; i <= argc - 1; i++)	{
		if (strcmp(argv[i], "-v") == 0) { cout << argv[0] << " " << VERSION << endl; exit(0); }
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) { usage(argv); exit(0); }
		if (strcmp(argv[i], "-d") == 0) { base++; bDetailed = true; }
		if (strcmp(argv[i], "-pd") == 0) { base++; bma = true; } // prime difference
		if (strcmp(argv[i], "-cd") == 0) { base++; bcd = true; } // clausal difference
		if (strcmp(argv[i], "-min") == 0) { base++; bmin = true;  bma = false; } // minimal difference
		if (strcmp(argv[i], "-MIN") == 0) { base++; bMIN = true; bmin = true;  bma = false; }
		if (strcmp(argv[i], "-embed") == 0) { base++; bEmbed = true; }
	}
	bout = !(bma || bmin);
	// initialize the two knowledge bases	
	if (argc - base < 2) { usage(argv); return; }

	KB1Name = argv[base];
	kb1.TmpFileName = KB1Name.c_str();
	kb1.init(argv[base++]);
	KB2Name = argv[base];
	kb2.TmpFileName = KB2Name.c_str();
	kb2.init(argv[base++]);
	SigName = argv[base];
	if (bEmbed) {
		vec<Lit> dummy;
		lbool ret;
		gzFile in;
		in = gzopen(KB1Name.c_str(), "rb");
		parse_DIMACS(in, *s1);
		s1->simplify();
		gzclose(in);

		if (s1->nClauses() == 0) //tautology
			kb1.initialUNSAT = 0;
		else
		{
			ret = s1->solveLimited(dummy);		
			if (ret == l_False)	kb1.initialUNSAT = 1; //UNSAT
		}

		in = gzopen(KB2Name.c_str(), "rb");
		parse_DIMACS(in, *s2);
		s2->simplify();
		gzclose(in);

		if (s2->nClauses() == 0) //tautology
			kb2.initialUNSAT = 0;
		else
		{
			ret = s2->solveLimited(dummy);
			//if (ret == l_True)  kb2.initialUNSAT = 0; //SAT
			if (ret == l_False)	kb2.initialUNSAT = 1; //UNSAT
		}
	} 
	// get the signature
	ifstream infs(argv[base++]);
	assert(infs.good()); 
	while (infs >> i) sig.push_back(i);
	if (sig.size() > MAXSIGLENGTH) {
		cerr << "The maximal number of relevant signatures is: " << MAXSIGLENGTH << endl;
		exit(-1);
	}
	infs.close();

	sort(sig.begin(), sig.end()); // sort the signature

	if (base < argc) { // there is the out put file
		OutPutFile = argv[base]; 
		bputfile = true; 
		diff.setSave(argv[base]);
	}else if (!bEmbed){    
        char buf[11] = ".tmpXXXXXX";
		int fd;
		fd = mkstemp(buf);
		diff.TmpFileName = buf; //tmpnam(NULL);
		close(fd);
        strncpy(buf, ".res", 4);
		diff.TmpSatResult = buf; //tmpnam(NULL); 
		close(fd);
        sprintf(diff.SatRun, "minisat -verb=0 %s %s 2>/dev/null 1>/dev/null", diff.TmpFileName.c_str(), diff.TmpSatResult.c_str());
    }

	return;
}

void statistics() 
{
	double  cpu_time = cpuTime();
	long long  mem_used = memUsed();

	cout << "Memory used (MB) : " << mem_used / 1048576.0 << endl
		<<  "CPU time (second): " << cpu_time << endl;

	return;
}


bool _subsum(vector<int> *s, vector<int> *t)
// check if the vector s subsumes the vector t
// return true if yes, and false otherwise
{
	if (s->size() > t->size()) return false;
	vector<int>::iterator te = t->end(), tb = t->begin();

	unsigned int i = 0;
	while (i < s->size())
	{
		if (te == find(tb, te, s->at(i))) // not find 
			return false;
		i++;
	}
	return true;
}


void BuildClause(vector<int> &vi, bool pos, string& st) 
// build the clause in DIMACS from vector to string 
// the last one is the real last one literal, not 0
{
	unsigned int i;
	char buf[10];

	for (i = 0; i < vi.size(); i++){
		if (pos) sprintf(buf, "%d ", vi[i]);
		else
			sprintf(buf, "-%d ", vi[i]);
		st.append(buf);
	}
	return;
}


void GetClause(vector<int> &v, char* st)
// get the clause from string to vector
// the clause is in the form of DIMACS, the last one is 0
{
	char *p = st;
	int t;

	while (p != NULL)
	{
		sscanf(p, "%d ", &t);
		v.push_back(t);
		if (t == 0) return;
		while (*p != ' ') p++;
		p++;
	}
	return;
}

 

void GetClause(vec<Lit> &v, const char* st)
// get the clause from string to vector
// the clause is in the form of DIMACS, the last one is 0
{
	const char *p = st;
	int t;

	while (p != NULL)
	{
		sscanf(p, "%d ", &t);
		if (t>0)
			v.push(mkLit((t - 1)));
		else
			v.push(mkLit(~(abs(t) - 1))); 
		if (t == 0) return;
		while (*p != ' ') p++;
		p++;
	}
	return;
}
