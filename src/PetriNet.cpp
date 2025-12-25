#include "PetriNet.h"

PetriNet::PetriNet() {}

PetriNet::PetriNet(int places, int transitions) {
    setSize(places, transitions);
}

void PetriNet::clear() {
    P = 0; T = 0;
    Wn.clear(); Wp.clear(); W.clear();
    u0.clear();
    kmRoot = nullptr; // утечки не чистим 
}

bool PetriNet::isEmpty() const {
    return P <= 0 || T <= 0;
}

//  задание сети 
void PetriNet::setSize(int places, int transitions) {
    P = places;
    T = transitions;

    Wn.assign(P, vector<int>(T, 0));
    Wp.assign(P, vector<int>(T, 0));
    W.assign(P, vector<int>(T, 0));

    u0.assign(P, 0);
    kmRoot = nullptr;
}

void PetriNet::setWn(const vector<vector<int>>& wn) {
    Wn = wn;
    computeW();
}

void PetriNet::setWp(const vector<vector<int>>& wp) {
    Wp = wp;
    computeW();
}

void PetriNet::setInitialMarking(const vector<int>& marking) {
    u0 = marking;
}

void PetriNet::computeW() {
    if (P <= 0 || T <= 0) return;
    for (int i = 0; i < P; ++i)
        for (int t = 0; t < T; ++t)
            W[i][t] = Wp[i][t] - Wn[i][t];
}

bool PetriNet::loadFromFile(const string& filename) {
    ifstream in(filename);
    if (!in.is_open()) return false;

    string tag;
    int places = 0, transitions = 0;

    in >> tag >> places;       // "P <число>"
    in >> tag >> transitions;  // "T <число>"
    if (!in || places <= 0 || transitions <= 0) return false;

    setSize(places, transitions);

    in >> tag; // "Wn"
    if (!in || tag != "Wn") return false;
    for (int i = 0; i < P; ++i)
        for (int t = 0; t < T; ++t) {
            in >> Wn[i][t];
            if (!in || Wn[i][t] < 0) return false;
        }

    in >> tag; // "Wp"
    if (!in || tag != "Wp") return false;
    for (int i = 0; i < P; ++i)
        for (int t = 0; t < T; ++t) {
            in >> Wp[i][t];
            if (!in || Wp[i][t] < 0) return false;
        }

    in >> tag; // "u0"
    if (!in || tag != "u0") return false;
    for (int i = 0; i < P; ++i) {
        in >> u0[i];
        if (!in || u0[i] < 0) return false;
    }

    computeW();
    kmRoot = nullptr;
    return true;
}

bool PetriNet::saveToFile(const string& filename) const {
    ofstream out(filename);
    if (!out.is_open()) return false;

    out << "P " << P << "\n";
    out << "T " << T << "\n";

    out << "Wn\n";
    for (int i = 0; i < P; ++i) {
        for (int t = 0; t < T; ++t) out << Wn[i][t] << " ";
        out << "\n";
    }

    out << "Wp\n";
    for (int i = 0; i < P; ++i) {
        for (int t = 0; t < T; ++t) out << Wp[i][t] << " ";
        out << "\n";
    }

    out << "u0\n";
    for (int i = 0; i < P; ++i) out << u0[i] << " ";
    out << "\n";

    return true;
}

void PetriNet::generateRandom(int maxArcWeight, int maxTokens) {
    if (P <= 0 || T <= 0)
        throw runtime_error("generateRandom: setSize() must be called first.");
    if (maxArcWeight < 0 || maxTokens < 0)
        throw runtime_error("generateRandom: params must be >= 0.");

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> arcDist(0, maxArcWeight);
    uniform_int_distribution<int> tokDist(0, maxTokens);

    for (int i = 0; i < P; ++i)
        for (int t = 0; t < T; ++t) {
            Wn[i][t] = arcDist(gen);
            Wp[i][t] = arcDist(gen);
        }

    for (int i = 0; i < P; ++i)
        u0[i] = tokDist(gen);

    computeW();
    kmRoot = nullptr;
}

//  анализ 
bool PetriNet::isTransitionEnabled(int t, const vector<int>& marking) const {
    for (int i = 0; i < P; ++i)
        if (marking[i] < Wn[i][t]) return false;
    return true;
}

vector<int> PetriNet::fireTransition(int t, const vector<int>& marking) const {
    vector<int> r = marking;
    for (int i = 0; i < P; ++i)
        r[i] += W[i][t];
    return r;
}

