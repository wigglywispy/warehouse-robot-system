# Warehouse Robot Navigation System

## Project Overview
This is a C++ university assignment for CT077-3-2-DSTR (Data Structures).
It is a warehouse robot navigation system built as a single `.cpp` file.

## Absolute Rules (Never Break These)
- Single `.cpp` file only — do NOT split into multiple files unless explicitly asked
- NO STL containers allowed — no <vector>, <list>, <queue>, <stack>, <deque>, <set>, <map>
- All data structures must be manually implemented from scratch
- Language: C++ only

## Team Members & Modules
- Member 1 — Order Management Module (Queue using linked list)
- Member 2 — Robot Assignment Module (Circular Queue using array)
- Member 3 — Robot Navigation & Path Tracking Module (Stack using linked list)

## Data Structures Used
| Module          | Data Structure  | Implementation       |
|-----------------|-----------------|----------------------|
| Order Mgmt      | Queue           | Linked list nodes    |
| Robot Assignment| Circular Queue  | Fixed-size array     |
| Navigation      | Stack           | Linked list nodes    |

## Code Style Requirements
- Meaningful variable and function names
- Proper indentation (use 4 spaces)
- Comments on every major block and function
- No magic numbers — use named constants
- Section headers separating each module inside the .cpp file

## File Naming Convention
<GroupNo>_<leaderID>_<member1ID>_<member2ID>.cpp
Example: G1_TP012345_TP012344_TP012123.cpp

## Submission Deadline
Wednesday Week 14 — 3 June 2026 by 5:00 PM via Moodle

## When Helping With This Project
- Always check that NO STL containers are used before suggesting any code
- Always keep everything inside one .cpp file
- If fixing a bug, do not break other modules
- If adding features, follow the existing code style and structure
- Memory management: always implement destructors and free heap memory