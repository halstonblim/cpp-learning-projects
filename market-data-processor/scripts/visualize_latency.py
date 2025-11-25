#!/usr/bin/env python3
"""
Latency Distribution Visualizer for Market Data Processor

This script generates publication-quality histograms comparing the latency
distributions of lock-based vs lock-free queue implementations.

The visualization clearly shows:
- Lock-based: "Fat tail" distribution due to mutex contention
- Lock-free: "Tight cluster" with predictable, low-jitter performance

Usage:
    python3 scripts/visualize_latency.py

    # Or with custom file paths:
    python3 scripts/visualize_latency.py --lock path/to/lock.csv --free path/to/free.csv
"""

import argparse
import sys
from pathlib import Path

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter

# === Configuration ===
FIGURE_SIZE = (14, 8)
DPI = 150
DATA_DIR = Path("data")
OUTPUT_FILE = DATA_DIR / "latency_comparison.png"

# Color palette - distinctive colors for clarity
COLORS = {
    'lock_based': '#E74C3C',   # Warm red for lock-based (slower)
    'lock_free': '#27AE60',    # Cool green for lock-free (faster)
    'grid': '#ECEFF1',
    'text': '#263238',
    'accent': '#546E7A'
}


def load_latency_data(filepath: Path) -> np.ndarray:
    """Load latency data from CSV file."""
    if not filepath.exists():
        raise FileNotFoundError(f"Data file not found: {filepath}")

    df = pd.read_csv(filepath)
    return df['latency_ns'].values


def compute_stats(data: np.ndarray, name: str) -> dict:
    """Compute comprehensive latency statistics."""
    sorted_data = np.sort(data)
    n = len(sorted_data)

    stats = {
        'name': name,
        'count': n,
        'min': sorted_data[0],
        'max': sorted_data[-1],
        'mean': np.mean(data),
        'std': np.std(data),
        'p50': sorted_data[int(n * 0.50)],
        'p90': sorted_data[int(n * 0.90)],
        'p99': sorted_data[int(n * 0.99)],
        'p999': sorted_data[int(n * 0.999)] if n >= 1000 else sorted_data[-1],
    }

    # Jitter metric: coefficient of variation (std / mean)
    stats['jitter_cv'] = (stats['std'] / stats['mean']) * 100

    return stats


def format_ns(value: float, pos=None) -> str:
    """Format nanoseconds with appropriate units."""
    if value >= 1_000_000:
        return f'{value/1_000_000:.1f}ms'
    elif value >= 1_000:
        return f'{value/1_000:.1f}μs'
    else:
        return f'{value:.0f}ns'


