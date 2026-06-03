// WAREHOUSE LAYOUT AND NAVIGATION MODULE
// Task 5

#ifndef WAREHOUSE_TREE_HPP
#define WAREHOUSE_TREE_HPP

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <fstream>

// Step is defined here because Navigation.hpp needs it
struct Step {
    int  stepNumber;
    char direction[15];  // "Forward", "Left", "Right", "Backward"
    char location[50];
};

struct TreeNode {
    char      name[50];
    char      type[15];        // "Root", "Zone", "Aisle", "Shelf"
    TreeNode* children[10];
    int       childCount;
};

class WarehouseTree {
private:
    static const int MAX_NODES = 100;

    TreeNode* root;
    TreeNode* nodeTable[MAX_NODES];
    int       nodeIDs[MAX_NODES];
    int       parentIDs[MAX_NODES];
    int       tableSize;

    TreeNode* createNode(const char* name, const char* type) {
        TreeNode* node = new TreeNode();
        strncpy(node->name, name, 49);
        node->name[49] = '\0';
        strncpy(node->type, type, 14);
        node->type[14] = '\0';
        node->childCount = 0;
        for (int i = 0; i < 10; i++) node->children[i] = nullptr;
        return node;
    }

    void addChild(TreeNode* parent, TreeNode* child) {
        if (parent->childCount < 10)
            parent->children[parent->childCount++] = child;
    }

    // recursive DFS - fills pathNodes when the target shelf is found
    // zoneFilter makes sure we only match shelves inside the correct zone
    // (needed when two zones have a shelf with the same name)
    bool tracePath(TreeNode* current, const char* target,
                   const char* zoneFilter,
                   TreeNode* pathNodes[], int& pathLen) {
        if (current == nullptr) return false;
        pathNodes[pathLen++] = current;
        if (strcmp(current->name, target) == 0) {
            if (zoneFilter != nullptr && zoneFilter[0] != '\0') {
                bool inZone = false;
                for (int i = 0; i < pathLen; i++) {
                    if (strcmp(pathNodes[i]->type, "Zone") == 0 &&
                        strcmp(pathNodes[i]->name, zoneFilter) == 0) {
                        inZone = true;
                        break;
                    }
                }
                if (!inZone) { pathLen--; return false; }
            }
            return true;
        }
        for (int i = 0; i < current->childCount; i++) {
            if (tracePath(current->children[i], target, zoneFilter, pathNodes, pathLen))
                return true;
        }
        pathLen--;  // backtrack
        return false;
    }

    // free tree memory post-order (children before parent)
    void freeTree(TreeNode* node) {
        if (node == nullptr) return;
        for (int i = 0; i < node->childCount; i++)
            freeTree(node->children[i]);
        delete node;
    }

    void parseLine(char* line, char fields[][100], int& fieldCount) {
        fieldCount = 0;
        int ci = 0;
        for (int i = 0; ; i++) {
            char c = line[i];
            if (c == ',' || c == '\0' || c == '\n' || c == '\r') {
                fields[fieldCount][ci] = '\0';
                fieldCount++;
                ci = 0;
                if (c == '\0' || c == '\n' || c == '\r') break;
                if (fieldCount >= 10) break;
            } else {
                fields[fieldCount][ci++] = c;
            }
        }
    }

public:
    WarehouseTree() : root(nullptr), tableSize(0) {
        for (int i = 0; i < MAX_NODES; i++) {
            nodeTable[i] = nullptr;
            nodeIDs[i]   = 0;
            parentIDs[i] = 0;
        }
    }

    ~WarehouseTree() {
        freeTree(root);
    }

    TreeNode* getRoot() { return root; }

    void displayLayout(TreeNode* node, int depth) {
        if (node == nullptr) {
            if (depth == 0) printf("Warehouse layout not loaded.\n");
            return;
        }
        for (int i = 0; i < depth * 4; i++) printf(" ");
        printf("[%s] %s\n", node->type, node->name);
        for (int i = 0; i < node->childCount; i++)
            displayLayout(node->children[i], depth + 1);
    }

    // runs DFS from root to find destinationName and converts nodes into Steps
    Step* findPath(const char* startName, const char* destinationName,
                   int& stepCount) {
        stepCount = 0;
        if (root == nullptr) {
            printf("Warehouse layout not loaded.\n");
            return nullptr;
        }

        static TreeNode* pathNodes[50];
        int pathLen = 0;
        bool found = tracePath(root, destinationName, startName, pathNodes, pathLen);

        if (!found) {
            printf("No path found to destination: %s\n", destinationName);
            return nullptr;
        }

        static Step steps[50];
        int aisleCount = 0;

        for (int i = 0; i < pathLen; i++) {
            steps[stepCount].stepNumber = stepCount + 1;
            strncpy(steps[stepCount].location, pathNodes[i]->name, 49);
            steps[stepCount].location[49] = '\0';

            if (strcmp(pathNodes[i]->type, "Aisle") == 0) {
                // alternate left and right for each aisle
                if (aisleCount % 2 == 0)
                    strncpy(steps[stepCount].direction, "Left", 14);
                else
                    strncpy(steps[stepCount].direction, "Right", 14);
                aisleCount++;
            } else {
                strncpy(steps[stepCount].direction, "Forward", 14);
            }
            steps[stepCount].direction[14] = '\0';
            stepCount++;
        }

        return steps;
    }

    void loadFromCSV(const char* filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            printf("Error: warehouse.csv not found\n");
            return;
        }

        char line[256];
        file.getline(line, 256);  // skip header

        tableSize = 0;
        while (file.getline(line, 256) && tableSize < MAX_NODES) {
            int len = strlen(line);
            if (len > 0 && line[len - 1] == '\r') line[len - 1] = '\0';
            if (strlen(line) == 0) continue;

            char fields[6][100];
            int fc = 0;
            parseLine(line, fields, fc);
            if (fc < 4) continue;

            int nodeID   = atoi(fields[0]);
            int parentID = atoi(fields[3]);
            char nodeName[50]; strncpy(nodeName, fields[1], 49); nodeName[49] = '\0';
            char nodeType[15]; strncpy(nodeType, fields[2], 14); nodeType[14] = '\0';

            nodeTable[tableSize]  = createNode(nodeName, nodeType);
            nodeIDs[tableSize]    = nodeID;
            parentIDs[tableSize]  = parentID;
            tableSize++;
        }
        file.close();

        // second pass: link each node to its parent
        for (int i = 0; i < tableSize; i++) {
            if (parentIDs[i] == 0) {
                root = nodeTable[i];  // node with parentID 0 is the root
                continue;
            }
            bool parentFound = false;
            for (int j = 0; j < tableSize; j++) {
                if (nodeIDs[j] == parentIDs[i]) {
                    addChild(nodeTable[j], nodeTable[i]);
                    parentFound = true;
                    break;
                }
            }
            if (!parentFound) {
                printf("Warning: parent %d not found for node %d, skipping.\n",
                       parentIDs[i], nodeIDs[i]);
            }
        }

        printf("Warehouse layout loaded successfully.\n");
    }
};

#endif // WAREHOUSE_TREE_HPP
