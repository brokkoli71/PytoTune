pushd ../cmake-build-relwithdebinfo

cmake --build . --target pytotune_benchmarks -j 10

# Run the benchmark
#./pytotune_benchmarks midi
#./pytotune_benchmarks scale false
./pytotune_benchmarks pitch_detection true 1
./pytotune_benchmarks pitch_detection false 2
./pytotune_benchmarks pitch_detection false 4
./pytotune_benchmarks pitch_detection false 8