# Command Line Args

Pass command line args from your game to the engine via the ```EngineStart``` function.

The engine accepts the following command line args, mostly relating to networking and logging:
- ```--role``` - networking role (```-r```)
    - valid options:
        - ```client```
        - ```server```
- ```--server_addres``` - server IP and port (```-s```)
- ```--client_address``` - client IP and port (```-c```)
- ```--log_level``` - drop logs that are below this level (```-l```)
    - valid options (increasing severity):
        - ```verbose``` (```v```)
        - ```info``` (```i```)
        - ```warning``` (```w```)
        - ```error``` (```e```)
- ```--disable_log_colour``` - don't use ansii colour codes in logs (looks nicer when loggin to a file)
- ```--disable_log_timestamp``` - don't add a timestamp to logs
- ```--logfile``` - file to log to (as well as to console)
- ```--disable_log_tid``` - don't include thread id in the logged messages
- ```--disable_console_log``` - don't log to console (but still log to file if one chosen)
- ```--network_sim_config``` - path to network simulator config file to use

If unknown args are encountered they're just ignored so your game can handle its own arguments and then pass argc and argv

