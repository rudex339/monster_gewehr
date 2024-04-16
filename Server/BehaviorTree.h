#pragma once

#include <iostream>
#include <vector>
#include <functional>

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
        root->print();
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
        std::cout << "선택 노드: " << name << std::endl;
        for (Node* child : children) {
            child->print();
        }
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
        std::cout << "순차 노드: " << name << std::endl;
        for (Node* child : children) {
            child->print();
        }
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

    void add_child(Node* child)  {
        std::cout << "에러: 리프 노드에는 자식 노드를 추가할 수 없습니다." << std::endl;
    }

    void add_children(std::initializer_list<Node*> children)  {
        std::cout << "에러: 리프 노드에는 자식 노드를 추가할 수 없습니다." << std::endl;
    }

    int run() override {
        print();
        return func();
    }

    void print() override {
        std::cout << "리프 노드: " << name << std::endl;
    }

};


