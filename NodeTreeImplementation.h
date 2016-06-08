template <typename key, typename data, typename keyComp>
typename NodeTree<key, data, keyComp>::iterator &NodeTree<key, data, keyComp>::iterator::operator++(void)
{
	NodeTree *node = this->m_node;

	if (0 == node)
		return *this;

	// Use depth first search to get the next node.
	if (!node->m_children.empty())
	{
		this->m_node = node->m_children.front();
	}
	else if (0 != node->m_sibling)
	{
		this->m_node = node->m_sibling;
	}
	else
	{
		do
		{
			node = node->m_parent;
		}while((0 != node) && (0 == node->m_sibling));
		m_node = (0 == node) ? 0 : node->m_sibling;
	}

	return *this;
}

template <typename key, typename data, typename keyComp>
typename NodeTree<key, data, keyComp>::iterator NodeTree<key, data, keyComp>::iterator::operator++(int)
{
	iterator itr(*this);

	++(*this);

	return itr;
}

template <typename key, typename data, typename keyComp>
typename NodeTree<key, data, keyComp>::const_iterator &NodeTree<key, data, keyComp>::const_iterator::operator++(void)
{
	const NodeTree *node = this->m_node;

	if (0 == node)
		return *this;

	// Use depth first search to get the next node.
	if (!node->m_children.empty())
	{
		this->m_node = node->m_children.front();
	}
	else if (0 != node->m_sibling)
	{
		this->m_node = node->m_sibling;
	}
	else
	{
		do
		{
			node = node->m_parent;
		}while((0 != node) && (0 == node->m_sibling));
		m_node = (0 == node) ? 0 : node->m_sibling;
	}

	return *this;
}

template <typename key, typename data, typename keyComp>
typename NodeTree<key, data, keyComp>::const_iterator NodeTree<key, data, keyComp>::const_iterator::operator++(int)
{
	const_iterator itr(*this);

	++(*this);

	return itr;
}

template <typename key, typename data, typename keyComp>
NodeTree<key, data, keyComp>::~NodeTree(void)
{
	std::list<NodeTree *>::iterator itr;

	if (0 != m_data)
		delete m_data;
	// FIXME: This will trigger a recursive call to ~NodeTree to delete all the nodes,
	// is it better to use a stack to delete all the nodes in one ~NodeTree call?
	for (itr = m_children.begin(); itr != m_children.end();)
	{
		delete *itr;
		itr = m_children.erase(itr);
	}
}

template <typename key, typename data, typename keyComp>
void NodeTree<key, data, keyComp>::getPath(iterator itr, std::list<key_type> &keyPath) const
{
	NodeTree *tmp = itr.m_node;

	while(0 != tmp)
	{
		keyPath.push_front(tmp->m_data->first);
		tmp = tmp->m_parent;
	}

	return;
}

template <typename key, typename data, typename keyComp>
typename NodeTree<key, data, keyComp>::iterator NodeTree<key, data, keyComp>::find(iterator root, const key_type &key)
{
	keyComp comp;
	iterator lastItr, tmp;
	NodeTree *lastNode = root.m_node;

	// The iterator increment uses depth first search, so the most-right node of the subtree is the last node.
	while(!lastNode->m_children.empty())
		lastNode = lastNode->m_children.back();
	lastItr.m_node = lastNode;

	for (tmp = root; tmp != lastItr; ++tmp)
	{
		if (!comp(tmp->first, key) && !comp(key, tmp->first))
			break;
	}
	if ((tmp == lastItr) && (comp(tmp->first, key) || comp(key, tmp->first)))
		return end();

	return tmp;
}

template <typename key, typename data, typename keyComp>
typename NodeTree<key, data, keyComp>::iterator NodeTree<key, data, keyComp>::find(const std::list<key_type> &keyPath)
{
	std::list<key_type>::const_iterator listItr;
	NodeTree *node = 0, *prevNode = 0;
	keyComp comp;

	// Empty node tree, return an end iterator.
	if (0 == m_data)
		return end();

	for (listItr = keyPath.begin(), prevNode = 0, node = this; listItr != keyPath.end(); ++listItr)
	{
		// Search the direct children of the node to find whether the keys match.
		while(0 != node)
		{
			if (!comp(node->m_data->first, *listItr) && !comp(*listItr, node->m_data->first))
				break;
			node = node->m_sibling;
		}
		// No node key at the same level matchs the key in the key path.
		if (0 == node)
		{
			return end();
		}
		// The keys between node and path at this level match, let's go into the next level.
		// Namely, we should match keys between children of the node and the next key path.
		prevNode = node;
		node = node->m_children.empty() ? 0 : node->m_children.front();
	}

	// Find the matched node.
	return iterator(prevNode);
}

