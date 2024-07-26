#pragma once

#include <iostream>
#include <vector>
#include <functional>
#include <random>
#include <string>
#include <chrono>

//#define PrintNode

class Node {
public:
    virtual void reset() = 0;
    virtual int run() = 0;
    virtual void print() = 0;
};

class BehaviorTree {
private:
    Node* root;

public:
    static const int FAIL = -1;
    static const int RUNNING = 0;
    static const int SUCCESS = 1;

    BehaviorTree(Node* root_node) : root(root_node) {}
    BehaviorTree() {}

    void run() {
        root->run();
    }

    void print() {
        //root->print();
    }
};

class Selector : public Node {
public:
    Selector(const std::string& name, std::initializer_list<Node*> nodes)
        : children(nodes), name(name), prev_running_pos(0) {}
    Selector() {}

    void reset() override {
        prev_running_pos = 0;
        for (Node* node : children) {
            node->reset();
        }
    }

    int run() override {
        for (int pos = 0; pos < children.size(); ++pos) {
            int result = children[pos]->run();
            if (BehaviorTree::RUNNING == result) {
                prev_running_pos = pos;
                return BehaviorTree::RUNNING;
            }
            else if (BehaviorTree::SUCCESS == result) {
                prev_running_pos = 0;
                for (size_t i = pos + 1; i < children.size(); ++i) {
                    children[i]->reset();
                }
                return BehaviorTree::SUCCESS;
            }
        }
        prev_running_pos = 0;
        return BehaviorTree::FAIL;
    }

    void print() override {
#ifdef PrintNode
        std::cout << "���� ���: " << name << std::endl;
        for (Node* child : children) {
            child->print();
        }
#endif
    }

private:
    std::vector<Node*> children;
    std::string name;
    int prev_running_pos;
};

class Sequence : public Node {
public:
    Sequence(const std::string& name, std::initializer_list<Node*> nodes)
        : children(nodes), name(name), prev_running_pos(0) {}
    Sequence() {}

    void reset() override {
        prev_running_pos = 0;
        for (Node* node : children) {
            node->reset();
        }
    }

    int run() override {
        for (int pos = prev_running_pos; pos < children.size(); ++pos) {
            int result = children[pos]->run();
            if (BehaviorTree::RUNNING == result) {
                prev_running_pos = pos;
                return BehaviorTree::RUNNING;
            }
            else if (BehaviorTree::FAIL == result) {
                prev_running_pos = 0;
                return BehaviorTree::FAIL;
            }
        }
        prev_running_pos = 0;
        return BehaviorTree::SUCCESS;
    }

    void print() override {
#ifdef PrintNode
        std::cout << "���� ���: " << name << std::endl;
        for (Node* child : children) {
            child->print();
        }
#endif
    }

private:
    std::vector<Node*> children;
    std::string name;
    int prev_running_pos;
};

class Leaf : public Node {
private:
    std::string name;
    std::function<int()> func;
public:
    Leaf(const std::string& name, std::function<int()> func)
        : name(name), func(func) {}
    Leaf() {}

    void reset() override {}

    int run() override {
        print();
        return func();
    }

    void print() override {
#ifdef PrintNode
        std::cout << "���� ���: " << name << std::endl;
#endif
    }
};

// Random Node
class RandomNode : public Node {
private:
    std::vector<Node*> children;
    std::string name;
    std::mt19937 rng{ std::random_device{}() };
    int current_running_index;

public:
    RandomNode(const std::string& name, std::initializer_list<Node*> nodes)
        : children(nodes), name(name), current_running_index(-1) {}
    RandomNode() {}

    void reset() override {
        current_running_index = -1;
        for (Node* node : children) {
            node->reset();
        }
    }

    int run() override {
        if (current_running_index == -1) {
            if (children.empty()) {
                return BehaviorTree::FAIL;
            }
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<int> dist(0, children.size() - 1);
            current_running_index = dist(gen);
        }

        int result = children[current_running_index]->run();

        if (result != BehaviorTree::RUNNING) {
            current_running_index = -1;  // Reset running index if the node is not running
        }

        return result;
    }

