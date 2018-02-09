gcc test_boost.cpp -g -o test_boost -lboost_thread -lboost_system -lstdc++

./bootstrap.sh --with-libraries=all --with-toolset=gcc

./b2 toolset=gcc

./b2 install --prefix=/usr
