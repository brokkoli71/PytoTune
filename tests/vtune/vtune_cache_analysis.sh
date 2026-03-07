#!/bin/bash
# VTune Cache Miss Analysis - Line-by-Line Report

RESULT_DIR="../testoutput/vtune_results/cache_with_hwy_x8"
OUTPUT_CSV="$RESULT_DIR/cache_line_report.csv"
PYTOTUNE_CLI="../../cmake-build-relwithdebinfo/pytotune_cli"
WAV_FILE="../data/benchmarking/e-minor-singing-10x.wav"
WAV_OUT="../testoutput/test.wav"

rm -rf "$RESULT_DIR"

echo "Running uarch-exploration analysis..."
echo ""

vtune -collect uarch-exploration \
  -result-dir "$RESULT_DIR" \
  -- $PYTOTUNE_CLI \
  scale $WAV_FILE "C major" $WAV_OUT

echo ""
echo "Generating cache miss report..."

# Get cache metrics as CSV
vtune -report hw-events \
  -result-dir "$RESULT_DIR" \
  -group-by source-line \
  -column="Source File,Source Line,MEM_LOAD_RETIRED.L1_MISS,MEM_LOAD_RETIRED.L3_MISS,MEM_INST_RETIRED.ALL_LOADS,CPU_CLK_UNHALTED.THREAD,INST_RETIRED.ANY" \
  -format csv \
  -report-output "$OUTPUT_CSV"

echo "Processing and sorting by L1 cache misses..."

# Process CSV: clean column names, sort by L1_MISS
python3 - "$OUTPUT_CSV" << 'PYTHON_SCRIPT'
import sys
import pandas as pd

input_csv = sys.argv[1]

# Remove first line if contains war:Column filter is ON.
with open(input_csv, 'r+') as f:
    lines = f.readlines()
    if lines and lines[0].startswith('war:Column filter is ON.'):
        lines = lines[1:]
    f.seek(0)
    f.writelines(lines)

# Read CSV with tab delimiter (VTune uses tabs)
df = pd.read_csv(input_csv, sep='\t')

# Clean column names - remove "Hardware Event Count:" prefix
df.columns = df.columns.str.replace('Hardware Event Count:', '', regex=False).str.strip()

print(df.columns.tolist())  # Debug: print column names to verify
# Convert numeric columns (remove commas if present)
numeric_cols = ['MEM_LOAD_RETIRED.L1_MISS', 'MEM_LOAD_RETIRED.L2_MISS', 'MEM_LOAD_RETIRED.L3_MISS',
                'MEM_INST_RETIRED.ALL_LOADS', 'CPU_CLK_UNHALTED.THREAD', 'INST_RETIRED.ANY']

for col in numeric_cols:
    if col in df.columns:
        df[col] = pd.to_numeric(df[col].astype(str).str.replace(',', ''), errors='coerce')

# Sort by L1_MISS descending, then L3_MISS descending
df = df.sort_values(by=['MEM_LOAD_RETIRED.L1_MISS', 'MEM_LOAD_RETIRED.L3_MISS'], 
                    ascending=[False, False])

# Save sorted CSV
df.to_csv(input_csv, sep='\t', index=False)

PYTHON_SCRIPT

echo ""
echo "Sorted report saved to: $OUTPUT_CSV"
