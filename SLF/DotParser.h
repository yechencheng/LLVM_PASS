//
//  DotParser.h
//  Project
//
//  Created by ycc on 13-9-9.
//
//

#ifndef __Project__DotParser__
#define __Project__DotParser__

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <graphviz/cgraph.h>

using namespace std;

class DotNode;

class DotEdge
{
private:
    Agedge_t *e;
public:
    //Add Other attribute here, such as weight, label.
    DotEdge(Agedge_t *_e);
    DotNode GetHead();
    DotNode GetTail();
};

class DotNode
{
public:
    Agnode_t *n;
    string label;
    
    DotNode(Agnode_t *_n);
    bool operator<(const DotNode &x) const; //used for map
};

class DotGraph
{
private:
    Agraph_t *g;
    map<DotNode, vector<DotEdge>*> OutputEdge;
    map<DotNode, vector<DotEdge>*> InputEdge;
    vector<DotEdge>& GetEdgeVec(DotNode n, bool IsIn);
public:
    vector<DotNode> Node;
    DotGraph(Agraph_t *_g);
    DotGraph(string DotFile);
    vector<DotEdge>& GetInEdgeVec(DotNode node);    //get all input edge for node
    vector<DotEdge>& GetOutEdgeVec(DotNode node);   //get all output edge for node
};

#endif /* defined(__Project__DotParser__) */
