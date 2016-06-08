/*
* A simple and ugly node tree container, which supports one to many mapping(namely, one key can have many values).
* Note: 
* 1. The topology of node tree is like the tree data structure in computer science. Every node made up of the tree is a <key, values> pair.
* 2. Every iterator used as the argument of function should be valid, or the behavior is unspecified.
* 3. The keyComp is a class type supporting strict weak ordering, namely, should override operator () to support "<" operation.
* 4. The key path is made up of all the keys from the root node to the target node.
* 5. The key of every direct child node of a particular parent node should be unique, while the keys of non-sibling nodes need not be unique. 
*/
#pragma once

#include <vector>
#include <list>

template <typename key, typename data, typename keyComp = std::less<key> >
class NodeTree
{
public:
	typedef key key_type;
	typedef data data_type;
	typedef keyComp comp_type;
	typedef std::size_t size_type;
	typedef std::pair<const key_type, std::vector<data_type> > value_type;
	typedef value_type &reference;
	typedef const value_type &const_reference;

	// Traverse all the direct children of the given parent node of tree.
	// tree: a pointer to the node tree; parent: an iterator pointing to the parent node; child: a iterator pointing to the next child node in each loop.
	#define NT_FOR_EACH_CHILD(tree, parent, child)	\
		for (std::vector<iterator> children, std::vector<iterator>::size_t index = 0, (tree)->getChildren((parent), children); \
			(index < children.size()) && ((child) = children[index], true); ++index)

	class iterator: public std::iterator<std::forward_iterator_tag, value_type>
	{
		friend class NodeTree<key, data, keyComp>;
	public:
		iterator(): m_node(0) {}
		iterator(const iterator &itr): m_node(itr.m_node) {}
		iterator &operator=(const iterator &rhs) { m_node = rhs.m_node; return *this; }
		bool operator==(const iterator &rhs) { return m_node == rhs.m_node; }
		bool operator!=(const iterator &rhs) { return m_node != rhs.m_node; }
		iterator &operator++(void);
		iterator operator++(int);
		value_type &operator*(void) { return *(m_node->m_data); }
		value_type *operator->(void) { return m_node->m_data; }

	private:
		iterator(NodeTree *node): m_node(node) {}
		NodeTree *m_node;
	};
	class const_iterator: public std::iterator<std::input_iterator_tag, value_type>
	{
		friend class NodeTree<key, data, keyComp>;
	public:
		const_iterator(): m_node(0) {}
		const_iterator(const const_iterator &itr): m_node(itr.m_node) {}
		const_iterator &operator=(const const_iterator &rhs) { m_node = rhs.m_node; return *this; }
		bool operator==(const const_iterator &rhs) const { return m_node == rhs.m_node; }
		bool operator!=(const const_iterator &rhs) const { return m_node != rhs.m_node; }
		const_iterator &operator++(void);
		const_iterator operator++(int);
		const value_type &operator*(void) { return *(m_node->m_data); }
		const value_type *operator->(void) { return m_node->m_data; }

