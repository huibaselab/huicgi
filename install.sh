#!/bin/bash

PNAME="cgicomm"

rm -fr ../$PNAME

#apt-get install perl -y
#apt-get install -y libmysqlclient-dev

if [ ! -d deps ]
then 
    mkdir deps;
fi

cd deps

if [ ! -f cgicc.tar.gz ]
then 
    cp ../../deps/cgicc.tar.gz ./
fi

if [ ! -f libconfig.tar.gz ]
then 
    cp ../../deps/libconfig.tar.gz ./
fi

#if [ ! -f openssl.tar.gz ]
#then 
#    cp ../../deps/openssl.tar.gz ./
#fi

#if [ ! -f jsoncpp.tar.gz ]
#then 
#    cp ../../deps/jsoncpp.tar.gz ./
#fi

#if [ ! -f hiredis.tar.gz ]
#then 
#    cp ../../deps/hiredis.tar.gz ./
#fi

tar xvf cgicc.tar.gz
rm -fr cgicc.tar.gz

tar xvf libconfig.tar.gz
rm -fr libconfig.tar.gz

#tar xvf openssl.tar.gz
#rm -fr openssl.tar.gz

#tar xvf jsoncpp.tar.gz
#rm -fr jsoncpp.tar.gz

#tar xvfz hiredis.tar.gz
#rm -fr hiredis.tar.gz

# install cgicc
cd cgicc
./configure
make -j8
cd ..

# install libconfig
cd libconfig
make -j8
cd ..

# install openssl
#cd openssl
#export CFLAGS=-fPIC
#./config shared
#make -j8
#make install
#cd ..

# install hiredis
#cd hiredis
#make -j8
#cd ..

# return to root
cd ..
make

rename_cwd() {
  cd . || return
  new_dir=${PWD%/*}/$1
  mv -- "$PWD" "$new_dir" &&
    cd -- "$new_dir"
}

# change to cgicomm
#thisname=$(basename $(pwd))
#parentdir=$(dirname $(pwd))
#cd ..
#mv $thisname cgicomm
#cd $parentdir/cgicomm
#cd -P .
#cd .
#rename_cwd $PNAME

exit 0
