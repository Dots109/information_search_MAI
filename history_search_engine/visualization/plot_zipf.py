import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path

def plot_zipf_law(csv_file='output/zipf_analysis.csv', output_dir='output'):
    print("=== Zipf Law Visualization ===")
    
    df = pd.read_csv(csv_file)
    
    Path(output_dir).mkdir(parents=True, exist_ok=True)
    
    plt.style.use('seaborn-v0_8-darkgrid')
    fig, axes = plt.subplots(2, 2, figsize=(16, 12))
    
    ax1 = axes[0, 0]
    ax1.plot(df['rank'][:1000], df['frequency'][:1000], 
             'b-', linewidth=2, label='Actual Frequency', alpha=0.7)
    ax1.plot(df['rank'][:1000], df['zipf_prediction'][:1000], 
             'r--', linewidth=2, label='Zipf Prediction', alpha=0.7)
    ax1.set_xlabel('Rank', fontsize=12)
    ax1.set_ylabel('Frequency', fontsize=12)
    ax1.set_title('Zipf Law: Linear Scale (Top 1000 terms)', fontsize=14, fontweight='bold')
    ax1.legend(fontsize=11)
    ax1.grid(True, alpha=0.3)
    
    ax2 = axes[0, 1]
    ax2.loglog(df['rank'], df['frequency'], 
               'bo', markersize=3, label='Actual', alpha=0.5)
    ax2.loglog(df['rank'], df['zipf_prediction'], 
               'r-', linewidth=2, label='Zipf Law', alpha=0.7)
    ax2.set_xlabel('Rank (log scale)', fontsize=12)
    ax2.set_ylabel('Frequency (log scale)', fontsize=12)
    ax2.set_title('Zipf Law: Log-Log Scale (All terms)', fontsize=14, fontweight='bold')
    ax2.legend(fontsize=11)
    ax2.grid(True, alpha=0.3)
    
    ax3 = axes[1, 0]
    ax3.plot(df['log_rank'], df['log_frequency'], 
             'go', markersize=4, label='Observed', alpha=0.5)
    
    coeffs = np.polyfit(df['log_rank'][:1000], df['log_frequency'][:1000], 1)
    poly_line = np.poly1d(coeffs)
    ax3.plot(df['log_rank'][:1000], poly_line(df['log_rank'][:1000]), 
             'r-', linewidth=2, label=f'Fitted: y={coeffs[0]:.2f}x+{coeffs[1]:.2f}')
    
    ax3.set_xlabel('log₁₀(Rank)', fontsize=12)
    ax3.set_ylabel('log₁₀(Frequency)', fontsize=12)
    ax3.set_title('Zipf Law: Linear Regression', fontsize=14, fontweight='bold')
    ax3.legend(fontsize=11)
    ax3.grid(True, alpha=0.3)
    
    ax4 = axes[1, 1]
    top_30 = df.head(30)
    bars = ax4.barh(range(len(top_30)), top_30['frequency'], color='steelblue', alpha=0.8)
    ax4.set_yticks(range(len(top_30)))
    ax4.set_yticklabels(top_30['term'], fontsize=9)
    ax4.set_xlabel('Frequency', fontsize=12)
    ax4.set_title('Top 30 Most Frequent Terms', fontsize=14, fontweight='bold')
    ax4.invert_yaxis()
    ax4.grid(axis='x', alpha=0.3)
    
    for i, bar in enumerate(bars):
        width = bar.get_width()
        ax4.text(width, bar.get_y() + bar.get_height()/2, 
                f' {int(width)}', ha='left', va='center', fontsize=8)
    
    plt.tight_layout()
    
    output_file = f'{output_dir}/zipf_law_analysis.png'
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"✓ Saved plot to: {output_file}")
    
    stats_file = f'{output_dir}/zipf_statistics.txt'
    with open(stats_file, 'w', encoding='utf-8') as f:
        f.write("=== ZIPF LAW STATISTICS ===\n\n")
        f.write(f"Total unique terms: {len(df)}\n")
        f.write(f"Most frequent term: '{df.iloc[0]['term']}' ({df.iloc[0]['frequency']} occurrences)\n")
        f.write(f"Median frequency: {df['frequency'].median()}\n")
        f.write(f"Mean frequency: {df['frequency'].mean():.2f}\n\n")
        
        f.write(f"Regression coefficient: {coeffs[0]:.4f}\n")
        f.write(f"Expected Zipf coefficient: -1.0\n")
        f.write(f"Deviation from Zipf: {abs(coeffs[0] + 1.0) * 100:.2f}%\n\n")
        
        f.write("TOP 30 TERMS:\n")
        for idx, row in top_30.iterrows():
            f.write(f"{row['rank']:3d}. {row['term']:20s} - {row['frequency']:6d}\n")
    
    print(f"✓ Saved statistics to: {stats_file}")
    
    plt.show()
    
    print("\n✅ Visualization complete!")

if __name__ == "__main__":
    plot_zipf_law()
