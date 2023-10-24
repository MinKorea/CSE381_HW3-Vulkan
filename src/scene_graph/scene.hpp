#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "common/glm_common.hpp"
#include "scene_graph/node.hpp"

namespace W3D::sg
{

class Node;
class Component;
class SubMesh;

class Scene
{
  private:
	std::string name_;
	Node       *root_ = nullptr;

	std::vector<std::unique_ptr<Node>>                                           p_nodes_;
	std::unordered_map<std::type_index, std::vector<std::unique_ptr<Component>>> p_components_;

  public:
	Scene() = default;
	Scene(const std::string &name);

	void add_component(std::unique_ptr<Component> &&pComponent);
	template <typename T>
	T *find_component(const std::string &name)
	{
		T *result = nullptr;
		if (has_component<T>())
		{
			const std::vector<std::unique_ptr<Component>> &p_query_comp = get_components(typeid(T));
			for (auto &p_component : p_query_comp)
			{
				if (p_component->get_name() == name)
				{
					result = dynamic_cast<T *>(p_component.get());
					break;
				}
			}
		}
		return result;
	}

	void  add_node(std::unique_ptr<Node> &&pNode);
	void  add_child(Node &child);
	void  set_root_node(Node &node);
	void  set_nodes(std::vector<std::unique_ptr<Node>> &&nodes);
	Node *find_node(const std::string &name);
	void  add_component_to_node(std::unique_ptr<Component> &&pComponent, Node &node);

	template <typename T>
	void set_components(std::vector<std::unique_ptr<T>> p_ts)
	{
		std::vector<std::unique_ptr<Component>> p_components(p_ts.size());
		std::transform(p_ts.begin(), p_ts.end(), p_components.begin(), [](std::unique_ptr<T> &p_t) -> std::unique_ptr<Component> {
			return std::unique_ptr<Component>(std::move(p_t));
		});
		set_components(typeid(T), std::move(p_components));
	}
	void set_components(const std::type_index                   type,
	                    std::vector<std::unique_ptr<Component>> pComponents);

	template <typename T>
	std::vector<T *> get_components() const
	{
		std::vector<T *> result;
		if (has_component(typeid(T)))
		{
			auto &scene_components = get_components(typeid(T));
			result.resize(scene_components.size());
			std::transform(scene_components.begin(), scene_components.end(), result.begin(), [](const std::unique_ptr<Component> &component) -> T * {
				return dynamic_cast<T *>(component.get());
			});
		}
		return result;
	}

	const std::vector<std::unique_ptr<Component>> &get_components(
	    const std::type_index &type) const;

	template <typename T>
	bool has_component() const
	{
		return has_component(typeid(T));
	}
	bool has_component(const std::type_index &type) const;

	Node &get_root_node();
};

}        // namespace W3D::sg