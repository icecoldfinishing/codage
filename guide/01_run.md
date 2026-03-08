docker build -t codage .

docker build --pull=false -t codage .
docker run --rm codage


gcc main.c resolution.c -o projet ; ./projet