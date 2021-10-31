#ifndef NPRG058_HA1_LOCKFREE_GUARD__
#define NPRG058_HA1_LOCKFREE_GUARD__

#include <atomic>

template <typename T>
class LFStack {
   private:
    struct SmartNodePointer;
    struct Node;

    /*
		My own pointer which points to the next element and also
		uses 64b counter which is used for solving ABA problem
	*/
    struct SmartNodePointer {
        Node* node;
        uint64_t counter;
    };

    /* Node which contains element of type T and SmartNodePointer to the next element */
    struct Node {
        T element;
        SmartNodePointer next;
    };

    /* Head for our LFStack */
    std::atomic<SmartNodePointer> lfStackHead;
    /* Head for stack used as memory allocator */
    std::atomic<SmartNodePointer> stackAllocatorHead;
    /* Current size of the stackAllocator */
    std::atomic<uint64_t> stackAllocatorSize;
    /* Counter used for assigning unique integer for SmartNodePointers */
    std::atomic<uint64_t> abaCounter;
    /* It stores size which is currently limit for size of the stackAllocator */
    std::atomic<uint64_t> allocSize;

    /* Function which pops some element from stack (either LFStack or memory allocator stack) */
    Node* internalPop(std::atomic<SmartNodePointer>& head) {
        SmartNodePointer poppedSmartNodePointer = head.load();
        do {
            if (poppedSmartNodePointer.node == nullptr && poppedSmartNodePointer.counter == 0)
                return nullptr;
        } while (!head.compare_exchange_weak(poppedSmartNodePointer, poppedSmartNodePointer.node->next));
        return poppedSmartNodePointer.node;
    }

    /* Function which pushes some element on stack (either LFStack or memory allocator stack) */
    void internalPush(Node* node, std::atomic<SmartNodePointer>& head) {
        SmartNodePointer smartNodePointer({node, abaCounter.fetch_add(1)});
        while (!head.compare_exchange_weak(smartNodePointer.node->next, smartNodePointer));
    }

    /*
        Function for allocation new memory on the stackAllocator. It helps to allocate additional memory
        if there is no more initialized memory.
    */
    void allocMemory() {
        while (stackAllocatorSize.load() <= allocSize.load()) {
            Node* newNode = new Node();
            newNode->next.node = nullptr;
            newNode->next.counter = 0;
            internalPush(newNode, stackAllocatorHead);
            stackAllocatorSize.fetch_add(1);
        }

        if (stackAllocatorSize.load() > allocSize.load() / 2)
            allocSize.fetch_add(1);
    }

   public:
    /* Constructor which initializes heads and counter. Then allocates allocSize free nodes on the stackAllocator */
    LFStack() {
        lfStackHead.store({nullptr, 0});
        stackAllocatorHead.store({nullptr, 0});
        stackAllocatorSize.store(0);
        abaCounter.store(0);
        allocSize.store(1);
        allocMemory();
    }

    /* Function which pops some element from the LFStack and pushes free node on the stackAllocator */
    T pop() {
        Node* oldHead = internalPop(lfStackHead);
        const T element = oldHead->element;
        internalPush(oldHead, stackAllocatorHead);
        stackAllocatorSize.fetch_add(1);
        return element;
    }

    /* Function which pops free node from the stackAllocator, assign its and then pushes it on the LFStack */
    void push(const T& v) {
        Node* freeNode = internalPop(stackAllocatorHead);
        /* Alloc memory until there is at least one free node on the stackAllocator */
        while (freeNode == nullptr) {
            allocMemory();
            freeNode = internalPop(stackAllocatorHead);
        }
        stackAllocatorSize.fetch_sub(1);
        freeNode->element = v;
        internalPush(freeNode, lfStackHead);
    }

    bool empty() const {
        SmartNodePointer smartNodePointer = lfStackHead.load();
        return smartNodePointer.node == nullptr && smartNodePointer.counter == 0;
    }

    /* Destructor which frees all nodes on both stacks */
    ~LFStack() {
        Node* unallocatedNode = internalPop(lfStackHead);
        while (unallocatedNode != nullptr) {
            delete unallocatedNode;
            unallocatedNode = internalPop(lfStackHead);
        }

        unallocatedNode = internalPop(stackAllocatorHead);
        while (unallocatedNode != nullptr) {
            delete unallocatedNode;
            unallocatedNode = internalPop(stackAllocatorHead);
        }
    }
};

#endif
