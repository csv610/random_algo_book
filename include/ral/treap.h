#pragma once
#include <iostream>
#include <random>
#include <vector>
#include <iomanip>
#include <cassert>
#include <cmath>

namespace chapter9 {

struct TreapNode {
    int key;
    int priority;
    TreapNode* left;
    TreapNode* right;

    TreapNode(int k, int p) : key(k), priority(p), left(nullptr), right(nullptr) {}
};

class Treap {
private:
    TreapNode* root_;
    std::mt19937& rng_;

    void destroy(TreapNode* node) {
        if (!node) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

    TreapNode* rotate_right(TreapNode* node) {
        TreapNode* y = node->left;
        node->left = y->right;
        y->right = node;
        return y;
    }

    TreapNode* rotate_left(TreapNode* node) {
        TreapNode* x = node->right;
        node->right = x->left;
        x->left = node;
        return x;
    }

    TreapNode* insert_impl(TreapNode* node, int key, int priority) {
        if (!node) {
            return new TreapNode(key, priority);
        }
        if (key < node->key) {
            node->left = insert_impl(node->left, key, priority);
            if (node->left->priority < node->priority) {
                node = rotate_right(node);
            }
        } else if (key > node->key) {
            node->right = insert_impl(node->right, key, priority);
            if (node->right->priority < node->priority) {
                node = rotate_left(node);
            }
        }
        return node;
    }

    TreapNode* remove_impl(TreapNode* node, int key) {
        if (!node) return nullptr;
        if (key < node->key) {
            node->left = remove_impl(node->left, key);
        } else if (key > node->key) {
            node->right = remove_impl(node->right, key);
        } else {
            if (!node->left && !node->right) {
                delete node;
                return nullptr;
            } else if (!node->left) {
                TreapNode* result = node->right;
                node->right = nullptr;
                delete node;
                return result;
            } else if (!node->right) {
                TreapNode* result = node->left;
                node->left = nullptr;
                delete node;
                return result;
            } else {
                if (node->left->priority < node->right->priority) {
                    node = rotate_right(node);
                    node->right = remove_impl(node->right, key);
                } else {
                    node = rotate_left(node);
                    node->left = remove_impl(node->left, key);
                }
            }
        }
        return node;
    }

    TreapNode* search_impl(TreapNode* node, int key) const {
        if (!node) return nullptr;
        if (key == node->key) return node;
        if (key < node->key) return search_impl(node->left, key);
        return search_impl(node->right, key);
    }

    void inorder_impl(const TreapNode* node, std::vector<int>& result) const {
        if (!node) return;
        inorder_impl(node->left, result);
        result.push_back(node->key);
        inorder_impl(node->right, result);
    }

    bool verify_bst_impl(const TreapNode* node, int min_val, int max_val) const {
        if (!node) return true;
        if (node->key <= min_val || node->key >= max_val) return false;
        return verify_bst_impl(node->left, min_val, node->key) &&
               verify_bst_impl(node->right, node->key, max_val);
    }

    bool verify_heap_impl(const TreapNode* node) const {
        if (!node) return true;
        bool ok = true;
        if (node->left && node->left->priority < node->priority) ok = false;
        if (node->right && node->right->priority < node->priority) ok = false;
        return ok && verify_heap_impl(node->left) && verify_heap_impl(node->right);
    }

    int height_impl(const TreapNode* node) const {
        if (!node) return 0;
        return 1 + std::max(height_impl(node->left), height_impl(node->right));
    }

    int count_nodes(const TreapNode* node) const {
        if (!node) return 0;
        return 1 + count_nodes(node->left) + count_nodes(node->right);
    }

public:
    explicit Treap(std::mt19937& rng) : root_(nullptr), rng_(rng) {}
    ~Treap() { destroy(root_); }

    Treap(const Treap&) = delete;
    Treap& operator=(const Treap&) = delete;

    void insert(int key) {
        std::uniform_int_distribution<int> dist(1, 2000000000);
        int priority = dist(rng_);
        root_ = insert_impl(root_, key, priority);
    }

    void remove(int key) {
        root_ = remove_impl(root_, key);
    }

    bool search(int key) const {
        return search_impl(root_, key) != nullptr;
    }

    bool verify_bst() const {
        return verify_bst_impl(root_, std::numeric_limits<int>::min(),
                                std::numeric_limits<int>::max());
    }

    bool verify_heap() const {
        return verify_heap_impl(root_);
    }

    int height() const {
        return height_impl(root_);
    }

    int size() const {
        return count_nodes(root_);
    }

    std::vector<int> inorder() const {
        std::vector<int> result;
        inorder_impl(root_, result);
        return result;
    }
};

inline void demonstrate_treap() {
    std::cout << "=== Random Treap ===\n\n";

    std::mt19937 rng(42);

    {
        Treap treap(rng);
        std::cout << "Inserting elements 1..20:\n";
        for (int i = 1; i <= 20; i++) {
            treap.insert(i);
        }

        std::cout << "  Size: " << treap.size() << "\n";
        std::cout << "  Height: " << treap.height() << "\n";
        std::cout << "  BST property: " << (treap.verify_bst() ? "OK" : "FAIL") << "\n";
        std::cout << "  Heap property: " << (treap.verify_heap() ? "OK" : "FAIL") << "\n";

        auto sorted = treap.inorder();
        std::cout << "  In-order: ";
        for (int i = 0; i < (int)sorted.size() && i < 20; i++) {
            std::cout << sorted[i];
            if (i < 19) std::cout << " ";
        }
        std::cout << "\n";

        std::cout << "  Search 10: " << (treap.search(10) ? "found" : "not found") << "\n";
        std::cout << "  Search 99: " << (treap.search(99) ? "found" : "not found") << "\n";

        treap.remove(10);
        std::cout << "  After removing 10:\n";
        std::cout << "    Search 10: " << (treap.search(10) ? "found" : "not found") << "\n";
        std::cout << "    Size: " << treap.size() << "\n";
        std::cout << "    BST property: " << (treap.verify_bst() ? "OK" : "FAIL") << "\n";
        std::cout << "    Heap property: " << (treap.verify_heap() ? "OK" : "FAIL") << "\n";
    }

    std::cout << "\n--- Height analysis (10000 trials) ---\n";
    {
        std::vector<int> sizes = {100, 500, 1000, 5000, 10000};
        for (int n : sizes) {
            double total_height = 0;
            int trials = 10000;
            for (int t = 0; t < trials; t++) {
                std::mt19937 trial_rng(t * 12345 + n);
                Treap treap(trial_rng);
                for (int i = 0; i < n; i++) {
                    treap.insert(i + 1);
                }
                total_height += treap.height();
            }
            double avg = total_height / trials;
            double theoretical = 2.0 * std::log2(n);
            std::cout << "  n=" << std::setw(5) << n
                      << "  avg_height=" << std::fixed << std::setprecision(1) << std::setw(6) << avg
                      << "  2*lg(n)=" << std::fixed << std::setprecision(1) << std::setw(6) << theoretical
                      << "  ratio=" << std::fixed << std::setprecision(2) << avg / theoretical << "\n";
        }
    }
}

} // namespace chapter9
