#include <iostream>
#include <map>
#include <list>
#include <vector>
#include <fstream>
#include <sstream>

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



class Tile{
    map<Direction, list<Direction>> flow;
public:
    Tile(map<Direction, list<Direction>> _flow): flow(_flow){}

    void rotateRight(){
        map<Direction, list<Direction>> newFlow;
        for (auto fldir: flow){
            newFlow[++fldir.first];
            for (auto &dir:fldir.second){
                ++dir;
            }
        }
        flow = newFlow;
    }
};

class Pipe: public Tile{
public:
    Pipe(map<Direction, list<Direction>> _flow): Tile(_flow){}
};

class Tap: public Tile{
    bool open = true;
public:
    Tap(map<Direction, list<Direction>> _flow): Tile(_flow){}
};

class Source: protected Tile{
    Color color;
};

class Load: protected Tile{
    Color color;
};

class Board{
    vector<vector<Tile*>> field;

public:
    Board(vector<vector<Tile*>> _field): field(_field){};

};

Direction charToDir(char c){
    switch (c){
        case 'N': return N;
        case 'S': return S;
        case 'W': return W;
        case 'E': return E;
        default: throw std::invalid_argument("Invalid Direction");
    }
}

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
        map<Direction, list<Direction>> _flow;
        while(ss.good()){
            char delim;
            char from, to;
            ss >> from >> to >> delim;
            _flow[charToDir(from)].push_back(charToDir(to));
            _flow[charToDir(to)].push_back(charToDir(from));
        }
        if (type == "pipe"){
            t = new Pipe(_flow);
        } else if (type == "tap"){
            t = new Tap(_flow);
        }
        tiles.push_back(t);
    }
}

int main() {
    vector<Tile*> tiles;
    ifstream f("tiles.csv");
    readTiles(f, tiles);
    return 0;
}
