// Splay tree
class SplayTree {
   private:
    // A node of the tree
    class Node {
       public:
        int key;
        Node* left;
        Node* right;
        Node* parent;

        // Constructor
        Node(int key, Node* parent = nullptr, Node* left = nullptr, Node* right = nullptr) {
            this->key = key;
            this->parent = parent;
            this->left = left;
            this->right = right;
            if (left) left->parent = this;
            if (right) right->parent = this;
        }
    };

    // Pointer to root of the tree; nullptr if the tree is empty.
    Node* root;

    // Rotate the given `node` up. Perform a single rotation of the edge
    // between the node and its parent, choosing left or right rotation
    // appropriately.
    void rotate(Node* node) {
        if (node->parent) {
            if (node->parent->left == node) {
                if (node->right) node->right->parent = node->parent;
                node->parent->left = node->right;
                node->right = node->parent;
            } else {
                if (node->left) node->left->parent = node->parent;
                node->parent->right = node->left;
                node->left = node->parent;
            }
            if (node->parent->parent) {
                if (node->parent->parent->left == node->parent)
                    node->parent->parent->left = node;
                else
                    node->parent->parent->right = node;
            } else {
                root = node;
            }

            Node* original_parent = node->parent;
            node->parent = node->parent->parent;
            original_parent->parent = node;
        }
    }

    // Splay the given node.
    void splay(Node* x) {
        while (x && x != this->root) {
            // execute double rotation or single rotation if parent is root
            if (x->parent->parent) {
                Node* y = x->parent;
                Node* z = y->parent;
                // zig-zig rotation
                if ((z->left == y && y->left == x) || (z->right == y && y->right == x))
                    this->rotate(x->parent);
                // zig-zag rotation
                else
                    this->rotate(x);
            }
            this->rotate(x);
        }
    }

   public:
    // Constructor for Splay tree
    SplayTree(Node* root = nullptr) {
        this->root = root;
    }
    // Look up the given key in the tree, returning the
    // the node with the requested key or nullptr.
    Node* lookup(int key) {
        Node* node = root;
        while (node && node->key != key) {
            if (key < node->key && node->left)
                node = node->left;
            else if (key > node->key && node->right)
                node = node->right;
            else
                break;
        }
        this->splay(node);
        return node->key == key ? node : nullptr;
    }

    // Insert a key into the tree.
    // If the key is already present, nothing happens.
    void insert(int key) {
        if (!root) {
            root = new Node(key);
            return;
        }

        Node* node = root;
        while (node->key != key) {
            if (key < node->key) {
                if (!node->left)
                    node->left = new Node(key, node);
                node = node->left;
            } else {
                if (!node->right)
                    node->right = new Node(key, node);
                node = node->right;
            }
        }
        this->splay(node);
    }

    // Delete given key from the tree.
    // It the key is not present, nothing happens.
    void remove(int key) {
        Node* node = root;
        while (node && node->key != key) {
            if (key < node->key && node->left)
                node = node->left;
            else if (key > node->key && node->right)
                node = node->right;
            else
                break;
        }

        if (node && node->key == key) {
            if (node->left && node->right) {
                Node* replacement = node->right;
                while (replacement->left)
                    replacement = replacement->left;
                node->key = replacement->key;
                node = replacement;
            }

            Node* replacement = node->left ? node->left : node->right;
            if (node->parent) {
                if (node->parent->left == node)
                    node->parent->left = replacement;
                else
                    node->parent->right = replacement;
            } else {
                root = replacement;
            }
            if (replacement)
                replacement->parent = node->parent;
            else
                replacement = node->parent;
            delete node;
            this->splay(replacement);
        } else
            this->splay(node);
    }

    // Destructor to free all allocated memory.
    ~SplayTree() {
        Node* node = root;
        while (node) {
            Node* next;
            if (node->left) {
                next = node->left;
                node->left = nullptr;
            } else if (node->right) {
                next = node->right;
                node->right = nullptr;
            } else {
                next = node->parent;
                delete node;
            }
            node = next;
        }
    }
};
