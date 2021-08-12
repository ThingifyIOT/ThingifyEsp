#ifndef _NODE_COLLECTION_H
#define _NODE_COLLECTION_H

#include <vector>
#include "Node.h"
#include "Settings/ThingSettings.h"
#include "Node/NodeId.h"

class NodeCollection
{
private:
    uint16_t _lastNodeId;
    std::vector<Node*> _nodes;
    Logger& _logger;
    ThingSettings* _settings;
protected:
    std::vector<Node*> GetWorkingNodes();
    std::vector<Node*> GetUpdatedNodes();
    void LogUpdatedNodes(std::vector<Node*> updatedNodes) const;
    void LogNodes();
    Node _nullNode;
    std::vector<NodeId> _removedNodes;
public:
    NodeCollection(ThingSettings* settings);

    Node* AddNode(const char* nodeName, NodeType type, ValueType valueType, ThingifyUnit unit = ThingifyUnit::None);
    Node* AddBoolean(const char* nodeName, ThingifyUnit unit = ThingifyUnit::None);
    Node* AddString(const char* nodeName, const char *value, ThingifyUnit unit = ThingifyUnit::None);
    Node* AddString(const char * nodeName, ThingifyUnit unit = ThingifyUnit::None);
    Node* AddInt(const char* nodeName, ThingifyUnit unit = ThingifyUnit::None);
    Node* AddInt(const char* nodeName, int value, ThingifyUnit unit = ThingifyUnit::None);
    Node* AddFloat(const char* nodeName, float value, ThingifyUnit unit = ThingifyUnit::None);
    Node* AddFloat(const char* nodeName, ThingifyUnit unit = ThingifyUnit::None);
    Node* AddColor(const char * nodeName);
    Node * AddTimeSpan(const char *name);
    Node* AddRange(const char * nodeName, int min, int max, int step, ThingifyUnit unit = ThingifyUnit::None);
    Node* AddFunction(const char * nodeName, FunctionExecutionCallback callback);
    Node* AddFunction(const char * nodeName, FunctionExecutionCallback callback, void* context);

    Node * operator[](const char * nodeName);
    Node* FindNode(const char* nodeName);
    bool NodeExists(const char* nodeName);
    bool RemoveNode(const char *nodeName);  
};

#endif