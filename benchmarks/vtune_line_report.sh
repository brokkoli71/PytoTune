#!/bin/bash
# Generate line-by-line CPU time report for YIN pitch detector
# make sure to enable:
# sudo sysctl -w kernel.yama.ptrace_scope=0


RESULT_DIR="../tests/testoutput/vtune_results/with_hwy_x8"
OUTPUT_FILE="./line_report.txt"
PYTOTUNE_CLI="../cmake-build-relwithdebinfo/pytotune_cli"
WAV_FILE="../tests/data/benchmarking/e-minor-singing-10x.wav"
WAV_OUT="../tests/testoutput/test.wav"

cmake ../cmake-build-relwithdebinfo -Wno-dev > /dev/null 2>&1
cmake --build ../cmake-build-relwithdebinfo --target pytotune_cli -j 10 > /dev/null 2>&1

rm -rf "$RESULT_DIR"

vtune -collect hotspots \
  -result-dir "$RESULT_DIR" \
  -- $PYTOTUNE_CLI \
  scale $WAV_FILE "C major" $WAV_OUT

echo "Generating line-by-line timing report..."

# Get line-level timings
#  -filter "source-file=yin_pitch_detector.cpp" \
vtune -report hotspots \
  -result-dir "$RESULT_DIR" \
  -group-by source-line \
  -format text \
  > "$OUTPUT_FILE"

echo "Report saved to: $OUTPUT_FILE"
echo ""
cat "$OUTPUT_FILE"
