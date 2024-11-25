linux:
	gcc src/backend/web_app.c -lsqlite3 -lssl -lcrypto -o web_app

windows:
	gcc src/backend/web_app.c -lsqlite3 -lssl -lcrypto -o web_app.exe

cert:
	openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -sha256 -days 365
