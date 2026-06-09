#ifndef ITEM_BST_HPP
#define ITEM_BST_HPP

#include <cstring>
#include <cstdio>
#include <fstream>

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
            if (node->left == nullptr && node->right == nullptr) {
                delete node;
                return nullptr;
            }
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

    Item* searchByNameHelper(BSTNode* node, char* itemName) {
        if (node == nullptr) return nullptr;
        Item* leftResult = searchByNameHelper(node->left, itemName);
        if (leftResult != nullptr) return leftResult;
        if (strcmp(node->data.itemName, itemName) == 0) return &node->data;
        return searchByNameHelper(node->right, itemName);
    }

    void freeTree(BSTNode* node) {
        if (node == nullptr) return;
        freeTree(node->left);
        freeTree(node->right);
        delete node;
    }

    void parseLine(char* line, char fields[][100], int maxFields, int& fieldCount) {
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
                if (ci < 99) fields[fieldCount][ci++] = c;
            }
        }
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
        file.getline(line, 256);

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

#endif
