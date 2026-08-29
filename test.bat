@echo off
echo Building the Order Book Engine (this may take a minute the first time)...
docker build -t orderbook-engine .

echo.
echo =========================================
echo 1. RUNNING UNIT TESTS
echo =========================================
docker run --rm orderbook-engine ./build/unit_tests

echo.
echo =========================================
echo 2. RUNNING PERFORMANCE BENCHMARK
echo =========================================
docker run --rm orderbook-engine ./build/benchmark --orders 500000 --producers 4

echo.
echo =========================================
echo 3. RUNNING STRESS TEST
echo =========================================
docker run --rm orderbook-engine ./build/stress_test --threads 8 --duration 3

echo.
pause
