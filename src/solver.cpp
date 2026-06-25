#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <emscripten/bind.h>

const std::string SOLVED_STATE = "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB";

class Cube {
public:
    std::string state;
    Cube(std::string s = SOLVED_STATE) : state(s) {}

    void applyMoveU() {
        std::string next = state;
        next[0]=state[6]; next[1]=state[3]; next[2]=state[0];
        next[3]=state[7];                   next[5]=state[1];
        next[6]=state[8]; next[7]=state[5]; next[8]=state[2];
        for(int i=0; i<3; ++i) {
            next[18+i]=state[9+i];  next[36+i]=state[18+i];
            next[45+i]=state[36+i]; next[9+i]=state[45+i];
        }
        state = next;
    }

    void applyMoveD() {
        std::string next = state;
        next[27]=state[33]; next[28]=state[30]; next[29]=state[27];
        next[30]=state[34];                     next[32]=state[28];
        next[33]=state[35]; next[34]=state[32]; next[35]=state[29];
        for(int i=0; i<3; ++i) {
            next[24+i]=state[42+i]; next[15+i]=state[24+i];
            next[51+i]=state[15+i]; next[42+i]=state[51+i];
        }
        state = next;
    }

    void applyMoveR() {
        std::string next = state;
        next[9]=state[15]; next[10]=state[12]; next[11]=state[9];
        next[12]=state[16];                    next[14]=state[10];
        next[15]=state[17]; next[16]=state[14]; next[17]=state[11];
        int u[3]={2,5,8}, b[3]={45,48,51}, d[3]={29,32,35}, f[3]={20,23,26};
        for(int i=0; i<3; ++i) {
            next[u[i]]=state[f[i]]; next[b[i]]=state[u[i]];
            next[d[i]]=state[b[i]]; next[f[i]]=state[d[i]];
        }
        state = next;
    }

    void applyMoveL() {
        std::string next = state;
        next[36]=state[42]; next[37]=state[39]; next[38]=state[36];
        next[39]=state[43];                     next[41]=state[37];
        next[42]=state[44]; next[43]=state[41]; next[44]=state[39];
        int u[3]={0,3,6}, f[3]={18,21,24}, d[3]={27,30,33}, b[3]={53,50,47};
        for(int i=0; i<3; ++i) {
            next[f[i]]=state[u[i]]; next[d[i]]=state[f[i]];
            next[b[i]]=state[d[i]]; next[u[i]]=state[b[i]];
        }
        state = next;
    }

    void applyMoveF() {
        std::string next = state;
        next[18]=state[24]; next[19]=state[21]; next[22]=state[18];
        next[20]=state[25];                     next[23]=state[19];
        next[24]=state[26]; next[25]=state[23]; next[26]=state[20];
        next[9]=state[6]; next[12]=state[7]; next[15]=state[8];
        next[27]=state[15]; next[28]=state[12]; next[29]=state[9];
        next[38]=state[29]; next[41]=state[28]; next[44]=state[27];
        next[6]=state[44]; next[7]=state[41]; next[8]=state[38];
        state = next;
    }

    void applyMoveB() {
        std::string next = state;
        next[45]=state[51]; next[46]=state[48]; next[47]=state[45];
        next[48]=state[52];                     next[50]=state[46];
        next[51]=state[53]; next[52]=state[50]; next[53]=state[47];
        next[36]=state[0]; next[39]=state[1]; next[42]=state[2];
        next[35]=state[36]; next[34]=state[39]; next[33]=state[42];
        next[17]=state[35]; next[14]=state[34]; next[11]=state[33];
        next[0]=state[17]; next[1]=state[14]; next[2]=state[11];
        state = next;
    }

    void move(std::string m) {
        if(m=="U") applyMoveU(); else if(m=="R") applyMoveR();
        else if(m=="F") applyMoveF(); else if(m=="D") applyMoveD();
        else if(m=="L") applyMoveL(); else if(m=="B") applyMoveB();
        else if(m=="U'") { applyMoveU(); applyMoveU(); applyMoveU(); }
        else if(m=="R'") { applyMoveR(); applyMoveR(); applyMoveR(); }
        else if(m=="F'") { applyMoveF(); applyMoveF(); applyMoveF(); }
        else if(m=="D'") { applyMoveD(); applyMoveD(); applyMoveD(); }
        else if(m=="L'") { applyMoveL(); applyMoveL(); applyMoveL(); }
        else if(m=="B'") { applyMoveB(); applyMoveB(); applyMoveB(); }
    }

