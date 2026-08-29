@echo off
echo Connecting to the local Matching Engine...
echo Type your orders below (e.g. "NEW BUY 100 500 10" or "CANCEL 100")

:: We use host.docker.internal to connect from this docker container back to the host port
docker run -it --rm orderbook-engine ./build/order_client --host host.docker.internal --port 9000
