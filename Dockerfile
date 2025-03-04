FROM ubuntu:24.04

RUN DEBIAN_FRONTEND=noninteractive

RUN apt update && apt install -y make gcc build-essential
RUN apt install -y python3

WORKDIR /fork

COPY . .
