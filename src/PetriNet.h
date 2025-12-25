#pragma once
#include "Header.h"

class PetriNet {
public:
    // жизненный цикл
    PetriNet();
    PetriNet(int places, int transitions);

    void clear();
    bool isEmpty() const;

    // задание сети
    void setSize(int places, int transitions);
    void setWn(const vector<vector<int>>& wn);
    void setWp(const vector<vector<int>>& wp);
    void setInitialMarking(const vector<int>& marking);

    bool loadFromFile(const string& filename);
    bool saveToFile(const string& filename) const;
    void generateRandom(int maxArcWeight, int maxTokens);

    // анализ
    bool isTransitionEnabled(int t, const vector<int>& marking) const;
    vector<int> fireTransition(int t, const vector<int>& marking) const;

    // достижимость
    vector<vector<int>> buildReachabilitySet() const;
    void printReachabilityGraph() const;

    // KarpЦMiller
    void buildKarpMillerTree();
    void printKarpMillerTree() const;

    // вывод
    void printPetriNet() const;

    // геттеры
    int getPlacesCount() const;
    int getTransitionsCount() const;
    vector<int> getInitialMarking() const;

    void buildReachabilityAdjListClassic() const;
private:
    static const int OMEGA = -1; // бесконечность

    int P = 0, T = 0; // p - позиции t переходы
    vector<vector<int>> Wn, Wp, W; // матрица входных дуг матрица выходных дуг матрица изменени€ разметки расход приход итог 
    vector<int> u0; // начальна€ разметка

    void computeW();

    struct KMNode {
        vector<int> marking;
        KMNode* parent;
        vector<pair<int, KMNode*>> children;
    };

    KMNode* kmRoot = nullptr;

    bool isEnabledWithOmega(int t, const vector<int>& m) const;
    vector<int> fireWithOmega(int t, const vector<int>& m) const;
    bool markingGE(const vector<int>& a, const vector<int>& b) const;
    bool markingEQ(const vector<int>& a, const vector<int>& b) const;
    bool hasOmega(const vector<int>& m) const;

    void expandKM(KMNode* node);

    // печать
    string markingToString(const vector<int>& m) const;
    void printKMNode(const KMNode* node, const string& prefix, bool isLast, ostream& os) const;
};
