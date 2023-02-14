# irc_server

An internet relay chat server

## How To Use

first run the server it's up to you to choose any port or password you'd like.
we used as a port : 2345 and as a password : 123

```
git clone https://github.com/Asalek/irc_server.git
cd irc_server
make && ./ircserv 2345 123
```

to use this irc server i recommend to use <strong>nc, limechat or pidgin</strong> These are what we test and we sure they work perfectly with our server.

### netcut client (nc) :

	nc (ip_address) (port) 	// ex : nc 10.12.2.2 1066

### LimeChat
	
	click right on limechat bar and fill in the blank with port, ip address, nick name and password

### Pidgin

	click on add to add the server and fill the blank with ip, password and nick name


## TCP /IP MODEL :
	TCP / IP Model contains 4 layer :
		1_Application Layer :
			youtube, google, ... this layer contains protocols,
			such as: HTTP on internet web sites and SMTP email check
			or FTP to transfer files
		2_Transport Layer :
			data become small piece of packages and send them to distination
			then get gathered and check if are all there
		3_Internet Layer :
			uses the internet protocol (IP) to know the sender and reciever
		4_Network Layer :
			uses macAddress to reach the exact machine

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

Please make sure to update tests as appropriate.

## Contact us

feel free to contact me if you counter any issue : 
- discord :
	asalek#7750
	ta×_×ha#0400
- email :
	ayoub.salek8599@gmail.com
        tahanamir@hotmail.com
- Linkedin :

	ayoub :
		https://www.linkedin.com/in/ayoub-salek-3841811b7/

	taha :
		https://www.linkedin.com/in/taha-namir-063ba7256/

## License
[MIT](https://choosealicense.com/licenses/mit/)
