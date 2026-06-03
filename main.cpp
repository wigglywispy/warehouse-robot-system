// Warehouse Robot Navigation System
<<<<<<< Updated upstream
// CT077-3-2-DSTR (Data Structures)
// Members: Member 1, Member 2, Member 3, Member 4, Member 5
// ============================================================
// RENAME THIS FILE to: <GroupNo>_<leaderID>_<member1ID>_<member2ID>.cpp
// before submitting (e.g. G1_TP012345_TP012344_TP012123.cpp)
// ============================================================
=======
// All module intergration

#include "ItemBST.hpp"
#include "OrderQueue.hpp"
#include "RobotQueue.hpp"
#include "Navigation.hpp"
>>>>>>> Stashed changes

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <fstream>

// ============================================================
// UTILITY
// Shared CSV parser used by all modules.
// maxFields caps writes so callers never overflow their array.
// ============================================================

static void parseLine(char* line, char fields[][100], int maxFields, int& fieldCount) {
    fieldCount = 0;
    int ci = 0;
    for (int i = 0; ; i++) {
        char c = line[i];
        if (c == ',' || c == '\0' || c == '\n' || c == '\r') {
            fields[fieldCount][ci] = '\0';
            fieldCount++;
            ci = 0;
            if (c == '\0' || c == '\n' || c == '\r') break;
            if (fieldCount >= maxFields) break;
        } else {
            // guard against field characters overflowing the 100-char slot
            if (ci < 99) fields[fieldCount][ci++] = c;
        }
    }
}

// ============================================================
// MODULE: Warehouse Layout
// Member 5
// General tree (N-ary): Root -> Zones -> Aisles -> Shelves
// DFS is used to find a path from root to the target shelf.
// No STL used.
// ============================================================

// Step is defined here because NavigationStack needs it
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
            parseLine(line, fields, 6, fc);
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

// ============================================================
// MODULE: Item Search and Management
// Member 4
// Binary Search Tree ordered by itemID.
// Supports insert, search, delete, and inorder display.
// No STL used.
// ============================================================

struct Item {
    char itemID[10];
    char itemName[30];
    char zone[10];
    char aisle[10];
    char shelf[10];
};

struct BSTNode {
    Item     data;
    BSTNode* left;
    BSTNode* right;
};

class ItemBST {
private:
    BSTNode* root;

    BSTNode* insertHelper(BSTNode* node, Item item) {
        if (node == nullptr) {
            BSTNode* newNode = new BSTNode();
            newNode->data  = item;
            newNode->left  = nullptr;
            newNode->right = nullptr;
            return newNode;
        }
        int cmp = strcmp(item.itemID, node->data.itemID);
        if      (cmp < 0) node->left  = insertHelper(node->left,  item);
        else if (cmp > 0) node->right = insertHelper(node->right, item);
        else              printf("Item %s already exists.\n", item.itemID);
        return node;
    }

    // find the smallest node in a subtree (used in delete)
    BSTNode* findMin(BSTNode* node) {
        while (node->left != nullptr) node = node->left;
        return node;
    }

    BSTNode* deleteHelper(BSTNode* node, char* itemID) {
        if (node == nullptr) return nullptr;
        int cmp = strcmp(itemID, node->data.itemID);
        if (cmp < 0) {
            node->left = deleteHelper(node->left, itemID);
        } else if (cmp > 0) {
            node->right = deleteHelper(node->right, itemID);
        } else {
            // Case 1: no children
            if (node->left == nullptr && node->right == nullptr) {
                delete node;
                return nullptr;
            }
            // Case 2: one child
            if (node->left == nullptr) {
                BSTNode* temp = node->right;
                delete node;
                return temp;
            }
            if (node->right == nullptr) {
                BSTNode* temp = node->left;
                delete node;
                return temp;
            }
            // Case 3: two children - replace with inorder successor
            BSTNode* successor = findMin(node->right);
            node->data = successor->data;
            node->right = deleteHelper(node->right, successor->data.itemID);
        }
        return node;
    }

    void inOrderHelper(BSTNode* node) {
        if (node == nullptr) return;
        inOrderHelper(node->left);
        printf("  %-10s %-30s %-10s %-10s %-10s\n",
               node->data.itemID, node->data.itemName,
               node->data.zone,   node->data.aisle, node->data.shelf);
        inOrderHelper(node->right);
    }

