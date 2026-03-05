#!/bin/bash
# Generate line-by-line CPU time report for YIN pitch detector

RESULT_DIR="../testoutput/vtune_results/with_hwy_x8"
OUTPUT_FILE="$RESULT_DIR/line_by_line_report.txt"
PYTOTUNE_CLI="../../cmake-build-relwithdebinfo/pytotune_cli"
WAV_FILE="../data/untitled_8x.wav"
WAV_OUT="../testoutput/test.wav"

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
