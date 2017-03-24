# Lightning Network installation

### Requirements:
* Fullnode bitcoind running.
* Open lightning daemon port on firewall (default TCP 8334)

### compile and install protobuf-c from git
```c
git clone https://github.com/protobuf-c/protobuf-c.git
cd protobuf-c
./autogen.sh && ./configure && make && make install
```
### compile from source
```c
git clone https://github.com/BHBNETWORK/lightning
cd lightning/
git checkout azure
make
```

### Running daemon
```c
./daemon/lightningd
```

### Running daemon on specified port/folder
```c
./daemon/lightningd --ignore-dbversion --port 9911 --lightning-dir /tmp/lnd_dir0
```


[CC0 1.0 Universal - Public Domain
2017 Blockchainlab.it](https://creativecommons.org/publicdomain/zero/1.0/)