//  достижимость 
vector<vector<int>> PetriNet::buildReachabilitySet() const {
    vector<vector<int>> result;
    queue<vector<int>> q;
    set<vector<int>> visited;

    q.push(u0);
    visited.insert(u0);

    while (!q.empty()) {
        auto cur = q.front(); q.pop();
        result.push_back(cur);

        for (int t = 0; t < T; ++t) {
            if (isTransitionEnabled(t, cur)) {
                auto nxt = fireTransition(t, cur);
                if (!visited.count(nxt)) {
                    visited.insert(nxt);
                    q.push(nxt);
                }
            }
        }
    }
    return result;
}

void PetriNet::printReachabilityGraph() const {
    const int MAX_OUTPUT = 10;

    ofstream out("output.txt");
    if (!out.is_open()) {
        cout << "Ошибка открытия файла output.txt\n";
        return;
    }

    cout << "Граф достижимости (ограниченный вывод):\n";
    out << "Граф достижимости (ограниченный вывод):\n";

    queue<vector<int>> q;
    set<vector<int>> visited;

    q.push(u0);
    visited.insert(u0);

    int printed = 0;

    while (!q.empty() && printed < MAX_OUTPUT) {
        vector<int> cur = q.front();
        q.pop();

        bool hasOutgoing = false;

        for (int t = 0; t < T; ++t) {
            if (isTransitionEnabled(t, cur)) {
                vector<int> next = fireTransition(t, cur);

                string line =
                    markingToString(cur) +
                    " --T" + to_string(t) + "--> " +
                    markingToString(next);

                cout << line << "\n";
                out << line << "\n";

                hasOutgoing = true;

                if (!visited.count(next)) {
                    visited.insert(next);
                    q.push(next);
                }
            }
        }

        if (!hasOutgoing) {
            string line = markingToString(cur) + " (тупик)";
            cout << line << "\n";
            out << line << "\n";
        }

        printed++;
    }

    if (!q.empty()) {
        cout << "...\n";
        cout << "Вывод остановлен после "
            << MAX_OUTPUT
            << " вершин (граф может быть бесконечным).\n";

        out << "...\n";
        out << "Вывод остановлен после "
            << MAX_OUTPUT
            << " вершин (граф может быть бесконечным).\n";
    }

    out.close();
}


// карп милер
bool PetriNet::isEnabledWithOmega(int t, const vector<int>& m) const {
    for (int i = 0; i < P; ++i) {
        if (m[i] == OMEGA) continue;
        if (m[i] < Wn[i][t]) return false;
    }
    return true;
}

vector<int> PetriNet::fireWithOmega(int t, const vector<int>& m) const {
    vector<int> r = m;
    for (int i = 0; i < P; ++i) {
        if (r[i] == OMEGA) continue;
        r[i] += W[i][t];
    }
    return r;
}

bool PetriNet::markingEQ(const vector<int>& a, const vector<int>& b) const {
    return a == b;
}

// a >= b и строго больше хотя бы в одной компоненте (или ω “делает больше”)
bool PetriNet::markingGE(const vector<int>& a, const vector<int>& b) const {
    bool strict = false;
    for (int i = 0; i < P; ++i) {
        if (a[i] == OMEGA && b[i] != OMEGA) {
            strict = true;
        }
        else if (a[i] != OMEGA && b[i] == OMEGA) {
            return false;
        }
        else if (a[i] < b[i]) {
            return false;
        }
        else if (a[i] > b[i]) {
            strict = true;
        }
    }
    return strict;
}

bool PetriNet::hasOmega(const vector<int>& m) const {
    for (int x : m) if (x == OMEGA) return true;
    return false;
}

void PetriNet::expandKM(KMNode* node) {
    for (int t = 0; t < T; ++t) {

        if (!isEnabledWithOmega(t, node->marking))
            continue;
        vector<int> next = fireWithOmega(t, node->marking);
        for (KMNode* p = node; p != nullptr; p = p->parent) {
            if (markingEQ(next, p->marking)) {
                
                node->children.push_back(
                    { t, new KMNode{ next, node } }
                );
                goto next_transition;
            }
        }

        for (KMNode* p = node; p != nullptr; p = p->parent) {
            if (markingGE(next, p->marking)) {
                for (int i = 0; i < P; ++i) {
                    if (p->marking[i] != OMEGA &&
                        next[i] != OMEGA &&
                        next[i] > p->marking[i]) {
                        next[i] = OMEGA;
                    }
                }
            }
        }

        {
            KMNode* child = new KMNode{ next, node };
            node->children.push_back({ t, child });

            bool repeated = false;
            for (KMNode* p = node; p != nullptr; p = p->parent) {
                if (markingEQ(next, p->marking)) {
                    repeated = true;
                    break;
                }
            }

            if (!repeated) {
                expandKM(child);
            }
        }

    next_transition:
        ;
    }
}


void PetriNet::buildKarpMillerTree() {
    kmRoot = new KMNode{ u0, nullptr };
    expandKM(kmRoot);
}