    void preOrderHelper(BSTNode* node) {
        if (node == nullptr) return;
        printf("  %-10s %-30s %-10s %-10s %-10s\n",
               node->data.itemID, node->data.itemName,
               node->data.zone,   node->data.aisle, node->data.shelf);
        preOrderHelper(node->left);
        preOrderHelper(node->right);
    }

    Item* searchHelper(BSTNode* node, char* itemID) {
        if (node == nullptr) return nullptr;
        int cmp = strcmp(itemID, node->data.itemID);
        if (cmp == 0) return &node->data;
        if (cmp < 0)  return searchHelper(node->left,  itemID);
        return              searchHelper(node->right, itemID);
    }

    // BST is ordered by ID so name search has to scan all nodes
    Item* searchByNameHelper(BSTNode* node, char* itemName) {
        if (node == nullptr) return nullptr;
        Item* leftResult = searchByNameHelper(node->left, itemName);
        if (leftResult != nullptr) return leftResult;
        if (strcmp(node->data.itemName, itemName) == 0) return &node->data;
        return searchByNameHelper(node->right, itemName);
    }

    // free children before parent (post-order)
    void freeTree(BSTNode* node) {
        if (node == nullptr) return;
        freeTree(node->left);
        freeTree(node->right);
        delete node;
    }

public:
    ItemBST() : root(nullptr) {}

    ~ItemBST() {
        freeTree(root);
    }

    void insert(Item item) {
        root = insertHelper(root, item);
    }

    Item* search(char* itemID) {
        Item* result = searchHelper(root, itemID);
        if (result == nullptr)
            printf("Item not found: %s\n", itemID);
        else
            printf("Found: %s - %s (Zone: %s, Aisle: %s, Shelf: %s)\n",
                   result->itemID, result->itemName,
                   result->zone, result->aisle, result->shelf);
        return result;
    }

    void deleteItem(char* itemID) {
        if (root == nullptr) {
            printf("Tree is empty.\n");
            return;
        }
        root = deleteHelper(root, itemID);
    }

    void displayInOrder() {
        printf("\n--- All Items (Sorted by ItemID) ---\n");
        printf("  %-10s %-30s %-10s %-10s %-10s\n",
               "ItemID", "Item Name", "Zone", "Aisle", "Shelf");
        printf("  %-70s\n",
               "----------------------------------------------------------------------");
        inOrderHelper(root);
        printf("\n");
    }

    void displayPreOrder() {
        printf("\n--- All Items (Pre-Order) ---\n");
        preOrderHelper(root);
        printf("\n");
    }

    Item* searchByName(char* itemName) {
        Item* result = searchByNameHelper(root, itemName);
        if (result == nullptr)
            printf("Item not found by name: %s\n", itemName);
        else
            printf("Found: %s - %s (Zone: %s, Aisle: %s, Shelf: %s)\n",
                   result->itemID, result->itemName,
                   result->zone, result->aisle, result->shelf);
        return result;
    }

    void loadFromCSV(const char* filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            printf("Error: items.csv not found\n");
            return;
        }

        char line[256];
        file.getline(line, 256);  // skip header

        while (file.getline(line, 256)) {
            int len = strlen(line);
            if (len > 0 && line[len - 1] == '\r') line[len - 1] = '\0';
            if (strlen(line) == 0) continue;

            char fields[6][100];
            int fc = 0;
            parseLine(line, fields, 6, fc);
            if (fc < 5) continue;

            Item item;
            strncpy(item.itemID,   fields[0], 9);  item.itemID[9]    = '\0';
            strncpy(item.itemName, fields[1], 29); item.itemName[29] = '\0';
            strncpy(item.zone,     fields[2], 9);  item.zone[9]      = '\0';
            strncpy(item.aisle,    fields[3], 9);  item.aisle[9]     = '\0';
            strncpy(item.shelf,    fields[4], 9);  item.shelf[9]     = '\0';

            insert(item);
        }
        file.close();
        printf("Items loaded successfully.\n");
    }
};

// ============================================================
// MODULE: Order Management
// Member 1
// Queue implemented using a singly linked list (FIFO).
// Each order is stored in a heap-allocated OrderNode.
// Dequeued nodes move to processedHead so markCompleted
// can still find and update them after removal from queue.
// No STL used.
// ============================================================

struct Order {
    char orderID[10];
    char itemName[30];
    char destination[50];
    char status[15];   // "Pending" or "Completed"
};

