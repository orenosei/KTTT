#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <iomanip>
#include <cstdlib>
#include <ctime>

using namespace std;

// Định nghĩa số phức double
typedef complex<double> Complex;

// ============================================================
// BIẾN TOÀN CỤC HỆ THỐNG
// ============================================================
int K_directions;      // Số hướng (K)
int M_antennas;        // Số ăng-ten (M)
vector<vector<Complex>> E_proj; // Ma trận lỗi chiếu: (A*A_dagger - I)
vector<double> Desired_Mag;     // Vector biên độ mong muốn

// ============================================================
// KHỞI TẠO TEST CASE CỤ THỂ
// ============================================================


void initSampleTestCase() {
    // 1. Cấu hình kích thước
    K_directions = 3;
    M_antennas = 2;

    // 2. Cấu hình Vector biên độ mong muốn D_v
    // Ma trận D_v đường chéo [1, 1, 0] -> chuyển thành vector
    Desired_Mag = {1.0, 1.0, 0.0};

    // 3. Cấu hình Ma trận lỗi chiếu E_proj
    // Dựa trên tính toán: E_proj = diag(0, 0, -1)
    // Hàng 0: [0, 0, 0]
    // Hàng 1: [0, 0, 0]
    // Hàng 2: [0, 0, -1]

    E_proj.assign(K_directions, vector<Complex>(K_directions, Complex(0, 0)));

    // Gán giá trị cụ thể cho phần tử khác 0 duy nhất tại (2,2)
    E_proj[2][2] = Complex(-1.0, 0.0);

    cout << "--- System Initialization ---\n";
    cout << "K (Directions) = " << K_directions << ", M (Antennas) = " << M_antennas << "\n";
    // cout << "Desired Magnitude Vector: [1, 1, 0]\n";
    // cout << "Projection Error Matrix E calculated from A.\n\n";
}

void initSpecificTestCase() {
    // 1. Cấu hình kích thước
    K_directions = 5;
    M_antennas   = 3;

    // 2. Vector biên độ mong muốn
    Desired_Mag = {1.0, 0.8, 0.6, 0.4, 0.2};

    // 3. Ma trận lỗi chiếu E_proj (Hermitian, phức)
    E_proj.assign(K_directions, vector<Complex>(K_directions, Complex(0, 0)));

    E_proj[0][1] = Complex(0,  0.2);
    E_proj[1][0] = Complex(0, -0.2);

    E_proj[1][1] = Complex(0.3, 0);
    E_proj[1][2] = Complex(0.1, 0);
    E_proj[2][1] = Complex(0.1, 0);

    E_proj[2][2] = Complex(0.5, 0);
    E_proj[2][3] = Complex(0,  0.2);
    E_proj[3][2] = Complex(0, -0.2);

    E_proj[3][3] = Complex(0.7, 0);
    E_proj[3][4] = Complex(0.1, 0);
    E_proj[4][3] = Complex(0.1, 0);

    E_proj[4][4] = Complex(1.0, 0);

    cout << "--- Complex High-Dimensional Test Case Initialized ---\n";
    cout << "K = " << K_directions << ", M = " << M_antennas << "\n";
    // cout << "Desired Magnitude Vector: ";
    // for (double d : Desired_Mag) cout << d << " ";
    // cout << "\n\n";
}

void initVeryHardLargeTestCase() {
    // ===============================
    // 1. Kích thước rất lớn
    // ===============================
    K_directions = 32;     // Số hướng rất lớn
    M_antennas   = 8;      // Antenna ít hơn nhiều

    // ===============================
    // 2. Vector biên độ mong muốn
    // Giảm dần + nhiễu nhỏ
    // ===============================
    Desired_Mag.resize(K_directions);
    for (int k = 0; k < K_directions; k++) {
        Desired_Mag[k] = exp(-0.08 * k) + 0.05 * sin(0.7 * k);
    }

    // ===============================
    // 3. Ma trận lỗi chiếu E_proj
    // Hermitian + dense + complex
    // ===============================
    E_proj.assign(
        K_directions,
        vector<Complex>(K_directions, Complex(0.0, 0.0))
    );

    srand(2025); // cố định seed để tái lập

    // (a) Thành phần đường chéo chính – tăng dần
    for (int i = 0; i < K_directions; i++) {
        double val = 0.2 + 0.03 * i;
        E_proj[i][i] = Complex(val, 0.0);
    }

    // (b) Coupling lân cận (banded)
    for (int i = 0; i < K_directions; i++) {
        for (int j = max(0, i - 2); j <= min(K_directions - 1, i + 2); j++) {
            if (i != j) {
                double re = 0.05 * cos(0.4 * (i + j));
                double im = 0.05 * sin(0.6 * (i - j));
                E_proj[i][j] = Complex(re, im);
                E_proj[j][i] = conj(E_proj[i][j]);
            }
        }
    }

    // (c) Coupling xa – sparse nhưng mạnh
    for (int t = 0; t < K_directions * 2; t++) {
        int i = rand() % K_directions;
        int j = rand() % K_directions;
        if (i != j) {
            double re = ((rand() % 100) / 100.0 - 0.5) * 0.2;
            double im = ((rand() % 100) / 100.0 - 0.5) * 0.2;
            E_proj[i][j] += Complex(re, im);
            E_proj[j][i] = conj(E_proj[i][j]);
        }
    }

    // ===============================
    // 4. Log thông tin
    // ===============================
    cout << "--- VERY HARD LARGE-SCALE TEST CASE INITIALIZED ---\n";
    cout << "K (directions) = " << K_directions << "\n";
    cout << "M (antennas)   = " << M_antennas << "\n";
    // cout << "Desired Magnitude (first 10): ";
    // for (int i = 0; i < 10; i++)
    //     cout << Desired_Mag[i] << " ";
    // cout << "\n";
}

