#include <iostream>
#include <limits>
#include <vector>

using namespace std;

// If the condition is not true, report an error and halt.
#define EXPECT(condition, message)                \
    do {                                          \
        if (!(condition)) expect_failed(message); \
    } while (0)

void expect_failed(const string &message);

/*** Tree ***/

class ab_tree {
   private:
    /*** One node ***/

    class ab_node {
       public:
        // Keys stored in this node and the corresponding children
        // The vectors are large enough to accomodate one extra entry
        // in overflowing nodes.
        vector<ab_node *> children;
        vector<int> keys;

        // If this node contains the given key, return true and set i to key's position.
        // Otherwise return false and set i to the first key greater than the given one.
        bool find_branch(int key, int &i) {
            i = 0;
            while (i < keys.size() && keys[i] <= key) {
                if (keys[i] == key)
                    return true;
                i++;
            }
            return false;
        }

        // Insert a new key at posision i and add a new child between keys i and i+1.
        void insert_branch(int i, int key, ab_node *child) {
            keys.insert(keys.begin() + i, key);
            children.insert(children.begin() + i + 1, child);
        }
    };

    int a;          // Minimum allowed number of children
    int b;          // Maximum allowed number of children
    ab_node *root;  // Root node (even a tree with no keys has a root)
    int num_nodes;  // We keep track of how many nodes the tree has

    // Create a new node and return a pointer to it.
    ab_node *new_node() {
        ab_node *n = new ab_node;
        n->keys.reserve(b);
        n->children.reserve(b + 1);
        num_nodes++;
        return n;
    }

    // Delete a given node, assuming that its children have been already unlinked.
    void delete_node(ab_node *n) {
        num_nodes--;
        delete n;
    }

    // An auxiliary function for deleting a subtree recursively.
    void delete_tree(ab_node *n) {
        for (int i = 0; i < n->children.size(); i++)
            if (n->children[i])
                delete_tree(n->children[i]);
        delete_node(n);
    }

    void recursive_insert(int key, ab_node *node) {
        // finding node
        int index;
        if (node->find_branch(key, index))
            return;

        ab_node *child = node->children[index];

        if (child == nullptr)                          // we are in the last internal node, deeper is only nullptr leaf
            node->insert_branch(index, key, nullptr);  // insert the new node
        else
            this->recursive_insert(key, child);  // we need to go deeper for finding or inserting the new node

        // fixing potencial too big child of node
        this->divide_too_big_child(index, node, child);
    }

    void divide_too_big_child(int parentIndex, ab_node *parent, ab_node *child) {
        if (child && child->keys.size() == this->b) {
            int middleIndex = (this->b - 1) / 2;
            // create new left child from too big child
            ab_node *newLeftChild = this->new_node();
            for (int i = 0; i <= middleIndex; i++) {
                if (i != middleIndex) newLeftChild->keys.push_back(child->keys[i]);
                newLeftChild->children.push_back(child->children[i]);
            }

            // create new right child from too big child
            ab_node *newRightChild = this->new_node();
            for (int i = middleIndex + 1; i <= this->b; i++) {
                if (i != this->b) newRightChild->keys.push_back(child->keys[i]);
                newRightChild->children.push_back(child->children[i]);
            }

            // join new left and right children to the current parent and delete old too big child
            if (parent) {  // it is called for inner node
                parent->children[parentIndex] = newRightChild;
                parent->keys.insert(parent->keys.begin() + parentIndex, child->keys[middleIndex]);
                parent->children.insert(parent->children.begin() + parentIndex, newLeftChild);
            } else {  // it is called for the root and the root does not have a parent
                ab_node *newRoot = this->new_node();
                newRoot->keys.push_back(child->keys[middleIndex]);
                newRoot->children.push_back(newLeftChild);
                newRoot->children.push_back(newRightChild);
                this->root = newRoot;
            }
            this->delete_node(child);
        }
    }

   public:
    // Constructor: initialize an empty tree with just the root.
    ab_tree(int a, int b) {
        EXPECT(a >= 2 && b >= 2 * a - 1, "Invalid values of a,b");
        this->a = a;
        this->b = b;
        num_nodes = 0;
        // The root has no keys and one null child pointer.
        root = new_node();
        root->children.push_back(nullptr);
    }

    // Find a key: returns true if it is present in the tree.
    bool find(int key) {
        ab_node *n = root;
        while (n) {
            int i;
            if (n->find_branch(key, i))
                return true;
            n = n->children[i];
        }
        return false;
    }

    // Insert: add key to the tree (unless it was already present).
    void insert(int key) {
        this->recursive_insert(key, this->root);
        // fixing potencial too big the root
        this->divide_too_big_child(0, nullptr, this->root);
    }

    // remove: remove key from the tree (unless it doesnt exist in the tree).
    void remove(int key) {
        // TODO
    }

    // Destructor: delete all nodes.
    ~ab_tree() {
        this->delete_tree(root);
        EXPECT(num_nodes == 0, "Memory leak detected: some nodes were not deleted");
    }
};