// linked list node for the order queue
struct OrderNode {
    Order      data;
    OrderNode* next;
};

class OrderQueue {
private:
    static const int MAX_SIZE = 50;  // soft upper bound on pending orders

    OrderNode* front;          // head of the pending queue
    OrderNode* rear;           // tail of the pending queue
    OrderNode* processedHead;  // head of the dequeued/completed list
    int        count;          // number of currently pending orders

public:
    OrderQueue() : front(nullptr), rear(nullptr), processedHead(nullptr), count(0) {}

    ~OrderQueue() {
        // free all pending nodes
        while (front != nullptr) {
            OrderNode* temp = front;
            front = front->next;
            delete temp;
        }
        // free all processed nodes
        while (processedHead != nullptr) {
            OrderNode* temp = processedHead;
            processedHead = processedHead->next;
            delete temp;
        }
    }

    bool isEmpty() { return front == nullptr; }
    bool isFull()  { return count >= MAX_SIZE; }

    void enqueue(Order o) {
        if (isFull()) {
            printf("Order queue is full.\n");
            return;
        }
        OrderNode* newNode = new OrderNode();
        newNode->data = o;
        newNode->next = nullptr;
        if (rear == nullptr) {
            front = rear = newNode;
        } else {
            rear->next = newNode;
            rear = newNode;
        }
        count++;
    }

    // remove from front; node moves to processedHead for markCompleted tracking
    Order dequeue() {
        if (isEmpty()) {
            printf("No pending orders.\n");
            Order empty;
            empty.orderID[0]     = '\0';
            empty.itemName[0]    = '\0';
            empty.destination[0] = '\0';
            empty.status[0]      = '\0';
            return empty;
        }
        OrderNode* temp = front;
        front = front->next;
        if (front == nullptr) rear = nullptr;
        count--;

        Order result = temp->data;
        // prepend to processedHead so markCompleted can locate the order later
        temp->next    = processedHead;
        processedHead = temp;
        return result;
    }

    // show orders still waiting in the pending queue
    void displayPending() {
        printf("\n--- Pending Orders ---\n");
        bool found = false;
        for (OrderNode* cur = front; cur != nullptr; cur = cur->next) {
            if (strcmp(cur->data.status, "Pending") == 0) {
                printf("  %-10s | %-20s | %-35s | %s\n",
                       cur->data.orderID, cur->data.itemName,
                       cur->data.destination, cur->data.status);
                found = true;
            }
        }
        if (!found) printf("  No pending orders.\n");
        printf("\n");
    }

    // scan the processed list for orders marked Completed
    void displayCompleted() {
        printf("\n--- Completed Orders ---\n");
        bool found = false;
        for (OrderNode* cur = processedHead; cur != nullptr; cur = cur->next) {
            if (strcmp(cur->data.status, "Completed") == 0) {
                printf("  %-10s | %-20s | %-35s | %s\n",
                       cur->data.orderID, cur->data.itemName,
                       cur->data.destination, cur->data.status);
                found = true;
            }
        }
        if (!found) printf("  No completed orders.\n");
        printf("\n");
    }

    void markCompleted(char* orderID) {
        for (OrderNode* cur = processedHead; cur != nullptr; cur = cur->next) {
            if (strcmp(cur->data.orderID, orderID) == 0) {
                strncpy(cur->data.status, "Completed", 14);
                cur->data.status[14] = '\0';
                printf("Order %s marked as Completed.\n", orderID);
                return;
            }
        }
        printf("Order not found: %s\n", orderID);
    }

    void loadFromCSV(const char* filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            printf("Error: orders.csv not found\n");
            return;
        }

        char line[256];
        file.getline(line, 256);  // skip header

        while (file.getline(line, 256)) {
            int len = strlen(line);
            if (len > 0 && line[len - 1] == '\r') line[len - 1] = '\0';
            if (strlen(line) == 0) continue;

            char fields[5][100];
            int fc = 0;
            parseLine(line, fields, 5, fc);
            if (fc < 4) continue;

            Order o;
            strncpy(o.orderID,     fields[0], 9);  o.orderID[9]      = '\0';
            strncpy(o.itemName,    fields[1], 29); o.itemName[29]    = '\0';
            strncpy(o.destination, fields[2], 49); o.destination[49] = '\0';
            strncpy(o.status,      fields[3], 14); o.status[14]      = '\0';

            // completed orders from CSV go directly to processedHead
            if (strcmp(o.status, "Completed") == 0) {
                OrderNode* newNode = new OrderNode();
                newNode->data  = o;
                newNode->next  = processedHead;
                processedHead  = newNode;
            } else {
                enqueue(o);
            }
        }
        file.close();
        printf("Orders loaded successfully.\n");
    }
};

