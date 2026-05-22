import serial
import sys
import time
import numpy as np
import matplotlib.pyplot as plt

def main(port):
    try:
        ser = serial.Serial(port, 9600, timeout=1)
        time.sleep(2)
    except Exception as e:
        print(f"не удалось подключиться к порту {port}: {e}")
        return

    print("подключено", port)
    min_dist = input("Введите мин расстояние калибровки (см): ")
    max_dist = input("Введите макс расстояние калибровки (см): ")

    command = f"CAL {min_dist} {max_dist}\n"
    ser.write(command.encode())
    print("плавно перемещайте объект перед датчиками")

    ir_values = []
    us_distances = []

    while True:
        if ser.in_waiting > 0:
            try:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if not line:
                    continue

                if line == "DONE":
                    print("\nOK")
                    break

                if line.startswith("DATA"):
                    parts = line.split()
                    if len(parts) == 3:
                        try:
                            ir_val = int(parts[1])
                            us_dist = float(parts[2])

                            if float(min_dist) <= us_dist <= float(max_dist):
                                ir_values.append(ir_val)
                                us_distances.append(us_dist)
                                print(f"Собрана точка: ИК={ir_val}, УЗ={us_dist:.2f} см", end='\r')
                        except ValueError:
                            continue

            except Exception as e:
                continue

    ser.close()

    if len(ir_values) < 5:
        print("С=собрано слишком мало данных для построения модели")
        return

    x = np.array(ir_values)
    y = np.array(us_distances)

    x_plot = np.linspace(min(x), max(x), 500)

    plt.figure(figsize=(10, 6))
    plt.scatter(x, y, color='gray', alpha=0.5, label='калибровка')

    degrees = [1, 2, 3]
    colors = ['blue', 'green', 'red']

    for deg, color in zip(degrees, colors):
        coeffs = np.polyfit(x, y, deg)
        poly_fn = np.poly1d(coeffs)

        y_plot = poly_fn(x_plot)
        plt.plot(x_plot, y_plot, color=color, linewidth=2,
                 label=f'модель (Степень {deg})')

        print(f"\nполином степени {deg}")
        coeff_str = ", ".join([f"{c:.6f}" for c in coeffs])
        print(f"float coeffs_deg{deg}[] = {{{coeff_str}}};")

    plt.title('Калибровочная кривая ИК-датчика (Модель расстояния)')
    plt.xlabel('Значение АЦП (analogRead)')
    plt.ylabel('Расстояние (см)')
    plt.legend()
    plt.grid(True)
    plt.show()

if __name__ == "__main__":
    main(sys.argv[1])