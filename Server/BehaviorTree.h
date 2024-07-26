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
        std::cout << "선택 노드: " << name << std::endl;
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
        std::cout << "순차 노드: " << name << std::endl;
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
        std::cout << "리프 노드: " << name << std::endl;
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
        std::cout << "랜덤 노드: " << name << std::endl;
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
                count = 0;  // 실패 시 반복 횟수 초기화
                return BehaviorTree::FAIL;
            }
            // 결과가 SUCCESS일 경우, 카운트 증가
            count++;
            if (count >= limit) {
                count = 0;
                return BehaviorTree::SUCCESS;
            }
            // 반복 횟수를 초과하지 않았을 경우, 다음 반복을 위해 자식 노드 리셋
            child->reset();
        }
        return BehaviorTree::SUCCESS;
    }

    void print() override {
#ifdef PrintNode
        std::cout << "반복자 노드: " << std::endl;
        Decorator::print();
#endif
    }
};

class TimeLimiter : public Decorator {
private:
    std::chrono::seconds delay;  // 지연 시간 (초 단위)
    std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
    bool waiting;  // 현재 대기 상태인지 여부를 나타내는 플래그

public:
    TimeLimiter(Node* child, std::chrono::seconds delay)
        : Decorator(child), delay(delay), waiting(true) {}
    TimeLimiter(Node* child, std::chrono::duration<double> delay)
        : Decorator(child), delay(std::chrono::duration_cast<std::chrono::seconds>(delay)), waiting(true) {}
    TimeLimiter() {}

    void reset() override {
        // 초기화 시 현재 시간을 저장하여 지연 시간을 시작
        start_time = std::chrono::steady_clock::now();
        waiting = true;  // 대기 상태로 설정
        Decorator::reset();
    }

    int run() override {
        auto now = std::chrono::steady_clock::now();
        if (waiting) {
            // 지연 시간이 경과했는지 확인
            if (now - start_time >= delay) {
                waiting = false;  // 대기 상태 종료
            }
            else {
                return BehaviorTree::RUNNING;  // 대기 중
            }
        }
        waiting = true;
        start_time = std::chrono::steady_clock::now();
        // 지연 시간이 지나면 자식 노드를 실행
        return child->run();
    }

    void print() override {
#ifdef PrintNode
        std::cout << "시간 제한 노드 (초 단위): " << delay.count() << "초 지연" << std::endl;
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
        std::cout << "조건 검사 노드: " << std::endl;
        Decorator::print();
#endif
    }
};