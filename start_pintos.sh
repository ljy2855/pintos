#!/bin/bash

# 사용자의 이름이나 고유한 값을 사용하여 컨테이너 이름 설정
USER_NAME=$(whoami)
CONTAINER_NAME="pintos_${USER_NAME}"

# docker-compose.yaml 파일 생성
cat <<EOL > docker-compose.yaml
version: '3'
services:
  dev:
    image: thierrysans/pintos
    container_name: ${CONTAINER_NAME}
    volumes:
      - .:/pintos
    working_dir: /pintos
    hostname: ${CONTAINER_NAME}
    tty: true
    entrypoint: bash
EOL

echo "docker-compose.yaml 파일이 생성되었습니다."
echo "컨테이너 이름은 ${CONTAINER_NAME} 입니다."

# Docker Compose로 컨테이너를 실행
docker-compose up -d --build

# 실행된 컨테이너로 bash 접속
docker exec -it ${CONTAINER_NAME} bash
