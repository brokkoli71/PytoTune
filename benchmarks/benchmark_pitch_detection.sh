SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
RESULTS="$SCRIPT_DIR/results_pitch_detection.txt"

pushd ../cmake-build-relwithdebinfo

> "$RESULTS"  # truncate

print_header=true
for hwy in ON OFF; do
    cmake . -DPYTOTUNE_USE_HWY=$hwy -Wno-dev > /dev/null 2>&1
    cmake --build . --target pytotune_benchmarks -j 10 > /dev/null 2>&1

    for omp in ON OFF; do
        omp_threads=$( [ "$omp" = "OFF" ] && echo 1 || echo "" )
        OMP_NUM_THREADS=${omp_threads:-$(nproc)} ./pytotune_benchmarks detection $( [ "$print_header" = "false" ] && echo false ) | tee -a "$RESULTS"
        echo "" >> "$RESULTS"
        print_header=false
    done
done

popd