    // Heuristic for IDA* (Estimates minimum moves to solve based on misplaced facelets)
    int getHeuristic() const {
        int misplaced = 0;
        for (int i = 0; i < 54; ++i) {
            if (state[i] != SOLVED_STATE[i]) misplaced++;
        }
        // One turn alters 20 facelets maximum. 
        // Dividing by 20 gives a safe, admissible estimate for the A* math.
        return (misplaced + 19) / 20; 
    }
};

const std::vector<std::string> MOVES = {"U", "R", "F", "D", "L", "B", "U'", "R'", "F'", "D'", "L'", "B'"};

// --- ALGORITHM 1: Breadth First Search (BFS) ---
std::string solveBFS(std::string startState) {
    if (startState == SOLVED_STATE) return "Already Solved!";
    std::queue<std::string> q;
    std::unordered_map<std::string, std::pair<std::string, std::string>> parent;
    q.push(startState);
    parent[startState] = {"", ""};
    int nodesExplored = 0;

    while (!q.empty()) {
        std::string current = q.front();
        q.pop();
        nodesExplored++;
        
        if (nodesExplored > 15000) return "TIMEOUT: BFS limit exceeded. Use IDA* Advanced Search!";
        if (current == SOLVED_STATE) {
            std::string path = "";
            std::string curr = current;
            while (parent[curr].first != "") {
                path = parent[curr].second + " " + path;
                curr = parent[curr].first;
            }
            return "BFS Route: " + path;
        }
        for (const std::string& m : MOVES) {
            Cube c(current);
            c.move(m);
            if (parent.find(c.state) == parent.end()) {
                parent[c.state] = {current, m};
                q.push(c.state);
            }
        }
    }
    return "Unsolvable configuration";
}

// --- ALGORITHM 2: Advanced IDA* (Smart Kociemba Alternative) ---
bool searchIDA(Cube currentCube, int g, int bound, std::string currentPath, std::string& solution, int& nextBound, int& nodesExplored) {
    nodesExplored++;
    if (nodesExplored > 500000) return false; // Browser crash prevention

    int f = g + currentCube.getHeuristic();
    if (f > bound) {
        if (f < nextBound) nextBound = f;
        return false;
    }
    
    if (currentCube.state == SOLVED_STATE) {
        solution = "IDA* Route: " + currentPath;
        return true;
    }

    for (const std::string& m : MOVES) {
        // Prevent redundant reverse moves
        if (!currentPath.empty() && currentPath.back() == m[0]) continue; 

        Cube nextCube = currentCube;
        nextCube.move(m);
        
        std::string nextPath = currentPath == "" ? m : currentPath + " " + m;
        if (searchIDA(nextCube, g + 1, bound, nextPath, solution, nextBound, nodesExplored)) {
            return true;
        }
    }
    return false;
}

std::string solveIDAStar(std::string startState) {
    if (startState == SOLVED_STATE) return "Already Solved!";
    
    Cube startCube(startState);
    int bound = startCube.getHeuristic();
    std::string solution = "";
    int nodesExplored = 0;

    // Iterative Deepening Loop
    while (bound <= 12) { // Capped at 12 depths for browser memory safety
        int nextBound = 999;
        if (searchIDA(startCube, 0, bound, "", solution, nextBound, nodesExplored)) {
            return solution;
        }
        if (nodesExplored > 500000) return "TIMEOUT: Scramble is too deep for safe browser computation.";
        bound = nextBound;
    }
    return "TIMEOUT: Depth limit exceeded.";
}

std::string solveLayerByLayer(std::string state) {
    return "Layer-by-Layer Logic: This requires 50+ custom human macros to be coded. Use BFS or IDA*!";
}

// --- MASTER ROUTER ---
std::string universalSolver(std::string cubeState, std::string method) {
    if (method == "BFS") return solveBFS(cubeState);
    if (method == "LAYER") return solveLayerByLayer(cubeState);
    if (method == "IDASTAR") return solveIDAStar(cubeState); // Fixed: Accurately route to IDA*
    return "Unknown Method";
}

EMSCRIPTEN_BINDINGS(cube_solver) {
    emscripten::function("universalSolver", &universalSolver);
}
