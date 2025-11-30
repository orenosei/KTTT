#include <bits/stdc++.h>
using namespace std;

// ============================================================
// HÀM MỤC TIÊU
// f(x) = x1*x2 + x3^2 + x4^3
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

// Hàm kẹp giá trị
double clamp(double val, double lb, double ub) {
    return min(ub, max(lb, val));
}

int main() {
    int D = 4;               
    int SN = 20;             
    int foodNumber = SN / 2; 
    int limit = 30;          
    int MaxCycle = 2000;     

    double LB = -5.0, UB = 5.0; 
    double epsilon = 1e-5;   // Sai số cho GSS

    vector<vector<double>> x(foodNumber, vector<double>(D));
    vector<double> f(foodNumber), fit(foodNumber);
    vector<int> trial(foodNumber, 0);

    // ============================================================
    // 1. KHỞI TẠO
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
    
    updateBest();

    // ============================================================
    // 2. VÒNG LẶP CHÍNH
    // ============================================================
    for (int cycle = 1; cycle <= MaxCycle; cycle++) {

        // --- PHA 1: ONG THỢ ---
        for (int i = 0; i < foodNumber; i++) {
            int k;
            do { k = rand() % foodNumber; } while (k == i);

            vector<double> v = x[i];
            int j = rand() % D;
            double phi = (double)rand() / RAND_MAX * 2 - 1; 

            v[j] = x[i][j] + phi * (x[i][j] - x[k][j]);
            v[j] = clamp(v[j], LB, UB);

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

        // --- PHA 2: ONG QUAN SÁT  ---
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
            if(i >= foodNumber) i = foodNumber - 1;

            int k;
            do { k = rand() % foodNumber; } while (k == i);

            vector<double> v = x[i];
            int j = rand() % D;
            double phi = (double)rand() / RAND_MAX * 2 - 1;

            v[j] = x[i][j] + phi * (x[i][j] - x[k][j]);
            v[j] = clamp(v[j], LB, UB);

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

        // --- PHA 3: ONG TRINH SÁT ---
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

        // --- PHA 4: MEMETIC SEARCH  ---
        // Tinh chỉnh nghiệm tốt nhất bằng Golden Section Search
        
        int j_mem = rand() % D;          
        int k_mem = rand() % foodNumber; 
        
        auto evaluateGSS = [&](double F) -> double {
            vector<double> tempX = bestSolution;
            tempX[j_mem] = bestSolution[j_mem] + F * (bestSolution[j_mem] - x[k_mem][j_mem]);
            tempX[j_mem] = clamp(tempX[j_mem], LB, UB);
            return objectiveFunction(tempX);
        };

        double a_gss = -1.0; 
        double b_gss = 1.0;
        double ratio = 0.618; 
        
        double F1 = b_gss - (b_gss - a_gss) * ratio;
        double F2 = a_gss + (b_gss - a_gss) * ratio;
        
        while (abs(b_gss - a_gss) > epsilon) {
            double val1 = evaluateGSS(F1);
            double val2 = evaluateGSS(F2);

            if (val1 < val2) {
                b_gss = F2;
                F2 = F1;
                F1 = b_gss - (b_gss - a_gss) * ratio;
                
                if (val1 < bestValue) {
                    bestValue = val1;
                    bestSolution[j_mem] += F1 * (bestSolution[j_mem] - x[k_mem][j_mem]);
                    bestSolution[j_mem] = clamp(bestSolution[j_mem], LB, UB);
                }
            } else {
                a_gss = F1;
                F1 = F2;
                F2 = a_gss + (b_gss - a_gss) * ratio;
                
                if (val2 < bestValue) {
                    bestValue = val2;
                    bestSolution[j_mem] += F2 * (bestSolution[j_mem] - x[k_mem][j_mem]);
                    bestSolution[j_mem] = clamp(bestSolution[j_mem], LB, UB);
                }
            }
        }
    }

    cout << "Optimization completed.\n";
    cout << "Function: f(x) = x1*x2 + x3^2 + x4^3\n";
    cout << "Search space: [" << LB << ", " << UB << "]\n";
    cout << "Best value found = " << bestValue << "\n";
    cout << "Best solution: ";
    for (double v : bestSolution) cout << v << " ";
    cout << "\n";

    return 0;
}