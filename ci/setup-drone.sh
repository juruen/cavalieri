#!/bin/bash

# Script to set up building environment for drone.io
# We need to add a few repos because drone's builders are precise (12.04)

set -e

# Use g++4.8
echo 2 | sudo update-alternatives --config gcc

# Preconfigure postfix
echo "postfix postfix/mailname string foo.com" | sudo debconf-set-selections
echo "postfix postfix/main_mailer_type string \'Local only\'" | sudo debconf-set-selections

# Some deps
sudo add-apt-repository -y ppa:fcitx-team/nightly
sudo add-apt-repository -y ppa:svn
sudo add-apt-repository -y ppa:mapnik/boost-backports-1-54

# Some repos seem to fail quite ofen
while ! timeout 45 sh -c "sudo apt-get update"; do
  true
done



# Instal deps
sudo apt-get install debhelper cmake subversion protobuf-compiler \
  libprotobuf-dev libev-dev libgoogle-glog-dev \
  libcurl4-openssl-dev libssl-dev libtbb-dev libjsoncpp-dev lcov \
  flex bison libboost-filesystem-dev libboost-system-dev python2.7-dev \
  postfix

# Drone.io builders are precise
wget https://gflags.googlecode.com/files/libgflags0_2.0-1_amd64.deb
wget https://gflags.googlecode.com/files/libgflags-dev_2.0-1_amd64.deb

sudo dpkg -i libgflags0_2.0-1_amd64.deb libgflags-dev_2.0-1_amd64.deb
