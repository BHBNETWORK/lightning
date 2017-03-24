# Lightning Network legacy on `azure`

This is a fork of [cdecker/lightning](https://github.com/cdecker/lightning)

`azure`'s branch extend the legacy lightning version (staged in folder [daemon](https://github.com/BHBNETWORK/lightning/tree/azure/daemon)) with:
* pinging every seconds the connection to make it alive
* adding parameter `--payout-address` as explained in [payout-address](https://github.com/BHBNETWORK/lightning/commit/f94d912659a6663fb5d1bbc1cc2929ff7bf34cf6)

# Known issue

Unilateral closing channel is not working nor will be develop in this legacy version.

## Installation

* [lightning-installation.md](lightning-installation.md)

## Usage

* [lightning-usage.md](lightning-usage.md)

## Testing

We tested successfully, "star configuration" both on `testnet` and `shared regtest`

This documentation is released under:
[CC0 1.0 Universal - Public Domain
2017 Blockchainlab.it](https://creativecommons.org/publicdomain/zero/1.0/)
