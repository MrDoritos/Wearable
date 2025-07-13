#pragma once

namespace wbl {

template<typename Node>
struct NodeIteratorT {
    using value_type = Node;
    using difference_type = void*;
    using pointer = Node*;
    using reference = Node&;

    pointer ptr;

    constexpr NodeIteratorT(pointer p):ptr(p){}

    constexpr inline reference operator*() const { return *ptr; }
    constexpr inline pointer operator->() const { return ptr; }

    constexpr inline NodeIteratorT operator++(int) {
        NodeIteratorT temp = *this;
        ++(*this);
        return temp;
    }

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
            node = node->sbiling;
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

    static constexpr inline Node *breadth_first_next(Node *node) {
        Node *next = node->child ? node->child : node->sibling;

        if (!next && node->parent)
            return node->parent->sibling;
            
        return next;
    }

    constexpr inline bool operator==(const NodeIteratorT &other) const { return ptr == other.ptr; }
    constexpr inline bool operator!=(const NodeIteratorT &other) const { return ptr != other.ptr; }
};

template<typename Node, typename NodeIterator = NodeIteratorT<Node>>
struct ChildIteratorT : public NodeIterator {
    using NodeIterator::NodeIterator;
    using NodeIterator::ptr;

    constexpr inline ChildIteratorT &operator++() {
        ptr = ptr->sibling;
        return *this;
    }
};

template<typename Node, typename NodeIterator = NodeIteratorT<Node>>
struct ReverseChildIteratorT : public NodeIterator {
    using NodeIterator::NodeIterator;
    using NodeIterator::ptr;

    constexpr inline ReverseChildIteratorT &operator++() {
        ptr = NodeIterator::previous_sibling(ptr);
        return *this;
    }
};

template<typename Node, typename NodeIterator = NodeIteratorT<Node>>
struct DepthFirstIteratorT : public NodeIterator {
    using NodeIterator::NodeIterator;
    using NodeIterator::ptr;

    constexpr inline DepthFirstIteratorT &operator++() {
        ptr = NodeIterator::depth_first_next(ptr);
        return *this;
    }
};

template<typename Node, typename NodeIterator = NodeIteratorT<Node>>
struct BreadthFirstIteratorT : public NodeIterator {
    using NodeIterator::NodeIterator;
    using NodeIterator::ptr;

    constexpr inline BreadthFirstIteratorT &operator++() {
        ptr = NodeIterator::breadth_first_next(ptr);
        return *this;
    }
};


}