string PetriNet::markingToString(const vector<int>& m) const {
    string s = "[ ";
    for (int x : m) {
        if (x == OMEGA) s += "w ";
        else s += to_string(x) + " ";
    }
    s += "]";
    return s;
}

void PetriNet::printKMNode(const KMNode* node, const string& prefix, bool isLast, ostream& os) const {

    os << prefix << markingToString(node->marking) << "\n";

    for (size_t i = 0; i < node->children.size(); ++i) {
        bool last = (i + 1 == node->children.size());
        int t = node->children[i].first;
        const KMNode* child = node->children[i].second;

        os << prefix
            << (last ? "\\-- " : "+-- ")
            << "T" << t << " -> ";

        printKMNode(child,
            prefix + (last ? "    " : "|   "),
            last,
            os);
    }
}


void PetriNet::printKarpMillerTree() const {
    if (!kmRoot) {
        cout << "Дерево Карпа–Миллера пустое.\n";
        return;
    }

    ofstream out("output.txt");
    if (!out.is_open()) {
        cout << "Ошибка открытия файла output.txt\n";
        return;
    }

    cout << "Дерево Карпа–Миллера:\n";
    out << "Дерево Карпа–Миллера:\n";

    printKMNode(kmRoot, "", true, cout);
    printKMNode(kmRoot, "", true, out);

    out.close();
}


//  вывод сети
void PetriNet::printPetriNet() const {
    cout << "P = " << P << ", T = " << T << "\n";

    cout << "\nWn:\n";
    for (int i = 0; i < P; ++i) {
        for (int t = 0; t < T; ++t) cout << Wn[i][t] << " ";
        cout << "\n";
    }

    cout << "\nWp:\n";
    for (int i = 0; i < P; ++i) {
        for (int t = 0; t < T; ++t) cout << Wp[i][t] << " ";
        cout << "\n";
    }

    cout << "\nW = Wp - Wn:\n";
    for (int i = 0; i < P; ++i) {
        for (int t = 0; t < T; ++t) cout << W[i][t] << " ";
        cout << "\n";
    }

    cout << "\nНачальная маркировка u0:\n";
    cout << markingToString(u0) << "\n";
}

//  геттеры
int PetriNet::getPlacesCount() const { return P; }
int PetriNet::getTransitionsCount() const { return T; }
vector<int> PetriNet::getInitialMarking() const { return u0; }

void PetriNet::buildReachabilityAdjListClassic() const
{
    const int OMEGA = -1;

    // соответствие разметка -> номер вершины
    map<vector<int>, int> id;

    // номер -> разметка (для печати)
    vector<vector<int>> vertices;

    // классический список смежности
    vector<vector<int>> adj;

    queue<vector<int>> q;

    // начальная разметка
    id[u0] = 0;
    vertices.push_back(u0);
    adj.push_back({});
    q.push(u0);

    while (!q.empty()) {
        vector<int> current = q.front();
        q.pop();

        int from = id[current];

        for (int t = 0; t < T; ++t) {

            // проверка разрешимости 
            bool enabled = true;
            for (int i = 0; i < P; ++i) {
                if (current[i] != OMEGA && current[i] < Wn[i][t]) {
                    enabled = false;
                    break;
                }
            }
            if (!enabled) continue;

            // срабатывание перехода
            vector<int> next = current;
            for (int i = 0; i < P; ++i) {
                if (current[i] == OMEGA)
                    next[i] = OMEGA;
                else
                    next[i] += W[i][t];
            }

            // обобщение роста → w
            bool growth = false;
            for (int i = 0; i < P; ++i) {
                if (current[i] != OMEGA &&
                    next[i] != OMEGA &&
                    next[i] > current[i]) {
                    growth = true;
                }
            }
            if (growth) {
                for (int i = 0; i < P; ++i) {
                    if (current[i] != OMEGA && next[i] > current[i]) {
                        next[i] = OMEGA;
                    }
                }
            }

            // если вершина новая — нумеруем
            if (id.find(next) == id.end()) {
                int newId = (int)vertices.size();
                id[next] = newId;
                vertices.push_back(next);
                adj.push_back({});
                q.push(next);
            }

            int to = id[next];
            adj[from].push_back(to);
        }
    }


    cout << "Vertices (id -> marking):" << endl;
    for (size_t i = 0; i < vertices.size(); ++i) {
        cout << i << ": [ ";
        for (int x : vertices[i]) {
            if (x == OMEGA) cout << "w ";
            else cout << x << " ";
        }
        cout << "]" << endl;
    }

    cout << "\nAdjacency list:" << endl;
    for (size_t i = 0; i < adj.size(); ++i) {
        cout << i << ": ";
        for (int v : adj[i]) {
            cout << v << " ";
        }
        cout << endl;
    }
}