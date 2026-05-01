#pragma once

#include <memory>
#include <string>
#include <vector>
#include "common.h"
#include "interoptypes.h"

namespace usd
{
class ProxyInternal;

class StageProxy
{
    std::unique_ptr<ProxyInternal> m_internal;
    
public:
    StageProxy(std::unique_ptr<ProxyInternal> proxyInternal);
    ~StageProxy();

    static StageProxy *create(const std::string& path, ObjectsChangedFunc objectsChangedCallback);
    static StageProxy *open(const std::string& path, ObjectsChangedFunc objectsChangedCallback);
    
    void flushChanged() const;
    
    void save() const;
    void flatten(std::vector<Prim> &flatList, bool useDefaultPrim) const;
    void addMesh(const std::string &assetId, const std::string &assetPath, const std::string &primPath) const;
    void bakeMesh(const std::string &assetId, const std::string &assetPath, const std::string &primPath) const;
    void createBrush(const std::string &meshPath) const;
    void placeBrush(const std::string &brushPath) const;
};
}
