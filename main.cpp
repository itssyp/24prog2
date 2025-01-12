#include <iostream>
#include <map>
#include <list>
#include <set>
#include <vector>
#include <queue>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <stdexcept>

using namespace std;

enum Direction{N,E,S,W};
enum Color{R,G,B,Y};

Direction operator++(Direction d) {
    switch (d) {
        case N: return E;
        case E: return S;
        case S: return W;
        case W: return N;
        default: throw std::invalid_argument("Invalid direction");
    }
}

Direction charToDir(char c){
    switch (c){
        case 'N': return N;
        case 'S': return S;
        case 'W': return W;
        case 'E': return E;
        default: throw std::invalid_argument("Invalid Direction");
    }
}

string directionToArrow(Direction dir) {
    switch (dir) {
        case N: return "N";
        case E: return "E";
        case S: return "S";
        case W: return "W";
        default: return " ";
    }
}

struct Position {
    int row, col;

    bool operator<(const Position &other) const {
        return (row < other.row || (row == other.row && col < other.col));
    }

    bool operator==(const Position &other) const {
        return (row == other.row && col == other.row);
    }

    Position move(Direction dir) const {
        switch (dir) {
            case N: return {row - 1, col};
            case E: return {row, col + 1};
            case S: return {row + 1, col};
            case W: return {row, col - 1};
        }
        throw std::invalid_argument("Invalid direction");
    }
};

class Tile{
    string type;
    map<Direction, set<Direction>> flow;
public:
    Tile(string _type, map<Direction, set<Direction>> _flow) : type(_type), flow(_flow) {}

    void rotateRight(){
        map<Direction, set<Direction>> newFlow;
        for (auto fldir: flow){
            Direction newFrom = ++fldir.first;
            newFlow[newFrom];
            for (auto &dir:fldir.second){
                Direction newTo = ++dir;
                newFlow[newFrom].insert(newTo);
            }
        }
        flow = newFlow;
    }

    virtual string getDisplayChar() const {
        return type.substr(0, 1);  // Show first letter of the type (e.g., "P" for Pipe)
    }

    virtual string getDisplayFlow() const {
        string arrows = "";
        for (const auto &fldir : flow) {
            arrows += directionToArrow(fldir.first);
        }
        return arrows;
    }

    virtual bool isValidFlow(Direction from, Direction to) {
        return flow[from].size() > 0;
    }

    const map<Direction, set<Direction>> &getFlow() const {
        return flow;
    }

    void display() const {
        // Display this tile's flow in a simplified way
        cout << "[";
        for (auto &[from, toSet] : flow) {
            cout << "(";
            cout << (from == N ? "N" : from == E ? "E" : from == S ? "S" : "W");
            cout << "->";
            for (Direction to : toSet) {
                cout << (to == N ? "N" : to == E ? "E" : to == S ? "S" : "W") << " ";
            }
            cout << ")";
        }
        cout << "] ";
    }

};

class Pipe: public Tile{
public:
    Pipe(map<Direction, set<Direction>> _flow) : Tile("Pipe", _flow) {}

    string getDisplayChar() const override {
        return "P";
    }
};

class Tap: public Tile{
    bool open = true;
public:
    Tap(map<Direction, set<Direction>> _flow) : Tile("Tap", _flow) {}

    void setOpen(bool state) {
        open = state;
    }

    bool isOpen() const {
        return open;
    }

};

class Source: public Tile{
    Color color;
public:
    Source(Color _color, map<Direction, set<Direction>> _flow) : Tile("Source", _flow), color(_color) {}

    string getDisplayChar() const override {
        return "S";
    }

    string getDisplayFlow() const override {
        return "S" + Tile::getDisplayFlow();
    }

    Color getColor() const {
        return color;
    }
};

class Load: public Tile{
    Color color;
public:
    Load(Color _color, map<Direction, set<Direction>> _flow) : Tile("Load", _flow), color(_color) {}

    string getDisplayChar() const override {
        return "L";
    }

    string getDisplayFlow() const override {
        return "L" + Tile::getDisplayFlow();
    }

    Color getColor() const {
        return color;
    }
};

