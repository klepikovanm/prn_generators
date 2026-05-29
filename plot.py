import csv
import numpy as np
import matplotlib.pyplot as plt

sizes = []
cong = []
mid = []
fib = []
std_mt = []

with open('timing.csv', newline='', encoding='utf-8') as f:
    reader = csv.reader(f, delimiter=';')  # Изменил на ;
    next(reader)
    for row in reader:
        sizes.append(int(row[0]))
        cong.append(float(row[1]))
        mid.append(float(row[2]))
        fib.append(float(row[3]))
        std_mt.append(float(row[4]))

x = np.arange(len(sizes))

plt.figure(figsize=(12, 6))
plt.plot(x, cong,    marker='o', label='Квадратичный конгруэнтный + перемешивание')
plt.plot(x, mid,  marker='o', label='Серединные квадраты + серединные произведения')
plt.plot(x, fib, marker='o', label='Параллельный Фибоначчи + линейный конгруэнтный')
plt.plot(x, std_mt, marker='o', label='std::mt19937_64 (Mersenne Twister)')

plt.title('Сравнение скорости генерации псевдослучайных чисел')
plt.xlabel('Количество элементов')
plt.ylabel('Время (мс)')
plt.xticks(x, sizes, rotation=45)
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('timing_plot.png', dpi=150)
plt.show()