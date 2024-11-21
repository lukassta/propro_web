linux:
	gcc src/backend/web_app.c -l sqlite3 -o web_app

windows:
	gcc src/backend/web_app.c -l sqlite3 -o web_app.exe

