Install EH enulator

https://github.com/Azure/azure-event-hubs-emulator-installer

bash ./EventHub-Emulator/Scripts/Linux/LaunchEmulator.sh


## Run Sample

install build dependencies
```
  sudo apt-get update
  sudo apt-get install -y git cmake build-essential curl libcurl4-openssl-dev libssl-dev uuid-dev
```


 install https://github.com/Azure/azure-c-shared-utility

follow README instructions, this will install into `/usr/local/[include|lib] /usr/local/cmake/`

install  https://github.com/Azure/azure-uamqp-c.git

follow README instructions, this will install into `/usr/local/[include|lib] /usr/local/cmake/`

create build directory
change to build and run `cmake ..`
run 'make'


https://github.com/Azure/azure-uamqp-c/tree/master/samples/eh_sender_with_sas_token_sample