def create_visualization(lock_data: np.ndarray, free_data: np.ndarray, output_path: Path):
    """Create the main latency comparison visualization."""

    # Compute statistics
    lock_stats = compute_stats(lock_data, "Lock-Based (Mutex)")
    free_stats = compute_stats(free_data, "Lock-Free (Atomic)")

    # Print stats to console
    print("\n" + "="*60)
    print("LATENCY STATISTICS COMPARISON")
    print("="*60)

    for stats in [lock_stats, free_stats]:
        print(f"\n{stats['name']}:")
        print(f"  Samples:    {stats['count']:,}")
        print(f"  Min:        {format_ns(stats['min'])}")
        print(f"  Mean:       {format_ns(stats['mean'])}")
        print(f"  P50:        {format_ns(stats['p50'])}")
        print(f"  P90:        {format_ns(stats['p90'])}")
        print(f"  P99:        {format_ns(stats['p99'])}")
        print(f"  P99.9:      {format_ns(stats['p999'])}")
        print(f"  Max:        {format_ns(stats['max'])}")
        print(f"  Std Dev:    {format_ns(stats['std'])}")
        print(f"  Jitter CV:  {stats['jitter_cv']:.1f}%")

    # Calculate improvement metrics
    p99_improvement = ((lock_stats['p99'] - free_stats['p99']) / lock_stats['p99']) * 100
    jitter_improvement = ((lock_stats['jitter_cv'] - free_stats['jitter_cv']) / lock_stats['jitter_cv']) * 100

    print(f"\n{'─'*60}")
    print(f"LOCK-FREE ADVANTAGE:")
    print(f"  P99 Latency Reduction:  {p99_improvement:.1f}%")
    print(f"  Jitter Reduction:       {jitter_improvement:.1f}%")
    print("="*60 + "\n")

    # Create figure with custom styling
    plt.style.use('seaborn-v0_8-whitegrid')
    fig, axes = plt.subplots(2, 2, figsize=FIGURE_SIZE, dpi=DPI)
    fig.patch.set_facecolor('#FAFAFA')

    # === Top Left: Overlaid Histogram (Linear Scale) ===
    ax1 = axes[0, 0]

    # Compute shared bins for fair comparison
    all_data = np.concatenate([lock_data, free_data])
    percentile_99_5 = np.percentile(all_data, 99.5)
    bins = np.linspace(0, percentile_99_5, 100)

    ax1.hist(lock_data, bins=bins, alpha=0.7, color=COLORS['lock_based'],
             label=f"Lock-Based (P99: {format_ns(lock_stats['p99'])})",
             edgecolor='white', linewidth=0.5)
    ax1.hist(free_data, bins=bins, alpha=0.7, color=COLORS['lock_free'],
             label=f"Lock-Free (P99: {format_ns(free_stats['p99'])})",
             edgecolor='white', linewidth=0.5)

    ax1.set_xlabel('Latency', fontsize=11, fontweight='medium')
    ax1.set_ylabel('Frequency', fontsize=11, fontweight='medium')
    ax1.set_title('Latency Distribution (Linear Scale)', fontsize=13, fontweight='bold', pad=10)
    ax1.legend(loc='upper right', framealpha=0.9)
    ax1.xaxis.set_major_formatter(FuncFormatter(format_ns))

    # === Top Right: Log-Scale Histogram (Shows Tail Better) ===
    ax2 = axes[0, 1]

    # Use log-spaced bins for tail visibility
    min_val = max(1, min(lock_data.min(), free_data.min()))
    max_val = max(lock_data.max(), free_data.max())
    log_bins = np.logspace(np.log10(min_val), np.log10(max_val), 80)

    ax2.hist(lock_data, bins=log_bins, alpha=0.7, color=COLORS['lock_based'],
             label='Lock-Based', edgecolor='white', linewidth=0.5)
    ax2.hist(free_data, bins=log_bins, alpha=0.7, color=COLORS['lock_free'],
             label='Lock-Free', edgecolor='white', linewidth=0.5)

    ax2.set_xscale('log')
    ax2.set_xlabel('Latency (log scale)', fontsize=11, fontweight='medium')
    ax2.set_ylabel('Frequency', fontsize=11, fontweight='medium')
    ax2.set_title('Latency Distribution (Log Scale) - Reveals Tail', fontsize=13, fontweight='bold', pad=10)
    ax2.legend(loc='upper right', framealpha=0.9)
    ax2.xaxis.set_major_formatter(FuncFormatter(format_ns))

    # === Bottom Left: Cumulative Distribution (CDF) ===
    ax3 = axes[1, 0]

    lock_sorted = np.sort(lock_data)
    free_sorted = np.sort(free_data)
    lock_cdf = np.arange(1, len(lock_sorted) + 1) / len(lock_sorted)
    free_cdf = np.arange(1, len(free_sorted) + 1) / len(free_sorted)

    ax3.plot(lock_sorted, lock_cdf * 100, color=COLORS['lock_based'],
             linewidth=2, label='Lock-Based', alpha=0.9)
    ax3.plot(free_sorted, free_cdf * 100, color=COLORS['lock_free'],
             linewidth=2, label='Lock-Free', alpha=0.9)

    # Add percentile reference lines
    for pct in [50, 90, 99]:
        ax3.axhline(y=pct, color=COLORS['accent'], linestyle='--', alpha=0.5, linewidth=0.8)
        ax3.text(ax3.get_xlim()[1] * 0.95, pct + 1, f'P{pct}',
                fontsize=9, color=COLORS['accent'], ha='right')

    ax3.set_xlabel('Latency', fontsize=11, fontweight='medium')
    ax3.set_ylabel('Cumulative Percentage (%)', fontsize=11, fontweight='medium')
    ax3.set_title('Cumulative Distribution Function (CDF)', fontsize=13, fontweight='bold', pad=10)
    ax3.legend(loc='lower right', framealpha=0.9)
    ax3.xaxis.set_major_formatter(FuncFormatter(format_ns))
    ax3.set_ylim(0, 101)

    # === Bottom Right: Box Plot + Stats Summary ===
    ax4 = axes[1, 1]

    # Create box plot
    bp = ax4.boxplot([lock_data, free_data], labels=['Lock-Based', 'Lock-Free'],
                     patch_artist=True, showfliers=False, widths=0.6)

    # Style the box plots
    bp['boxes'][0].set_facecolor(COLORS['lock_based'])
    bp['boxes'][1].set_facecolor(COLORS['lock_free'])
    for box in bp['boxes']:
        box.set_alpha(0.7)
    for median in bp['medians']:
        median.set_color('white')
        median.set_linewidth(2)

    ax4.set_ylabel('Latency', fontsize=11, fontweight='medium')
    ax4.set_title('Latency Box Plot (Outliers Hidden)', fontsize=13, fontweight='bold', pad=10)
    ax4.yaxis.set_major_formatter(FuncFormatter(format_ns))

    # Add stats text box
    stats_text = (
        f"Lock-Free Advantage:\n"
        f"━━━━━━━━━━━━━━━━━━━━\n"
        f"P99 Reduction: {p99_improvement:.1f}%\n"
        f"Jitter Reduction: {jitter_improvement:.1f}%\n"
        f"\n"
        f"Lock-Free P99: {format_ns(free_stats['p99'])}\n"
        f"Lock-Based P99: {format_ns(lock_stats['p99'])}"
    )

    props = dict(boxstyle='round,pad=0.5', facecolor='white', alpha=0.9, edgecolor=COLORS['accent'])
    ax4.text(0.98, 0.98, stats_text, transform=ax4.transAxes, fontsize=10,
             verticalalignment='top', horizontalalignment='right',
             fontfamily='monospace', bbox=props)

    # === Main Title ===
    fig.suptitle('Lock-Free vs Lock-Based Queue: Latency Jitter Analysis\n',
                 fontsize=16, fontweight='bold', y=0.98)

    plt.tight_layout(rect=[0, 0.02, 1, 0.95])

    # Save figure
    plt.savefig(output_path, dpi=DPI, bbox_inches='tight',
                facecolor=fig.get_facecolor(), edgecolor='none')
    print(f"✓ Visualization saved to: {output_path}")

    # Also save a high-res version for resume/portfolio
    hires_path = output_path.with_stem(output_path.stem + "_hires")
    plt.savefig(hires_path, dpi=300, bbox_inches='tight',
                facecolor=fig.get_facecolor(), edgecolor='none')
    print(f"✓ High-resolution version saved to: {hires_path}")

    plt.close()


