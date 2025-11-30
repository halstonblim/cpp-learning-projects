import pandas as pd
import matplotlib.pyplot as plt
import sys
import os

def plot_benchmark(csv_file, output_file):
    # Load data
    try:
        # The Google Benchmark CSV output includes metadata header lines before the actual CSV data.
        # We need to find the start of the CSV data (the line starting with 'name,iterations').
        header_row = 0
        with open(csv_file, 'r') as f:
            for i, line in enumerate(f):
                if line.startswith('name,iterations'):
                    header_row = i
                    break
        
        df = pd.read_csv(csv_file, skiprows=header_row)
    except FileNotFoundError:
        print(f"Error: Could not find {csv_file}")
        return

    # Google Benchmark CSVs usually have 'name', 'real_time', 'cpu_time', 'time_unit', etc.
    # The 'name' column looks like: "BenchmarkScalarMean/8192"
    
    # Extract the base name and the size (arg)
    # We split by '/' and take the last part as size
    df['Size'] = df['name'].apply(lambda x: int(x.split('/')[-1]))
    df['Test'] = df['name'].apply(lambda x: x.split('/')[0])
    
    # Filter for the two tests we care about
    scalar_df = df[df['Test'] == 'BenchmarkScalarMean'].sort_values('Size')
    avx_df = df[df['Test'] == 'BenchmarkAVXMean'].sort_values('Size')

    # Setup the plot
    plt.style.use('seaborn-v0_8-whitegrid')
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 10), sharex=True)

    # Plot 1: Execution Time
    ax1.plot(scalar_df['Size'], scalar_df['cpu_time'], 'o-', label='Scalar (Baseline)', color='#E74C3C', markersize=8, linewidth=2)
    ax1.plot(avx_df['Size'], avx_df['cpu_time'], 'o-', label='AVX2 (SIMD)', color='#27AE60', markersize=8, linewidth=2)
    
    ax1.set_ylabel('Time (ns)', fontsize=14)
    ax1.set_title('Execution Time: Scalar vs AVX2 Rolling Mean', fontsize=18, fontweight='bold')
    ax1.legend(fontsize=12)
    ax1.set_yscale('log') # Log scale is crucial for large ranges
    ax1.set_xscale('log')
    ax1.grid(True, which="both", ls="-", alpha=0.2)
    ax1.tick_params(axis='both', which='major', labelsize=12)

    # Plot 2: Speedup Factor
    # Merge dataframes to calculate ratio
    merged = pd.merge(scalar_df, avx_df, on='Size', suffixes=('_scalar', '_avx'))
    merged['Speedup'] = merged['cpu_time_scalar'] / merged['cpu_time_avx']

    ax2.plot(merged['Size'], merged['Speedup'], 'D-', color='#2980B9', label='Speedup Factor', markersize=8, linewidth=2)
    
    # Add a reference line at 4x (Theoretical max for doubles in AVX2)
    ax2.axhline(y=4.0, color='#8E44AD', linestyle='--', alpha=0.8, label='Theoretical Max (4x)', linewidth=2)
    
    ax2.set_xlabel('Array Size (Elements)', fontsize=14)
    ax2.set_ylabel('Speedup (x)', fontsize=14)
    ax2.set_title('Relative Performance (Scalar / AVX)', fontsize=16, fontweight='bold')
    ax2.legend(fontsize=12)
    ax2.set_ylim(0, 5) # Scale to show 4x clearly
    ax2.grid(True, which="both", ls="-", alpha=0.2)
    ax2.tick_params(axis='both', which='major', labelsize=12)

    # Annotate the max speedup
    max_speedup = merged['Speedup'].max()
    ax2.text(merged['Size'].iloc[-1], max_speedup + 0.2, f'Max: {max_speedup:.2f}x', 
             ha='center', color='#2980B9', fontweight='bold', fontsize=12)

    plt.tight_layout()
    plt.savefig(output_file, dpi=150)
    print(f"Visualization saved to {output_file}")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python3 visualize_benchmark.py <input.csv> <output.png>")
    else:
        plot_benchmark(sys.argv[1], sys.argv[2])