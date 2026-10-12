/*
 ============================================================================
 * Project Name  : Campus Shortest Route Finder (PBLE 2)
 * Course        : Data Structures and Algorithms Laboratory
 * Topic         : Dijkstra's Algorithm on Weighted Graph
 * Author        : Amitabh Sohani
 * Semester      : 3rd / 4th Semester (Second Year B.Tech CSE)
 * Description   : A menu-driven C application representing a campus road
 *                 network using an adjacency matrix, computing single-source
 *                 shortest paths to all landmarks using Dijkstra's Algorithm,
 *                 reconstructing paths via a predecessor array, and analyzing
 *                 computational time complexity.
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LOCATIONS 30
#define MAX_NAME_LEN 50
#define INF 99999

/* --------------------------------------------------------------------------
 * Global Graph & State Variables
 * -------------------------------------------------------------------------- */
int numLocations = 0;
char locationNames[MAX_LOCATIONS][MAX_NAME_LEN];
int adjMatrix[MAX_LOCATIONS][MAX_LOCATIONS];

int dist[MAX_LOCATIONS];
bool visited[MAX_LOCATIONS];
int parent[MAX_LOCATIONS];

int sourceLocation = -1;  /* 0-indexed */
int isGraphLoaded = 0;
int isDijkstraRun = 0;

/* --------------------------------------------------------------------------
 * Utility Functions
 * -------------------------------------------------------------------------- */

/* Helper to flush remaining input characters from stdin */
void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* Helper to safely read a string line */
void readLine(char *buffer, int maxLen) {
    if (fgets(buffer, maxLen, stdin) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
    }
}

/* --------------------------------------------------------------------------
 * Graph Initialization & Sample Loading
 * -------------------------------------------------------------------------- */

/* Initialize an empty adjacency matrix */
void initGraph(int n) {
    numLocations = n;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j) {
                adjMatrix[i][j] = 0;       /* Distance to self is 0 */
            } else {
                adjMatrix[i][j] = INF;     /* INF indicates no direct edge */
            }
        }
    }
    isGraphLoaded = 1;
    isDijkstraRun = 0;
    sourceLocation = -1;
}

/* Load predefined sample campus network as described in PBLE 2 specification */
void loadSampleCampusGraph(void) {
    initGraph(5);

    /* Sample Campus Locations */
    strcpy(locationNames[0], "Main Gate");
    strcpy(locationNames[1], "Library");
    strcpy(locationNames[2], "Computer Department");
    strcpy(locationNames[3], "Laboratory");
    strcpy(locationNames[4], "Auditorium");

    /* Road distances (undirected weighted edges) */
    /* Main Gate <-> Library: 4 */
    adjMatrix[0][1] = 4; adjMatrix[1][0] = 4;

    /* Library <-> Computer Department: 3 */
    adjMatrix[1][2] = 3; adjMatrix[2][1] = 3;

    /* Computer Department <-> Laboratory: 2 */
    adjMatrix[2][3] = 2; adjMatrix[3][2] = 2;

    /* Laboratory <-> Auditorium: 3 */
    adjMatrix[3][4] = 3; adjMatrix[4][3] = 3;

    /* Alternate campus routes */
    adjMatrix[0][2] = 10; adjMatrix[2][0] = 10; /* Main Gate <-> Computer Dept */
    adjMatrix[1][3] = 6;  adjMatrix[3][1] = 6;  /* Library <-> Laboratory */
    adjMatrix[2][4] = 7;  adjMatrix[4][2] = 7;  /* Computer Dept <-> Auditorium */

    sourceLocation = 0; /* Default source: Main Gate */
    isGraphLoaded = 1;
    isDijkstraRun = 0;

    printf("\n[SUCCESS] Sample Campus Road Network loaded successfully!\n");
    printf("Total Locations: 5\n");
    printf("Default Source set to: 1. %s\n", locationNames[0]);
}

