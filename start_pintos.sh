#!/bin/bash

# 사용자의 이름이나 고유한 값을 사용하여 컨테이너 이름 설정
USER_NAME=$(whoami)
CONTAINER_NAME="pintos_${USER_NAME}"
DOCKER_COMPOSE_FILE="docker-compose.yaml"


# docker-compose.yaml 파일 생성
cat <<EOL > ${DOCKER_COMPOSE_FILE}
version: '3'
services:
  dev:
    image: thierrysans/pintos
    container_name: pintos_${USER_NAME}
    volumes:
      - .:/pintos
    working_dir: /pintos
    hostname: pintos_${USER_NAME}
    tty: true
    entrypoint: bash -c "echo 'export TMOUT=0' >> ~/.bashrc && bash"
EOL

# 사용자별 docker-compose 파일로 컨테이너 실행
docker-compose -p pintos_${USER_NAME} up -d --build
# 실행된 컨테이너로 bash 접속
docker exec -it ${CONTAINER_NAME} bash