    void print() override {
#ifdef PrintNode
        std::cout << "���� ���: " << name << std::endl;
        for (Node* child : children) {
            child->print();
        }
#endif
    }
};

// Decorator base class
class Decorator : public Node {
protected:
    Node* child;
public:
    Decorator(Node* child) : child(child) {}
    Decorator() {}
    void reset() override {
        if (child) child->reset();
    }

    int run() override {
        if (child) return child->run();
        return BehaviorTree::FAIL;
    }

    void print() override {
        if (child) child->print();
    }
};

class Repeater : public Decorator {
private:
    int limit;
    int count;

public:
    Repeater(Node* child, int limit) : Decorator(child), limit(limit), count(0) {}
    Repeater() {}

    void reset() override {
        count = 0;
        Decorator::reset();
    }

    int run() override {
        while (count < limit) {
            int result = child->run();
            if (result == BehaviorTree::RUNNING) {
                return BehaviorTree::RUNNING;
            }
            else if (result == BehaviorTree::FAIL) {
                count = 0;  // ���� �� �ݺ� Ƚ�� �ʱ�ȭ
                return BehaviorTree::FAIL;
            }
            // ����� SUCCESS�� ���, ī��Ʈ ����
            count++;
            if (count >= limit) {
                count = 0;
                return BehaviorTree::SUCCESS;
            }
            // �ݺ� Ƚ���� �ʰ����� �ʾ��� ���, ���� �ݺ��� ���� �ڽ� ��� ����
            child->reset();
        }
        return BehaviorTree::SUCCESS;
    }

    void print() override {
#ifdef PrintNode
        std::cout << "�ݺ��� ���: " << std::endl;
        Decorator::print();
#endif
    }
};

class TimeLimiter : public Decorator {
private:
    std::chrono::seconds delay;  // ���� �ð� (�� ����)
    std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
    bool waiting;  // ���� ��� �������� ���θ� ��Ÿ���� �÷���

public:
    TimeLimiter(Node* child, std::chrono::seconds delay)
        : Decorator(child), delay(delay), waiting(true) {}
    TimeLimiter(Node* child, std::chrono::duration<double> delay)
        : Decorator(child), delay(std::chrono::duration_cast<std::chrono::seconds>(delay)), waiting(true) {}
    TimeLimiter() {}

    void reset() override {
        // �ʱ�ȭ �� ���� �ð��� �����Ͽ� ���� �ð��� ����
        start_time = std::chrono::steady_clock::now();
        waiting = true;  // ��� ���·� ����
        Decorator::reset();
    }

    int run() override {
        auto now = std::chrono::steady_clock::now();
        if (waiting) {
            // ���� �ð��� ����ߴ��� Ȯ��
            if (now - start_time >= delay) {
                waiting = false;  // ��� ���� ����
            }
            else {
                return BehaviorTree::RUNNING;  // ��� ��
            }
        }
        waiting = true;
        start_time = std::chrono::steady_clock::now();
        // ���� �ð��� ������ �ڽ� ��带 ����
        return child->run();
    }

    void print() override {
#ifdef PrintNode
        std::cout << "�ð� ���� ��� (�� ����): " << delay.count() << "�� ����" << std::endl;
        Decorator::print();
#endif
    }
};

// Condition Checker Decorator
class ConditionChecker : public Decorator {
private:
    std::function<bool()> condition;

public:
    ConditionChecker(Node* child, std::function<bool()> condition)
        : Decorator(child), condition(condition) {}
    ConditionChecker() {}

    void reset() override {
        Decorator::reset();
    }

    int run() override {
        if (condition()) {
            return child->run();
        }
        return BehaviorTree::FAIL;
    }

    void print() override {
#ifdef PrintNode
        std::cout << "���� �˻� ���: " << std::endl;
        Decorator::print();
#endif
    }
};