// ============================================================
// MODULE: Robot Assignment
// Member 2
// Circular Queue (array-based) for fair robot rotation.
// Robots rotate so the same one is not always picked first.
// No STL used.
// ============================================================

struct Robot {
    char robotID[10];
    char status[15];      // "Available", "Busy", "Maintenance"
    char currentTask[10];
};

class CircularQueue {
private:
    static const int MAX_SIZE = 10;
    Robot robots[MAX_SIZE];
    int   front;
    int   rear;
    int   count;
    char  lastAssignedRobotID[10];

    // find the index of the next available robot, returns -1 if none found
    int findNextAvailableIndex() {
        int busyCount  = 0;
        int maintCount = 0;

        for (int i = 0; i < count; i++) {
            int idx = (front + i) % MAX_SIZE;

            if (i > 0 && (front + i) % MAX_SIZE == 0) {
                printf("[Circular Queue] Wrap-around: search resumed from index 0.\n");
            }

            if (strcmp(robots[idx].status, "Available") == 0) {
                return idx;
            }
            if (strcmp(robots[idx].status, "Busy") == 0)        busyCount++;
            if (strcmp(robots[idx].status, "Maintenance") == 0) maintCount++;
        }

        // print reason why no robot is free
        if (busyCount == count)
            printf("All robots are currently busy.\n");
        else if (maintCount == count)
            printf("All robots are under maintenance.\n");
        else
            printf("No robots available.\n");

        return -1;
    }

public:
    CircularQueue() : front(0), rear(-1), count(0) {
        lastAssignedRobotID[0] = '\0';
    }

    bool isEmpty() { return count == 0; }
    bool isFull()  { return count == MAX_SIZE; }

    void enqueue(Robot r) {
        if (isFull()) {
            printf("Robot queue is full.\n");
            return;
        }
        rear = (rear + 1) % MAX_SIZE;  // wrap around using modulo
        if (rear == 0 && count > 0) {
            printf("[Circular Queue] Rear wrapped around to index 0.\n");
        }
        robots[rear] = r;
        count++;
    }

    // returns a copy of the next available robot without removing it
    Robot getNextAvailable() {
        int idx = findNextAvailableIndex();
        if (idx == -1) {
            Robot empty;
            empty.robotID[0]     = '\0';
            empty.status[0]      = '\0';
            empty.currentTask[0] = '\0';
            return empty;
        }
        return robots[idx];
    }

    void assignTask(char* orderID) {
        int idx = findNextAvailableIndex();
        if (idx == -1) {
            lastAssignedRobotID[0] = '\0';
            return;
        }
        strncpy(robots[idx].status, "Busy", 14);
        robots[idx].status[14] = '\0';
        strncpy(robots[idx].currentTask, orderID, 9);
        robots[idx].currentTask[9] = '\0';
        strncpy(lastAssignedRobotID, robots[idx].robotID, 9);
        lastAssignedRobotID[9] = '\0';
        printf("Robot %s assigned to order %s.\n",
               robots[idx].robotID, orderID);
    }

    // getter for the last robot that was assigned a task
    const char* getLastAssignedRobotID() {
        return lastAssignedRobotID;
    }

    void releaseRobot(char* robotID) {
        for (int i = 0; i < count; i++) {
            int idx = (front + i) % MAX_SIZE;
            if (strcmp(robots[idx].robotID, robotID) == 0) {
                strncpy(robots[idx].status, "Available", 14);
                robots[idx].status[14]     = '\0';
                robots[idx].currentTask[0] = '\0';
                printf("Robot %s is now Available.\n", robotID);
                return;
            }
        }
        printf("Robot not found: %s\n", robotID);
    }

    void displayAllRobots() {
        printf("\n--- All Robots Status ---\n");
        printf("  %-10s %-15s %-15s\n", "RobotID", "Status", "Current Task");
        printf("  -----------------------------------------\n");
        for (int i = 0; i < count; i++) {
            int idx = (front + i) % MAX_SIZE;
            const char* task = (robots[idx].currentTask[0] != '\0')
                               ? robots[idx].currentTask : "(none)";
            printf("  %-10s %-15s %-15s\n",
                   robots[idx].robotID, robots[idx].status, task);
        }
        printf("\n");
    }

