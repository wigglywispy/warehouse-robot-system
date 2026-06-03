// ================================================================
// Robot Assignment Module
// Member 2
// Uses a circular queue (array-based) to assign tasks to robots.
// Robots rotate fairly so the same one doesn't always get picked.
// No STL used.
// ================================================================

#ifndef ROBOT_QUEUE_HPP
#define ROBOT_QUEUE_HPP

#include <cstring>
#include <cstdio>
#include <fstream>

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
                robots[idx].status[14]      = '\0';
                robots[idx].currentTask[0]  = '\0';
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

            char fields[5][100];
            int fc = 0;
            parseLine(line, fields, fc);
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

#endif // ROBOT_QUEUE_HPP
