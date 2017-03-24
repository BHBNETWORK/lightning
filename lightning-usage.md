# Lightning Network usage

This document will guide how to create & close a channel, node and transactions between nodes using a channel.

## Opening a payment channel
To create a new channel we need an address, a transaction id, and a raw trasaction.

### NEW ADDRESS
Generate a new multisig address using Lightning-cli for funding the channel.
#### Command
```c
./lightning-cli --lightning-dir "/tmp/lnd_folder" newaddr
```
#### Response
```c
{
  "err": null,
  "result": "{ "address" : "2N5YkXBXFnqcJ4hKWXxsoGsXohDKtNkBdGw" }",
  "resHeaders": null
}
```
$address: 2N5YkXBXFnqcJ4hKWXxsoGsXohDKtNkBdGw

### SEND TO ADDRESS (TXID)
Fund the previous multisig address creating a transaction from bitcoin-cli
#### Command
```c
bitcoin-cli -testnet sendtoaddress "$address" "$amount"
```
#### Response
```c
{
  "err": null,
  "result": "2a8563a5c72c86bccff230fbd13d34db1bb53a3ad37468b01142636954f8a2d7",
  "resHeaders": null
}
```
$txid: 2a8563a5c72c86bccff230fbd13d34db1bb53a3ad37468b01142636954f8a2d7

### GET RAW TRANSACTION
get the raw transaction from previous TX.
#### Command

```c
bitcoin-cli -testnet getrawtransaction "$txid"
```

#### Response

```c
{
  "err": null,
  "result": "0100000001ee484a7a67584c8736916aa4912c44cb7de3b427fb6e9848a5e815867685ef39010000006b483045022100d63923b1c21386b67f4c95dd599df21b4b75d993e4ea5208cca46308e061798c022029649a0876e20e065dc50663214519a27d261d1da8b9808ae11b317c3099778901210286599777ffd50be2be2f7714935c0eae9c7b30c8d80df1ae507b05b53330c8d1feffffff02102700000000000017a91486f211752d6750fef61f78350bdfe375fc4659488798d67c00000000001976a914a794db94283ce745386d2a74748edaea63fe76f188ac55a61000",
  "resHeaders": null
}
```

### START A CHANNEL
Open the channel using the row transaction.
#### Command
```c
./lightning-cli --lightning-dir "/tmp/lnd_folder" connect "IP Address/localhost" "PORT" "raw transaction key"
```

Please wait...It will take bit time to get response.

#### Response

```c
{
  "err": null,
  "result": "029a335a3231bbad7cd739a671ae5db74434c169948cf5783b3a738779e985372e",
  "resHeaders": null
}
```
Channel: 029a335a3231bbad7cd739a671ae5db74434c169948cf5783b3a738779e985372e

Note: Using API we could get empty response to open a channel, in that case we can use $txid to know the confirmation from http://tbtc.blockr.io

	
### Get Peers
Show peers from network
#### Command
```c
./lightning-cli --lightning-dir "/tmp/lnd_folder" getpeers | jq
```

#### Response
```c
{
  "err": null,
  "result": "{ "peers" : [ { "name" : "032acc25076766a7b45c0907bea30762232c99ef9461f72834beff86ee1646527b:", "state" : "STATE_OPEN_WAIT_ANCHORDEPTH_AND_THEIRCOMPLETE", "peerid" : "032acc25076766a7b45c0907bea30762232c99ef9461f72834beff86ee1646527b", "connected" : true, "our_amount" : 8112000, "our_fee" : 1686000, "their_amount" : 0, "their_fee" : 0, "our_htlcs" : [  ], "their_htlcs\" : [  ] } ] }",
  "resHeaders": null
}
```

### Attention: Opening a payment channel
- During this process there could be an error:

```c
    lightningd(43317): FATAL SIGNAL 6 RECEIVED
    Fatal signal 6. Log dumped in crash.log
    Aborted (core dumped)
```

#### Possible solution:
- use of "-ignore-dbversion" during start lightning service
- by deleting the folder "/tmp/lnd_folder" before starting the lightning service 



## Send/Receive payment via channel

### Invoice
create an invoice to receive a payment.
#### Command

```c
	./lightning-cli --lightning-dir "/tmp/lnd_folder" invoice 1000000 "Invoice # 1"
```

#### Response
```c
{
  "err": null,
  "result": "060715485d7ccac3ba2017a4691c1fbe4ec7d7fed93c8c217b8238fd22223d18",
  "resHeaders": null
}
```

Invoice: 060715485d7ccac3ba2017a4691c1fbe4ec7d7fed93c8c217b8238fd22223d18


### Get Route
Get the route for sending a payment.
#### Command

```c
./lightning-cli --lightning-dir "/tmp/lnd_folder" getroute "node id" "amount" "riskfactor"
```

#### Response

```c
{
  "err": null,
  "result": "[{"id":"029a335a3231bbad7cd739a671ae5db74434c169948cf5783b3a738779e985372e", "msatoshi":100000,"delay":36}]",
  "resHeaders": null
}
```

### Get Nodes
Get lightning nodes from network.
#### Command

```c
./lightning-cli --lightning-dir "/tmp/lnd_folder" getnodes
```

#### Response

```c
{
  "err": null,
  "result": "{ "nodes" :[{ "nodeid" : "024ff16fd0ac3969a52d8568fec16f29f8ef00a315d05a5cc25c262e3ccb6fad6c", "port" : 0, "hostname" : null },{ "nodeid" : "0352e4e03d9e763d9d41beffba3402f7c119ac67a65affdd7c25dec5a0a86177a7", "port" : 8334, "hostname" : "47.93.76.251" },{ "nodeid" : "035dfaa4c932a22018b1e583d2b9f5197240db60151c639e5c75614c0903349339", "port" : 9735, "hostname" : "2001:e42:102:1529:160:16:106:114" },{ "nodeid" : "0239d5b8bce059c3ac28c173f788f97cf87f5ee6d63a91179915d63a40a09c4af9", "port" : 9911, "hostname" : "52.166.151.1" },{ "nodeid" : "032acc25076766a7b45c0907bea30762232c99ef9461f72834beff86ee1646527b", "port" : 0, "hostname" : null },{ "nodeid" : "027583ad5a0f8c34844ade9721b4b57caf5ddc2ccc9abb78a9c9f8ae53c1b83738", "port" : 8899, "hostname" : "40.68.170.170" } ] }",
  "resHeaders": null
}
```


### Send Pay
Send payment using the open channel.
#### Command

```c
	./lightning-cli --lightning-dir "/tmp/lnd_folder" sendpay "route" "hash"
```

#### Response
```c
{
  "err": null,
  "result": "",
  "resHeaders": null
}
```

This documentation is released under:
[CC0 1.0 Universal - Public Domain
2017 Blockchainlab.it](https://creativecommons.org/publicdomain/zero/1.0/)

