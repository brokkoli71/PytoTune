pushd ../cmake-build-relwithdebinfo

cmake --build . --target pytotune_benchmarks -j 10

# Run the benchmark
#./pytotune_benchmarks midi
#./pytotune_benchmarks scale false
./pytotune_benchmarks midi true