    void loadFromCSV(const char* filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            printf("Error: robots.csv not found\n");
            return;
        }

        char line[256];
        file.getline(line, 256);  // skip header

        while (file.getline(line, 256)) {
            int len = strlen(line);
            if (len > 0 && line[len - 1] == '\r') line[len - 1] = '\0';
            if (strlen(line) == 0) continue;

            char fields[4][100];
            int fc = 0;
            parseLine(line, fields, 4, fc);
            if (fc < 2) continue;

            Robot r;
            strncpy(r.robotID, fields[0], 9);  r.robotID[9]  = '\0';
            strncpy(r.status,  fields[1], 14); r.status[14]  = '\0';
            if (fc >= 3 && strlen(fields[2]) > 0) {
                strncpy(r.currentTask, fields[2], 9);
                r.currentTask[9] = '\0';
            } else {
                r.currentTask[0] = '\0';
            }

            enqueue(r);
        }
        file.close();
        printf("Robots loaded successfully.\n");
    }
};

// ============================================================
// MODULE: Robot Navigation & Path Tracking
// Member 3
// Stack implemented using a singly linked list (LIFO).
// Each push() allocates a new StackNode on the heap;
// each pop() frees it. forwardPath[] is kept as a fixed-size
// copy purely for CSV logging after the stack is cleared.
// No STL used.
// ============================================================

// linked list node for the navigation stack
struct StackNode {
    Step       data;
    StackNode* next;
};

class NavigationStack {
private:
    static const int MAX_PATH = 20;

    StackNode* top;
    Step       forwardPath[MAX_PATH];  // copy of path for CSV logging after return
    int        forwardCount;

public:
    NavigationStack() : top(nullptr), forwardCount(0) {}

    ~NavigationStack() {
        clearStack();
    }

    bool isEmpty() { return top == nullptr; }

    void push(Step s) {
        StackNode* newNode = new StackNode();
        newNode->data = s;
        newNode->next = top;
        top = newNode;
    }

    Step pop() {
        if (isEmpty()) {
            printf("No steps to retrace.\n");
            Step empty;
            empty.stepNumber   = -1;
            empty.direction[0] = '\0';
            empty.location[0]  = '\0';
            return empty;
        }
        StackNode* temp = top;
        Step s = temp->data;
        top = top->next;
        delete temp;
        return s;
    }

    Step peek() {
        if (isEmpty()) {
            Step empty;
            empty.stepNumber   = -1;
            empty.direction[0] = '\0';
            empty.location[0]  = '\0';
            return empty;
        }
        return top->data;
    }

    // print forward path using the saved copy (step 1 first)
    void displayForwardPath() {
        if (forwardCount == 0) {
            printf("Navigation stack is empty.\n");
            return;
        }
        printf("\n--- FORWARD NAVIGATION PATH ---\n");
        for (int i = 0; i < forwardCount; i++) {
            printf("  Step %d: %-10s -> %s\n",
                   forwardPath[i].stepNumber,
                   forwardPath[i].direction,
                   forwardPath[i].location);
        }
        printf("--------------------------------\n\n");
    }

    // pop steps and print reverse direction for each one
    void returnToStart() {
        if (isEmpty()) {
            printf("No steps to retrace.\n");
            return;
        }
        printf("\n--- RETURN PATH ---\n");
        while (!isEmpty()) {
            Step s = pop();
            char reverseDir[20];
            if      (strcmp(s.direction, "Forward")  == 0) strcpy(reverseDir, "Move Backward");
            else if (strcmp(s.direction, "Left")     == 0) strcpy(reverseDir, "Turn Right");
            else if (strcmp(s.direction, "Right")    == 0) strcpy(reverseDir, "Turn Left");
            else if (strcmp(s.direction, "Backward") == 0) strcpy(reverseDir, "Move Forward");
            else                                            strcpy(reverseDir, "Move Backward");
            printf("  %s at: %s\n", reverseDir, s.location);
        }
        printf("-------------------\n\n");
    }

    void clearStack() {
        while (top != nullptr) {
            StackNode* temp = top;
            top = top->next;
            delete temp;
        }
        forwardCount = 0;
    }

