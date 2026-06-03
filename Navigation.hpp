
// Navigation & Path Tracking Module
// Member 3

#ifndef NAVIGATION_HPP
#define NAVIGATION_HPP

#include <cstring>
#include <cstdio>
#include "WarehouseTree.hpp"

class NavigationStack {
private:
    static const int MAX_FORWARD_LOG = 50;

    struct StackNode {
        Step data;
        StackNode* next;
    };

    StackNode* top;
    Step forwardPath[MAX_FORWARD_LOG];  // save forward path
    int forwardCount;

public:
    NavigationStack() : top(nullptr), forwardCount(0) {}

    ~NavigationStack() {
        clearStack();
    }

    bool isEmpty() {
        return top == nullptr;
    }

    bool isFull() {
        return false;
    }

    // push step onto the stack
    void push(Step s) {
        StackNode* newNode = new StackNode();
        newNode->data = s;
        newNode->next = top;
        top = newNode;
    }

    // pop step
    Step pop() {
        if (isEmpty()) {
            printf("No steps to retrace.\n");
            Step empty;
            empty.stepNumber = -1;
            empty.direction[0] = '\0';
            empty.location[0] = '\0';
            return empty;
        }

        StackNode* temp = top;
        Step value = temp->data;
        top = top->next;
        delete temp;

        return value;
    }

    // check top step
    Step peek() {
        if (isEmpty()) {
            Step empty;
            empty.stepNumber = -1;
            empty.direction[0] = '\0';
            empty.location[0] = '\0';
            return empty;
        }

        return top->data;
    }

    // show forward path (reads from forwardPath[], not the stack)
    void displayForwardPath() {
        if (forwardCount == 0) {
            printf("No forward path recorded.\n");
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

    // return by popping stack
    void returnToStart() {
        if (isEmpty()) {
            printf("No steps to retrace.\n");
            return;
        }

        printf("\n--- RETURN PATH ---\n");
        while (!isEmpty()) {
            Step s = pop();
            char reverseDir[20];

            if (strcmp(s.direction, "Forward") == 0)
                strcpy(reverseDir, "Move Backward");
            else if (strcmp(s.direction, "Left") == 0)
                strcpy(reverseDir, "Turn Right");
            else if (strcmp(s.direction, "Right") == 0)
                strcpy(reverseDir, "Turn Left");
            else if (strcmp(s.direction, "Backward") == 0)
                strcpy(reverseDir, "Move Forward");
            else
                strcpy(reverseDir, "Move Backward");

            printf("  %s at: %s\n", reverseDir, s.location);
        }
        printf("-------------------\n\n");
    }

    // clear stack memory
    void clearStack() {
        while (top != nullptr) {
            StackNode* temp = top;
            top = top->next;
            delete temp;
        }
    }

    // get path for log file
    const Step* getForwardPath(int& count) {
        count = forwardCount;
        return forwardPath;
    }

    // load path without obstacle
    void loadPathFromWarehouse(WarehouseTree& tree,
                               const char* startZone,
                               const char* destination) {
        loadPathFromWarehouse(tree, startZone, destination, "");
    }

    // load path with obstacle checking
    void loadPathFromWarehouse(WarehouseTree& tree,
                               const char* startZone,
                               const char* destination,
                               const char* blockedLocation) {
        clearStack();
        forwardCount = 0;

        int stepCount = 0;
        Step* pathSteps = tree.findPath(startZone, destination, stepCount);

        if (pathSteps == nullptr || stepCount == 0) {
            printf("No path found to destination: %s\n", destination);
            return;
        }

        for (int i = 0; i < stepCount; i++) {
            if (blockedLocation != nullptr &&
                blockedLocation[0] != '\0' &&
                strcmp(pathSteps[i].location, blockedLocation) == 0) {
                printf("\nObstacle detected at %s! Robot cannot proceed.\n", blockedLocation);
                printf("Robot is backtracking to the starting point.\n");
                returnToStart();  
                forwardCount = 0;
                return;
            }

            push(pathSteps[i]);
            if (i < MAX_FORWARD_LOG) {
                forwardPath[i] = pathSteps[i];
            }
        }

        if (stepCount < MAX_FORWARD_LOG)
            forwardCount = stepCount;
        else
            forwardCount = MAX_FORWARD_LOG;
    }
};

#endif 
