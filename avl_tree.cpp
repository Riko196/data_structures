#include <iostream>
using namespace std;

/* AVL tree which represents multiset */

class AVLTree
{
private:
	class Node
	{
	public:
		int value;
		int count;
		int height;
		Node *left;
		Node *right;

		Node(int value)
		{
			this->value = value;
			this->count = 1;
			this->height = 1;
			this->left = NULL;
			this->right = NULL;
		}

		~Node()
		{
			if (this->left != NULL)
				delete this->left;
			if (this->right != NULL)
				delete this->right;
		}
	};

	Node *root;

	Node *leftRotate(Node *root)
	{
		Node *newRoot = root->right;
		root->right = root->right->left;
		newRoot->left = root;
		root->height = this->setHeight(root);
		newRoot->height = this->setHeight(newRoot);
		return newRoot;
	}

	Node *rightRotate(Node *root)
	{
		Node *newRoot = root->left;
		root->left = root->left->right;
		newRoot->right = root;
		root->height = this->setHeight(root);
		newRoot->height = this->setHeight(newRoot);
		return newRoot;
	}

	int max(int a, int b)
	{
		return (a > b) ? a : b;
	}

	int setHeight(Node *root)
	{
		if (root == NULL)
			return 0;
		return 1 + this->max(this->height(root->left), this->height(root->right));
	}

	int height(Node *root)
	{
		if (root == NULL)
			return 0;
		return root->height;
	}

	int getBalance(Node *node)
	{
		if (node == NULL)
			return 0;
		return height(node->left) - height(node->right);
	}

	Node *balanceTree(Node *root)
	{
		int balance = this->getBalance(root);
		// Left Left Case
		if (balance > 1 && this->getBalance(root->left) >= 0)
			return this->rightRotate(root);
		// Left Right Case
		else if (balance > 1 && this->getBalance(root->left) < 0)
		{
			root->left = this->leftRotate(root->left);
			return this->rightRotate(root);
		}
		// Right Right Case
		else if (balance < -1 && this->getBalance(root->right) <= 0)
			return this->leftRotate(root);
		// Right Left Case
		else if (balance < -1 && this->getBalance(root->right) > 0)
		{
			root->right = this->rightRotate(root->right);
			return this->leftRotate(root);
		}
		// Dont balance tree, just return node
		else
			return root;
	}

	Node *min(Node *node)
	{
		if (node == NULL)
			return NULL;

		if (node->left != NULL)
			return min(node->left);
		return node;
	}

	Node *recursiveInsert(Node *node, int value)
	{
		if (node == NULL)
			return new Node(value);

		if (value < node->value)
			node->left = this->recursiveInsert(node->left, value);
		else if (value == node->value)
		{
			node->count++;
			return node;
		}
		else
			node->right = this->recursiveInsert(node->right, value);
		node->height = this->setHeight(node);
		return this->balanceTree(node);
	}

	Node *recursiveFind(Node *node, int value)
	{
		if (node == NULL)
			return NULL;

		if (value < node->value)
			return this->recursiveFind(node->left, value);
		else if (value == node->value)
			return node;
		else
			return this->recursiveFind(node->right, value);
	}

	Node *recursiveRemove(Node *node, int value, bool firstRemovingNode)
	{
		if (node == NULL)
			return NULL;

		if (value < node->value)
			node->left = this->recursiveRemove(node->left, value, firstRemovingNode);
		else if (value == node->value)
		{
			if (node->count > 1 && firstRemovingNode)
			{
				node->count--;
				return node;
			}

			if (node->left == NULL && node->right == NULL)
				return NULL;
			else if (node->left == NULL)
				return node->right;
			else if (node->right == NULL)
				return node->left;
			else
			{
				Node *rightMin = this->min(node->right);
				node->value = rightMin->value;
				node->count = rightMin->count;
				node->right = this->recursiveRemove(node->right, rightMin->value, false);
				return node;
			}
		}
		else
			node->right = this->recursiveRemove(node->right, value, firstRemovingNode);
		return this->balanceTree(node);
	}

public:
	void insert(int value)
	{
		this->root = this->recursiveInsert(this->root, value);
	}

	int find(int value)
	{
		Node *node = this->recursiveFind(this->root, value);
		if (node == NULL)
			return 0;
		return node->count;
	}

	void remove(int value)
	{
		this->root = this->recursiveRemove(this->root, value, true);
	}

	~AVLTree()
	{
		if (this->root != NULL)
			delete this->root;
	}
};