    // expose saved forward path so main can log it after returnToStart()
    const Step* getForwardPath(int& count) {
        count = forwardCount;
        return forwardPath;
    }

    // query warehouse tree for path, then push each step onto the stack
    void loadPathFromWarehouse(WarehouseTree& tree,
                               const char* startZone,
                               const char* destination) {
        clearStack();
        forwardCount = 0;

        int stepCount = 0;
        Step* pathSteps = tree.findPath(startZone, destination, stepCount);

        if (pathSteps == nullptr || stepCount == 0) {
            printf("No path found to destination: %s\n", destination);
            return;
        }

        int limit = (stepCount < MAX_PATH) ? stepCount : MAX_PATH;
        for (int i = 0; i < limit; i++) {
            push(pathSteps[i]);
            forwardPath[i] = pathSteps[i];
        }
        forwardCount = limit;
    }
};

// ============================================================
// MAIN - Integrates all modules
// ============================================================

static void printMenu() {
    printf("\n");
    printf("========================================\n");
    printf(" WAREHOUSE ROBOT NAVIGATION SYSTEM\n");
    printf("========================================\n");
    printf("[1]  Add New Order\n");
    printf("[2]  Process Next Order\n");
    printf("[3]  Complete Current Order (Robot Returns)\n");
    printf("[4]  Display Pending Orders\n");
    printf("[5]  Display Completed Orders\n");
    printf("[6]  Display All Robots Status\n");
    printf("[7]  Search Item by ID\n");
    printf("[8]  Search Item by Name\n");
    printf("[9]  Display Warehouse Layout\n");
    printf("[10] Display All Items (Inorder)\n");
    printf("[11] Exit\n");
    printf("========================================\n");
    printf("Enter choice: ");
}

// write completed journey steps to navigation.csv
static void logJourney(const char* journeyID, const char* robotID,
                        const char* orderID,
                        const Step* steps, int stepCount) {
    std::ofstream navLog("navigation.csv", std::ios::app);
    if (!navLog.is_open()) {
        printf("Warning: could not open navigation.csv for logging.\n");
        return;
    }
    for (int i = 0; i < stepCount; i++) {
        navLog << journeyID   << ","
               << robotID     << ","
               << orderID     << ","
               << steps[i].stepNumber << ","
               << steps[i].direction  << ","
               << steps[i].location   << "\n";
    }
    navLog.close();
}