def main():
    parser = argparse.ArgumentParser(
        description='Visualize latency distributions from queue benchmarks',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__
    )
    parser.add_argument('--lock', type=Path, default=DATA_DIR / 'latency_lock_based.csv',
                        help='Path to lock-based queue latency CSV')
    parser.add_argument('--free', type=Path, default=DATA_DIR / 'latency_lock_free.csv',
                        help='Path to lock-free queue latency CSV')
    parser.add_argument('--output', '-o', type=Path, default=OUTPUT_FILE,
                        help='Output image path')

    args = parser.parse_args()

    print("="*60)
    print("MARKET DATA PROCESSOR - LATENCY VISUALIZER")
    print("="*60)

    # Check for required packages
    try:
        import matplotlib
        import pandas
        import numpy
    except ImportError as e:
        print(f"\nError: Missing required package: {e.name}")
        print("Install with: pip install matplotlib pandas numpy")
        sys.exit(1)

    # Load data
    try:
        print(f"\nLoading lock-based data from: {args.lock}")
        lock_data = load_latency_data(args.lock)

        print(f"Loading lock-free data from: {args.free}")
        free_data = load_latency_data(args.free)
    except FileNotFoundError as e:
        print(f"\nError: {e}")
        print("\nMake sure you've run the benchmark first:")
        print("  cd build && ./queue_benchmark both")
        sys.exit(1)

    # Create visualization
    create_visualization(lock_data, free_data, args.output)

    print("\n" + "="*60)
    print("Done! Add the generated image to your portfolio/resume.")
    print("="*60)


if __name__ == '__main__':
    main()

