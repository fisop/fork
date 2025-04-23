FROM ubuntu:24.04

RUN DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y make gcc build-essential
RUN apt-get install -y python3

WORKDIR /fork

COPY . .
