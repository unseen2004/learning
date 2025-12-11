#ifndef BINOMIAL_HEAP_HPP
#define BINOMIAL_HEAP_HPP

#include <vector>
#include <stdexcept>

template<typename T>
class BinomialHeap {
private:
    struct Node {
        T key;
        int degree;
        Node* parent;
        Node* child;
        Node* sibling;

        Node(const T& k) : key(k), degree(0), parent(nullptr), child(nullptr), sibling(nullptr) {}
    };

    Node* head;

    void link(Node* y, Node* z) {
        y->parent = z;
        y->sibling = z->child;
        z->child = y;
        z->degree++;
    }

    Node* merge_root_lists(Node* h1, Node* h2) {
        if (!h1) return h2;
        if (!h2) return h1;

        Node* head = nullptr;
        Node* tail = nullptr;

        while (h1 && h2) {
            Node* next;
            if (h1->degree <= h2->degree) {
                next = h1;
                h1 = h1->sibling;
            } else {
                next = h2;
                h2 = h2->sibling;
            }

            if (!head) {
                head = tail = next;
            } else {
                tail->sibling = next;
                tail = next;
            }
        }

        if (h1) tail->sibling = h1;
        if (h2) tail->sibling = h2;

        return head;
    }

    Node* union_helper(Node* h1, Node* h2) {
        Node* h = merge_root_lists(h1, h2);
        if (!h) return nullptr;

        Node* prev_x = nullptr;
        Node* x = h;
        Node* next_x = x->sibling;

        while (next_x) {
            if ((x->degree != next_x->degree) ||
                (next_x->sibling && next_x->sibling->degree == x->degree)) {
                prev_x = x;
                x = next_x;
            } else {
                if (x->key <= next_x->key) {
                    x->sibling = next_x->sibling;
                    link(next_x, x);
                } else {
                    if (!prev_x) {
                        h = next_x;
                    } else {
                        prev_x->sibling = next_x;
                    }
                    link(x, next_x);
                    x = next_x;
                }
            }
            next_x = x->sibling;
        }

        return h;
    }

    Node* find_min_node() {
        if (!head) return nullptr;

        Node* min_node = head;
        Node* current = head->sibling;

        while (current) {
            if (current->key < min_node->key) {
                min_node = current;
            }
            current = current->sibling;
        }

        return min_node;
    }

    Node* reverse_sibling_list(Node* node) {
        Node* prev = nullptr;
        Node* current = node;

        while (current) {
            Node* next = current->sibling;
            current->sibling = prev;
            current->parent = nullptr;
            prev = current;
            current = next;
        }

        return prev;
    }

    void clear_node(Node* node) {
        if (!node) return;
        clear_node(node->child);
        clear_node(node->sibling);
        delete node;
    }

    void clear() {
        clear_node(head);
        head = nullptr;
    }

public:
    BinomialHeap() : head(nullptr) {}

    ~BinomialHeap() {
        clear();
    }

    void make_heap() {
        clear();
        head = nullptr;
    }

    void insert(const T& key) {
        BinomialHeap<T> temp_heap;
        temp_heap.head = new Node(key);
        head = union_helper(head, temp_heap.head);
        temp_heap.head = nullptr;
    }

    void union_with(BinomialHeap<T>& other) {
        head = union_helper(head, other.head);
        other.head = nullptr;
    }

    T extract_min() {
        if (!head) throw std::runtime_error("Heap is empty");

        Node* min_node = find_min_node();
        T min_key = min_node->key;

        if (min_node == head) {
            head = head->sibling;
        } else {
            Node* current = head;
            while (current->sibling != min_node) {
                current = current->sibling;
            }
            current->sibling = min_node->sibling;
        }

        BinomialHeap<T> child_heap;
        child_heap.head = reverse_sibling_list(min_node->child);

        head = union_helper(head, child_heap.head);
        child_heap.head = nullptr;

        delete min_node;
        return min_key;
    }

    bool is_empty() const {
        return head == nullptr;
    }

    T get_min() const {
        if (!head) throw std::runtime_error("Heap is empty");
        Node* min_node = find_min_node();
        return min_node->key;
    }
};

#endif // BINOMIAL_HEAP_HPP