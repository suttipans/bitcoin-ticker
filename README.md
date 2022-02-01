# bitcoin-ticker
Bitcoin price ticker in Thai Baht and mempool info on M5Stack

<img width="512" alt="btcticker_thb" src="https://user-images.githubusercontent.com/7954929/151911871-f5c9feec-75f5-4cd5-9fae-2b4ad1baac74.png">



## Features
- Digital Clock
- Bitcoin prices in Thai Baht and USD.
- Mempool basic information and fee rates
- Press button B, for receiving address

## Hardware
The hardware I have tested on

|description     | link                                    |
|----------------|-----------------------------------------|
| M5Stack Core2  | https://docs.m5stack.com/en/core/core2  |
| M5Stack Grey   | https://docs.m5stack.com/en/core/gray   |

## Using Arduino IDE
    git clone https://github.com/suttipans/bitcoin-ticker.git
    cd bitcoin-ticker/arduino-ide/btc-ticker
    Open btc-ticker.ino with Arduino IDE

## Using Platformio
    git clone https://github.com/suttipans/bitcoin-ticker.git
    cd bitcoin-ticker
    pio run -t upload --upload_port {your_device}.
    To see list of your devices run the command pio device list
