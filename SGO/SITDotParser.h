//
//  SITDotParser.h
//  Project
//
//  Created by ycc on 13-9-10.
//
//

#ifndef __Project__SITDotParser__
#define __Project__SITDotParser__

//Do not combine with SGO yet!!!!

//SITDotParser means StreamIt Dot graph Parser
#include <iostream>
#include "DotParser.h"

enum SITNodeType { FILTER, ROUNDROBIN, SPLITER, UNKNOW };
class SITNode
{
private:
    void ExtractLabel();
    void GetType();
public:
    SITNodeType Type;
    DotNode &n;
    string PushSize;
    string PopSize;
    string PeekSize;
    string Name;
    SITNode(DotNode& _n);
};

class SITGraph
{
public:
    map<DotNode, SITNode*> Dot2SIT;
    SITGraph(DotGraph g);
};


#endif /* defined(__Project__SITDotParser__) */
