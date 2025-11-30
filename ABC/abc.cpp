#include <bits/stdc++.h>
using namespace std;

// ============================================================
// HÀM MỤC TIÊU
// Công thức: f(x) = x1*x2 + x3^2 + x4^3
// ============================================================
double objectiveFunction(const vector<double>& x) {
    if (x.size() < 4) return 0.0; 
    double term1 = x[0] * x[1];
    double term2 = x[2] * x[2];
    double term3 = x[3] * x[3] * x[3]; 
    
    return term1 + term2 + term3;
}

// Hàm fitness
double fitness(double f) {
    if (f >= 0) return 1.0 / (1.0 + f);
    else return 1.0 + fabs(f);
}

int main() {
    int D = 4;               
    int SN = 20;             // Số ong (employed + onlooker)
    int foodNumber = SN / 2; // Số nguồn thức ăn
    int limit = 30;          // Giới hạn trial
    int MaxCycle = 2000;     // Số vòng lặp

    double LB = -5.0, UB = 5.0; 

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

    // Lambda cập nhật best solution
    auto updateBest = [&]() {
        for (int i = 0; i < foodNumber; i++) {
            if (f[i] < bestValue) {
                bestValue = f[i];
                bestSolution = x[i];
            }
        }
    };
    
    // Cập nhật best ngay sau khi khởi tạo
    updateBest();

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
        for (double val : fit) totalFit += val;

        for (int t = 0; t < foodNumber; t++) {
            double r = ((double)rand() / RAND_MAX) * totalFit;

            double acc = 0;
            int i = 0;
            for (i = 0; i < foodNumber; i++) {
                acc += fit[i];
                if (acc >= r) break;
            }
            // An toàn biên: nếu vòng lặp chạy hết mà acc < r (do sai số float)
            if(i >= foodNumber) i = foodNumber - 1;

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

    cout << "Optimization completed.\n";
    cout << "Function: f(x) = x1*x2 + x3^2 + x4^3\n";
    cout << "Number of dimensions: " << D << "\n";
    cout << "Search space: [" << LB << ", " << UB << "]\n";
    cout << "Best value found = " << bestValue << "\n";
    cout << "Best solution: ";
    for (double v : bestSolution) cout << v << " ";
    cout << "\n";

    return 0;
}
