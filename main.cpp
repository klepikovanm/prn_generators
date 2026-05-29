#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <string>
#include <cstdint>
#include <random>
#include <chrono>

using namespace std;
using namespace chrono;

// ====== Квадратичный конгруэнтный + перемешивание =====
class ModCongruent {
public: 
    const uint64_t M_1 = 2147483647;
    const uint64_t k1_1 = 69069;
    const uint64_t k2_1 = 1664525;
    const uint64_t b_1 = 1013904223;
    uint64_t state;

    ModCongruent(uint64_t seed) : state(seed % M_1 + 1) {}

    uint32_t next_num() {
        state = (k1_1 * (state * state) + k2_1 * state + b_1) % M_1;

        uint32_t temp = (uint32_t)((state * 0x100000000) / M_1);
        uint32_t left = (temp << 8) | (temp >> 24);
        uint32_t right = (temp >> 8) | (temp << 24);
        return left + right;
    }

    double next_double() { return (double)next_num() / (double)(0x100000000); }
};

// ===== Серединные квадраты + серединные произведения =====
class ModMid {
public:
    uint64_t prev;
    uint64_t curr;
    int step;

    ModMid(uint64_t seed) : prev(seed | 1), curr(seed | 1), step(0) {}

    uint32_t next_num() {
        uint64_t temp;
        if (step % 2 == 0)
            temp = curr * curr;
        else
            temp = curr * prev;
        uint64_t mid = (temp >> 16) & 0xFFFFFFFF;

        if (mid == 0)
            mid = (uint64_t)(step + 1) * 2654435761 & 0xFFFFFFFF;

        prev = curr;
        curr = mid;
        step++;
        return curr;
    }

    double next_double() { return (double)next_num() / (double)(0x100000000); }
};

// ===== Параллельный Фибоначчи + линейный конгруэнтный =====
class ModFib {
public:
    const int p = 17;
    const int q = 5;
    const uint64_t M_3 = 2147483647;
    const uint64_t k_3 = 1220703125;
    const uint64_t b_3 = 7;
    const uint64_t r0_3 = 7;
    const uint64_t M_fib = 0x100000000;

    uint32_t buf[17];
    int i;

    ModFib(uint64_t seed) : i(0) { init_buf(seed); }

    uint32_t next_num() {
        int ip = i;
        int iq = (i + q) % p;
        uint64_t first = (buf[ip] + buf[iq]) % M_fib;

        uint64_t second = (k_3 * first + b_3) % M_3;

        buf[i] = second;
        i = (i + 1) % p;
        return second;
    }

    double next_double() { return (double)next_num() / (double)M_3; }

private:
    void init_buf(uint64_t seed) {
        uint64_t r = (seed == 0) ? r0_3 : (seed % (M_3 - 1) + 1);
        for (int i = 0; i < p; i++) {
            r = (k_3 * r + b_3) % M_3;
            buf[i] = r;
        }
    }
};

// ===== Среднее, отклонение и коэффициент вариации =====
double Mean(const vector<double>& data) {
    double sum = 0;
    for (double d : data) 
        sum += d;
    return sum / data.size();
}

double Devation(const vector<double>& data, double mean) {
    double temp = 0;
    for (double d : data)
        temp += (d - mean) * (d - mean);
    return sqrt(temp / data.size());
}

double CV(double dev, double mean) {
    return dev / mean * 100.0;
}

// =================== Хи-квадрат ===================
double Chi2(const vector<double>& data) {
    const int k = 11;
    vector<int> count(k, 0);

    for (double d : data) {
        int i = (int)(d * k);
        count[i]++;
    }

    double E = (double)data.size() / k;
    double chi2 = 0;

    for (int i = 0; i < k; i++) {
        double d = count[i] - E;
        chi2 += d * d / E;
    }
    return chi2;
}

// =================== NIST-тесты ======================
double igamc(double a, double x) {
    double sum = 1.0, term = 1.0;
    for (int n = 1; n < 100; n++) {
        term *= x / (a + n);
        sum  += term;
    }
    double value = exp(-x + a * log(x) - lgamma(a)) * sum / a;
    return 1.0 - value;
}
 
