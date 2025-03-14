#!/bin/bash

# 사용자의 이름이나 고유한 값을 사용하여 컨테이너 이름 설정
USER_NAME=$(whoami)
CONTAINER_NAME="pintos_${USER_NAME}"
IMAGE_NAME="cocopam/pintos-runner:latest"  # Docker Hub에서 가져올 이미지 이름

# 실행할 사용자 및 그룹 ID 가져오기
HOST_UID=$(id -u)
HOST_GID=$(id -g)

# Docker Hub에서 이미지 가져오기
echo "Docker Hub에서 이미지를 가져옵니다: ${IMAGE_NAME}"
docker pull ${IMAGE_NAME}
if [ $? -ne 0 ]; then
    echo "Docker 이미지를 가져오는 데 실패했습니다."
    exit 1
fi
echo "Docker 이미지(${IMAGE_NAME}) 가져오기 완료."

# 기존 컨테이너가 있다면 삭제
EXISTING_CONTAINER=$(docker ps -a --filter "name=${CONTAINER_NAME}" --format "{{.ID}}")
if [ -n "$EXISTING_CONTAINER" ]; then
    echo "기존 컨테이너(${CONTAINER_NAME})가 존재합니다. 삭제를 진행합니다..."
    docker rm -f ${CONTAINER_NAME}
    echo "기존 컨테이너가 삭제되었습니다."
fi

# 컨테이너 실행
echo "컨테이너(${CONTAINER_NAME})를 실행합니다..."
docker run -it \
    --name ${CONTAINER_NAME} \
    --hostname ${CONTAINER_NAME} \
    -e HOST_UID=$(id -u) \
    -e HOST_GID=$(id -g) \
    -v $(pwd):/pintos \
    -w /pintos \
    ${IMAGE_NAME}