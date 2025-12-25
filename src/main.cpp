#include "PetriNet.h"
#include <clocale>
#include <chrono>
#include <thread>
#include <cstdlib>

void printMenu() {
    cout << "\n\tСЕТИ ПЕТРИ\n";
    cout << "\n";
    cout << "1. Создать случайную сеть\n";
    cout << "2. Загрузить сеть из файла\n";
    cout << "3. Показать сеть Петри\n";
    cout << "4. Граф достижимости\n";
    cout << "5. Дерево Карпа–Миллера\n";
    cout << "6. Представление в виде списка смежности\n";
    cout << "0. Выход\n";
    cout << "Выбор: ";
}

int main() {
    setlocale(LC_ALL, "Russian");

    std::cout << "Курсовая работа по дисциплине : Логика и основы алгоритмизации в инженерных задачах\n"
        << "на тему: Реализация алгоритма для построения графа достижимости сетей Петри\n"
        << "Выполнил студент группы 24ВВВ1 : Марушкин Данила\n";

    std::this_thread::sleep_for(std::chrono::seconds(3));  // задержка 3 секунды
    std::system("cls");  // очистка консоли

    PetriNet net;
    bool hasNet = false;

    while (true) {
        printMenu();

        int cmd;
        cin >> cmd;

        if (cmd == 0) break;

        switch (cmd) {

        case 1: { // случайная сеть
            int P, T, aw, tk;
            cout << "Число позиций: ";
            cin >> P;
            cout << "Число переходов: ";
            cin >> T;
            cout << "Максимальный вес дуги: ";
            cin >> aw;
            cout << "Максимальное число фишек: ";
            cin >> tk;

            net.setSize(P, T);
            net.generateRandom(aw, tk);
            hasNet = true;

            string fname;
            cout << "Введите имя файла для сохранения: ";
            cin >> fname;

            if (net.saveToFile(fname))
                cout << "Сеть сохранена в файл: " << fname << endl;
            else
                cout << "Ошибка сохранения файла\n";

            break;
        }

        case 2: { // загрузка из файла
            string name;
            cout << "Введите имя файла: ";
            cin >> name;

            if (net.loadFromFile(name)) {
                hasNet = true;
                cout << "Сеть успешно загружена\n";
            }
            else {
                cout << "Ошибка загрузки файла\n";
            }
            break;
        }

        case 3: { // показать сеть
            if (!hasNet) {
                cout << "Сеть не загружена\n";
                break;
            }
            net.printPetriNet();
            break;
        }

        case 4: { // граф достижимости
            if (!hasNet) {
                cout << "Сеть не загружена\n";
                break;
            }
            net.printReachabilityGraph();
            break;
        }

        case 5: { // Карп–Миллер
            if (!hasNet) {
                cout << "Сеть не загружена\n";
                break;
            }
            net.buildKarpMillerTree();
            net.printKarpMillerTree();
            break;
        }
        case 6: {
            if (!hasNet) {
                cout << "Сеть не загружена\n";
                break;
            }
            net.buildReachabilityAdjListClassic();
            cout << "Сипсок смежности графа\n\n";

            cout << "\n";
            break;
        }
        default:
            cout << "Неверный пункт меню\n";
        }
    }

    return 0;
}