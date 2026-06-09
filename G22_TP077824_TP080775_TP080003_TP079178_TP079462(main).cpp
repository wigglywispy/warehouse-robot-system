#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include "G22_TP077824_TP080775_TP080003_TP079178_TP079462(ItemBST).hpp"
#include "G22_TP077824_TP080775_TP080003_TP079178_TP079462(OrderQueue).hpp"
#include "G22_TP077824_TP080775_TP080003_TP079178_TP079462(RobotQueue).hpp"
#include "G22_TP077824_TP080775_TP080003_TP079178_TP079462(Navigation).hpp"

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
    printf("[11] Delete Item by ID\n");
    printf("[12] Exit\n");
    printf("========================================\n");
    printf("Enter choice: ");
}

static void logJourney(const char* journeyID, const char* robotID,
                        const char* orderID,
                        const Step* steps, int stepCount) {
    std::ofstream navLog("G22_TP077824_TP080775_TP080003_TP079178_TP079462(navigation).csv", std::ios::app);
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

    printf("=== Loading System Data ===\n");
    warehouseTree.loadFromCSV("G22_TP077824_TP080775_TP080003_TP079178_TP079462(warehouse).csv");
    itemBST.loadFromCSV("G22_TP077824_TP080775_TP080003_TP079178_TP079462(items).csv");
    orderQueue.loadFromCSV("G22_TP077824_TP080775_TP080003_TP079178_TP079462(orders).csv");
    robotQueue.loadFromCSV("G22_TP077824_TP080775_TP080003_TP079178_TP079462(robots).csv");
    printf("===========================\n");

    char assignedRobotID[10] = "";
    char currentOrderID[10]  = "";
    bool taskInProgress      = false;

    int journeyCount = 0;
    {
        std::ifstream navLog("G22_TP077824_TP080775_TP080003_TP079178_TP079462(navigation).csv");
        if (navLog.is_open()) {
            char line[256];
            navLog.getline(line, 256);
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

                if (taskInProgress) {
                    printf("A task is already in progress. Complete it first.\n");
                    break;
                }

                Order peeked = orderQueue.peekFront();
                if (strlen(peeked.orderID) == 0) {
                    printf("No pending orders.\n");
                    break;
                }

                Item* foundItem = itemBST.searchByName(peeked.itemName);
                if (foundItem == nullptr) {
                    printf("Item '%s' not in inventory. Order %s removed from queue.\n",
                           peeked.itemName, peeked.orderID);
                    orderQueue.dequeue();
                    break;
                }

                int testSteps = 0;
                Step* testPath = warehouseTree.findPath(
                    foundItem->zone, foundItem->shelf, testSteps);
                if (testPath == nullptr || testSteps == 0) {
                    printf("No path to '%s'. Order %s removed from queue.\n",
                           foundItem->shelf, peeked.orderID);
                    orderQueue.dequeue();
                    break;
                }

                Robot nextRobot = robotQueue.getNextAvailable();
                if (strlen(nextRobot.robotID) == 0) {
                    printf("No robot available. Order not removed from queue.\n");
                    break;
                }

                Order currentOrder = orderQueue.dequeue();

                printf("\n--- Processing Order ---\n");
                printf("  Order ID   : %s\n", currentOrder.orderID);
                printf("  Item       : %s\n", currentOrder.itemName);
                printf("  Destination: %s\n", currentOrder.destination);
                printf("  Located at : Zone=%s | Aisle=%s | Shelf=%s\n",
                       foundItem->zone, foundItem->aisle, foundItem->shelf);

                char blockedLocation[50] = "";
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

                if (!taskInProgress) {
                    printf("No task is currently in progress.\n");
                    break;
                }

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

            case 11: {
                char delID[10];
                printf("Enter Item ID to delete: ");
                scanf("%9s", delID);
                if (itemBST.search(delID) != nullptr) {
                    itemBST.deleteItem(delID);
                    printf("Item %s deleted successfully.\n", delID);
                }
                break;
            }

            case 12:
                printf("Exiting system. Goodbye!\n");
                break;

            default:
                printf("Invalid choice. Please enter a number between 1 and 12.\n");
        }

    } while (choice != 12);

    return 0;
}
