#include "NodeCollection.h"
#include "Logging/Logger.h"

NodeCollection::NodeCollection(ThingSettings* settings):
 _settings(settings),
 _logger(LoggerInstance),
 _nullNode(NodeType::BasicValue, ValueType::Null, "null",0 , ThingifyUnit::None)
{
    _lastNodeId = 0;
    _nodes.reserve(32);
}

Node* NodeCollection::AddNode(const char* nodeName, NodeType type, ValueType valueType, ThingifyUnit unit)
{
	if (NodeExists(nodeName))
	{
		_logger.err(L("Node %s already exists, returning null node"), nodeName);
		return &_nullNode;
	}
	const auto node = new Node(type, valueType, nodeName, _lastNodeId, unit);
	_lastNodeId++;
	_nodes.push_back(node);
	return node;
}

Node* NodeCollection::AddBoolean(const char* nodeName, ThingifyUnit unit)
{
	return AddNode(nodeName, NodeType::BasicValue, ValueType::Bool, unit);
}
Node* NodeCollection::AddString(const char* nodeName, const char *value, ThingifyUnit unit)
{
	auto node = AddNode(nodeName, NodeType::BasicValue, ValueType::String, unit);
	node->SetValue(NodeValue::String(value));
	return node;
}
Node* NodeCollection::AddString(const char * nodeName, ThingifyUnit unit)
{
	return AddString(nodeName, "", unit);
}
Node* NodeCollection::AddInt(const char* nodeName, ThingifyUnit unit)
{
	return AddNode(nodeName, NodeType::BasicValue, ValueType::Int, unit);
}
Node* NodeCollection::AddInt(const char* nodeName, int value, ThingifyUnit unit)
{	
	auto node = AddNode(nodeName, NodeType::BasicValue, ValueType::Int, unit);
	node->SetValue(NodeValue::Int(value));
	return node;
}
Node* NodeCollection::AddFloat(const char* nodeName, float value, ThingifyUnit unit)
{
	auto node = AddNode(nodeName, NodeType::BasicValue, ValueType::Float, unit);
	node->SetValue(NodeValue::Float(value));
	return node;
}
Node* NodeCollection::AddFloat(const char* nodeName, ThingifyUnit unit)
{
	return AddFloat(nodeName, 0.0f, unit);
}
Node* NodeCollection::AddColor(const char * nodeName)
{
	auto node = AddNode(nodeName, NodeType::BasicValue, ValueType::Color, ThingifyUnit::None);
	node->SetValue(NodeValue::NullColor());
	return node;
}
Node * NodeCollection::AddTimeSpan(const char *name)
{
	auto node = AddNode(name, NodeType::BasicValue, ValueType::TimeSpan, ThingifyUnit::None);
	node->SetValue(NodeValue::NullTimeSpan());
	return node;
}
Node* NodeCollection::AddRange(const char * nodeName, int min, int max, int step, ThingifyUnit unit)
{
	auto node = AddNode(nodeName, NodeType::Range, ValueType::Int, unit);
	node->SetValue(NodeValue::Int(min));
	node->SetRangeAttributes(min, max, step);
	return node;
}

Node* NodeCollection::AddFunction(const char * nodeName, FunctionExecutionCallback callback, void* context)
{
	auto node = AddNode(nodeName, NodeType::Function, ValueType::Bool, ThingifyUnit::None);
	node->_context = context;
	node->ExecutionCallback = callback;
	return node;
}

Node * NodeCollection::operator[](const char * nodeName)
{
	return FindNode(nodeName);
}

Node* NodeCollection::FindNode(const char* nodeName)
{
	for (auto i = 0; i < _nodes.size(); i++)
	{
		auto node = _nodes[i];
		if (strcmp(node->name(), nodeName) == 0)
		{
			return node;
		}
	}
	_logger.err(L("Failed to find %s node, returning null node"), nodeName);
	return &_nullNode;
}
bool NodeCollection::NodeExists(const char* nodeName)
{
	for (auto i = 0; i < _nodes.size(); i++)
	{
		auto node = _nodes[i];
		if (strcmp(node->name(), nodeName) == 0)
		{
			return true;
		}
	}
	return false;
}
bool NodeCollection::RemoveNode(const char *nodeName)
{
	for (auto i = 0; i < _nodes.size(); i++)
	{
		auto node = _nodes[i];
		if (strcmp(node->name(), nodeName) == 0)
		{
			_nodes.erase(_nodes.begin() + i);
			NodeId nodeId;
			nodeId.NodeName = node->name();
			nodeId.DeviceId = _settings->Token;
			_removedNodes.push_back(nodeId);
			delete node;
			return true;
		}
	}
	return false;
}
std::vector<Node*> NodeCollection::GetWorkingNodes()
{
	std::vector<Node*> workingNodes;
	workingNodes.reserve(_nodes.size());
	for (auto & node : _nodes)
	{
		if (!node->IsHidden)
		{
			workingNodes.push_back(node);
		}
	}
	return workingNodes;
}

std::vector<Node*> NodeCollection::GetUpdatedNodes()
{
	std::vector<Node*> updatedNodes;
	updatedNodes.reserve(_nodes.size());
	for (auto & node : _nodes)
	{
		if (node->_wasUpdated && !node->IsHidden)
		{
			updatedNodes.push_back(node);
		}
	}
	return updatedNodes;
}

void NodeCollection::LogUpdatedNodes(std::vector<Node*> updatedNodes) const
{
	FixedString64 updateNodesString;

	for (Node* updated_node : updatedNodes)
	{
		updateNodesString.appendFormat("'%s', ", updated_node->name());		
	}

	_logger.debug(L("nodes: [%s] changed value, send updates"), updateNodesString.c_str());
}

void NodeCollection::LogNodes()
{
	_logger.debug(LogComponent::Node, L("Node list: "));
	for(auto& node: _nodes)
	{
		_logger.debug(LogComponent::Node, L(" %s read_only: %d"), node->name(), node->isReadOnly());
	}
}