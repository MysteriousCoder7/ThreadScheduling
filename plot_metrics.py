import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns
sns.set_theme(style='darkgrid')

# Load and pivot the CSV
df = pd.read_csv('metrics.csv')
df = df.set_index('Run Type').T  # Transpose rows into metrics
df.reset_index(inplace=True)
df.columns.name = None
df.rename(columns={'index': 'Metric'}, inplace=True)

# Extract for plotting
metrics = df['Metric']
single = df['Single Processor']
multi = df['Multiprocessor']

# Check lengths of 'single' and 'multi' to prevent indexing errors
num_metrics = min(len(single), len(multi), 5)  # Plot only up to 5 metrics

# Create subplots
fig, axs = plt.subplots(3, 2, figsize=(12, 10))
fig.suptitle('Single vs Multi-Processor Performance', fontsize=16)

# Plot up to the number of available metrics
for idx, ax in enumerate(axs.flat[:num_metrics]):
    # Use bar charts instead of line plots
    ax.bar(['Single', 'Multi'], [single[idx], multi[idx]], label=metrics[idx])
    ax.set_title(metrics[idx])
    ax.set_ylabel('Value')
    ax.set_xlabel('Execution Mode')
    ax.grid(True)
    ax.legend()

# Remove extra subplots if there are fewer than 6 metrics
for i in range(num_metrics, len(axs.flat)):
    fig.delaxes(axs.flat[i])

# Adjust layout and save the figure
plt.tight_layout(rect=[0, 0.03, 1, 0.95])
plt.savefig('performance_comparison_bar.png')
plt.show()
