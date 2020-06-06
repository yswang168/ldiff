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

#ifndef GLOBAL__H
#define GLOBAL__H

#include <string>
#include <iostream>
#include <vector>
#include <algorithm>

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h> 

#include "mtl/Vec.h"
#include "SolverTypes.h"


#define VERSION 0.5
#define MAXSIGLENGTH 256   // THE MAXIMAL NUMBER OF THE SIGNATURES
#define MAX_CLAUSE_LENGTH 256
#define MINI_SAT "SAT"
#define MINI_UNSAT "UNSAT"

typedef long long int64;


using namespace Minisat;
using namespace std;

typedef enum{TAUT, UNSAT, SAT, UNKNOWN} t_satstatus;

static inline double cpuTime(void) {
	struct rusage ru, ru1;
	getrusage(RUSAGE_SELF, &ru);
	getrusage(RUSAGE_CHILDREN, &ru1);
	return (double)ru.ru_utime.tv_sec + (double)ru1.ru_utime.tv_sec + \
		((double)ru.ru_utime.tv_usec  +  (double)ru1.ru_utime.tv_usec) / 1000000;
}

static inline int memReadStat(int field)
{
	char    name[256];
	pid_t pid = getpid();
	sprintf(name, "/proc/%d/statm", pid);
	FILE*   in = fopen(name, "rb");
	if (in == NULL) return 0;
	int     value;
	for (; field >= 0; field--)
		fscanf(in, "%d", &value);
	fclose(in);
	return value;
}

void GetClause(vector<int> &v, char* st);
void GetClause(vec<Lit> &v, const char* st);
void BuildClause(vector<int> &vi, bool pos, string& st);
bool _subsum(vector<int> *s, vector<int> *t);

static inline int64 memUsed() { return (int64)memReadStat(0) * (int64)getpagesize(); } 

#endif
