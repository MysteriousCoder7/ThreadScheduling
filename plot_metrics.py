import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns
sns.set_theme(style='darkgrid')

df = pd.read_csv('metrics.csv')
df = df.set_index('Run Type').T
df.reset_index(inplace=True)
df.columns.name = None
df.rename(columns={'index': 'Metric'}, inplace=True)

metrics = df['Metric']
single = df['Single Processor']
multi = df['Multiprocessor']


num_metrics = min(len(single), len(multi), 5)


fig, axs = plt.subplots(3, 2, figsize=(12, 10))
fig.suptitle('Single vs Multi-Processor Performance', fontsize=16)

for idx, ax in enumerate(axs.flat[:num_metrics]):
    ax.bar(['Single', 'Multi'], [single[idx], multi[idx]], label=metrics[idx])
    ax.set_title(metrics[idx])
    ax.set_ylabel('Value')
    ax.set_xlabel('Execution Mode')
    ax.grid(True)
    ax.legend()


for i in range(num_metrics, len(axs.flat)):
    fig.delaxes(axs.flat[i])


plt.tight_layout(rect=[0, 0.03, 1, 0.95])
plt.savefig('performance_comparison_bar.png')
plt.show()