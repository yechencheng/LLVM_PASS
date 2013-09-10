//
//  SITDotParser.cpp
//  Project
//
//  Created by ycc on 13-9-10.
//
//

#include "SITDotParser.h"
#include "SGOUtil.h"
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string_regex.hpp>
#include <boost/foreach.hpp>

using namespace boost;
using namespace std;



SITNode::SITNode(DotNode& _n) : n(_n)
{
    ExtractLabel();
}

void SITNode::GetType()
{
    Type = UNKNOW;
}

void SITNode::ExtractLabel()
{
    //Boost regex will crash if regex("\\") are called. If you know the reason, please contact me at
    //yechencheng@gmail.com
    string target = n.label;
    replace_all(target, "\\n", "##");
    vector<string> SplitVector;
    split_regex(SplitVector, target, regex("##"));
    //SplitVector = Name, push, pop, peek
    if(SplitVector.size() == 1)
    {
        GetType();
        return;
    }
    
    Type = FILTER;
    Name = SplitVector[0];
    PushSize = SplitVector[1].substr(SplitVector[1].find("=") + 1);
    PopSize = SplitVector[2].substr(SplitVector[2].find("=") + 1);
    PeekSize = SplitVector[3].substr(SplitVector[3].find("=") + 1);
}
