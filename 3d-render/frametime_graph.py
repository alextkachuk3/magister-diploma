import matplotlib.pyplot as plt
import seaborn as sns
import os

def list_log_files(log_dir="log"):
    return sorted([
        f for f in os.listdir(log_dir)
        if f.startswith("frametime_log_") and f.endswith(".txt")
    ], key=lambda name: int(name.split('_')[-1].split('.')[0]))

def main():
    log_dir = "log"
    log_files = list_log_files(log_dir)

    if not log_files:
        print("У папці 'log/' немає лог-файлів.")
        return

    print("Доступні лог-файли:")
    for i, file in enumerate(log_files):
        print(f"[{i}] {file}")

    while True:
        try:
            index = int(input("Введіть номер лог-файлу: "))
            if 0 <= index < len(log_files):
                break
            else:
                print("Невірний номер.")
        except ValueError:
            print("Введіть число.")

    selected_file = os.path.join(log_dir, log_files[index])
    print(f"Завантажено: {selected_file}")

    with open(selected_file, "r") as f:
        frame_times = [float(line.strip()) * 1000.0 for line in f if line.strip()]

    if len(frame_times) <= 5:
        print("Недостатньо даних для побудови графіка.")
        return

    frame_times = frame_times[5:]
    avg_time = sum(frame_times) / len(frame_times)
    min_time = min(frame_times)

    sns.set_theme(style="whitegrid")

    # Збільшуємо висоту для простору зверху
    plt.figure(figsize=(12, 8))
    ax = sns.lineplot(
        x=range(len(frame_times)),
        y=frame_times,
        linewidth=2,
        color='blue',
        label='Час кадру (мс)'
    )
    plt.axhline(y=16.67, color='red', linestyle='--', label='Ціль 60 FPS (16.67 мс)')

    plt.xlabel("Номер кадру", fontsize=12)
    plt.ylabel("Час кадру (мс)", fontsize=12)
    plt.title("")
    plt.xlim(left=0)

    # Додаємо блок тексту (середній + мінімальний час) у верхньому лівому куті
    plt.gcf().text(
        0.01, 0.96,
        f"Середній час кадру: {avg_time:.2f} мс\nМінімальний час кадру: {min_time:.2f} мс",
        fontsize=11,
        color="black",
        ha="left", va="top",
        bbox=dict(boxstyle="round", facecolor="white", edgecolor="gray")
    )

    # Легенда в правому верхньому куті графіка
    plt.legend(loc="upper right", fontsize=10, frameon=True, facecolor="white", edgecolor="gray")

    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    main()
