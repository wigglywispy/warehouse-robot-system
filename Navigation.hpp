// ============================================================
// Module      : Robot Navigation and Path Tracking Module
// File        : Navigation.hpp
// Data Structure: Stack
// Responsible : Member 3
// Description : Records robot movement steps using a stack.
//               Enables reverse path retracing for robot
//               return journey using LIFO behaviour. No STL.
// ============================================================

#ifndef NAVIGATION_HPP
#define NAVIGATION_HPP

#include <cstring>
#include <cstdio>
#include "WarehouseTree.hpp"  // Step struct + WarehouseTree class defined there

class NavigationStack {
private:
    static const int MAX_SIZE = 20;
    Step steps[MAX_SIZE];        // fixed-size LIFO storage
    int  top;                    // index of top element; -1 when empty
    Step forwardPath[MAX_SIZE];  // backup of loaded path for post-return logging
    int  forwardCount;

public:
    NavigationStack() : top(-1), forwardCount(0) {}

    bool isEmpty() { return top == -1; }
    bool isFull()  { return top == MAX_SIZE - 1; }

    void push(Step s) {
        if (isFull()) {
            printf("Navigation stack is full.\n");
            return;
        }
        steps[++top] = s;
    }

    Step pop() {
        if (isEmpty()) {
            printf("No steps to retrace.\n");
            Step empty;
            empty.stepNumber  = -1;
            empty.direction[0] = '\0';
            empty.location[0]  = '\0';
            return empty;
        }
        return steps[top--];
    }

    Step peek() {
        if (isEmpty()) {
            Step empty;
            empty.stepNumber   = -1;
            empty.direction[0] = '\0';
            empty.location[0]  = '\0';
            return empty;
        }
        return steps[top];
    }

    // Print all steps bottom-to-top (index 0 = first move taken)
    void displayForwardPath() {
        if (isEmpty()) {
            printf("Navigation stack is empty.\n");
            return;
        }
        printf("\n--- FORWARD NAVIGATION PATH ---\n");
        for (int i = 0; i <= top; i++) {
            printf("  Step %d: %-10s -> %s\n",
                   steps[i].stepNumber,
                   steps[i].direction,
                   steps[i].location);
        }
        printf("--------------------------------\n\n");
    }

    // Pop every step and print the reverse instruction for each
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
        top = -1;  // fixed array — reset index only
    }

    // Expose the saved forward path so main.cpp can log it after returnToStart()
    const Step* getForwardPath(int& count) {
        count = forwardCount;
        return forwardPath;
    }

    // Integration point: query WarehouseTree for the path then push each step
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

        for (int i = 0; i < stepCount && i < MAX_SIZE; i++) {
            push(pathSteps[i]);
            forwardPath[i] = pathSteps[i];  // save backup for CSV logging
        }
        forwardCount = (stepCount < MAX_SIZE) ? stepCount : MAX_SIZE;
    }
};

#endif // NAVIGATION_HPP
