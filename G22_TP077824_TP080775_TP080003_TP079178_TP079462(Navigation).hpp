#ifndef NAVIGATION_HPP
#define NAVIGATION_HPP

#include <cstring>
#include <cstdio>
#include "G22_TP077824_TP080775_TP080003_TP079178_TP079462(WarehouseTree).hpp"

struct StackNode {
    Step       data;
    StackNode* next;
};

class NavigationStack {
private:
    static const int MAX_PATH = 20;

    StackNode* top;
    Step       forwardPath[MAX_PATH];
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

    const Step* getForwardPath(int& count) {
        count = forwardCount;
        return forwardPath;
    }

    void loadPathFromWarehouse(WarehouseTree& tree,
                               const char* startZone,
                               const char* destination) {
        loadPathFromWarehouse(tree, startZone, destination, "");
    }

    void loadPathFromWarehouse(WarehouseTree& tree,
                               const char* startZone,
                               const char* destination,
                               const char* blockedLocation) {
        clearStack();

        int stepCount = 0;
        Step* pathSteps = tree.findPath(startZone, destination, stepCount);

        if (pathSteps == nullptr || stepCount == 0) {
            printf("No path found to destination: %s\n", destination);
            return;
        }

        int limit = (stepCount < MAX_PATH) ? stepCount : MAX_PATH;

        for (int i = 0; i < limit; i++) {
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
            forwardPath[i] = pathSteps[i];
        }
        forwardCount = limit;
    }
};

#endif
