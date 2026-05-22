// ============================================================
// System      : Warehouse Robot Navigation System
// File        : main.cpp
// Description : Main entry point. Integrates all 5 modules
//               into one complete warehouse management system.
// Members     : Member 1 (Task 1), Member 2 (Task 2),
//               Member 3 (Task 3), Member 4 (Task 4),
//               Member 5 (Task 5)
// ============================================================

// WarehouseTree must be first — it defines Step, which Navigation.hpp uses
#include "WarehouseTree.hpp"
#include "ItemBST.hpp"
#include "OrderQueue.hpp"
#include "RobotQueue.hpp"
#include "Navigation.hpp"

#include <cstring>
#include <cstdio>
#include <fstream>

// Print the main menu
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

// Append one completed journey's forward steps to navigation.csv
static void logJourney(const char* journeyID, const char* robotID,
                        const char* orderID,
                        const Step* steps, int stepCount) {
    std::ofstream navLog("navigation.csv", std::ios::app);  // append — preserve history
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

    // Load all data sources before presenting the menu
    printf("=== Loading System Data ===\n");
    warehouseTree.loadFromCSV("warehouse.csv");
    itemBST.loadFromCSV("items.csv");
    orderQueue.loadFromCSV("orders.csv");
    robotQueue.loadFromCSV("robots.csv");
    printf("===========================\n");

    char assignedRobotID[10] = "";   // robot handling the current task
    char currentOrderID[10]  = "";   // order currently being processed
    bool taskInProgress      = false;
    int  journeyCount        = 0;    // increments per completed journey for ID generation

    int choice = 0;
    do {
        printMenu();
        scanf("%d", &choice);

        switch (choice) {

            case 1: {
                // Add a new customer order to the queue
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
                // Dequeue the next order, find the item, assign a robot, plan the route
                if (taskInProgress) {
                    printf("A task is already in progress. Complete it first.\n");
                    break;
                }

                Order currentOrder = orderQueue.dequeue();
                if (strlen(currentOrder.orderID) == 0) {
                    break;  // dequeue already printed "No pending orders"
                }

                printf("\n--- Processing Order ---\n");
                printf("  Order ID   : %s\n", currentOrder.orderID);
                printf("  Item       : %s\n", currentOrder.itemName);
                printf("  Destination: %s\n", currentOrder.destination);

                // Locate item in BST to get precise zone/aisle/shelf
                Item* foundItem = itemBST.searchByName(currentOrder.itemName);
                if (foundItem == nullptr) {
                    printf("Item not in warehouse inventory. Order cannot be processed.\n");
                    break;
                }
                printf("  Located at : Zone=%s | Aisle=%s | Shelf=%s\n",
                       foundItem->zone, foundItem->aisle, foundItem->shelf);

                // Assign an available robot
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

                // Generate the navigation path using the shelf as destination
                navigationStack.loadPathFromWarehouse(
                    warehouseTree, "Warehouse", foundItem->shelf);

                navigationStack.displayForwardPath();
                taskInProgress = true;
                printf("Robot %s is navigating to %s.\n",
                       assignedRobotID, foundItem->shelf);
                break;
            }

            case 3: {
                // Robot retraces its path, is released, and the order is closed
                if (!taskInProgress) {
                    printf("No task is currently in progress.\n");
                    break;
                }

                // Retrieve the forward path before returnToStart() empties the stack
                int pathCount = 0;
                const Step* forwardSteps =
                    navigationStack.getForwardPath(pathCount);

                navigationStack.returnToStart();
                robotQueue.releaseRobot(assignedRobotID);
                orderQueue.markCompleted(currentOrderID);

                // Append this journey to navigation.csv
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
                itemBST.search(itemID);  // prints result internally
                break;
            }

            case 8: {
                char itemName[30];
                printf("Enter Item Name: ");
                scanf(" %29[^\n]", itemName);
                itemBST.searchByName(itemName);  // prints result internally
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
                printf("System shutting down. Goodbye.\n");
                break;

            default:
                printf("Invalid option. Please enter a number between 1 and 11.\n");
        }

    } while (choice != 11);

    return 0;
}