template <typename key, typename data, typename keyComp>
typename NodeTree<key, data, keyComp>::iterator NodeTree<key, data, keyComp>::insert(iterator parent, const key_type &key, const std::vector<data_type> &data)
{
	keyComp comp;
	NodeTree *parentNode = parent.m_node;
	NodeTree *siblingNode = 0, *tmp = 0, *newNode = 0;
	
	if (!parentNode->m_children.empty())
	{
		for (tmp = parentNode->m_children.front(); 0 != tmp; tmp = tmp->m_sibling)
		{
			// The node with the given key exists, add new values to it and then return an iterator pointing to it.
			if (!comp(tmp->m_data->first, key) && !comp(key, tmp->m_data->first))
			{
				std::vector<data_type>::iterator itr = tmp->m_data->second.end();
				tmp->m_data->second.insert(itr, data.begin(), data.end());
				return iterator(tmp);
			}
		}
		siblingNode = parentNode->m_children.back();
	}
	// No such node with the given key, create it.
	newNode = new NodeTree(key, data);
	newNode->m_parent = parentNode;
	newNode->m_sibling = 0;
	parentNode->m_children.push_back(newNode);
	if (0 != siblingNode)
		siblingNode->m_sibling = newNode;

	return iterator(newNode);
}

template <typename key, typename data, typename keyComp>
typename NodeTree<key, data, keyComp>::iterator NodeTree<key, data, keyComp>::add(const std::list<key_type> &keyPath, const std::vector<data_type> &data)
{
	std::list<key_type>::const_iterator listItr;
	NodeTree *parent = 0, *current = 0;
	keyComp comp;

	if (keyPath.empty())
		return end();

	listItr = keyPath.begin();
	// If the tree is empty, firstly update "this" node to make it the root node.
	if (0 == m_data)
	{
		std::vector<data_type> tmp;

		m_data = new value_type(*listItr, tmp);
		parent = this;
		++listItr;
	}
	// Else, try to find the given node.
	else
	{
		for(current = this, parent = current->m_parent; listItr != keyPath.end(); ++listItr)
		{
			// Search the current node and its siblings to find the node with the matched key.
			while(0 != current)
			{
				if (!comp(current->m_data->first, *listItr) && !comp(*listItr, current->m_data->first))
					break;
				current = current->m_sibling;
			}
			// No tree node matches the key at the same level, this is the first missing node.
			if (0 == current)
				break;
			parent = current;
			current = current->m_children.empty() ? 0 : current->m_children.front();
		}
		// If the given node exists, it is the node pointed to by parent. Add new values to it.
		if (listItr == keyPath.end())
		{
			std::vector<data_type>::iterator itr = parent->m_data->second.end();
			parent->m_data->second.insert(itr, data.begin(), data.end());
			return iterator(parent);
		}
		// If the starting node indicated by the key path is not the root node of current tree, this key path
		// indicates another node tree, return end() in this case.
		else if (listItr == keyPath.begin())
		{
			return end();
		}
	}
	
	// Create the given node and its missing parent nodes. Here we are sure the parent node must exist, so use it directly.	
	for(;listItr != keyPath.end(); ++listItr)
	{
		NodeTree *tmp = new NodeTree(*listItr);
		tmp->m_parent = parent;
		tmp->m_sibling = 0;
		if (!parent->m_children.empty())
		{
			parent->m_children.back()->m_sibling = tmp;
		}
		parent->m_children.push_back(tmp);
		parent = tmp;
	}
	parent->m_data->second = data;

	return iterator(parent);
}

template <typename key, typename data, typename keyComp>
typename NodeTree<key, data, keyComp>::iterator NodeTree<key, data, keyComp>::erase(iterator subTreeRoot)
{
	NodeTree *currNode = subTreeRoot.m_node;
	NodeTree *nextNode = 0, *parentNode = currNode->m_parent;
	std::list<NodeTree *>::iterator prevItr, currItr;

	// The node being erased is the root node, the whole node tree will be deleted.
	if (0 == parentNode)
	{
		// Erase all its descendant nodes of the root node.
		std::list<NodeTree *>::iterator iter = currNode->m_children.begin(); 
		while(iter != currNode->m_children.end())
		{
			delete *iter;
			iter = currNode->m_children.erase(iter);
		}
		// Update the root node to make it an empty node.
		delete m_data;
		m_data = 0;
		return end();
	}
	// Delete the subtree rooted at the given node and update the subroot node's previous sibling node if any.
	prevItr = currItr = parentNode->m_children.begin();
	while(*currItr != currNode)
	{
		prevItr = currItr;
		++currItr;
	}
	if (prevItr != currItr)
	{
		(*prevItr)->m_sibling = (*currItr)->m_sibling;
	}
	delete *currItr;
	currItr = parentNode->m_children.erase(currItr);

	// If the erased node has siblings, the sibling node next to it is the next node.
	if (currItr != parentNode->m_children.end())
		return iterator(*currItr);
	// Otherwise, the sibling node of the first ancestor node or end() is the next node.
	while((0 != parentNode) && (0 == parentNode->m_sibling))
		parentNode = parentNode->m_parent;

	return (0 == parentNode) ? end() : iterator(parentNode->m_sibling);
}