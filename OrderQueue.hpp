// ============================================================
// Module      : Order Management
// Member      : Member 1
// Data Structure: Queue (array-based, linear)
// Description : Handles incoming customer orders using a queue.
//               Orders are processed first-in first-out (FIFO).
// ============================================================

#ifndef ORDER_QUEUE_HPP
#define ORDER_QUEUE_HPP

#include <cstring>
#include <cstdio>
#include <fstream>

struct Order {
    char orderID[10];
    char itemName[30];
    char destination[50];
    char status[15];   // "Pending" or "Completed"
};

class OrderQueue {
private:
    static const int MAX_SIZE = 10;
    Order orders[MAX_SIZE];
    int   front;   // index of next order to dequeue
    int   rear;    // index of last added order (-1 if empty)
    int   count;   // number of active orders

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
    OrderQueue() : front(0), rear(-1), count(0) {}

    bool isEmpty() { return count == 0; }

    // queue is full when rear reaches the end of the array
    bool isFull() { return rear == MAX_SIZE - 1; }

    void enqueue(Order o) {
        if (isFull()) {
            printf("Order queue is full.\n");
            return;
        }
        orders[++rear] = o;
        count++;
    }

    // dequeue moves front forward, old slot stays for markCompleted()
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
        Order o = orders[front];
        front++;
        count--;
        return o;
    }

    // show orders that are still pending (from front to rear)
    void displayPending() {
        printf("\n--- Pending Orders ---\n");
        bool found = false;
        for (int i = front; i <= rear; i++) {
            if (strcmp(orders[i].status, "Pending") == 0) {
                printf("  %-10s | %-20s | %-35s | %s\n",
                       orders[i].orderID, orders[i].itemName,
                       orders[i].destination, orders[i].status);
                found = true;
            }
        }
        if (!found) printf("  No pending orders.\n");
        printf("\n");
    }

    // scan entire array including already-dequeued slots for completed ones
    void displayCompleted() {
        printf("\n--- Completed Orders ---\n");
        bool found = false;
        for (int i = 0; i <= rear; i++) {
            if (strcmp(orders[i].status, "Completed") == 0) {
                printf("  %-10s | %-20s | %-35s | %s\n",
                       orders[i].orderID, orders[i].itemName,
                       orders[i].destination, orders[i].status);
                found = true;
            }
        }
        if (!found) printf("  No completed orders.\n");
        printf("\n");
    }

    void markCompleted(char* orderID) {
        for (int i = 0; i <= rear; i++) {
            if (strcmp(orders[i].orderID, orderID) == 0) {
                strncpy(orders[i].status, "Completed", 14);
                orders[i].status[14] = '\0';
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

            char fields[6][100];
            int fc = 0;
            parseLine(line, fields, fc);
            if (fc < 4) continue;

            Order o;
            strncpy(o.orderID,     fields[0], 9);  o.orderID[9]      = '\0';
            strncpy(o.itemName,    fields[1], 29); o.itemName[29]    = '\0';
            strncpy(o.destination, fields[2], 49); o.destination[49] = '\0';
            strncpy(o.status,      fields[3], 14); o.status[14]      = '\0';

            enqueue(o);
        }
        file.close();
        printf("Orders loaded successfully.\n");
    }
};

#endif // ORDER_QUEUE_HPP
