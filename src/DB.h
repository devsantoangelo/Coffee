#include <LittleFS.h>

#define FILE_READ       "r"
#define FILE_WRITE      "w"
#define FILE_APPEND     "a"

class DB {

    private:
        String pathFile;

        using FuncCallBack = void(*)( String data );

        void open() {

            LittleFS.begin();
            File file = LittleFS.open(pathFile, FILE_READ );
            if ( file ) {
                LittleFS.end();
            }
            else {
                Serial.println("Erro ao ler arquivo");
            }
        }

    public: 

        DB( String pathF ) {
            pathFile = pathF;
            open();
        }

        void salve( String data ) {

            LittleFS.begin();
            File file = LittleFS.open( pathFile, FILE_APPEND );
            if ( file ) {
                file.print(data);
                file.close();
            }
            LittleFS.end();
        }

        void listAll(FuncCallBack fnc ) {

            LittleFS.begin();
            File file = LittleFS.open( pathFile, FILE_READ );
            
            int sizeData = ( file.size() / 286 ) + 1;

            String dataJson[sizeData];
            int idx = 0;
            while ( file.available() ) { 
                dataJson[idx] = file.readString();
                idx += 1;
            }
            file.close();
            LittleFS.end();
            clean();

            int idx2 = 0;
            while ( idx2 < sizeData ) {
                (* fnc)( dataJson[idx2] );
                idx2 += 1;
            }
        }

        void clean() {
            LittleFS.begin();
            File file = LittleFS.open( pathFile, FILE_WRITE );
            if ( file ) {
                file.print("");
                file.close();
            }
            LittleFS.end();
        }

};