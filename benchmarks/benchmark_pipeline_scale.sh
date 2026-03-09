SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
RESULTS="$SCRIPT_DIR/results_pipeline_scale.csv"

pushd ../cmake-build-relwithdebinfo

> "$RESULTS"  # truncate

cmake . -Wno-dev > /dev/null 2>&1
cmake --build . --target pytotune_benchmarks -j 10 > /dev/null 2>&1

./pytotune_benchmarks pipeline_ranges false scale | tee -a "$RESULTS"
./pytotune_benchmarks pipeline_windows false scale | tee -a "$RESULTS"

popd
