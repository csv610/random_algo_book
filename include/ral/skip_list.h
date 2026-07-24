#pragma once
#include <iostream>
#include <random>
#include <vector>
#include <iomanip>
#include <cassert>
#include <cmath>
#include <limits>

namespace chapter9 {

class SkipList {
private:
    static constexpr int MAX_LEVEL = 16;

    struct Node {
        int key;
        int level;
        std::vector<Node*> forward;

        Node(int k, int lvl) : key(k), level(lvl), forward(lvl + 1, nullptr) {}
    };

    Node* head_;
    int current_level_;
    int size_;
    std::mt19937& rng_;

    int random_level() {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        int lvl = 0;
        while (dist(rng_) < 0.5 && lvl < MAX_LEVEL - 1) {
            lvl++;
        }
        return lvl;
    }

public:
    explicit SkipList(std::mt19937& rng)
        : head_(new Node(std::numeric_limits<int>::min(), MAX_LEVEL - 1)),
          current_level_(0), size_(0), rng_(rng) {
        for (int i = 0; i < MAX_LEVEL; i++) {
            head_->forward[i] = new Node(std::numeric_limits<int>::max(), 0);
        }
    }

    ~SkipList() {
        Node* curr = head_;
        while (curr) {
            Node* next = curr->forward[0];
            delete curr;
            curr = next;
        }
    }

    void insert(int key) {
        std::vector<Node*> update(MAX_LEVEL, nullptr);
        Node* curr = head_;

        for (int i = current_level_; i >= 0; i--) {
            while (curr->forward[i]->key < key) {
                curr = curr->forward[i];
            }
            update[i] = curr;
        }

        curr = curr->forward[0];
        if (curr->key == key) return;

        int new_level = random_level();
        if (new_level > current_level_) {
            for (int i = current_level_ + 1; i <= new_level; i++) {
                update[i] = head_;
            }
            current_level_ = new_level;
        }

        Node* new_node = new Node(key, new_level);
        for (int i = 0; i <= new_level; i++) {
            new_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = new_node;
        }
        size_++;
    }

    bool search(int key) const {
        Node* curr = head_;
        for (int i = current_level_; i >= 0; i--) {
            while (curr->forward[i]->key < key) {
                curr = curr->forward[i];
            }
        }
        curr = curr->forward[0];
        return curr->key == key;
    }

    void remove(int key) {
        std::vector<Node*> update(MAX_LEVEL, nullptr);
        Node* curr = head_;

        for (int i = current_level_; i >= 0; i--) {
            while (curr->forward[i]->key < key) {
                curr = curr->forward[i];
            }
            update[i] = curr;
        }

        curr = curr->forward[0];
        if (curr->key != key) return;

        for (int i = 0; i <= current_level_; i++) {
            if (update[i]->forward[i] != curr) break;
            update[i]->forward[i] = curr->forward[i];
        }
        delete curr;

        while (current_level_ > 0 && head_->forward[current_level_]->key ==
                                       std::numeric_limits<int>::max()) {
            current_level_--;
        }
        size_--;
    }

    int size() const { return size_; }
    int max_level() const { return current_level_; }

    void print(int max_display = 30) const {
        int count = 0;
        for (int i = current_level_; i >= 0 && count < max_display; i--) {
            std::cout << "  Level " << i << ": -> ";
            Node* curr = head_->forward[i];
            int shown = 0;
            while (curr->key != std::numeric_limits<int>::max() && shown < 8) {
                std::cout << curr->key << " ";
                curr = curr->forward[i];
                shown++;
            }
            if (curr->key != std::numeric_limits<int>::max()) {
                std::cout << "...";
            }
            std::cout << "\n";
        }
    }
};

inline void demonstrate_skip_list() {
    std::cout << "=== Skip List ===\n\n";

    std::mt19937 rng(42);

    {
        SkipList sl(rng);
        std::cout << "Inserting elements 1..20:\n";
        for (int i = 1; i <= 20; i++) {
            sl.insert(i);
        }

        std::cout << "  Size: " << sl.size() << "\n";
        std::cout << "  Max level: " << sl.max_level() << "\n";
        std::cout << "  Search 10: " << (sl.search(10) ? "found" : "not found") << "\n";
        std::cout << "  Search 99: " << (sl.search(99) ? "found" : "not found") << "\n";

        sl.print();

        sl.remove(10);
        std::cout << "\n  After removing 10:\n";
        std::cout << "    Search 10: " << (sl.search(10) ? "found" : "not found") << "\n";
        std::cout << "    Size: " << sl.size() << "\n";
    }

    std::cout << "\n--- Search time analysis (counting pointer traversals) ---\n";
    {
        std::vector<int> sizes = {100, 500, 1000, 5000, 10000};
        for (int n : sizes) {
            double total_traversals = 0;
            int trials = 1000;
            for (int t = 0; t < trials; t++) {
                std::mt19937 trial_rng(t * 99991 + n);
                SkipList sl(trial_rng);
                std::vector<int> elements(n);
                for (int i = 0; i < n; i++) {
                    elements[i] = i + 1;
                    sl.insert(elements[i]);
                }
                std::uniform_int_distribution<int> dist(0, n - 1);
                int searches = 100;
                for (int s = 0; s < searches; s++) {
                    int idx = dist(trial_rng);
                    sl.search(elements[idx]);
                }
                total_traversals += sl.max_level();
            }
            double avg_level = total_traversals / trials;
            double theoretical = std::log2(n);
            std::cout << "  n=" << std::setw(5) << n
                      << "  avg_max_level=" << std::fixed << std::setprecision(1) << std::setw(5) << avg_level
                      << "  lg(n)=" << std::fixed << std::setprecision(1) << std::setw(5) << theoretical << "\n";
        }
    }
}

} // namespace chapter9
