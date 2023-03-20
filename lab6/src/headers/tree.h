#ifndef _TREE_H
#define _TREE_H

#include <vector>

using namespace std;

class treeNode {
private:
    treeNode *left;
    treeNode *right;
    int value;

public:
    explicit treeNode(int value) : left(nullptr), right(nullptr), value(value) {}

    int &getValue() {
        return value;
    }

    treeNode *&getLeft() {
        return left;
    }

    treeNode *&getRight() {
        return right;
    }

    bool operator<(treeNode &rhs) const {
        return this->value < rhs.getValue();
    }

    bool operator==(treeNode &rhs) const {
        return this->value == rhs.getValue();
    }
};

class Tree {
private:
    treeNode *root;

    void deleteTree(treeNode *&current) {
        if (!current) { return; }
        deleteTree(current->getLeft());
        deleteTree(current->getRight());
        delete current;
        current = nullptr;
    }

    static void print(treeNode *&current, int h = 0) {
        if (!current) { return; }
        print(current->getRight(), h + 2);
        for (int i = 0; i < h; i++) {
            cout << "-";
        }
        cout << current->getValue() << endl;
        print(current->getLeft(), h + 2);
    }

    bool find(treeNode *current, int value) {
        if (!current) { return false; }
        if (value < current->getValue()) {
            return find(current->getLeft(), value);
        } else if (value > current->getValue()) {
            return find(current->getRight(), value);
        } else {
            return true;
        }
    }

    void insert(treeNode *&current, int value) {
        if (!current) {
            current = new treeNode(value);
        } else if (value < current->getValue()) {
            insert(current->getLeft(), value);
        } else {
            insert(current->getRight(), value);
        }
    }

    void remove(treeNode *&current, int value) {
        if (!current) { return; }
        if (value < current->getValue()) {
            remove(current->getLeft(), value);
        } else if (value > current->getValue()) {
            remove(current->getRight(), value);
        } else {
            deleteTree(current);
        }
    }

    int getParent(treeNode *&current, int value) {
        if (value < current->getValue()) {
            if (!current->getLeft()) {
                return current->getValue();
            }
            return getParent(current->getLeft(), value);
        } else if (value > current->getValue()) {
            if (!current->getRight()) {
                return current->getValue();
            }
            return getParent(current->getRight(), value);
        }
        return -1;
    }

    void getAll(treeNode *current, vector<int> &tmp) {
        if (!current) { return; }
        getAll(current->getLeft(), tmp);
        tmp.push_back(current->getValue());
        getAll(current->getRight(), tmp);
    }

public:
    Tree() : root(nullptr) {};

    void insert(int value) {
        insert(root, value);
    }

    bool find(int value) {
        return find(root, value);
    }

    void print() {
        print(root);
    }

    void remove(int value) {
        remove(root, value);
    }

    int getPlace(int value) {
        return getParent(root, value);
    }

    vector<int> getElements(){
        vector<int> tmp;
        getAll(root, tmp);
        return tmp;
    }

    ~Tree() {
        deleteTree(root);
    }
};

#endif