vector<int> to_bits(const vector<double>& data) {
    vector<int> bits;
    for (double d : data) {
        uint32_t raw = (uint32_t)(d * 256);
        for (int b = 7; b >= 0; b--)
            bits.push_back((raw >> b) & 1);
    }
    return bits;
}

double test_frequency(const vector<int>& bits) {
    int n = bits.size(), S = 0;
    for (int b : bits) 
        S += (b == 1) ? 1 : -1;
    return erfc(fabs((double)S) / sqrt((double)n) / sqrt(2.0));
}

double test_seria_common(const vector<int>& bits) {
    int n = bits.size(), ones = 0;
    for (int b : bits)
        ones += b;
    double pi = (double)ones / n;
    if (fabs(pi - 0.5) >= 2.0 / sqrt((double)n))
        return 0.0;
    int V = 1;
    for (int i = 1; i < n; i++)
        if (bits[i] != bits[i-1]) V++;
    return erfc(fabs((double)V - 2.0*n*pi*(1-pi)) / (2.0 * sqrt(2.0*n) * pi * (1-pi)) / sqrt(2.0));
}

double test_block_freq(const vector<int>& bits) {
    int M = 64;
    int n = bits.size(), N = n / M;
    if (N < 1) return 0.0;
    double chi2 = 0;
    for (int blk = 0; blk < N; blk++) {
        int ones = 0;
        for (int i = 0; i < M; i++) 
            ones += bits[blk*M + i];
        double pi = (double)ones / M - 0.5;
        chi2 += pi * pi;
    }
    return igamc((double)N/2, (chi2 * 4.0 * M)/2);
}

double test_cusum(const vector<int>& bits) {
    int n = bits.size(), S = 0, maxS = 0;
    for (int b : bits) {
        S += (b == 1) ? 1 : -1;
        if (abs(S) > maxS) 
            maxS = abs(S);
    }
    return erfc((double)maxS / sqrt((double)n) / sqrt(2.0));
}

double test_seria_local(const vector<int>& bits) {
    int n = bits.size();
    int count[4] = {0};
    for (int i = 0; i < n - 1; i++)
        count[bits[i] * 2 + bits[i+1]]++;
    double E = (double)(n - 1) / 4.0;
    double chi2 = 0;
    for (int i = 0; i < 4; i++) {
        double d = count[i] - E;
        chi2 += d * d / E;
    }
    return igamc(2, chi2/2);
}

