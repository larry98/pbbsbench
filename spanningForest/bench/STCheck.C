// This code is part of the Problem Based Benchmark Suite (PBBS)
// Copyright (c) 2011 Guy Blelloch and the PBBS team
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights (to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <iostream>
#include <algorithm>
#include <cstring>
#include "parallel.h"
#include "graph.h"
#include "graphIO.h"
#include "parse_command_line.h"
#include "sequence.h"
#include "ST.h"
using namespace std;
using namespace benchIO;

using intT = int;

// The serial spanning tree used for checking against
//pair<int*,int> st(edgeArray<int> EA);

// This needs to be augmented with a proper check

int main(int argc, char* argv[]) {
  commandLine P(argc,argv,"<inFile> <outfile>");
  pair<char*,char*> fnames = P.IOFileNames();
  char* iFile = fnames.first;
  char* oFile = fnames.second;

  edgeArray<intT> In = readEdgeArrayFromFile<intT>(iFile);
  pbbs::sequence<intT> Out = readIntArrayFromFile<intT>(oFile);
  intT n = Out.size();

  //run serial ST
  pair<intT*,intT> serialST = st(In);
  pbbs::free_array(serialST.first);
  if (n != serialST.second){
    cout << "Wrong edge count: ST has " << serialST.second
	 << " edges but algorithm returned " << n << " edges\n";
    return (1);
  }
  
  //check if ST has cycles by running serial ST on it
  //and seeing if result changes
  pbbs::sequence<bool> flags(In.nonZeros, false);
  parallel_for(0, n, [&] (size_t i) {
      flags[Out[i]] = true;});
  pbbs::sequence<edge<intT>> E = pbbs::pack(In.E, flags);
  size_t m = E.size();
  
  edgeArray<intT> EA(std::move(E), In.numRows, In.numCols); 
  pair<intT*,intT> check = st(EA);
  pbbs::free_array(check.first);

  if (m != check.second){
    cout << "Result is not a spanning tree " << endl;
    return (1);
  }

  return 0;
}