// ============================================================
// HÀM MỤC TIÊU (Objective Function)
// J = || (A*A_dagger - I) * D_v * p_v ||^2
// ============================================================
double objectiveFunction(const vector<double>& x) {
    int K = x.size();

    // Bước 1: Tạo vector v = D_v * p_v
    // p_v được tạo từ góc pha x: p_v[k] = e^(j * x[k])
    vector<Complex> v_vec(K);
    for(int i = 0; i < K; i++) {
        Complex p_v_i = polar(1.0, x[i]); // Chuyển góc sang số phức đơn vị
        v_vec[i] = Desired_Mag[i] * p_v_i;
    }

    // Bước 2: Tính vector lỗi = E_proj * v_vec
    vector<Complex> error_vec(K, Complex(0, 0));
    for(int i = 0; i < K; i++) {
        for(int j = 0; j < K; j++) {
            // Nhân ma trận số phức thông thường
            error_vec[i] += E_proj[i][j] * v_vec[j];
        }
    }

    // Bước 3: Tính chuẩn L2 bình phương
    double sum_squared_error = 0.0;
    for(int i = 0; i < K; i++) {
        sum_squared_error += norm(error_vec[i]);
    }

    return sum_squared_error;
}

// Hàm Fitness (chuyển đổi Min problem sang Max problem)
double fitness(double f) {
    if (f >= 0) return 1.0 / (1.0 + f);
    else return 1.0 + fabs(f);
}

// ============================================================
// MAIN - ABC ALGORITHM
// ============================================================
int main() {
    srand(time(NULL));

    // Khởi tạo dữ liệu Test Case
    //initSampleTestCase();
    //initSpecificTestCase();
    initVeryHardLargeTestCase();

    // Tham số thuật toán ABC
    int D = K_directions;    // Số chiều = 3
    int SN = 20;             // Số lượng ong
    int foodNumber = SN / 2;
    int limit = 50;
    int executedCycle = 0;
    int MaxCycle = 10000;      
    double C = 1.5;          // Tham số C

    const double PI = 3.14159265358979323846;
    double LB = -PI, UB = PI; // Tìm kiếm góc pha từ -PI đến PI

    // Cấu trúc dữ liệu
    vector<vector<double>> x(foodNumber, vector<double>(D));
    vector<double> f(foodNumber), fit(foodNumber);
    vector<int> trial(foodNumber, 0);

    // --- 1. KHỞI TẠO NGẪU NHIÊN ---
    for (int i = 0; i < foodNumber; i++) {
        for (int j = 0; j < D; j++) {
            x[i][j] = LB + (UB - LB) * ((double)rand() / RAND_MAX);
        }
        f[i] = objectiveFunction(x[i]);
        fit[i] = fitness(f[i]);
    }

    // Lưu nghiệm tốt nhất
    vector<double> bestSolution = x[0];
    double bestValue = f[0];

    auto updateBest = [&]() {
        for (int i = 0; i < foodNumber; i++) {
            if (f[i] < bestValue) { // Bài toán tìm Min
                bestValue = f[i];
                bestSolution = x[i];
            }
        }
    };
    updateBest();

    // --- 2. VÒNG LẶP CHÍNH ---
    for (int cycle = 1; cycle <= MaxCycle; cycle++) {
        executedCycle = cycle;
        if (cycle%100==0){
            cout << "Cycle: " << cycle << ", Best Error: " << bestValue << "\n";
        }

        // --- Giai đoạn Ong Thợ (Employed Bees) ---
        for (int i = 0; i < foodNumber; i++) {
            int k;
            do { k = rand() % foodNumber; } while (k == i);

            vector<double> v = x[i];
            int j = rand() % D;
            double phi = (double)rand() / RAND_MAX * 2 - 1; // [-1, 1]
            double psi = (double)rand() / RAND_MAX * C; // [0, C]

            v[j] = x[i][j] + phi * (x[i][j] - x[k][j]) + psi * (bestSolution[j]-x[k][j]);
            v[j] = min(UB, max(LB, v[j])); // Kẹp biên

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

        // --- Giai đoạn Ong Quan Sát (Onlooker Bees) ---
        double totalFit = 0;
        for (double val : fit) totalFit += val;

        int m = 0, i = 0;
        while (m < foodNumber) {
            double r = ((double)rand() / RAND_MAX) * totalFit;
            if (r < fit[i]) {
                int k;
                do { k = rand() % foodNumber; } while (k == i);

                vector<double> v = x[i];
                int j = rand() % D;
                double phi = (double)rand() / RAND_MAX * 2 - 1;
                double psi = (double)rand() / RAND_MAX * C;

                v[j] = x[i][j] + phi * (x[i][j] - x[k][j]) + psi * (bestSolution[j]-x[k][j]);
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
                m++;
            }
            i++;
            if (i == foodNumber) i = 0;
        }

        // --- Giai đoạn Ong Trinh Sát (Scout Bees) ---
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

        // Nếu đạt được lỗi tuyệt đối = 0 thì có thể dừng sớm (tùy chọn)
        if (bestValue < 1e-9) {
            cout << "Converged early at cycle " << cycle << endl;
            break;
        }
    }

    // ============================================================
    // IN KẾT QUẢ
    // ============================================================
    cout << "\n--- Optimization Results ---\n";
    cout << "Number of ABC cycles executed: " << executedCycle << "\n";
    cout << "Best Error (Objective Function Value): " << bestValue << "\n";
   //cout << "Best Phase Angles (Radians): [ ";
    // cout << fixed << setprecision(4);
    // for (double val : bestSolution) cout << val << " ";
    // cout << "]\n";

    return 0;
}