int main() {
    const int NUM_SAMPLES = 20;
    const int SAMPLE_SIZE = 1000;
    const uint64_t SEED_STEP[3] = {314159265, 271828182, 161803398};

    cout << "\nГПСЧ" << "\n";
    const string names[3] = {
        "Квадратичный конгруэнтный + перемешивание",
        "Серединные квадраты + серединные произведения",
        "Параллельный Фибоначчи + линейный конгруэнтный"
    };

    uint64_t seeds[3] = {123456789, 987654321, 112358132134};

    const double RANGE = 5000.0;
    vector<vector<vector<double>>> data(3, vector<vector<double>>(NUM_SAMPLES));

    for (int s = 0; s < NUM_SAMPLES; s++) {
        { ModCongruent g(seeds[0]); seeds[0] += SEED_STEP[0];
          for (int i = 0; i < SAMPLE_SIZE; i++)
              data[0][s].push_back(g.next_double() * RANGE); }

        { ModMid g(seeds[1]); seeds[1] += SEED_STEP[1];
          for (int i = 0; i < SAMPLE_SIZE; i++)
              data[1][s].push_back(g.next_double() * RANGE); }

        { ModFib g(seeds[2]); seeds[2] += SEED_STEP[2];
          for (int i = 0; i < SAMPLE_SIZE; i++)
              data[2][s].push_back(g.next_double() * RANGE); }
    }

    for (int m = 0; m < 3; m++) {
        cout << "\n\t\t" << names[m] << "\n" << "\n";
        cout << "№" << "   \t" << "Среднее" << "\t\t" << "  СКО" << "\t\t" << "  КВ,%" << "\t\t" << "  χ²" << "\t" << "  Равномерное\n";
        cout << string(80, '-') << "\n";

        for (int s = 0; s < NUM_SAMPLES; s++) {
            vector<double> norm(SAMPLE_SIZE);
            for (int i = 0; i < SAMPLE_SIZE; i++)
                norm[i] = data[m][s][i] / RANGE;

            double mean = Mean(norm);
            double dev = Devation(norm, mean);
            double cv = CV(dev, mean);
            double chi = Chi2(norm);

            bool verdict = (3.94 < chi && chi < 18.31);

            cout << s+1 << "   \t" << mean << "   \t" << dev << "   \t" << cv << "   \t" << chi << "      \t" << (verdict ? "ДА" : "НЕТ") << "\n";
        }
    }
    cout << "\nNIST-тесты" << "\n";
    const string test_names[5] = {
        "Частотный тест           ", "Серийный тест (общий)    ", "Блочная частота (M=64)   ",
        "Кумулятивные суммы      ", "Серийный тест (локальный)"
    };
 
    for (int m = 0; m < 3; m++) {
        double pv_sum[5] = {0};
        int pv_pass[5] = {0};
 
        for (int s = 0; s < NUM_SAMPLES; s++) {
            vector<double> norm(SAMPLE_SIZE);
            for (int i = 0; i < SAMPLE_SIZE; i++)
                norm[i] = data[m][s][i] / RANGE;
            vector<int> bits = to_bits(norm);
 
            double pv[5] = {
                test_frequency(bits),
                test_seria_common(bits),
                test_block_freq(bits),
                test_cusum(bits),
                test_seria_local(bits)
            };
 
            for (int t = 0; t < 5; t++) {
                pv_sum[t] += pv[t];
                pv_pass[t] += (pv[t] > 0.01) ? 1 : 0;
            }
        }
 
        cout << "\n\t\t" << names[m] << "\n";
        cout << "Тест" << "\t\t\t" << "      Прошли" << "   \t" << "Ср. p-value" << "   \t" << "Итог\n";
        cout << string(80, '-') << "\n";
 
        for (int t = 0; t < 5; t++) {
            double P_val = pv_sum[t] / NUM_SAMPLES;
            int pass = pv_pass[t];
            string verdict;
            if (pass >= (int)(NUM_SAMPLES * 0.95))
                verdict = "ПРОШЕЛ";
            else if (pass >= (int)(NUM_SAMPLES * 0.80))
                verdict = "ЧАСТИЧНО";
            else
                verdict = "НЕ ПРОШЕЛ";
            
            cout << test_names[t] << "\t" << pass << "\t" << P_val << "    \t" << verdict << "\n";
        }
        cout << "\n";
    }

    vector<int> sizes = {1000, 2000, 5000, 10000, 20000, 100000, 200000, 500000, 1000000};

    ofstream csv("timing.csv");
    csv << "n;cong_ms;mid_ms;fib_ms;std_ms\n";

    for (int sz : sizes) {
        double vc, vm, vf, vs;
    
        auto t0 = high_resolution_clock::now();
        { ModCongruent g(123); uint64_t x = 0;
        for (int i = 0; i < sz; i++) x = g.next_num(); }
        vc = duration<double, milli>(high_resolution_clock::now() - t0).count();

        t0 = high_resolution_clock::now();
        { ModMid g(123); uint64_t x = 0;
        for (int i = 0; i < sz; i++) x = g.next_num(); }
        vm = duration<double, milli>(high_resolution_clock::now() - t0).count();

        t0 = high_resolution_clock::now();
        { ModFib g(123); uint64_t x = 0;
        for (int i = 0; i < sz; i++) x = g.next_num(); }
        vf = duration<double, milli>(high_resolution_clock::now() - t0).count();

        t0 = high_resolution_clock::now();
        { mt19937_64 g(123); uint64_t x = 0;
        for (int i = 0; i < sz; i++) x = g(); }
        vs = duration<double, milli>(high_resolution_clock::now() - t0).count();

        csv << sz << ";"
            << vc << ";"
            << vm << ";"
            << vf << ";"
            << vs << "\n";
    }
    csv.close();
    return 0;
}