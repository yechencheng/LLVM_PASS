//
//  DotParser.cpp
//  Project
//
//  Created by ycc on 13-9-9.
//
//

#include "DotParser.h"
#include "Util.h"

using namespace std;



DotEdge::DotEdge(Agedge_t *_e) : e(_e)
{}

DotNode DotEdge::GetHead()
{
    return DotNode(aghead(e));
}
DotNode DotEdge::GetTail()
{
    return DotNode(agtail(e));
}


DotNode::DotNode(Agnode_t *_n) : n(_n)
{
    label = agget(n, "label");
}

bool DotNode::operator<(const DotNode &x) const
{
    return n < x.n;
}

vector<DotEdge>& DotGraph::GetEdgeVec(DotNode n, bool IsIn)
{
    vector<DotEdge>* &store = IsIn ? InputEdge[n] : OutputEdge[n];
    if(ExistOrCreate(store)) return *store;
    
    Agedge_t* (*IteratorHeader)(Agraph_t*, Agnode_t*) = IsIn ? agfstin : agfstout;
    Agedge_t* (*IteratorFunc)(Agraph_t*, Agedge_t*) = IsIn ? agnxtin : agnxtout;
    
    for(Agedge_t *e = IteratorHeader(g, n.n); e; e = IteratorFunc(g, e))
        (*store).push_back(DotEdge(e));
    return *store;
}

vector<DotEdge>& DotGraph::GetInEdgeVec(DotNode n)
{
    return GetEdgeVec(n, true);
}

vector<DotEdge>& DotGraph::GetOutEdgeVec(DotNode n)
{
    return GetEdgeVec(n, false);
}

DotGraph::DotGraph(Agraph_t *_g) : g(_g)
{
    for(Agnode_t *n = agfstnode(g); n; n = agnxtnode(g, n))
    {
        DotNode x(n);
        Node.push_back(x);
        InputEdge[x] = NULL;
        OutputEdge[x] = NULL;
    }
}

DotGraph::DotGraph(string DotFile)
{
    FILE *fin;
    fin = fopen(DotFile.c_str(), "r");
    new(this) DotGraph(agread(fin, NULL));
}

/*
 Example Code
int main(int argc, const char * argv[])
{
    DotGraph x("/Users/apple/Project/Temp/stream-graph.dot");
    for(vector<DotNode>::iterator itr = x.Node.begin(); itr != x.Node.end(); itr++)
    {
        cout << itr->label << endl;
        for(vector<DotEdge>::iterator eitr = x.GetOutEdgeVec(*itr).begin();
            eitr != x.GetOutEdgeVec(*itr).end(); eitr++)
            cout << "\t" << eitr->GetHead().label << endl;
    }
    return 0;
}
*/