class Board{
    int rows, cols;
    map<Position, Tile *> grid; // Grid of tiles

public:
    Board(int _rows, int _cols) : rows(_rows), cols(_cols) {}

    bool isCellEmpty(const Position &pos) const {
        return grid.find(pos) == grid.end();
    }

    bool isWithinBounds(const Position &pos) const {
        return pos.row >= 0 && pos.row < rows && pos.col >= 0 && pos.col < cols;
    }

    bool canPlaceTile(const Position &pos, Tile *tile) {
        if (!isWithinBounds(pos) || !isCellEmpty(pos)) return false;

        for (const auto &dirFlow : tile->getFlow()) {
            Direction dir = dirFlow.first;
            Position neighbor = getNeighbor(pos, dir);
            if (!isWithinBounds(neighbor)) continue;

            auto neighborTile = grid.find(neighbor);
            if (neighborTile != grid.end()) {
                for (const auto &connectDir : dirFlow.second) {
                    if (!neighborTile->second->isValidFlow(oppositeDirection(dir), connectDir)) {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    void placeTile(const Position &pos, Tile *tile) {
        grid[pos] = tile;
    }

    void removeTile(const Position &pos) {
        grid.erase(pos);
    }

    Position getNeighbor(const Position &pos, Direction dir) const {
        switch (dir) {
            case N: return {pos.row - 1, pos.col};
            case S: return {pos.row + 1, pos.col};
            case E: return {pos.row, pos.col + 1};
            case W: return {pos.row, pos.col - 1};
        }
        return pos; // Unreachable
    }

    Direction oppositeDirection(Direction dir) const {
        switch (dir) {
            case N: return S;
            case S: return N;
            case E: return W;
            case W: return E;
        }
        throw std::invalid_argument("Invalid direction");
    }

    bool canWaterFlow(const Position &start, const Position &end) {
        queue<Position> toVisit;
        set<Position> visited;

        toVisit.push(start);
        visited.insert(start);

        while (!toVisit.empty()) {
            Position current = toVisit.front();
            toVisit.pop();

            // Check if we've reached the end
            if (current == end) {
                return true;
            }

            Tile *currentTile = getTileAt(current);
            if (!currentTile) {
                continue;  // Skip empty cells
            }

            // Iterate through all flow directions of the current tile
            for (auto &[fromDir, toDirs] : currentTile->getFlow()) {
                for (Direction toDir : toDirs) {
                    Position neighbor = current.move(toDir);
                    // Ensure the neighbor is within bounds and not already visited
                    if (!isWithinBounds(neighbor) || visited.count(neighbor)) {
                        continue;
                    }


                    // Check if the neighbor tile exists and accepts the flow from the current tile
                    Tile *neighborTile = getTileAt(neighbor);
//                    for (auto p: neighborTile->getFlow()){
//                        cout << p.first << ":";
//                        for (auto d: p.second){
//                            cout << d << " ";
//                        }
//                        cout << endl;
//                    }

//                    cout << static_cast<Direction>((toDir + 2) % 4) << endl;
                    if (neighborTile && neighborTile->isValidFlow(static_cast<Direction>((toDir + 2) % 4), toDir)) {
                        toVisit.push(neighbor);
                        visited.insert(neighbor);
                    }
                }
            }
        }

        return false;  // No valid path to the load
    }

    Tile* getTileAt(const Position &pos) const {
        auto it = grid.find(pos);
        return it != grid.end() ? it->second : nullptr;
    }

    int getRows(){
        return rows;
    }

    int getCols(){
        return cols;
    }

    void display() const {
        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                const Tile *tile = getTileAt({row, col});
                if (tile) {
                    tile->display();
                } else {
                    cout << " . ";  // Empty cell
                }
            }
            cout << "\n";
        }
    }
};

void readTiles(ifstream &f, vector<Tile*> &tiles){
    {
        string tmp;
        getline(f, tmp);
    }
    while(f.good()){
        Tile *t;
        string type;
        getline(f, type, ';');
        string s;
        getline(f, s);
        stringstream ss(s);
        map<Direction, set<Direction>> _flow;
        while(ss.good()){
            char delim;
            char from, to;
            ss >> from >> to >> delim;
            _flow[charToDir(from)].insert(charToDir(to));
            _flow[charToDir(to)].insert(charToDir(from));
        }
        if (type == "pipe"){
            t = new Pipe(_flow);
        } else if (type == "tap"){
            t = new Tap(_flow);
        }
        tiles.push_back(t);
    }
}

bool solve(Board &board, const Position &loadPos, const Position &sourcePos, vector<Tile *> &tiles, vector<int> &tileCounts, int rows, int cols) {
    if (board.canWaterFlow(sourcePos, loadPos)) {
        return true;
    }

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            Position pos = {r, c};

            if (board.isCellEmpty(pos)) {
                for (size_t i = 0; i < tiles.size(); ++i) {
                    if (tileCounts[i] > 0) {
                        Tile *tile = tiles[i];

                        for (int rot = 0; rot < 4; ++rot) {
                            board.placeTile(pos, tile);
                            --tileCounts[i];

                            if (solve(board, loadPos, sourcePos, tiles, tileCounts, rows, cols)) {
                                return true;
                            }

                            ++tileCounts[i];
                            board.removeTile(pos);
                            tile->rotateRight();
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool solve2(Board &board, const vector<Tile *> &tiles, set<int> &usedTiles, Position source, Position load) {
    if (board.canWaterFlow(source, load)) {
        cout << "\nSolved Board:\n";
        board.display();
        return true;
    }

    // Iterate through every position on the board
    for (int row = 0; row < board.getRows(); ++row) {
        for (int col = 0; col < board.getCols(); ++col) {
            Position pos = {row, col};

            if (board.getTileAt(pos) != nullptr) continue;  // Skip if there's already a tile at this position

            // Try placing every unused tile at this position
            for (int i = 0; i < tiles.size(); ++i) {
                if (usedTiles.count(i)) continue;  // Skip already used tiles

                Tile *tile = tiles[i];

                // Try all 4 rotations for the tile
                for (int rotation = 0; rotation < 4; ++rotation) {
                    board.placeTile(pos, tile);    // Place the tile
                    usedTiles.insert(i);          // Mark this tile as used

                    cout << "\nBoard State After Placement:\n";
                    board.display();              // Display the board state

                    // Recursively solve the board
                    if (solve2(board, tiles, usedTiles, source, load)) {
                        return true;
                    }

                    // Undo placement
                    board.removeTile(pos);
                    usedTiles.erase(i);
                    tile->rotateRight();          // Rotate the tile for the next try
                }
            }
        }
    }

    return false;  // If no valid placements work
}

int main() {
    int rows = 2, cols = 3;
    Board board(rows, cols);

    // Define the green source
    Source greenSource(G, {{E, {E}}});
    Position sourcePos = {0, 0};
    board.placeTile({0, 0}, &greenSource);

    // Define the green consumer
    Load greenLoad(G, {{W, {W}}});
    Position loadPos = {1, 2};
    board.placeTile({1, 2}, &greenLoad);

//    board.placeTile({0, 1}, new Pipe({{W, {S}}, {S, {W}}}));
//    board.placeTile({1, 1}, new Pipe({{N, {E}}, {E, {N}}}));
//    cout << board.canWaterFlow({0, 0}, {1, 2}) << endl;

    vector<Tile *> tiles = {
            new Pipe({{W, {N}}, {N, {W}}}),
            new Pipe({{W, {N}}, {N, {W}}})
    };
    tiles[0]->display();
    tiles[0]->rotateRight();
    tiles[0]->display();
    vector<int> tileCounts = {1, 1};

//    if (solve(board, loadPos, sourcePos, tiles, tileCounts, rows, cols)) {
//        cout << "Solution found!\n";
//    } else {
//        cout << "No solution exists.\n";
//    }
    set<int> usedTiles;
    if (solve2(board, tiles, usedTiles, sourcePos, loadPos)) {
        cout << "Solution found!\n";
    } else {
        cout << "No solution exists.\n";
    }

    // Cleanup
    for (Tile *tile : tiles) {
        delete tile;
    }

    return 0;
}