	private:
		const_iterator(const NodeTree *node): m_node(node) {}
		const NodeTree *m_node;
	};
	// An empty node tree.
	NodeTree(): m_data(0), m_parent(0), m_sibling(0) {}
	// A node tree rooted at the node with the given key and empty value.
	NodeTree(const key_type &key): m_parent(0), m_sibling(0) { std::vector<data_type> tmp; m_data = new value_type(key, tmp); }
	// A node tree rooted at the node with the given <key, value>
	NodeTree(const key_type &key, const data_type &data): m_parent(0), m_sibling(0) { std::vector<data_type> tmp; tmp.push_back(data); m_data = new value_type(key, tmp); }
	// A node tree rooted at the node with the given <key, values>
	NodeTree(const key_type &key, const std::vector<data_type> &data): m_data(new value_type(key, data)), m_parent(0), m_sibling(0) {}
	virtual ~NodeTree();
	iterator begin(void)
	{
		NodeTree *tmp = this;
		
		// Empty node tree, return this->end().
		if (0 == m_data)
			return iterator(0);
		while(0 != tmp->m_parent)
			tmp = tmp->m_parent;

		return iterator(tmp);
	}
	iterator end(void) { return iterator(0); }
	const_iterator begin(void) const
	{
		const NodeTree *tmp = this;
		
		// Empty node tree, return this->end().
		if (0 == m_data)
			return const_iterator(0);
		while(0 != tmp->m_parent)
			tmp = tmp->m_parent;

		return const_iterator(tmp);
	}
	const_iterator end(void) const { return const_iterator(0); }

#if 0
	// Get the first value of this node or the node pointed by path, both of the cases return the number of values the node has: [0-1].
	size_type get(data_type &data, const std::list<key_type> *path = 0) const;
	// Get all the values of this node or the node pointed by path, both of the cases return the number of values the node has:[0 or more].
	size_type gets(std::vector<data_type> &data, const std::list<key_type> *path = 0) const;
	// Add a new value to this node or node pointed by path. If the node doesn't exist, it will be created, including all the missing parent nodes.
	void put(const data_type &data, const std::list<key_type> *path = 0);
	// Add one or more values to this node or node pointed by path. If the node doesn't exist, it will be created, including all the missing parents node.
	void puts(const std::vector<data_type> &data, const std::list<key_type> *path = 0);
#endif
	// The number of direct children of the given node.
	size_type size(iterator itr) const { return itr.m_node->m_children.size(); }
	// Count the number of values this node has.
	size_type count(iterator itr) const { return itr.m_node->m_data->second.size(); }
	// Whether current node tree is empty.
	bool empty(void) const { return 0 == m_data; }
	// Get the key path of the given node.
	void getPath(iterator itr, std::list<key_type> &keyPath) const;
	// Get the root node, this->end() will be returned if the tree is empty. Now the root node is the begin() node actually.
	iterator root(void) { return begin(); }
	// Search the sub-tree rooted at the selected node(the whole tree will be searched if the sub-root node is the root node of the whole tree) to 
	// find the first node with the given key, this->end() will be returned if not found.
	iterator find(iterator root, const key_type &key);
	// Find the node with the given key path, this->end() will be returned if not found.
	iterator find(const std::list<key_type> &keyPath);
	// Insert a node with the given value as the direct child of the given parent node and reutrn an iterator to the new node.
	// If the node exists, replaces it. Otherwise, creates it.
	iterator insert(iterator parent, const key_type &key, const std::vector<data_type> &data);
	iterator insert(iterator parent, const key_type &key, const data_type &data) { std::vector<data_type> tmp; tmp.push_back(data); return insert(parent, key, tmp); }
	// Replace the values of the given node with new values and return the node with new values.
	iterator replace(iterator node, const std::vector<data_type> &data) { node->second = data; return node; }
	// If the node specified by the given path exists, add the new values; Otherwise, creates it, including all the missing parent nodes. Both cases
	// return a iterator pointing to the target node. This is the only way adding new nodes into a empty node tree. Specially, if the starting node of
	// the key path is not the root node of current tree, return end().
	iterator add(const std::list<key_type> &keyPath, const std::vector<data_type> &data);
	iterator add(const std::list<key_type> &keyPath, const data_type &data) { std::vector<data_type> tmp; tmp.push_back(data); return add(keyPath, tmp); }
	// Get all the direct children of the given parent node
	void getChildren(iterator parent, std::vector<iterator> &children) const
	{
		NodeTree *child = 0;
		
		if (0 == size(parent))
			return;
		for (child = parent.m_node->m_children.front(); 0 != child; child = child->m_sibling)
			children.push_back(iterator(child));

		return;
	}
	// Delete the subtree rooted at the selected node and return the next node or this->end() if the whole tree is deleted.
	// Note: 1. ~NodeTree will be trigger recursively to delete all the nodes.
	iterator erase(iterator subTreeRoot);
	// Delete the whole node tree.
	void clear(void) { if (0 == m_data) return; (void)erase(root()); }

private:
	friend class iterator;
	friend class const_iterator;

	// Forbid copy control functions
	NodeTree(const NodeTree &rhs);
	NodeTree &operator=(const NodeTree &rhs);

	value_type *m_data;
	NodeTree *m_parent;
	NodeTree *m_sibling;
	std::list<NodeTree *> m_children;
};
#include "NodeTreeImplementation.h"