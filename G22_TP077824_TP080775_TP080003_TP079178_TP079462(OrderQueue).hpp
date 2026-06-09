#ifndef ORDER_QUEUE_HPP
#define ORDER_QUEUE_HPP

#include <cstring>
#include <cstdio>
#include <fstream>

struct Order {
    char orderID[10];
    char itemName[30];
    char destination[50];
    char status[15];
};

struct OrderNode {
    Order      data;
    OrderNode* next;
};

class OrderQueue {
private:
    static const int MAX_SIZE = 50;

    OrderNode* front;
    OrderNode* rear;
    OrderNode* processedHead;
    int        count;

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
    OrderQueue() : front(nullptr), rear(nullptr), processedHead(nullptr), count(0) {}

    ~OrderQueue() {
        while (front != nullptr) {
            OrderNode* temp = front;
            front = front->next;
            delete temp;
        }
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
        temp->next    = processedHead;
        processedHead = temp;
        return result;
    }

    Order peekFront() {
        if (isEmpty()) {
            Order empty;
            empty.orderID[0]     = '\0';
            empty.itemName[0]    = '\0';
            empty.destination[0] = '\0';
            empty.status[0]      = '\0';
            return empty;
        }
        return front->data;
    }

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
        file.getline(line, 256);

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

#endif
