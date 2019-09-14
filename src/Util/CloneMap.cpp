#include "CloneMap.h"
#include "Referenced.h"
#include <unordered_map>

using namespace std;
using namespace cnoid;

namespace cnoid {

class CloneMap::Impl
{
public:
    unordered_map<ReferencedPtr, ReferencedPtr> orgToCloneMap;

    CloneFunction cloneFunction;

    struct ReplaceFunctionInfo
    {
        ReferencedPtr object;
        function<void(Referenced* clone)> replaceFunction;
        ReplaceFunctionInfo(Referenced* object, function<void(Referenced* clone)> replaceFunction)
            : object(object), replaceFunction(replaceFunction)
        { }
    };

    vector<ReplaceFunctionInfo> replaceFunctions;
    
    Impl(CloneFunction& cloneFunction);
    Impl(const Impl& org);
};

}


CloneMap::CloneMap(CloneFunction cloneFunction)
{
    impl = new Impl(cloneFunction);
}


CloneMap::Impl::Impl(CloneFunction& cloneFunction)
    : cloneFunction(cloneFunction)
{

}


CloneMap::CloneMap(const CloneMap& org)
{
    impl = new Impl(*org.impl);
}


CloneMap::Impl::Impl(const Impl& org)
    : orgToCloneMap(org.orgToCloneMap),
      cloneFunction(org.cloneFunction)
{

}


CloneMap::~CloneMap()
{
    delete impl;
}


void CloneMap::clear()
{
    impl->orgToCloneMap.clear();
    impl->replaceFunctions.clear();
}


Referenced* CloneMap::findClone(const Referenced* org)
{
    auto iter = impl->orgToCloneMap.find(const_cast<Referenced*>(org));
    if(iter != impl->orgToCloneMap.end()){
        return iter->second;
    }
    return nullptr;
}


Referenced* CloneMap::findOrCreateClone(const Referenced* org)
{
    auto clone = findClone(org);
    if(!clone){
        clone = impl->cloneFunction(org);
        impl->orgToCloneMap[const_cast<Referenced*>(org)] = clone;
    }
    return clone;
}


Referenced* CloneMap::findCloneOrReplaceLater
(const Referenced* org, std::function<void(Referenced* clone)> replaceFunction)
{
    auto clone = findClone(org);
    if(!clone){
        impl->replaceFunctions.push_back(
            Impl::ReplaceFunctionInfo(const_cast<Referenced*>(org), replaceFunction));
    }
    return clone;
}


void CloneMap::replacePendingObjects()
{
    for(auto& info : impl->replaceFunctions){
        auto clone = findClone(info.object);
        if(clone){
            info.replaceFunction(clone);
        }
    }
    impl->replaceFunctions.clear();
}


void CloneMap::setOriginalAsClone(const Referenced* org)
{
    impl->orgToCloneMap[const_cast<Referenced*>(org)] = const_cast<Referenced*>(org);
}