linux: src/backend/web_app.o src/backend/server.o src/backend/database.o
	gcc src/backend/web_app.o src/backend/server.o src/backend/database.o -lsqlite3 -lssl -lcrypto -o web_app

windows: src/backend/web_app.o src/backend/server.o src/backend/database.o
	#FIXME:
	gcc src/backend/web_app.o src/backend/server.o src/backend/database.o -o web_app.exe

src/backend/server.o: src/backend/server.c src/backend/headers.h
	gcc src/backend/web_app.c -Wall -Wextra -Werror -Wno-unused-parameter -c -o src/backend/web_app.o

src/backend/server.o: src/backend/server.c src/backend/headers.h
	gcc src/backend/server.c -Wall -Wextra -Werror -Wno-unused-parameter -c -o src/backend/server.o

src/backend/database.o: src/backend/database.c src/backend/headers.h
	gcc src/backend/database.c -Wall -Wextra -Werror -Wno-unused-parameter -c -o src/backend/database.o

cert:
	openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -sha256 -days 365

