SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
RESULTS="$SCRIPT_DIR/results_pitch_correction.csv"

pushd ../cmake-build-relwithdebinfo

> "$RESULTS"  # truncate

print_header=true
for twiddles in ON OFF; do
    for windowing in ON OFF; do
        cmake . -DPYTOTUNE_USE_PREDEFINED_TWIDDLES=$twiddles -DPYTOTUNE_REIMPLEMENTED_WINDOWING=$windowing -Wno-dev > /dev/null 2>&1
        cmake --build . --target pytotune_benchmarks -j 10 > /dev/null 2>&1

        ./pytotune_benchmarks correction $( [ "$print_header" = "false" ] && echo false ) | tee -a "$RESULTS"
#        echo "" >> "$RESULTS"
        print_header=false
    done
done

popd
