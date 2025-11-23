#include <bits/stdc++.h>
using namespace std;

// Hàm Rastrigin
// Global minimum tại x = 0 với giá trị = 0
// Miền tìm kiếm tiêu chuẩn: [-5.12, 5.12]
double objectiveFunction(const vector<double>& x) {
    double sum = 0;
    const double A = 10;
    const double PI = 3.14159265358979323846;
    
    for (double v : x) {
        sum += (v * v - A * cos(2 * PI * v));
    }
    
    return A * x.size() + sum;
}
// Hàm fitness
double fitness(double f) {
    if (f >= 0) return 1.0 / (1.0 + f);
    else return 1.0 + fabs(f);
}

int main() {
    int D = 5;               // Số chiều nghiệm
    int SN = 20;             // Số ong (employed + onlooker)
    int foodNumber = SN / 2; // Số nguồn thức ăn
    int limit = 30;          // Giới hạn trial
    int MaxCycle = 2000;     // Số vòng lặp

    double LB = -5.12, UB = 5.12; // Miền tìm kiếm chuẩn của Rastrigin

    vector<vector<double>> x(foodNumber, vector<double>(D));
    vector<double> f(foodNumber), fit(foodNumber);
    vector<int> trial(foodNumber, 0);

    // ============================================================
    // 1. KHỞI TẠO NGẪU NHIÊN
    // ============================================================
    srand(time(NULL));

    for (int i = 0; i < foodNumber; i++) {
        for (int j = 0; j < D; j++) {
            x[i][j] = LB + (UB - LB) * ((double)rand() / RAND_MAX);
        }
        f[i] = objectiveFunction(x[i]);
        fit[i] = fitness(f[i]);
    }

    vector<double> bestSolution = x[0];
    double bestValue = f[0];

    auto updateBest = [&]() {
        for (int i = 0; i < foodNumber; i++) {
            if (f[i] < bestValue) {
                bestValue = f[i];
                bestSolution = x[i];
            }
        }
    };

    // ============================================================
    // 2. VÒNG LẶP CHÍNH ABC
    // ============================================================
    for (int cycle = 1; cycle <= MaxCycle; cycle++) {

        // =======================
        // --- Employed Bees -----
        // =======================
        for (int i = 0; i < foodNumber; i++) {
            int k;
            do { k = rand() % foodNumber; } while (k == i);

            vector<double> v = x[i];

            int j = rand() % D;
            double phi = (double)rand() / RAND_MAX * 2 - 1; // [-1,1]

            v[j] = x[i][j] + phi * (x[i][j] - x[k][j]);

            // Giới hạn trong miền tìm kiếm
            v[j] = min(UB, max(LB, v[j]));

            double fv = objectiveFunction(v);
            double fitv = fitness(fv);

            if (fitv > fit[i]) {
                x[i] = v;
                f[i] = fv;
                fit[i] = fitv;
                trial[i] = 0;
            } else {
                trial[i]++;
            }
        }

        // =======================
        // --- Onlooker Bees ----
        // =======================
        double totalFit = 0;
        for (double v : fit) totalFit += v;

        for (int t = 0; t < foodNumber; t++) {
            double r = ((double)rand() / RAND_MAX) * totalFit;

            // Roulette wheel selection
            double acc = 0;
            int i;
            for (i = 0; i < foodNumber; i++) {
                acc += fit[i];
                if (acc >= r) break;
            }

            int k;
            do { k = rand() % foodNumber; } while (k == i);

            vector<double> v = x[i];
            int j = rand() % D;
            double phi = (double)rand() / RAND_MAX * 2 - 1;

            v[j] = x[i][j] + phi * (x[i][j] - x[k][j]);
            v[j] = min(UB, max(LB, v[j]));

            double fv = objectiveFunction(v);
            double fitv = fitness(fv);

            if (fitv > fit[i]) {
                x[i] = v;
                f[i] = fv;
                fit[i] = fitv;
                trial[i] = 0;
            } else {
                trial[i]++;
            }
        }

        // =======================
        // --- Scout Bees -------
        // =======================
        for (int i = 0; i < foodNumber; i++) {
            if (trial[i] > limit) {
                for (int j = 0; j < D; j++) {
                    x[i][j] = LB + (UB - LB) * ((double)rand() / RAND_MAX);
                }
                f[i] = objectiveFunction(x[i]);
                fit[i] = fitness(f[i]);
                trial[i] = 0;
            }
        }

        updateBest();
    }

    // ============================================================
    // IN RA KẾT QUẢ
    // ============================================================
    cout << "Optimization completed.\n";
    cout << "Number of dimensions: " << D << "\n";
    cout << "Number of food sources: " << foodNumber << "\n";
    cout << "Search space: [" << LB << ", " << UB << "]\n";
    cout << "Maximum cycles: " << MaxCycle << "\n";
    cout << "Limit for scout bees: " << limit << "\n";
    cout << "Best value found = " << bestValue << "\n";
    cout << "Best solution: ";
    for (double v : bestSolution) cout << v << " ";
    cout << "\n";

    return 0;
}
