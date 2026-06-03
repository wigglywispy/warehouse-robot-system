// ============================================================
// Navigation & Path Tracking Module
// Member 3
// Stack (array-based, LIFO) - records each step the robot takes
// so it can retrace back to start by popping in reverse order.
// No STL used.
// ============================================================

#ifndef NAVIGATION_HPP
#define NAVIGATION_HPP

#include <cstring>
#include <cstdio>
#include "WarehouseTree.hpp"

class NavigationStack {
private:
    static const int MAX_SIZE = 20;
    Step steps[MAX_SIZE];
    int  top;
    Step forwardPath[MAX_SIZE];  // copy of path for CSV logging after return
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
            empty.stepNumber   = -1;
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

    // print path from bottom to top (step 1 first)
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
        top = -1;
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

        for (int i = 0; i < stepCount && i < MAX_SIZE; i++) {
            push(pathSteps[i]);
            forwardPath[i] = pathSteps[i];
        }
        forwardCount = (stepCount < MAX_SIZE) ? stepCount : MAX_SIZE;
    }
};

#endif // NAVIGATION_HPP