int main() {
    WarehouseTree    warehouseTree;
    ItemBST          itemBST;
    OrderQueue       orderQueue;
    CircularQueue    robotQueue;
    NavigationStack  navigationStack;

    // load all data before showing the menu
    printf("=== Loading System Data ===\n");
    warehouseTree.loadFromCSV("warehouse.csv");
    itemBST.loadFromCSV("items.csv");
    orderQueue.loadFromCSV("orders.csv");
    robotQueue.loadFromCSV("robots.csv");
    printf("===========================\n");

    char assignedRobotID[10] = "";
    char currentOrderID[10]  = "";
    bool taskInProgress      = false;

    // count existing journeys in the log so IDs continue from where we left off
    int journeyCount = 0;
    {
        std::ifstream navLog("navigation.csv");
        if (navLog.is_open()) {
            char line[256];
            navLog.getline(line, 256);  // skip header
            while (navLog.getline(line, 256)) {
                if (line[0] == 'J') {
                    int n = atoi(line + 1);
                    if (n > journeyCount) journeyCount = n;
                }
            }
            navLog.close();
        }
    }

    int choice = 0;
    do {
        printMenu();
        scanf("%d", &choice);

        switch (choice) {

            case 1: {
                // add a new order to the queue
                Order newOrder;
                printf("Enter Order ID   : ");
                scanf("%9s", newOrder.orderID);
                printf("Enter Item Name  : ");
                scanf(" %29[^\n]", newOrder.itemName);
                printf("Enter Destination: ");
                scanf(" %49[^\n]", newOrder.destination);
                strncpy(newOrder.status, "Pending", 14);
                newOrder.status[14] = '\0';
                orderQueue.enqueue(newOrder);
                printf("Order %s added to queue.\n", newOrder.orderID);
                break;
            }

            case 2: {
                // dequeue next order, find item, assign robot, plan path
                if (taskInProgress) {
                    printf("A task is already in progress. Complete it first.\n");
                    break;
                }

                // check robot availability before removing order from queue
                Robot nextRobot = robotQueue.getNextAvailable();
                if (strlen(nextRobot.robotID) == 0) {
                    printf("No robot available. Order not removed from queue.\n");
                    break;
                }

                Order currentOrder = orderQueue.dequeue();
                if (strlen(currentOrder.orderID) == 0) {
                    break;
                }

                printf("\n--- Processing Order ---\n");
                printf("  Order ID   : %s\n", currentOrder.orderID);
                printf("  Item       : %s\n", currentOrder.itemName);
                printf("  Destination: %s\n", currentOrder.destination);

                // search for item in BST to get its exact location
                Item* foundItem = itemBST.searchByName(currentOrder.itemName);
                if (foundItem == nullptr) {
                    printf("Item not in warehouse inventory. Order cannot be processed.\n");
                    break;
                }
                printf("  Located at : Zone=%s | Aisle=%s | Shelf=%s\n",
                       foundItem->zone, foundItem->aisle, foundItem->shelf);

                char blockedLocation[50] = "";  // No input prevention
                printf("Enter obstacle location to simulate (NONE for no obstacle): ");
                scanf(" %49[^\n]", blockedLocation);
                if (strcmp(blockedLocation, "NONE") == 0 ||
                    strcmp(blockedLocation, "none") == 0 ||
                    strcmp(blockedLocation, "None") == 0) {
                    blockedLocation[0] = '\0';
                }

                strncpy(currentOrderID, currentOrder.orderID, 9);
                currentOrderID[9] = '\0';
                robotQueue.assignTask(currentOrderID);
                strncpy(assignedRobotID,
                        robotQueue.getLastAssignedRobotID(), 9);
                assignedRobotID[9] = '\0';

                if (strlen(assignedRobotID) == 0) {
                    printf("No robot available. Order cannot proceed.\n");
                    break;
                }

                // pass zone name so DFS picks the right shelf if duplicate names exist
                navigationStack.loadPathFromWarehouse(
                    warehouseTree, foundItem->zone, foundItem->shelf, blockedLocation);

                if (navigationStack.isEmpty()) {
                    printf("Navigation could not continue for %s. Order cannot proceed.\n",
                           foundItem->shelf);
                    robotQueue.releaseRobot(assignedRobotID);
                    break;
                }

                navigationStack.displayForwardPath();
                taskInProgress = true;
                printf("Robot %s is navigating to %s.\n",
                       assignedRobotID, foundItem->shelf);
                break;
            }

            case 3: {
                // robot retraces path back and order is marked complete
                if (!taskInProgress) {
                    printf("No task is currently in progress.\n");
                    break;
                }

                // save forward path before returnToStart() clears the stack
                int pathCount = 0;
                const Step* forwardSteps =
                    navigationStack.getForwardPath(pathCount);

                navigationStack.returnToStart();
                robotQueue.releaseRobot(assignedRobotID);
                orderQueue.markCompleted(currentOrderID);

                journeyCount++;
                char journeyID[10];
                snprintf(journeyID, sizeof(journeyID), "J%03d", journeyCount);
                logJourney(journeyID, assignedRobotID,
                           currentOrderID, forwardSteps, pathCount);

                navigationStack.clearStack();
                taskInProgress = false;

                printf("Order %s completed. Robot %s has returned.\n",
                       currentOrderID, assignedRobotID);

                assignedRobotID[0] = '\0';
                currentOrderID[0]  = '\0';
                break;
            }

            case 4:
                orderQueue.displayPending();
                break;

            case 5:
                orderQueue.displayCompleted();
                break;

            case 6:
                robotQueue.displayAllRobots();
                break;

            case 7: {
                char itemID[10];
                printf("Enter Item ID: ");
                scanf("%9s", itemID);
                itemBST.search(itemID);
                break;
            }

            case 8: {
                char itemName[30];
                printf("Enter Item Name: ");
                scanf(" %29[^\n]", itemName);
                itemBST.searchByName(itemName);
                break;
            }

            case 9:
                printf("\n--- Warehouse Layout ---\n");
                warehouseTree.displayLayout(warehouseTree.getRoot(), 0);
                break;

            case 10:
                itemBST.displayInOrder();
                break;

            case 11:
                printf("Exiting system. Goodbye!\n");
                break;

            default:
                printf("Invalid choice. Please enter a number between 1 and 11.\n");
        }

    } while (choice != 11);

    return 0;
}