/* Interactive user entry for custom campus graph */
void enterCustomCampusGraph(void) {
    int n;
    printf("\n==================================================\n");
    printf("             ENTER CUSTOM CAMPUS GRAPH            \n");
    printf("==================================================\n");
    printf("Enter number of campus locations (2 to %d): ", MAX_LOCATIONS);
    if (scanf("%d", &n) != 1 || n < 2 || n > MAX_LOCATIONS) {
        printf("[ERROR] Invalid number of locations. Returning to menu.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    initGraph(n);

    printf("\nEnter name for each campus location:\n");
    for (int i = 0; i < n; i++) {
        printf("  Location %d: ", i + 1);
        readLine(locationNames[i], MAX_NAME_LEN);
        if (strlen(locationNames[i]) == 0) {
            snprintf(locationNames[i], MAX_NAME_LEN, "Location %d", i + 1);
        }
    }

    printf("\nChoose method to enter road distances:\n");
    printf("  1. Road-by-Road Entry (Connect two locations with distance)\n");
    printf("  2. Full Adjacency Matrix Entry (Row by row)\n");
    printf("Enter choice (1-2): ");
    int inputChoice;
    if (scanf("%d", &inputChoice) != 1) {
        clearInputBuffer();
        inputChoice = 1;
    }
    clearInputBuffer();

    if (inputChoice == 2) {
        /* Adjacency Matrix Entry */
        printf("\nEnter the %d x %d adjacency matrix:\n", n, n);
        printf("(Use 0 for diagonal, %d for no direct road, or road distance)\n", INF);
        for (int i = 0; i < n; i++) {
            printf("Row %d (%s):\n", i + 1, locationNames[i]);
            for (int j = 0; j < n; j++) {
                int weight;
                printf("  Distance to %s: ", locationNames[j]);
                if (scanf("%d", &weight) == 1) {
                    if (i == j) {
                        adjMatrix[i][j] = 0;
                    } else if (weight <= 0 || weight >= INF) {
                        adjMatrix[i][j] = INF;
                    } else {
                        adjMatrix[i][j] = weight;
                    }
                } else {
                    adjMatrix[i][j] = (i == j) ? 0 : INF;
                    clearInputBuffer();
                }
            }
        }
        clearInputBuffer();
    } else {
        /* Road-by-road entry */
        printf("\nEnter road connections (undirected):\n");
        printf("Available Locations:\n");
        for (int i = 0; i < n; i++) {
            printf("  %d. %s\n", i + 1, locationNames[i]);
        }
        printf("\nEnter connections as: <Loc1> <Loc2> <Distance>\n");
        printf("Enter 0 0 0 when finished adding roads.\n\n");

        int u, v, weight;
        while (1) {
            printf("Add road (<from> <to> <dist>): ");
            if (scanf("%d %d %d", &u, &v, &weight) != 3) {
                clearInputBuffer();
                break;
            }
            if (u == 0 && v == 0 && weight == 0) {
                break;
            }
            if (u < 1 || u > n || v < 1 || v > n) {
                printf("  [!] Invalid location number! Must be between 1 and %d.\n", n);
                continue;
            }
            if (u == v) {
                printf("  [!] Self-loops are not needed (distance is 0).\n");
                continue;
            }
            if (weight < 0) {
                printf("  [!] Road distance cannot be negative!\n");
                continue;
            }
            /* Set undirected edge */
            adjMatrix[u - 1][v - 1] = weight;
            adjMatrix[v - 1][u - 1] = weight;
            printf("  -> Connected %s <-> %s with distance %d.\n",
                   locationNames[u - 1], locationNames[v - 1], weight);
        }
        clearInputBuffer();
    }

    sourceLocation = 0; /* Default source to first location */
    isGraphLoaded = 1;
    isDijkstraRun = 0;
    printf("\n[SUCCESS] Custom Campus Graph configured with %d locations!\n", numLocations);
}

/* --------------------------------------------------------------------------
 * Menu Option 1: Enter Campus Graph
 * -------------------------------------------------------------------------- */
void menuEnterCampusGraph(void) {
    printf("\n==================================================\n");
    printf("              1. ENTER CAMPUS GRAPH               \n");
    printf("==================================================\n");
    printf("1. Load Predefined Sample Campus Network (5 Locations)\n");
    printf("2. Enter Custom Campus Locations and Road Distances\n");
    printf("Enter choice (1-2): ");
    int choice;
    if (scanf("%d", &choice) != 1) {
        clearInputBuffer();
        printf("[ERROR] Invalid choice.\n");
        return;
    }
    clearInputBuffer();

    if (choice == 1) {
        loadSampleCampusGraph();
    } else if (choice == 2) {
        enterCustomCampusGraph();
    } else {
        printf("[ERROR] Invalid choice.\n");
    }
}

/* --------------------------------------------------------------------------
 * Menu Option 2: Display Adjacency Matrix
 * -------------------------------------------------------------------------- */
void menuDisplayAdjacencyMatrix(void) {
    if (!isGraphLoaded) {
        printf("\n[!] Graph is not loaded yet! Please enter/load a campus graph first.\n");
        return;
    }

    printf("\n=========================================================================================\n");
    printf("                             CAMPUS GRAPH ADJACENCY MATRIX                              \n");
    printf("=========================================================================================\n");
    printf("Legend: 0 = Self (Diagonal), INF = No Direct Road, Weights = Road Distance\n\n");

    /* Print column headers */
    printf("%-24s", "Location Name / Index");
    for (int j = 0; j < numLocations; j++) {
        printf(" [%2d]", j + 1);
    }
    printf("\n");

    printf("------------------------");
    for (int j = 0; j < numLocations; j++) {
        printf("-----");
    }
    printf("\n");

    /* Print matrix rows */
    for (int i = 0; i < numLocations; i++) {
        printf("[%2d] %-19.19s", i + 1, locationNames[i]);
        for (int j = 0; j < numLocations; j++) {
            if (adjMatrix[i][j] == INF) {
                printf("  INF");
            } else {
                printf(" %4d", adjMatrix[i][j]);
            }
        }
        printf("\n");
    }
    printf("=========================================================================================\n");

    printf("\nLocation Index Reference:\n");
    for (int i = 0; i < numLocations; i++) {
        printf("  [%d] %s\n", i + 1, locationNames[i]);
    }
}

/* --------------------------------------------------------------------------
 * Menu Option 3: Select Source Location
 * -------------------------------------------------------------------------- */
void menuSelectSourceLocation(void) {
    if (!isGraphLoaded) {
        printf("\n[!] Graph is not loaded yet! Please enter/load a campus graph first.\n");
        return;
    }

    printf("\n==================================================\n");
    printf("             3. SELECT SOURCE LOCATION            \n");
    printf("==================================================\n");
    printf("Available Campus Locations:\n");
    for (int i = 0; i < numLocations; i++) {
        printf("  %d. %s\n", i + 1, locationNames[i]);
    }

    printf("\nEnter Source Location Number (1 to %d): ", numLocations);
    int src;
    if (scanf("%d", &src) != 1 || src < 1 || src > numLocations) {
        printf("[ERROR] Invalid location number!\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    sourceLocation = src - 1;
    isDijkstraRun = 0; /* Reset Dijkstra so it is recalculated for new source */

    printf("\n[SUCCESS] Source Location selected: [%d] %s\n",
           sourceLocation + 1, locationNames[sourceLocation]);
}

/* --------------------------------------------------------------------------
 * Dijkstra's Shortest Path Algorithm Implementation
 * -------------------------------------------------------------------------- */

/* Helper: Find unvisited vertex with minimum tentative distance */
int findMinDistanceVertex(int distances[], bool isVisited[], int V) {
    int min = INF;
    int minIndex = -1;

    for (int v = 0; v < V; v++) {
        if (!isVisited[v] && distances[v] <= min) {
            min = distances[v];
            minIndex = v;
        }
    }
    return minIndex;
}

/* Run Dijkstra's Algorithm from selected sourceLocation */
void runDijkstraAlgorithm(void) {
    if (!isGraphLoaded) {
        printf("\n[!] Please load or enter the campus graph first.\n");
        return;
    }

    if (sourceLocation < 0 || sourceLocation >= numLocations) {
        printf("\n[!] Source location not set. Defaulting to Location 1: %s\n", locationNames[0]);
        sourceLocation = 0;
    }

    int V = numLocations;

    /* Step 1: Initialize all distances to INF, visited to false, parent to -1 */
    for (int i = 0; i < V; i++) {
        dist[i] = INF;
        visited[i] = false;
        parent[i] = -1;
    }

    /* Distance from source to itself is 0 */
    dist[sourceLocation] = 0;

    /* Step 2: Iterate (V - 1) times to find shortest paths */
    for (int count = 0; count < V - 1; count++) {
        /* Select unvisited vertex u with minimum distance */
        int u = findMinDistanceVertex(dist, visited, V);

        /* If no reachable vertex remains, terminate early */
        if (u == -1 || dist[u] == INF) {
            break;
        }

        /* Mark selected vertex as visited/settled */
        visited[u] = true;

        /* Step 3: Relaxation step for all adjacent vertices v of u */
        for (int v = 0; v < V; v++) {
            if (!visited[v] &&
                adjMatrix[u][v] != 0 &&
                adjMatrix[u][v] != INF &&
                dist[u] != INF) {

                /* Check if a shorter path to v exists via u */
                if (dist[u] + adjMatrix[u][v] < dist[v]) {
                    dist[v] = dist[u] + adjMatrix[u][v];
                    parent[v] = u; /* Record u as predecessor of v */
                }
            }
        }
    }

    isDijkstraRun = 1;
}

/* --------------------------------------------------------------------------
 * Menu Option 4: Find Shortest Distance
 * -------------------------------------------------------------------------- */
void menuFindShortestDistance(void) {
    if (!isGraphLoaded) {
        printf("\n[!] Please load or enter the campus graph first.\n");
        return;
    }

    if (sourceLocation == -1) {
        printf("\n[!] Source location is not selected. Setting source to Location 1: %s\n", locationNames[0]);
        sourceLocation = 0;
    }

    printf("\n==================================================\n");
    printf("             4. FIND SHORTEST DISTANCE            \n");
    printf("==================================================\n");
    printf("Computing single-source shortest paths using Dijkstra's Algorithm...\n");
    printf("Selected Source: [%d] %s\n", sourceLocation + 1, locationNames[sourceLocation]);

    runDijkstraAlgorithm();

    printf("\n[SUCCESS] Dijkstra's Algorithm executed successfully!\n");
    printf("Shortest distances and predecessor paths have been updated.\n");
    printf("Use Option 5 to view full path routes or Option 6 to view summary.\n");
}

/* --------------------------------------------------------------------------
 * Path Reconstruction Helper
 * -------------------------------------------------------------------------- */
void buildPathHelper(int curr, int pathArr[], int *pathLen) {
    if (curr == -1) return;
    buildPathHelper(parent[curr], pathArr, pathLen);
    pathArr[(*pathLen)++] = curr;
}

/* --------------------------------------------------------------------------
 * Menu Option 5: Display Shortest Paths
 * Matches the exact illustrative output format specified in PBLE 2
 * -------------------------------------------------------------------------- */
void menuDisplayShortestPaths(void) {
    if (!isGraphLoaded) {
        printf("\n[!] Please load or enter the campus graph first.\n");
        return;
    }

    if (!isDijkstraRun) {
        printf("\n[*] Running Dijkstra's algorithm for current source...\n");
        runDijkstraAlgorithm();
    }

    printf("\n====================================================================================================\n");
    printf("                         SHORTEST PATHS FROM SOURCE: %s\n", locationNames[sourceLocation]);
    printf("====================================================================================================\n");
    printf("%-24s | %-17s | %s\n", "Destination", "Shortest Distance", "Shortest Path");
    printf("-------------------------+-------------------+------------------------------------------------------\n");

    for (int i = 0; i < numLocations; i++) {
        /* Skip source to itself in main destination list or display if desired */
        if (i == sourceLocation) {
            continue;
        }

        printf("%-24s | ", locationNames[i]);

        if (dist[i] == INF) {
            printf("%-17s | No path available (Disconnected)\n", "INF (Unreachable)");
        } else {
            printf("%-17d | ", dist[i]);

            /* Reconstruct path using parent array */
            int pathArr[MAX_LOCATIONS];
            int pathLen = 0;
            buildPathHelper(i, pathArr, &pathLen);

            for (int k = 0; k < pathLen; k++) {
                printf("%s", locationNames[pathArr[k]]);
                if (k < pathLen - 1) {
                    printf(" -> ");
                }
            }
            printf("\n");
        }
    }
    printf("====================================================================================================\n");
}

/* --------------------------------------------------------------------------
 * Menu Option 6: Display Distance from Source to All Locations
 * Includes Time Complexity Analysis and Theoretical Justification
 * -------------------------------------------------------------------------- */
void menuDisplayDistanceToAllLocations(void) {
    if (!isGraphLoaded) {
        printf("\n[!] Please load or enter the campus graph first.\n");
        return;
    }

    if (!isDijkstraRun) {
        printf("\n[*] Running Dijkstra's algorithm for current source...\n");
        runDijkstraAlgorithm();
    }

    printf("\n==================================================================\n");
    printf("       SHORTEST DISTANCES FROM SOURCE: [%d] %s\n",
           sourceLocation + 1, locationNames[sourceLocation]);
    printf("==================================================================\n");
    printf("%-5s | %-25s | %-15s\n", "No.", "Location Name", "Shortest Distance");
    printf("------+---------------------------+-------------------------------\n");

    for (int i = 0; i < numLocations; i++) {
        if (i == sourceLocation) {
            printf("%-5d | %-25s | %-15s (Source)\n", i + 1, locationNames[i], "0");
        } else if (dist[i] == INF) {
            printf("%-5d | %-25s | %-15s\n", i + 1, locationNames[i], "INF (Unreachable)");
        } else {
            printf("%-5d | %-25s | %-15d\n", i + 1, locationNames[i], dist[i]);
        }
    }
    printf("==================================================================\n");

    /* Theoretical Time and Space Complexity Statement & Justification */
    printf("\n------------------------------------------------------------------\n");
    printf("                 COMPLEXITY ANALYSIS & JUSTIFICATION              \n");
    printf("------------------------------------------------------------------\n");
    printf("1. Time Complexity: O(V^2)\n");
    printf("   - Number of Vertices (Locations) = V = %d\n", numLocations);
    printf("   - The outer loop runs (V - 1) times to process each vertex: O(V).\n");
    printf("   - In each iteration, finding the vertex with minimum tentative\n");
    printf("     distance requires a linear scan over all vertices: O(V).\n");
    printf("   - Updating adjacent vertices across the row of the Adjacency Matrix\n");
    printf("     takes O(V) time.\n");
    printf("   - Total Time = (V - 1) * (O(V) + O(V)) = O(V^2).\n\n");
    printf("2. Space Complexity: O(V^2)\n");
    printf("   - Adjacency Matrix: Stores edge weights in a %d x %d 2D array: O(V^2).\n", numLocations, numLocations);
    printf("   - Auxiliary arrays (dist, visited, parent): 3 * O(V) = O(V).\n");
    printf("   - Total Space Complexity: O(V^2).\n");
    printf("------------------------------------------------------------------\n");
}

/* --------------------------------------------------------------------------
 * Main Menu & Program Driver
 * -------------------------------------------------------------------------- */
void displayMenu(void) {
    printf("\n==================================================\n");
    printf("   CAMPUS SHORTEST ROUTE FINDER - DIJKSTRA        \n");
    printf("==================================================\n");
    if (isGraphLoaded) {
        printf(" Status: Graph Loaded (%d Locations) | Source: %s\n",
               numLocations,
               (sourceLocation >= 0) ? locationNames[sourceLocation] : "Not Selected");
    } else {
        printf(" Status: No Graph Loaded\n");
    }
    printf("--------------------------------------------------\n");
    printf(" 1. Enter Campus Graph\n");
    printf(" 2. Display Adjacency Matrix\n");
    printf(" 3. Select Source Location\n");
    printf(" 4. Find Shortest Distance\n");
    printf(" 5. Display Shortest Paths\n");
    printf(" 6. Display Distance from Source to All Locations\n");
    printf(" 7. Exit\n");
    printf("==================================================\n");
    printf("Enter your choice (1-7): ");
}

int main(void) {
    int choice = 0;

    printf("\n============================================================\n");
    printf(" Welcome to Campus Shortest Route Finder (Dijkstra's Algo) \n");
    printf(" Academic Project - Data Structures & Algorithms Lab       \n");
    printf("============================================================\n");

    /* Automatically load default sample graph on start for convenience */
    loadSampleCampusGraph();

    do {
        displayMenu();
        if (scanf("%d", &choice) != 1) {
            printf("\n[ERROR] Invalid input! Please enter a number between 1 and 7.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        switch (choice) {
            case 1:
                menuEnterCampusGraph();
                break;
            case 2:
                menuDisplayAdjacencyMatrix();
                break;
            case 3:
                menuSelectSourceLocation();
                break;
            case 4:
                menuFindShortestDistance();
                break;
            case 5:
                menuDisplayShortestPaths();
                break;
            case 6:
                menuDisplayDistanceToAllLocations();
                break;
            case 7:
                printf("\n============================================================\n");
                printf(" Thank you for using Campus Shortest Route Finder. Goodbye! \n");
                printf("============================================================\n\n");
                break;
            default:
                printf("\n[ERROR] Invalid option selected! Please choose between 1 and 7.\n");
                break;
        }
    } while (choice != 7);

    return 0;
}
