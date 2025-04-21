import pandas as pd
import matplotlib.pyplot as plt

# Read data from CSV file
file_path = 'metrics.csv'  # Replace with the path to your CSV file
df = pd.read_csv(file_path)

# Plotting the metrics

# 1. Execution Time (s) vs Run Type
plt.figure(figsize=(10, 6))
plt.bar(df['Run Type'], df['Execution Time (s)'], color=['blue', 'green'])
plt.title('Execution Time (s) vs Run Type')
plt.ylabel('Execution Time (s)')
plt.xlabel('Run Type')
plt.show()

# 2. Average Turnaround Time (ms) vs Run Type
plt.figure(figsize=(10, 6))
plt.bar(df['Run Type'], df['Average Turnaround Time (ms)'], color=['blue', 'green'])
plt.title('Average Turnaround Time (ms) vs Run Type')
plt.ylabel('Average Turnaround Time (ms)')
plt.xlabel('Run Type')
plt.show()

# 3. Throughput (tasks/sec) vs Run Type
plt.figure(figsize=(10, 6))
plt.bar(df['Run Type'], df['Throughput (tasks/sec)'], color=['blue', 'green'])
plt.title('Throughput (tasks/sec) vs Run Type')
plt.ylabel('Throughput (tasks/sec)')
plt.xlabel('Run Type')
plt.show()

# 4. CPU Utilization (%) vs Run Type
plt.figure(figsize=(10, 6))
plt.bar(df['Run Type'], df['CPU Utilization (%)'], color=['blue', 'green'])
plt.title('CPU Utilization (%) vs Run Type')
plt.ylabel('CPU Utilization (%)')
plt.xlabel('Run Type')
plt.show()
