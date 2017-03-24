# Lightning Network installation

This is a fork of [cdecker/lightnin](https://github.com/cdecker/lightning)

`azure`'s branch purpose is to extend the legacy lightning version (staged in folder [daemon](deamon)) with:
* pinging every seconds the connection to make it alive
* adding parameter `--payout-address` as explained in [payout-address](https://github.com/BHBNETWORK/lightning/commit/f94d912659a6663fb5d1bbc1cc2929ff7bf34cf6)

# Known issue

Unilateral closing channel is not working nor will be develop in this legacy version.

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
