#pragma once

#include <type_traits>

namespace wbl {

template<typename Node>
struct NodeMovementT {
    static constexpr inline Node *last_sibling_r(Node *node) {
        if (node->sibling)
            return last_sibling_r(node->sibling);
        return node;
    }

    static constexpr inline Node *last_sibling(Node *node) {
        while (node->sibling) node = node->sibling;
        return node;
    }

    static constexpr inline Node *deepest_child(Node *node) {
        while (node->child) node = node->child;
        return node;
    }

    static constexpr inline Node *root_node(Node *node) {
        while (node->parent) node = node->parent;
        return node;
    }

    static constexpr inline Node *previous_sibling(Node *node) {
        if (!node->parent || !node->parent->child || node->parent->child == node)
            return nullptr;
        const Node *start = node;
        node = node->parent->child;
        while (node->sibling != start && node->sibling)
            node = node->sibling;
        return node;
    }

    static constexpr inline Node *depth_first_next(Node *node) {
        Node *next = node->sibling;

        if (!next)
            return node->parent;

        if (next->child)
            return deepest_child(next);

        return next;
    }

    static constexpr inline Node *depth_first_reverse_next(Node *node) {
        Node *next = node->child ? node->child : node->sibling;

        if (!next && node->parent) {
            while (node && node->parent && !node->sibling) node = node->parent;
            return node->sibling;
        }
            
        return next;
    }

    static constexpr inline Node *breadth_first_next(Node *node) {
        if (node->sibling)
            return node->sibling;

        if (node->parent) {
            Node *candidate = node->parent;
            const int level = node_depth(node);
            if (node->parent->sibling) {

            }
        }

        return node->child;
    }

    static constexpr inline int node_depth(const Node *node) {
        int i = 0;
        for (;node->parent;node = node->parent, i++);
        return i;
    }
};

template<typename Derived, typename Node>
struct NodeIteratorT : public NodeMovementT<Node> {
    using value_type = Node;
    using difference_type = void*;
    using pointer = Node*;
    using reference = Node&;

    pointer ptr;
    const pointer ptr_begin;

    constexpr NodeIteratorT(pointer p)
        :ptr(p),ptr_begin(p) { }
    constexpr NodeIteratorT():NodeIteratorT(nullptr) { }
    constexpr NodeIteratorT(const Derived &p):NodeIteratorT(p.ptr) { }

    constexpr inline reference operator*() const { return *ptr; }
    constexpr inline pointer operator->() const { return ptr; }

    constexpr inline Derived begin() const { return Derived(ptr_begin); }

    constexpr inline Derived end() const { return Derived(nullptr); }


    constexpr inline bool operator==(const Derived &other) const { return ptr == other.ptr; }
    constexpr inline bool operator!=(const Derived &other) const { return ptr != other.ptr; }
};

template<typename Node>
struct ChildIteratorT : public NodeIteratorT<ChildIteratorT<Node>, Node> {
    using NodeIterator = NodeIteratorT<ChildIteratorT<Node>, Node>;
    using NodeIterator::NodeIterator;
    using NodeIterator::ptr;

    constexpr inline ChildIteratorT &operator++() {
        ptr = ptr->sibling;
        return *this;
    }

    constexpr inline static ChildIteratorT make(Node *p) { return ChildIteratorT(p->child); }
};

template<typename Node>
struct ReverseChildIteratorT : public NodeIteratorT<ReverseChildIteratorT<Node>, Node> {
    using NodeIterator = NodeIteratorT<ReverseChildIteratorT<Node>, Node>;
    using NodeIterator::NodeIterator;
    using NodeIterator::ptr;

    constexpr inline ReverseChildIteratorT &operator++() {
        ptr = NodeIterator::previous_sibling(ptr);
        return *this;
    }

    constexpr inline static ReverseChildIteratorT make(Node *p) { return ReverseChildIteratorT(NodeIterator::last_sibling(p->child)); }
};

template<typename Node>
struct DepthFirstIteratorT : public NodeIteratorT<DepthFirstIteratorT<Node>, Node> {
    using NodeIterator = NodeIteratorT<DepthFirstIteratorT<Node>, Node>;
    using NodeIterator::NodeIterator;
    using NodeIterator::ptr;

    constexpr inline DepthFirstIteratorT &operator++() {
        ptr = NodeIterator::depth_first_next(ptr);
        return *this;
    }

    constexpr inline static DepthFirstIteratorT make(Node *p) { return DepthFirstIteratorT(NodeIterator::deepest_child(p)); }
};

template<typename Node>
struct DepthFirstReverseIteratorT : public NodeIteratorT<DepthFirstReverseIteratorT<Node>, Node> {
    using NodeIterator = NodeIteratorT<DepthFirstReverseIteratorT<Node>, Node>;
    using NodeIterator::NodeIterator;
    using NodeIterator::ptr;

    constexpr inline DepthFirstReverseIteratorT &operator++() {
        ptr = NodeIterator::depth_first_reverse_next(ptr);
        return *this;
    }

    constexpr inline static DepthFirstReverseIteratorT make(Node *p) { return DepthFirstReverseIteratorT(p); }
};

template<typename NodeIterT, typename = std::is_base_of<NodeIteratorT<NodeIterT, typename NodeIterT::value_type>, NodeIterT>>
static constexpr inline NodeIterT operator++(NodeIterT &src) {
    NodeIterT temp = src;
    ++(src);
    return temp;
}

template<typename Derived>
struct NodeMovementOpsT : public NodeMovementT<Derived> {
    private:
    Derived *super;
    using Movement = NodeMovementT<Derived>;
    using ChildIterator = ChildIteratorT<Derived>;
    using ReverseChildIterator = ReverseChildIteratorT<Derived>;
    using DepthFirstIterator = DepthFirstIteratorT<Derived>;
    using DepthFirstReverseIterator = DepthFirstReverseIteratorT<Derived>;

    public:
    NodeMovementOpsT():super(static_cast<Derived*>(this)){}

    constexpr inline Derived *last_sibling() { return last_sibling(super); }
    constexpr inline Derived *deepest_child() { return deepest_child(super); }
    constexpr inline Derived *root_node() { return root_node(super); }
    constexpr inline Derived *previous_sibling() { return previous_sibling(super); }
    constexpr inline Derived *depth_first_next() { return depth_first_next(super); }
    constexpr inline Derived *depth_first_reverse_next() { return depth_first_reverse_next(super); }
    constexpr inline Derived *breadth_first_next() { return breadth_first_next(super); }
    constexpr inline unsigned short node_depth() const { return node_depth(super); }

    constexpr inline ChildIterator children() { return ChildIterator::make(super); }
    constexpr inline ReverseChildIterator rchildren() { return ReverseChildIterator::make(super); }
    constexpr inline DepthFirstIterator depthfirst() { return DepthFirstIterator::make(super); }
    constexpr inline DepthFirstReverseIterator rdepthfirst() { return DepthFirstReverseIterator::make(super); }
};

}