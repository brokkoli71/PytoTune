#!/bin/bash
# Generate line-by-line CPU time report for YIN pitch detector

RESULT_DIR="./tests/testoutput/vtune_results/with_hwy"
#OUTPUT_FILE="$RESULT_DIR/yin_line_by_line_report.txt"
OUTPUT_FILE="$RESULT_DIR/line_by_line_report.txt"

rm -rf "$RESULT_DIR"

vtune -collect hotspots \
  -result-dir "$RESULT_DIR" \
  -- cmake-build-relwithdebinfo/pytotune_cli \
  midi tests/data/untitled.wav tests/data/test.mid tests/testoutput/test.wav

echo "Generating line-by-line timing report..."

# Get line-level timings
vtune -report hotspots \
  -result-dir "$RESULT_DIR" \
#  -filter "source-file=yin_pitch_detector.cpp" \
  -group-by source-line \
  -format text \
  > "$OUTPUT_FILE"

echo "Report saved to: $OUTPUT_FILE"
echo ""
cat "$OUTPUT_FILE"
