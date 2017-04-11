#ifndef NdefMessage_h
#define NdefMessage_h

#include <Ndef.h>
#include <NdefRecord.h>

#define MAX_NDEF_RECORDS 4

class NdefMessage
{
    public:
        NdefMessage(void);
        NdefMessage(const byte *data, const uint16_t numBytes);
        NdefMessage(const NdefMessage& rhs);
        ~NdefMessage();
        NdefMessage& operator=(const NdefMessage& rhs);

        uint16_t getEncodedSize(); // need so we can pass array to encode
        void encode(byte *data);
        uint16_t getHeaderSize();
        void getHeader(byte* header);
        uint16_t getPackagedSize();
        void getPackaged(uint8_t *data); 


        boolean addRecord(NdefRecord& record);
        void addMimeMediaRecord(const char *mimeType, const char *payload);
        void addMimeMediaRecord(const char *mimeType, const byte *payload, uint16_t payloadLength);
        void addTextRecord(const char *text);
        void addTextRecord(const char *text, const char *encoding);
        void addUriRecord(const char *uri);

        void addExternalRecord(const char *type, const char *payload);
        void addExternalRecord(const char *type, const byte *payload, uint16_t payloadLength);

		/** 
		 * Creates an Android Application Record (AAR) http://developer.android.com/guide/topics/connectivity/nfc/nfc.html#aar
		 * Use an AAR record to cause a P2P message pushed to your Android phone to launch your app even if it's not running.
		 * Note, Android version must be >= 4.0 and your app must have the package you pass to this method
		 * 
		 * @param packageName example: "com.acme.myapp" 
		 */
        void addAndroidApplicationRecord(const char *packageName);

        void addUnknownRecord(const byte *payload, int payloadLength);
        void addEmptyRecord();

        uint8_t getRecordCount();
        NdefRecord getRecord(uint8_t index);
        uint16_t getOffset(uint8_t index);
        NdefRecord operator[](uint8_t index);

#ifdef NDEF_USE_SERIAL
        void print();
#endif
    private:
        NdefRecord _records[MAX_NDEF_RECORDS];
        uint16_t _offsets[MAX_NDEF_RECORDS];    //Stores address offsets of payloads in packaged NDEF
        unsigned int _recordCount;
};

#endif
